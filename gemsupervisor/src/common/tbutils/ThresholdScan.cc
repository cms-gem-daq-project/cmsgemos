#include "gem/supervisor/tbutils/ThresholdScan.h"
#include "gem/hw/vfat/HwVFAT2.h"

#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::tbutils::ThresholdScan)

void gem::supervisor::tbutils::ThresholdScan::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{
  readoutDelay = 1U; //readout delay in milleseconds/microseconds?

  latency   = 128U;
  nTriggers = 2500U;
  minThresh = -25;
  maxThresh = 0;
  stepSize  = 1U;

  deviceName   = "";
  deviceChipID = 0x0;
  deviceVT1    = 0x0;
  deviceVT2    = 0x0;

  triggersSeen = 0;
  
  bag->addField("readoutDelay",   &readoutDelay);

  bag->addField("nTriggers",   &nTriggers);
  bag->addField("minThresh",   &minThresh);
  bag->addField("maxThresh",   &maxThresh);
  bag->addField("stepSize",    &stepSize );

  bag->addField("deviceName",   &deviceName  );
  bag->addField("deviceChipID", &deviceChipID);
  bag->addField("deviceVT1",    &deviceVT1   );
  bag->addField("deviceVT2",    &deviceVT2   );
  bag->addField("triggersSeen", &triggersSeen);

}

