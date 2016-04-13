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
#include "xcept/tools.h"
#include "amc13/AMC13.hh"
#include "amc13/Status.hh"

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPConstants.h"
#include "xoap/SOAPBody.h"
#include "xoap/Method.h"
#include "xoap/AttachmentPart.h"


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

  bag->addField("EnableCalPulse",      &enableCalpulse );

  bag->addField("InternalPeriodicPeriod", &internalPeriodicPeriod );
  bag->addField("L1Amode", &l1Amode );
  bag->addField("L1Arules", &l1Arules );
  bag->addField("L1Aburst", &l1Aburst );

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
  p_amc13(NULL),
  is_initialized_ (false),
  is_configured_  (false),
  is_running_     (false),
  is_paused_     (false),
  is_resumed_     (false)

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

  xoap::bind(this, &gem::hw::amc13::AMC13Manager::callbackinitialize, "CallBackInitialize", XDAQ_NS_URI );   
  xoap::bind(this, &gem::hw::amc13::AMC13Manager::callbackconfigure, "CallBackConfigure", XDAQ_NS_URI );   
  xoap::bind(this, &gem::hw::amc13::AMC13Manager::callbackstart, "CallBackStart", XDAQ_NS_URI );   
  xoap::bind(this, &gem::hw::amc13::AMC13Manager::callbackpause, "CallBackPause", XDAQ_NS_URI );   
  xoap::bind(this, &gem::hw::amc13::AMC13Manager::callbackresume, "CallBackResume", XDAQ_NS_URI );   
  xoap::bind(this, &gem::hw::amc13::AMC13Manager::callbackstop, "CallBackStop", XDAQ_NS_URI );   

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

  m_enableCalpulse     = m_amc13Params.bag.enableCalpulse.value_;

  m_internalPeriodicPeriod = m_amc13Params.bag.internalPeriodicPeriod.value_;
  m_L1Amode            = m_amc13Params.bag.l1Amode.value_;
  m_L1Arules           = m_amc13Params.bag.l1Arules.value_;
  m_L1Aburst           = m_amc13Params.bag.l1Aburst.value_;
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
  //  usleep(500);

  // Use local TTC signal if config doc says so
  p_amc13->localTtcSignalEnable(m_enableLocalTTC);

  // Enable Monitor Buffer Backpressure if config doc says so
  p_amc13->monBufBackPressEnable(m_monBackPressEnable);

  // m_dtc->configurePrescale(1,m_preScaleFactNumOfZeros);
  p_amc13->configurePrescale(0, m_prescaleFactor);

  // set the FED id
  p_amc13->setFEDid(m_fedID);

  // reset the T1
  p_amc13->reset(::amc13::AMC13::T1);

  // reset the T1 counters
  p_amc13->resetCounters();

  // Setting L1A if config doc says so
  if (m_enableLocalL1A) p_amc13->configureLocalL1A(m_enableLocalL1A,m_L1Amode,m_L1Aburst,m_internalPeriodicPeriod,m_L1Arules);

  
  int chan = 1;
  uint8_t cmd = 0x14;
  uint16_t bx = 0x1;
  uint16_t prescale =0x1;
  bool repeat = true;

  if (m_enableCalpulse){
  p_amc13->configureBGOShort( chan, cmd, bx, prescale, repeat);
  p_amc13->getBGOConfig(chan);
  } 

  try{
    std::string state =  getCurrentState();
    is_initialized_ = true;
    INFO("STATE AMC13MAnager Start: " << state << "  " << is_initialized_);
    if(state == "Initial"){
    }
  }// end try
  catch (const xgi::exception::Exception& e) {
    INFO("Something went wrong initializing AMC13: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }

  

  //unlock the access
}

void gem::hw::amc13::AMC13Manager::configureAction()
  throw (gem::hw::amc13::exception::Exception)
{

  std::string state =  getCurrentState();
  is_configured_ = true;
  INFO("STATE AMC13MAnager Start: " << state << "  " << is_configured_);

}

void gem::hw::amc13::AMC13Manager::startAction()
  throw (gem::hw::amc13::exception::Exception)
{
  DEBUG("Entering gem::hw::amc13::AMC13Manager::startAction()");

  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);

  p_amc13->reset(::amc13::AMC13::T1);
  p_amc13->startRun();

  p_amc13->localTtcSignalEnable(m_enableLocalL1A);
  p_amc13->enableLocalL1A(m_enableLocalL1A);

  if (m_enableLocalL1A) p_amc13->startContinuousL1A();

  int chan = 1;
  if (m_enableCalpulse){
    p_amc13->enableBGO(chan);
    p_amc13->sendBGO();
  }

  INFO("AFTER startcontinousl1a" << p_amc13->read(::amc13::AMC13::T1, "STATUS.LOCAL_TRIG.CONTINUOUS_ON"));

  std::string state =  getCurrentState();
  INFO("STATE AMC13MAnager Start: " << state << "  " << is_running_);

}

