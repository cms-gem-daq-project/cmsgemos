/**
 * class: GEMSupervisor
 * description: Supervisor application for GEM system
 *              structure borrowed from TCDS core, with nods to HCAL (hcalSupervisor)
 * author: J. Sturdy
 * date:
 */

#include "gem/supervisor/GEMSupervisor.h"

#include "gem/supervisor/GEMSupervisorWeb.h"
#include "gem/supervisor/GEMSupervisorMonitor.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"

typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::GEMSupervisor);

gem::supervisor::GEMSupervisor::GEMSupervisor(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub),
  m_globalState(this->getApplicationContext(), this),
  m_reportToRCMS(false),
  m_gemRCMSNotifier(this->getApplicationLogger(),
                    this->getApplicationDescriptor(),
                    this->getApplicationContext())
{

  // xgi::framework::deferredbind(this, this, &GEMSupervisor::xgiDefault, "Default");

  DEBUG("Creating the GEMSupervisorWeb interface");
  p_gemMonitor      = new gem::supervisor::GEMSupervisorMonitor(this);
  // p_gemMonitor      = new gem::supervisor::GEMSupervisorMonitor(this->getApplicationLogger(),this);
  p_gemWebInterface = new gem::supervisor::GEMSupervisorWeb(this);
  DEBUG("done");
  //p_gemMonitor      = new gem generic system monitor

  v_supervisedApps.clear();
  // reset the GEMInfoSpaceToolBox object?
  // where can we get some nice PNG images for our different applications?
  // getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemsupervisor/images/supervisor/GEMSupervisor.png");
  init();

  // Find connection to RCMS.
  /*p_appInfoSpaceToolBox->createBag("rcmsStateListener", m_gemRCMSNotifier.getRcmsStateListenerParameter(),
    m_gemRCMSNotifier.getRcmsStateListenerParameter(),
    GEMUpdateType::PROCESS);*/
  p_appInfoSpace->fireItemAvailable("rcmsStateListener",
                                    m_gemRCMSNotifier.getRcmsStateListenerParameter());
  /*p_appInfoSpaceToolBox->createBool( "foundRcmsStateListener", m_gemRCMSNotifier.getFoundRcmsStateListenerParameter()->value_,
    m_gemRCMSNotifier.getFoundRcmsStateListenerParameter(),
    GEMUpdateType::PROCESS);*/
  p_appInfoSpace->fireItemAvailable("foundRcmsStateListener",
                                    m_gemRCMSNotifier.getFoundRcmsStateListenerParameter());
  m_gemRCMSNotifier.findRcmsStateListener();
  m_gemRCMSNotifier.subscribeToChangesInRcmsStateListener(p_appInfoSpace);

  p_appInfoSpaceToolBox->createString("RCMSStateListenerURL", m_rcmsStateListenerUrl.toString(),
                                      &m_rcmsStateListenerUrl,
                                      GEMUpdateType::PROCESS);

  p_appInfoSpace->addItemRetrieveListener("rcmsStateListener",      this);
  p_appInfoSpace->addItemRetrieveListener("foundRcmsStateListener", this);
  p_appInfoSpace->addItemRetrieveListener("RCMSStateListenerURL",   this);
  p_appInfoSpace->addItemChangedListener( "rcmsStateListener",      this);
  p_appInfoSpace->addItemChangedListener( "foundRcmsStateListener", this);
  p_appInfoSpace->addItemChangedListener( "RCMSStateListenerURL",   this);
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
    // p_gemMonitor->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::supervisor::GEMSupervisor::init()
{
  v_supervisedApps.clear();
  v_supervisedApps.reserve(0);

  m_globalState.clear();

  DEBUG("init:: looping over " << p_appZone->getGroupNames().size() << " groups");
  std::set<xdaq::ApplicationDescriptor*> used;
  std::set<std::string> groups = p_appZone->getGroupNames();
  for (auto i =groups.begin(); i != groups.end(); ++i) {
    DEBUG("init::xDAQ group: " << *i
          << "getApplicationGroup() " << p_appZone->getApplicationGroup(*i)->getName());

    xdaq::ApplicationGroup* ag = p_appZone->getApplicationGroup(*i);
    std::set<xdaq::ApplicationDescriptor*> allApps = ag->getApplicationDescriptors();

    DEBUG("init::getApplicationDescriptors() " << allApps.size());
    for (auto j = allApps.begin(); j != allApps.end(); ++j) {
      DEBUG("init::xDAQ application descriptor " << *j
            << " " << (*j)->getClassName()
            << " we are " << p_appDescriptor);

      if (used.find(*j) != used.end())
        continue;  // no duplicates
      if ((*j) == p_appDescriptor )
        continue;  // don't fire the command into the GEMSupervisor again

      // maybe just write a function that populates some vectors
      // with the application classes that we want to supervise
      // avoids the problem of picking up all the xDAQ related processes
      // if (isGEMSupervised(*j))
      if (manageApplication((*j)->getClassName())) {
        INFO("GEMSupervisor::init::pushing " << (*j)->getClassName() << "(" << *j << ") to list of supervised applications");
        v_supervisedApps.push_back(*j);
        std::stringstream managedAppStateName;
        managedAppStateName << (*j)->getClassName() << ":lid:" << (*j)->getLocalId();
        std::stringstream managedAppStateURN;
        managedAppStateURN << (*j)->getURN();
        // have to figure out what we want here, with change to pointers
        p_appStateInfoSpaceToolBox->createString(managedAppStateName.str(), managedAppStateURN.str(), NULL);

        m_globalState.addApplication(*j);
      }
      DEBUG("done");
    }  // done iterating over applications in group
    DEBUG("init::done iterating over applications in group");
  }  // done iterating over groups in zone
  DEBUG("init::done iterating over groups in zone");

  DEBUG("init::starting the monitoring");

  // borrowed from hcalSupervisor
  if (m_reportToRCMS /*&& !m_hasDoneStandardInit*/) {
    m_gemRCMSNotifier.findRcmsStateListener();
    std::string classname = m_gemRCMSNotifier.getRcmsStateListenerParameter()->bag.classname.value_;
    int instance          = m_gemRCMSNotifier.getRcmsStateListenerParameter()->bag.instance.value_;
    m_rcmsStateListenerUrl = getApplicationContext()->getDefaultZone()->getApplicationDescriptor(classname, instance)->getContextDescriptor()->getURL();
    INFO("RCMSStateListener found with url: " << m_rcmsStateListenerUrl.toString());
  }

  // when to do this, have to make sure that all applications have been loaded...
  // p_gemMonitor->addInfoSpace("AppStateMonitoring", p_appStateInfoSpaceToolBox);
  dynamic_cast<gem::supervisor::GEMSupervisorMonitor*>(p_gemMonitor)->setupAppStateMonitoring();
  p_gemMonitor->startMonitoring();
  m_globalState.startTimer();
};

// state transitions
void gem::supervisor::GEMSupervisor::initializeAction()
  throw (gem::supervisor::exception::Exception)
{
  INFO("gem::supervisor::GEMSupervisor::initializeAction Initializing");

  // for (std::vector<xdaq::ApplicationDescriptor*>::iterator i=v_supervisedApps.begin(); i!=v_supervisedApps.end(); i++) {
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO(std::string("Initializing ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Initialize", p_appContext, p_appDescriptor, *i);
  }
}

void gem::supervisor::GEMSupervisor::configureAction()
  throw (gem::supervisor::exception::Exception)
{
  try {
    for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
      sendCfgType("testCfgType", (*i));
      sendRunType("testRunType", (*i));
      sendRunNumber(10254, (*i));
      INFO(std::string("Configuring ")+(*i)->getClassName());
      gem::utils::soap::GEMSOAPToolBox::sendCommand("Configure", p_appContext, p_appDescriptor, *i);
      if (((*i)->getClassName()).rfind("AMC13") != std::string::npos) {
        INFO(std::string("Seinding AMC13 Parameters to ")+(*i)->getClassName());
        gem::utils::soap::GEMSOAPToolBox::sendAMC13Config(p_appContext, p_appDescriptor, *i);
      }
    }
  } catch (gem::supervisor::exception::Exception& e) {
    ERROR("GEMSupervisor::configureAction " << e.what());
    throw e;
  } catch (xcept::Exception& e) {
    ERROR("GEMSupervisor::configureAction " << e.what());
    throw e;
  } catch (std::exception& e) {
    ERROR("GEMSupervisor::configureAction " << e.what());
    throw e;
  } catch (...) {
    ERROR("GEMSupervisor::configureAction ");
  }
  m_globalState.update();
}

