/**
 * class: AMCManager
 * description: Manager application for AMC cards
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date:
 */

#include "gem/hw/devices/glib/HwGLIB.h"

#include "gem/hw/managers/amc/AMCManager.h"

#include "gem/hw/managers/amc/AMCMonitor.h"
#include "gem/hw/managers/amc/AMCManagerWeb.h"

#include "gem/hw/managers/exception/Exception.h"

#include "gem/hw/utils/GEMCrateUtils.h"

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPConstants.h"
#include "xoap/SOAPBody.h"
#include "xoap/Method.h"
#include "xoap/AttachmentPart.h"
#include "xoap/domutils.h"

#include <iterator>

XDAQ_INSTANTIATOR_IMPL(gem::hw::amc::AMCManager);

gem::hw::amc::AMCManager::AMCInfo::AMCInfo()
{
  present    = false;
  crateID    = -1;
  slotID     = -1;
  sbitSource = 0;
  enableZS   = true;
}

void gem::hw::amc::AMCManager::AMCInfo::registerFields(xdata::Bag<gem::hw::amc::AMCManager::AMCInfo>* bag)
{
  bag->addField("crateID",    &crateID);
  bag->addField("slot",       &slotID);
  bag->addField("present",    &present);
  bag->addField("sbitSource", &sbitSource);
  bag->addField("enableZS",   &enableZS);
}

gem::hw::amc::AMCManager::AMCManager(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub),
  m_amcEnableMask(0),
  m_doPhaseShift(false),
  m_bc0LockPhaseShift(false),
  m_relockPhase(true)
{
  m_amcInfo.setSize(MAX_AMCS_PER_CRATE);

  p_appInfoSpace->fireItemAvailable("AllAMCsInfo",      &m_amcInfo);
  p_appInfoSpace->fireItemAvailable("AMCSlots",          &m_amcSlots);
  p_appInfoSpace->fireItemAvailable("ConnectionFile",    &m_connectionFile);
  p_appInfoSpace->fireItemAvailable("DoPhaseShift",      &m_doPhaseShift);
  p_appInfoSpace->fireItemAvailable("BC0LockPhaseShift", &m_bc0LockPhaseShift);
  p_appInfoSpace->fireItemAvailable("RelockPhase",       &m_relockPhase);

  p_appInfoSpace->addItemRetrieveListener("AllAMCsInfo",      this);
  p_appInfoSpace->addItemRetrieveListener("AMCSlots",          this);
  p_appInfoSpace->addItemRetrieveListener("ConnectionFile",    this);
  p_appInfoSpace->addItemRetrieveListener("DoPhaseShift",      this);
  p_appInfoSpace->addItemRetrieveListener("BC0LockPhaseShift", this);
  p_appInfoSpace->addItemRetrieveListener("RelockPhase",       this);
  p_appInfoSpace->addItemChangedListener( "AllAMCsInfo",      this);
  p_appInfoSpace->addItemChangedListener( "AMCSlots",          this);
  p_appInfoSpace->addItemChangedListener( "ConnectionFile",    this);
  p_appInfoSpace->addItemChangedListener( "DoPhaseShift",      this);
  p_appInfoSpace->addItemChangedListener( "BC0LockPhaseShift", this);
  p_appInfoSpace->addItemChangedListener( "RelockPhase",       this);

  xgi::bind(this, &AMCManager::dumpAMCFIFO, "dumpAMCFIFO");

  // initialize the AMC application objects
  CMSGEMOS_DEBUG("AMCManager::Connecting to the AMCManagerWeb interface");
  p_gemWebInterface = new gem::hw::amc::AMCManagerWeb(this);
  // p_gemMonitor      = new gem::hw::amc::AMCHwMonitor(this);
  CMSGEMOS_DEBUG("AMCManager::done");

  // set up the info hwCfgInfoSpace
  init();

  // getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemhardware/images/amc/AMCManager.png");
}

gem::hw::amc::AMCManager::~AMCManager()
{
  // memory management, maybe not necessary here?
}

