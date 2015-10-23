#include<unistd.h>
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
  slotFileName = "slot_table.csv";
  ohGTXLink    = 0;

  for (int i = 0; i < 24; ++i) {
    deviceName.push_back("");
    deviceNum.push_back(-1);
  }

  triggerSource = 0x0;
  deviceChipID  = 0x0; 
  // can't assume a single value for all chips
  deviceVT1     = 0x0; 
  deviceVT2     = 0x0; 

  bag->addField("latency",       &latency );
  bag->addField("outputType",    &outputType  );
  bag->addField("outFileName",   &outFileName );
  bag->addField("slotFileName",  &slotFileName);

  bag->addField("deviceName",    &deviceName );
  bag->addField("deviceNum",     &deviceNum  );

  bag->addField("deviceIP",      &deviceIP    );
  bag->addField("ohGTXLink",     &ohGTXLink   );
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
  // readout_mask, as it is currently implemented, is not sensible in the V2 firmware
  // can consider using this as the tracking/broadcast mask (initializing to 0xffffffff (everything masked off)
  // readout_mask(0xffffffff),
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
  
  wlf_  = toolbox::task::WorkLoopFactory::getInstance();

  // Workloop bindings
  configure_signature_ = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::configureAction, "configureAction");
  start_signature_     = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::startAction,     "startAction"    );
  stop_signature_      = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::stopAction,      "stopAction"     );
  halt_signature_      = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction,      "haltAction"     );
  run_signature_       = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::runAction,       "runAction"      );
  read_signature_      = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::readAction,      "readAction"     );
  select_signature_    = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::selectAction,    "selectAction"   );

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

  m_counter = {0,0,0,0,0};
}