gem::supervisor::tbutils::ThresholdScan::ThresholdScan(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception) :
  xdaq::WebApplication(s),
  fsmP_(0),
  wl_semaphore_(toolbox::BSem::FULL),
  initSig_ (0),
  confSig_ (0),
  startSig_(0),
  stopSig_ (0),
  haltSig_ (0),
  runSig_  (0),
  readSig_ (0),
  //deviceName_(""),
  //deviceChipID_(0x0),
  is_working_     (false),
  is_initialized_ (false),
  is_configured_  (false),
  is_running_     (false),
  vfatDevice_(0)
{
  
  // Detect when the setting of default parameters has been performed
  this->getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");

  getApplicationInfoSpace()->fireItemAvailable("confParams",   &confParams_);
  //getApplicationInfoSpace()->fireItemAvailable("deviceName",   &deviceName_);
  //getApplicationInfoSpace()->fireItemAvailable("deviceChipID", &deviceChipID_);
  //getApplicationInfoSpace()->fireItemAvailable("deviceVT1",    &deviceVT1_);
  //getApplicationInfoSpace()->fireItemAvailable("deviceVT2",    &deviceVT2_);
  //getApplicationInfoSpace()->fireItemAvailable("triggersSeen", &triggersSeen_);

  getApplicationInfoSpace()->fireItemValueRetrieve("confParams", &confParams_);

  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webDefault,     "Default"   );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webInitialize,  "Initialize");
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webConfigure,   "Configure" );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webStart,       "Start"     );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webStop,        "Stop"      );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webHalt,        "Halt"      );
  
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onInitialize,  "Initialize",  XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onConfigure,   "Configure",   XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onStart,       "Start",       XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onStop,        "Stop",        XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onHalt,        "Halt",        XDAQ_NS_URI);
  
  initSig_  = toolbox::task::bind(this, &ThresholdScan::initialize, "initialize");
  confSig_  = toolbox::task::bind(this, &ThresholdScan::configure,  "configure" );
  startSig_ = toolbox::task::bind(this, &ThresholdScan::start,      "start"     );
  stopSig_  = toolbox::task::bind(this, &ThresholdScan::stop,       "stop"      );
  haltSig_  = toolbox::task::bind(this, &ThresholdScan::halt,       "halt"      );
  runSig_   = toolbox::task::bind(this, &ThresholdScan::run,        "run"       );
  readSig_  = toolbox::task::bind(this, &ThresholdScan::readFIFO,   "readFIFO"  );

  fsmP_ = new toolbox::fsm::AsynchronousFiniteStateMachine("GEMTestBeamThresholdScan");
  
  fsmP_->addState('I', "Initial",     this, &gem::supervisor::tbutils::ThresholdScan::stateChanged);
  fsmP_->addState('H', "Halted",      this, &gem::supervisor::tbutils::ThresholdScan::stateChanged);
  fsmP_->addState('C', "Configured",  this, &gem::supervisor::tbutils::ThresholdScan::stateChanged);
  fsmP_->addState('E', "Running",     this, &gem::supervisor::tbutils::ThresholdScan::stateChanged);
  
  fsmP_->setStateName('F', "Error");
  fsmP_->setFailedStateTransitionAction(this,  &gem::supervisor::tbutils::ThresholdScan::transitionFailed);
  fsmP_->setFailedStateTransitionChanged(this, &gem::supervisor::tbutils::ThresholdScan::stateChanged);
  
  fsmP_->addStateTransition('I', 'H', "Initialize", this, &gem::supervisor::tbutils::ThresholdScan::initializeAction);
  fsmP_->addStateTransition('H', 'C', "Configure",  this, &gem::supervisor::tbutils::ThresholdScan::configureAction);
  fsmP_->addStateTransition('C', 'C', "Configure",  this, &gem::supervisor::tbutils::ThresholdScan::configureAction);
  fsmP_->addStateTransition('C', 'E', "Start",      this, &gem::supervisor::tbutils::ThresholdScan::startAction);
  fsmP_->addStateTransition('E', 'C', "Stop",       this, &gem::supervisor::tbutils::ThresholdScan::stopAction);
  fsmP_->addStateTransition('C', 'H', "Halt",       this, &gem::supervisor::tbutils::ThresholdScan::haltAction);
  fsmP_->addStateTransition('E', 'H', "Halt",       this, &gem::supervisor::tbutils::ThresholdScan::haltAction);
  fsmP_->addStateTransition('H', 'H', "Halt",       this, &gem::supervisor::tbutils::ThresholdScan::haltAction);

  // Define invalid transitions, too, so that they can be ignored, or else FSM will be unhappy when one is fired.
  fsmP_->addStateTransition('E', 'E', "Configure", this, &gem::supervisor::tbutils::ThresholdScan::noAction);
  fsmP_->addStateTransition('H', 'H', "Start"    , this, &gem::supervisor::tbutils::ThresholdScan::noAction);
  fsmP_->addStateTransition('E', 'E', "Start"    , this, &gem::supervisor::tbutils::ThresholdScan::noAction);
  fsmP_->addStateTransition('H', 'H', "Stop"     , this, &gem::supervisor::tbutils::ThresholdScan::noAction);
  fsmP_->addStateTransition('C', 'C', "Stop"     , this, &gem::supervisor::tbutils::ThresholdScan::noAction);


  fsmP_->setInitialState('I');
  fsmP_->reset();

  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:ThresholdScan","waiting");
  wl_->activate();
}

gem::supervisor::tbutils::ThresholdScan::~ThresholdScan()
  
{
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:ThresholdScan","waiting");
  wl_->cancel();
  wl_ = 0;
  
  if (fsmP_)
    delete fsmP_;
  fsmP_ = 0;
  
}


void gem::supervisor::tbutils::ThresholdScan::actionPerformed(xdata::Event& event)
{
  // This is called after all default configuration values have been
  // loaded (from the XDAQ configuration file).

}

void gem::supervisor::tbutils::ThresholdScan::fireEvent(const std::string& name)
{
  toolbox::Event::Reference event((new toolbox::Event(name, this)));  
  fsmP_->fireEvent(event);
}

void gem::supervisor::tbutils::ThresholdScan::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
{
  //keep_refresh_ = false;
  
  LOG4CPLUS_DEBUG(getApplicationLogger(),"Current state is: [" << fsm.getStateName (fsm.getCurrentState()) << "]");
  std::string state_=fsm.getStateName (fsm.getCurrentState());
  
  LOG4CPLUS_DEBUG(getApplicationLogger(), "StateChanged: " << (std::string)state_);
  
}

