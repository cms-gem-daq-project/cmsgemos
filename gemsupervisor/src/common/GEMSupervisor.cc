/**
 * class: GEMSupervisor
 * description: Supervisor application for GEM system
 *              structure borrowed from TCDS core, with nods to HCAL (hcalSupervisor)
 * author: J. Sturdy
 * date:
 */

#include "gem/supervisor/GEMSupervisor.h"

#include <iomanip>
#include <ctime>

#include "gem/supervisor/GEMSupervisorWeb.h"
#include "gem/supervisor/GEMSupervisorMonitor.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"
#include "gem/utils/exception/Exception.h"
#include "gem/utils/vfat/VFAT2ConfigManager.h"


using namespace std;
typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::GEMSupervisor);

gem::supervisor::GEMSupervisor::GEMSupervisor(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub),
  m_globalState(this->getApplicationContext(), this),
  m_scanParameter(0),
  m_reportToRCMS(false),
  m_gemRCMSNotifier(this->getApplicationLogger(),
                    this->getApplicationDescriptor(),
                    this->getApplicationContext())
{

  xoap::bind(this, &gem::supervisor::GEMSupervisor::EndScanPoint, "EndScanPoint",  XDAQ_NS_URI );
  // xgi::framework::deferredbind(this, this, &GEMSupervisor::xgiDefault, "Default");
  xgi::bind(this, &gem::supervisor::GEMSupervisor::Test, "Test");

  DEBUG("Creating the GEMSupervisorWeb interface");
  p_gemMonitor      = new gem::supervisor::GEMSupervisorMonitor(this);
  // p_gemMonitor      = new gem::supervisor::GEMSupervisorMonitor(this->getApplicationLogger(),this);
  p_gemWebInterface = new gem::supervisor::GEMSupervisorWeb(this);
  DEBUG("done");
  //p_gemMonitor      = new gem generic system monitor

  p_appInfoSpace->fireItemAvailable("DatabaseInfo",&m_dbInfo);
  // p_appInfoSpace->fireItemAvailable("DatabaseName",&m_dbName);
  // p_appInfoSpace->fireItemAvailable("DatabaseHost",&m_dbHost);
  // p_appInfoSpace->fireItemAvailable("DatabasePort",&m_dbPort);
  // p_appInfoSpace->fireItemAvailable("DatabaseUser",&m_dbUser);
  // p_appInfoSpace->fireItemAvailable("DatabasePass",&m_dbPass);

  // p_appInfoSpace->fireItemAvailable("SetupTag",     &m_setupTag);
  // p_appInfoSpace->fireItemAvailable("RunPeriod",    &m_runPeriod);
  // p_appInfoSpace->fireItemAvailable("SetupLocation",&m_setupLocation);

  p_appInfoSpace->addItemRetrieveListener("DatabaseInfo", this);
  // p_appInfoSpace->addItemRetrieveListener("DatabaseName", this);
  // p_appInfoSpace->addItemRetrieveListener("DatabaseHost", this);
  // p_appInfoSpace->addItemRetrieveListener("DatabasePort", this);
  // p_appInfoSpace->addItemRetrieveListener("DatabaseUser", this);
  // p_appInfoSpace->addItemRetrieveListener("DatabasePass", this);

  // p_appInfoSpace->addItemRetrieveListener("SetupTag",      this);
  // p_appInfoSpace->addItemRetrieveListener("RunPeriod",     this);
  // p_appInfoSpace->addItemRetrieveListener("SetupLocation", this);

  p_appInfoSpace->addItemChangedListener("DatabaseInfo", this);
  // p_appInfoSpace->addItemChangedListener("DatabaseName", this);
  // p_appInfoSpace->addItemChangedListener("DatabaseHost", this);
  // p_appInfoSpace->addItemChangedListener("DatabasePort", this);
  // p_appInfoSpace->addItemChangedListener("DatabaseUser", this);
  // p_appInfoSpace->addItemChangedListener("DatabasePass", this);

  // p_appInfoSpace->addItemChangedListener("SetupTag",      this);
  // p_appInfoSpace->addItemChangedListener("RunPeriod",     this);
  // p_appInfoSpace->addItemChangedListener("SetupLocation", this);

  v_supervisedApps.clear();
  // reset the GEMInfoSpaceToolBox object?
  // where can we get some nice PNG images for our different applications?
  // getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemsupervisor/images/supervisor/GEMSupervisor.png");
  init();

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

    m_dbName        = m_dbInfo.bag.dbName.toString();
    m_dbHost        = m_dbInfo.bag.dbHost.toString();
    m_dbPort        = m_dbInfo.bag.dbPort.value_;
    m_dbUser        = m_dbInfo.bag.dbUser.toString();
    m_dbPass        = m_dbInfo.bag.dbPass.toString();
    m_setupTag      = m_dbInfo.bag.setupTag.toString();
    m_runPeriod     = m_dbInfo.bag.runPeriod.toString();
    m_setupLocation = m_dbInfo.bag.setupLocation.toString();

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
{
  INFO("gem::supervisor::GEMSupervisor::initializeAction Initializing");

  // while ((m_gemfsm.getCurrentState()) != m_gemfsm.getStateName(gem::base::STATE_CONFIGURING)) {  // deal with possible race condition
  while (!(m_globalState.getStateName() == "Initial" && getCurrentState() == "Initializing")) {
    INFO("GEMSupervisor::initializeAction global state not in " << gem::base::STATE_INITIAL
	 << " sleeping (" << m_globalState.getStateName() << ","
	 << getCurrentState() << ")");
    usleep(100);
    m_globalState.update();
  }

  p_gemDBHelper = std::make_shared<gem::utils::db::GEMDatabaseUtils>(m_dbHost.toString(),
                                                                     m_dbPort.value_,
                                                                     m_dbUser.toString(),
                                                                     m_dbPass.toString());

  try {
    // if (p_gemDBHelper->connect(m_dbName.toString())) {
    p_gemDBHelper->connect(m_dbName.toString());

    // for (std::vector<xdaq::ApplicationDescriptor*>::iterator i=v_supervisedApps.begin(); i!=v_supervisedApps.end(); i++) {
    for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
      INFO("GEMSupervisor::initializeAction Initializing " << (*i)->getClassName());
      gem::utils::soap::GEMSOAPToolBox::sendCommand("Initialize", p_appContext, p_appDescriptor, *i);
    }
  } catch (gem::utils::exception::DBConnectionError& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::initializeAction unable to connect to the database (DBConnectionError)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
    // XCEPT_RETHROW(gem::supervisor::exception::Exception, msg.str(), e);
    // XCEPT_RETHROW(gem::utils::exception::Exception, msg.str(), e);
  } catch (xcept::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::initializeAction unable to connect to the database (xcept)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
    // XCEPT_RETHROW(gem::supervisor::exception::Exception, msg.str(), e);
    // XCEPT_RETHROW(gem::utils::exception::Exception, msg.str(), e);
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::initializeAction unable to connect to the database (std)" << e.what();
    ERROR(msg.str());
    fireEvent("Fail");
    m_globalState.update();
    // XCEPT_RETHROW(gem::supervisor::exception::Exception, msg.str(), e);
    // XCEPT_RAISE(gem::utils::exception::Exception, msg.str());
  }
  m_globalState.update();
}