std::vector<uint32_t> gem::hw::amc::AMCManager::dumpAMCFIFO(int const& amc)
{
/*
  std::vector<uint32_t> dump;
  if (amc < 0 || amc > 11) {
    CMSGEMOS_WARN("AMCManager::dumpAMCFIFO Specified invalid AMC card " << amc+1);
    return dump;
  } else if (!m_amcs.at(amc)) {
    CMSGEMOS_WARN("AMCManager::dumpAMCFIFO Specified AMC card " << amc+1
                  << " is not connected");
    return dump;
    //} else if (!(m_amcs.at(amc)->hasTrackingData(0))) {
    //  CMSGEMOS_WARN("AMCManager::dumpAMCFIFO Specified AMC card " << amc
    //       << " has no tracking data in the FIFO");
    //  return dump;
  }

  try {
    CMSGEMOS_INFO("AMCManager::dumpAMCFIFO Dumping FIFO for specified AMC card " << amc+1);
    return m_amcs.at(amc)->getTrackingData(0, 24);
  } catch (gem::hw::devices::exception::Exception const& e) {
    std::stringstream msg;
    msg << "AMCManager::dumpAMCFIFO Unable to read tracking data from AMC" << amc+1
        << " FIFO, caught exception " << e.what();
    CMSGEMOS_ERROR(msg.str());
    return dump;
  } catch (std::exception const& e) {
    std::stringstream msg;
    msg << "AMCManager::dumpAMCFIFO Unable to read tracking data from AMC" << amc+1
        << " FIFO, caught exception " << e.what();
    CMSGEMOS_ERROR(msg.str());
    return dump;
  } catch (...) {
    std::stringstream msg;
    msg << "AMCManager::dumpAMCFIFO Unable to read tracking data from AMC" << amc+1
        << " FIFO, caught unknown exception ";
    CMSGEMOS_ERROR(msg.str());
    return dump;
  }
*/
}

// This is the callback used for handling xdata:Event objects
void gem::hw::amc::AMCManager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    CMSGEMOS_DEBUG("AMCManager::actionPerformed() setDefaultValues"
                   << "Default configuration values have been loaded from xml profile");
    m_amcEnableMask = gem::hw::utils::parseAMCEnableList(m_amcSlots.toString(),
                                                         // &m_gemLogger);
                                                         m_gemLogger);
    CMSGEMOS_INFO("AMCManager::actionPerformed() Parsed AMCEnableList m_amcSlots = " << m_amcSlots.toString()
                  << " to slotMask 0x" << std::hex << m_amcEnableMask << std::dec);

    // how to handle passing in various values nested in a vector in a bag
    for (auto& slot : m_amcInfo) {
      if (slot.bag.crateID.value_ > -1) {
        slot.bag.present = true;
        CMSGEMOS_DEBUG("AMCManager::Found attribute:" << slot.bag.toString());
      }
    }
    // p_gemMonitor->startMonitoring();
  }

  // FIXME update monitoring variables?
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::amc::AMCManager::init()
{
  // anything needed here?
}

