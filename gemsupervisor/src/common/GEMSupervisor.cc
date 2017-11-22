/**
 * class: GEMSupervisor
 * description: Supervisor application for GEM system
 *              structure borrowed from TCDS core, with nods to HCAL (hcalSupervisor)
 * author: J. Sturdy
 * date:
 */

#include "gem/supervisor/GEMSupervisor.h"

#include <cstdlib>
#include <iomanip>

#include <map>
#include <set>
#include <vector>
#include <algorithm>

#include <boost/algorithm/string.hpp>

#include "gem/supervisor/GEMSupervisorWeb.h"
#include "gem/supervisor/GEMSupervisorMonitor.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"
#include "gem/utils/exception/Exception.h"

typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::GEMSupervisor);

gem::supervisor::GEMSupervisor::TCDSConfig::TCDSConfig() {
  handleTCDS     = true;
  iciHWConfig    = "";
  piHWConfig     = "";
  lpmHWConfig    = "";
  cpmHWConfig    = "";
  fedEnableMask  = "%";
  usePrimaryTCDS = true;
}

void gem::supervisor::GEMSupervisor::TCDSConfig::registerFields(xdata::Bag<gem::supervisor::GEMSupervisor::TCDSConfig>* bag) {
  bag->addField("HandleTCDS",           &handleTCDS    );
  bag->addField("ICIHardwareConfig",    &iciHWConfig   );
  bag->addField("PIHardwareConfig",     &piHWConfig    );
  bag->addField("LPMHardwareConfig",    &lpmHWConfig   );
  bag->addField("CPMHardwareConfig",    &cpmHWConfig   );
  bag->addField("FEDEnableMask",        &fedEnableMask );
  bag->addField("UsePrimaryTCDS",       &usePrimaryTCDS);
}

gem::supervisor::GEMSupervisor::GEMSupervisor(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub),
  m_deviceLock(toolbox::BSem::FULL, true),
  m_tcdsLock(toolbox::BSem::FULL,   true),
  m_globalState(this->getApplicationContext(), this),
  m_scanParameter(0),
  m_reportToRCMS(false),
  m_gemRCMSNotifier(this->getApplicationLogger(),
                    this->getApplicationDescriptor(),
                    this->getApplicationContext())
{

  xoap::bind(this, &gem::supervisor::GEMSupervisor::EndScanPoint, "EndScanPoint",  XDAQ_NS_URI);
  // xgi::framework::deferredbind(this, this, &GEMSupervisor::xgiDefault, "Default");

  DEBUG("Creating the GEMSupervisorWeb interface");
  p_gemMonitor      = new gem::supervisor::GEMSupervisorMonitor(this);
  // p_gemMonitor      = new gem::supervisor::GEMSupervisorMonitor(this->getApplicationLogger(),this);
  p_gemWebInterface = new gem::supervisor::GEMSupervisorWeb(this);
  DEBUG("done");
  //p_gemMonitor      = new gem generic system monitor

  p_appInfoSpace->fireItemAvailable("DatabaseInfo",&m_dbInfo);
  p_appInfoSpace->fireItemAvailable("TCDSConfig",  &m_tcdsConfig);

  p_appInfoSpace->addItemRetrieveListener("DatabaseInfo", this);
  p_appInfoSpace->addItemRetrieveListener("TCDSConfig",   this);

  p_appInfoSpace->addItemChangedListener("DatabaseInfo", this);
  p_appInfoSpace->addItemChangedListener("TCDSConfig",   this);

  p_appInfoSpaceToolBox->createBool("UseLocalDB",       m_useLocalDBInstance.value_, &m_useLocalDBInstance, GEMUpdateType::PROCESS);
  p_appInfoSpaceToolBox->createBool("UseLocalRunNumber",m_useLocalRunNumber.value_,  &m_useLocalRunNumber,  GEMUpdateType::PROCESS);
  p_appInfoSpaceToolBox->createBool("HandleTCDS",       m_handleTCDS.value_,         &m_handleTCDS,         GEMUpdateType::PROCESS);
  p_appInfoSpaceToolBox->createBool("UseLocalReadout",  m_useLocalReadout.value_,    &m_useLocalReadout,    GEMUpdateType::PROCESS);
  p_appInfoSpaceToolBox->createBool("UseFedKitReadout", m_useFedKitReadout.value_,   &m_useFedKitReadout,   GEMUpdateType::PROCESS);
  // Find connection to RCMS.
  /*p_appInfoSpaceToolBox->createBag("rcmsStateListener", m_gemRCMSNotifier.getRcmsStateListenerParameter(),
    m_gemRCMSNotifier.getRcmsStateListenerParameter(),
    GEMUpdateType::PROCESS);*/

  //  p_appInfoSpace->fireItemAvailable("ScanParameters",&m_scanparameters);
  /*xdata::Bag<GEMSupervisor> *bagscan;
  bagscan->addField("runType", &m_RunType);
  p_appInfoSpace->fireItemAvailable("runType_",        &m_RunType_ );
  p_appInfoSpace->fireItemAvailable("Min_",           &m_Min_ );
  p_appInfoSpace->fireItemAvailable("Max_",           &m_Max_ );
  p_appInfoSpace->fireItemAvailable("StepSize_",      &m_StepSize_ );
  p_appInfoSpace->fireItemAvailable("NTriggers_",      &m_NTriggers_ );
  */

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

  p_appInfoSpaceToolBox->createBool("ReportStateToRCMS", m_reportToRCMS.value_,
                                    &m_reportToRCMS,
                                    GEMUpdateType::PROCESS);

  p_appInfoSpace->addItemRetrieveListener("rcmsStateListener",      this);
  p_appInfoSpace->addItemRetrieveListener("foundRcmsStateListener", this);
  p_appInfoSpace->addItemRetrieveListener("RCMSStateListenerURL",   this);
  p_appInfoSpace->addItemRetrieveListener("ReportStateToRCMS",      this);
  p_appInfoSpace->addItemChangedListener( "rcmsStateListener",      this);
  p_appInfoSpace->addItemChangedListener( "foundRcmsStateListener", this);
  p_appInfoSpace->addItemChangedListener( "RCMSStateListenerURL",   this);
  p_appInfoSpace->addItemChangedListener( "ReportStateToRCMS",      this);
}

