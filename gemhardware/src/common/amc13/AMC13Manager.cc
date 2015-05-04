/**
 * class: AMC13Manager
 * description: Manager application for AMC13 cards
 *              structure borrowed from TCDS core, with nods to HCAL (DTCManager)
 * author: J. Sturdy
 * date: 
 */

#include "gem/hw/amc13/AMC13ManagerWeb.h"
#include "gem/hw/amc13/AMC13Manager.h"

#include "gem/hw/amc13/exception/Exception.h"

#include "amc13/AMC13.hh"
#include "amc13/Status.hh"

XDAQ_INSTANTIATOR_IMPL(gem::hw::amc13::AMC13Manager);

gem::hw::amc13::AMC13Manager::AMC13Manager(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub),
  deviceLock_(toolbox::BSem::FULL, true),
  amc13Device_(0)
{
  m_crateID = -1;
  m_slot = 13;
  
  getApplicationInfoSpace()->fireItemAvailable("crateID", &m_crateID);
  getApplicationInfoSpace()->fireItemAvailable("slot",    &m_slot);

  //initialize the AMC13Manager application objects
  LOG4CPLUS_DEBUG(getApplicationLogger(), "connecting to the AMC13ManagerWeb interface");
  gemWebInterfaceP_ = new gem::hw::amc13::AMC13ManagerWeb(this);
  LOG4CPLUS_DEBUG(getApplicationLogger(), "done");
  //gemMonitorP_      = new gem::hw::amc13::AMC13HwMonitor();

  LOG4CPLUS_DEBUG(getApplicationLogger(), "executing preInit for AMC13Manager");
  preInit();
  LOG4CPLUS_DEBUG(getApplicationLogger(), "done");
  getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemhardware/images/amc13/AMC13Manager.png");
}

gem::hw::amc13::AMC13Manager::~AMC13Manager() {
  
}

void gem::hw::amc13::AMC13Manager::preInit()
  throw (gem::base::exception::Exception)
{
  std::string addressBase  = "${AMC13_ADDRESS_TABLE_PATH}/";
  std::string connection   = "${BUILD_HOME}/gemdaq-testing/gemhardware/xml/amc13/connectionSN170_ch.xml";
  std::string friendlyname = "gem.shelf01.amc13.";
  try {
    gem::utils::LockGuard<gem::utils::Lock> guardedLock(deviceLock_);
    amc13Device_ = new ::amc13::AMC13(connection,friendlyname+"T1",friendlyname+"T2");
  } catch (uhal::exception::exception & e) {
    LOG4CPLUS_ERROR(getApplicationLogger(), std::string("AMC13::AMC13() failed, caught uhal::exception:") + e.what() );
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create class: ")+e.what());
  } catch (std::exception& e) {
    LOG4CPLUS_ERROR(getApplicationLogger(), std::string("AMC13::AMC13() failed, caught std::exception:") + e.what() );
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create class: ")+e.what());
  } catch (...) {
    LOG4CPLUS_ERROR(getApplicationLogger(), std::string("AMC13::AMC13() failed, caught ...") );
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create AMC13 connection"));
  }

  LOG4CPLUS_DEBUG(getApplicationLogger(),"finished with AMC13::AMC13()");

  try {
    // just T2-related work here.
    gem::utils::LockGuard<gem::utils::Lock> guardedLock(deviceLock_);
    amc13Device_->reset(::amc13::AMC13::T2);
    
    amc13Device_->enableAllTTC(); // this is convenient for debugging, works with _some_ firmwares
    
  } catch (uhal::exception::exception & e) {
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Problem during preinit : ")+e.what());
  } catch (std::exception& e) {
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Problem during preinit : ")+e.what());
  }
  LOG4CPLUS_DEBUG(getApplicationLogger(),"finished with AMC13Manager::preInit()");
}

// This is the callback used for handling xdata:Event objects
void gem::hw::amc13::AMC13Manager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    LOG4CPLUS_DEBUG(getApplicationLogger(), "AMC13Manager::actionPerformed() setDefaultValues" << 
		    "Default configuration values have been loaded from xml profile");
    //gemMonitorP_->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::amc13::AMC13Manager::init()
  throw (gem::base::exception::Exception)
{
  gem::base::GEMFSMApplication::init();

  LOG4CPLUS_DEBUG(getApplicationLogger(),"Entering gem::hw::amc13::AMC13Manager::init()");
  if (amc13Device_==0) return;

  //have to set up the initialization of the AMC13 for the desired running situation
  //possibilities are TTC/TCDS mode, DAQ link, local trigger scheme
}

void gem::hw::amc13::AMC13Manager::enable()
  throw (gem::base::exception::Exception) {
  LOG4CPLUS_DEBUG(getApplicationLogger(),"Entering gem::hw::amc13::AMC13Manager::enable()");
  //gem::base::GEMFSMApplication::enable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(deviceLock_);
  amc13Device_->startRun();
}

