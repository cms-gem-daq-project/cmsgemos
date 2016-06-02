/**
 * class: AMC13Manager
 * description: Manager application for AMC13 cards
 *              structure borrowed from TCDS core, with nods to HCAL (DTCManager)
 * author: J. Sturdy
 * date:
 */

#include "amc13/AMC13.hh"
#include "amc13/Status.hh"

#include "gem/hw/amc13/AMC13ManagerWeb.h"
#include "gem/hw/amc13/AMC13Manager.h"

//#include "gem/hw/amc13/exception/Exception.h"
#include "gem/utils/soap/GEMSOAPToolBox.h"
#include "gem/utils/exception/Exception.h"

XDAQ_INSTANTIATOR_IMPL(gem::hw::amc13::AMC13Manager);

gem::hw::amc13::AMC13Manager::BGOInfo::BGOInfo()
{
  channel  = -1;  // want this to somehow automatically get the position in the struct
  cmd      = 0x0;
  bx       = 0x0;
  prescale = 0x0;
  repeat   = false;
  isLong   = false;
}

void gem::hw::amc13::AMC13Manager::BGOInfo::registerFields(xdata::Bag<BGOInfo> *bgobag)
{
  bgobag->addField("BGOChannel",     &channel );
  bgobag->addField("BGOcmd",         &cmd     );
  bgobag->addField("BGObx",          &bx      );
  bgobag->addField("BGOprescale",    &prescale);
  bgobag->addField("BGOrepeat",      &repeat  );
  bgobag->addField("BGOlong",        &isLong  );
}

gem::hw::amc13::AMC13Manager::L1AInfo::L1AInfo()
{
  enableLocalL1A         = false;
  internalPeriodicPeriod = 1;
  l1Amode                = 0;
  l1Arules               = 0;
  l1Aburst               = 1;
  sendl1ATriburst        = false;
  sendl1ATriburst        = false;
}

void gem::hw::amc13::AMC13Manager::L1AInfo::registerFields(xdata::Bag<L1AInfo> *l1Abag)
{
  l1Abag->addField("EnableLocalL1A",         &enableLocalL1A );
  l1Abag->addField("InternalPeriodicPeriod", &internalPeriodicPeriod );
  l1Abag->addField("L1Amode",                &l1Amode  );
  l1Abag->addField("L1Arules",               &l1Arules );
  l1Abag->addField("L1Aburst",               &l1Aburst );
  l1Abag->addField("sendL1ATriburst",        &sendl1ATriburst );
  l1Abag->addField("startL1ATricont",        &startl1ATricont );
}

void gem::hw::amc13::AMC13Manager::AMC13Info::registerFields(xdata::Bag<AMC13Info> *bag)
{

  bag->addField("ConnectionFile", &connectionFile);
  bag->addField("CardName",       &cardName);

  bag->addField("AMCInputEnableList", &amcInputEnableList);
  bag->addField("AMCIgnoreTTSList",   &amcIgnoreTTSList  );

  bag->addField("EnableDAQLink",       &enableDAQLink  );
  bag->addField("EnableFakeData",      &enableFakeData );
  bag->addField("MonitorBackPressure", &monBackPressure);
  bag->addField("EnableLocalTTC",      &enableLocalTTC );

  bag->addField("LocalTriggerConfig",  &localTriggerConfig );

  bag->addField("PrescaleFactor", &prescaleFactor);
  bag->addField("BCOffset",       &bcOffset      );
  bag->addField("BGOConfig",      &bgoConfig     );

  bag->addField("FEDID",    &fedID   );
  bag->addField("SFPMask",  &sfpMask );
  bag->addField("SlotMask", &slotMask);

  //bag->addField("LocalL1AMask", &localL1AMask);
}