gem::supervisor::GEMSupervisor::~GEMSupervisor()
{
  // make sure to empty the v_supervisedApps  vector and free the pointers
  v_supervisedApps.clear();
  m_tcdsLock.lock();
  v_leasedTCDSApps.clear();
  m_tcdsLock.unlock();
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

    m_dbName        = m_dbInfo.bag.dbName.toString();
    m_dbHost        = m_dbInfo.bag.dbHost.toString();
    m_dbPort        = m_dbInfo.bag.dbPort.value_;
    m_dbUser        = m_dbInfo.bag.dbUser.toString();
    m_dbPass        = m_dbInfo.bag.dbPass.toString();
    m_setupTag      = m_dbInfo.bag.setupTag.toString();
    m_runPeriod     = m_dbInfo.bag.runPeriod.toString();
    m_setupLocation = m_dbInfo.bag.setupLocation.toString();

    m_handleTCDS    = m_tcdsConfig.bag.handleTCDS.value_;
    DEBUG("GEMSupervisor::actionPerformed m_dbInfo = " << m_dbInfo.bag.toString());
  }

  // item is changed, update it
  if (event.type() == "ItemChangedEvent" || event.type() == "urn:xdata-event:ItemChangedEvent") {
    DEBUG("GEMSupervisor::actionPerformed() ItemChangedEvent");
  }

  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::supervisor::GEMSupervisor::init()
{
  v_supervisedApps.clear();
  v_supervisedApps.reserve(0);

  // put a mutex on this
  m_tcdsLock.lock();
  DEBUG("GEMSupervisor::init clearing TCDS leased applications list");
  v_leasedTCDSApps.clear();
  v_leasedTCDSApps.reserve(0);
  m_tcdsLock.unlock();
  // until here

  m_globalState.clear();

  DEBUG("GEMSupervisor::init:: looping over " << p_appZone->getGroupNames().size() << " groups");
  std::set<xdaq::ApplicationDescriptor*> used;
  std::set<std::string> groups = p_appZone->getGroupNames();
  for (auto i =groups.begin(); i != groups.end(); ++i) {
    DEBUG("GEMSupervisor::init::xDAQ group: " << *i
          << "getApplicationGroup() " << p_appZone->getApplicationGroup(*i)->getName());

    xdaq::ApplicationGroup* ag = const_cast<xdaq::ApplicationGroup*>(p_appZone->getApplicationGroup(*i));
#ifdef x86_64_centos7
    std::set<const xdaq::ApplicationDescriptor*> allApps = ag->getApplicationDescriptors();
#else
    std::set<xdaq::ApplicationDescriptor*> allApps = ag->getApplicationDescriptors();
#endif
    DEBUG("GEMSupervisor::init::getApplicationDescriptors() " << allApps.size());
    for (auto j = allApps.begin(); j != allApps.end(); ++j) {
      std::string classname = (*j)->getClassName();
      DEBUG("GEMSupervisor::init::xDAQ application descriptor " << *j << " " << classname << " we are " << p_appDescriptor);

      if (used.find(const_cast<xdaq::ApplicationDescriptor*>(*j)) != used.end())
        continue;  // no duplicates
      if ((*j) == p_appDescriptor )
        continue;  // don't fire the command into the GEMSupervisor again

      // maybe just write a function that populates some vectors
      // with the application classes that we want to supervise
      // avoids the problem of picking up all the xDAQ related processes
      // if (isGEMSupervised(*j))
      if (manageApplication(classname)) {
        INFO("GEMSupervisor::init::pushing " << classname << "(" << *j << ") to list of supervised applications");
        v_supervisedApps.push_back(const_cast<xdaq::ApplicationDescriptor*>(*j));
        std::stringstream managedAppStateName;
        managedAppStateName << classname << ":lid:" << (*j)->getLocalId();
        std::stringstream managedAppStateURN;
        managedAppStateURN << (*j)->getURN();
        // have to figure out what we want here, with change to pointers
        std::string appNameKey = managedAppStateName.str();
        appNameKey = appNameKey.substr(appNameKey.rfind("::")+2);
        boost::replace_all(appNameKey, ":", "-");
        p_appStateInfoSpaceToolBox->createString(appNameKey,
                                                 // appNameKey, NULL);
                                                 managedAppStateURN.str(), NULL);

        m_globalState.addApplication(const_cast<xdaq::ApplicationDescriptor*>(*j));
      }
      DEBUG("done");
    }  // done iterating over applications in group
    DEBUG("GEMSupervisor::init::done iterating over applications in group");
  }  // done iterating over groups in zone
  DEBUG("GEMSupervisor::init::done iterating over groups in zone");

  DEBUG("GEMSupervisor::init::starting the monitoring");

  // borrowed from hcalSupervisor
  if (m_reportToRCMS /*&& !m_hasDoneStandardInit*/) {
    m_gemRCMSNotifier.findRcmsStateListener();
    std::string classname = m_gemRCMSNotifier.getRcmsStateListenerParameter()->bag.classname.value_;
    int instance          = m_gemRCMSNotifier.getRcmsStateListenerParameter()->bag.instance.value_;
    m_rcmsStateListenerUrl = getApplicationContext()->getDefaultZone()
      ->getApplicationDescriptor(classname, instance)
      ->getContextDescriptor()->getURL();
    INFO("RCMSStateListener found with url: " << m_rcmsStateListenerUrl.toString());
  }

  // when to do this, have to make sure that all applications have been loaded...
  // p_gemMonitor->addInfoSpace("AppStateMonitoring", p_appStateInfoSpaceToolBox);
  dynamic_cast<gem::supervisor::GEMSupervisorMonitor*>(p_gemMonitor)->setupAppStateMonitoring();
  p_gemMonitor->startMonitoring();
  m_globalState.startTimer();
}

