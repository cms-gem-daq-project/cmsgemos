/**
 * class: AMC13Manager
 * description: Manager application for AMC13 cards
 *              structure borrowed from TCDS core, with nods to HCAL (DTCManager)
 * author: J. Sturdy
 * date: 
 */

#include "gem/hw/amc13/AMC13ManagerWeb.h"
#include "gem/hw/amc13/AMC13Manager.h"

//#include "gem/hw/amc13/exception/Exception.h"

#include "amc13/AMC13.hh"
#include "amc13/Status.hh"

XDAQ_INSTANTIATOR_IMPL(gem::hw::amc13::AMC13Manager);

void gem::hw::amc13::AMC13Manager::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{

  bag->addField("ConnectionFile",           &connectionFile     );

  bag->addField("AMCInputEnableList",       &amcInputEnableList );
  bag->addField("AMCIgnoreTTSList",         &amcIgnoreTTSList   );

  bag->addField("EnableFakeData",         &enableFakeData    );
  bag->addField("MonitorBackPressure",    &monBackPressure   );
  bag->addField("EnableLocalTTC",         &enableLocalTTC    );

  bag->addField("Prescale",               &prescaleFactor    );
  bag->addField("BCOffset",               &bcOffset          );

  bag->addField("FEDID",                  &fedID             );
  bag->addField("SFPMask",                &sfpMask           );

  bag->addField("EnableLocalL1AMask",     &enableLocalL1AMask);
}

gem::hw::amc13::AMC13Manager::AMC13Manager(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub),
  deviceLock_(toolbox::BSem::FULL, true),
  amc13Device_(0)
{
  m_crateID = -1;
  m_slot    = 13;
  
  getApplicationInfoSpace()->fireItemAvailable("crateID", &m_crateID);
  getApplicationInfoSpace()->fireItemAvailable("slot",    &m_slot);
  getApplicationInfoSpace()->fireItemAvailable("amc13ConfigParams",    &m_amc13Params);

  //initialize the AMC13Manager application objects
  DEBUG("connecting to the AMC13ManagerWeb interface");
  p_gemWebInterface = new gem::hw::amc13::AMC13ManagerWeb(this);
  //p_gemMonitor      = new gem::hw::amc13::AMC13HwMonitor(this);
  DEBUG("done");

  DEBUG("executing preInit for AMC13Manager");
  preInit();
  DEBUG("done");
  getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemhardware/html/images/amc13/AMC13Manager.png");
}

gem::hw::amc13::AMC13Manager::~AMC13Manager() {
  
}

// This is the callback used for handling xdata:Event objects
void gem::hw::amc13::AMC13Manager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("AMC13Manager::actionPerformed() setDefaultValues" << 
                    "Default configuration values have been loaded from xml profile");
    //p_gemMonitor->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::amc13::AMC13Manager::preInit()
  throw (gem::base::exception::Exception)
{
  std::string addressBase  = "${AMC13_ADDRESS_TABLE_PATH}/";
  std::string connection   = "${BUILD_HOME}/gemdaq-testing/gemhardware/xml/amc13/connectionSN170_ch.xml";
  std::string cardname = "gem.shelf01.amc13.";
  try {
    gem::utils::LockGuard<gem::utils::Lock> guardedLock(deviceLock_);
    amc13Device_ = new ::amc13::AMC13(connection,cardname+"T1",cardname+"T2");
  } catch (uhal::exception::exception & e) {
    ERROR(std::string("AMC13::AMC13() failed, caught uhal::exception:") + e.what() );
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create class: ")+e.what());
  } catch (std::exception& e) {
    ERROR(std::string("AMC13::AMC13() failed, caught std::exception:") + e.what() );
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create class: ")+e.what());
  } catch (...) {
    ERROR(std::string("AMC13::AMC13() failed, caught ...") );
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create AMC13 connection"));
  }

  DEBUG("finished with AMC13::AMC13()");

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
  DEBUG("finished with AMC13Manager::preInit()");
}

void gem::hw::amc13::AMC13Manager::init()
  throw (gem::base::exception::Exception)
{
  gem::base::GEMFSMApplication::init();

  DEBUG("Entering gem::hw::amc13::AMC13Manager::init()");
  if (amc13Device_==0) return;
  
  //have to set up the initialization of the AMC13 for the desired running situation
  //possibilities are TTC/TCDS mode, DAQ link, local trigger scheme
  //lock the access
  
  //enable daq link (if sfp mask is non-zero
  //enable sfp outputs based on mask configuration
  
  //ignore AMC tts state per mask
  
  //enable specified AMCs
  
  
  
  //unlock the access
}

void gem::hw::amc13::AMC13Manager::enable()
  throw (gem::base::exception::Exception) {
  DEBUG("Entering gem::hw::amc13::AMC13Manager::enable()");
  //gem::base::GEMFSMApplication::enable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(deviceLock_);
  amc13Device_->startRun();
}

void gem::hw::amc13::AMC13Manager::disable()
  throw (gem::base::exception::Exception) {
  DEBUG("Entering gem::hw::amc13::AMC13Manager::disable()");
  //gem::base::GEMFSMApplication::disable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(deviceLock_);
  amc13Device_->endRun();
}

::amc13::Status* gem::hw::amc13::AMC13Manager::getHTMLStatus() const {
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(deviceLock_);
  return amc13Device_->getStatus(); 
}

//state transitions
void gem::hw::amc13::AMC13Manager::initializeAction() throw (gem::hw::amc13::exception::Exception) {}
void gem::hw::amc13::AMC13Manager::configureAction()  throw (gem::hw::amc13::exception::Exception) {}
void gem::hw::amc13::AMC13Manager::startAction()      throw (gem::hw::amc13::exception::Exception) {}
void gem::hw::amc13::AMC13Manager::pauseAction()      throw (gem::hw::amc13::exception::Exception) {}
void gem::hw::amc13::AMC13Manager::resumeAction()     throw (gem::hw::amc13::exception::Exception) {}
void gem::hw::amc13::AMC13Manager::stopAction()       throw (gem::hw::amc13::exception::Exception) {}
void gem::hw::amc13::AMC13Manager::haltAction()       throw (gem::hw::amc13::exception::Exception) {}
void gem::hw::amc13::AMC13Manager::resetAction()      throw (gem::hw::amc13::exception::Exception) {}
//void gem::hw::amc13::AMC13Manager::noAction()         throw (gem::hw::amc13::exception::Exception) {}

void gem::hw::amc13::AMC13Manager::failAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}

void gem::hw::amc13::AMC13Manager::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}