// state transitions
void gem::hw::amc::AMCManager::initializeAction()
{
  CMSGEMOS_DEBUG("AMCManager::initializeAction begin");
  // FIXME make me more streamlined
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    CMSGEMOS_DEBUG("AMCManager::looping over slots(" << (slot+1) << ") and finding expected cards");
    AMCInfo& info = m_amcInfo[slot].bag;
    if ((m_amcEnableMask >> (slot)) & 0x1) {
      CMSGEMOS_DEBUG("AMCManager::info:" << info.toString());
      CMSGEMOS_DEBUG("AMCManager::expect a card in slot " << (slot+1));
      CMSGEMOS_DEBUG("AMCManager::bag: "
                     << "crate " << info.crateID.value_
                     << " slot " << info.slotID.value_);
      // this maybe shouldn't be done?
      info.slotID  = slot+1;
      CMSGEMOS_DEBUG("AMCManager::bag: "
                     << "crate " << info.crateID.value_
                     << " slot " << info.slotID.value_);
      // this maybe shouldn't be done?
      info.present = true;
      // actually check presence? this just says that we expect it to be there
      // check if there is a AMC in the specified slot, if not, do not initialize
      // set the web view to be empty or grey
      // if (!info.present.value_) continue;
      // needs .value_?
      // p_gemWebInterface->amcInSlot(slot);
    }
  }

  // FIXME make me more streamlined
  for (auto const& infoi : m_amcInfo) {
    auto&& info = infoi.bag;

    // check the config file if there should be a AMC in the specified slot, if not, do not initialize
    if (!info.present)
      continue;

    uint32_t slot = info.slotID.value_-1;
    CMSGEMOS_DEBUG("AMCManager::creating pointer to card in slot " << (slot+1));

    // create the cfgInfoSpace object (qualified vs non?)
    std::string deviceName = toolbox::toString("gem-shelf%02d-amc%02d",
                                               info.crateID.value_,
                                               info.slotID.value_);
    toolbox::net::URN hwCfgURN("urn:gem:hw:"+deviceName);

    if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
      CMSGEMOS_DEBUG("AMCManager::initializeAction::infospace " << hwCfgURN.toString() << " already exists, getting");
      is_amcs.at(slot) = is_toolbox_ptr(new gem::base::utils::GEMInfoSpaceToolBox(this,
                                                                                   xdata::getInfoSpaceFactory()->get(hwCfgURN.toString()),
                                                                                   true));
    } else {
      CMSGEMOS_DEBUG("AMCManager::initializeAction::infospace " << hwCfgURN.toString() << " does not exist, creating");
      is_amcs.at(slot) = is_toolbox_ptr(new gem::base::utils::GEMInfoSpaceToolBox(this,
                                                                                   hwCfgURN.toString(),
                                                                                   true));
    }

    try {
      CMSGEMOS_DEBUG("AMCManager::obtaining pointer to HwGLIB");
      m_amcs.at(slot) = amc_shared_ptr(new gem::hw::glib::HwGLIB(deviceName, m_connectionFile.toString()));
      amc_shared_ptr amc = m_amcs.at(slot);
      if (amc->isHwConnected()) {
        CMSGEMOS_DEBUG("AMCManager::Creating InfoSpace items for AMC device " << deviceName);

        // FIXME should not need this here?
        amc->disableDAQLink();

        // maybe better to raise exception here and fail if not connected, as we expected the card to be here?
        createAMCInfoSpaceItems(is_amcs.at(slot), amc);

        if (!m_disableMonitoring) {
          m_amcMonitors.at(slot) = std::shared_ptr<AMCMonitor>(new AMCMonitor(amc, this, slot+1));
          m_amcMonitors.at(slot)->addInfoSpace("HWMonitoring", is_amcs.at(slot));
          m_amcMonitors.at(slot)->setupHwMonitoring();
          m_amcMonitors.at(slot)->startMonitoring();
        }
      } else {
        std::stringstream msg;
        msg << "AMCManager::initializeAction unable to communicate with AMC in slot " << slot;
        CMSGEMOS_ERROR(msg.str());
        XCEPT_RAISE(gem::hw::devices::exception::Exception, "initializeAction failed");
      }
    } GEM_HW_TRANSITION_CATCH("AMCManager::initializeAction",gem::hw::devices::exception::Exception);
    // catch (uhalException const& e) {
    //   std::stringstream msg;
    //   msg << "AMCManager::initializeAction caught uHAL exception " << e.what();
    //   CMSGEMOS_ERROR(msg.str());
    //   XCEPT_RAISE(gem::hw::devices::exception::Exception, msg.str());
    // } catch (gem::hw::devices::exception::Exception const& e) {
    //   std::stringstream msg;
    //   msg << "AMCManager::initializeAction caught exception " << e.what();
    //   CMSGEMOS_ERROR(msg.str());
    //   XCEPT_RAISE(gem::hw::devices::exception::Exception, msg.str());
    // } catch (toolbox::net::exception::MalformedURN const& e) {
    //   std::stringstream msg;
    //   msg << "AMCManager::initializeAction caught exception " << e.what();
    //   CMSGEMOS_ERROR(msg.str());
    //   XCEPT_RAISE(gem::hw::devices::exception::Exception, msg.str());
    // } catch (std::exception const& e) {
    //   std::stringstream msg;
    //   msg << "AMCManager::initializeAction caught exception " << e.what();
    //   CMSGEMOS_ERROR(msg.str());
    //   XCEPT_RAISE(gem::hw::devices::exception::Exception, msg.str());
    // }
    CMSGEMOS_DEBUG("AMCManager::connected");
    // set the web view to be empty or grey
    // if (!info.present.value_) continue;
    // p_gemWebInterface->amcInSlot(slot);
  }

  // FIXME make me more streamlined
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    AMCInfo& info = m_amcInfo[slot].bag;

    if (!info.present)
      continue;

    amc_shared_ptr amc = m_amcs.at(slot);
    if (amc->isHwConnected()) {
      CMSGEMOS_DEBUG("AMCManager::connected a card in slot " << (slot+1));
    } else {
      std::stringstream msg;
      msg << "AMCManager::initializeAction AMC in slot " << (slot+1) << " is not connected";
      CMSGEMOS_ERROR(msg.str());
      // fireEvent("Fail");
      XCEPT_RAISE(gem::hw::devices::exception::Exception, msg.str());
    }
  }
  // usleep(10); // just for testing the timing of different applications
  CMSGEMOS_INFO("AMCManager::initializeAction end");
}