void gem::supervisor::GEMSupervisor::configureAction()
{
  while (!((m_globalState.getStateName() == "Halted"     && getCurrentState() == "Configuring") ||
           (m_globalState.getStateName() == "Configured" && getCurrentState() == "Configuring"))) {
    INFO("GEMSupervisor::configureAction global state not in " << gem::base::STATE_HALTED
	 << " or "  << gem::base::STATE_CONFIGURED
	 << " sleeping (" << m_globalState.getStateName() << ","
	 << getCurrentState() << ")");
    usleep(100);
    m_globalState.update();
  }

  try {
    for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
      sendCfgType("testCfgType", (*i));
      sendRunType("testRunType", (*i));
      sendRunNumber(10254, (*i));

      if (m_scanInfo.bag.scanType.value_ == 2 || m_scanInfo.bag.scanType.value_ == 3) {
	INFO("GEMSupervisor::configureAction Setting ScanParameters " << (*i)->getClassName());
	sendScanParameters(*i);
      }

      INFO("GEMSupervisor::configureAction Configuring " << (*i)->getClassName());
      gem::utils::soap::GEMSOAPToolBox::sendCommand("Configure", p_appContext, p_appDescriptor, *i);
      if (((*i)->getClassName()).rfind("AMC13") != std::string::npos) {
        INFO("GEMSupervisor::configureAction Sending AMC13 Parameters to " << (*i)->getClassName());
        gem::utils::soap::GEMSOAPToolBox::sendAMC13Config(p_appContext, p_appDescriptor, *i);
      }
    }

  } catch (gem::supervisor::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::configureAction " << e.what();
    ERROR(msg.str());
    m_globalState.update();
    XCEPT_RETHROW(gem::supervisor::exception::Exception, msg.str(), e);
  } catch (gem::utils::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::configureAction " << e.what();
    ERROR(msg.str());
    m_globalState.update();
    XCEPT_RETHROW(gem::supervisor::exception::Exception, msg.str(), e);
  } catch (xcept::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::configureAction " << e.what();
    ERROR(msg.str());
    m_globalState.update();
    XCEPT_RETHROW(gem::supervisor::exception::Exception, msg.str(), e);
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::configureAction " << e.what();
    ERROR(msg.str());
    m_globalState.update();
    XCEPT_RAISE(gem::supervisor::exception::Exception, msg.str());
  } catch (...) {
    std::stringstream msg;
    msg << "GEMSupervisor::configureAction (unknown exception)";
    ERROR(msg.str());
    m_globalState.update();
    XCEPT_RAISE(gem::supervisor::exception::Exception, msg.str());
  }
  m_globalState.update();
  INFO("GEMSupervisor::configureAction GlobalState = " << m_globalState.getStateName());
}