void gem::supervisor::tbutils::ThresholdScan::transitionFailed(toolbox::Event::Reference event)
{
  //keep_refresh_ = false;
  toolbox::fsm::FailedEvent &failed = dynamic_cast<toolbox::fsm::FailedEvent&>(*event);
  
  std::stringstream reason;
  reason << "<![CDATA["
         << std::endl
         << "Failure occurred when performing transition"
         << " from "        << failed.getFromState()
         << " to "          << failed.getToState()
         << ". Exception: " << xcept::stdformat_exception_history( failed.getException() )
         << std::endl
         << "]]>";
  
  LOG4CPLUS_ERROR(getApplicationLogger(), reason.str());
}



//Actions
bool gem::supervisor::tbutils::ThresholdScan::initialize(toolbox::task::WorkLoop* wl)
{
  
  fireEvent("Initialize");
  
  return false; //do once?
}

bool gem::supervisor::tbutils::ThresholdScan::configure(toolbox::task::WorkLoop* wl)
{
  //here the configuration of the parametrs should be done

  fireEvent("Configure");
  return false; //do once?
}

bool gem::supervisor::tbutils::ThresholdScan::start(toolbox::task::WorkLoop* wl)
{
  fireEvent("Start");
  //repeat continuously? or just send the command to start and have the hardware take care of the rest?
  return false;
}

bool gem::supervisor::tbutils::ThresholdScan::stop(toolbox::task::WorkLoop* wl)
{
  fireEvent("Stop");
  return false; //do once?
}

bool gem::supervisor::tbutils::ThresholdScan::halt(toolbox::task::WorkLoop* wl)
{
  fireEvent("Halt");
  return false; //do once?
}

bool gem::supervisor::tbutils::ThresholdScan::run(toolbox::task::WorkLoop* wl)
{

  wl_semaphore_.take();
  if (!is_running_)
    vfatDevice_->setRunMode(1);
  
  if ((confParams_.bag.deviceVT2-confParams_.bag.deviceVT1) <= confParams_.bag.maxThresh) {
    if ((uint64_t)(confParams_.bag.triggersSeen) < (uint64_t)(confParams_.bag.nTriggers)) {
      if (true) {
	//if (bufferEmpty) {
	//update triggersSeen
	sleep(1);
      }
      else {
	wl_semaphore_.give();
	//maybe don't do the readout as a workloop?
	wl_->submit(readSig_);
      }
      return true;
    }
    else {
      wl_semaphore_.give();
      wl_->submit(readSig_);
      vfatDevice_->setVThreshold1(confParams_.bag.deviceVT1 + confParams_.bag.stepSize);
      confParams_.bag.deviceVT1    = vfatDevice_->getVThreshold1();
      confParams_.bag.triggersSeen = 0;
      return true;
    }
  }
  else {
    wl_semaphore_.give();
    wl_->submit(readSig_);
    return false;
  }
}