gem::hw::amc13::AMC13Manager::AMC13Manager(xdaq::ApplicationStub* stub)
  throw (xdaq::exception::Exception) :
  gem::base::GEMFSMApplication(stub),
  m_amc13Lock(toolbox::BSem::FULL, true),
  p_amc13(NULL)
{
  m_bgoConfig.setSize(4);

  m_crateID = -1;
  m_slot    = 13;

  p_appInfoSpace->fireItemAvailable("crateID",          &m_crateID    );
  p_appInfoSpace->fireItemAvailable("slot",             &m_slot       );
  p_appInfoSpace->fireItemAvailable("amc13ConfigParams",&m_amc13Params);

  uhal::setLogLevelTo(uhal::Error);

  //initialize the AMC13Manager application objects
  DEBUG("AMC13Manager::connecting to the AMC13ManagerWeb interface");
  p_gemWebInterface = new gem::hw::amc13::AMC13ManagerWeb(this);
  //p_gemMonitor      = new gem::hw::amc13::AMC13HwMonitor(this);
  DEBUG("AMC13Manager::done");

  //DEBUG("AMC13Manager::executing preInit for AMC13Manager");
  //preInit();
  //DEBUG("AMC13Manager::done");
  p_appDescriptor->setAttribute("icon","/gemdaq/gemhardware/html/images/amc13/AMC13Manager.png");

  xoap::bind(this, &gem::hw::amc13::AMC13Manager::sendTriggerBurst,"sendtriggerburst", XDAQ_NS_URI );   
  xoap::bind(this, &gem::hw::amc13::AMC13Manager::enableTriggers,  "enableTriggers",   XDAQ_NS_URI );   
  xoap::bind(this, &gem::hw::amc13::AMC13Manager::disableTriggers, "disableTriggers",  XDAQ_NS_URI );   
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
  m_cardName           = m_amc13Params.bag.cardName.value_;
  m_amcInputEnableList = m_amc13Params.bag.amcInputEnableList.value_;
  m_amcIgnoreTTSList   = m_amc13Params.bag.amcIgnoreTTSList.value_;
  m_enableDAQLink      = m_amc13Params.bag.enableDAQLink.value_;
  m_enableFakeData     = m_amc13Params.bag.enableFakeData.value_;
  m_monBackPressEnable = m_amc13Params.bag.monBackPressure.value_;
  m_enableLocalTTC     = m_amc13Params.bag.enableLocalTTC.value_;

  m_enableLocalL1A         = m_localTriggerConfig.bag.enableLocalL1A.value_;
  m_internalPeriodicPeriod = m_localTriggerConfig.bag.internalPeriodicPeriod.value_;
  m_L1Amode                = m_localTriggerConfig.bag.l1Amode.value_;
  m_L1Arules               = m_localTriggerConfig.bag.l1Arules.value_;
  m_L1Aburst               = m_localTriggerConfig.bag.l1Aburst.value_;
  m_sendL1ATriburst        = m_localTriggerConfig.bag.sendl1ATriburst.value_;
  m_startL1ATricont        = m_localTriggerConfig.bag.startl1ATricont.value_;

  DEBUG("AMC13Manager::actionPerformed BGO channels "
        << m_amc13Params.bag.bgoConfig.size());

  for (auto bconf = m_amc13Params.bag.bgoConfig.begin(); bconf != m_amc13Params.bag.bgoConfig.end(); ++bconf)
    if (bconf->bag.channel > -1)
      m_bgoConfig.at(bconf->bag.channel) = *bconf;

  if (m_bgoConfig.size() > 0) {
    m_bgoChannel         = 0;
    m_bgoCMD             = m_bgoConfig.at(0).bag.cmd.value_;
    m_bgoBX              = m_bgoConfig.at(0).bag.bx.value_;
    m_bgoPrescale        = m_bgoConfig.at(0).bag.prescale.value_;
    m_bgoRepeat          = m_bgoConfig.at(0).bag.repeat.value_;
    m_bgoIsLong          = m_bgoConfig.at(0).bag.isLong.value_;
  }

  m_prescaleFactor     = m_amc13Params.bag.prescaleFactor.value_;
  m_bcOffset           = m_amc13Params.bag.bcOffset.value_;
  m_fedID              = m_amc13Params.bag.fedID.value_;
  m_sfpMask            = m_amc13Params.bag.sfpMask.value_;
  m_slotMask           = m_amc13Params.bag.slotMask.value_;
  //m_localL1AMask       = m_amc13Params.bag.localL1AMask.value_;

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
  std::string connection  = "${GEM_ADDRESS_TABLE_PATH}/"+m_connectionFile;
  //std::string cardname    = toolbox::toString("gem.shelf%02d.amc13",m_crateID);
  std::string cardname    = m_cardName;

  try {
    gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);
    DEBUG("Trying to create connection to " << m_cardName << " in " << connection);
    p_amc13 = new ::amc13::AMC13(connection, cardname+".T1", cardname+".T2");
  } catch (uhal::exception::exception & e) {
    ERROR("AMC13Manager::AMC13::AMC13() failed, caught uhal::exception:" <<  e.what() );
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create class: ")+e.what());
  } catch (std::exception& e) {
    ERROR("AMC13Manager::AMC13::AMC13() failed, caught std::exception:" << e.what() );
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create class: ")+e.what());
  } catch (...) {
    ERROR("AMC13Manager::AMC13::AMC13() failed, caught ...");
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create AMC13 connection"));
  }

  DEBUG("AMC13Manager::finished with AMC13::AMC13()");

  try {
    gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);
    p_amc13->reset(::amc13::AMC13::T2);

    p_amc13->enableAllTTC();
  } catch (uhal::exception::exception & e) {
    ERROR("AMC13Manager::AMC13::AMC13() failed, caught uhal::exception " << e.what());
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Problem during preinit : ")+e.what());
  } catch (std::exception& e) {
    ERROR("AMC13Manager::AMC13::AMC13() failed, caught std::exception " << e.what());
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Problem during preinit : ")+e.what());
  }

  //equivalent to hcal init part
  if (p_amc13==0)
    return;

  //have to set up the initialization of the AMC13 for the desired running situation
  //possibilities are TTC/TCDS mode, DAQ link, local trigger scheme
  //lock the access
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);

  //enable daq link (if SFP mask is non-zero
  if (m_enableDAQLink) {
    DEBUG("Enabling DAQLink with settings: fake data:" << m_enableFakeData
          << ", sfpMask:" << m_sfpMask);
    p_amc13->fakeDataEnable(m_enableFakeData);
    p_amc13->daqLinkEnable(m_enableDAQLink);
    p_amc13->sfpOutputEnable(m_sfpMask);
  }
  //enable SFP outputs based on mask configuration

  //ignore AMC tts state per mask

  //enable specified AMCs
  m_slotMask = p_amc13->parseInputEnableList(m_amcInputEnableList,true);
  p_amc13->AMCInputEnable(m_slotMask);

  // Use local TTC signal if config doc says so
  
  p_amc13->localTtcSignalEnable(m_enableLocalTTC);

  // need to ensure that all BGO channels are disabled
  for (int bchan = 0; bchan < 4; ++bchan)
    p_amc13->disableBGO(bchan);

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
  //DEBUG("Looking at L1A history before configure");
  //p_amc13->getL1AHistory(4);
  //std::cout << p_amc13->getL1AHistory(4) << std::endl;

  //unlock the access
}