// state transitions
void gem::supervisor::GEMSupervisor::initializeAction()
{
  INFO("GEMSupervisor::initializeAction start");
  DEBUG("GEMSupervisor::initializeAction:: HandleTCDS is " << m_handleTCDS.value_);

  // moved here from constructor, this is not what i want but it's how XDAQ works with setDefaultParameters,
  // as i need to know about handleTCDS before the init function...
  v_supervisedApps.clear();
  // reset the GEMInfoSpaceToolBox object?
  // where can we get some nice PNG images for our different applications?
  // getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemsupervisor/images/supervisor/GEMSupervisor.png");
  init();

  // while ((m_gemfsm.getCurrentState()) != m_gemfsm.getStateName(gem::base::STATE_CONFIGURING)) {  // deal with possible race condition
  while (!(m_globalState.getStateName() == "Initial" && getCurrentState() == "Initializing")) {
    INFO("GEMSupervisor::initializeAction global state not in " << gem::base::STATE_INITIAL
	 << " sleeping (" << m_globalState.getStateName() << ","
	 << getCurrentState() << ")");
    usleep(100);
    m_globalState.update();
  }


  if (m_useLocalDBInstance)
    p_gemDBHelper = std::make_shared<gem::utils::db::GEMDatabaseUtils>(m_dbHost.toString(),
                                                                       m_dbPort.value_,
                                                                       m_dbUser.toString(),
                                                                       m_dbPass.toString());

  try {
    if (m_useLocalDBInstance)
      p_gemDBHelper->connect(m_dbName.toString());

    // do this only when RCMS is not present
    // for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    auto initorder = getInitializationOrder();
    for (auto i = initorder.begin(); i != initorder.end(); ++i) {
      // if (!m_gemRCMSNotifier.getFoundRcmsStateListenerParameter()) {
      if (true) {
        INFO("GEMSupervisor::initializeAction No RCMS state listener found, continuing to initialize children ");
        for (auto j = i->begin(); j != i->end(); ++j) {
          if (((*j)->getClassName()).rfind("tcds::") != std::string::npos) {
            INFO("GEMSupervisor::initializeAction Halting " << (*j)->getClassName()
                 << " in case it is not in 'Halted'");
            // need to ensure leases are properly respected
            gem::utils::soap::GEMSOAPToolBox::sendCommand("Halt", p_appContext, p_appDescriptor, *j);
          } else {
            INFO("GEMSupervisor::initializeAction Initializing " << (*j)->getClassName());
            gem::utils::soap::GEMSOAPToolBox::sendCommand("Initialize", p_appContext, p_appDescriptor, *j);
          }
        }
      }
      // check that group state of *i has moved to desired state before continuing
      while (m_globalState.compositeState(*i) != gem::base::STATE_HALTED) {
        DEBUG("GEMSupervisor::initializeAction waiting for group to reach Halted: "
              << m_globalState.compositeState(*i));
        usleep(10);
        m_globalState.update();
      }
    }
    // why is initializeAction treated differently than the other state transitions?
    // should make this uniform, or was it due to wanting to fail on DB errors?
  } catch (gem::supervisor::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::initializeAction unable to initialize " << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::SOAPException& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::initializeAction unable to initialize " << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::DBConnectionError& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::initializeAction unable to initialize " << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::initializeAction unable to initialize " << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (xcept::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::initializeAction unable to initialize " << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::initializeAction unable to initialize " << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (...) {
    std::stringstream msg;
    msg << "GEMSupervisor::initializeAction unable to initialize";
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  }

  // SHOULD ONLY REPORT "INITIALIZED" TO RCMS HERE
  m_globalState.update();
  INFO("GEMSupervisor::initializeAction GlobalState = " << m_globalState.getStateName()
       << " with GlobalStateMessage = " << m_globalState.getStateMessage());
}

void gem::supervisor::GEMSupervisor::configureAction()
{
  INFO("GEMSupervisor::configureAction start");

  while (!((m_globalState.getStateName() == "Halted"     && getCurrentState() == "Configuring") ||
           (m_globalState.getStateName() == "Configured" && getCurrentState() == "Configuring"))) {
    INFO("GEMSupervisor::configureAction global state not in " << gem::base::STATE_HALTED
	 << " or "  << gem::base::STATE_CONFIGURED
	 << " sleeping (" << m_globalState.getStateName() << ","
	 << getCurrentState() << ")");
    usleep(10);
    m_globalState.update();
  }

  try {
    for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
      sendCfgType("testCfgType", (*i));
      sendRunType("testRunType", (*i));
      sendRunNumber(10254, (*i));

      if (!(isGEMApplication((*i)->getClassName())))
        continue;

      if (m_scanInfo.bag.scanType.value_ == 2 || m_scanInfo.bag.scanType.value_ == 3) {
	INFO("GEMSupervisor::configureAction Setting ScanParameters " << (*i)->getClassName());
	sendScanParameters(*i);
      }
    }
    std::string command = "Configure";
    auto configorder = getConfigureOrder();
    for (auto i = configorder.begin(); i != configorder.end(); ++i) {
      for (auto j = i->begin(); j != i->end(); ++j) {
        INFO("GEMSupervisor::configureAction Configuring " << (*j)->getClassName());
        m_globalState.setGlobalStateMessage("Configuring " + (*j)->getClassName());
        if (((*j)->getClassName()).rfind("tcds::") != std::string::npos) {
          // if (tcdsState() == gem::base::STATE_CONFIGURED)
          //   command = "Reconfigure";
          // xdata::Bag<xdata::Serializable> tcdsParams;
          std::unordered_map<std::string, xdata::Serializable*> tcdsParams;
          std::string content;
          if (((*j)->getClassName()).rfind("ICI") != std::string::npos) {
            std::ifstream ifs(m_tcdsConfig.bag.iciHWConfig.toString());
            content.assign( (std::istreambuf_iterator<char>(ifs) ),
                            (std::istreambuf_iterator<char>()    ) );
            INFO("GEMSupervisor::configureAction ICI HW config " << m_tcdsConfig.bag.iciHWConfig.toString()
                 << " (0x" << std::hex << ifs << std::dec << ") is:" << std::endl << content);
          } else if (((*j)->getClassName()).rfind("PI") != std::string::npos) {
            std::ifstream ifs(m_tcdsConfig.bag.piHWConfig.toString());
            content.assign( (std::istreambuf_iterator<char>(ifs) ),
                            (std::istreambuf_iterator<char>()    ) );
            // tcdsParams.addField("usePrimaryTCDS",m_tcdsConfig.bag.usePrimaryTCDS);
            // tcdsParams.addField("fedEnableMask",m_tcdsConfig.bag.fedEnableMask);
            tcdsParams.insert(std::make_pair("usePrimaryTCDS",&(m_tcdsConfig.bag.usePrimaryTCDS)));
            tcdsParams.insert(std::make_pair("fedEnableMask", &(m_tcdsConfig.bag.fedEnableMask)));
          } else if (((*j)->getClassName()).rfind("LPM") != std::string::npos) {
            std::ifstream ifs(m_tcdsConfig.bag.lpmHWConfig.toString());
            content.assign( (std::istreambuf_iterator<char>(ifs) ),
                            (std::istreambuf_iterator<char>()    ) );
            // tcdsParams.addField("fedEnableMask",m_tcdsConfig.bag.fedEnableMask);
            tcdsParams.insert(std::make_pair("fedEnableMask",&(m_tcdsConfig.bag.fedEnableMask)));
          } else if (((*j)->getClassName()).rfind("CPM") != std::string::npos) {
            std::ifstream ifs(m_tcdsConfig.bag.cpmHWConfig.toString());
            content.assign( (std::istreambuf_iterator<char>(ifs) ),
                            (std::istreambuf_iterator<char>()    ) );
            // tcdsParams.addField("fedEnableMask",m_tcdsConfig.bag.fedEnableMask);
            // tcdsParams.addField("noBeamActive", m_tcdsConfig.bag.fedEnableMask);
            tcdsParams.insert(std::make_pair("fedEnableMask",&(m_tcdsConfig.bag.fedEnableMask)));
            tcdsParams.insert(std::make_pair("noBeamActive", &(m_tcdsConfig.bag.fedEnableMask)));
          }

          xdata::String hwConfig(content);
          // tcdsParams.addField("hardwareConfigurationString",content);
          tcdsParams.insert(std::make_pair("hardwareConfigurationString",&(hwConfig)));
          gem::utils::soap::GEMSOAPToolBox::sendCommandWithParameterBag(command, tcdsParams, p_appContext, p_appDescriptor, *j);

          // put a mutex around this
          m_tcdsLock.lock();
          DEBUG("GEMSupervisor::configureAction adding " << (*j)->getClassName() << " to TCDS leased applications list");
          v_leasedTCDSApps.push_back(*j);
          m_tcdsLock.unlock();
          // until here
        } else {
          if (((*j)->getClassName()).rfind("AMC13") != std::string::npos) {
            INFO("GEMSupervisor::configureAction Sending AMC13 Parameters to " << (*j)->getClassName());
            gem::utils::soap::GEMSOAPToolBox::sendAMC13Config(p_appContext, p_appDescriptor, *j);
          }

          gem::utils::soap::GEMSOAPToolBox::sendCommand(command, p_appContext, p_appDescriptor, *j);
        }
      }
      // check that group state of *i has moved to desired state before continuing
      while (m_globalState.compositeState(*i) != gem::base::STATE_CONFIGURED) {
        DEBUG("GEMSupervisor::configureAction waiting for group to reach Configured: "
              << m_globalState.compositeState(*i));
        usleep(10);
        m_globalState.update();
      }
    }

    /*
    // temp workaround, call confAllChambers python script?
    // if P5 config?
    if (m_setupLocation.toString().rfind("P5") != std::string::npos) {
      INFO("GEMSupervisor::configureAction running confAllChambers for P5 setup");
      std::stringstream confcmd;
      // FIXME hard coded for now, but super hacky garbage
      confcmd << "confAllChambers.py -s" << 3
              << " --ztrim=" << 4.0
              << " --vt1bump=" << 10
              << " --config --run";
      int retval = std::system(confcmd.str().c_str());
      if (retval) {
        std::stringstream msg;
        msg << "GEMSupervisor::configureAction unable to configure chambers: " << retval;
        WARN(msg.str());
        XCEPT_RAISE(gem::supervisor::exception::ConfigurationProblem, msg.str());
      }
    }
    */
  } catch (gem::supervisor::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::configureAction unable to configure (gem::supervisor::exception) " << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::SOAPException& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::configureAction unable to configure (gem::utils::exception::SOAPException)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::DBConnectionError& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::configureAction unable to configure (gem::utils::exception::DBConnectionError)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::configureAction unable to configure (gem::utils::exception)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (xcept::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::configureAction unable to configure (xcept)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::configureAction unable to configure (std)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (...) {
    std::stringstream msg;
    msg << "GEMSupervisor::configureAction unable to configure (unknown exception)";
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  }

  // SHOULD ONLY REPORT "CONFIGURED" TO RCMS HERE
  m_globalState.update();
  INFO("GEMSupervisor::configureAction GlobalState = " << m_globalState.getStateName()
       << " with GlobalStateMessage = " << m_globalState.getStateMessage());
}

void gem::supervisor::GEMSupervisor::startAction()
{
  INFO("GEMSupervisor::startAction start");

  while (!(m_globalState.getStateName() == "Configured" && getCurrentState() == "Starting")) {
    INFO("GEMSupervisor::startAction global state not in " << gem::base::STATE_CONFIGURED
	 << " sleeping (" << m_globalState.getStateName() << ","
	 << getCurrentState() << ")");
    usleep(10);
    m_globalState.update();
  }

  try {
    updateRunNumber();
  } catch (gem::utils::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::startAction updateRunNumber failed:" << e.what();
    ERROR(msg.str());
    m_globalState.update();
    XCEPT_RETHROW(gem::supervisor::exception::Exception, msg.str(), e);
    return;
    //throw e;
  } catch (xcept::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::startAction updateRunNumber failed:" << e.what();
    ERROR(msg.str());
    m_globalState.update();
    XCEPT_RETHROW(gem::supervisor::exception::Exception, msg.str(), e);
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::startAction updateRunNumber failed:" << e.what();
    ERROR(msg.str());
    m_globalState.update();
    XCEPT_RAISE(gem::supervisor::exception::Exception, msg.str());
  } catch (...) {
    std::stringstream msg;
    msg << "GEMSupervisor::startAction updateRunNumber failed: unknown exception";
    ERROR(msg.str());
    m_globalState.update();
    XCEPT_RAISE(gem::supervisor::exception::Exception, msg.str());
  }

  if (m_scanType.value_ == 2 || (m_scanType.value_ == 3)){
    m_scanParameter = m_scanInfo.bag.scanMin.value_;
    INFO("GEMSupervisor::startAction Scan");
    if (m_scanType.value_ == 2) {
      INFO(" Latency " << m_scanMin.value_);
    } else if(m_scanType.value_ == 3) {
      INFO(" VT1 " << m_scanMin.value_);
    }
  }

  try {
    for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i)
      sendRunNumber(m_runNumber, (*i));

    auto startorder = getEnableOrder();
    for (auto i = startorder.begin(); i != startorder.end(); ++i) {
      for (auto j = i->begin(); j != i->end(); ++j) {
        INFO("GEMSupervisor::startAction Starting " << (*j)->getClassName());
        if (((*j)->getClassName()).rfind("tcds::") != std::string::npos) {
          std::unordered_map<std::string, xdata::Serializable*> tcdsParams;
          xdata::UnsignedInteger tcdsRunNumber(m_runNumber);
          DEBUG("GEMSupervisor::startAction sending TCDS application " << (*j)->getClassName()
                << " run number: " << m_runNumber.value_ << "(" << m_runNumber.toString() << ")"
                << " as: " << tcdsRunNumber.value_ << "(" << tcdsRunNumber.toString() << ")");
          tcdsParams.insert(std::make_pair("runNumber", &(tcdsRunNumber)));
          gem::utils::soap::GEMSOAPToolBox::sendCommandWithParameterBag("Enable", tcdsParams, p_appContext, p_appDescriptor, *j);
        } else {
          gem::utils::soap::GEMSOAPToolBox::sendCommand("Start", p_appContext, p_appDescriptor, *j);
        }
      }
      // check that group state of *i has moved to desired state before continuing
      while (m_globalState.compositeState(*i) != gem::base::STATE_RUNNING) {
        DEBUG("GEMSupervisor::startAction waiting for group to reach Running: "
              << m_globalState.compositeState(*i));
        usleep(10);
        m_globalState.update();
      }
    }
  } catch (gem::supervisor::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::startAction unable to start (gem::supervisor::exception) " << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::SOAPException& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::startAction unable to start (gem::utils::exception::SOAPException)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::DBConnectionError& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::startAction unable to start (gem::utils::exception::DBConnectionError)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::startAction unable to start (gem::utils::exception)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (xcept::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::startAction unable to start (xcept)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::startAction unable to start (std)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (...) {
    std::stringstream msg;
    msg << "GEMSupervisor::startAction unable to start (unknown exception)";
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  }

  // SHOULD ONLY REPORT "RUNNING" TO RCMS HERE
  m_globalState.update();
  INFO("GEMSupervisor::startAction GlobalState = " << m_globalState.getStateName()
       << " with GlobalStateMessage = " << m_globalState.getStateMessage());
}