void gem::supervisor::GEMSupervisor::startAction()
{
  while (!(m_globalState.getStateName() == "Configured" && getCurrentState() == "Starting")) {
    INFO("GEMSupervisor::startAction global state not in " << gem::base::STATE_CONFIGURED
	 << " sleeping (" << m_globalState.getStateName() << ","
	 << getCurrentState() << ")");
    usleep(100);
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

  if(m_scanType.value_ == 2 || (m_scanType.value_ == 3)){
    m_scanParameter = m_scanInfo.bag.scanMin.value_;
    INFO("GEMSupervisor::startAction Scan");
    if (m_scanType.value_ == 2) {
      INFO(" Latency " << m_scanMin.value_);
    } else if(m_scanType.value_ == 3) {
      INFO(" VT1 " << m_scanMin.value_);
    }
  }

  try {
    for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
      sendRunNumber(m_runNumber, (*i));
      INFO("GEMSupervisor::startAction Starting " << (*i)->getClassName());
      gem::utils::soap::GEMSOAPToolBox::sendCommand("Start", p_appContext, p_appDescriptor, *i);
    }

  } catch (gem::supervisor::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::startAction " << e.what();
    ERROR(msg.str());
    m_globalState.update();
    XCEPT_RETHROW(gem::supervisor::exception::Exception, msg.str(), e);
  } catch (gem::utils::exception::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::startAction " << e.what();
    ERROR(msg.str());
    m_globalState.update();
    XCEPT_RETHROW(gem::supervisor::exception::Exception, msg.str(), e);
  } catch (xcept::Exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::startAction " << e.what();
    ERROR(msg.str());
    m_globalState.update();
    XCEPT_RETHROW(gem::supervisor::exception::Exception, msg.str(), e);
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::startAction " << e.what();
    ERROR(msg.str());
    m_globalState.update();
    XCEPT_RAISE(gem::supervisor::exception::Exception, msg.str());
  } catch (...) {
    std::stringstream msg;
    msg << "GEMSupervisor::startAction (unknown exception)";
    ERROR(msg.str());
    m_globalState.update();
    XCEPT_RAISE(gem::supervisor::exception::Exception, msg.str());
  }
  m_globalState.update();
  INFO("GEMSupervisor::startAction GlobalState = " << m_globalState.getStateName());
}