void gem::hw::amc13::AMC13Manager::configureAction()
  throw (gem::hw::amc13::exception::Exception)
{
  if (m_enableLocalL1A) {
    m_L1Aburst           = m_localTriggerConfig.bag.l1Aburst.value_;
    p_amc13->configureLocalL1A(m_enableLocalL1A, m_L1Amode, m_L1Aburst, m_internalPeriodicPeriod, m_L1Arules);
  }
  //DEBUG("Looking at L1A history after configure");
  //std::cout << p_amc13->getL1AHistory(4) << std::endl;

  if (m_enableLocalTTC) {
    DEBUG("AMC13Manager::configureAction configuring BGO channels "
          << m_bgoConfig.size());
    for (auto bchan = m_bgoConfig.begin(); bchan != m_bgoConfig.end(); ++bchan) {
      DEBUG("AMC13Manager::configureAction channel "
            << bchan->bag.channel.value_);
      if (bchan->bag.channel.value_ > -1) {
        if (bchan->bag.isLong.value_)
          p_amc13->configureBGOLong(bchan->bag.channel.value_, bchan->bag.cmd.value_, bchan->bag.bx.value_,
                                    bchan->bag.prescale.value_, bchan->bag.repeat.value_);
        else
          p_amc13->configureBGOShort(bchan->bag.channel.value_, bchan->bag.cmd.value_, bchan->bag.bx.value_,
                                     bchan->bag.prescale.value_, bchan->bag.repeat.value_);

        p_amc13->getBGOConfig(bchan->bag.channel.value_);
      }
    }
  }

  INFO("AMC13 Configured L1ABurst = " << m_L1Aburst);
  //set the settings from the config options
  usleep(500); // just for testing the timing of different applications
}

void gem::hw::amc13::AMC13Manager::startAction()
  throw (gem::hw::amc13::exception::Exception)
{
  DEBUG("AMC13Manager::Entering gem::hw::amc13::AMC13Manager::startAction()");
  //gem::base::GEMFSMApplication::enable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);
  p_amc13->reset(::amc13::AMC13::T1);
  usleep(500);

  p_amc13->reset(::amc13::AMC13::T1);

  p_amc13->startRun();
  INFO("AMC13 Configured L1ABurst = " << m_L1Aburst);

  INFO("AMC13 Configured L1ABurst = " << m_L1Aburst);

  
  if (m_enableLocalL1A && m_startL1ATricont) {
    //    p_amc13->localTtcSignalEnable(m_enableLocalL1A);
    p_amc13->enableLocalL1A(m_enableLocalL1A);
    //    p_amc13->startContinuousL1A();
  }
  if (m_enableLocalTTC) {
    for (auto bchan = m_bgoConfig.begin(); bchan != m_bgoConfig.end(); ++bchan)
      if (bchan->bag.channel.value_ > -1)
        p_amc13->enableBGO(bchan->bag.channel.value_);
    p_amc13->sendBGO();
  }
}