bool gem::supervisor::tbutils::ThresholdScan::readFIFO(toolbox::task::WorkLoop* wl)
{
  wl_semaphore_.take();
  vfatDevice_->setRunMode(0);
  sleep(5);
  //read the fifo (x3 times fifo depth), add headers, write to disk, save disk
  boost::format linkForm("glib_links.link%d");
  //should all links have the same fifo depth? if not is this an error?

  uint32_t fifoDepth[3];
  fifoDepth[0] = vfatDevice_->readReg(boost::str(linkForm%(link))+".tracking_data.FIFO_depth");
  fifoDepth[1] = vfatDevice_->readReg(boost::str(linkForm%(link))+".tracking_data.FIFO_depth");
  fifoDepth[2] = vfatDevice_->readReg(boost::str(linkForm%(link))+".tracking_data.FIFO_depth");

  //check that the fifos are all the same size?
  int bufferDepth = 0;
  if (fifoDepth[0] != fifoDepth[1] || 
      fifoDepth[0] != fifoDepth[2] || 
      fifoDepth[1] != fifoDepth[2]) {
    LOG4CPLUS_INFO(getApplicationLogger(), "tracking data fifos had different depths:: "
		   << fifoDepth[0] << ","
		   << fifoDepth[0] << ","
		   << fifoDepth[0]);
    //use the minimum
    bufferDepth = std::min(fifoDepth[0],std::min(fifoDepth[1],fifoDepth[2]));
  }
    
  //grab events from the fifo
  for ( int fiEvt = 0; fiEvt < bufferDepth; ++fiEvt) {
    //create the event header (run number, event number, anything else?
    
    //createCommonEventHeader();
    //loop over the links
    for (int link = 0; link < 3; ++link) {
      fifoDepth[link] = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");
      std::stringstream regname;
      regname << "tracking_data.COL" << link << ".DATA_RDY";
      vfatDevice_->readReg(regname.str());
      //block read the 6 words (are the words from each vfat read in sequentially?
      
      //how do we put all the data from a single trigger into a single event?
      //block read
      //vfatPacket = vfatDevice_->readReg("tracking_data.COL" << link << ".DATA");
    }
    //createCommonEventTrailer();
  }
  //header should have event number...
  //return to running
  wl_semaphore_.give();
  return false;
}

// SOAP interface
xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onInitialize(xoap::MessageReference message)
  throw (xoap::exception::Exception) {

  wl_->submit(initSig_);
  return message;
}


xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onConfigure(xoap::MessageReference message)
  throw (xoap::exception::Exception) {

  wl_->submit(confSig_);
  return message;
}


xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onStart(xoap::MessageReference message)
  throw (xoap::exception::Exception) {

  wl_->submit(startSig_);
  return message;
}


xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onStop(xoap::MessageReference message)
  throw (xoap::exception::Exception) {

  wl_->submit(stopSig_);
  return message;
}


xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onHalt(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  wl_->submit(haltSig_);
  return message;
}