void gem::hw::amc::AMCManager::configureAction()
{
  CMSGEMOS_DEBUG("AMCManager::configureAction");

  // FIXME make me more streamlined
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    // usleep(10); // just for testing the timing of different applications
    AMCInfo& info = m_amcInfo[slot].bag;

    if (!info.present)
      continue;

    amc_shared_ptr amc = m_amcs.at(slot);
    if (amc->isHwConnected()) {
      if (m_amcMonitors.at(slot))
        m_amcMonitors.at(slot)->pauseMonitoring();

      bool enableZS     = info.enableZS.value_;
      bool doPhaseShift = m_doPhaseShift.value_;
      uint32_t runType  = 0x0;
      try { // FIXME if we fail, do we go to error?
        amc->configureDAQModule(enableZS, doPhaseShift, runType, 0xfaac, m_relockPhase.value_, m_bc0LockPhaseShift.value_);
      } GEM_CATCH_RPC_ERROR("AMCManager::configureAction", gem::hw::devices::exception::ConfigurationProblem);

      if (m_scanType.value_ == 2) {
        CMSGEMOS_INFO("AMCManager::configureAction: FIRST  " << m_scanMin.value_);

	amc->setDAQLinkRunType(0x2);
	amc->setDAQLinkRunParameter(0x1,m_scanMin.value_);
	// amc->setDAQLinkRunParameter(0x2,VT1);  // set these at start so DQM has them?
	// amc->setDAQLinkRunParameter(0x3,VT2);  // set these at start so DQM has them?
      } else if (m_scanType.value_ == 3) {
	uint32_t initialVT1 = m_scanMin.value_;
	uint32_t initialVT2 = 0;  // std::max(0,(uint32_t)m_scanMax.value_);
        CMSGEMOS_INFO("AMCManager::configureAction FIRST VT1 " << initialVT1 << " VT2 " << initialVT2);

	amc->setDAQLinkRunType(0x3);
	// amc->setDAQLinkRunParameter(0x1,latency);  // set this at start so DQM has it?
	amc->setDAQLinkRunParameter(0x2,initialVT1);
	amc->setDAQLinkRunParameter(0x3,initialVT2);
      } else {
	amc->setDAQLinkRunType(0x1);          // FIXME duplicated in configureDAQModule call
	amc->setDAQLinkRunParameters(0xfaac); // FIXME duplicated in configureDAQModule call
      }

      // what else is required for configuring the AMC?
      // need to reset optical links?
      // reset counters?
      // setup run mode?
      // setup DAQ mode?

      if (m_amcMonitors.at(slot))
        m_amcMonitors.at(slot)->resumeMonitoring();
    } else {
      std::stringstream msg;
      msg << "AMCManager::configureAction AMC in slot " << (slot+1) << " is not connected";
      CMSGEMOS_ERROR(msg.str());
      // fireEvent("Fail");
      XCEPT_RAISE(gem::hw::devices::exception::Exception, msg.str());
    }
  }

  CMSGEMOS_INFO("AMCManager::configureAction end");
}

void gem::hw::amc::AMCManager::startAction()
{
  if (m_scanType.value_ == 2) {
    CMSGEMOS_INFO("AMCManager::startAction() " << std::endl << m_scanInfo.bag.toString());
    m_lastLatency = m_scanMin.value_;
    m_lastVT1 = 0;
  } else if (m_scanType.value_ == 3) {
    CMSGEMOS_INFO("AMCManager::startAction() " << std::endl << m_scanInfo.bag.toString());
    m_lastLatency = 0;
    m_lastVT1 = m_scanMin.value_;
  }

  CMSGEMOS_INFO("AMCManager::startAction begin");
  // what is required for starting the AMC?
  // FIXME make me more streamlined
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    // usleep(10);
    CMSGEMOS_DEBUG("AMCManager::looping over slots(" << (slot+1) << ") and finding infospace items");
    AMCInfo& info = m_amcInfo[slot].bag;

    if (!info.present)
      continue;

    amc_shared_ptr amc = m_amcs.at(slot);
    if (amc->isHwConnected()) {
      if (m_amcMonitors.at(slot))
        m_amcMonitors.at(slot)->pauseMonitoring();

      CMSGEMOS_DEBUG("connected a card in slot " << (slot+1));
      // enable the DAQ
      // amc->enableDAQModule(info.enableZS.value_);

      amc->ttcModuleReset();
      amc->enableDAQLink(0x4);  // FIXME
      amc->resetDAQLink();
      amc->setZS(info.enableZS.value_);
      amc->setL1AEnable(true);
      usleep(10); // just for testing the timing of different applications

      if (m_amcMonitors.at(slot))
        m_amcMonitors.at(slot)->resumeMonitoring();
    } else {
      std::stringstream msg;
      msg << "AMCManager::startAction AMC in slot " << (slot+1) << " is not connected";
      CMSGEMOS_ERROR(msg.str());
      // fireEvent("Fail");
      XCEPT_RAISE(gem::hw::devices::exception::Exception, msg.str());
    }

    /*
    // reset the hw monitor, this was in release-v2 but not in integrated-application-framework, may have forgotten something
    if (m_amcMonitors.at(slot))
    m_amcMonitors.at(slot)->reset();
    */
  }
  // usleep(10);
  CMSGEMOS_INFO("AMCManager::startAction end");
}

