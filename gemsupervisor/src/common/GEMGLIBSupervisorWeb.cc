#include "gem/supervisor/GEMGLIBSupervisorWeb.h"
#include "gem/readout/GEMDataParker.h"

#include "gem/hw/vfat/HwVFAT2.h"
#include "gem/hw/glib/HwGLIB.h"
#include "gem/hw/optohybrid/HwOptoHybrid.h"

#include <iomanip>
#include <iostream>
#include <ctime>
#include <sstream>
#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::GEMGLIBSupervisorWeb)

void gem::supervisor::GEMGLIBSupervisorWeb::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{
  latency   = 25U;

  outFileName  = "";
  outputType   = "Hex";

  for (int i = 0; i < 24; ++i) {
    deviceName.push_back("");
    deviceNum.push_back(-1);
  }

  triggerSource = 0x0;
  deviceChipID  = 0x0; 
  //can't assume a single value for all chips
  deviceVT1     = 0x0; 
  deviceVT2     = 0x0; 

  bag->addField("latency",       &latency );
  bag->addField("outputType",    &outputType  );
  bag->addField("outFileName",   &outFileName );

  bag->addField("deviceName",    &deviceName );
  bag->addField("deviceNum",     &deviceNum  );

  bag->addField("deviceIP",      &deviceIP    );
  bag->addField("triggerSource", &triggerSource );
  bag->addField("deviceChipID",  &deviceChipID  );
  bag->addField("deviceVT1",     &deviceVT1   );
  bag->addField("deviceVT2",     &deviceVT2   );

}

// Main constructor
gem::supervisor::GEMGLIBSupervisorWeb::GEMGLIBSupervisorWeb(xdaq::ApplicationStub * s):
  xdaq::WebApplication(s),
  m_gemLogger(this->getApplicationLogger()),
  wl_semaphore_(toolbox::BSem::FULL),
  hw_semaphore_(toolbox::BSem::FULL),
  readout_mask(0x0),
  is_working_ (false),
  is_initialized_ (false),
  is_configured_ (false),
  is_running_ (false)
{
  // Detect when the setting of default parameters has been performed
  this->getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");

  getApplicationInfoSpace()->fireItemAvailable("confParams", &confParams_);
  getApplicationInfoSpace()->fireItemValueRetrieve("confParams", &confParams_);

  // HyperDAQ bindings
  xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::webDefault,     "Default"    );
  xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::webConfigure,   "Configure"  );
  xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::webStart,       "Start"      );
  xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::webStop,        "Stop"       );
  xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::webHalt,        "Halt"       );
  xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::webTrigger,     "Trigger"    );
  xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::webL1ACalPulse, "L1ACalPulse");
  xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::webResync,      "Resync"     );
  xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::webBC0,         "BC0"        );

  xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::setParameter,   "setParameter");

  // SOAP bindings
  xoap::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::onConfigure, "Configure", XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::onStart,     "Start",     XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::onStop,      "Stop",      XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::onHalt,      "Halt",      XDAQ_NS_URI);

  // Initiate and activate main workloop
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("GEMGLIBSupervisorWebWorkLoop", "waiting");
  wl_->activate();

  // Workloop bindings
  configure_signature_ = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::configureAction, "configureAction");
  start_signature_     = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::startAction,     "startAction"    );
  stop_signature_      = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::stopAction,      "stopAction"     );
  halt_signature_      = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction,      "haltAction"     );
  run_signature_       = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::runAction,       "runAction"      );
  read_signature_      = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::readAction,      "readAction"     );

  // Define FSM states
  fsm_.addState('I', "Initial",    this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);
  fsm_.addState('H', "Halted",     this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);
  fsm_.addState('C', "Configured", this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);
  fsm_.addState('R', "Running",    this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);

  // Define error FSM state
  fsm_.setStateName('F', "Error");
  fsm_.setFailedStateTransitionAction(this, &gem::supervisor::GEMGLIBSupervisorWeb::transitionFailed);
  fsm_.setFailedStateTransitionChanged(this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);

  // Define allowed FSM state transitions
  fsm_.addStateTransition('H', 'C', "Configure", this, &gem::supervisor::GEMGLIBSupervisorWeb::configureAction);
  fsm_.addStateTransition('H', 'H', "Halt",      this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction);
  fsm_.addStateTransition('C', 'C', "Configure", this, &gem::supervisor::GEMGLIBSupervisorWeb::configureAction);
  fsm_.addStateTransition('C', 'R', "Start",     this, &gem::supervisor::GEMGLIBSupervisorWeb::startAction);
  fsm_.addStateTransition('C', 'H', "Halt",      this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction);
  fsm_.addStateTransition('R', 'C', "Stop",      this, &gem::supervisor::GEMGLIBSupervisorWeb::stopAction);
  fsm_.addStateTransition('R', 'H', "Halt",      this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction);

  // Define forbidden FSM state transitions
  fsm_.addStateTransition('R', 'R', "Configure" , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);
  fsm_.addStateTransition('H', 'H', "Start"     , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);
  fsm_.addStateTransition('R', 'R', "Start"     , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);
  fsm_.addStateTransition('H', 'H', "Stop"      , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);
  fsm_.addStateTransition('C', 'C', "Stop"      , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);

  // Set initial FSM state and reset FSM
  fsm_.setInitialState('H');
  fsm_.reset();

  counter_ = {0,0,0};

}

