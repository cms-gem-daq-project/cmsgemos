/**
 * class: GEMSupervisor
 * description: Supervisor application for GEM system
 *              structure borrowed from TCDS core, with nods to HCAL (hcalSupervisor)
 * author: J. Sturdy
 * date: 
 */

#include "gem/supervisor/GEMSupervisorWeb.h"
#include "gem/supervisor/GEMSupervisor.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::GEMSupervisor);

gem::supervisor::GEMSupervisor::GEMSupervisor(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub)
{

  //getApplicationInfoSpace()->fireItemAvailable("crateID", &m_crateID);
  //getApplicationInfoSpace()->fireItemAvailable("slot",    &m_slot);

  //xgi::framework::deferredbind(this, this, &GEMSupervisor::xgiDefault, "Default");

  DEBUG("Creating the GEMSupervisorWeb interface");
  p_gemWebInterface = new gem::supervisor::GEMSupervisorWeb(this);
  DEBUG("done");
  //p_gemMonitor      = new gem generic system monitor
  
  v_supervisedApps.clear();
  //where can we get some nice PNG images for our different applications?
  getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemsupervisor/images/supervisor/GEMSupervisor.png");
  init();
}

gem::supervisor::GEMSupervisor::~GEMSupervisor()
{
  // make sure to empty the v_supervisedApps  vector and free the pointers
  v_supervisedApps.clear();
}


// This is the callback used for handling xdata:Event objects
void gem::supervisor::GEMSupervisor::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("GEMSupervisor::actionPerformed() setDefaultValues" << 
          "Default configuration values have been loaded from xml profile");
    importConfigurationParameters();
    importMonitoringParameters();
    //p_gemMonitor->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::supervisor::GEMSupervisor::init()
{
  v_supervisedApps.clear();
  v_supervisedApps.reserve(0);
  DEBUG("init:: looping over " << p_appZone->getGroupNames().size() << " groups");
  std::set<xdaq::ApplicationDescriptor*> used;
  std::set<std::string> groups = p_appZone->getGroupNames();
  //for (std::set<std::string>::const_iterator i = groups.begin(); i != groups.end(); i++) {
  for (auto i =groups.begin(); i != groups.end(); ++i) {
    DEBUG("init::xDAQ group: " << *i
          << "getApplicationGroup() " << p_appZone->getApplicationGroup(*i)->getName());
    
    xdaq::ApplicationGroup* ag = p_appZone->getApplicationGroup(*i);
    std::set<xdaq::ApplicationDescriptor*> allApps = ag->getApplicationDescriptors();
    
    DEBUG("init::getApplicationDescriptors() " << allApps.size());
    //for (std::set<xdaq::ApplicationDescriptor*>::const_iterator j=allApps.begin(); j!=allApps.end(); j++) {
    for (auto j = allApps.begin(); j != allApps.end(); ++j) {
      DEBUG("init::xDAQ application descriptor " << *j
            << " " << (*j)->getClassName()
            << " we are " << p_appDescriptor);
    
      if (used.find(*j) != used.end()) continue; // no duplicates
      if ((*j) == p_appDescriptor ) continue; // don't fire the command into the GEMSupervisor again
      //maybe just write a function that populates some vectors
      //with the application classes that we want to supervise
      //avoids the problem of picking up all the xDAQ related processes
      //if (isGEMSupervised(*j))
      DEBUG("init::pushing " << (*j)->getClassName() << "(" << *j << ") to list of supervised applications");
      v_supervisedApps.push_back(*j);
      DEBUG("done");
    } // done iterating over applications in group
    DEBUG("init::done iterating over applications in group");
  } // done iterating over groups in zone
  DEBUG("init::done iterating over groups in zone");
};

//state transitions
void gem::supervisor::GEMSupervisor::initializeAction()
  throw (gem::supervisor::exception::Exception)
{
  INFO("gem::supervisor::GEMSupervisor::initializeAction Initializing");
  
  //for (std::vector<xdaq::ApplicationDescriptor*>::iterator i=v_supervisedApps.begin(); i!=v_supervisedApps.end(); i++) {
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO(std::string("Initializing ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Initialize",p_appContext,p_appDescriptor,*i);
  }
}

void gem::supervisor::GEMSupervisor::configureAction()
  throw (gem::supervisor::exception::Exception)
{
}

void gem::supervisor::GEMSupervisor::startAction()
  throw (gem::supervisor::exception::Exception)
{
}

void gem::supervisor::GEMSupervisor::pauseAction()
  throw (gem::supervisor::exception::Exception)
{
}

void gem::supervisor::GEMSupervisor::resumeAction()
  throw (gem::supervisor::exception::Exception)
{
}

void gem::supervisor::GEMSupervisor::stopAction()
  throw (gem::supervisor::exception::Exception)
{
}

void gem::supervisor::GEMSupervisor::haltAction()
  throw (gem::supervisor::exception::Exception)
{
}

void gem::supervisor::GEMSupervisor::resetAction()
  throw (gem::supervisor::exception::Exception)
{
}

/*
  void gem::supervisor::GEMSupervisor::noAction()
  throw (gem::supervisor::exception::Exception)
  {
  }
*/

void gem::supervisor::GEMSupervisor::failAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}

void gem::supervisor::GEMSupervisor::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}