void gem::supervisor::GEMSupervisor::startAction()
  throw (gem::supervisor::exception::Exception)
{
  updateRunNumber();

  try {
    for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
      sendRunNumber(m_runNumber, (*i));
      INFO(std::string("Starting ")+(*i)->getClassName());
      gem::utils::soap::GEMSOAPToolBox::sendCommand("Start", p_appContext, p_appDescriptor, *i);
    }
  } catch (gem::supervisor::exception::Exception& e) {
    ERROR("GEMSupervisor::startAction " << e.what());
    throw e;
  } catch (xcept::Exception& e) {
    ERROR("GEMSupervisor::startAction " << e.what());
    throw e;
  } catch (std::exception& e) {
    ERROR("GEMSupervisor::startAction " << e.what());
    throw e;
  } catch (...) {
    ERROR("GEMSupervisor::startAction ");
  }
  m_globalState.update();
}

void gem::supervisor::GEMSupervisor::pauseAction()
  throw (gem::supervisor::exception::Exception)
{
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO(std::string("Pausing ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Pause", p_appContext, p_appDescriptor, *i);
  }
  m_globalState.update();
}

void gem::supervisor::GEMSupervisor::resumeAction()
  throw (gem::supervisor::exception::Exception)
{
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO(std::string("Resuming ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Resume", p_appContext, p_appDescriptor, *i);
  }
  m_globalState.update();
}