void gem::supervisor::GEMSupervisor::pauseAction()
{
  INFO("GEMSupervisor::pauseAction start");

  while (!(m_globalState.getStateName() == "Running" && getCurrentState() == "Pausing")) {
    INFO("GEMSupervisor::pauseAction global state not in " << gem::base::STATE_RUNNING
	 << " sleeping (" << m_globalState.getStateName() << ","
	 << getCurrentState() << ")");
    usleep(10);
    m_globalState.update();
  }

  try {
    auto disableorder = getDisableOrder();
    for (auto i = disableorder.begin(); i != disableorder.end(); ++i) {
      for (auto j = i->begin(); j != i->end(); ++j) {
        INFO("GEMSupervisor::pauseAction Pausing " << (*j)->getClassName());
        gem::utils::soap::GEMSOAPToolBox::sendCommand("Pause", p_appContext, p_appDescriptor, *j);
      }
      // check that group state of *i has moved to desired state before continuing
      while (m_globalState.compositeState(*i) != gem::base::STATE_PAUSED) {
        DEBUG("GEMSupervisor::pauseAction waiting for group to reach Paused: "
              << m_globalState.compositeState(*i));
        usleep(10);
        m_globalState.update();
      }
    }
  } catch (gem::supervisor::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::pauseAction unable to pause (gem::supervisor::exception) " << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::SOAPException& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::pauseAction unable to pause (gem::utils::exception::SOAPException)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::DBConnectionError& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::pauseAction unable to pause (gem::utils::exception::DBConnectionError)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::pauseAction unable to pause (gem::utils::exception)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (xcept::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::pauseAction unable to pause (xcept)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::pauseAction unable to pause (std)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (...) {
    std::stringstream msg;
    msg << "GEMSupervisor::pauseAction unable to pause (unknown exception)";
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  }

  // SHOULD ONLY REPORT "PAUSED" TO RCMS HERE
  m_globalState.update();
  INFO("GEMSupervisor::pauseAction GlobalState = " << m_globalState.getStateName()
       << " with GlobalStateMessage = " << m_globalState.getStateMessage());
}

void gem::supervisor::GEMSupervisor::resumeAction()
{
  INFO("GEMSupervisor::resumeAction start");

  while (!(m_globalState.getStateName() == "Paused" && getCurrentState() == "Resuming")) {
    INFO("GEMSupervisor::pauseAction global state not in " << gem::base::STATE_PAUSED
	 << " sleeping (" << m_globalState.getStateName() << ","
	 << getCurrentState() << ")");
    usleep(10);
    m_globalState.update();
  }

  try {
    auto resumeorder = getEnableOrder();
    for (auto i = resumeorder.begin(); i != resumeorder.end(); ++i) {
      for (auto j = i->begin(); j != i->end(); ++j) {
        INFO("GEMSupervisor::resumeAction Resuming " << (*j)->getClassName());
        gem::utils::soap::GEMSOAPToolBox::sendCommand("Resume", p_appContext, p_appDescriptor, *j);
      }
      // check that group state of *i has moved to desired state before continuing
      while (m_globalState.compositeState(*i) != gem::base::STATE_RUNNING) {
        DEBUG("GEMSupervisor::resumeAction waiting for group to reach Running: "
              << m_globalState.compositeState(*i));
        usleep(10);
        m_globalState.update();
      }
    }
  } catch (gem::supervisor::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::resumeAction unable to resume (gem::supervisor::exception) " << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::SOAPException& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::resumeAction unable to resume (gem::utils::exception::SOAPException)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::DBConnectionError& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::resumeAction unable to resume (gem::utils::exception::DBConnectionError)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::resumeAction unable to resume (gem::utils::exception)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (xcept::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::resumeAction unable to resume (xcept)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::resumeAction unable to resume (std)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (...) {
    std::stringstream msg;
    msg << "GEMSupervisor::resumeAction unable to resume (unknown exception)";
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  }

  // SHOULD ONLY REPORT "RUNNING" TO RCMS HERE
  m_globalState.update();
  INFO("GEMSupervisor::resumeAction GlobalState = " << m_globalState.getStateName()
       << " with GlobalStateMessage = " << m_globalState.getStateMessage());
}

