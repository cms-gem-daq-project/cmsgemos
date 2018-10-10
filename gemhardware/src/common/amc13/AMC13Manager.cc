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

// #include "gem/hw/amc13/exception/Exception.h"
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
  sendl1ATriburst        = false;  // CLEANME need to remove
  sendl1ATriburst        = false;  // CLEANME need to remove
  enableLEMO             = false;
}

void gem::hw::amc13::AMC13Manager::L1AInfo::registerFields(xdata::Bag<L1AInfo> *l1Abag)
{
  l1Abag->addField("EnableLocalL1A",         &enableLocalL1A );
  l1Abag->addField("InternalPeriodicPeriod", &internalPeriodicPeriod );
  l1Abag->addField("L1Amode",                &l1Amode  );
  l1Abag->addField("L1Arules",               &l1Arules );
  l1Abag->addField("L1Aburst",               &l1Aburst );
  l1Abag->addField("sendL1ATriburst",        &sendl1ATriburst );  // CLEANME need to remove
  l1Abag->addField("startL1ATricont",        &startl1ATricont );  // CLEANME need to remove
  l1Abag->addField("EnableLEMO",             &enableLEMO );
}

gem::hw::amc13::AMC13Manager::TTCInfo::TTCInfo()
{
  resyncCommand = 0x04;
  resyncMask    = 0x01;
  oc0Command    = 0x08;
  oc0Mask       = 0x01;
}

void gem::hw::amc13::AMC13Manager::TTCInfo::registerFields(xdata::Bag<TTCInfo> *ttcbag)
{
  ttcbag->addField("RESYNC_CMD",  &resyncCommand);
  ttcbag->addField("RESYNC_MASK", &resyncMask   );
  ttcbag->addField("OC0_CMD",     &oc0Command   );
  ttcbag->addField("OC0_MASK",    &oc0Mask      );
}

gem::hw::amc13::AMC13Manager::AMC13Info::AMC13Info()
{
  connectionFile     = "N/A";
  cardName           = "N/A";
  amcInputEnableList = "N/A";
  amcIgnoreTTSList   = "N/A";

  enableDAQLink   = false;
  enableFakeData  = false;
  monBackPressure = false;
  enableLocalTTC  = false;
  skipPLLReset    = true;

  // localTriggerConfig;

  // bgoConfig;

  prescaleFactor = 0;
  bcOffset       = 0;

  fedID    = 0;
  sfpMask  = 0;
  slotMask = 0;

  // localL1AMask;
}

void gem::hw::amc13::AMC13Manager::AMC13Info::registerFields(xdata::Bag<AMC13Info> *bag)
{

  bag->addField("ConnectionFile", &connectionFile);
  bag->addField("CardName",       &cardName);

  bag->addField("AMCInputEnableList", &amcInputEnableList);
  bag->addField("AMCIgnoreTTSList",   &amcIgnoreTTSList  );
  bag->addField("AMC13TTCConfig",     &amc13TTCConfig    );

  bag->addField("EnableDAQLink",       &enableDAQLink  );
  bag->addField("EnableFakeData",      &enableFakeData );
  bag->addField("MonitorBackPressure", &monBackPressure);
  bag->addField("EnableLocalTTC",      &enableLocalTTC );
  bag->addField("SkipPLLReset",        &skipPLLReset   );

  bag->addField("LocalTriggerConfig",  &localTriggerConfig );
  bag->addField("PrescaleFactor", &prescaleFactor);
  bag->addField("BCOffset",       &bcOffset      );
  bag->addField("BGOConfig",      &bgoConfig     );

  bag->addField("FEDID",    &fedID   );
  bag->addField("SFPMask",  &sfpMask );
  bag->addField("SlotMask", &slotMask);

  // bag->addField("LocalL1AMask", &localL1AMask);
}