void gem::supervisor::GEMSupervisor::stopAction()
  throw (gem::supervisor::exception::Exception)
{
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO(std::string("Stopping ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Stop", p_appContext, p_appDescriptor, *i);
  }
  m_globalState.update();
}

void gem::supervisor::GEMSupervisor::haltAction()
  throw (gem::supervisor::exception::Exception)
{
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO(std::string("Halting ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Halt", p_appContext, p_appDescriptor, *i);
  }
  m_globalState.update();
}

void gem::supervisor::GEMSupervisor::resetAction()
  throw (gem::supervisor::exception::Exception)
{
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO(std::string("Resetting ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Reset", p_appContext, p_appDescriptor, *i);
  }
  // gem::base::GEMFSMApplication::resetAction();
  m_globalState.update();
}

/*
  void gem::supervisor::GEMSupervisor::noAction()
  throw (gem::supervisor::exception::Exception)
  {
  }
*/

void gem::supervisor::GEMSupervisor::failAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  m_globalState.update();
}

void gem::supervisor::GEMSupervisor::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  m_globalState.update();
}


bool gem::supervisor::GEMSupervisor::isGEMApplication(const std::string& classname) const
{
  if (classname.find("gem") != std::string::npos)
    return true;  // handle all HCAL applications
  /*
  if (m_otherClassesToSupport.count(classname) != 0)
    return true;  // include from list
  */
  return false;
}

bool gem::supervisor::GEMSupervisor::manageApplication(const std::string& classname) const
{
  if (classname == "GEMSupervisor")
    return false;  // ignore ourself
  /*
  if (m_otherClassesToSupport.count(classname) != 0)
    return true;  // include from list
  */
  if (classname.find("gem") != std::string::npos)
    return true;  // handle all GEM applications
  if (classname.find("PeerTransport") != std::string::npos)
    return false;  // ignore all peer transports
  /*
  if ((classname == "TTCciControl" || classname == "ttc::TTCciControl") && m_handleTTCci.value_)
    return true;
  if ((classname == "LTCControl" || classname == "ttc::LTCControl") && m_handleTTCci.value_)
    return true;
  if (classname.find("tcds") != std::string::npos && m_handleTCDS.value_)
    return true;
  */
  return false;  // assume not ok.
}

void gem::supervisor::GEMSupervisor::globalStateChanged(toolbox::fsm::State before, toolbox::fsm::State after)
{
  DEBUG("GEMSupervisor::globalStateChanged(" << before << "," << after << ")");

  // Notify RCMS of a state change.
  m_stateName = GEMGlobalState::getStateName(after);
  try {
    if (m_reportToRCMS)
      m_gemRCMSNotifier.stateChanged(GEMGlobalState::getStateName(after), "GEM global state changed");
  } catch(xcept::Exception& err) {
    ERROR("GEMSupervisor::globalStateChanged::Failed to notify RCMS of state change: "
          << xcept::stdformat_exception_history(err));
    XCEPT_DECLARE_NESTED(gem::base::utils::exception::RCMSNotificationError, top,
                         "Failed to notify RCMS of state change.", err);
    notifyQualified("error", top);
  }
}

void gem::supervisor::GEMSupervisor::updateRunNumber()
{
  // should be able to find the run number from the run number service, or some other source
  m_runNumber = 10472;
}

void gem::supervisor::GEMSupervisor::sendCfgType(std::string const& cfgType, xdaq::ApplicationDescriptor* ad)
  throw (gem::supervisor::exception::Exception)
{
  INFO(std::string("GEMSupervisor::sendCfgType to ")+ad->getClassName());
  gem::utils::soap::GEMSOAPToolBox::sendApplicationParameter("CfgType", "xsd:string", m_cfgType.toString(),
                                                             p_appContext, p_appDescriptor, ad);
}

void gem::supervisor::GEMSupervisor::sendRunType(std::string const& runType, xdaq::ApplicationDescriptor* ad)
  throw (gem::supervisor::exception::Exception)
{
  INFO(std::string("GEMSupervisor::sendRunType to ")+ad->getClassName());
  gem::utils::soap::GEMSOAPToolBox::sendApplicationParameter("RunType", "xsd:string", m_runType.toString(),
                                                             p_appContext, p_appDescriptor, ad);
}

void gem::supervisor::GEMSupervisor::sendRunNumber(int64_t const& runNumber, xdaq::ApplicationDescriptor* ad)
  throw (gem::supervisor::exception::Exception)
{
  INFO(std::string("GEMSupervisor::sendRunNumber to ")+ad->getClassName());
  gem::utils::soap::GEMSOAPToolBox::sendApplicationParameter("RunNumber", "xsd:long",
                                                             m_runNumber.toString(),
                                                             p_appContext, p_appDescriptor, ad);
}