void gem::supervisor::GEMSupervisor::stopAction()
{
  INFO("GEMSupervisor::stopAction start");

  while (!((m_globalState.getStateName() == "Running" && getCurrentState() == "Stopping") ||
           (m_globalState.getStateName() == "Paused"  && getCurrentState() == "Stopping"))) {
    INFO("GEMSupervisor::pauseAction global state not in " << gem::base::STATE_RUNNING
	 << " or " << gem::base::STATE_PAUSED
	 << " sleeping (" << m_globalState.getStateName() << ","
	 << getCurrentState() << ")");
    usleep(10);
    m_globalState.update();
  }

  try {
    auto disableorder = getDisableOrder();
    for (auto i = disableorder.begin(); i != disableorder.end(); ++i) {
      for (auto j = i->begin(); j != i->end(); ++j) {
        INFO("GEMSupervisor::stopAction Stopping " << (*j)->getClassName());
        gem::utils::soap::GEMSOAPToolBox::sendCommand("Stop", p_appContext, p_appDescriptor, *j);
      }
      // check that group state of *i has moved to desired state before continuing
      while (m_globalState.compositeState(*i) != gem::base::STATE_CONFIGURED) {
        DEBUG("GEMSupervisor::stopAction waiting for group to reach Configured: "
              << m_globalState.compositeState(*i));
        usleep(10);
        m_globalState.update();
      }
    }
  } catch (gem::supervisor::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::stopAction unable to stop (gem::supervisor::exception) " << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::SOAPException& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::stopAction unable to stop (gem::utils::exception::SOAPException)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::DBConnectionError& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::stopAction unable to stop (gem::utils::exception::DBConnectionError)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::stopAction unable to stop (gem::utils::exception)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (xcept::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::stopAction unable to stop (xcept)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::stopAction unable to stop (std)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (...) {
    std::stringstream msg;
    msg << "GEMSupervisor::stopAction unable to stop (unknown exception)";
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  }

  // SHOULD ONLY REPORT "CONFIGURED" TO RCMS HERE
  m_globalState.update();
  INFO("GEMSupervisor::stopAction GlobalState = " << m_globalState.getStateName()
       << " with GlobalStateMessage = " << m_globalState.getStateMessage());
}

void gem::supervisor::GEMSupervisor::haltAction()
{
  INFO("GEMSupervisor::haltAction start");

  try {
    auto disableorder = getDisableOrder();
    for (auto i = disableorder.begin(); i != disableorder.end(); ++i) {
      for (auto j = i->begin(); j != i->end(); ++j) {
        INFO("GEMSupervisor::haltAction Halting " << (*j)->getClassName());
        gem::utils::soap::GEMSOAPToolBox::sendCommand("Halt", p_appContext, p_appDescriptor, *j);
      }
      // check that group state of *i has moved to desired state before continuing
      while (m_globalState.compositeState(*i) != gem::base::STATE_HALTED) {
        DEBUG("GEMSupervisor::haltAction waiting for group to reach Halted: "
              << m_globalState.compositeState(*i));
        usleep(10);
        m_globalState.update();
      }
    }
  } catch (gem::supervisor::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::haltAction unable to halt (gem::supervisor::exception) " << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::SOAPException& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::haltAction unable to halt (gem::utils::exception::SOAPException)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::DBConnectionError& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::haltAction unable to halt (gem::utils::exception::DBConnectionError)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::haltAction unable to halt (gem::utils::exception)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (xcept::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::haltAction unable to halt (xcept)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::haltAction unable to halt (std)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (...) {
    std::stringstream msg;
    msg << "GEMSupervisor::haltAction unable to halt (unknown exception)";
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  }

  // SHOULD ONLY REPORT "HALTED" TO RCMS HERE
  m_globalState.update();
  INFO("GEMSupervisor::haltAction GlobalState = " << m_globalState.getStateName()
       << " with GlobalStateMessage = " << m_globalState.getStateMessage());
}

void gem::supervisor::GEMSupervisor::resetAction()
{
  INFO("GEMSupervisor::resetAction start");

  try {
    for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
      if (((*i)->getClassName()).rfind("tcds::") != std::string::npos)
        continue;  // Don't send reset to TCDS
      INFO("GEMSupervisor::resetAction Resetting " << (*i)->getClassName());
      gem::utils::soap::GEMSOAPToolBox::sendCommand("Reset", p_appContext, p_appDescriptor, *i);
    }
    // gem::base::GEMFSMApplication::resetAction();
  } catch (gem::supervisor::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::resetAction unable to reset (gem::supervisor::exception) " << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::SOAPException& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::resetAction unable to reset (gem::utils::exception::SOAPException)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::DBConnectionError& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::resetAction unable to reset (gem::utils::exception::DBConnectionError)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (gem::utils::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::resetAction unable to reset (gem::utils::exception)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (xcept::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::resetAction unable to reset (xcept)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::resetAction unable to reset (std)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  } catch (...) {
    std::stringstream msg;
    msg << "GEMSupervisor::resetAction unable to reset (unknown exception)";
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
  }

  // SHOULD ONLY REPORT "INITIAL" TO RCMS HERE
  m_globalState.update();
  INFO("GEMSupervisor::resetAction GlobalState = " << m_globalState.getStateName()
       << " with GlobalStateMessage = " << m_globalState.getStateMessage());
}

/*
  void gem::supervisor::GEMSupervisor::noAction()
  {
  }
*/

void gem::supervisor::GEMSupervisor::failAction(toolbox::Event::Reference e)
{
  m_globalState.update();
  INFO("GEMSupervisor::failAction GlobalState = " << m_globalState.getStateName()
       << " with GlobalStateMessage = " << m_globalState.getStateMessage());
}

void gem::supervisor::GEMSupervisor::resetAction(toolbox::Event::Reference e)
{
  m_globalState.update();
  INFO("GEMSupervisor::resetAction GlobalState = " << m_globalState.getStateName()
       << " with GlobalStateMessage = " << m_globalState.getStateMessage());
}


bool gem::supervisor::GEMSupervisor::isGEMApplication(const std::string& classname) const
{
  if (classname.find("gem::") != std::string::npos)
    return true;  // handle all GEM applications
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
  if (classname.find("gem::") != std::string::npos)
    return true;  // handle all GEM applications
  if (classname.find("PeerTransport") != std::string::npos)
    return false;  // ignore all peer transports

  DEBUG("GEMSupervisor::manageApplication:: classname is '"
        << classname << "' HandleTCDS is " << m_handleTCDS.value_);
  if (classname.find("tcds::") != std::string::npos && m_handleTCDS.value_)
    return true;

  // if using uFedKit readout, following applications:
  if ((classname == "evb::EVM" || classname == "evb::BU" ||
       classname == "pt::blit::Application" || classname == "ferol::FerolController")
      && m_useFedKitReadout.value_)
    return true;

  return false;  // assume not ok.
}