gem::hw::amc13::AMC13Manager::AMC13Manager(xdaq::ApplicationStub* stub)
  throw (xdaq::exception::Exception) :
  gem::base::GEMFSMApplication(stub),
  m_amc13Lock(toolbox::BSem::FULL, true),
  p_timer(NULL)
{
  m_bgoConfig.setSize(4);

  m_crateID = -1;
  m_slot    = 13;

  p_appInfoSpace->fireItemAvailable("crateID",          &m_crateID    );
  p_appInfoSpace->fireItemAvailable("slot",             &m_slot       );
  p_appInfoSpace->fireItemAvailable("amc13ConfigParams",&m_amc13Params);

  xgi::bind(this, &AMC13Manager::setDisplayLevel, "setDisplayLevel");
  xgi::bind(this, &AMC13Manager::updateStatus,    "updateStatus"   );

  uhal::setLogLevelTo(uhal::Error);

  // initialize the AMC13Manager application objects
  CMSGEMOS_DEBUG("AMC13Manager::connecting to the AMC13ManagerWeb interface");
  p_gemWebInterface = new gem::hw::amc13::AMC13ManagerWeb(this);
  // p_gemMonitor      = new gem::hw::amc13::AMC13HwMonitor(this);
  CMSGEMOS_DEBUG("AMC13Manager::done");

  // CMSGEMOS_DEBUG("AMC13Manager::executing preInit for AMC13Manager");
  // preInit();
  // CMSGEMOS_DEBUG("AMC13Manager::done");
  p_appDescriptor->setAttribute("icon","/gemdaq/gemhardware/html/images/amc13/AMC13Manager.png");

  xoap::bind(this, &gem::hw::amc13::AMC13Manager::sendTriggerBurst,"sendtriggerburst", XDAQ_NS_URI );
  xoap::bind(this, &gem::hw::amc13::AMC13Manager::enableTriggers,  "enableTriggers",   XDAQ_NS_URI );
  xoap::bind(this, &gem::hw::amc13::AMC13Manager::disableTriggers, "disableTriggers",  XDAQ_NS_URI );

  p_timer = toolbox::task::getTimerFactory()->createTimer("AMC13ScanTriggerCounter");

  m_updatedL1ACount = 0;
}

gem::hw::amc13::AMC13Manager::~AMC13Manager() {
  CMSGEMOS_DEBUG("AMC13Manager::dtor called");
}

// This is the callback used for handling xdata:Event objects
void gem::hw::amc13::AMC13Manager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    CMSGEMOS_DEBUG("AMC13Manager::actionPerformed() setDefaultValues" <<
          "Default configuration values have been loaded from xml profile");
    // p_gemMonitor->startMonitoring();
    // update configuration variables
    m_connectionFile     = m_amc13Params.bag.connectionFile.value_;
    m_cardName           = m_amc13Params.bag.cardName.value_;
    m_amcInputEnableList = m_amc13Params.bag.amcInputEnableList.value_;
    m_amcIgnoreTTSList   = m_amc13Params.bag.amcIgnoreTTSList.value_;
    m_enableDAQLink      = m_amc13Params.bag.enableDAQLink.value_;
    m_enableFakeData     = m_amc13Params.bag.enableFakeData.value_;
    m_monBackPressEnable = m_amc13Params.bag.monBackPressure.value_;
    m_enableLocalTTC     = m_amc13Params.bag.enableLocalTTC.value_;
    m_skipPLLReset       = m_amc13Params.bag.skipPLLReset.value_;
    m_amc13TTCConfig     = m_amc13Params.bag.amc13TTCConfig;

    m_localTriggerConfig     = m_amc13Params.bag.localTriggerConfig;
    m_enableLocalL1A         = m_localTriggerConfig.bag.enableLocalL1A.value_;
    m_internalPeriodicPeriod = m_localTriggerConfig.bag.internalPeriodicPeriod.value_;
    m_L1Amode                = m_localTriggerConfig.bag.l1Amode.value_;
    m_L1Arules               = m_localTriggerConfig.bag.l1Arules.value_;
    m_L1Aburst               = m_localTriggerConfig.bag.l1Aburst.value_;
    m_sendL1ATriburst        = m_localTriggerConfig.bag.sendl1ATriburst.value_;
    m_startL1ATricont        = m_localTriggerConfig.bag.startl1ATricont.value_;
    m_enableLEMO             = m_localTriggerConfig.bag.enableLEMO.value_;

    CMSGEMOS_DEBUG("AMC13Manager::actionPerformed m_enableLocalL1A " << std::endl
         << m_localTriggerConfig.bag.toString());

    CMSGEMOS_DEBUG("AMC13Manager::actionPerformed BGO channels "
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
    // m_localL1AMask       = m_amc13Params.bag.localL1AMask.value_;
  }

  // item is changed, update it
  if (event.type() == "ItemChangedEvent" || event.type() == "urn:xdata-event:ItemChangedEvent") {
    CMSGEMOS_DEBUG("AMC13Manager::actionPerformed() ItemChangedEvent");
  }

  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::amc13::AMC13Manager::init()
{
}

gem::hw::amc13::amc13_status_ptr gem::hw::amc13::AMC13Manager::getHTMLStatus() const
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);
  return std::shared_ptr< ::amc13::Status>(p_amc13->getStatus());
}