void gem::hw::amc13::AMC13Manager::pauseAction()
  throw (gem::hw::amc13::exception::Exception)
{
  //what does pause mean here?
  //if local triggers are enabled, do we have a separate trigger application?
  //we can just disable them here maybe?
  if (m_enableLocalL1A) p_amc13->stopContinuousL1A();
  //  usleep(500);
  std::string state =  getCurrentState();
  is_paused_ = true;
  INFO("STATE AMC13MAnager Start: " << state << "  " << is_paused_);
}

void gem::hw::amc13::AMC13Manager::resumeAction()
  throw (gem::hw::amc13::exception::Exception)
{
  //undo the actions taken in pauseAction
  if (m_enableLocalL1A) p_amc13->startContinuousL1A();

  std::string state =  getCurrentState();
  is_resumed_ = true;
  INFO("STATE AMC13MAnager Start: " << state << "  " << is_resumed_);
  //  usleep(500);
}

void gem::hw::amc13::AMC13Manager::stopAction()
  throw (gem::hw::amc13::exception::Exception)
{
  DEBUG("Entering gem::hw::amc13::AMC13Manager::stopAction()");
  //gem::base::GEMFSMApplication::disable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);
  if (m_enableLocalL1A) p_amc13->stopContinuousL1A();
  if (m_enableCalpulse) p_amc13->disableBGO(1);

  usleep(500);
  p_amc13->endRun();

  std::string state =  getCurrentState();
  INFO("STATE AMC13MAnager: " << state);
}

void gem::hw::amc13::AMC13Manager::haltAction()
  throw (gem::hw::amc13::exception::Exception)
{
  //what is necessary for a halt on the AMC13?
  usleep(500);
std::string state =  getCurrentState();
  INFO("STATE AMC13MAnager: " << state);
}

void gem::hw::amc13::AMC13Manager::resetAction()
  throw (gem::hw::amc13::exception::Exception)
{
  //what is necessary for a reset on the AMC13?
  DEBUG("Entering gem::hw::amc13::AMC13Manager::resetAction()");
  if (p_amc13!=0) delete p_amc13;
  p_amc13=0;
  usleep(500);
  std::string state =  getCurrentState();
  INFO("STATE AMC13MAnager: " << state);
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


xoap::MessageReference gem::hw::amc13::AMC13Manager::callbackstart(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(),"SOAP Message Received--Starting AMC13---------------");

  LOG4CPLUS_INFO(this->getApplicationLogger(),"AMC13State before " <<  getCurrentState());
  fireEvent("Start");

  // reply to caller                                                                 
  xoap::MessageReference reply         = xoap::createMessage();
  xoap::SOAPEnvelope     envelope      = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName         responseName  = envelope.createName( "onMessageResponse", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement  e             = envelope.getBody().addBodyElement ( responseName );

   while( getCurrentState() != "Running"){
     sleep(0.001);
     is_running_ = false;
   }
   is_running_ = true;

   LOG4CPLUS_INFO(this->getApplicationLogger(),"New AMC13State " <<  getCurrentState());
   return reply;
   
}