void gem::supervisor::tbutils::ThresholdScan::selectVFAT(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    *out << cgicc::div()   << std::endl
	 << cgicc::table() << cgicc::br() << std::endl
	 << cgicc::tr()    << cgicc::br() << std::endl
	 << cgicc::td() << "Selected VFAT:" << cgicc::td() << cgicc::br() << std::endl
	 << cgicc::td() << "ChipID:"        << cgicc::td() << cgicc::br() << std::endl
	 << cgicc::tr() << cgicc::br()      << std::endl
	 << cgicc::td() << std::endl
	 << cgicc::select().set("id","VFATDevice").set("name","VFATDevice")     << std::endl
	 << ((confParams_.bag.deviceName.toString().compare("CMS_hybrid_J8")) == 0 ?
	     (cgicc::option("CMS_hybrid_J8").set("value","CMS_hybrid_J8").set("selected")) :
	     (cgicc::option("CMS_hybrid_J8").set("value","CMS_hybrid_J8"))) << std::endl
	 << ((confParams_.bag.deviceName.toString().compare("CMS_hybrid_J26")) == 0 ?
	     (cgicc::option("CMS_hybrid_J26").set("value","CMS_hybrid_J26").set("selected")) :
	     (cgicc::option("CMS_hybrid_J26").set("value","CMS_hybrid_J26"))) << std::endl
	 << ((confParams_.bag.deviceName.toString().compare("CMS_hybrid_J44")) == 0 ?
	     (cgicc::option("CMS_hybrid_J44").set("value","CMS_hybrid_J44").set("selected")) :
	     (cgicc::option("CMS_hybrid_J44").set("value","CMS_hybrid_J44"))) << std::endl
      
	 << ((confParams_.bag.deviceName.toString().compare("TOTEM_hybrid_J58")) == 0 ?
	     (cgicc::option("TOTEM_hybrid_J58").set("value","TOTEM_hybrid_J58").set("selected")) :
	     (cgicc::option("TOTEM_hybrid_J58").set("value","TOTEM_hybrid_J58"))) << std::endl
	 << ((confParams_.bag.deviceName.toString().compare("TOTEM_hybrid_J57")) == 0 ?
	     (cgicc::option("TOTEM_hybrid_J57").set("value","TOTEM_hybrid_J57").set("selected")) :
	     (cgicc::option("TOTEM_hybrid_J57").set("value","TOTEM_hybrid_J57"))) << std::endl
	 << ((confParams_.bag.deviceName.toString().compare("TOTEM_hybrid_J56")) == 0 ?
	     (cgicc::option("TOTEM_hybrid_J56").set("value","TOTEM_hybrid_J56").set("selected")) :
	     (cgicc::option("TOTEM_hybrid_J56").set("value","TOTEM_hybrid_J56"))) << std::endl
	 << cgicc::select()<< std::endl
	 << cgicc::td() << cgicc::br() << std::endl
      
	 << cgicc::td() 
	 << cgicc::input().set("type","text").set("id","ChipID")
                          .set("name","ChipID").set("readonly")
                          .set("value",boost::str(boost::format("0x%04x")%(confParams_.bag.deviceChipID)))
	 << std::endl
	 << cgicc::td()    << cgicc::br() << std::endl
	 << cgicc::tr()    << cgicc::br() << std::endl
	 << cgicc::table() << cgicc::br() << std::endl
	 << cgicc::div()   << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(),"Something went wrong displaying VFATS(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(),"Something went wrong displaying VFATS(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}


void gem::supervisor::tbutils::ThresholdScan::scanParameters(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    *out << cgicc::div()   << std::endl
	 << cgicc::label("Latency").set("for","Latency") << std::endl
	 << cgicc::input().set("id","Latency").set("name","Latency")
                          .set("type","number").set("min","0").set("max","255")
                          .set("value",boost::str(boost::format("%d")%static_cast<unsigned>(confParams_.bag.latency)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("MinThreshold").set("for","MinThreshold") << std::endl
	 << cgicc::input().set("id","MinThreshold").set("name","MinThreshold")
                          .set("type","number").set("min","-255").set("max","255")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.minThresh)))
	 << std::endl

	 << cgicc::label("MaxThreshold").set("for","MaxThreshold") << std::endl
	 << cgicc::input().set("id","MaxThreshold").set("name","MaxThreshold")
                          .set("type","number").set("min","-255").set("max","255")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.maxThresh)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("VStep").set("for","VStep") << std::endl
	 << cgicc::input().set("id","VStep").set("name","VStep")
                          .set("type","number").set("min","1").set("max","255")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.stepSize)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("VT1").set("for","VT1") << std::endl
	 << cgicc::input().set("id","VT1").set("name","VT1").set("readonly")
                          .set("value",boost::str(boost::format("%d")%static_cast<unsigned>(confParams_.bag.deviceVT1)))
	 << std::endl

	 << cgicc::label("VF2").set("for","VF2") << std::endl
	 << cgicc::input().set("id","VF2").set("name","VF2").set("readonly")
                          .set("value",boost::str(boost::format("%d")%static_cast<unsigned>(confParams_.bag.deviceVT2)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("NTrigsStep").set("for","NTrigsStep") << std::endl
	 << cgicc::input().set("id","NTrigsStep").set("name","NTrigsStep")
                          .set("type","number").set("min","0")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.nTriggers)))
	 << std::endl
	 << cgicc::label("NTrigsSeen").set("for","NTrigsSeen") << std::endl
	 << cgicc::input().set("id","NTrigsSeen").set("name","NTrigsSeen")
                          .set("type","number").set("min","0").set("readonly")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.triggersSeen)))
	 << std::endl

	 << cgicc::br() << std::endl

	 << cgicc::div()   << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(),"Something went wrong displaying VFATS(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(),"Something went wrong displaying VFATS(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}