void gem::hw::amc::AMCManager::pauseAction()
{
  // what is required for pausing the AMC?
  // FIXME make me more streamlined
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    // usleep(10);
    CMSGEMOS_DEBUG("AMCManager::looping over slots(" << (slot+1) << ") and finding infospace items");
    AMCInfo& info = m_amcInfo[slot].bag;

    if (!info.present)
      continue;

    amc_shared_ptr amc = m_amcs.at(slot);
    if (amc->isHwConnected()) {
      if (m_amcMonitors.at(slot))
        m_amcMonitors.at(slot)->pauseMonitoring();

      CMSGEMOS_DEBUG("connected a card in slot " << (slot+1));

      if (m_scanType.value_ == 2) {
	uint8_t updatedLatency = m_lastLatency + m_stepSize.value_;
        CMSGEMOS_INFO("AMCManager::pauseAction LatencyScan AMC" << (slot+1) << " Latency " << (int)updatedLatency);

        // wait for events to finish building
        while (!amc->l1aFIFOIsEmpty()) {
          CMSGEMOS_DEBUG("AMCManager::pauseAction waiting for AMC" << (slot+1) << " to finish building events");
          usleep(10);
        }
        CMSGEMOS_DEBUG("AMCManager::pauseAction AMC" << (slot+1) << " finished building events, updating run parameter "
                       << (int)updatedLatency);
        amc->setDAQLinkRunParameter(0x1,updatedLatency);
      } else if (m_scanType.value_ == 3) {
        uint8_t updatedVT1 = m_lastVT1 + m_stepSize.value_;
        uint8_t updatedVT2 = 0; //std::max(0,(int)m_scanMax.value_);
        CMSGEMOS_INFO("AMCManager::pauseAction ThresholdScan AMC" << (slot+1) << ""
                      << " VT1 " << (int)updatedVT1
                      << " VT2 " << (int)updatedVT2);

        // wait for events to finish building
        while (!amc->l1aFIFOIsEmpty()) {
          CMSGEMOS_DEBUG("AMCManager::pauseAction waiting for AMC" << (slot+1) << " to finish building events");
          usleep(10);
        }
        CMSGEMOS_DEBUG("AMCManager::pauseAction finished AMC" << (slot+1) << " building events, updating VT1 " << (int)updatedVT1
                       << " and VT2 " << (int)updatedVT2);
	amc->setDAQLinkRunParameter(0x2,updatedVT1);
	amc->setDAQLinkRunParameter(0x3,updatedVT2);
      }
      // usleep(100); // just for testing the timing of different applications

      if (m_amcMonitors.at(slot))
        m_amcMonitors.at(slot)->resumeMonitoring();
    } else {
      std::stringstream msg;
      msg << "AMCManager::pauseAction AMC in slot " << (slot+1) << " is not connected";
      CMSGEMOS_ERROR(msg.str());
      // fireEvent("Fail");
      XCEPT_RAISE(gem::hw::devices::exception::Exception, msg.str());
    }
  }

  // Update the scan parameters
  if (m_scanType.value_ == 2) {
    CMSGEMOS_INFO("AMCManager::pauseAction LatencyScan old Latency " << (int)m_lastLatency);
    m_lastLatency += m_stepSize.value_;
    CMSGEMOS_INFO("AMCManager::pauseAction LatencyScan new Latency " << (int)m_lastLatency);
  } else if (m_scanType.value_ == 3) {
    CMSGEMOS_INFO("AMCManager::pauseAction ThresholdScan old VT1 " << (int)m_lastVT1);
    m_lastVT1 += m_stepSize.value_;
    CMSGEMOS_INFO("AMCManager::pauseAction ThresholdScan new VT1 " << (int)m_lastVT1);
  }
  CMSGEMOS_INFO("AMCManager::pauseAction end");
}

void gem::hw::amc::AMCManager::resumeAction()
{
  // what is required for resuming the AMC?
  usleep(10);  // just for testing the timing of different applications
  CMSGEMOS_INFO("AMCManager::resumeAction end");
}

void gem::hw::amc::AMCManager::stopAction()
{
  CMSGEMOS_INFO("AMCManager::stopAction begin");
  // FIXME make me more streamlined
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    // usleep(10);
    CMSGEMOS_DEBUG("AMCManager::looping over slots(" << (slot+1) << ") and finding infospace items");
    AMCInfo& info = m_amcInfo[slot].bag;

    if (!info.present)
      continue;

    amc_shared_ptr amc = m_amcs.at(slot);
    if (amc->isHwConnected()) {
      if (m_amcMonitors.at(slot))
        m_amcMonitors.at(slot)->pauseMonitoring();

      // what is required for stopping the AMC?
      // FIXME temporarily inhibit triggers at the AMC
      amc->setL1AEnable(false);
      amc->disableDAQLink();
      amc->resetDAQLink();

      if (m_amcMonitors.at(slot))
        m_amcMonitors.at(slot)->resumeMonitoring();
    }
  }
  usleep(10);  // just for testing the timing of different applications
  CMSGEMOS_INFO("AMCManager::stopAction end");
}