void gem::supervisor::GEMGLIBSupervisorWeb::actionPerformed(xdata::Event& event)
{
  // This is called after all default configuration values have been
  // loaded (from the XDAQ configuration file).
  if (event.type() == "urn:xdaq-event:setDefaultValues") {
    std::stringstream ss;
    ss << "deviceIP=["      << confParams_.bag.deviceIP.toString()      << "]" << std::endl;
    ss << "ohGTXLink=["     << confParams_.bag.ohGTXLink.toString()     << "]" << std::endl;
    ss << "outFileName=["   << confParams_.bag.outFileName.toString()   << "]" << std::endl;
    ss << "slotFileName=["  << confParams_.bag.slotFileName.toString()  << "]" << std::endl;
    ss << "outputType=["    << confParams_.bag.outputType.toString()    << "]" << std::endl;
    ss << "latency=["       << confParams_.bag.latency.toString()       << "]" << std::endl;
    ss << "triggerSource=[" << confParams_.bag.triggerSource.toString() << "]" << std::endl;
    ss << "deviceChipID=["  << confParams_.bag.deviceChipID.toString()  << "]" << std::endl;
    ss << "deviceVT1=["     << confParams_.bag.deviceVT1.toString()     << "]" << std::endl;
    ss << "deviceVT2=["     << confParams_.bag.deviceVT2.toString()     << "]" << std::endl;

    auto num = confParams_.bag.deviceNum.begin();
    for (auto chip = confParams_.bag.deviceName.begin();
      chip != confParams_.bag.deviceName.end(); ++chip, ++num) {
        ss << "Device name: " << chip->toString() << std::endl;
      }
    INFO(ss.str());
    slotInfo = std::unique_ptr<gem::readout::GEMslotContents>(new gem::readout::GEMslotContents(confParams_.bag.slotFileName.toString()));
  }

  // get the workloop instance after loading config parameters
  wl_ = wlf_->getWorkLoop(toolbox::toString("GEMGLIBSupervisorWebWorkLoop_GTX%d",
                                            confParams_.bag.ohGTXLink.value_),
                          "waiting");
  if (!wl_->isActive())
    wl_->activate();
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
    head.addHeader("Refresh","3");
  }
  else if (is_running_) {
    cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
    head.addHeader("Refresh","30");
  }

  if (is_configured_) {
    for (unsigned count = 0; count < 5; ++count) {
      // 0 ttc, 1 internal/firmware, 2 external, 3 loopback, 4 sent
      m_l1aCount[count]      = optohybridDevice_->getL1ACount(count);
      m_calPulseCount[count] = optohybridDevice_->getCalPulseCount(count);
      m_resyncCount[count]   = optohybridDevice_->getResyncCount(count);
      m_bc0Count[count]      = optohybridDevice_->getBC0Count(count);
    }
  }
  
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
  *out << "Current state: "       << theState                           << cgicc::br() << std::endl;
  *out << "Event counter: "       << m_counter[1]  << " Events counter" << cgicc::br() << std::endl;
  // *out << "<table class=\"xdaq-table\">" << std::endl
  *out << cgicc::table().set("class", "xdaq-table") << std::endl
       << cgicc::thead() << std::endl
       << cgicc::tr()    << std::endl // open
       << cgicc::th()    << "T1 counters" << cgicc::th() << std::endl
       << cgicc::th()    << "TTC"         << cgicc::th() << std::endl
       << cgicc::th()    << "Firmware"    << cgicc::th() << std::endl
       << cgicc::th()    << "External"    << cgicc::th() << std::endl
       << cgicc::th()    << "Loopback"    << cgicc::th() << std::endl
       << cgicc::th()    << "Sent"        << cgicc::th() << std::endl
       << cgicc::tr()    << std::endl // close
       << cgicc::thead() << std::endl 
    
       << cgicc::tbody() << std::endl 
       << cgicc::br()    << std::endl;
  *out << cgicc::tr() << std::endl
       << cgicc::td() << "L1A:"        << cgicc::td() << std::endl
       << cgicc::td() << m_l1aCount[0] << cgicc::td() << std::endl
       << cgicc::td() << m_l1aCount[1] << cgicc::td() << std::endl
       << cgicc::td() << m_l1aCount[2] << cgicc::td() << std::endl
       << cgicc::td() << m_l1aCount[3] << cgicc::td() << std::endl
       << cgicc::td() << m_l1aCount[4] << cgicc::td() << std::endl
       << cgicc::tr() << cgicc::br() << std::endl;
  *out << cgicc::tr() << std::endl
       << cgicc::td() << "CalPulse:"        << cgicc::td() << std::endl
       << cgicc::td() << m_calPulseCount[0] << cgicc::td() << std::endl
       << cgicc::td() << m_calPulseCount[1] << cgicc::td() << std::endl
       << cgicc::td() << m_calPulseCount[2] << cgicc::td() << std::endl
       << cgicc::td() << m_calPulseCount[3] << cgicc::td() << std::endl
       << cgicc::td() << m_calPulseCount[4] << cgicc::td() << std::endl
       << cgicc::tr() << cgicc::br() << std::endl;
  *out << cgicc::tr() << std::endl
       << cgicc::td() << "Resync:"        << cgicc::td() << std::endl
       << cgicc::td() << m_resyncCount[0] << cgicc::td() << std::endl
       << cgicc::td() << m_resyncCount[1] << cgicc::td() << std::endl
       << cgicc::td() << m_resyncCount[2] << cgicc::td() << std::endl
       << cgicc::td() << m_resyncCount[3] << cgicc::td() << std::endl
       << cgicc::td() << m_resyncCount[4] << cgicc::td() << std::endl
       << cgicc::tr() << cgicc::br() << std::endl;
  *out << cgicc::tr() << std::endl
       << cgicc::td() << "BC0:"        << cgicc::td() << std::endl
       << cgicc::td() << m_bc0Count[0] << cgicc::td() << std::endl
       << cgicc::td() << m_bc0Count[1] << cgicc::td() << std::endl
       << cgicc::td() << m_bc0Count[2] << cgicc::td() << std::endl
       << cgicc::td() << m_bc0Count[3] << cgicc::td() << std::endl
       << cgicc::td() << m_bc0Count[4] << cgicc::td() << std::endl
       << cgicc::tr() << cgicc::br() << std::endl
       << cgicc::tbody() << std::endl << cgicc::br()
       << cgicc::table() << std::endl << cgicc::br();
  *out << "VFAT blocks counter:       " << m_counter[0] << " dumped to disk" << std::endl << cgicc::br();
  *out << "VFATs counter, last event: " << m_counter[2] << " VFATs chips"    << std::endl << cgicc::br();
  *out << "VFAT good blocks counter:  " << m_counter[3] << " dumped to GEMDAQ" << std::endl << cgicc::br();
  *out << "VFAT bad blocks counter:   " << m_counter[4] << " dumped to ERRORS" << std::endl << cgicc::br();
  *out << "Output filename: " << confParams_.bag.outFileName.toString() << std::endl << cgicc::br();
  *out << "Output type: "     << confParams_.bag.outputType.toString()  << std::endl << cgicc::br();

  // Table with action buttons
  *out << cgicc::table().set("border","0");

  // Row with action buttons
  *out << cgicc::tr();
  if (!is_working_) {
    if (!is_configured_) {
      // Configure button
      *out << cgicc::td();
      std::string configureButton = toolbox::toString("/%s/Configure",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",configureButton) << std::endl;
      *out << cgicc::input().set("type","submit").set("value","Configure")    << std::endl;
      *out << cgicc::form();
      *out << cgicc::td();
    } else {
      if (!is_running_) {
        // Start button
        *out << cgicc::td();
        std::string startButton = toolbox::toString("/%s/Start",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","GET").set("action",startButton) << std::endl;
        *out << cgicc::input().set("type","submit").set("value","Start")    << std::endl;
        *out << cgicc::form();
        *out << cgicc::td();
      } else {
        // Stop button
        *out << cgicc::td() << std::endl;
        std::string stopButton = toolbox::toString("/%s/Stop",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","GET").set("action",stopButton) << std::endl;
        *out << cgicc::input().set("type","submit").set("value","Stop")    << std::endl;
        *out << cgicc::form() << std::endl;
        *out << cgicc::td()   << std::endl;
      }
      // Halt button
      *out << cgicc::td()   << std::endl;
      std::string haltButton = toolbox::toString("/%s/Halt",getApplicationDescriptor()->getURN().c_str());
      *out << cgicc::form().set("method","GET").set("action",haltButton) << std::endl;
      *out << cgicc::input().set("type","submit").set("value","Halt")    << std::endl;
      *out << cgicc::form() << std::endl;
      *out << cgicc::td()   << std::endl;
    
      // Firmware T1 generator
      // make a table, first column is the command, next three are the parameters (N T1 signals, rate, delay (only for L1A+CalPulse)
      *out << cgicc::td() << std::endl; // opens the T1 portion of the command table
      //start new table, but have to not close the previous one
      std::ostringstream t1Table;
      
      //t1Table << cgicc::table().set("border","0") << std::endl
      //t1Table << cgicc::table().set("class", "xdaq-table") << std::endl
      t1Table << "<table \"class\"=\"xdaq-table\">" << std::endl
              << cgicc::thead() << std::endl
              << "<tr>"    << std::endl // open
              << cgicc::th()    << "T1 Signal" << cgicc::th() << std::endl
              << cgicc::th()    << "N T1s"     << cgicc::th() << std::endl
              << cgicc::th()    << "Rate"      << cgicc::th() << std::endl
              << cgicc::th()    << "Delay"     << cgicc::th() << std::endl
              << "</tr>"    << std::endl // close
              << cgicc::thead() << std::endl;
      
      t1Table << cgicc::tbody() << std::endl 
              << "<tr>" << std::endl;

      // Send L1A signal
      std::string triggerButton = toolbox::toString("/%s/Trigger",getApplicationDescriptor()->getURN().c_str());
      t1Table << cgicc::form().set("method","GET").set("action",triggerButton) << std::endl;

      t1Table << "<td>" << std::endl
              << cgicc::input().set("type","submit").set("value","Send L1A") << std::endl
              << "</td>" << std::endl;
      
      t1Table << "<td>" << std::endl
        //<< cgicc::label("NTrigs").set("for","NTrigs")
              << cgicc::input().set("type","number").set("style","width:100px").set("id","NTrigs").set("name","NTrigs")
        .set("min","0x0").set("max","0xFFFFFFFF").set("value","0")
              << std::endl
              << "</td>" << std::endl;
      
      t1Table << "<td>" << std::endl
        //<< cgicc::label("Rate").set("for","Rate")
              << cgicc::input().set("type","number").set("style","width:100px").set("id","Rate").set("name","Rate")
        .set("min","0").set("max","").set("value","2500")
              << std::endl;

      t1Table << "<td>" << std::endl
              << "</td>" << std::endl;

      t1Table << cgicc::form() << std::endl
              << "</tr>"   << std::endl;
    
      // Send L1ACalPulse signal
      t1Table << "<tr>" << std::endl;
      std::string calpulseButton = toolbox::toString("/%s/L1ACalPulse",getApplicationDescriptor()->getURN().c_str());
      t1Table << cgicc::form().set("method","GET").set("action",calpulseButton) << std::endl;

      t1Table << "<td>" << std::endl
              << cgicc::input().set("type","submit").set("value","Send L1ACalPulse") << std::endl
              << "</td>" << std::endl;
      
      t1Table << "<td>" << std::endl
        //<< cgicc::label("NTrigs").set("for","NTrigs")
              << cgicc::input().set("type","number").set("style","width:100px").set("id","NTrigs").set("name","NTrigs")
        .set("min","0x0").set("max","0xFFFFFFFF").set("value","0")
              << "</td>" << std::endl;
      
      t1Table << "<td>" << std::endl
        //<< cgicc::label("Delay").set("for","Delay")
              << cgicc::input().set("type","number").set("style","width:100px").set("id","Delay").set("name","Delay")
        .set("min","0x0").set("max","0xFF").set("value","15")
              << std::endl
              << "</td>" << std::endl;
      
      t1Table << "<td>" << std::endl
        //<< cgicc::label("Rate").set("for","Rate")
              << cgicc::input().set("type","number").set("style","width:100px").set("id","Rate").set("name","Rate")
        .set("min","0").set("max","").set("value","2500")
              << std::endl;
      t1Table << cgicc::form() << std::endl
              << "</tr>"   << std::endl;   

      // Send Resync signal
      t1Table << "<tr>" << std::endl;
      std::string resyncButton = toolbox::toString("/%s/Resync",getApplicationDescriptor()->getURN().c_str());
      t1Table << cgicc::form().set("method","GET").set("action",resyncButton)   << std::endl;
      
      t1Table << "<td>" << std::endl
              << cgicc::input().set("type","submit").set("value","Send Resync") << std::endl
              << "</td>" << std::endl;
      
      t1Table << "<td>" << std::endl
        //<< cgicc::label("NResyncs").set("for","NResyncs")
              << cgicc::input().set("type","number").set("style","width:100px").set("id","NResyncs").set("name","NResyncs")
        .set("min","0x0").set("max","0xFFFFFFFF").set("value","1")
              << "</td>" << std::endl;
      
      t1Table << "<td>" << std::endl
        //<< cgicc::label("Rate").set("for","Rate")
              << cgicc::input().set("type","number").set("style","width:100px").set("id","Rate").set("name","Rate")
        .set("min","0").set("max","").set("value","1")
              << std::endl;

      t1Table << "<td>" << std::endl
              << "</td>" << std::endl;

      t1Table << cgicc::form() << std::endl
              << "</tr>"   << std::endl;
    
      // Send BC0 signal
      t1Table << "<tr>" << std::endl;
      std::string bc0Button = toolbox::toString("/%s/BC0",getApplicationDescriptor()->getURN().c_str());
      t1Table << cgicc::form().set("method","GET").set("action",bc0Button) << std::endl;

      t1Table << "<td>" << std::endl
              << cgicc::input().set("type","submit").set("value","Send BC0") << std::endl
              << "</td>" << std::endl;
      
      t1Table << "<td>" << std::endl
        //<< cgicc::label("NBC0s").set("for","NBC0s")
              << cgicc::input().set("type","number").set("style","width:100px")
        .set("id","NBC0s").set("name","NBC0s")
        .set("min","0x0").set("max","0xFFFFFFFF").set("value","1")
              << std::endl
              << "</td>" << std::endl;
      
      t1Table << "<td>" << std::endl
        //<< cgicc::label("Rate").set("for","Rate")
              << cgicc::input().set("type","number").set("style","width:100px").set("id","Rate").set("name","Rate")
        .set("min","0").set("max","").set("value","1")
              << std::endl;
      
      t1Table << "<td>" << std::endl
              << "</td>" << std::endl;

      t1Table << cgicc::form()  << std::endl
              << "</tr>"    << std::endl;
      
      t1Table << cgicc::tbody() << std::endl
              << "</table>"     << std::endl; // closes the T1 command sub-table
      //using the t1Table stringstream did not reset the tag counter
      //<< cgicc::table() << std::endl; // closes the T1 command sub-table
      *out << t1Table.str();
      *out << cgicc::td()    << std::endl; // closes T1 half of the command table
    }// end is_configured
  }// end is_working
  // Finish row with action buttons
  *out << cgicc::tr();

  // Finish table with action buttons
  *out << cgicc::table();

}

void gem::supervisor::GEMGLIBSupervisorWeb::setParameter(xgi::Input * in, xgi::Output * out ) {
  try{
    cgicc::Cgicc cgi(in);
    confParams_.bag.outputType = cgi["value"]->getValue();

    // re-display form page 
    this->webDefault(in,out);		
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	
}

void gem::supervisor::GEMGLIBSupervisorWeb::webConfigure(xgi::Input * in, xgi::Output * out ) {
  // Derive device number from device name

  /*
  int islot = 0;
  for (auto chip = confParams_.bag.deviceName.begin(); chip != confParams_.bag.deviceName.end(); ++chip, ++islot ) {
    std::string VfatName = chip->toString();
    if (VfatName != ""){ 
      if ( islot >= 0 ) {
        // readout_mask, as it is currently implemented, is not sensible in the V2 firmware
        // can consider using this as the tracking/broadcast mask (initializing to 0xffffffff (everything masked off)
        // readout_mask &= (0xffffffff & 0x0 <<;
        readout_mask |= 0x1 << islot;
        INFO(" webConfigure : DeviceName " << VfatName );
        INFO(" webConfigure : readout_mask 0x" << std::hex << (int)readout_mask << std::dec );
      }
    }// end if VfatName
  }// end for chip
  // hard code the readout mask for now, since this readout mask is an artifact of V1.5 / * *JS Oct 8 * /
  readout_mask = ~readout_mask;
  */
  readout_mask = confParams_.bag.ohGTXLink;

  // Initiate configure workloop
  wl_->submit(configure_signature_);

  INFO(" webConfigure : readout_mask 0x" << std::hex << (int)readout_mask << std::dec);
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
  cgicc::Cgicc cgi(in);
  optohybridDevice_->sendL1A(cgi["NTrigs"]->getIntegerValue(),
                             cgi["Rate"]->getIntegerValue());
  
  for (unsigned count = 0; count < 5; ++count)
    // 0 ttc, 1 internal/firmware, 2 external, 3 loopback, 4 sent
    m_l1aCount[count]      = optohybridDevice_->getL1ACount(count);  

  hw_semaphore_.give();

  // Go back to main web interface
  this->webRedirect(in, out);
}

void gem::supervisor::GEMGLIBSupervisorWeb::webL1ACalPulse(xgi::Input * in, xgi::Output * out ) {
  // Send L1A signal
  hw_semaphore_.take();
  INFO("webCalPulse: CalPulses with L1As delayed");
  cgicc::Cgicc cgi(in);
  optohybridDevice_->sendL1ACal(cgi["NTrigs"]->getIntegerValue(),
                                cgi["Delay"]->getIntegerValue(),
                                cgi["Rate"]->getIntegerValue());
  
  for (unsigned count = 0; count < 5; ++count)
    // 0 ttc, 1 internal/firmware, 2 external, 3 loopback, 4 sent
    m_calPulseCount[count] = optohybridDevice_->getCalPulseCount(count);  

  hw_semaphore_.give();

  // Go back to main web interface
  this->webRedirect(in, out);
}

void gem::supervisor::GEMGLIBSupervisorWeb::webResync(xgi::Input * in, xgi::Output * out ) {
  // Send L1A signal
  hw_semaphore_.take();

  INFO("webResync: sending Resync");
  cgicc::Cgicc cgi(in);
  optohybridDevice_->sendResync(cgi["NResyncs"]->getIntegerValue(),
                                cgi["Rate"]->getIntegerValue());
  for (unsigned count = 0; count < 5; ++count)
    // 0 ttc, 1 internal/firmware, 2 external, 3 loopback, 4 sent
    m_resyncCount[count]   = optohybridDevice_->getResyncCount(count);
  
  hw_semaphore_.give();

  // Go back to main web interface
  this->webRedirect(in, out);
}

void gem::supervisor::GEMGLIBSupervisorWeb::webBC0(xgi::Input * in, xgi::Output * out ) {
  // Send L1A signal
  hw_semaphore_.take();

  INFO("webBC0: sending BC0");
  cgicc::Cgicc cgi(in);
  optohybridDevice_->sendBC0(cgi["NBC0s"]->getIntegerValue(),
                             cgi["Rate"]->getIntegerValue());
  for (unsigned count = 0; count < 5; ++count)
    // 0 ttc, 1 internal/firmware, 2 external, 3 loopback, 4 sent
    m_bc0Count[count]  = optohybridDevice_->getBC0Count(count);
  
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

  optohybridDevice_->sendResync();

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

  uint32_t bufferDepth = 0;
  bufferDepth = glibDevice_->getFIFOVFATBlockOccupancy(readout_mask);
  wl_semaphore_.give();
  hw_semaphore_.give();

  DEBUG("Combined bufferDepth = 0x" << std::hex << bufferDepth << std::dec);

  // If GLIB data buffer has non-zero size, initiate read workloop
  if (bufferDepth>3) {
    wl_->submit(read_signature_);
  }// end bufferDepth

  // should possibly return true so the workloop is automatically resubmitted
  return true;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::readAction(toolbox::task::WorkLoop *wl)
{
  hw_semaphore_.take();

  uint32_t* pDupm = gemDataParker->dumpData(readout_mask);
  if (pDupm) {
    // m_counter[0] = *pDupm;     // VFAT Blocks counter
    // m_counter[1] = *(pDupm+1); // Events counter
    // m_counter[2] = *(pDupm+2); // Sum VFAT per last event
  }

  hw_semaphore_.give();

  // should possibly return true so the workloop is automatically resubmitted
  return false;
}


bool gem::supervisor::GEMGLIBSupervisorWeb::selectAction(toolbox::task::WorkLoop *wl)
{
  // uint32_t  Counter[5] = {0,0,0,0,0};
  uint32_t* pDQ =  gemDataParker->selectData(m_counter);
  if (pDQ) {
    m_counter[0] = *(pDQ+0);
    m_counter[1] = *(pDQ+1); // Events counter
    m_counter[2] = *(pDQ+2); 
    m_counter[3] = *(pDQ+3);
    m_counter[4] = *(pDQ+4);
    m_counter[5] = *(pDQ+5);
  }

  if (is_running_) 
    return true;
  else if (gemDataParker->queueDepth() > 0)
    return true;
  else 
    return false;
}


// State transitions
void gem::supervisor::GEMGLIBSupervisorWeb::configureAction(toolbox::Event::Reference evt) {
  is_working_ = true;
  hw_semaphore_.take();

  m_counter = {0,0,0,0,0};

  std::stringstream tmpURI;
  tmpURI << "chtcp-2.0://localhost:10203?target=" << confParams_.bag.deviceIP.toString() << ":50001";
  glibDevice_ = glib_shared_ptr(new gem::hw::glib::HwGLIB("HwGLIB", tmpURI.str(),
                                                          "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));

  // assumes only a single glib per optohybrid and hard codes the optohybrid to be on GTX 0
  // better to take this as a configuration parameter, or have the active links in this
  std::string ohDeviceName = toolbox::toString("HwOptoHybrid%d",confParams_.bag.ohGTXLink.value_);
  optohybridDevice_ = optohybrid_shared_ptr(new gem::hw::optohybrid::HwOptoHybrid(ohDeviceName, tmpURI.str(),
  //optohybridDevice_ = optohybrid_shared_ptr(new gem::hw::optohybrid::HwOptoHybrid("HwOptoHybrid0", tmpURI.str(),
                                                                                  "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));
  INFO("setTrigSource OH mode 1");
  optohybridDevice_->setTrigSource(0x1);

  // Times for output files
  time_t now  = time(0);
  tm    *gmtm = gmtime(&now);
  char* utcTime = asctime(gmtm);

  // Setup file, information header
  std::string SetupFileName = "Setup_";
  SetupFileName.append(toolbox::toString("GTX%d_",confParams_.bag.ohGTXLink.value_));
  SetupFileName.append(utcTime);
  SetupFileName.erase(std::remove(SetupFileName.begin(), SetupFileName.end(), '\n'), SetupFileName.end());
  SetupFileName.append(".txt");
  std::replace(SetupFileName.begin(), SetupFileName.end(), ' ', '_' );
  std::replace(SetupFileName.begin(), SetupFileName.end(), ':', '-');

  INFO("::configureAction Created Setup file " << SetupFileName );

  std::ofstream SetupFile(SetupFileName.c_str(), std::ios::app );
  if (SetupFile.is_open()){
    SetupFile << std::endl << "The Time & Date : " << utcTime << std::endl;
  }

  int islot=0;
  for (auto chip = confParams_.bag.deviceName.begin(); chip != confParams_.bag.deviceName.end(); ++chip, ++islot) {
    std::string VfatName = chip->toString();

    if (VfatName != ""){ 
      vfat_shared_ptr tmpVFATDevice(new gem::hw::vfat::HwVFAT2(VfatName, tmpURI.str(),
                                                               "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));
      tmpVFATDevice->setDeviceBaseNode(toolbox::toString("GLIB.OptoHybrid_%d.OptoHybrid.GEB.VFATS.%s",
                                                         confParams_.bag.ohGTXLink.value_,
                                                         VfatName.c_str()));
      tmpVFATDevice->setDeviceIPAddress(confParams_.bag.deviceIP);
      tmpVFATDevice->setRunMode(0);
      // need to put all chips in sleep mode to start off
      vfatDevice_.push_back(tmpVFATDevice);
      }
  }
  
  islot=0;
  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip, ++islot) {
    (*chip)->setDeviceIPAddress(confParams_.bag.deviceIP);
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
  std::string tmpFileName = "GEMDAQ_", tmpType = "";
  tmpFileName.append(toolbox::toString("GTX%d_",confParams_.bag.ohGTXLink.value_));
  tmpFileName.append(utcTime);
  tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
  tmpFileName.append(".dat");
  std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
  std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');

  std::string errFileName = "ERRORS_";
  errFileName.append(toolbox::toString("GTX%d_",confParams_.bag.ohGTXLink.value_));
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
  gemDataParker =
    std::shared_ptr<gem::readout::GEMDataParker>(new gem::readout::GEMDataParker(*glibDevice_,
                                                                                 tmpFileName,
                                                                                 errFileName,
                                                                                 tmpType,
                                                                                 confParams_.bag.slotFileName.toString()));
  
  // Data Stream close
  outf.close();
  errf.close();

  if (SetupFile.is_open()){
    SetupFile << " Latency       " << latency_   << std::endl;
    SetupFile << " Threshold     " << deviceVT1_ << std::endl << std::endl;
  }
  // this is not good!!!
  // hw_semaphore_.give();
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

          int islot = slotInfo->GEBslotIndex( (uint32_t)((*chip)->getChipID()));

          if (SetupFile.is_open()){
            SetupFile << " VFAT device connected: slot "
                      << std::setw(2) << std::setfill('0') << islot << " chip ID = 0x" 
                      << std::setw(3) << std::setfill('0') << std::hex
                      << (uint32_t)((*chip)->getChipID())  << std::dec << std::endl;
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

  // is_configured_  = true;
  is_working_     = false;    
  
}

void gem::supervisor::GEMGLIBSupervisorWeb::startAction(toolbox::Event::Reference evt) {
  is_working_ = true;

  is_running_ = true;

  hw_semaphore_.take();

  INFO("setTrigSource OH mode 0");
  optohybridDevice_->setTrigSource(0x0);

  INFO("Enabling run mode for selected VFATs");
  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip)
    (*chip)->setRunMode(1);

  // flush FIFO, how to disable a specific, misbehaving, chip
  INFO("Flushing the FIFOs, readout_mask 0x" <<std::hex << (int)readout_mask << std::dec);
  DEBUG("Flushing FIFO" << readout_mask << " (depth " << glibDevice_->getFIFOOccupancy(readout_mask));
  glibDevice_->flushFIFO(readout_mask);
  while (glibDevice_->hasTrackingData(readout_mask)) {
    glibDevice_->flushFIFO(readout_mask);
    std::vector<uint32_t> dumping = glibDevice_->getTrackingData(readout_mask,
                                                                 glibDevice_->getFIFOVFATBlockOccupancy(readout_mask));
  }
  // once more for luck
  glibDevice_->flushFIFO(readout_mask);

  // send resync
  INFO("Sending a resync");
  optohybridDevice_->sendResync();

  // reset counters
  INFO("Resetting counters");
  optohybridDevice_->resetL1ACount(0x5);
  optohybridDevice_->resetResyncCount(0x5);
  optohybridDevice_->resetBC0Count(0x5);
  optohybridDevice_->resetCalPulseCount(0x5);

  for (unsigned count = 0; count < 5; ++count) {
    // 0 ttc, 1 internal/firmware, 2 external, 3 loopback, 4 sent
    m_l1aCount[count]      = optohybridDevice_->getL1ACount(count);
    m_calPulseCount[count] = optohybridDevice_->getCalPulseCount(count);
    m_resyncCount[count]   = optohybridDevice_->getResyncCount(count);
    m_bc0Count[count]      = optohybridDevice_->getBC0Count(count);
  }

  INFO("setTrigSource OH Trigger source 0x" << std::hex << confParams_.bag.triggerSource << std::dec);
  glibDevice_->flushFIFO(readout_mask);
  optohybridDevice_->sendResync();
  optohybridDevice_->sendBC0();
  optohybridDevice_->sendResync();
  optohybridDevice_->setTrigSource(confParams_.bag.triggerSource);

  hw_semaphore_.give();
  is_working_ = false;

  m_counter = {0,0,0,0,0};// maybe instead reset the counters here in start rather than stop?

  // start running
  wl_->submit(run_signature_);
  wl_->submit(select_signature_);
}

void gem::supervisor::GEMGLIBSupervisorWeb::stopAction(toolbox::Event::Reference evt) {
  is_running_ = false;
  // reset all counters?
  vfat_     = 0;
  event_    = 0;
  sumVFAT_  = 0;
  //m_counter = {0,0,0,0,0}; do not reset displaying counters

  INFO("setTrigSource GLIB, OH mode 0");
  optohybridDevice_->setTrigSource(0x1);

  // turn off all chips?
  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
    (*chip)->setRunMode(0);
    INFO((*chip)->printErrorCounts());
  }
  // flush FIFO, how to disable a specific, misbehaving, chip
  INFO("Flushing the FIFOs, readout_mask 0x" <<std::hex << (int)readout_mask << std::dec);
  glibDevice_->flushFIFO(readout_mask);
  while (glibDevice_->hasTrackingData(readout_mask)) {
    glibDevice_->flushFIFO(readout_mask);
    std::vector<uint32_t> dumping = glibDevice_->getTrackingData(readout_mask,
                                                                 glibDevice_->getFIFOVFATBlockOccupancy(readout_mask));
  }
  // once more for luck
  glibDevice_->flushFIFO(readout_mask);

  wl_->submit(select_signature_);
}

void gem::supervisor::GEMGLIBSupervisorWeb::haltAction(toolbox::Event::Reference evt) {
  is_running_ = false;

  //m_counter = {0,0,0,0,0}; do not reset displaying counters (should possibly treat the same as halt?

  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
    (*chip)->setRunMode(0);
    INFO((*chip)->printErrorCounts());
  }
  // flush FIFO, how to disable a specific, misbehaving, chip
  INFO("Flushing the FIFOs, readout_mask 0x" <<std::hex << (int)readout_mask << std::dec);
  glibDevice_->flushFIFO(readout_mask);
  while (glibDevice_->hasTrackingData(readout_mask)) {
    glibDevice_->flushFIFO(readout_mask);
    std::vector<uint32_t> dumping = glibDevice_->getTrackingData(readout_mask,
                                                                 glibDevice_->getFIFOVFATBlockOccupancy(readout_mask));
  }
  // once more for luck
  glibDevice_->flushFIFO(readout_mask);

  wl_->submit(select_signature_);
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