void gem::supervisor::GEMGLIBSupervisorWeb::actionPerformed(xdata::Event& event)
{
  // This is called after all default configuration values have been
  // loaded (from the XDAQ configuration file).
  if (event.type() == "urn:xdaq-event:setDefaultValues") {
    std::stringstream ss;
    ss << "deviceIP=["    << confParams_.bag.deviceIP.toString()    << "]" << std::endl;
    ss << "outFileName=[" << confParams_.bag.outFileName.toString() << "]" << std::endl;
    ss << "outputType=["  << confParams_.bag.outputType.toString()  << "]" << std::endl;
    ss << "latency=["     << confParams_.bag.latency.toString()     << "]" << std::endl;
    ss << "triggerSource=[" << confParams_.bag.triggerSource.toString() << "]" << std::endl;
    ss << "deviceChipID=["  << confParams_.bag.deviceChipID.toString()  << "]" << std::endl;
    ss << "deviceVT1=[" << confParams_.bag.deviceVT1.toString() << "]" << std::endl;
    ss << "deviceVT2=[" << confParams_.bag.deviceVT2.toString() << "]" << std::endl;

    auto num = confParams_.bag.deviceNum.begin();
    for (auto chip = confParams_.bag.deviceName.begin();
      chip != confParams_.bag.deviceName.end(); ++chip, ++num) {
        ss << "Device name: " << chip->toString() << std::endl;
      }
    INFO(ss.str());
  }

}

xoap::MessageReference gem::supervisor::GEMGLIBSupervisorWeb::onConfigure(xoap::MessageReference message) {
  is_working_ = true;

  wl_->submit(configure_signature_);
  return message;
}

xoap::MessageReference gem::supervisor::GEMGLIBSupervisorWeb::onStart(xoap::MessageReference message) {
  is_working_ = true;

  wl_->submit(start_signature_);
  return message;
}

xoap::MessageReference gem::supervisor::GEMGLIBSupervisorWeb::onStop(xoap::MessageReference message) {
  is_working_ = true;

  wl_->submit(stop_signature_);
  return message;
}

xoap::MessageReference gem::supervisor::GEMGLIBSupervisorWeb::onHalt(xoap::MessageReference message) {
  is_working_ = true;

  wl_->submit(halt_signature_);
  return message;
}

