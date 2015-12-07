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

void gem::hw::amc13::AMC13Manager::AMC13Info::registerFields(xdata::Bag<AMC13Info> *bag)
{

  bag->addField("ConnectionFile", &connectionFile);

  bag->addField("AMCInputEnableList", &amcInputEnableList);
  bag->addField("AMCIgnoreTTSList",   &amcIgnoreTTSList  );

  bag->addField("EnableDAQLink",       &enableDAQLink  );
  bag->addField("EnableFakeData",      &enableFakeData );
  bag->addField("MonitorBackPressure", &monBackPressure);
  bag->addField("EnableLocalTTC",      &enableLocalTTC );
  bag->addField("EnableLocalL1A",      &enableLocalL1A );

  bag->addField("PrescaleFactor", &prescaleFactor);
  bag->addField("BCOffset",       &bcOffset      );

  bag->addField("FEDID",    &fedID   );
  bag->addField("SFPMask",  &sfpMask );
  bag->addField("SlotMask", &slotMask);

  bag->addField("LocalL1AMask", &localL1AMask);
}

gem::hw::amc13::AMC13Manager::AMC13Manager(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub),
  m_amc13Lock(toolbox::BSem::FULL, true),
  p_amc13(NULL)
{
  m_crateID = -1;
  m_slot    = 13;
  
  p_appInfoSpace->fireItemAvailable("crateID",          &m_crateID    );
  p_appInfoSpace->fireItemAvailable("slot",             &m_slot       );
  p_appInfoSpace->fireItemAvailable("amc13ConfigParams",&m_amc13Params);

  uhal::setLogLevelTo(uhal::Error);

  //initialize the AMC13Manager application objects
  DEBUG("connecting to the AMC13ManagerWeb interface");
  p_gemWebInterface = new gem::hw::amc13::AMC13ManagerWeb(this);
  //p_gemMonitor      = new gem::hw::amc13::AMC13HwMonitor(this);
  DEBUG("done");

  //DEBUG("executing preInit for AMC13Manager");
  //preInit();
  //DEBUG("done");
  p_appDescriptor->setAttribute("icon","/gemdaq/gemhardware/html/images/amc13/AMC13Manager.png");
}

gem::hw::amc13::AMC13Manager::~AMC13Manager() {
  if (p_amc13)
    delete p_amc13;
  p_amc13 = NULL;
}

// This is the callback used for handling xdata:Event objects
void gem::hw::amc13::AMC13Manager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("AMC13Manager::actionPerformed() setDefaultValues" << 
          "Default configuration values have been loaded from xml profile");
    //p_gemMonitor->startMonitoring();
  }
  // update configuration variables
  m_connectionFile     = m_amc13Params.bag.connectionFile.value_;
  m_amcInputEnableList = m_amc13Params.bag.amcInputEnableList.value_;
  m_amcIgnoreTTSList   = m_amc13Params.bag.amcIgnoreTTSList.value_;
  m_enableDAQLink      = m_amc13Params.bag.enableDAQLink.value_;
  m_enableFakeData     = m_amc13Params.bag.enableFakeData.value_;
  m_monBackPressEnable = m_amc13Params.bag.monBackPressure.value_;
  m_enableLocalTTC     = m_amc13Params.bag.enableLocalTTC.value_;
  m_enableLocalL1A     = m_amc13Params.bag.enableLocalL1A.value_;
  m_prescaleFactor     = m_amc13Params.bag.prescaleFactor.value_;
  m_bcOffset           = m_amc13Params.bag.bcOffset.value_;
  m_fedID              = m_amc13Params.bag.fedID.value_;
  m_sfpMask            = m_amc13Params.bag.sfpMask.value_;
  m_slotMask           = m_amc13Params.bag.slotMask.value_;
  m_localL1AMask       = m_amc13Params.bag.localL1AMask.value_;
  
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::amc13::AMC13Manager::init()
{
}

::amc13::Status* gem::hw::amc13::AMC13Manager::getHTMLStatus() const {
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);
  return p_amc13->getStatus(); 
}

