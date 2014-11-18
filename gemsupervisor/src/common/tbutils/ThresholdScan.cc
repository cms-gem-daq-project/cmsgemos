#include "gem/supervisor/tbutils/ThresholdScan.h"
#include "gem/hw/vfat/HwVFAT2.h"

#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "cgicc/HTTPRedirectHeader.h"

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
  hw_semaphore_(toolbox::BSem::FULL),
  initSig_ (0),
  confSig_ (0),
  startSig_(0),
  stopSig_ (0),
  haltSig_ (0),
  resetSig_(0),
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
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webReset,       "Reset"     );
  
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onInitialize,  "Initialize",  XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onConfigure,   "Configure",   XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onStart,       "Start",       XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onStop,        "Stop",        XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onHalt,        "Halt",        XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onReset,       "Reset",       XDAQ_NS_URI);
  
  initSig_  = toolbox::task::bind(this, &ThresholdScan::initialize, "initialize");
  confSig_  = toolbox::task::bind(this, &ThresholdScan::configure,  "configure" );
  startSig_ = toolbox::task::bind(this, &ThresholdScan::start,      "start"     );
  stopSig_  = toolbox::task::bind(this, &ThresholdScan::stop,       "stop"      );
  haltSig_  = toolbox::task::bind(this, &ThresholdScan::halt,       "halt"      );
  resetSig_ = toolbox::task::bind(this, &ThresholdScan::reset,      "reset"     );
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
  fsmP_->addStateTransition('C', 'I', "Reset",      this, &gem::supervisor::tbutils::ThresholdScan::resetAction);
  fsmP_->addStateTransition('H', 'I', "Reset",      this, &gem::supervisor::tbutils::ThresholdScan::resetAction);

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
  fireEvent("Configure");
  return false; //do once?
}

bool gem::supervisor::tbutils::ThresholdScan::start(toolbox::task::WorkLoop* wl)
{
  fireEvent("Start");
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

bool gem::supervisor::tbutils::ThresholdScan::reset(toolbox::task::WorkLoop* wl)
{
  fireEvent("Reset");
  return false; //do once?
}

bool gem::supervisor::tbutils::ThresholdScan::run(toolbox::task::WorkLoop* wl)
{

  wl_semaphore_.take();
  if (!is_running_) {
    hw_semaphore_.take();
    vfatDevice_->setRunMode(1);
    hw_semaphore_.give();
  }
  
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
      wl_semaphore_.give();
      return true;
    }
    else {
      wl_semaphore_.give();
      wl_->submit(readSig_);
      wl_semaphore_.take();
      hw_semaphore_.take();
      //vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      vfatDevice_->setVThreshold1(confParams_.bag.deviceVT1 + confParams_.bag.stepSize);
      confParams_.bag.deviceVT1    = vfatDevice_->getVThreshold1();
      confParams_.bag.triggersSeen = 0;
      hw_semaphore_.give();
      wl_semaphore_.give();
      return true;
    }
  }
  else {
    wl_semaphore_.give();
    wl_->submit(readSig_);
    return false;
  }
}

//might be better done not as a workloop?
bool gem::supervisor::tbutils::ThresholdScan::readFIFO(toolbox::task::WorkLoop* wl)
{
  wl_semaphore_.take();
  hw_semaphore_.take();

  //maybe not even necessary?
  vfatDevice_->setRunMode(0);
  sleep(5);
  //read the fifo (x3 times fifo depth), add headers, write to disk, save disk
  boost::format linkForm("glib_links.link%d");
  //should all links have the same fifo depth? if not is this an error?

  uint32_t fifoDepth[3];
  //set proper base address
  //vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
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
  hw_semaphore_.give();
  wl_semaphore_.give();
  return false;
}