void gem::supervisor::GEMSupervisor::globalStateChanged(toolbox::fsm::State before, toolbox::fsm::State after)
{
  INFO("GEMSupervisor::globalStateChanged(" << before << "," << after << ")");

  // Notify RCMS of a state change.
  m_stateName = GEMGlobalState::getStateName(after);

  // if state is terminal only?
  // ignore Initial? (only after after Reset?)
  if (std::string("UBHCEPF").rfind(after) != std::string::npos) {
    try {
      if (m_reportToRCMS)
        INFO("GEMSupervisor::globalStateChanged::Notifying RCMS of state change: ("
             << before << "," << after << "), "
             << m_globalState.getStateMessage());
      m_gemRCMSNotifier.stateChanged(GEMGlobalState::getStateName(after), "GEM global state changed: "
                                     + (m_globalState.getStateMessage()));
    } catch(xcept::Exception& err) {
      ERROR("GEMSupervisor::globalStateChanged::Failed to notify RCMS of state change: "
            << xcept::stdformat_exception_history(err));
      XCEPT_DECLARE_NESTED(gem::base::utils::exception::RCMSNotificationError, top,
                           "Failed to notify RCMS of state change.", err);
      notifyQualified("error", top);
    }
  }

  // ensure that the supervisor FSM goes to error if the composite state is error
  // May want to also disable the updating of the state until a reset is issued...
  if (m_stateName == "Error") {
    fireEvent("Fail");
    m_globalState.update();
  }

  // if (m_stateName == "Error") {
  //   XCEPT_RAISE(gem::supervisor::exception::TransitionProblem, "Composite state is 'Error'");
  // }
}

void gem::supervisor::GEMSupervisor::updateRunNumber()
{
  INFO("GEMSupervisor::updateRunNumber called");
  // should be able to find the run number from the run number service, or some other source
  // get the last entry
  // query parameter
  /*
    std::string sqlString = ""
    + "SELECT DISTINCT runnumbertbl.runnumber, runnumbertbl.username, runnumbertbl.bookingtime, runsession_parameter.session_id "
    + "FROM runnumbertbl "
    + "LEFT JOIN runsession_parameter ON (runsession_parameter.runnumber = runnumbertbl.runnumber) "
    + whereSessionId
    + whereUserName
    + whereSequenceName
    + "ORDER BY runnumbertbl.runnumber DESC "
    + limit;
  */

  // // book the next run number
  // std::string sqlInsert = "INSERT INTO runnumbertbl (USERNAME,SEQUENCENAME,SEQUENCENUMBER) VALUES (?,?,?)";

  /*ldqm_db example
    | id  | Name                             | Type  | Number | Date       | Period | Station | Status | State_id |
    +-----+----------------------------------+-------+--------+------------+--------+---------+--------+----------+
    |   4 | run000001_bench_TAMU_2015-12-15  | bench | 000001 | 2015-12-16 | 2015T  | TAMU    |      1 |     NULL |
  */

  if (m_useLocalDBInstance) {
    // hacky time for teststand/local runs, before connection through RCMS to RunInfoDB is established
    // get these from or send them to the readout application
    std::string    setup = m_setupTag.toString();
    std::string   period = m_runPeriod.toString();
    std::string location = m_setupLocation.toString();
    try {
      INFO("GEMSupervisor::updateRunNumber trying to configure the local DB");
      p_gemDBHelper->configure(location,setup,period, m_runNumber.value_);
    } catch (gem::utils::exception::DBPythonError& e) {
      std::stringstream msg;
      msg << "GEMSupervisor::updateRunNumber python DB Configure call failed";
      ERROR(msg.str());
      m_globalState.update();
      // fireEvent("Fail");
      XCEPT_RETHROW(gem::supervisor::exception::Exception, msg.str(), e);
    }

    if (m_useLocalRunNumber) {
      try {
        INFO("GEMSupervisor::updateRunNumber trying to connect to the local DB");
        p_gemDBHelper->connect(m_dbName.toString());

        std::string lastRunNumberQuery = "SELECT Number FROM ldqm_db_run WHERE Station LIKE '";
        lastRunNumberQuery += location;
        lastRunNumberQuery += "' ORDER BY Number DESC LIMIT 1;";

        try {
          INFO("GEMSupervisor::updateRunNumber trying to get the latest run number");
          m_runNumber.value_ = p_gemDBHelper->query(lastRunNumberQuery);
        } catch (gem::utils::exception::DBEmptyQueryResult& e) {
          ERROR("GEMSupervisor::updateRunNumber caught gem::utils::DBEmptyQueryResult " << e.what());
          m_globalState.update();
          XCEPT_RAISE(gem::utils::exception::DBConnectionError, e.what());
        } catch (xcept::Exception& e) {
          ERROR("GEMSupervisor::updateRunNumber caught xcept::Exception " << e.what());
          m_globalState.update();
          XCEPT_RAISE(gem::utils::exception::DBConnectionError, e.what());
        } catch (std::exception& e) {
          ERROR("GEMSupervisor::updateRunNumber caught std::exception " << e.what());
          m_globalState.update();
          XCEPT_RAISE(gem::utils::exception::DBConnectionError, e.what());
        }

        INFO("GEMSupervisor::updateRunNumber, run number from database is : " << m_runNumber.toString());
        //parse and increment by 1, if it is a new station, start at 1
        //m_runNumber.value_ += 1;
        INFO("GEMSupervisor::updateRunNumber, new run number is: " << m_runNumber.toString());
      } catch (gem::utils::exception::DBConnectionError& e) {
        std::stringstream msg;
        msg << "GEMSupervisor::updateRunNumber unable to connect to the database (DBConnectionError)" << e.what();
        ERROR(msg.str());
        fireEvent("Fail");
        m_globalState.update();
        // XCEPT_RETHROW(gem::supervisor::exception::Exception, msg.str(), e);
        // XCEPT_RAISE(gem::utils::exception::Exception, msg.str());
        // XCEPT_RETHROW(gem::utils::exception::Exception, msg.str(), e);
      } catch (xcept::Exception& e) {
        std::stringstream msg;
        msg << "GEMSupervisor::updateRunNumber unable to connect to the database (xcept)" << e.what();
        ERROR(msg.str());
        fireEvent("Fail");
        m_globalState.update();
        // XCEPT_RETHROW(gem::supervisor::exception::Exception, msg.str(), e);
        // XCEPT_RAISE(gem::utils::exception::Exception, msg.str());
        // XCEPT_RETHROW(gem::utils::exception::Exception, msg.str(), e);
      } catch (std::exception& e) {
        std::stringstream msg;
        msg << "GEMSupervisor::updateRunNumber unable to connect to the database (std)" << e.what();
        ERROR(msg.str());
        fireEvent("Fail");
        m_globalState.update();
        // XCEPT_RETHROW(gem::supervisor::exception::Exception, msg.str(), e);
        // XCEPT_RAISE(gem::utils::exception::Exception, msg.str());
        // XCEPT_RAISE(gem::utils::exception::Exception, msg.str());
      }
    }
  }
  INFO("GEMSupervisor::updateRunNumber done");
}

void gem::supervisor::GEMSupervisor::sendCfgType(std::string const& cfgType, xdaq::ApplicationDescriptor* ad)
//  throw (xoap::exception::Exception)
{
  INFO("GEMSupervisor::sendCfgType to " << ad->getClassName());
  gem::utils::soap::GEMSOAPToolBox::sendApplicationParameter("CfgType", "xsd:string", m_cfgType.toString(),
                                                             p_appContext, p_appDescriptor, ad);
}