// HyperDAQ interface
void gem::supervisor::tbutils::ThresholdScan::webDefault(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{

  try {
    ////update the page refresh 
    //if (is_working_) {
    //  cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
    //  head.addHeader("Refresh","2");
    //}
    //else {
    //  cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
    //  head.addHeader("Refresh","Off");
    //}
    
    //generate the control buttons and display the ones that can be touched depending on the run mode
    if (!is_initialized_) {
      //have a menu for selecting the VFAT
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Initialize") << std::endl;

      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command")
	.set("title", "Initialize hardware acces.")
	.set("value", "Initialize") << std::endl;

      *out << cgicc::form() << std::endl;
    }
    
    else if (!is_configured_) {
      //this will allow the parameters to be set to the chip and scan routine
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Configure") << std::endl;
      
      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command")
	.set("title", "Configure threshold scan.")
	.set("value", "Configure") << std::endl;
      *out << cgicc::form()        << std::endl;
    }
    
    else if (!is_running_) {
      //hardware is initialized and configured, we can start the run
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Start") << std::endl;
      
      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command")
	.set("title", "Start threshold scan.")
	.set("value", "Start") << std::endl;
      *out << cgicc::form()    << std::endl;
    }
    
    else if (is_running_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Stop") << std::endl;
      
      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command")
	.set("title", "Stop threshold scan.")
	.set("value", "Stop") << std::endl;
      *out << cgicc::form()   << std::endl;
    }
    
    //always should have a halt command
    *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Halt") << std::endl;
    
    *out << cgicc::input().set("type", "submit")
      .set("name", "command")
      .set("title", "Halt threshold scan.")
      .set("value", "Halt") << std::endl;
    *out << cgicc::form() << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(),"Something went wrong displaying ThresholdScan control panel(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(),"Something went wrong displaying ThresholdScan control panel(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}


void gem::supervisor::tbutils::ThresholdScan::webInitialize(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  
  try {
    cgicc::Cgicc cgi(in);
    std::vector<cgicc::FormEntry> vfat2FormEntries = cgi.getElements();
    LOG4CPLUS_DEBUG(getApplicationLogger(), "debugging form entries");
    std::vector<cgicc::FormEntry>::const_iterator myiter = vfat2FormEntries.begin();
    
    //for (; myiter != vfat2FormEntries.end(); ++myiter ) {
    //  LOG4CPLUS_DEBUG(getApplicationLogger(), "form entry::" myiter->getName());
    //}

    
    std::string tmpDeviceName = "";
    cgicc::const_form_iterator name = cgi.getElement("VFATDevice");
    if (name != cgi.getElements().end())
      tmpDeviceName = name->getValue();
    
    //std::string tmpDeviceName = cgi["VFATDevice"]->getValue();
    LOG4CPLUS_INFO(getApplicationLogger(), "deviceName_::" << confParams_.bag.deviceName.toString());
    LOG4CPLUS_INFO(getApplicationLogger(), "setting deviceName_ to ::" << tmpDeviceName);
    confParams_.bag.deviceName = tmpDeviceName;
    LOG4CPLUS_INFO(getApplicationLogger(), "deviceName_::" << confParams_.bag.deviceName.toString());
    
    is_working_ = true;
    wl_->submit(initSig_);
    
    //change the status to initializing and make sure the page displays this information
  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  
  this->webDefault(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webConfigure(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  is_working_ = true;
  
  try {
    cgicc::Cgicc cgi(in);

    cgicc::const_form_iterator element = cgi.getElement("Latency");
    if (element != cgi.getElements().end())
      confParams_.bag.latency   = element->getIntegerValue();
    element = cgi.getElement("Latency");
    if (element != cgi.getElements().end())
      confParams_.bag.nTriggers = element->getIntegerValue();

    element = cgi.getElement("MinThreshold");
    if (element != cgi.getElements().end())
      confParams_.bag.minThresh = element->getIntegerValue();
    
    element = cgi.getElement("MaxThreshold");
    if (element != cgi.getElements().end())
      confParams_.bag.maxThresh = element->getIntegerValue();

    element = cgi.getElement("VStep");
    if (element != cgi.getElements().end())
      confParams_.bag.stepSize  = element->getIntegerValue();
        
    element = cgi.getElement("NTrigsStep");
    if (element != cgi.getElements().end())
      confParams_.bag.nTriggers  = element->getIntegerValue();
  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  
  wl_->submit(confSig_);
  
  //change the status to configuring and make sure the page displays this information
  this->webDefault(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webStart(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  is_working_ = true;
  wl_->submit(startSig_);

  //change the status to starting/running and make sure the page displays this information

  ////update the page refresh 
  //cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
  //head.addHeader("Refresh","2");

  this->webDefault(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webStop(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  is_working_ = true;
  wl_->submit(stopSig_);
  
  
  //change the status to halting and make sure the page displays this information
  this->webDefault(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webHalt(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  is_working_ = true;
  wl_->submit(haltSig_);
  
  //change the status to halting and make sure the page displays this information
  this->webDefault(in,out);
}


// State transitions
//is initialize different than halt? they come from different positions but put the software/hardware in the same state 'halted'
void gem::supervisor::tbutils::ThresholdScan::initializeAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  //Need to know which device to connnect to here...
  //dropdown list from the web interface?
  //deviceName_ = "CMS_hybrid_J44";
  //here the connection to the device should be made
  setLogLevelTo(uhal::Error());  // Minimize uHAL logging
  vfatDevice_ = new gem::hw::vfat::HwVFAT2(this, confParams_.bag.deviceName.toString());
  vfatDevice_->setAddressTableFileName("allregsnonfram.xml");
  vfatDevice_->setDeviceBaseNode("user_regs.vfats."+confParams_.bag.deviceName.toString());
  vfatDevice_->connectDevice();
  
  //read in default parameters from an xml file?
  //vfatDevice_->setRegisters(xmlFile);
  vfatDevice_->readVFAT2Counters();
  confParams_.bag.deviceChipID = vfatDevice_->getChipID();
  
  is_working_ = false;
  is_initialized_ = true;
}


void gem::supervisor::tbutils::ThresholdScan::configureAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  latency_   = confParams_.bag.latency;
  nTriggers_ = confParams_.bag.nTriggers;
  stepSize_  = confParams_.bag.stepSize;
  minThresh_ = confParams_.bag.minThresh;
  maxThresh_ = confParams_.bag.maxThresh;
  
  vfatDevice_->setLatency(latency_);
  
  vfatDevice_->setVThreshold1(0-minThresh_);
  confParams_.bag.deviceVT1 = vfatDevice_->getVThreshold1();
  vfatDevice_->setVThreshold2(0);
  
  is_working_ = false;
  is_configured_ = true;
}


void gem::supervisor::tbutils::ThresholdScan::startAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
  
  //start data aquisition, reset triggers, reset counters, flush buffers
  
  //start triggers

  //start scan routine
  
  //wl_->submit(runSig_);
  
  is_working_ = false;
  is_running_ = true;
}


void gem::supervisor::tbutils::ThresholdScan::stopAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  if (is_running_)
    vfatDevice_->setRunMode(0);
  
  //wl_->submit(stopSig_);
  
  is_working_ = false;
  is_running_ = false;
}


void gem::supervisor::tbutils::ThresholdScan::haltAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  vfatDevice_->setRunMode(0);
  //wl_->submit(haltSig_);
  is_working_ = false;
  is_configured_ = false;
  is_running_ = false;
}


void gem::supervisor::tbutils::ThresholdScan::noAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = false;
  vfatDevice_->setRunMode(0);
}