void gem::hw::amc::AMCManager::haltAction()
{
  // what is required for halting the AMC?
  CMSGEMOS_DEBUG("AMCManager::resetAction begin");
  // FIXME make me more streamlined
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    CMSGEMOS_DEBUG("AMCManager::looping over slots(" << (slot+1) << ") and finding infospace items");
    AMCInfo& info = m_amcInfo[slot].bag;

    if (!info.present)
      continue;

    amc_shared_ptr amc = m_amcs.at(slot);
    if (amc->isHwConnected()) {
      if (m_amcMonitors.at(slot))
        m_amcMonitors.at(slot)->pauseMonitoring();

      // what is required for halting the AMC?
      // FIXME temporarily inhibit triggers at the AMC
      amc->setL1AEnable(false);
      amc->writeReg("GEM_AMC.DAQ.CONTROL.INPUT_ENABLE_MASK", 0x0);

      if (m_amcMonitors.at(slot))
        m_amcMonitors.at(slot)->resumeMonitoring();
    }
  }
  CMSGEMOS_INFO("AMCManager::haltAction end");
}

void gem::hw::amc::AMCManager::resetAction()
{
  // what is required for resetting the AMC?
  // unregister listeners and items in info spaces

  CMSGEMOS_DEBUG("AMCManager::resetAction begin");
  // FIXME make me more streamlined
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    // usleep(10);  // just for testing the timing of different applications
    CMSGEMOS_DEBUG("AMCManager::looping over slots(" << (slot+1) << ") and finding infospace items");
    AMCInfo& info = m_amcInfo[slot].bag;

    if (!info.present)
      continue;

    amc_shared_ptr amc = m_amcs.at(slot);
    if (amc->isHwConnected()) {
      if (m_amcMonitors.at(slot))
        m_amcMonitors.at(slot)->pauseMonitoring();

      // what is required for resetting the AMC?
      // FIXME temporarily inhibit triggers at the AMC
      amc->setL1AEnable(false);
      amc->writeReg("GEM_AMC.DAQ.CONTROL.INPUT_ENABLE_MASK", 0x0);
    }
    // reset the hw monitor
    if (m_amcMonitors.at(slot))
      m_amcMonitors.at(slot)->reset();

    CMSGEMOS_DEBUG("AMCManager::looking for hwCfgInfoSpace items for AMC in slot " << (slot+1));
    toolbox::net::URN hwCfgURN("urn:gem:hw:"+toolbox::toString("gem-shelf%02d-amc%02d",
                                                               info.crateID.value_,
                                                               info.slotID.value_));

    if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
      CMSGEMOS_DEBUG("AMCManager::revoking config parameters infospace");

      // reset the hw infospace toolbox
      is_amcs.at(slot)->reset();
    } else {
      CMSGEMOS_DEBUG("AMCManager::resetAction::infospace " << hwCfgURN.toString() << " does not exist, no further action");
      continue;
    }
  }
  // gem::base::GEMFSMApplication::resetAction();
  CMSGEMOS_INFO("AMCManager::resetAction end");
}

/*
  void gem::hw::amc::AMCManager::noAction()
  {
  }
*/

void gem::hw::amc::AMCManager::failAction(toolbox::Event::Reference e)
{
}

void gem::hw::amc::AMCManager::resetAction(toolbox::Event::Reference e)
{
}


