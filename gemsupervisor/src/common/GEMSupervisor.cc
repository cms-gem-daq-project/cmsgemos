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

  //xgi::framework::deferredbind(this, this, &GEMSupervisor::xgiDefault, "Default");

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

//void gem::supervisor::GEMSupervisor::xgiDefault(xgi::Input* in, xgi::Output* out) {
//  dynamic_cast<gem::supervisor::GEMSupervisorWeb*>(gemWebInterfaceP_)->controlPanel(in,out);
//}

void gem::supervisor::GEMSupervisor::init() {};

//state transitions
void gem::supervisor::GEMSupervisor::initializeAction() {}
void gem::supervisor::GEMSupervisor::enableAction(    ) {}
void gem::supervisor::GEMSupervisor::configureAction( ) {}
void gem::supervisor::GEMSupervisor::startAction(     ) {}
void gem::supervisor::GEMSupervisor::pauseAction(     ) {}
void gem::supervisor::GEMSupervisor::resumeAction(    ) {}
void gem::supervisor::GEMSupervisor::stopAction(      ) {}
void gem::supervisor::GEMSupervisor::haltAction(      ) {}
void gem::supervisor::GEMSupervisor::noAction(        ) {}

void gem::supervisor::GEMSupervisor::failAction(      toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}

void gem::supervisor::GEMSupervisor::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}