void gem::hw::amc13::AMC13Manager::pauseAction()
  throw (gem::hw::amc13::exception::Exception)
{
  //what does pause mean here?
  //if local triggers are enabled, do we have a separate trigger application?
  //we can just disable them here maybe?
  if (m_enableLocalL1A)
    p_amc13->stopContinuousL1A();

  if (m_enableLocalTTC)
    for (auto bchan = m_bgoConfig.begin(); bchan != m_bgoConfig.end(); ++bchan)
      if (bchan->bag.channel.value_ > -1)
        p_amc13->disableBGO(bchan->bag.channel.value_);

  usleep(500);
}

void gem::hw::amc13::AMC13Manager::resumeAction()
  throw (gem::hw::amc13::exception::Exception)
{
  //undo the actions taken in pauseAction
  if (m_enableLocalL1A)
    p_amc13->startContinuousL1A();

  if (m_enableLocalTTC) {
    for (auto bchan = m_bgoConfig.begin(); bchan != m_bgoConfig.end(); ++bchan)
      if (bchan->bag.channel.value_ > -1)
        p_amc13->enableBGO(bchan->bag.channel.value_);

    p_amc13->sendBGO();
  }

  usleep(500);
}

void gem::hw::amc13::AMC13Manager::stopAction()
  throw (gem::hw::amc13::exception::Exception)
{
  DEBUG("AMC13Manager::Entering gem::hw::amc13::AMC13Manager::stopAction()");
  //gem::base::GEMFSMApplication::disable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);

  if (m_enableLocalL1A)
    p_amc13->stopContinuousL1A();

  if (m_enableLocalTTC)
    for (auto bchan = m_bgoConfig.begin(); bchan != m_bgoConfig.end(); ++bchan)
      if (bchan->bag.channel.value_ > -1)
        p_amc13->disableBGO(bchan->bag.channel.value_);

  // need to ensure that all BGO channels are disabled, rather than just the ones in the config
  for (int bchan = 0; bchan < 4; ++bchan)
    p_amc13->disableBGO(bchan);

  usleep(500);
  p_amc13->endRun();
}

void gem::hw::amc13::AMC13Manager::haltAction()
  throw (gem::hw::amc13::exception::Exception)
{
  //what is necessary for a halt on the AMC13?
  usleep(500); // just for testing the timing of different applications
}