void gem::hw::amc::AMCManager::createAMCInfoSpaceItems(is_toolbox_ptr is_amc, amc_shared_ptr amc)
{
  // system registers
  is_amc->createUInt32("BOARD_ID",             amc->getBoardID(),      NULL, GEMUpdateType::NOUPDATE, "docstring", "id");
  is_amc->createUInt32("SYSTEM_ID",            amc->getSystemID(),     NULL, GEMUpdateType::NOUPDATE, "docstring", "id");
  is_amc->createUInt32("FIRMWARE_VERSION",     amc->getFirmwareVer(),  NULL, GEMUpdateType::PROCESS,  "docstring", "fwver");
  is_amc->createUInt32("FIRMWARE_DATE",        amc->getFirmwareDate(), NULL, GEMUpdateType::PROCESS,  "docstring", "date");
  is_amc->createUInt32("AMC_FIRMWARE_VERSION", amc->getFirmwareVer(),  NULL, GEMUpdateType::PROCESS,  "docstring", "fwveramc");
  is_amc->createUInt32("AMC_FIRMWARE_DATE",    amc->getFirmwareDate(), NULL, GEMUpdateType::PROCESS,  "docstring", "dateoh");

  // FIXME AMC ONLY? OBSOLETE?
  is_amc->createUInt32("IP_ADDRESS",    amc->getIPAddress(),       NULL, GEMUpdateType::NOUPDATE, "docstring", "ip");
  is_amc->createUInt64("MAC_ADDRESS",   amc->getMACAddress(),      NULL, GEMUpdateType::NOUPDATE, "docstring", "mac");
  is_amc->createUInt32("SFP1_STATUS",   amc->SFPStatus(1),         NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("SFP2_STATUS",   amc->SFPStatus(2),         NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("SFP3_STATUS",   amc->SFPStatus(3),         NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("SFP4_STATUS",   amc->SFPStatus(4),         NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("FMC1_STATUS",   amc->FMCPresence(0),       NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("FMC2_STATUS",   amc->FMCPresence(1),       NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("FPGA_RESET",    amc->FPGAResetStatus(),    NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("GBE_INT",       amc->GbEInterrupt(),       NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("V6_CPLD",       amc->V6CPLDStatus(),       NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("CPLD_LOCK",     amc->CDCELockStatus(),     NULL, GEMUpdateType::HW32);

  is_amc->createUInt32("L1A"       , amc->getTTCCounter(AMCTTCCommand::TTC_L1A),        NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("BC0"       , amc->getTTCCounter(AMCTTCCommand::TTC_BC0),        NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("EC0"       , amc->getTTCCounter(AMCTTCCommand::TTC_EC0),        NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("RESYNC"    , amc->getTTCCounter(AMCTTCCommand::TTC_RESYNC),     NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("OC0"       , amc->getTTCCounter(AMCTTCCommand::TTC_OC0),        NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("HARD_RESET", amc->getTTCCounter(AMCTTCCommand::TTC_HARD_RESET), NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("CALPULSE"  , amc->getTTCCounter(AMCTTCCommand::TTC_CALPULSE),   NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("START"     , amc->getTTCCounter(AMCTTCCommand::TTC_START),      NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("STOP"      , amc->getTTCCounter(AMCTTCCommand::TTC_STOP),       NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("TEST_SYNC" , amc->getTTCCounter(AMCTTCCommand::TTC_TEST_SYNC),  NULL, GEMUpdateType::HW32);

  // DAQ link registers
  is_amc->createUInt32("CONTROL",           amc->getDAQLinkControl(),               NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("STATUS",            amc->getDAQLinkStatus(),                NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("INPUT_ENABLE_MASK", amc->getDAQLinkInputMask(),             NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("DAV_TIMEOUT",       amc->getDAQLinkDAVTimeout(),            NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("MAX_DAV_TIMER",     amc->getDAQLinkDAVTimer(0),             NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("LAST_DAV_TIMER",    amc->getDAQLinkDAVTimer(1),             NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("NOTINTABLE_ERR",    amc->getDAQLinkNonidentifiableErrors(), NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("DISPER_ERR",        amc->getDAQLinkDisperErrors(),          NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("EVT_SENT",          amc->getDAQLinkEventsSent(),            NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("L1AID",             amc->getDAQLinkL1AID(),                 NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("RUN_TYPE",          amc->getDAQLinkL1AID(),                 NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("RUN_PARAMS",        amc->getDAQLinkL1AID(),                 NULL, GEMUpdateType::HW32);

  is_amc->createUInt32("L1A_FIFO_DATA_CNT",      amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("DAQ_FIFO_DATA_CNT",      amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("L1A_FIFO_NEAR_FULL_CNT", amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("DAQ_FIFO_NEAR_FULL_CNT", amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("DAQ_ALMOST_FULL_CNT",    amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("TTS_WARN_CNT",           amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("DAQ_WORD_RATE",          amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);

  // TTC registers
  // FIXME NOT YET PRESENT // is_amc->createUInt32("TTC_CONTROL", amc->getTTCControl(),   NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("TTC_SPY"            , amc->getTTCSpyBuffer(), NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("MMCM_LOCKED"        , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("BC0_LOCKED"         , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("PHASE_LOCKED"       , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("SYNC_DONE"          , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("L1A_RATE"           , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("MMCM_UNLOCK_CNT"    , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("BC0_UNLOCK_CNT"     , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("PHASE_UNLOCK_CNT"   , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("PHASE_UNLOCK_TIME"  , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("SYNC_DONE_TIME"     , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("BC0_OVERFLOW_CNT"   , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("BC0_UNDERFLOW_CNT"  , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("PA_PLL_LOCK_WINDOW" , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("PA_PHASE_SHIFT_CNT" , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("PA_PLL_LOCK_CLOCKS" , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("PA_FSM_STATE"       , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("TTC_PM_PHASE"           , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("TTC_PM_PHASE_MEAN"      , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("TTC_PM_PHASE_MIN"       , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("TTC_PM_PHASE_MAX"       , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("TTC_PM_PHASE_JUMP_CNT"  , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("TTC_PM_PHASE_JUMP_SIZE" , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("TTC_PM_PHASE_JUMP_TIME" , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("GTH_PM_PHASE"           , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("GTH_PM_PHASE_MEAN"      , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("GTH_PM_PHASE_MIN"       , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("GTH_PM_PHASE_MAX"       , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("GTH_PM_PHASE_JUMP_CNT"  , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  // FIXME NOT YET PRESENT // is_amc->createUInt32("GTH_PM_PHASE_JUMP_SIZE" , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);
  is_amc->createUInt32("GTH_PM_PHASE_JUMP_TIME" , amc->getDAQLinkL1AID(), NULL, GEMUpdateType::HW32);

  // B-GO command words
  is_amc->createUInt32("BC0_CMD"       , amc->getTTCConfig(AMCTTCCommand::TTC_BC0),        NULL, GEMUpdateType::NOUPDATE);
  is_amc->createUInt32("EC0_CMD"       , amc->getTTCConfig(AMCTTCCommand::TTC_EC0),        NULL, GEMUpdateType::NOUPDATE);
  is_amc->createUInt32("RESYNC_CMD"    , amc->getTTCConfig(AMCTTCCommand::TTC_RESYNC),     NULL, GEMUpdateType::NOUPDATE);
  is_amc->createUInt32("OC0_CMD"       , amc->getTTCConfig(AMCTTCCommand::TTC_OC0),        NULL, GEMUpdateType::NOUPDATE);
  is_amc->createUInt32("HARD_RESET_CMD", amc->getTTCConfig(AMCTTCCommand::TTC_HARD_RESET), NULL, GEMUpdateType::NOUPDATE);
  is_amc->createUInt32("CALPULSE_CMD"  , amc->getTTCConfig(AMCTTCCommand::TTC_CALPULSE),   NULL, GEMUpdateType::NOUPDATE);
  is_amc->createUInt32("START_CMD"     , amc->getTTCConfig(AMCTTCCommand::TTC_START),      NULL, GEMUpdateType::NOUPDATE);
  is_amc->createUInt32("STOP_CMD"      , amc->getTTCConfig(AMCTTCCommand::TTC_STOP),       NULL, GEMUpdateType::NOUPDATE);
  is_amc->createUInt32("TEST_SYNC_CMD" , amc->getTTCConfig(AMCTTCCommand::TTC_TEST_SYNC),  NULL, GEMUpdateType::NOUPDATE);

  // TRIGGER registers
  for (uint8_t oh = 0; oh < amc->getSupportedOptoHybrids(); ++oh) {
    std::stringstream ohname;
    ohname << "OH" << (int)oh;
    is_amc->createUInt32(ohname.str()+"_STATUS",               amc->getLinkDAQStatus(oh),      NULL, GEMUpdateType::HW32);
    is_amc->createUInt32(ohname.str()+"_CORRUPT_VFAT_BLK_CNT", amc->getLinkDAQCounters(oh, 0), NULL, GEMUpdateType::HW32);
    is_amc->createUInt32(ohname.str()+"_EVN",                  amc->getLinkDAQCounters(oh, 1), NULL, GEMUpdateType::HW32);
    is_amc->createUInt32(ohname.str()+"_EOE_TIMEOUT",          amc->getDAQLinkDAVTimer(oh),    NULL, GEMUpdateType::HW32);
    is_amc->createUInt32(ohname.str()+"_MAX_EOE_TIMER",        amc->getDAQLinkDAVTimer(0),     NULL, GEMUpdateType::HW32);
    is_amc->createUInt32(ohname.str()+"_LAST_EOE_TIMER",       amc->getDAQLinkDAVTimer(1),     NULL, GEMUpdateType::HW32);

    is_amc->createUInt32(ohname.str()+"_TRIGGER_CNT",          amc->getDAQLinkDAVTimer(0),     NULL, GEMUpdateType::HW32);
    is_amc->createUInt32(ohname.str()+"_TRIGGER_RATE",         amc->getDAQLinkDAVTimer(0),     NULL, GEMUpdateType::HW32);

    for (int cluster = 0; cluster < 8; ++cluster) {
      std::stringstream cluname;
      cluname << "CLUSTER_SIZE_" << cluster;
      is_amc->createUInt32(ohname.str()+"_"+cluname.str()+"_RATE",
                            amc->getLinkDAQCounters(oh, 1), NULL, GEMUpdateType::HW32);
      is_amc->createUInt32(ohname.str()+"_"+cluname.str()+"_CNT",
                            amc->getLinkDAQCounters(oh, 1), NULL, GEMUpdateType::HW32);
      // cluname.str("");
      // cluname.clear();
      // cluname << "DEBUG_LAST_CLUSTER_" << cluster;
      // is_amc->createUInt32(ohname.str()+"_"+cluname.str(),
      //                       amc->getLinkDAQCounters(oh, 1), NULL, GEMUpdateType::HW32);
    }
  }
}

void gem::hw::amc::AMCManager::dumpAMCFIFO(xgi::Input* in, xgi::Output* out)
{
  dynamic_cast<AMCManagerWeb*>(p_gemWebInterface)->dumpAMCFIFO(in, out);
}
