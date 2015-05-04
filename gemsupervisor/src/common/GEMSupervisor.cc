/**
 * class: GEMSupervisor
 * description: Supervisor application for GEM system
 *              structure borrowed from TCDS core, with nods to HCAL (DTCManager)
 * author: J. Sturdy
 * date: 
 */

#include "gem/supervisor/GEMSupervisorWeb.h"
#include "gem/supervisor/GEMSupervisor.h"

#include "gem/supervisor/exception/Exception.h"

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::GEMSupervisor);

gem::supervisor::GEMSupervisor::GEMSupervisor(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub)
{

  //getApplicationInfoSpace()->fireItemAvailable("crateID", &m_crateID);
  //getApplicationInfoSpace()->fireItemAvailable("slot",    &m_slot);
  //initialize the AMC13Manager application objects

  LOG4CPLUS_DEBUG(getApplicationLogger(), "Creating the GEMSupervisorWeb interface");
  gemWebInterfaceP_ = new gem::supervisor::GEMSupervisorWeb(this);
  LOG4CPLUS_DEBUG(getApplicationLogger(), "done");
  //gemMonitorP_      = new gem generic system monitor
  
  //where can we get some nice PNG images for our different applications?
  getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemsupervisor/images/supervisor/GEMSupervisor.png");
}

gem::supervisor::GEMSupervisor::~GEMSupervisor() {
  
}


// This is the callback used for handling xdata:Event objects
void gem::supervisor::GEMSupervisor::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    LOG4CPLUS_DEBUG(getApplicationLogger(), "GEMSupervisor::actionPerformed() setDefaultValues" << 
		    "Default configuration values have been loaded from xml profile");
    importConfigurationParameters();
    importMonitoringParameters();
    //gemMonitorP_->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

//state transitions
void gem::supervisor::GEMSupervisor::initializeAction(toolbox::Event::Reference e) {};
void gem::supervisor::GEMSupervisor::enableAction(    toolbox::Event::Reference e) {};
void gem::supervisor::GEMSupervisor::configureAction( toolbox::Event::Reference e) {};
void gem::supervisor::GEMSupervisor::startAction(     toolbox::Event::Reference e) {};
void gem::supervisor::GEMSupervisor::pauseAction(     toolbox::Event::Reference e) {};
void gem::supervisor::GEMSupervisor::resumeAction(    toolbox::Event::Reference e) {};
void gem::supervisor::GEMSupervisor::stopAction(      toolbox::Event::Reference e) {};
void gem::supervisor::GEMSupervisor::haltAction(      toolbox::Event::Reference e) {};
void gem::supervisor::GEMSupervisor::noAction(        toolbox::Event::Reference e) {}; 
void gem::supervisor::GEMSupervisor::failAction(      toolbox::Event::Reference e) {}; 
	
void gem::supervisor::GEMSupervisor::resetAction()//toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {};
	
void gem::supervisor::GEMSupervisor::stateChanged(    toolbox::fsm::FiniteStateMachine &fsm)
  throw (toolbox::fsm::exception::Exception) {};
void gem::supervisor::GEMSupervisor::transitionFailed(toolbox::Event::Reference event)
  throw (toolbox::fsm::exception::Exception) {};

void gem::supervisor::GEMSupervisor::fireEvent(std::string event)
  throw (toolbox::fsm::exception::Exception) {};
	
xoap::MessageReference gem::supervisor::GEMSupervisor::changeState(xoap::MessageReference msg) {};