void gem::hw::amc13::AMC13Manager::setDisplayLevel(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  dynamic_cast<AMC13ManagerWeb*>(p_gemWebInterface)->setDisplayLevel(in);
}

void gem::hw::amc13::AMC13Manager::updateStatus(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  dynamic_cast<AMC13ManagerWeb*>(p_gemWebInterface)->updateStatus(out);
}

//state transitions
void gem::hw::amc13::AMC13Manager::initializeAction()
{
  // hcal has a pre-init, what is the reason to not do everything in initialize?
  std::string connection  = "${GEM_ADDRESS_TABLE_PATH}/"+m_connectionFile;
  // std::string cardname    = toolbox::toString("gem.shelf%02d.amc13",m_crateID);
  std::string cardname    = m_cardName;

  CMSGEMOS_INFO("AMC13Manager::initializeAction m_amc13Params is:" << std::endl << m_amc13Params.bag.toString());

  try {
    gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);
    CMSGEMOS_DEBUG("Trying to create connection to " << m_cardName << " in " << connection);
    p_amc13 = std::make_shared< ::amc13::AMC13>(connection, cardname+".T1", cardname+".T2");
  } catch (::amc13::Exception::exBase & e) {
    std::stringstream msg;
    msg << "AMC13Manager::AMC13::AMC13() failed, caught amc13::Exception: " << e.what();
    CMSGEMOS_ERROR(msg.str());
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem, msg.str());
  } catch (uhal::exception::exception & e) {
    std::stringstream msg;
    msg << "AMC13Manager::AMC13::AMC13() failed, caught uhal::exception: " << e.what();
    CMSGEMOS_ERROR(msg.str());
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem, msg.str());
  } catch (std::exception& e) {
    std::stringstream msg;
    msg << "AMC13Manager::AMC13::AMC13() failed, caught std::exception: " << e.what();
    CMSGEMOS_ERROR(msg.str());
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem, msg.str());
  } catch (...) {
    std::stringstream msg;
    msg << "AMC13Manager::AMC13::AMC13() failed (unknown exception)";
    CMSGEMOS_ERROR(msg.str());
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem, msg.str());
  }

  CMSGEMOS_DEBUG("AMC13Manager::initializeAction finished with AMC13::AMC13()");

  try {
    gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);
    p_amc13->reset(::amc13::AMC13::T2);

    // enable specified AMCs
    m_slotMask = p_amc13->parseInputEnableList(m_amcInputEnableList,true);
    p_amc13->AMCInputEnable(m_slotMask);

    // Re-map AMC13 Resync and OCR to match TCDS and CTP7 expectation
    // NOT FINAL FIXME
    p_amc13->write(::amc13::AMC13::T1,"CONF.TTC.RESYNC.COMMAND", m_amc13TTCConfig.bag.resyncCommand.value_);
    p_amc13->write(::amc13::AMC13::T1,"CONF.TTC.RESYNC.MASK",    m_amc13TTCConfig.bag.resyncMask.value_);
    p_amc13->write(::amc13::AMC13::T1,"CONF.TTC.OCR_COMMAND",    m_amc13TTCConfig.bag.oc0Command.value_);
    p_amc13->write(::amc13::AMC13::T1,"CONF.TTC.OCR_MASK",       m_amc13TTCConfig.bag.oc0Mask.value_);

    p_amc13->enableAllTTC();
  } catch (uhal::exception::exception & e) {
    CMSGEMOS_ERROR("AMC13Manager::AMC13::AMC13() failed, caught uhal::exception " << e.what());
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Problem during preinit : ")+e.what());
  } catch (std::exception& e) {
    CMSGEMOS_ERROR("AMC13Manager::AMC13::AMC13() failed, caught std::exception " << e.what());
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Problem during preinit : ")+e.what());
  }

  // equivalent to hcal init part
  if (!p_amc13)
    return;

  // have to set up the initialization of the AMC13 for the desired running situation
  // possibilities are TTC/TCDS mode, DAQ link, local trigger scheme
  // lock the access
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);

  // enable daq link (if SFP mask is non-zero
  CMSGEMOS_DEBUG("AMC13Manager::initializeAction Enabling DAQLink with settings: fake data:" << m_enableFakeData
        << ", sfpMask:" << m_sfpMask);

  p_amc13->fakeDataEnable(m_enableFakeData);
  p_amc13->daqLinkEnable(m_enableDAQLink);

  // enable SFP outputs based on mask configuration
  p_amc13->sfpOutputEnable(m_sfpMask);

  // ignore AMC tts state per mask
  CMSGEMOS_INFO("AMC13Manager::initializeAction m_amcIgnoreTTSList " << m_amcIgnoreTTSList);
  m_ignoreAMCTTS = p_amc13->parseInputEnableList(m_amcIgnoreTTSList,true);
  CMSGEMOS_INFO("AMC13Manager::initializeAction m_amcIgnoreTTSList " << m_amcIgnoreTTSList
       << " parsed as m_ignoreAMCTTS: " << std::hex << m_ignoreAMCTTS << std::dec);
  if (m_ignoreAMCTTS) {
    p_amc13->ttsDisableMask(m_ignoreAMCTTS);
  } else {
    p_amc13->ttsDisableMask(0x0);
  }

  // Use local TTC signal if config says to
  p_amc13->localTtcSignalEnable(m_enableLocalTTC);

  // Use local trigger generator if config says to
  p_amc13->configureLocalL1A(m_enableLocalL1A, m_L1Amode, m_L1Aburst, m_internalPeriodicPeriod, m_L1Arules);
  // probably shouldn't enable until we're running? CHECKME
  // p_amc13->enableLocalL1A(m_enableLocalL1A);
  p_amc13->write(::amc13::AMC13::T1,"CONF.TTC.T3_TRIG",0);

  // need to ensure that all BGO channels are disabled
  for (int bchan = 0; bchan < 4; ++bchan)
    p_amc13->disableBGO(bchan);

  // Enable Monitor Buffer Backpressure if config doc says so
  p_amc13->monBufBackPressEnable(m_monBackPressEnable);

  // m_dtc->configurePrescale(1,m_preScaleFactNumOfZeros);
  p_amc13->configurePrescale(0, m_prescaleFactor);

  // set the FED id
  p_amc13->setFEDid(m_fedID);

  // reset the PLL on the T1
  if (!m_skipPLLReset)
    p_amc13->write(::amc13::AMC13::T1, 0x0, 0x8);
  usleep(200);

  // reset the T1
  p_amc13->reset(::amc13::AMC13::T1);

  // reset the T1 counters
  p_amc13->resetCounters();

  // unlock the access
  CMSGEMOS_INFO("AMC13Manager::initializeAction end");
}