void gem::supervisor::GEMSupervisor::sendRunType(std::string const& runType, xdaq::ApplicationDescriptor* ad)
//  throw (xoap::exception::Exception)
{
  INFO("GEMSupervisor::sendRunType to " << ad->getClassName());
  gem::utils::soap::GEMSOAPToolBox::sendApplicationParameter("RunType", "xsd:string", m_runType.toString(),
                                                             p_appContext, p_appDescriptor, ad);
}

void gem::supervisor::GEMSupervisor::sendRunNumber(int64_t const& runNumber, xdaq::ApplicationDescriptor* ad)
//  throw (xoap::exception::Exception)
{
  INFO("GEMSupervisor::sendRunNumber " << m_runNumber.toString() << " to " << ad->getClassName());
  gem::utils::soap::GEMSOAPToolBox::sendApplicationParameter("RunNumber", "xsd:long",
                                                             m_runNumber.toString(),
                                                             p_appContext, p_appDescriptor, ad);

  // if using uFedKit:
  // run number parameter is runNumber for applications evb::EVM and evb::BU
  // rawDataDir and metaDataDir are output location
  // ferol::FerolController, pt::blit::Application, evb::EVM, evb::BU
}

void gem::supervisor::GEMSupervisor::renewTCDSLease()
//  throw (xoap::exception::Exception)
{
  if (m_handleTCDS) {
    gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_tcdsLock);
    for (auto tcdsApp = v_leasedTCDSApps.begin(); tcdsApp != v_leasedTCDSApps.end(); ++tcdsApp) {
      DEBUG("GEMSupervisor::renewTCDSLease renewing lease for " <<  (*tcdsApp)->getClassName());
      gem::utils::soap::GEMSOAPToolBox::sendCommand("RenewHardwareLease", p_appContext, p_appDescriptor, *tcdsApp);
    }
  }
}

void gem::supervisor::GEMSupervisor::sendScanParameters(xdaq::ApplicationDescriptor* ad)
//  throw (xoap::exception::Exception)
{

  INFO("GEMSupervisor::sendScanParameter ScanInfo " << std::endl
       << m_scanInfo.bag.toString());
  // needs try/catch block
  gem::utils::soap::GEMSOAPToolBox::sendApplicationParameterBag("ScanInfo", m_scanInfo, p_appContext, p_appDescriptor, ad);

}

xoap::MessageReference gem::supervisor::GEMSupervisor::EndScanPoint(xoap::MessageReference msg)
//  throw (xoap::exception::Exception)
{
  std::string commandName = "EndScanPoint";

  uint32_t updatedParameter = m_scanParameter + m_stepSize.value_;

  INFO("GEMSupervisor::EndScanPoint GlobalState = " << m_globalState.getStateName() << std::endl
       << " GlobalStateMessage  = " << m_globalState.getStateName()  << std::endl
       << " m_scanParameter  = " << m_scanParameter  << std::endl
       << " updatedParameter = " << updatedParameter << std::endl
       << " m_scanMax.value_ = " << m_scanMax.value_ << std::endl
       << " m_scanInfo.bag.scanMax.value_ = " << m_scanInfo.bag.scanMax.value_ << std::endl
       );
  if (updatedParameter <= m_scanMax.value_) {
    if (m_scanType.value_ == 2) {
      INFO("GEMSupervisor::EndScanPoint LatencyScan Latency " << updatedParameter);
    } else if (m_scanType.value_ == 3) {
      INFO("GEMSupervisor::EndScanPoint ThresholdScan VT1 " << updatedParameter);
    }

    while (!(m_globalState.getStateName() == "Running" && getCurrentState() == "Running")) {
      TRACE("GEMSupervisor::EndScanPoint GlobalState = " << m_globalState.getStateName()
	    << " FSM state " << getCurrentState());
      usleep(10);
      m_globalState.update();
    }

    INFO("GEMSupervisor::EndScanPoint GlobalState = " << m_globalState.getStateName()
	  << " FSM state " << getCurrentState()
	  << " calling pauseAction");
    fireEvent("Pause");

    while (!(m_globalState.getStateName() == "Paused" && getCurrentState() == "Paused")) {
      TRACE("GEMSupervisor::EndScanPoint GlobalState = " << m_globalState.getStateName()
	    << " FSM state " << getCurrentState());
      usleep(10);
      m_globalState.update();
    }

    INFO("GEMSupervisor::EndScanPoint GlobalState = " << m_globalState.getStateName()
	  << " FSM state " << getCurrentState()
	  << " calling resumeAction");
    fireEvent("Resume");

    m_scanParameter = updatedParameter;
    while (!(m_globalState.getStateName() == "Running" && getCurrentState() == "Running")) {
      TRACE("GEMSupervisor::EndScanPoint GlobalState = " << m_globalState.getStateName()
	    << " FSM state " << getCurrentState());
      usleep(10);
      m_globalState.update();
    }
  } else {
    INFO("GEMSupervisor::EndScanPoint Scan Finished " << updatedParameter);
    INFO("GEMSupervisor::EndScanPoint GlobalState = " << m_globalState.getStateName()
	  << " FSM state " << getCurrentState()
          << " calling stopAction");

    fireEvent("Stop");
    usleep(100);
  }

  try {
    INFO("GEMSupervisor::EndScanPoint " << commandName << " succeeded ");
    return
      gem::utils::soap::GEMSOAPToolBox::makeSOAPReply(commandName, "SentTriggers");
  } catch(xcept::Exception& err) {
    std::string msgBase = toolbox::toString("Failed to create SOAP reply for command '%s'",
                                            commandName.c_str());
    ERROR(toolbox::toString("%s: %s.", msgBase.c_str(), xcept::stdformat_exception(err).c_str()));
    XCEPT_DECLARE_NESTED(gem::base::utils::exception::SoftwareProblem,
                         top, toolbox::toString("%s.",msgBase.c_str()), err);
    this->notifyQualified("error", top);

    m_globalState.update();
    XCEPT_RETHROW(xoap::exception::Exception, msgBase, err);
  }
  m_globalState.update();
  XCEPT_RAISE(xoap::exception::Exception,"command not found");
}

/////////////////////////////////////////
//* Order of transition operations*//
/*
 - Initialize - Connect to hardware, set up some defaults?
    - TCDS applications DO NOTHING as per prescription, but maybe what we call Initialize is not what they call Initialize
    - GEM applications
    - Trigger applications
      - AMC13Manager

 - Configure (same as Initialize? maybe eventually remove Initialize?)
    -
*/
// Strongly "borrowed" from HCAL
class InitCompare {
public:
  bool operator()(xdaq::ApplicationDescriptor* a, xdaq::ApplicationDescriptor* b) const
  {
    return initPriority(a->getClassName()) > initPriority(b->getClassName());
  }

  // higher priority -- initialize first
  static int initPriority(const std::string& classname)
  {
    int priority = 50; // default
    if      (classname == "gemCrate")                               priority = 200; // very first to check the crate connections
    else if (classname == "tcds::lpm::LPMController")               priority = 102; // must be first of TCDS!
    else if (classname == "tcds::ici::ICIController")               priority = 101; // must be second of TCDS!
    else if (classname == "tcds::pi::PIController")                 priority = 100; // must be first!?
    else if (classname == "gem::hw::glib::GLIBManager")             priority =  90; // before (or simultaneous with) OH manager and before  AMC13
    else if (classname == "gem::hw::ctp7::CTP7Manager")             priority =  90; // before (or simultaneous with) OH manager and before  AMC13
    else if (classname == "gem::hw::AMCManager")                    priority =  90; // before (or simultaneous with) OH manager and before  AMC13
    else if (classname == "gem::hw::optohybrid::OptoHybridManager") priority =  90; // after (or simultaneous with) AMC managers and before  AMC13
    else if (classname == "gem::hw::amc13::AMC13Manager")           priority =  35; // after AMCManagers but not last
    else if (classname == "gem::hw::amc13::AMC13Readout")           priority =  30; // after AMC13Manager but not last
    else if (classname == "gem::hw::gemPartitionViewer")            priority =  10; //
    return priority;
  }
};