void gem::hw::amc13::AMC13Manager::disable()
  throw (gem::base::exception::Exception) {
  LOG4CPLUS_DEBUG(getApplicationLogger(),"Entering gem::hw::amc13::AMC13Manager::disable()");
  //gem::base::GEMFSMApplication::disable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(deviceLock_);
  amc13Device_->endRun();
}

::amc13::Status* gem::hw::amc13::AMC13Manager::getHTMLStatus() const {
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(deviceLock_);
  return amc13Device_->getStatus(); 
}

/*
// SOAP interface
xoap::MessageReference gem::hw::amc13::AMC13Manager::onEnable(     xoap::MessageReference message)
throw (xoap::exception::Exception) {};
xoap::MessageReference gem::hw::amc13::AMC13Manager::onConfigure(  xoap::MessageReference message)
throw (xoap::exception::Exception) {};
xoap::MessageReference gem::hw::amc13::AMC13Manager::onStart(      xoap::MessageReference message)
throw (xoap::exception::Exception) {};
xoap::MessageReference gem::hw::amc13::AMC13Manager::onPause(      xoap::MessageReference message)
throw (xoap::exception::Exception) {};
xoap::MessageReference gem::hw::amc13::AMC13Manager::onResume(     xoap::MessageReference message)
throw (xoap::exception::Exception) {};
xoap::MessageReference gem::hw::amc13::AMC13Manager::onStop(       xoap::MessageReference message)
throw (xoap::exception::Exception) {};
xoap::MessageReference gem::hw::amc13::AMC13Manager::onHalt(       xoap::MessageReference message)
throw (xoap::exception::Exception) {};
xoap::MessageReference gem::hw::amc13::AMC13Manager::onReset(      xoap::MessageReference message)
throw (xoap::exception::Exception) {};
xoap::MessageReference gem::hw::amc13::AMC13Manager::onRunSequence(xoap::MessageReference message)
throw (xoap::exception::Exception) {};
xoap::MessageReference gem::hw::amc13::AMC13Manager::reset(        xoap::MessageReference message)
throw (xoap::exception::Exception) {};
xoap::MessageReference gem::hw::amc13::AMC13Manager::fireEvent(    xoap::MessageReference message)
throw (xoap::exception::Exception) {};
xoap::MessageReference gem::hw::amc13::AMC13Manager::createReply(  xoap::MessageReference message)
throw (xoap::exception::Exception) {};

// work loop call-back functions
bool gem::hw::amc13::AMC13Manager::initializeAction(toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::enableAction(    toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::configureAction( toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::startAction(     toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::pauseAction(     toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::resumeAction(    toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::stopAction(      toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::haltAction(      toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::resetAction(     toolbox::task::WorkLoop *wl) {};
//bool gem::hw::amc13::AMC13Manager::noAction(        toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::failAction(      toolbox::task::WorkLoop *wl) {};

//bool gem::hw::amc13::AMC13Manager::calibrationAction(toolbox::task::WorkLoop *wl) {};
//bool gem::hw::amc13::AMC13Manager::calibrationSequencer(toolbox::task::WorkLoop *wl) {};
*/
	
//state transitions
void gem::hw::amc13::AMC13Manager::initializeAction(toolbox::Event::Reference e) {};
void gem::hw::amc13::AMC13Manager::enableAction(    toolbox::Event::Reference e) {};
void gem::hw::amc13::AMC13Manager::configureAction( toolbox::Event::Reference e) {};
void gem::hw::amc13::AMC13Manager::startAction(     toolbox::Event::Reference e) {};
void gem::hw::amc13::AMC13Manager::pauseAction(     toolbox::Event::Reference e) {};
void gem::hw::amc13::AMC13Manager::resumeAction(    toolbox::Event::Reference e) {};
void gem::hw::amc13::AMC13Manager::stopAction(      toolbox::Event::Reference e) {};
void gem::hw::amc13::AMC13Manager::haltAction(      toolbox::Event::Reference e) {};
void gem::hw::amc13::AMC13Manager::noAction(        toolbox::Event::Reference e) {}; 
void gem::hw::amc13::AMC13Manager::failAction(      toolbox::Event::Reference e) {}; 
	
// void gem::hw::amc13::AMC13Manager::resetAction()//toolbox::Event::Reference e)
//   throw (toolbox::fsm::exception::Exception) {};
	
// void gem::hw::amc13::AMC13Manager::stateChanged(    toolbox::fsm::FiniteStateMachine &fsm)
//   throw (toolbox::fsm::exception::Exception) {};
// void gem::hw::amc13::AMC13Manager::transitionFailed(toolbox::Event::Reference event)
//   throw (toolbox::fsm::exception::Exception) {};

// void gem::hw::amc13::AMC13Manager::fireEvent(std::string event)
//   throw (toolbox::fsm::exception::Exception) {};
	
// xoap::MessageReference gem::hw::amc13::AMC13Manager::changeState(xoap::MessageReference msg) {};