void gem::hw::amc13::AMC13Manager::resetAction()
  throw (gem::hw::amc13::exception::Exception)
{
  //what is necessary for a reset on the AMC13?
  DEBUG("Entering gem::hw::amc13::AMC13Manager::resetAction()");
  if (p_amc13!=0) delete p_amc13;
  p_amc13 = 0;
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

//void gem::hw::amc13::AMC13Manager::sendTriggerBurst()
//  throw (gem::hw::amc13::exception::Exception)

xoap::MessageReference gem::hw::amc13::AMC13Manager::sendTriggerBurst(xoap::MessageReference msg)
  throw (xoap::exception::Exception)
{
  //set to send a burst of trigger
  INFO("Entering gem::hw::amc13::AMC13Manager::sendTriggerBurst()");

  if (msg.isNull()) {
    XCEPT_RAISE(xoap::exception::Exception,"Null message received!");
  }
  
  std::string commandName = "sendTriggerBurst";

  try {
    if (m_enableLocalL1A &&  m_sendL1ATriburst) {
      //      p_amc13->localTtcgSignalEnable(m_enableLocalL1A);
      //p_amc13->enableLocalL1A(m_enableLocalL1A);
      p_amc13->sendL1ABurst();
    }
  } catch(xoap::exception::Exception& err) {
    std::string msgBase     = toolbox::toString("Unable to extract command from SOAP message");
    std::string faultString = toolbox::toString("%s failed", commandName.c_str());
    std::string faultCode   = "Client";
    std::string detail      = toolbox::toString("%s: %s.",
                                                msgBase.c_str(),
                                                err.message().c_str());
    std::string faultActor = this->getFullURL();
    xoap::MessageReference reply =
      gem::utils::soap::GEMSOAPToolBox::makeSOAPFaultReply(faultString, faultCode, detail, faultActor);
    return reply;
  }
  try {
    INFO("AMC13Manager::sendTriggerBurst command " << commandName << " succeeded ");
    return
      gem::utils::soap::GEMSOAPToolBox::makeSOAPReply(commandName, "SentTriggers");
  } catch(xcept::Exception& err) {
    std::string msgBase = toolbox::toString("Failed to create SOAP reply for command '%s'",
                                            commandName.c_str());
    ERROR(toolbox::toString("%s: %s.", msgBase.c_str(), xcept::stdformat_exception(err).c_str()));
    XCEPT_DECLARE_NESTED(gem::base::utils::exception::SoftwareProblem,
                         top, toolbox::toString("%s.",msgBase.c_str()), err);
    this->notifyQualified("error", top);

    XCEPT_RETHROW(xoap::exception::Exception, msgBase, err);
  }
  XCEPT_RAISE(xoap::exception::Exception,"command not found");
}


xoap::MessageReference gem::hw::amc13::AMC13Manager::enableTriggers(xoap::MessageReference msg)
  throw (xoap::exception::Exception)
{
  DEBUG("AMC13Manager::enableTriggers");
  //gem::base::GEMFSMApplication::disable();

  std::string commandName = "enableTriggers";

  if (!p_amc13) {
    std::string msgBase = toolbox::toString("Failed to create SOAP reply for command '%s', AMC13 not yet connected",
                                            commandName.c_str());
    ERROR(toolbox::toString("%s", msgBase.c_str()));
    return
      gem::utils::soap::GEMSOAPToolBox::makeSOAPReply(commandName, "Failed");
  }
  
  if(!m_startL1ATricont){
    if (m_enableLocalL1A)
      p_amc13->enableLocalL1A(true);
    else
      //disable localL1A generator in order to enable the CSC triggers
      p_amc13->enableLocalL1A(false);
  }
  
  if (m_enableLocalL1A && m_startL1ATricont) {
    p_amc13->startContinuousL1A();
  }
  
  try {
    INFO("AMC13Manager::enableTriggers command " << commandName << " succeeded ");
    return
      gem::utils::soap::GEMSOAPToolBox::makeSOAPReply(commandName, "SentTriggers");
  } catch(xcept::Exception& err) {
    std::string msgBase = toolbox::toString("Failed to create SOAP reply for command '%s'",
                                            commandName.c_str());
    ERROR(toolbox::toString("%s: %s.", msgBase.c_str(), xcept::stdformat_exception(err).c_str()));
    XCEPT_DECLARE_NESTED(gem::base::utils::exception::SoftwareProblem,
                         top, toolbox::toString("%s.",msgBase.c_str()), err);
    this->notifyQualified("error", top);
    
    XCEPT_RETHROW(xoap::exception::Exception, msgBase, err);
  }  
  XCEPT_RAISE(xoap::exception::Exception,"command not found");
}

xoap::MessageReference gem::hw::amc13::AMC13Manager::disableTriggers(xoap::MessageReference msg)
  throw (xoap::exception::Exception)
{
  DEBUG("AMC13Manager::disableTriggers");
  //gem::base::GEMFSMApplication::disable();
  std::string commandName = "disableTriggers";

  if (!p_amc13) {
    std::string msgBase = toolbox::toString("Failed to create SOAP reply for command '%s', AMC13 not yet connected",
                                            commandName.c_str());
    ERROR(toolbox::toString("%s", msgBase.c_str()));
    return
      gem::utils::soap::GEMSOAPToolBox::makeSOAPReply(commandName, "Failed");
  }
  
  if(!m_startL1ATricont){
    if (m_enableLocalL1A)
    p_amc13->enableLocalL1A(false);
    else
      //disable localL1A generator in order to enable the CSC triggers
      p_amc13->enableLocalL1A(true);
  }
  
  if (m_enableLocalL1A && m_startL1ATricont) {
    p_amc13->stopContinuousL1A();
  }
  
  try {
    INFO("AMC13Manager::disableTriggers " << commandName << " succeeded ");
    return
      gem::utils::soap::GEMSOAPToolBox::makeSOAPReply(commandName, "SentTriggers");
  } catch(xcept::Exception& err) {
    std::string msgBase = toolbox::toString("Failed to create SOAP reply for command '%s'",
                                            commandName.c_str());
    ERROR(toolbox::toString("%s: %s.", msgBase.c_str(), xcept::stdformat_exception(err).c_str()));
    XCEPT_DECLARE_NESTED(gem::base::utils::exception::SoftwareProblem,
                         top, toolbox::toString("%s.",msgBase.c_str()), err);
    this->notifyQualified("error", top);
    
    XCEPT_RETHROW(xoap::exception::Exception, msgBase, err);
  }  
  XCEPT_RAISE(xoap::exception::Exception,"command not found");
}