std::vector<std::vector<xdaq::ApplicationDescriptor* > > gem::supervisor::GEMSupervisor::getInitializationOrder()
{
  std::multimap<int, xdaq::ApplicationDescriptor*, std::greater<int> > tool;
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    // if ((*i)->getClassName().find("tcds") != std::string::npos) {
    //   INFO("GEMSupervisor::getInitializationOrder::Skipping " << (*i)->getClassName()
    //        << " for initialization");
    //   continue;
    // }
    INFO("GEMSupervisor::getInitializationOrder: application "
         << (*i)->getClassName() << " has priority "
         << InitCompare::initPriority((*i)->getClassName()));
    tool.insert(std::make_pair(InitCompare::initPriority((*i)->getClassName()), *i));
  }
  std::vector<std::vector<xdaq::ApplicationDescriptor*> > retval;
  int level = -1;
  for (auto j = tool.begin(); j != tool.end(); ++j) {
    if (j->first < 0)
      continue;
    if (j->first != level) {
      retval.push_back(std::vector<xdaq::ApplicationDescriptor*>());
      level = j->first;
    }
    retval.back().push_back(j->second);
  }
  return retval;
}

std::vector<std::vector<xdaq::ApplicationDescriptor* > > gem::supervisor::GEMSupervisor::getConfigureOrder()
{
  std::multimap<int, xdaq::ApplicationDescriptor*, std::greater<int> > tool;
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO("GEMSupervisor::getConfigureOrder: application "
         << (*i)->getClassName() << " has priority "
         << InitCompare::initPriority((*i)->getClassName()));
    tool.insert(std::make_pair(InitCompare::initPriority((*i)->getClassName()), *i));
  }
  std::vector<std::vector<xdaq::ApplicationDescriptor*> > retval;
  int level = -1;
  for (auto j = tool.begin(); j != tool.end(); ++j) {
    if (j->first < 0)
      continue;
    if (j->first != level) {
      retval.push_back(std::vector<xdaq::ApplicationDescriptor*>());
      level = j->first;
    }
    retval.back().push_back(j->second);
  }
  return retval;
}

/*
    - Start
    - GEM applications first
    - Trigger applications
      - AMC13Manager
    - TCDS applications
      - CPMController/LPMController
      - PIController next
      - ICIController (APVEController) last
*/
class EnableCompare {
public:
  bool operator()(xdaq::ApplicationDescriptor* a, xdaq::ApplicationDescriptor* b) const
  {
    return enablePriority(a->getClassName()) > enablePriority(b->getClassName());
  }

  // higher priority -- enable first
  static int enablePriority(const std::string& classname)
  {
    int priority = 50; // default
    if      (classname == "gem::hw::glib::GLIBManager")             priority = 100; // before (or simultaneous with) OH manager and before  AMC13
    else if (classname == "gem::hw::ctp7::CTP7Manager")             priority = 100; // before (or simultaneous with) OH manager and before  AMC13
    else if (classname == "gem::hw::AMCManager")                    priority = 100; // before (or simultaneous with) OH manager and before  AMC13
    else if (classname == "gem::hw::optohybrid::OptoHybridManager") priority = 100; // after (or simultaneous with) AMC managers and before  AMC13
    else if (classname == "gem::hw::amc13::AMC13Readout")           priority =  90; // before AMC13Manager
    else if (classname == "gem::hw::amc13::AMC13Manager")           priority =  35; // after AMCManagers but not last
    else if (classname == "tcds::lpm::LPMController")               priority =  12; // must be first of TCDS!
    else if (classname == "tcds::pi::PIController")                 priority =  11; // must be before ICI!?
    else if (classname == "tcds::ici::ICIController")               priority =  10; // must be last of TCDS!
    return priority;
  }
};

std::vector<std::vector<xdaq::ApplicationDescriptor* > > gem::supervisor::GEMSupervisor::getEnableOrder()
{
  std::multimap<int, xdaq::ApplicationDescriptor*, std::greater<int> > tool;
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO("GEMSupervisor::getEnableOrder: application "
         << (*i)->getClassName() << " has priority "
         << EnableCompare::enablePriority((*i)->getClassName()));
    tool.insert(std::make_pair(EnableCompare::enablePriority((*i)->getClassName()), *i));
  }
  std::vector<std::vector<xdaq::ApplicationDescriptor*> > retval;
  int level = -1;
  for (auto j = tool.begin(); j != tool.end(); ++j) {
    if (j->first < 0)
      continue;
    if (j->first != level) {
      retval.push_back(std::vector<xdaq::ApplicationDescriptor*>());
      level = j->first;
    }
    retval.back().push_back(j->second);
  }
  return retval;
}


/*
 - Pause
    - TCDS applications
      - CPMController/LPMController
      - ICIController (APVEController) first
      - PIController next
    - Trigger applications
      - AMC13Manager
    - GEM applications

 - Stop (same order as Pause, except Pause TCDS first, then Stop TCDS at the end)
    - Stop TCDS applications? this removes clock?
      - CPMController/LPMController

 - Resume (same as Start)

 - Halt (same order as pause)
    - TCDS applications
      - ICIController (APVEController) first
      - PIController next
*/
// Strongly "borrowed" from HCAL
class DisableCompare {
public:
  bool operator()(xdaq::ApplicationDescriptor* a, xdaq::ApplicationDescriptor* b) const
  {
    return disablePriority(a->getClassName()) > disablePriority(b->getClassName());
  }

  // higher priority -- disable first
  static int disablePriority(const std::string& classname)
  {
    int priority = 50; // default
    if      (classname == "tcds::lpm::LPMController")               priority = 102; // must be first of TCDS!
    else if (classname == "tcds::ici::ICIController")               priority = 101; // must be second of TCDS!
    else if (classname == "tcds::pi::PIController")                 priority = 100; // must be first!?
    else if (classname == "gem::hw::amc13::AMC13Manager")           priority =  95; // after AMCManagers but not last
    else if (classname == "gem::hw::amc13::AMC13Readout")           priority =  90; // after AMC13Manager but not last
    else if (classname == "gem::hw::glib::GLIBManager")             priority =  70; // before (or simultaneous with) OH manager and before  AMC13
    else if (classname == "gem::hw::ctp7::CTP7Manager")             priority =  70; // before (or simultaneous with) OH manager and before  AMC13
    else if (classname == "gem::hw::AMCManager")                    priority =  70; // before (or simultaneous with) OH manager and before  AMC13
    else if (classname == "gem::hw::optohybrid::OptoHybridManager") priority =  70; // after (or simultaneous with) AMC managers and before  AMC13
    return priority;
  }
};

std::vector<std::vector<xdaq::ApplicationDescriptor* > > gem::supervisor::GEMSupervisor::getDisableOrder()
{
  std::multimap<int, xdaq::ApplicationDescriptor*, std::greater<int> > tool;
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO("GEMSupervisor::getDisableOrder: application "
         << (*i)->getClassName() << " has priority "
         << DisableCompare::disablePriority((*i)->getClassName()));

    tool.insert(std::make_pair(DisableCompare::disablePriority((*i)->getClassName()), *i));
  }
  std::vector<std::vector<xdaq::ApplicationDescriptor*> > retval;
  int level = -1;
  for (auto j = tool.begin(); j != tool.end(); ++j) {
    if (j->first < 0)
      continue;
    if (j->first != level) {
      retval.push_back(std::vector<xdaq::ApplicationDescriptor*>());
      level = j->first;
    }
    retval.back().push_back(j->second);
  }
  return retval;
}


/*

 - Reset

 - Fix

 */