// HyperDAQ interface
void gem::supervisor::GEMGLIBSupervisorWeb::webDefault(xgi::Input * in, xgi::Output * out ) {
  // Define how often main web interface refreshes
  if (!is_working_ && !is_running_) {
  }
  else if (is_working_) {
    cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
    head.addHeader("Refresh","7");
  }
  else if (is_running_) {
    cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
    head.addHeader("Refresh","7");
  }

  if (is_configured_) {
    //counting "1" Internal triggers, one link enough 
    L1ACount_[0] = optohybridDevice_->GetL1ACount(0); //external
    L1ACount_[1] = optohybridDevice_->GetL1ACount(1); //internal
    L1ACount_[2] = optohybridDevice_->GetL1ACount(2); //delayed
    L1ACount_[3] = optohybridDevice_->GetL1ACount(3); //total
    
    CalPulseCount_[0] = optohybridDevice_->GetCalPulseCount(0); //internal
    CalPulseCount_[1] = optohybridDevice_->GetCalPulseCount(1); //delayed
    CalPulseCount_[2] = optohybridDevice_->GetCalPulseCount(2); //total
    
    ResyncCount_ = optohybridDevice_->GetResyncCount();
    
    BC0Count_ = optohybridDevice_->GetBC0Count();
  }
  // If we are in "Running" state, check if GLIB has any data available
  if (is_running_) wl_->submit(run_signature_);

  // Page title
  *out << cgicc::h1("GEM DAQ Supervisor")<< std::endl;

  // Choose DAQ type: Spy or Global
  *out << "DAQ type: " << cgicc::select().set("name", "runtype");
  *out << cgicc::option().set("value", "Spy").set("selected","") << "Spy" << cgicc::option();
  *out << cgicc::option().set("value", "Global") << "Global" << cgicc::option();
  *out << cgicc::select() << std::endl;
  *out << cgicc::input().set("type", "submit").set("name", "command").set("title", "Set DAQ type").set("value", "Set DAQ type") 
       << cgicc::br() << cgicc::br();

  *out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;") << std::endl;
  std::string method = toolbox::toString("/%s/setParameter",getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::legend("Set Hex/Binary of output") << cgicc::p() << std::endl;
  *out << cgicc::form().set("method","GET").set("action", method) << std::endl;
  *out << cgicc::input().set("type","text").set("name","value").set("value", confParams_.bag.outputType.toString())   << std::endl;
  *out << cgicc::input().set("type","submit").set("value","Apply")  << std::endl;
  *out << cgicc::form() << std::endl;
  *out << cgicc::fieldset();

  // Show current state, counter, output filename
  std::string theState = fsm_.getStateName(fsm_.getCurrentState());
  *out << "Current state: "       << theState                          << cgicc::br();
  *out << "Event counter: "       << counter_[1]  << " Events counter" << cgicc::br();
  *out << "L1A counter: "         << L1ACount_[0] << " (external) "
       << L1ACount_[1] << " (internal) "
       << L1ACount_[2] << " (delayed) "
       << L1ACount_[3] << " (total)"
       << cgicc::br();
  *out << "CalPulse counter: "
       << CalPulseCount_[0] << " (internal) "
       << CalPulseCount_[1] << " (delayed) "
       << CalPulseCount_[2] << " (total)"
       << cgicc::br();
  *out << "Resync counter: "      << ResyncCount_    << cgicc::br();
  *out << "BC0 counter: "         << BC0Count_       << cgicc::br();
  *out << "VFAT blocks counter: " << counter_[0]     << " dumped to disk"          << cgicc::br();
  *out << "VFATs counter, last event: " << counter_[2]     << " VFATs chips" << cgicc::br();
  *out << "Output filename: "     << confParams_.bag.outFileName.toString()        << cgicc::br();
  *out << "Output type: "         << confParams_.bag.outputType.toString()         << cgicc::br();

  // Table with action buttons
  *out << cgicc::table().set("border","0");

  // Row with action buttons
  *out << cgicc::tr();
  if (!is_working_) {
    if (!is_configured_) {
      // Configure button
      *out << cgicc::td();
      std::string configureButton = toolbox::toString("/%s/Configure",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",configureButton) << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","Configure")    << std::endl ;
      *out << cgicc::form();
      *out << cgicc::td();
    } else {
      if (!is_running_) {
        // Start button
        *out << cgicc::td();
        std::string startButton = toolbox::toString("/%s/Start",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","GET").set("action",startButton) << std::endl ;
        *out << cgicc::input().set("type","submit").set("value","Start")    << std::endl ;
        *out << cgicc::form();
        *out << cgicc::td();
      } else {
        // Stop button
        *out << cgicc::td();
        std::string stopButton = toolbox::toString("/%s/Stop",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","GET").set("action",stopButton) << std::endl ;
        *out << cgicc::input().set("type","submit").set("value","Stop")    << std::endl ;
        *out << cgicc::form();
        *out << cgicc::td();
      }
      // Halt button
      *out << cgicc::td();
      std::string haltButton = toolbox::toString("/%s/Halt",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",haltButton) << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","Halt")    << std::endl ;
      *out << cgicc::form();
      *out << cgicc::td();
    
      // Send L1A signal
      *out << cgicc::td();
      std::string triggerButton = toolbox::toString("/%s/Trigger",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",triggerButton) << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","Send L1A")   << std::endl ;
      *out << cgicc::form();
      *out << cgicc::td();
    
      // Send L1ACalPulse signal
      *out << cgicc::td();
      std::string calpulseButton = toolbox::toString("/%s/L1ACalPulse",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",calpulseButton)      << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","Send L1ACalPulse") << std::endl ;
      *out << cgicc::form();
      *out << cgicc::td();
    
      // Send Resync signal
      *out << cgicc::td();
      std::string resyncButton = toolbox::toString("/%s/Resync",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",resyncButton)   << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","Send Resync") << std::endl ;
      *out << cgicc::form();
      *out << cgicc::td();
    
      // Send BC0 signal
      *out << cgicc::td();
      std::string bc0Button = toolbox::toString("/%s/BC0",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",bc0Button)   << std::endl ;
      *out << cgicc::input().set("type","submit").set("value","Send BC0") << std::endl ;
      *out << cgicc::form();
      *out << cgicc::td();
    }// end is_configured
  }//end is_working
  // Finish row with action buttons
  *out << cgicc::tr();

  // Finish table with action buttons
  *out << cgicc::table();

}

void gem::supervisor::GEMGLIBSupervisorWeb::setParameter(xgi::Input * in, xgi::Output * out ) {
  try{
    cgicc::Cgicc cgi(in);
    confParams_.bag.outputType = cgi["value"]->getValue();
    //INFO(" outputType " << confParams_.bag.outputType.toString());

    // re-display form page 
    this->webDefault(in,out);		
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	
}

void gem::supervisor::GEMGLIBSupervisorWeb::webConfigure(xgi::Input * in, xgi::Output * out ) {
  // Derive device number from device name

  int islot=0;
  for (auto chip = confParams_.bag.deviceName.begin(); chip != confParams_.bag.deviceName.end(); ++chip, ++islot ) {
    std::string VfatName = chip->toString();
    if (VfatName != ""){ 
      if ( islot >= 0 ) {
        if (islot < 8)
          readout_mask |= 0x1; //slot [0-7] maps to 1
        else if (islot < 16)
          readout_mask |= 0x2; //slot [8-15] maps to 2
        else if (islot < 24)
          readout_mask |= 0x4; //slot [16-23] maps to 4
  
        INFO(" webConfigure : DeviceName " << VfatName );
        INFO(" webConfigure : readout_mask 0x"  << std::hex << (int)readout_mask << std::dec );
      }
    }//end if VfatName
  }//end for chip
  
  // Initiate configure workloop
  wl_->submit(configure_signature_);

  INFO(" webConfigure : readout_mask 0x"  << std::hex << (int)readout_mask << std::dec);
  // Go back to main web interface
  this->webRedirect(in, out);
}

void gem::supervisor::GEMGLIBSupervisorWeb::webStart(xgi::Input * in, xgi::Output * out ) {
  // Initiate start workloop
  wl_->submit(start_signature_);
    
  // Go back to main web interface
  this->webRedirect(in, out);
}

void gem::supervisor::GEMGLIBSupervisorWeb::webStop(xgi::Input * in, xgi::Output * out ) {
  // Initiate stop workloop
  wl_->submit(stop_signature_);

  // Go back to main web interface
  this->webRedirect(in, out);
}

void gem::supervisor::GEMGLIBSupervisorWeb::webHalt(xgi::Input * in, xgi::Output * out ) {
  // Initiate halt workloop
  wl_->submit(halt_signature_);

  // Go back to main web interface
  this->webRedirect(in, out);
}

void gem::supervisor::GEMGLIBSupervisorWeb::webTrigger(xgi::Input * in, xgi::Output * out ) {
  // Send L1A signal
  hw_semaphore_.take();

  INFO(" webTrigger: sending L1A");
  optohybridDevice_->SendL1A(100);

  L1ACount_[0] = optohybridDevice_->GetL1ACount(0); //external
  L1ACount_[1] = optohybridDevice_->GetL1ACount(1); //internal
  L1ACount_[2] = optohybridDevice_->GetL1ACount(2); //delayed
  L1ACount_[3] = optohybridDevice_->GetL1ACount(3); //total

  hw_semaphore_.give();

  // Go back to main web interface
  this->webRedirect(in, out);
}

void gem::supervisor::GEMGLIBSupervisorWeb::webL1ACalPulse(xgi::Input * in, xgi::Output * out ) {
  // Send L1A signal
  hw_semaphore_.take();
  //INFO("webCalPulse: sending 1 CalPulse with 25 clock delayed L1A");
  for (int offset = -12; offset < 13; ++offset) {
    INFO("webCalPulse: sending 10 CalPulses with L1As delayed by " << (int)latency_ + offset <<  " clocks");
    optohybridDevice_->SendL1ACal(2, latency_ + offset);
    INFO("Sleeping for 0.5 seconds...");
    sleep(0.5);
    INFO("back!");
  }
  //optohybridDevice_->SendL1ACal(1, latency_);
  //sleep(0.1);
  //need some sleep here?
  CalPulseCount_[0] = optohybridDevice_->GetCalPulseCount(0); //internal
  CalPulseCount_[1] = optohybridDevice_->GetCalPulseCount(1); //delayed
  CalPulseCount_[2] = optohybridDevice_->GetCalPulseCount(2); //total
  
  hw_semaphore_.give();

  // Go back to main web interface
  this->webRedirect(in, out);
}

void gem::supervisor::GEMGLIBSupervisorWeb::webResync(xgi::Input * in, xgi::Output * out ) {
  // Send L1A signal
  hw_semaphore_.take();

  INFO("webResync: sending Resync");
  optohybridDevice_->SendResync();
  ResyncCount_ = optohybridDevice_->GetResyncCount();

  hw_semaphore_.give();

  // Go back to main web interface
  this->webRedirect(in, out);
}

void gem::supervisor::GEMGLIBSupervisorWeb::webBC0(xgi::Input * in, xgi::Output * out ) {
  // Send L1A signal
  hw_semaphore_.take();

  INFO("webBC0: sending BC0");
  optohybridDevice_->SendBC0();
  BC0Count_ = optohybridDevice_->GetBC0Count();

  hw_semaphore_.give();

  // Go back to main web interface
  this->webRedirect(in, out);
}

void gem::supervisor::GEMGLIBSupervisorWeb::webRedirect(xgi::Input *in, xgi::Output* out)  {
  // Redirect to main web interface
  std::string url = "/" + getApplicationDescriptor()->getURN() + "/Default";
  *out << "<meta http-equiv=\"refresh\" content=\"0;" << url << "\">" << std::endl;

  this->webDefault(in,out);
}

bool gem::supervisor::GEMGLIBSupervisorWeb::configureAction(toolbox::task::WorkLoop *wl)
{
  // fire "Configure" event to FSM
  fireEvent("Configure");

  optohybridDevice_->SendResync();
  // resetting BX counter
  // optohybridDevice_->ResetBXCount();
  //   ERROR - No branch found with ID-path "OptoHybrid.OptoHybrid_LINKS.LINK0.COUNTERS.RESETS.BXCount"

  return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::startAction(toolbox::task::WorkLoop *wl)
{
  // fire "Start" event to FSM
  fireEvent("Start");
  return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::stopAction(toolbox::task::WorkLoop *wl)
{
  // Fire "Stop" event to FSM
  fireEvent("Stop");
  return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::haltAction(toolbox::task::WorkLoop *wl)
{
  // Fire "Halt" event to FSM
  fireEvent("Halt");
  return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::runAction(toolbox::task::WorkLoop *wl)
{
  wl_semaphore_.take();
  hw_semaphore_.take();

  // GLIB data buffer validation
  boost::format linkForm("LINK%d");
  uint32_t fifoDepth[3];

  //lots of repetition here
  if (readout_mask&0x1)
    fifoDepth[0] = glibDevice_->getFIFOOccupancy(0x0);
  if (readout_mask&0x2)
    fifoDepth[1] = glibDevice_->getFIFOOccupancy(0x1);
  if (readout_mask&0x4)
    fifoDepth[2] = glibDevice_->getFIFOOccupancy(0x2);
    
  if (fifoDepth[0])
    INFO("bufferDepth[0] (runAction) = " << std::hex << fifoDepth[0] << std::dec);
  if (fifoDepth[1])
    INFO("bufferDepth[1] (runAction) = " << std::hex << fifoDepth[1] << std::dec);
  if (fifoDepth[2])
    INFO("bufferDepth[2] (runAction) = " << std::hex << fifoDepth[2] << std::dec);

  // Get the size of GLIB data buffer
  uint32_t bufferDepth = 0;
  if (readout_mask&0x1)
    bufferDepth  = glibDevice_->getFIFOOccupancy(0x0);
  if (readout_mask&0x2)
    bufferDepth += glibDevice_->getFIFOOccupancy(0x1);
  if (readout_mask&0x4)
    bufferDepth += glibDevice_->getFIFOOccupancy(0x2);

  wl_semaphore_.give();
  hw_semaphore_.give();

  INFO("Combined bufferDepth = " << std::hex << bufferDepth << std::dec);

  // If GLIB data buffer has non-zero size, initiate read workloop
  if (bufferDepth) {
    wl_->submit(read_signature_);
  }

  return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::readAction(toolbox::task::WorkLoop *wl)
{
  wl_semaphore_.take();
  hw_semaphore_.take();

  uint64_t* pDupm = gemDataParker->dumpData(readout_mask);
  if (pDupm) {
    counter_[0] = *pDupm;     // VFAT Blocks counter
    counter_[1] = *(pDupm+1); // Events counter
    counter_[2] = *(pDupm+2); // Sum VFAT per last event
  }

  hw_semaphore_.give();
  wl_semaphore_.give();

  return false;
}

// State transitions
void gem::supervisor::GEMGLIBSupervisorWeb::configureAction(toolbox::Event::Reference evt) {
  is_working_ = true;
  hw_semaphore_.take();

  counter_ = {0,0,0};

  std::stringstream tmpURI;
  tmpURI << "chtcp-2.0://localhost:10203?target=" << confParams_.bag.deviceIP.toString() << ":50001";
  //glibDevice_ = glib_shared_ptr(new gem::hw::glib::HwGLIB());
  glibDevice_ = glib_shared_ptr(new gem::hw::glib::HwGLIB("HwGLIB", tmpURI.str(),
                                                          "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));
  //glibDevice_->connectDevice();

  optohybridDevice_ = optohybrid_shared_ptr(new gem::hw::optohybrid::HwOptoHybrid("HwOptoHybrid", tmpURI.str(),
                                                                                  "file://${GEM_ADDRESS_TABLE_PATH}/optohybrid_address_table.xml"));
  //optohybridDevice_->setDeviceIPAddress(confParams_.bag.deviceIP);
  //optohybridDevice_->connectDevice();

  // Times for output files
  time_t now  = time(0);
  tm    *gmtm = gmtime(&now);
  char* utcTime = asctime(gmtm);

  // Setup file, information header
  std::string SetupFileName = "Setup_";
  SetupFileName.append(utcTime);
  SetupFileName.erase(std::remove(SetupFileName.begin(), SetupFileName.end(), '\n'), SetupFileName.end());
  SetupFileName.append(".txt");
  std::replace(SetupFileName.begin(), SetupFileName.end(), ' ', '_' );
  std::replace(SetupFileName.begin(), SetupFileName.end(), ':', '-');

  LOG4CPLUS_INFO(getApplicationLogger(),"::configureAction " << "Created Setup file " << SetupFileName );

  std::ofstream SetupFile(SetupFileName.c_str(), std::ios::app );
  if (SetupFile.is_open()){
    SetupFile << std::endl << "The Time & Date : " << utcTime << std::endl;
  }

  int islot=0;
  for (auto chip = confParams_.bag.deviceName.begin(); chip != confParams_.bag.deviceName.end(); ++chip, ++islot) {
    std::string VfatName = chip->toString();

    if (VfatName != ""){ 
      vfat_shared_ptr tmpVFATDevice(new gem::hw::vfat::HwVFAT2(VfatName, tmpURI.str(),
                                                               "file://${GEM_ADDRESS_TABLE_PATH}/geb_vfat_address_table.xml"));
      tmpVFATDevice->setDeviceIPAddress(confParams_.bag.deviceIP);
      //tmpVFATDevice->connectDevice();
      tmpVFATDevice->setRunMode(0);
      // need to put all chips in sleep mode to start off
      vfatDevice_.push_back(tmpVFATDevice);
      }
  }
  
  islot=0;
  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip, ++islot) {
    (*chip)->setDeviceIPAddress(confParams_.bag.deviceIP);
    //(*chip)->connectDevice();
    (*chip)->readVFAT2Counters();
    (*chip)->setRunMode(0);

    confParams_.bag.deviceChipID = (*chip)->getChipID();
    
    latency_   = confParams_.bag.latency;
    deviceVT1_ = confParams_.bag.deviceVT1;

    // Set VFAT2 registers
    (*chip)->loadDefaults();
    (*chip)->setLatency(latency_);
    confParams_.bag.latency = (*chip)->getLatency();
    
    (*chip)->setVThreshold1(deviceVT1_);
    confParams_.bag.deviceVT1 = (*chip)->getVThreshold1();

    (*chip)->setVThreshold2(0);
    confParams_.bag.deviceVT2 = (*chip)->getVThreshold2();
  }

  // Create a new output file for Data flow
  std::string tmpFileName = "GEM_DAQ_", tmpType = "";
  tmpFileName.append(utcTime);
  tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
  tmpFileName.append(".dat");
  std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
  std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');

  std::string errFileName = "ERRORS_";
  errFileName.append(utcTime);
  errFileName.erase(std::remove(errFileName.begin(), errFileName.end(), '\n'), errFileName.end());
  errFileName.append(".dat");
  std::replace(errFileName.begin(), errFileName.end(), ' ', '_' );
  std::replace(errFileName.begin(), errFileName.end(), ':', '-');

  confParams_.bag.outFileName = tmpFileName;
  std::ofstream outf(tmpFileName.c_str(), std::ios_base::app | std::ios::binary );
  std::ofstream errf(errFileName.c_str(), std::ios_base::app | std::ios::binary );

  tmpType = confParams_.bag.outputType.toString();

  // Book GEM Data Parker
  gemDataParker = std::shared_ptr<gem::readout::GEMDataParker>(new gem::readout::GEMDataParker(*glibDevice_, tmpFileName, errFileName, tmpType));

  // Data Stream close
  outf.close();
  errf.close();

  if (SetupFile.is_open()){
    SetupFile << " Latency       " << latency_   << std::endl;
    SetupFile << " Threshold     " << deviceVT1_ << std::endl << std::endl;
  }
  ////this is not good!!!
  //hw_semaphore_.give();
  /** Super hacky, also doesn't work as the state is taken from the FSM rather
      than this parameter (as it should), J.S July 16
      Failure of any of the conditions at the moment does't take the FSM to error, should it? J.S. Sep 13
  */
  if (glibDevice_->isHwConnected()) {
    INFO("GLIB device connected");
    if (optohybridDevice_->isHwConnected()) {
      INFO("OptoHybrid device connected");
      for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
        if ((*chip)->isHwConnected()) {
          INFO("VFAT device connected: chip ID = 0x"
               << std::setw(4) << std::setfill('0') << std::hex
               << (uint32_t)((*chip)->getChipID())  << std::dec);
          INFO((*chip)->printErrorCounts());

          int islot = gem::readout::GEMslotContents::GEBslotIndex( (uint32_t)((*chip)->getChipID()) );

          if (SetupFile.is_open()){
            SetupFile << " VFAT device connected: slot "
                      << std::setw(2) << std::setfill('0') << islot << " chip ID = 0x" 
                      << std::setw(3) << std::setfill('0') << std::hex
                      << (uint32_t)((*chip)->getChipID()) << std::dec << std::endl;
            (*chip)->printDefaults(SetupFile);
          }
          is_configured_  = true;
        } else {
          INFO("VFAT device not connected, breaking out");
          is_configured_  = false;
          is_working_     = false;    
          hw_semaphore_.give();
          // Setup header close, don't leave open file handles laying around
          SetupFile.close();
          return;
        }
      }
    } else {
      INFO("OptoHybrid device not connected, breaking out");
      is_configured_  = false;
      is_working_     = false;    
      hw_semaphore_.give();
      // Setup header close, don't leave open file handles laying around
      SetupFile.close();
      return;
    }
  } else {
    INFO("GLIB device not connected, breaking out");
    is_configured_  = false;
    is_working_     = false;    
    hw_semaphore_.give();
    // Setup header close, don't leave open file handles laying around
    SetupFile.close();
    return;
  }
  hw_semaphore_.give();

  // Setup header close
  SetupFile.close();

  //is_configured_  = true;
  is_working_     = false;    
  
}

void gem::supervisor::GEMGLIBSupervisorWeb::startAction(toolbox::Event::Reference evt) {
  is_working_ = true;

  is_running_ = true;
  hw_semaphore_.take();

  /*
   * set trigger source
  optohybridDevice_->setTrigSource(0x0);
  optohybridDevice_->setSBitSource((unsigned)confParams_.bag.deviceNum[11]);
  glibDevice_->setSBitSource((unsigned)confParams_.bag.deviceNum[11]);
  */

  INFO("Enabling run mode for selected VFATs");
  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip)
    (*chip)->setRunMode(1);

  //flush FIFO, how to disable a specific, misbehaving, chip
  INFO("Flushing the FIFOs, readout_mask 0x" <<std::hex << (int)readout_mask << std::dec);
  for (int i = 0; i < 2; ++i) {
    DEBUG("Flushing FIFO" << i << " (depth " << glibDevice_->getFIFOOccupancy(i));
    if ((readout_mask >> i)&0x1) {
      DEBUG("Flushing FIFO" << i << " (depth " << glibDevice_->getFIFOOccupancy(i));
      glibDevice_->flushFIFO(i);
      while (glibDevice_->hasTrackingData(i)) {
        glibDevice_->flushFIFO(i);
        std::vector<uint32_t> dumping = glibDevice_->getTrackingData(i);
      }
      glibDevice_->flushFIFO(i);
    }
  }

  //send resync
  INFO("Sending a resync");
  optohybridDevice_->SendResync();

  //reset counters
  INFO("Resetting counters");
  optohybridDevice_->ResetL1ACount(0x4);
  L1ACount_[0] = optohybridDevice_->GetL1ACount(0); //external
  L1ACount_[1] = optohybridDevice_->GetL1ACount(1); //internal
  L1ACount_[2] = optohybridDevice_->GetL1ACount(2); //delayed
  L1ACount_[3] = optohybridDevice_->GetL1ACount(3); //total

  optohybridDevice_->ResetResyncCount();
  ResyncCount_ = optohybridDevice_->GetResyncCount();

  optohybridDevice_->ResetBC0Count();
  BC0Count_ = optohybridDevice_->GetBC0Count();

  optohybridDevice_->ResetCalPulseCount(0x3);
  CalPulseCount_[0] = optohybridDevice_->GetCalPulseCount(0); //internal
  CalPulseCount_[1] = optohybridDevice_->GetCalPulseCount(1); //delayed
  CalPulseCount_[2] = optohybridDevice_->GetCalPulseCount(2); //total

  hw_semaphore_.give();
  is_working_ = false;
}

void gem::supervisor::GEMGLIBSupervisorWeb::stopAction(toolbox::Event::Reference evt) {
  is_running_ = false;
  //reset all counters?
  vfat_ = 0;
  event_ = 0;
  sumVFAT_ = 0;
  counter_ = {0,0,0};
  //turn off all chips?
  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
    (*chip)->setRunMode(0);
    //using smart_ptr
    //delete (*chip);
    //(*chip) = NULL;
    INFO((*chip)->printErrorCounts());
  }
}

void gem::supervisor::GEMGLIBSupervisorWeb::haltAction(toolbox::Event::Reference evt) {
  is_running_ = false;

  counter_ = {0,0,0};

  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
    (*chip)->setRunMode(0);
    //using smart_ptr
    //delete (*chip);
    //(*chip) = NULL;
    INFO((*chip)->printErrorCounts());
  }
  /*
  delete glibDevice_;
  glibDevice_ = NULL;

  delete optohybridDevice_;
  optohybridDevice_ = NULL;

  delete gemDataParker;
  gemDataParker = NULL;
  */
  is_configured_ = false;
}

void gem::supervisor::GEMGLIBSupervisorWeb::noAction(toolbox::Event::Reference evt) {
}

void gem::supervisor::GEMGLIBSupervisorWeb::fireEvent(std::string name) {
  toolbox::Event::Reference event(new toolbox::Event(name, this));
  fsm_.fireEvent(event);
}

void gem::supervisor::GEMGLIBSupervisorWeb::stateChanged(toolbox::fsm::FiniteStateMachine &fsm) {
}

void gem::supervisor::GEMGLIBSupervisorWeb::transitionFailed(toolbox::Event::Reference event) {
}