void gem::supervisor::GEMSupervisor::pauseAction()
{
  while (!(m_globalState.getStateName() == "Running" && getCurrentState() == "Pausing")) {
    INFO("GEMSupervisor::pauseAction global state not in " << gem::base::STATE_RUNNING
	 << " sleeping (" << m_globalState.getStateName() << ","
	 << getCurrentState() << ")");
    usleep(100);
    m_globalState.update();
  }

  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO("GEMSupervisor::pauseAction Pausing " << (*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Pause", p_appContext, p_appDescriptor, *i);
  }
  m_globalState.update();
}

void gem::supervisor::GEMSupervisor::resumeAction()
{
  while (!(m_globalState.getStateName() == "Paused" && getCurrentState() == "Resuming")) {
    INFO("GEMSupervisor::pauseAction global state not in " << gem::base::STATE_PAUSED
	 << " sleeping (" << m_globalState.getStateName() << ","
	 << getCurrentState() << ")");
    usleep(100);
    m_globalState.update();
  }

  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO("GEMSupervisor::resumeAction Resuming " << (*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Resume", p_appContext, p_appDescriptor, *i);
  }
  m_globalState.update();
}

void gem::supervisor::GEMSupervisor::stopAction()
{
  while (!((m_globalState.getStateName() == "Running" && getCurrentState() == "Stopping") ||
           (m_globalState.getStateName() == "Paused"  && getCurrentState() == "Stopping"))) {
    INFO("GEMSupervisor::pauseAction global state not in " << gem::base::STATE_RUNNING
	 << " or " << gem::base::STATE_PAUSED
	 << " sleeping (" << m_globalState.getStateName() << ","
	 << getCurrentState() << ")");
    usleep(100);
    m_globalState.update();
  }

  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO("GEMSupervisor::stopAction Stopping " << (*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Stop", p_appContext, p_appDescriptor, *i);
  }
  m_globalState.update();
}

void gem::supervisor::GEMSupervisor::haltAction()
{
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO("GEMSupervisor::haltAction Halting " << (*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Halt", p_appContext, p_appDescriptor, *i);
  }
  m_globalState.update();
}

void gem::supervisor::GEMSupervisor::resetAction()
{
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO("GEMSupervisor::resetAction Resetting " << (*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Reset", p_appContext, p_appDescriptor, *i);
  }
  // gem::base::GEMFSMApplication::resetAction();
  m_globalState.update();
}

void gem::supervisor::GEMSupervisor::Test(xgi::Input * in, xgi::Output * out)
{
  INFO("GEMSupervisor::Test running CAM's test code");
  //std::string glFilenameIn = "/afs/cern.ch/user/b/bravo/public/GEM_VFAT2_Configurations_Sample.xml";
  std::string glFilenameIn = "/afs/cern.ch/user/c/chmclean/public/testGLfile.xml";
  std::string glFilenameOut = "/afs/cern.ch/user/c/chmclean/public/testGLfile_out.xml";
  //std::string chFilenameIn = "/afs/cern.ch/user/b/bravo/public/DAQ_VFAT_CHAN_Settings_Sample.xml";
  std::string chFilenameIn = "/afs/cern.ch/user/c/chmclean/public/testCHfile.xml";
  std::string chFilenameOut = "/afs/cern.ch/user/c/chmclean/public/testCHfile_out.xml";
  
  //parse XML files
  gem::utils::vfat::VFAT2ConfigManager vfatMan(glFilenameIn,chFilenameIn);
  vfatMan.parseXMLFiles();
  INFO("Parsed the files");

  //print some of the saved VFAT parameters
  std::stringstream msg;
  msg << "calibMode: " << (vfatMan.localParams.calibMode);
  INFO(msg.str());
  msg.str("");
  msg << "channel 46 trim: " << int(vfatMan.localParams.channels[46].trimDAC);
  INFO(msg.str());
  
  //save the current time
  time_t rawtime;
  struct tm * timeinfo;
  char buffer [80];

  time (&rawtime);
  timeinfo = localtime (&rawtime);

  strftime (buffer,80,"%F %T",timeinfo);
  string s_time(buffer);
  vfatMan.setTime(s_time.c_str());

  //save the user's name
  string s_user;
  cout << "Please enter your name: ";
  cin >> s_user;
  vfatMan.setUser(s_user);

  //write XML files
  vfatMan.setGLfile(glFilenameOut.c_str());
  vfatMan.setCHfile(chFilenameOut.c_str());
  vfatMan.writeXMLFiles();
  INFO("Wrote the files");
}

/*
  void gem::supervisor::GEMSupervisor::noAction()
  {
  }
*/

void gem::supervisor::GEMSupervisor::failAction(toolbox::Event::Reference e)
{
  m_globalState.update();
}

void gem::supervisor::GEMSupervisor::resetAction(toolbox::Event::Reference e)
{
  m_globalState.update();
}


bool gem::supervisor::GEMSupervisor::isGEMApplication(const std::string& classname) const
{
  if (classname.find("gem") != std::string::npos)
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
  INFO("GEMSupervisor::globalStateChanged(" << before << "," << after << ")");

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

  // hacky time for teststand/local runs, before connection through RCMS to RunInfoDB is established
  // get these from or send them to the readout application
  std::string    setup = m_setupTag.toString();
  std::string   period = m_runPeriod.toString();
  std::string location = m_setupLocation.toString();
  try {
    INFO("GEMSupervisor::updateRunNumber trying to configure the local DB");
    p_gemDBHelper->configure(location,setup,period);
  } catch (gem::utils::exception::DBPythonError& e) {
    std::stringstream msg;
    msg << "GEMSupervisor::updateRunNumber python DB Configure call failed";
    ERROR(msg.str());
    m_globalState.update();
    // fireEvent("Fail");
    XCEPT_RETHROW(gem::supervisor::exception::Exception, msg.str(), e);
  }

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
  INFO("GEMSupervisor::sendRunNumber to " << ad->getClassName());
  gem::utils::soap::GEMSOAPToolBox::sendApplicationParameter("RunNumber", "xsd:long",
                                                             m_runNumber.toString(),
                                                             p_appContext, p_appDescriptor, ad);
}

void gem::supervisor::GEMSupervisor::sendScanParameters(xdaq::ApplicationDescriptor* ad)
//  throw (xoap::exception::Exception)
{

  INFO("GEMSupervisor::sendScanParameter ScanInfo " << std::endl
       << m_scanInfo.bag.toString());

  gem::utils::soap::GEMSOAPToolBox::sendApplicationParameterBag("ScanInfo", m_scanInfo, p_appContext, p_appDescriptor, ad);

}

xoap::MessageReference gem::supervisor::GEMSupervisor::EndScanPoint(xoap::MessageReference msg)
//  throw (xoap::exception::Exception)
{
  std::string commandName = "EndScanPoint";

  uint32_t updatedParameter = m_scanParameter + m_stepSize.value_;

  INFO("GEMSupervisor::EndScanPoint GlobalState = " << m_globalState.getStateName() << std::endl
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
      usleep(100);
      m_globalState.update();
    }

    INFO("GEMSupervisor::EndScanPoint GlobalState = " << m_globalState.getStateName()
	  << " FSM state " << getCurrentState()
	  << " calling pauseAction");
    fireEvent("Pause");

    while (!(m_globalState.getStateName() == "Paused" && getCurrentState() == "Paused")) {
      TRACE("GEMSupervisor::EndScanPoint GlobalState = " << m_globalState.getStateName()
	    << " FSM state " << getCurrentState());
      usleep(100);
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
      usleep(100);
      m_globalState.update();
    }
  } else {
    INFO("GEMSupervisor::EndScanPoint Scan Finished " << updatedParameter);
    INFO("GEMSupervisor::EndScanPoint GlobalState = " << m_globalState.getStateName()
	  << " FSM state " << getCurrentState()
          << " calling stopAction");

    fireEvent("Stop");
    usleep(1000);
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