void gem::hw::amc13::AMC13Manager::configureAction()
{
  /* REDUNDANT?
  if (m_enableLocalL1A) {
    m_L1Aburst = m_localTriggerConfig.bag.l1Aburst.value_;
    p_amc13->configureLocalL1A(m_enableLocalL1A, m_L1Amode, m_L1Aburst, m_internalPeriodicPeriod, m_L1Arules);
  } else {
    p_amc13->configureLocalL1A(m_enableLocalL1A, m_L1Amode, m_L1Aburst, m_internalPeriodicPeriod, m_L1Arules);
  }
  */
  m_L1Aburst = m_localTriggerConfig.bag.l1Aburst.value_;
  p_amc13->configureLocalL1A(m_enableLocalL1A, m_L1Amode, m_L1Aburst, m_internalPeriodicPeriod, m_L1Arules);

  if (m_enableLocalTTC) {
    CMSGEMOS_DEBUG("AMC13Manager::configureAction configuring BGO channels "
          << m_bgoConfig.size());
    for (auto bchan = m_bgoConfig.begin(); bchan != m_bgoConfig.end(); ++bchan) {
      CMSGEMOS_DEBUG("AMC13Manager::configureAction channel "
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
  // set the settings from the config options
  // usleep(10); // just for testing the timing of different applications
  CMSGEMOS_INFO("AMC13Manager::configureAction end");
}

void gem::hw::amc13::AMC13Manager::startAction()
{
  CMSGEMOS_DEBUG("AMC13Manager::Entering AMC13Manager::startAction()");
  // gem::base::GEMFSMApplication::enable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);

  // reset the T1?
  p_amc13->reset(::amc13::AMC13::T1);

  // reset the PLL on the T1
  // if (!m_skipPLLReset)
  //   p_amc13->write(::amc13::AMC13::T1, 0x0, 0x1);
  usleep(10);

  p_amc13->resetCounters();
  m_updatedL1ACount = p_amc13->read(::amc13::AMC13::T1,"STATUS.GENERAL.L1A_COUNT_LO");

  p_amc13->startRun();

  if (m_enableLocalTTC) {
    for (auto bchan = m_bgoConfig.begin(); bchan != m_bgoConfig.end(); ++bchan)
      if (bchan->bag.channel.value_ > -1) {
        CMSGEMOS_INFO("AMC13Manager::startAction enabling BGO channel " << bchan->bag.channel.value_);
	if (bchan->bag.repeat.value_)
          p_amc13->enableBGORepeat(bchan->bag.channel.value_);
	else
          p_amc13->enableBGOSingle(bchan->bag.channel.value_);
      }
    p_amc13->sendBGO();
  }

  if (m_enableLocalL1A) {
    if (m_enableLEMO) {
      CMSGEMOS_DEBUG("AMC13Manager::startAction enabling LEMO trigger " << m_enableLEMO);
      p_amc13->write(::amc13::AMC13::T1,"CONF.TTC.T3_TRIG",0x1);
    } else {
      p_amc13->startContinuousL1A();
    }
  } else {
    /*
    // HACK
    // when using TTC triggers, they should be enabled upstream of the AMC13 with start
    p_amc13->configureLocalL1A(m_enableLocalL1A, m_L1Amode, m_L1Aburst, m_internalPeriodicPeriod, m_L1Arules);
    */
  }

  if (m_scanType.value_ == 2 || m_scanType.value_ == 3) {
    CMSGEMOS_DEBUG("AMC13Manager::startAction Sending continuous triggers for ScanRoutines ");
    // p_amc13->enableLocalL1A(m_enableLocalL1A);

    // if (m_enableLocalL1A) {
    //   p_amc13->configureLocalL1A(m_enableLocalL1A, m_L1Amode, m_L1Aburst, m_internalPeriodicPeriod, m_L1Arules);
    //   if (m_enableLEMO)
    //     p_amc13->write(::amc13::AMC13::T1,"CONF.TTC.T3_TRIG",1);
    //   else
    //     p_amc13->startContinuousL1A();
    // } else {
    //   p_amc13->configureLocalL1A(m_enableLocalL1A, m_L1Amode, m_L1Aburst, m_internalPeriodicPeriod, m_L1Arules);
    // }

    try {
      p_timer->stop();
    } catch (toolbox::task::exception::NotActive const& ex) {
      CMSGEMOS_WARN("AMC13Manager::start could not stop timer " << ex.what());
    }
    p_timer->start();
    toolbox::TimeInterval interval(0.1,0);  // period of 0.1 secs
    toolbox::TimeVal start;
    start = toolbox::TimeVal::gettimeofday();
    p_timer->scheduleAtFixedRate(start, this, interval, 0, "" );
  }  // end scan type
  CMSGEMOS_INFO("AMC13Manager::startAction end");
}

void gem::hw::amc13::AMC13Manager::pauseAction()
{
  // what does pause mean here?
  // if local triggers are enabled, do we have a separate trigger application?
  // we can just disable them here maybe?
  if (m_scanType.value_ == 2 || m_scanType.value_ == 3) {
    CMSGEMOS_INFO("AMC13Manager::pauseAction disabling triggers for scan, triggers seen this point = "
	 << p_amc13->read(::amc13::AMC13::T1,"STATUS.GENERAL.L1A_COUNT_LO") - m_updatedL1ACount);
    m_updatedL1ACount = p_amc13->read(::amc13::AMC13::T1,"STATUS.GENERAL.L1A_COUNT_LO");
    CMSGEMOS_INFO("AMC13Manager::timeExpried, total triggers seen = " << m_updatedL1ACount);
  }

  if (m_enableLocalL1A) {
    if (m_enableLEMO)
      p_amc13->write(::amc13::AMC13::T1,"CONF.TTC.T3_TRIG",0);
    else
      // what if using both local triggers and LEMO triggers?
      p_amc13->stopContinuousL1A();
  } else {
    /*
    // HACK
    // when using external triggers, they should be stopped upstream of the AMC13 with a pause
    p_amc13->configureLocalL1A(true, m_L1Amode, m_L1Aburst, m_internalPeriodicPeriod, m_L1Arules);
    p_amc13->enableLocalL1A(true);  // is this fine to switch to local L1A as a way to fake turning off upstream?
    */
  }

  if (m_enableLocalTTC)
    for (auto bchan = m_bgoConfig.begin(); bchan != m_bgoConfig.end(); ++bchan)
      if (bchan->bag.channel.value_ > -1) {
        CMSGEMOS_DEBUG("AMC13Manager::pauseAction disabling BGO channel " << bchan->bag.channel.value_);
        p_amc13->disableBGO(bchan->bag.channel.value_);
      }

  // need to ensure that all BGO channels are disabled, rather than just the ones in the config
  for (int bchan = 0; bchan < 4; ++bchan)
    p_amc13->disableBGO(bchan);

  CMSGEMOS_INFO("AMC13Manager::pauseAction end");
}

void gem::hw::amc13::AMC13Manager::resumeAction()
{
  // undo the actions taken in pauseAction
  if (m_enableLocalTTC) {
    bool sendLocalBGO = false;
    for (auto bchan = m_bgoConfig.begin(); bchan != m_bgoConfig.end(); ++bchan)
      if (bchan->bag.channel.value_ > -1) {
        sendLocalBGO = true;
        CMSGEMOS_DEBUG("AMC13Manager::resumeAction enabling BGO channel " << bchan->bag.channel.value_);
	if (bchan->bag.repeat.value_)
          p_amc13->enableBGORepeat(bchan->bag.channel.value_);
	else
          p_amc13->enableBGOSingle(bchan->bag.channel.value_);
      }

    if (sendLocalBGO)
      p_amc13->sendBGO();
  }

  if (m_enableLocalL1A) {
    p_amc13->configureLocalL1A(m_enableLocalL1A, m_L1Amode, m_L1Aburst, m_internalPeriodicPeriod, m_L1Arules);
    p_amc13->enableLocalL1A(m_enableLocalL1A);

    if (m_enableLEMO)
      p_amc13->write(::amc13::AMC13::T1,"CONF.TTC.T3_TRIG",1);
    else
      p_amc13->startContinuousL1A();  // only if we want to send triggers continuously
  } else {
    /*
    // HACK
    // when using external triggers, they should be enabled upstream of the AMC13 with a resume
    p_amc13->configureLocalL1A(m_enableLocalL1A, m_L1Amode, m_L1Aburst, m_internalPeriodicPeriod, m_L1Arules);
    */
  }

  // if (m_scanType.value_ == 2 || m_scanType.value_ == 3) {
  //   CMSGEMOS_DEBUG("AMC13Manager::resumeAction enabling triggers for scan");

  //   if (m_enableLocalL1A) {
  //     p_amc13->configureLocalL1A(m_enableLocalL1A, m_L1Amode, m_L1Aburst, m_internalPeriodicPeriod, m_L1Arules);
  //     p_amc13->enableLocalL1A(m_enableLocalL1A);

  //     if (m_enableLEMO)
  //       p_amc13->write(::amc13::AMC13::T1,"CONF.TTC.T3_TRIG",1);
  //     else
  //       p_amc13->startContinuousL1A();
  //   } else {
  //     p_amc13->configureLocalL1A(m_enableLocalL1A, m_L1Amode, m_L1Aburst, m_internalPeriodicPeriod, m_L1Arules);
  //   }
  // }
  // usleep(10);
  CMSGEMOS_INFO("AMC13Manager::resumeAction end");
}

void gem::hw::amc13::AMC13Manager::stopAction()
{
  CMSGEMOS_DEBUG("AMC13Manager::Entering AMC13Manager::stopAction()");
  // gem::base::GEMFSMApplication::disable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);

  if (m_scanType.value_ == 2 || m_scanType.value_ == 3) {
    CMSGEMOS_DEBUG("AMC13Manager::stopAction Sending continuous triggers for ScanRoutines ");
    p_timer->stop();
  }

  if (m_enableLocalL1A) {
    // p_amc13->enableLocalL1A(false);

    if (m_enableLEMO)
      p_amc13->write(::amc13::AMC13::T1,"CONF.TTC.T3_TRIG",0);
    else
      p_amc13->stopContinuousL1A();
  } else {
    /*
    // HACK
    // when using external triggers, they should be stopped upstream of the AMC13 with a stop
    p_amc13->configureLocalL1A(m_enableLocalL1A, m_L1Amode, m_L1Aburst, m_internalPeriodicPeriod, m_L1Arules);
    p_amc13->enableLocalL1A(true);
    */
  }

  if (m_enableLocalTTC)
    for (auto bchan = m_bgoConfig.begin(); bchan != m_bgoConfig.end(); ++bchan)
      if (bchan->bag.channel.value_ > -1) {
        CMSGEMOS_DEBUG("AMC13Manager::stopAction disabling BGO channel " << bchan->bag.channel.value_);
        p_amc13->disableBGO(bchan->bag.channel.value_);
      }

  // need to ensure that all BGO channels are disabled, rather than just the ones in the config
  for (int bchan = 0; bchan < 4; ++bchan)
    p_amc13->disableBGO(bchan);

 // usleep(10);
  p_amc13->endRun();
  CMSGEMOS_INFO("AMC13Manager::stopAction end");
}

void gem::hw::amc13::AMC13Manager::haltAction()
{
  // what is necessary for a halt on the AMC13?
  usleep(10);  // just for testing the timing of different applications
  CMSGEMOS_INFO("AMC13Manager::haltAction end");
}

void gem::hw::amc13::AMC13Manager::resetAction()
{
  // what is necessary for a reset on the AMC13?
  CMSGEMOS_DEBUG("Entering AMC13Manager::resetAction()");

  if (p_timer) {
    try {
      p_timer->stop();
    } catch (toolbox::task::exception::NotActive const& ex) {
      CMSGEMOS_WARN("AMC13Manager::start could not stop timer " << ex.what());
    }
  }

  // maybe ensure triggers are disabled as well as BGO commands?
  usleep(10);
  // gem::base::GEMFSMApplication::resetAction();
  CMSGEMOS_INFO("AMC13Manager::resetAction end");
}

/*These should maybe only be implemented in GEMFSMApplication,
  unless there is a reason to perform some special action
  for each hardware*/
void gem::hw::amc13::AMC13Manager::failAction(toolbox::Event::Reference e)
{
}

void gem::hw::amc13::AMC13Manager::resetAction(toolbox::Event::Reference e)
{
}

// void gem::hw::amc13::AMC13Manager::sendTriggerBurst()

xoap::MessageReference gem::hw::amc13::AMC13Manager::sendTriggerBurst(xoap::MessageReference msg)
{
  // set to send a burst of trigger
  CMSGEMOS_INFO("Entering AMC13Manager::sendTriggerBurst()");

  if (msg.isNull()) {
    XCEPT_RAISE(xoap::exception::Exception,"Null message received!");
  }

  std::string commandName = "sendTriggerBurst";

  try {
    if (m_enableLocalL1A &&  m_sendL1ATriburst) {  // CLEANME need to remove
      // p_amc13->localTtcgSignalEnable(m_enableLocalL1A);
      // p_amc13->enableLocalL1A(m_enableLocalL1A);
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
    CMSGEMOS_INFO("AMC13Manager::sendTriggerBurst command " << commandName << " succeeded ");
    return
      gem::utils::soap::GEMSOAPToolBox::makeSOAPReply(commandName, "SentTriggers");
  } catch(xcept::Exception& err) {
    std::string msgBase = toolbox::toString("Failed to create SOAP reply for command '%s'",
                                            commandName.c_str());
    CMSGEMOS_ERROR(toolbox::toString("%s: %s.", msgBase.c_str(), xcept::stdformat_exception(err).c_str()));
    XCEPT_DECLARE_NESTED(gem::base::utils::exception::SoftwareProblem,
                         top, toolbox::toString("%s.",msgBase.c_str()), err);
    this->notifyQualified("error", top);

    XCEPT_RETHROW(xoap::exception::Exception, msgBase, err);
  }
  XCEPT_RAISE(xoap::exception::Exception,"command not found");
}


xoap::MessageReference gem::hw::amc13::AMC13Manager::enableTriggers(xoap::MessageReference msg)
{
  CMSGEMOS_DEBUG("AMC13Manager::enableTriggers");
  // gem::base::GEMFSMApplication::disable();

  std::string commandName = "enableTriggers";

  if (!p_amc13) {
    std::string msgBase = toolbox::toString("Failed to create SOAP reply for command '%s', AMC13 not yet connected",
                                            commandName.c_str());
    CMSGEMOS_ERROR(toolbox::toString("%s", msgBase.c_str()));
    return
      gem::utils::soap::GEMSOAPToolBox::makeSOAPReply(commandName, "Failed");
  }

  if (!m_startL1ATricont) {  // CLEANME need to remove
    p_amc13->enableLocalL1A(m_enableLocalL1A);
    if (m_enableLEMO)
      p_amc13->write(::amc13::AMC13::T1,"CONF.TTC.T3_TRIG",1);
  }

  if (m_enableLocalL1A && m_startL1ATricont) {  // CLEANME need to remove
    p_amc13->startContinuousL1A();
  }

  try {
    CMSGEMOS_INFO("AMC13Manager::enableTriggers command " << commandName << " succeeded ");
    return
      gem::utils::soap::GEMSOAPToolBox::makeSOAPReply(commandName, "SentTriggers");
  } catch(xcept::Exception& err) {
    std::string msgBase = toolbox::toString("Failed to create SOAP reply for command '%s'",
                                            commandName.c_str());
    CMSGEMOS_ERROR(toolbox::toString("%s: %s.", msgBase.c_str(), xcept::stdformat_exception(err).c_str()));
    XCEPT_DECLARE_NESTED(gem::base::utils::exception::SoftwareProblem,
                         top, toolbox::toString("%s.",msgBase.c_str()), err);
    this->notifyQualified("error", top);

    XCEPT_RETHROW(xoap::exception::Exception, msgBase, err);
  }
  XCEPT_RAISE(xoap::exception::Exception,"command not found");
}

xoap::MessageReference gem::hw::amc13::AMC13Manager::disableTriggers(xoap::MessageReference msg)
{
  CMSGEMOS_DEBUG("AMC13Manager::disableTriggers");
  // gem::base::GEMFSMApplication::disable();
  std::string commandName = "disableTriggers";

  if (!p_amc13) {
    std::string msgBase = toolbox::toString("Failed to create SOAP reply for command '%s', AMC13 not yet connected",
                                            commandName.c_str());
    CMSGEMOS_ERROR(toolbox::toString("%s", msgBase.c_str()));
    return
      gem::utils::soap::GEMSOAPToolBox::makeSOAPReply(commandName, "Failed");
  }

  if (!m_startL1ATricont) {  // CLEANME need to remove
    p_amc13->enableLocalL1A(!m_enableLocalL1A);
    if (m_enableLEMO)
      p_amc13->write(::amc13::AMC13::T1,"CONF.TTC.T3_TRIG",0);
  }

  if (m_enableLocalL1A && m_startL1ATricont) {  // CLEANME need to remove
    p_amc13->stopContinuousL1A();
  }

  try {
    CMSGEMOS_INFO("AMC13Manager::disableTriggers " << commandName << " succeeded ");
    return
      gem::utils::soap::GEMSOAPToolBox::makeSOAPReply(commandName, "SentTriggers");
  } catch(xcept::Exception& err) {
    std::string msgBase = toolbox::toString("Failed to create SOAP reply for command '%s'",
                                            commandName.c_str());
    CMSGEMOS_ERROR(toolbox::toString("%s: %s.", msgBase.c_str(), xcept::stdformat_exception(err).c_str()));
    XCEPT_DECLARE_NESTED(gem::base::utils::exception::SoftwareProblem,
                         top, toolbox::toString("%s.",msgBase.c_str()), err);
    this->notifyQualified("error", top);

    XCEPT_RETHROW(xoap::exception::Exception, msgBase, err);
  }
  XCEPT_RAISE(xoap::exception::Exception,"command not found");
}

void gem::hw::amc13::AMC13Manager::timeExpired(toolbox::task::TimerEvent& event)
{
  uint64_t currentTrigger = p_amc13->read(::amc13::AMC13::T1,"STATUS.GENERAL.L1A_COUNT_LO") - m_updatedL1ACount;

  CMSGEMOS_DEBUG("AMC13Manager::timeExpried, NTriggerRequested = " << m_nScanTriggers.value_
        << " currentT = " << currentTrigger << " triggercounter final =  " << m_updatedL1ACount );

  if (currentTrigger >=  m_nScanTriggers.value_){
    if (m_enableLocalL1A) {
      if (m_enableLEMO) {
	p_amc13->write(::amc13::AMC13::T1,"CONF.TTC.T3_TRIG",0);
      } else {
	p_amc13->stopContinuousL1A();
      }
    } else {
      /*
      // HACK
      // when using external triggers, they should be stopped upstream of the AMC13 with a stop
      // how do I hate thee? let me count the ways...
      p_amc13->configureLocalL1A(true, m_L1Amode, m_L1Aburst, m_internalPeriodicPeriod, m_L1Arules);
      p_amc13->enableLocalL1A(true);  // is this fine to switch to local L1A as a way to fake turning off upstream?
      */
    }
    CMSGEMOS_INFO("AMC13Manager::timeExpried, triggers seen this point = "
	 << p_amc13->read(::amc13::AMC13::T1,"STATUS.GENERAL.L1A_COUNT_LO") - m_updatedL1ACount);
    // m_updatedL1ACount = p_amc13->read(::amc13::AMC13::T1,"STATUS.GENERAL.L1A_COUNT_LO");
    // CMSGEMOS_INFO("AMC13Manager::timeExpried, total triggers seen = " << m_updatedL1ACount);
    endScanPoint();
  }
}

void gem::hw::amc13::AMC13Manager::endScanPoint()
{
  CMSGEMOS_INFO("AMC13Manager::endScanPoint");
  gem::utils::soap::GEMSOAPToolBox::sendCommand("EndScanPoint",
                                                p_appContext,p_appDescriptor,//getApplicationContext(),this->getApplicationDescriptor(),
                                                const_cast<xdaq::ApplicationDescriptor*>(p_appZone->getApplicationDescriptor("gem::supervisor::GEMSupervisor", 0)));  // this should not be hard coded
}