// SOAP interface
xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onInitialize(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(initSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onConfigure(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(confSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onStart(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(startSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onStop(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(stopSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onHalt(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(haltSig_);

  return message;
}

xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onReset(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(resetSig_);

  return message;
}

void gem::supervisor::tbutils::ThresholdScan::selectVFAT(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    std::string isDisabled = "";
    if (is_running_ || is_configured_)
      isDisabled = "disabled";
    
    *out << cgicc::span()   << std::endl
	 << cgicc::table()<< std::endl
	 << cgicc::tr()   << std::endl
	 << cgicc::td() << "Selected VFAT:" << cgicc::td() << std::endl
	 << cgicc::td() << "ChipID:"        << cgicc::td() << std::endl
	 << cgicc::tr() << std::endl

	 << cgicc::tr() << std::endl
	 << cgicc::td() << std::endl
	 << cgicc::select().set("id","VFATDevice").set("name","VFATDevice")     << std::endl
	 << ((confParams_.bag.deviceName.toString().compare("VFAT8")) == 0 ?
	     (cgicc::option("VFAT8").set(isDisabled).set("value","VFAT8").set("selected")) :
	     (cgicc::option("VFAT8").set(isDisabled).set("value","VFAT8"))) << std::endl
	 << ((confParams_.bag.deviceName.toString().compare("VFAT26")) == 0 ?
	     (cgicc::option("VFAT9").set(isDisabled).set("value","VFAT9").set("selected")) :
	     (cgicc::option("VFAT9").set(isDisabled).set("value","VFAT9"))) << std::endl
	 << ((confParams_.bag.deviceName.toString().compare("VFAT10")) == 0 ?
	     (cgicc::option("VFAT10").set(isDisabled).set("value","VFAT10").set("selected")) :
	     (cgicc::option("VFAT10").set(isDisabled).set("value","VFAT10"))) << std::endl
	 << ((confParams_.bag.deviceName.toString().compare("VFAT11")) == 0 ?
	     (cgicc::option("VFAT11").set(isDisabled).set("value","VFAT11").set("selected")) :
	     (cgicc::option("VFAT11").set(isDisabled).set("value","VFAT11"))) << std::endl
	 << ((confParams_.bag.deviceName.toString().compare("VFAT12")) == 0 ?
	     (cgicc::option("VFAT12").set(isDisabled).set("value","VFAT12").set("selected")) :
	     (cgicc::option("VFAT12").set(isDisabled).set("value","VFAT12"))) << std::endl
	 << ((confParams_.bag.deviceName.toString().compare("VFAT13")) == 0 ?
	     (cgicc::option("VFAT13").set(isDisabled).set("value","VFAT13").set("selected")) :
	     (cgicc::option("VFAT13").set(isDisabled).set("value","VFAT13"))) << std::endl
      /**
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
      **/
	 << cgicc::select()<< std::endl
	 << cgicc::td() << std::endl
      
	 << cgicc::td() 
	 << cgicc::input().set("type","text").set("id","ChipID")
                          .set("name","ChipID").set("readonly")
                          .set("value",boost::str(boost::format("0x%04x")%(confParams_.bag.deviceChipID)))
	 << std::endl
	 << cgicc::td()    << std::endl
	 << cgicc::tr()    << std::endl
	 << cgicc::table() << std::endl
	 << cgicc::span()  << std::endl;
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
    *out << cgicc::span()   << std::endl
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

	 << cgicc::label("VT2").set("for","VT2") << std::endl
	 << cgicc::input().set("id","VT2").set("name","VT2").set("readonly")
                          .set("value",boost::str(boost::format("%d")%static_cast<unsigned>(confParams_.bag.deviceVT2)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("NTrigsStep").set("for","NTrigsStep") << std::endl
	 << cgicc::input().set("id","NTrigsStep").set("name","NTrigsStep")
                          .set("type","number").set("min","0")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.nTriggers)))
	 << cgicc::br() << std::endl
	 << cgicc::label("NTrigsSeen").set("for","NTrigsSeen") << std::endl
	 << cgicc::input().set("id","NTrigsSeen").set("name","NTrigsSeen")
                          .set("type","number").set("min","0").set("readonly")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.triggersSeen)))
	 << cgicc::br() << std::endl

	 << cgicc::span()   << std::endl;
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

void gem::supervisor::tbutils::ThresholdScan::showCounterLayout(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    if (is_initialized_ && vfatDevice_) {
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS");
      //*out << cgicc::div().set("class","xdaq-tab").set("title","Counters")   << std::endl
      *out << "<table class=\"xdaq-table\">" << std::endl
	   << cgicc::caption("Counters")     << std::endl
	   << cgicc::thead() << std::endl
	   << cgicc::tr()    << std::endl //open
	   << cgicc::th()    << "L1A"      << cgicc::th() << std::endl
	   << cgicc::th()    << "CalPulse" << cgicc::th() << std::endl
	   << cgicc::th()    << "Other"    << cgicc::th() << std::endl
	   << cgicc::tr()    << std::endl //close
	   << cgicc::thead() << std::endl 

	   << cgicc::tbody() << std::endl;

      *out << "<tr><td>" << std::endl
	   << "<table class=\"xdaq-table\">" << std::endl
	   << cgicc::thead() << std::endl
	   << "<tr>" << std::endl
	   << cgicc::th()    << "Source" << cgicc::th() << std::endl
	   << cgicc::th()    << "Value"  << cgicc::th() << std::endl
	   << "</tr>" << std::endl //close
	   << cgicc::thead() << std::endl //close
      
	   << "<tbody>" << std::endl
	   << "<tr>" << std::endl
	   << cgicc::td()    << "External"    << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("L1A.External") << cgicc::td() << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Internal"    << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("L1A.Internal") << cgicc::td() << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Delayed"     << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("L1A.Delayed" ) << cgicc::td() << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Total"       << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("L1A.Total"   ) << cgicc::td() << std::endl
	   << "</tr>" << std::endl

	   << "</tbody>" << std::endl
	   << "</table>"     << std::endl
	   << "</td>" << std::endl;

      *out << "<td>" << std::endl
	   << "<table class=\"xdaq-table\">" << std::endl
	   << cgicc::thead() << std::endl
	   << "<tr>" << std::endl
	   << cgicc::th()    << "Source" << cgicc::th() << std::endl
	   << cgicc::th()    << "Value"  << cgicc::th() << std::endl
	   << "</tr>" << std::endl
	   << cgicc::thead() << std::endl

	   << "<tbody>" << std::endl
	   << "<tr>" << std::endl
	   << cgicc::td()    << "External"  << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("CalPulse.External") << cgicc::td() << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Internal"  << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("CalPulse.Internal") << cgicc::td() << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Total"     << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("CalPulse.Total"   ) << cgicc::td() << std::endl
	   << "</tr>" << std::endl
	   << "</tbody>" << std::endl
	   << "</table>"     << std::endl
	   << "</td>" << std::endl;
    
      *out << "<td>" << std::endl
	   << "<table class=\"xdaq-table\">" << std::endl
	   << cgicc::thead() << std::endl
	   << "<tr>" << std::endl
	   << cgicc::th()    << "Source" << cgicc::th() << std::endl
	   << cgicc::th()    << "Value"  << cgicc::th() << std::endl
	   << "</tr>" << std::endl
	   << cgicc::thead() << std::endl

	   << "<tbody>" << std::endl
	   << "<tr>" << std::endl
	   << cgicc::td()    << "Resync"    << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("Resync" ) << cgicc::td() << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "BC0"       << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("BC0"    ) << cgicc::td() << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "BXCount"   << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("BXCount") << cgicc::td() << std::endl
	   << "</tr>" << std::endl
	   << "</tbody>" << std::endl
	   << "</table>"     << std::endl
	   << "</td></tr>" << std::endl
	   << "</table>"     << std::endl;
      //<< cgicc::div()   << std::endl;
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying showCounterLayout(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying showCounterLayout(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  hw_semaphore_.take();
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  hw_semaphore_.give();
} //end showCounterLayout


void gem::supervisor::tbutils::ThresholdScan::redirect(xgi::Input *in, xgi::Output* out) {
  //change the status to halting and make sure the page displays this information
  std::string redURL = "/" + getApplicationDescriptor()->getURN() + "/Default";
  //cgicc::Cgicc cgi_in(in);
  //cgicc::Cgicc cgi_out(out);
  //cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
  //cgicc::HTTPRedirectHeader &head = out->getHTTPRedirectHeader();
  //head.addHeader("<meta http-equiv=\"refresh\" content=\"0;" + redURL + "\">");
  *out << "<meta http-equiv=\"refresh\" content=\"0;" << redURL << "\">" << std::endl;
  this->webDefault(in,out);
}

// HyperDAQ interface
void gem::supervisor::tbutils::ThresholdScan::webDefault(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{

  try {
    ////update the page refresh 
    if (!is_working_) {
    }
    else {
      cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
      head.addHeader("Refresh","2");
    }
    
    //generate the control buttons and display the ones that can be touched depending on the run mode
    *out << "<div class=\"xdaq-tab-wrapper\">"            << std::endl;
    *out << "<div class=\"xdaq-tab\" title=\"Control\">"  << std::endl;

    if (!is_initialized_) {
      //have a menu for selecting the VFAT
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Initialize") << std::endl;

      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Initialize hardware acces.")
	.set("value", "Initialize") << std::endl;

      *out << cgicc::form() << std::endl;
    }
    
    else if (!is_configured_) {
      //this will allow the parameters to be set to the chip and scan routine
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Configure") << std::endl;
      
      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Configure threshold scan.")
	.set("value", "Configure") << std::endl;
      *out << cgicc::form()        << std::endl;
    }
    
    else if (!is_running_) {
      //hardware is initialized and configured, we can start the run
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Start") << std::endl;
      
      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Start threshold scan.")
	.set("value", "Start") << std::endl;
      *out << cgicc::form()    << std::endl;
    }
    
    else if (is_running_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Stop") << std::endl;
      
      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Stop threshold scan.")
	.set("value", "Stop") << std::endl;
      *out << cgicc::form()   << std::endl;
    }
    
    *out << cgicc::comment() << "end the main commands, now putting the halt/reset commands" << cgicc::comment() << cgicc::br() << std::endl;
    *out << cgicc::span()  << std::endl
	 << cgicc::table() << std::endl
	 << cgicc::tr()    << std::endl
	 << cgicc::td()    << std::endl;
      
    //always should have a halt command
    *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Halt") << std::endl;
    
    *out << cgicc::input().set("type", "submit")
      .set("name", "command").set("title", "Halt threshold scan.")
      .set("value", "Halt") << std::endl;
    *out << cgicc::form() << std::endl
	 << cgicc::td()   << std::endl;
    
    *out << cgicc::td()    << std::endl;
    if (!is_running_) {
      //comand that will take the system to initial and allow to change the hw device
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Reset") << std::endl;
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Reset device.")
	.set("value", "Reset") << std::endl;
      *out << cgicc::form() << std::endl;
    }
    *out << cgicc::td()    << std::endl
	 << cgicc::tr()    << std::endl
	 << cgicc::table() << cgicc::br() << std::endl
	 << cgicc::span()  << std::endl;

    *out << "</div>" << std::endl;
    
    *out << "<div class=\"xdaq-tab\" title=\"Counters\">"  << std::endl;
    if (is_initialized_)
      showCounterLayout(out);
    *out << "</div>" << std::endl;

    *out << cgicc::br() << cgicc::br() << std::endl;
    
    //*out << "<div class=\"xdaq-tab\" title=\"Status\">"  << std::endl
    //*out << cgicc::div().set("class","xdaq-tab").set("title","Status")   << std::endl
    *out << cgicc::table() << std::endl
	 << cgicc::tr()    << std::endl
	 << cgicc::td() << "Status:"   << cgicc::td()
	 << cgicc::td() << "Value:"    << cgicc::td()
	 << cgicc::tr() << std::endl

	 << cgicc::tr() << std::endl
	 << cgicc::td() << "is_working_" << cgicc::td()
	 << cgicc::td() << is_working_   << cgicc::td()
	 << cgicc::tr()   << std::endl

	 << cgicc::tr() << std::endl
	 << cgicc::td() << "is_initialized_" << cgicc::td()
	 << cgicc::td() << is_initialized_   << cgicc::td()
	 << cgicc::tr()       << std::endl

	 << cgicc::tr() << std::endl
	 << cgicc::td() << "is_configured_" << cgicc::td()
	 << cgicc::td() << is_configured_   << cgicc::td()
	 << cgicc::tr()      << std::endl

	 << cgicc::tr() << std::endl
	 << cgicc::td() << "is_running_" << cgicc::td()
	 << cgicc::td() << is_running_   << cgicc::td()
	 << cgicc::tr()   << std::endl

	 << cgicc::table() << cgicc::br() << std::endl
      //<< "</div>" << std::endl
	 << "</div>" << std::endl;

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
    
    //change the status to initializing and make sure the page displays this information
  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  
  wl_->submit(initSig_);

  redirect(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webConfigure(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {

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
  
  redirect(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webStart(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(startSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webStop(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(stopSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webHalt(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(haltSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webReset(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(resetSig_);
  
  redirect(in,out);
}


// State transitions
//is initialize different than halt? they come from different positions but put the software/hardware in the same state 'halted'
void gem::supervisor::tbutils::ThresholdScan::initializeAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  //Need to know which device to connnect to here...
  //dropdown list from the web interface?
  //deviceName_ = "CMS_hybrid_J44";
  //here the connection to the device should be made
  setLogLevelTo(uhal::Debug());  // Set uHAL logging level Debug (most) to Error (least)
  hw_semaphore_.take();
  vfatDevice_ = new gem::hw::vfat::HwVFAT2(this, confParams_.bag.deviceName.toString());
  
  //vfatDevice_->setAddressTableFileName("allregsnonfram.xml");
  //vfatDevice_->setDeviceBaseNode("user_regs.vfats."+confParams_.bag.deviceName.toString());
  vfatDevice_->setAddressTableFileName("testbeam_registers.xml");
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  //sleep(1);
  vfatDevice_->connectDevice();
  
  //read in default parameters from an xml file?
  //vfatDevice_->setRegisters(xmlFile);
  vfatDevice_->readVFAT2Counters();
  confParams_.bag.deviceChipID = vfatDevice_->getChipID();
  is_initialized_ = true;
  hw_semaphore_.give();

  //sleep(5);
  is_working_     = false;
}


void gem::supervisor::tbutils::ThresholdScan::configureAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  latency_   = confParams_.bag.latency;
  nTriggers_ = confParams_.bag.nTriggers;
  stepSize_  = confParams_.bag.stepSize;
  minThresh_ = confParams_.bag.minThresh;
  maxThresh_ = confParams_.bag.maxThresh;
  
  hw_semaphore_.take();
  //vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());

  vfatDevice_->setLatency(latency_);
  
  vfatDevice_->setVThreshold1(0-minThresh_);
  confParams_.bag.deviceVT1 = vfatDevice_->getVThreshold1();
  vfatDevice_->setVThreshold2(0);
  is_configured_ = true;
  hw_semaphore_.give();
  
  is_working_    = false;
}


void gem::supervisor::tbutils::ThresholdScan::startAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
  
  is_working_ = true;

  is_running_ = true;
  //start data aquisition, reset triggers, reset counters, flush buffers
 
  //start triggers

  //start scan routine
  
  //wl_->submit(runSig_);
  
  is_working_ = false;
}


void gem::supervisor::tbutils::ThresholdScan::stopAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  if (is_running_) {
    hw_semaphore_.take();
    vfatDevice_->setRunMode(0);
    hw_semaphore_.give();
    is_running_ = false;
  }
  //wl_->submit(stopSig_);
  
  is_working_ = false;
}


void gem::supervisor::tbutils::ThresholdScan::haltAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;

  is_configured_ = false;
  is_running_    = false;

  hw_semaphore_.take();
  vfatDevice_->setRunMode(0);
  hw_semaphore_.give();
  
  //wl_->submit(haltSig_);
  
  //sleep(5);
  is_working_    = false;
}


void gem::supervisor::tbutils::ThresholdScan::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;

  is_initialized_ = false;
  is_configured_  = false;
  is_running_     = false;

  hw_semaphore_.take();
  vfatDevice_->setRunMode(0);

  if (vfatDevice_->isGEMHwDeviceConnected())
    vfatDevice_->releaseDevice();
  
  if (vfatDevice_)
    delete vfatDevice_;
  
  vfatDevice_ = 0;
  sleep(2);
  hw_semaphore_.give();

  confParams_.bag.latency   = 128U;
  confParams_.bag.nTriggers = 2500U;
  confParams_.bag.minThresh = -25;
  confParams_.bag.maxThresh = 0;
  confParams_.bag.stepSize  = 1U;

  confParams_.bag.deviceName   = "";
  confParams_.bag.deviceChipID = 0x0;
  confParams_.bag.deviceVT1    = 0x0;
  confParams_.bag.deviceVT2    = 0x0;
  confParams_.bag.triggersSeen = 0;
  
  //wl_->submit(resetSig_);
  
  //sleep(5);
  is_working_     = false;
}


void gem::supervisor::tbutils::ThresholdScan::noAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = false;
  hw_semaphore_.take();
  vfatDevice_->setRunMode(0);
  hw_semaphore_.give();
}