//state transitions
void gem::hw::amc13::AMC13Manager::initializeAction()
  throw (gem::hw::amc13::exception::Exception)
{
  //hcal has a pre-init, what is the reason to not do everything in initialize?
  //std::string addressBase = "${AMC13_ADDRESS_TABLE_PATH}/";
  //std::string connection  = "${BUILD_HOME}/gemdaq-testing/gemhardware/xml/amc13/"+m_connectionFile;
  std::string connection  = "${GEM_ADDRESS_TABLE_PATH}/"+m_connectionFile;
  std::string cardname    = "gem.shelf01.amc13";
  try {
    gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);
    p_amc13 = new ::amc13::AMC13(connection, cardname+".T1", cardname+".T2");
  } catch (uhal::exception::exception & e) {
    ERROR("AMC13::AMC13() failed, caught uhal::exception:" <<  e.what() );
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create class: ")+e.what());
  } catch (std::exception& e) {
    ERROR("AMC13::AMC13() failed, caught std::exception:" << e.what() );
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create class: ")+e.what());
  } catch (...) {
    ERROR("AMC13::AMC13() failed, caught ...");
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create AMC13 connection"));
  }

  DEBUG("finished with AMC13::AMC13()");

  try {
    gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);
    p_amc13->reset(::amc13::AMC13::T2);
    
    p_amc13->enableAllTTC();
  } catch (uhal::exception::exception & e) {
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Problem during preinit : ")+e.what());
  } catch (std::exception& e) {
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Problem during preinit : ")+e.what());
  }

  //equivalent to hcal init part
  if (p_amc13==0) return;
  
  //have to set up the initialization of the AMC13 for the desired running situation
  //possibilities are TTC/TCDS mode, DAQ link, local trigger scheme
  //lock the access
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);
  
  //enable daq link (if SFP mask is non-zero
  if (m_enableDAQLink) {
    p_amc13->fakeDataEnable(m_enableFakeData);
    p_amc13->daqLinkEnable(m_enableDAQLink);
    p_amc13->sfpOutputEnable(m_sfpMask);
    
  }
  //enable SFP outputs based on mask configuration
  
  //ignore AMC tts state per mask
  
  //enable specified AMCs
  m_slotMask = p_amc13->parseInputEnableList(m_amcInputEnableList,true);
  p_amc13->AMCInputEnable(m_slotMask);
  usleep(500);

  //unlock the access
}

void gem::hw::amc13::AMC13Manager::configureAction()
  throw (gem::hw::amc13::exception::Exception)
{
  //set the settings from the config options
  usleep(500);
}

void gem::hw::amc13::AMC13Manager::startAction()
  throw (gem::hw::amc13::exception::Exception)
{
  DEBUG("Entering gem::hw::amc13::AMC13Manager::startAction()");
  //gem::base::GEMFSMApplication::enable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);
  usleep(500);
  p_amc13->startRun();
}

void gem::hw::amc13::AMC13Manager::pauseAction()
  throw (gem::hw::amc13::exception::Exception)
{
  //what does pause mean here?
  //if local triggers are enabled, do we have a separate trigger application?
  //we can just disable them here maybe?
  usleep(500);
}

void gem::hw::amc13::AMC13Manager::resumeAction()
  throw (gem::hw::amc13::exception::Exception)
{
  //undo the actions taken in pauseAction
  usleep(500);
}

void gem::hw::amc13::AMC13Manager::stopAction()
  throw (gem::hw::amc13::exception::Exception)
{
  DEBUG("Entering gem::hw::amc13::AMC13Manager::stopAction()");
  //gem::base::GEMFSMApplication::disable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);
  usleep(500);
  p_amc13->endRun();
}

void gem::hw::amc13::AMC13Manager::haltAction()
  throw (gem::hw::amc13::exception::Exception)
{
  //what is necessary for a halt on the AMC13?
  usleep(500);
}

void gem::hw::amc13::AMC13Manager::resetAction()
  throw (gem::hw::amc13::exception::Exception)
{
  //what is necessary for a reset on the AMC13?
  usleep(500);
  //gem::base::GEMFSMApplication::resetAction();
}

/*These should maybe only be implemented in GEMFSMApplication,
  unless there is a reason to perform some special action
  for each hardware*/
void gem::hw::amc13::AMC13Manager::failAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}

void gem::hw::amc13::AMC13Manager::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}
