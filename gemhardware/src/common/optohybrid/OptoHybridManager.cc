/**
 * class: OptoHybridManager
 * description: Manager application for OptoHybrid cards
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date:
 */

#include "gem/hw/optohybrid/OptoHybridManager.h"

#include "gem/hw/optohybrid/HwOptoHybrid.h"
#include "gem/hw/optohybrid/OptoHybridMonitor.h"
#include "gem/hw/optohybrid/OptoHybridManagerWeb.h"

#include "gem/hw/optohybrid/exception/Exception.h"

#include "gem/hw/vfat/HwVFAT2.h"
#include "gem/hw/utils/GEMCrateUtils.h"

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPConstants.h"
#include "xoap/SOAPBody.h"
#include "xoap/Method.h"
#include "xoap/AttachmentPart.h"
#include "xoap/domutils.h"


XDAQ_INSTANTIATOR_IMPL(gem::hw::optohybrid::OptoHybridManager);

gem::hw::optohybrid::OptoHybridManager::OptoHybridInfo::OptoHybridInfo()
{
  present  = false;
  crateID  = -1;
  slotID   = -1;
  linkID   = -1;
  cardName = "";

  vfatBroadcastList = "0-23";
  vfatBroadcastMask = 0xff000000;

  vfatSBitList = "0-23";
  vfatSBitMask = 0xff000000;

  triggerSource = 0;
  // sbitSource    = 0;
  refClkSrc     = 0;

  // vfatClkSrc    = 0;
  // cdceClkSrc    = 0;
}

void gem::hw::optohybrid::OptoHybridManager::OptoHybridInfo::registerFields(xdata::Bag<gem::hw::optohybrid::OptoHybridManager::OptoHybridInfo>* bag) {
  bag->addField("crateID",  &crateID);
  bag->addField("slot",     &slotID);
  bag->addField("link",     &linkID);
  bag->addField("present",  &present);

  bag->addField("CardName", &cardName); // FIXME OBSOLETE/UPDATE?

  bag->addField("VFATBroadcastList", &vfatBroadcastList); // FIXME OBSOLETE/UPDATE?
  bag->addField("VFATBroadcastMask", &vfatBroadcastMask); // FIXME OBSOLETE/UPDATE?

  bag->addField("VFATSBitList", &vfatSBitList); // FIXME OBSOLETE/UPDATE?
  bag->addField("VFATSBitMask", &vfatSBitMask); // FIXME OBSOLETE/UPDATE?

  bag->addField("triggerSource", &triggerSource); // FIXME OBSOLETE/UPDATE?
  bag->addField("refClkSrc",     &refClkSrc);     // FIXME OBSOLETE/UPDATE?

  // bag->addField("SBitConfig",         &sbitConfig); // FIXME OBSOLETE  USE DB CONFIG OBJECT
  // bag->addField("CommonVFATSettings", &commonVFATSettings); // FIXME OBSOLETE USE DB CONFIG OBJECT
}

gem::hw::optohybrid::OptoHybridManager::OptoHybridManager(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub)
{
  m_optohybridInfo.setSize(MAX_OPTOHYBRIDS_PER_AMC*MAX_AMCS_PER_CRATE);

  p_appInfoSpace->fireItemAvailable("AllOptoHybridsInfo", &m_optohybridInfo);
  // p_appInfoSpace->fireItemAvailable("AMCSlots",           &m_amcSlots);
  p_appInfoSpace->fireItemAvailable("ConnectionFile",     &m_connectionFile);

  p_appInfoSpace->addItemRetrieveListener("AllOptoHybridsInfo", this);
  // p_appInfoSpace->addItemRetrieveListener("AMCSlots",           this);
  p_appInfoSpace->addItemRetrieveListener("ConnectionFile",     this);
  p_appInfoSpace->addItemChangedListener( "AllOptoHybridsInfo", this);
  // p_appInfoSpace->addItemChangedListener( "AMCSlots",           this);
  p_appInfoSpace->addItemChangedListener( "ConnectionFile",     this);

  // initialize the OptoHybrid application objects
  CMSGEMOS_DEBUG("OptoHybridManager::Connecting to the OptoHybridManagerWeb interface");
  p_gemWebInterface = new gem::hw::optohybrid::OptoHybridManagerWeb(this);
  // p_gemMonitor      = new gem::hw::optohybrid::OptoHybridHwMonitor(this);
  CMSGEMOS_DEBUG("OptoHybridManager::done");

  // set up the info hwCfgInfoSpace
  init();

  // getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemhardware/images/optohybrid/OptoHybridManager.png");
}

gem::hw::optohybrid::OptoHybridManager::~OptoHybridManager() {
  // memory management, maybe not necessary here?
}

// This is the callback used for handling xdata:Event objects
void gem::hw::optohybrid::OptoHybridManager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    CMSGEMOS_DEBUG("OptoHybridManager::actionPerformed() setDefaultValues" <<
          "Default configuration values have been loaded from xml profile");

    // how to handle passing in various values nested in a vector in a bag
    for (auto board = m_optohybridInfo.begin(); board != m_optohybridInfo.end(); ++board) {
      // if (board->bag.present.value_) {
      if (board->bag.crateID.value_ > -1) {
        board->bag.present = true;
        CMSGEMOS_INFO("OptoHybridManager::Found attribute:" << board->bag.toString());
        uint32_t tmpBroadcastMask = gem::hw::utils::parseVFATMaskList(board->bag.vfatBroadcastList.toString(),
                                                                      m_gemLogger);
        CMSGEMOS_INFO("OptoHybridManager::Parsed vfatBroadcastList = " << board->bag.vfatBroadcastList.toString()
             << " to broadcastMask 0x" << std::hex << tmpBroadcastMask << std::dec);
        board->bag.vfatBroadcastMask = tmpBroadcastMask;
        // board->bag.vfatBroadcastMask.push_back(parseVFATMaskList(board->bag.vfatBroadcastList.toString()),
        //                                                          m_gemLogger);

        uint32_t tmpSBitMask = gem::hw::utils::parseVFATMaskList(board->bag.vfatSBitList.toString(),
                                                                 m_gemLogger);
        CMSGEMOS_INFO("OptoHybridManager::Parsed vfatSBitList = " << board->bag.vfatSBitList.toString()
             << " to sbitMask 0x" << std::hex << tmpSBitMask << std::dec);
        board->bag.vfatSBitMask = tmpSBitMask;
      }
    }
    // p_gemMonitor->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::optohybrid::OptoHybridManager::init()
{
}

// state transitions
void gem::hw::optohybrid::OptoHybridManager::initializeAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  CMSGEMOS_DEBUG("OptoHybridManager::initializeAction begin");
  // FIXME make me more streamlined
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    CMSGEMOS_DEBUG("OptoHybridManager::initializeAction looping over slots(" << (slot+1) << ") and finding expected cards");
    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      CMSGEMOS_DEBUG("OptoHybridManager::initializeAction looping over links(" << link << ") and finding expected cards");
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      CMSGEMOS_DEBUG("OptoHybridManager::initializeAction index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;
      CMSGEMOS_DEBUG("OptoHybridManager::initializeAction bag"
            << "crate " << info.crateID.value_
            << " slot " << info.slotID.value_
            << " link " << info.linkID.value_);

      if (!info.present)
        continue;

      CMSGEMOS_DEBUG("OptoHybridManager::initializeAction: info is: " << info.toString());
      CMSGEMOS_DEBUG("OptoHybridManager::initializeAction creating pointer to board connected on link "
            << link << " to AMC in slot " << (slot+1));
      std::string deviceName = info.cardName.toString();
      if (deviceName.empty())
        deviceName = toolbox::toString("gem-shelf%02d-amc%02d-optohybrid%02d",
                                       info.crateID.value_,
                                       info.slotID.value_,
                                       info.linkID.value_);
      toolbox::net::URN hwCfgURN("urn:gem:hw:"+deviceName);

      if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
        CMSGEMOS_DEBUG("OptoHybridManager::initializeAction::infospace " << hwCfgURN.toString() << " already exists, getting");
        is_optohybrids.at(slot).at(link) = is_toolbox_ptr(new gem::base::utils::GEMInfoSpaceToolBox(this,
                                                                                                    hwCfgURN.toString(),
                                                                                                    true));

      } else {
        CMSGEMOS_DEBUG("OptoHybridManager::initializeAction::infospace " << hwCfgURN.toString() << " does not exist, creating");
        is_optohybrids.at(slot).at(link) = is_toolbox_ptr(new gem::base::utils::GEMInfoSpaceToolBox(this,
                                                                                                    hwCfgURN.toString(),
                                                                                                    true));
      }

      try {
        CMSGEMOS_DEBUG("OptoHybridManager::initializeAction obtaining pointer to HwOptoHybrid " << deviceName
              << " (slot " << slot+1 << ")"
              << " (link " << link   << ")");
        m_optohybrids.at(slot).at(link) = optohybrid_shared_ptr(new gem::hw::optohybrid::HwOptoHybrid(deviceName,m_connectionFile.toString()));
      } catch (gem::hw::optohybrid::exception::Exception const& e) {
        std::stringstream msg;
        msg << "OptoHybridManager::initializeAction caught exception " << e.what();
        CMSGEMOS_ERROR(msg.str());
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, msg.str());
      } catch (toolbox::net::exception::MalformedURN const& e) {
        std::stringstream msg;
        msg << "OptoHybridManager::initializeAction caught exception " << e.what();
        CMSGEMOS_ERROR(msg.str());
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, msg.str());
      } catch (std::exception const& e) {
        std::stringstream msg;
        msg << "OptoHybridManager::initializeAction caught exception " << e.what();
        CMSGEMOS_ERROR(msg.str());
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, msg.str());
      }
      CMSGEMOS_DEBUG("OptoHybridManager::initializeAction connected");
      // set the web view to be empty or grey
      // if (!info.present.value_) continue;
      // p_gemWebInterface->optohybridInSlot(slot);

      CMSGEMOS_DEBUG("OptoHybridManager::initializeAction grabbing pointer to hardware device");
      optohybrid_shared_ptr optohybrid = m_optohybrids.at(slot).at(link);
      if (optohybrid->isHwConnected()) {
        // get connected VFATs
        m_vfatMapping.at(slot).at(link)   = optohybrid->getConnectedVFATs(true);
        CMSGEMOS_INFO("OptoHybridManager::initializeAction Obtained vfatMapping");
        // all the rest of these are related to the first by bitwise logic, can avoid doing the 4 calls
        m_trackingMask.at(slot).at(link)  = optohybrid->getConnectedVFATMask(true);
        CMSGEMOS_INFO("OptoHybridManager::initializeAction Obtained trackingMask");
        m_broadcastList.at(slot).at(link) = m_trackingMask.at(slot).at(link);
        CMSGEMOS_INFO("OptoHybridManager::initializeAction Obtained broadcastList");
        m_sbitMask.at(slot).at(link) = m_trackingMask.at(slot).at(link);
        CMSGEMOS_INFO("OptoHybridManager::initializeAction Obtained sbitMask");

        createOptoHybridInfoSpaceItems(is_optohybrids.at(slot).at(link), optohybrid);
        CMSGEMOS_INFO("OptoHybridManager::initializeAction looping over created VFAT devices");
        for (auto mapit = m_vfatMapping.at(slot).at(link).begin();
             mapit != m_vfatMapping.at(slot).at(link).end(); ++mapit) {
          CMSGEMOS_INFO("OptoHybridManager::initializeAction VFAT" << (int)mapit->first << " has chipID "
               << std::hex << (int)mapit->second << std::dec << " (from map)");
          // gem::hw::vfat::HwVFAT2& vfatDevice = optohybrid->getVFATDevice(mapit->first);
          // CMSGEMOS_INFO("OptoHybridManager::initializeAction VFAT" << (int)mapit->first << " has chipID "
          //      << std::hex << (int)vfatDevice.getChipID() << std::dec << " (from HW device) ");
        }

        if (!m_disableMonitoring) {
          m_optohybridMonitors.at(slot).at(link) = std::shared_ptr<OptoHybridMonitor>(new OptoHybridMonitor(optohybrid, this, index));
          m_optohybridMonitors.at(slot).at(link)->addInfoSpace("HWMonitoring", is_optohybrids.at(slot).at(link));
          m_optohybridMonitors.at(slot).at(link)->setupHwMonitoring();
          m_optohybridMonitors.at(slot).at(link)->startMonitoring();
        }

        CMSGEMOS_INFO("OptoHybridManager::initializeAction OptoHybrid connected on link "
             << link << " to AMC in slot " << (slot+1) << std::endl
             << "Tracking mask: 0x" << std::hex << std::setw(8) << std::setfill('0')
             << m_trackingMask.at(slot).at(link)
             << std::dec << std::endl
             << "Broadcst mask: 0x" << std::hex << std::setw(8) << std::setfill('0')
             << m_broadcastList.at(slot).at(link)
             << std::dec << std::endl
             << "    SBit mask: 0x" << std::hex << std::setw(8) << std::setfill('0')
             << m_sbitMask.at(slot).at(link)
             << std::dec << std::endl
             );
        optohybrid->setVFATMask(m_trackingMask.at(slot).at(link));
        optohybrid->setSBitMask(m_sbitMask.at(slot).at(link));
        // turn off any that are excluded by the additional mask?
      } else {
        std::stringstream msg;
        msg << "OptoHybridManager::initializeAction OptoHybrid connected on link "
            << link << " to AMC in slot " << (slot+1) << " is not responding";
        CMSGEMOS_ERROR(msg.str());
        // fireEvent("Fail");
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, msg.str());
      }
      // FOR MISHA
      // hardware should be connected, can update ldqm_db for teststand/local runs
    }
  }
  CMSGEMOS_INFO("OptoHybridManager::initializeAction end");
}

void gem::hw::optohybrid::OptoHybridManager::configureAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  CMSGEMOS_DEBUG("OptoHybridManager::configureAction");
  // std::ofstream of

  std::map<int,std::set<int> > hwMapping;
  // will the manager operate for all connected optohybrids, or only those connected to certain AMCs?
  // FIXME make me more streamlined
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    // usleep(10); // just for testing the timing of different applications
    uint32_t inputMask = 0x0;

    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link)
      if (m_optohybridMonitors.at(slot).at(link))
        m_optohybridMonitors.at(slot).at(link)->pauseMonitoring();

    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      // usleep(10); // just for testing the timing of different applications
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      CMSGEMOS_DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      CMSGEMOS_DEBUG("OptoHybridManager::configureAction::info is: " << info.toString());
      if (!info.present)
        continue;

      CMSGEMOS_DEBUG("OptoHybridManager::configureAction::grabbing pointer to hardware device");
      optohybrid_shared_ptr optohybrid = m_optohybrids.at(slot).at(link);

      if (optohybrid->isHwConnected()) {
        hwMapping[slot+1].insert(link);

        // FIXME UPDATE
        // CMSGEMOS_DEBUG("OptoHybridManager::configureAction::setting sbit source to 0x"
        //                << std::hex << info.sbitSource.value_ << std::dec);
        // optohybrid->setSBitSource(info.sbitSource.value_);
        // CMSGEMOS_DEBUG("OptoHybridManager::setting reference clock source to 0x"
        //                << std::hex << info.refClkSrc.value_ << std::dec);
        // optohybrid->setReferenceClock(info.refClkSrc.value_);

        // CMSGEMOS_DEBUG("OptoHybridManager::setting vfat clock source to 0x" << std::hex << info.vfatClkSrc.value_ << std::dec);
        // optohybrid->setVFATClock(info.vfatClkSrc.value_,);
        // CMSGEMOS_DEBUG("OptoHybridManager::setting cdce clock source to 0x" << std::hex << info.cdceClkSrc.value_ << std::dec);
        // optohybrid->setSBitSource(info.cdceClkSrc.value_);
        // for (unsigned olink = 0; olink < HwAMC::N_GTX; ++olink) {
        // }

        // FIXME UPDATE
        // CMSGEMOS_DEBUG("OptoHybridManager::configureAction Setting output s-bit configuration parameters");
        // optohybrid->setHDMISBitMode(info.sbitConfig.bag.Mode.value_);

        std::array<uint8_t, 8> sbitSources = {{
          }};

        optohybrid->setHDMISBitSource(sbitSources);

        std::vector<std::pair<uint8_t, uint32_t> > chipIDs = optohybrid->getConnectedVFATs();

        for (auto chip = chipIDs.begin(); chip != chipIDs.end(); ++chip) {
          if (chip->second) {
            CMSGEMOS_INFO("VFAT found in GEB slot " << std::setw(2) << static_cast<uint32_t>(chip->first) << " has ChipID "
                          << "0x" << std::hex << std::setw(4) << chip->second << std::dec);
          } else {
            CMSGEMOS_INFO("No VFAT found in GEB slot " << std::setw(2) << static_cast<uint32_t>(chip->first));
          }
        }

        uint32_t vfatMask = m_broadcastList.at(slot).at(link);
        CMSGEMOS_INFO("Setting VFAT parameters with broadcast write using mask " << std::hex << vfatMask << std::dec);

        // FIXME OBSOLETE/UPDATE?
        std::map<std::string, uint8_t> vfatSettings;

        if (m_scanType.value_ == 2) {
          CMSGEMOS_INFO("OptoHybridManager::configureAction configureAction: FIRST Latency  " << m_scanMin.value_);
          vfatSettings["Latency"    ] = (uint8_t)(m_scanMin.value_);
          // FIXME optohybrid->setVFATsToDefaults(vfatSettings, vfatMask);
          // HACK
          // have to enable the pulse to the channel if using cal pulse latency scan
          // but shouldn't mess with other settings... not possible here, so just a hack
          // optohybrid->broadcastWrite("VFATChannels.ChanReg23",  0x40, vfatMask);
          // optohybrid->broadcastWrite("VFATChannels.ChanReg124", 0x40, vfatMask);
          // optohybrid->broadcastWrite("VFATChannels.ChanReg65",  0x40, vfatMask);
          // optohybrid->broadcastWrite("VCal",                    0xaf, vfatMask);
        } else if (m_scanType.value_ == 3) {
          uint32_t initialVT1 = m_scanMin.value_;
          // uint32_t VT1 = (m_scanMax.value_ - m_scanMin.value_);
          uint32_t initialVT2 = 0; //std::max(0,(uint32_t)m_scanMax.value_);
          CMSGEMOS_INFO("OptoHybridManager::configureAction FIRST VT1 " << initialVT1 << " VT2 " << initialVT2);
          vfatSettings["VThreshold1"] = (uint8_t)(initialVT1&0xff);
          vfatSettings["VThreshold2"] = (uint8_t)(initialVT2&0xff);
          // FIXME optohybrid->setVFATsToDefaults(vfatSettings, vfatMask);
        } else {
          // FIXME optohybrid->setVFATsToDefaults(vfatSettings, vfatMask);
        }

        std::array<std::string, 11> setupregs = {{"ContReg0", "ContReg2", "IPreampIn", "IPreampFeed", "IPreampOut",
                                                  "IShaper", "IShaperFeed", "IComp", "Latency",
                                                  "VThreshold1", "VThreshold2"}};

        CMSGEMOS_INFO("Reading back values after setting defaults:");
        for (auto reg = setupregs.begin(); reg != setupregs.end(); ++reg) {
          std::vector<uint32_t> res = optohybrid->broadcastRead(*reg,vfatMask);
          CMSGEMOS_INFO(*reg);
          for (auto r = res.begin(); r != res.end(); ++r) {
            CMSGEMOS_INFO(" 0x" << std::hex << std::setw(8) << std::setfill('0') << *r << std::dec);
          }
        }

        // what else is required for configuring the OptoHybrid?
        // need to reset optical links?
        // reset counters?

        // FIXME, should not be here or done like this
        uint32_t gtxMask = optohybrid->readReg("GEM_AMC.DAQ.CONTROL.INPUT_ENABLE_MASK");
        std::stringstream msg;
        msg << "OptoHybridManager::configureAction::OptoHybrid connected on link " << (int)link
            << " to AMC in slot " << (int)(slot+1) << " found, INPUT_ENABLE_MASK changed from "
            << std::hex << gtxMask << std::dec;
        gtxMask   |= (0x1<<link);
        inputMask |= (0x1<<link);
        optohybrid->writeReg("GEM_AMC.DAQ.CONTROL.INPUT_ENABLE_MASK", gtxMask);
        optohybrid->writeReg("GEM_AMC.DAQ.CONTROL.INPUT_ENABLE_MASK", inputMask);
        msg << " to " << std::hex << inputMask  << std::dec << std::endl;
        CMSGEMOS_INFO(msg.str());
      } else {
        std::stringstream msg;
        msg << "OptoHybridManager::configureAction::OptoHybrid connected on link " << (int)link
            << " to AMC in slot " << (int)(slot+1) << " is not responding";
        CMSGEMOS_ERROR(msg.str());
        // fireEvent("Fail");
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, msg.str());
      }
    }

    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link)
      if (m_optohybridMonitors.at(slot).at(link))
        m_optohybridMonitors.at(slot).at(link)->resumeMonitoring();
  }

  CMSGEMOS_INFO("OptoHybridManager::configureAction end");
}

void gem::hw::optohybrid::OptoHybridManager::startAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  if (m_scanType.value_ == 2) {
    m_lastLatency = m_scanMin.value_;
    m_lastVT1 = 0;
  } else if (m_scanType.value_ == 3) {
    m_lastLatency = 0;
    m_lastVT1 = m_scanMin.value_;
  }

  CMSGEMOS_DEBUG("OptoHybridManager::startAction");
  // will the manager operate for all connected optohybrids, or only those connected to certain AMCs?
  // FIXME make me more streamlined
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    // usleep(10); // just for testing the timing of different applications
    uint32_t inputMask = 0x0;

    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link)
      if (m_optohybridMonitors.at(slot).at(link))
        m_optohybridMonitors.at(slot).at(link)->pauseMonitoring();

    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      // usleep(10); // just for testing the timing of different applications
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      CMSGEMOS_DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      if (!info.present)
        continue;

      CMSGEMOS_DEBUG("OptoHybridManager::startAction::grabbing pointer to hardware device");
      optohybrid_shared_ptr optohybrid = m_optohybrids.at(slot).at(link);

      if (optohybrid->isHwConnected()) {
        // turn on all VFATs? or should they always be on?
        uint32_t vfatMask = m_broadcastList.at(slot).at(link);
        // reset counters
        optohybrid->counterReset();
        // // reset VFAT counters
        // optohybrid->resetVFATCRCCount();

        std::vector<uint32_t> res = optohybrid->broadcastRead("ContReg0",vfatMask);
        CMSGEMOS_INFO("ContReg0: vfatMask = " << std::hex << std::setw(8) << std::setfill('0') << vfatMask);
        for (auto r = res.begin(); r != res.end(); ++r)
          CMSGEMOS_INFO(" 0x" << std::hex << std::setw(8) << std::setfill('0') << *r << std::dec);

        // perhaps don't hardcode the full control register here, simply turn on the run bit?
        optohybrid->broadcastWrite("ContReg0", 0x37, vfatMask);
        res.clear();
        res = optohybrid->broadcastRead("ContReg0",vfatMask);
        CMSGEMOS_INFO("OptoHybridManager::startAction ContReg0");
        for (auto r = res.begin(); r != res.end(); ++r)
          CMSGEMOS_INFO(" 0x" << std::hex << std::setw(8) << std::setfill('0') << *r << std::dec);

        // FIXME, should not be here or done like this
        uint32_t gtxMask = optohybrid->readReg("GEM_AMC.DAQ.CONTROL.INPUT_ENABLE_MASK");
        std::stringstream msg;
        msg << "OptoHybridManager::startAction::OptoHybrid connected on link " << (int)link
            << " to AMC in slot " << (int)(slot+1) << " found, INPUT_ENABLE_MASK changed from "
            << std::hex << gtxMask << std::dec;
        gtxMask   |= (0x1<<link);
        inputMask |= (0x1<<link);
        optohybrid->writeReg("GEM_AMC.DAQ.CONTROL.INPUT_ENABLE_MASK", gtxMask);
        optohybrid->writeReg("GEM_AMC.DAQ.CONTROL.INPUT_ENABLE_MASK", inputMask);
        msg.str("");
        msg.clear();
        msg << "OptoHybridManager::startAction::OptoHybrid connected on link " << (int)link
            << " to AMC in slot " << (int)(slot+1) << " found, starting run with INPUT_ENABLE_MASK "
	    << std::hex << inputMask << std::dec;
        CMSGEMOS_INFO(msg.str());
        // what resets to do
      } else {
        std::stringstream msg;
        msg << "OptoHybridManager::startAction::OptoHybrid connected on link " << (int)link
            << " to AMC in slot " << (int)(slot+1) << " is not responding";
        CMSGEMOS_ERROR(msg.str());
        // fireEvent("Fail");
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, msg.str());
      }
    }

    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link)
      if (m_optohybridMonitors.at(slot).at(link))
        m_optohybridMonitors.at(slot).at(link)->resumeMonitoring();
  }
  CMSGEMOS_INFO("OptoHybridManager::startAction end");
}

void gem::hw::optohybrid::OptoHybridManager::pauseAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  // put all connected VFATs into sleep mode?
  // FIXME make me more streamlined
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    // usleep(10); // just for testing the timing of different applications

    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link)
      if (m_optohybridMonitors.at(slot).at(link))
        m_optohybridMonitors.at(slot).at(link)->pauseMonitoring();

    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      // usleep(10); // just for testing the timing of different applications
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      CMSGEMOS_DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      if (!info.present)
        continue;

      CMSGEMOS_DEBUG("OptoHybridManager::pauseAction::grabbing pointer to hardware device");
      optohybrid_shared_ptr optohybrid = m_optohybrids.at(slot).at(link);

      if (optohybrid->isHwConnected()) {
        // turn on all VFATs? or should they always be on?
        uint32_t vfatMask = m_broadcastList.at(slot).at(link);
	if (m_scanType.value_ == 2) {
	  uint8_t updatedLatency = m_lastLatency + m_stepSize.value_;
	  CMSGEMOS_INFO("OptoHybridManager::LatencyScan OptoHybrid on link " << (int)link
	       << " AMC slot " << (slot+1) << " Latency  " << (int)updatedLatency);

          optohybrid->broadcastWrite("Latency", updatedLatency, vfatMask);
      } else if (m_scanType.value_ == 3) {
	  uint8_t updatedVT1 = m_lastVT1 + m_stepSize.value_;
	  uint8_t VT2 = 0;  // std::max(0,(int)m_scanMax.value_);
	  CMSGEMOS_INFO("OptoHybridManager::ThresholdScan OptoHybrid on link " << (int)link
	       << " AMC slot " << (slot+1) << " VT1 " << (int)updatedVT1
               << " VT2 " << VT2 << " StepSize " << m_stepSize.value_);

          optohybrid->broadcastWrite("VThreshold1", updatedVT1, vfatMask);
          optohybrid->broadcastWrite("VThreshold2", VT2, vfatMask);
	}
        // what resets to do
      } else {
        std::stringstream msg;
        msg << "OptoHybridManager::pauseAction OptoHybrid connected on link " << (int)link
            << " to AMC in slot " << (int)(slot+1) << " is not responding";
        CMSGEMOS_ERROR(msg.str());
        // fireEvent("Fail");
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, msg.str());
      }
    }

    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link)
      if (m_optohybridMonitors.at(slot).at(link))
        m_optohybridMonitors.at(slot).at(link)->resumeMonitoring();
  }
  // Update the scan parameters
  if (m_scanType.value_ == 2) {
    CMSGEMOS_INFO("OptoHybridManager::pauseAction LatencyScan old Latency " << (int)m_lastLatency);
    m_lastLatency += m_stepSize.value_;
    CMSGEMOS_INFO("OptoHybridManager::pauseAction LatencyScan new Latency " << (int)m_lastLatency);
  } else if (m_scanType.value_ == 3) {
    CMSGEMOS_INFO("OptoHybridManager::pauseAction ThresholdScan old VT1 " << (int)m_lastVT1);
    m_lastVT1 += m_stepSize.value_;
    CMSGEMOS_INFO("OptoHybridManager::pauseAction ThresholdScan new VT1 " << (int)m_lastVT1);
  }
  CMSGEMOS_INFO("OptoHybridManager::pauseAction end");
}

void gem::hw::optohybrid::OptoHybridManager::resumeAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  // put all connected VFATs into run mode?
  usleep(10);
  CMSGEMOS_INFO("OptoHybridManager::resumeAction end");
}

void gem::hw::optohybrid::OptoHybridManager::stopAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  CMSGEMOS_DEBUG("OptoHybridManager::stopAction");
  // will the manager operate for all connected optohybrids, or only those connected to certain AMCs?
  // FIXME make me more streamlined
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    // usleep(10); // just for testing the timing of different applications
    uint32_t inputMask = 0x0;

    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link)
      if (m_optohybridMonitors.at(slot).at(link))
        m_optohybridMonitors.at(slot).at(link)->pauseMonitoring();

    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      // usleep(10); // just for testing the timing of different applications
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      CMSGEMOS_DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      if (!info.present)
        continue;

      CMSGEMOS_DEBUG("OptoHybridManager::stopAction::grabbing pointer to hardware device");
      optohybrid_shared_ptr optohybrid = m_optohybrids.at(slot).at(link);

      if (optohybrid->isHwConnected()) {
        // put all connected VFATs into sleep mode?
        uint32_t vfatMask = m_broadcastList.at(slot).at(link);
        // perhaps don't hardcode the full control register here, simply turn off the run bit?
        optohybrid->broadcastWrite("ContReg0", 0x36, vfatMask);
        // what resets to do

        // FIXME OBSOLETE/UPDATE?
        std::map<std::string, uint8_t > vfatSettings;

	if (m_scanType.value_ == 2) {
	  // FIXME optohybrid->setVFATsToDefaults(vfatSettings, vfatMask);
          // HACK
          // have to disable the pulse to the channel if using cal pulse latency scan
          // but shouldn't mess with other settings... not possible here, so just a hack
          // optohybrid->broadcastWrite("VFATChannels.ChanReg23",  0x00, vfatMask);
          // optohybrid->broadcastWrite("VFATChannels.ChanReg124", 0x00, vfatMask);
          // optohybrid->broadcastWrite("VFATChannels.ChanReg65",  0x00, vfatMask);
          // optohybrid->broadcastWrite("VCal",                    0x00, vfatMask);
	} else if (m_scanType.value_ == 3) {
	  // FIXME optohybrid->setVFATsToDefaults(vfatSettings, vfatMask);
        }
      } else {
        std::stringstream msg;
        msg << "OptoHybridManager::stopAction::OptoHybrid connected on link " << (int)link
            << " to AMC in slot " << (int)(slot+1) << " is not responding";
        CMSGEMOS_ERROR(msg.str());
        // fireEvent("Fail");
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, msg.str());
      }
    }

    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link)
      if (m_optohybridMonitors.at(slot).at(link))
        m_optohybridMonitors.at(slot).at(link)->resumeMonitoring();
  }
  CMSGEMOS_INFO("OptoHybridManager::stopAction end");
}

void gem::hw::optohybrid::OptoHybridManager::haltAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  // put all connected VFATs into sleep mode?
  usleep(10);
  CMSGEMOS_INFO("OptoHybridManager::haltAction end");
}

void gem::hw::optohybrid::OptoHybridManager::resetAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  // unregister listeners and items in info spaces
  CMSGEMOS_DEBUG("OptoHybridManager::resetAction begin");
  // FIXME make me more streamlined
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    // usleep(10);
    CMSGEMOS_DEBUG("OptoHybridManager::looping over slots(" << (slot+1) << ") and finding expected cards");
    uint32_t inputMask = 0x0;

    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link)
      if (m_optohybridMonitors.at(slot).at(link))
        m_optohybridMonitors.at(slot).at(link)->pauseMonitoring();

    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      // usleep(10);
      CMSGEMOS_DEBUG("OptoHybridManager::looping over links(" << link << ") and finding expected cards");
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      CMSGEMOS_DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      if (!info.present)
        continue;
      // set up the info space here rather than in initialize (where it can then get unset in reset?
      // should a value be set up for all of them by default?

      // reset the hw monitor
      if (m_optohybridMonitors.at(slot).at(link))
        m_optohybridMonitors.at(slot).at(link)->reset();

      CMSGEMOS_DEBUG("OptoHybridManager::revoking hwCfgInfoSpace items for board connected on link "
            << link << " to AMC in slot " << (slot+1));
      toolbox::net::URN hwCfgURN("urn:gem:hw:"+toolbox::toString("gem-shelf%02d-amc%02d-optohybrid%02d",
                                                                 info.crateID.value_,
                                                                 info.slotID.value_,
                                                                 info.linkID.value_));
      if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
        CMSGEMOS_DEBUG("OptoHybridManager::revoking config parameters into infospace");

        // reset the hw infospace toolbox
        is_optohybrids.at(slot).at(link)->reset();
      } else {
        CMSGEMOS_DEBUG("OptoHybridManager::resetAction::infospace " << hwCfgURN.toString() << " does not exist, no further action");
        continue;
      }
    }  // end loop on link < MAX_OPTOHYBRIDS_PER_AMC
  }  // end loop on slot < MAX_AMCS_PER_CRATE
  CMSGEMOS_INFO("OptoHybridManager::resetAction end");
}

/*
void gem::hw::optohybrid::OptoHybridManager::noAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
}
*/

void gem::hw::optohybrid::OptoHybridManager::failAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}

void gem::hw::optohybrid::OptoHybridManager::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}

void gem::hw::optohybrid::OptoHybridManager::createOptoHybridInfoSpaceItems(is_toolbox_ptr is_optohybrid,
                                                                            optohybrid_shared_ptr optohybrid)
{
  // system registers
  is_optohybrid->createUInt32("VFAT_Mask",    optohybrid->getVFATMask(),        NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("SBit_Mask",    optohybrid->getSBitMask(),        NULL, GEMUpdateType::HW32);
  // probably want this to be a set of 8?
  // is_optohybrid->createUInt32("HDMI SBitsOut", optohybrid->getHDMISBitSource(),  NULL, GEMUpdateType::HW32);
  // is_optohybrid->createUInt32("HDMI SBitMode", optohybrid->getHDMISBitMode(),    NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("CLK_STATUS",       optohybrid->getClockStatus(),  NULL, GEMUpdateType::HW32);

  is_optohybrid->createUInt32("FIRMWARE_DATE", optohybrid->getFirmwareDate(),
                              NULL, GEMUpdateType::PROCESS, "docstring", "fwdateoh");
  is_optohybrid->createUInt32("FIRMWARE_VERSION", optohybrid->getFirmwareVersion(),
                              NULL, GEMUpdateType::PROCESS, "docstring", "fwveroh");

  std::array<std::string, 2> ttcsources = {{"GBT_TTC","LOCAL_TTC"}};
  for (auto const& ttcsrc : ttcsources) {
    is_optohybrid->createUInt32((ttcsrc)+"L1A",    optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32((ttcsrc)+"Resync", optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32((ttcsrc)+"BC0",    optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32((ttcsrc)+"BXN",    optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  }

  // FIXME UPDATE REMOVE?
  is_optohybrid->createUInt32("GTX_TrackingLinkErrors", optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("GTX_TriggerLinkErrors",  optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("GTX_DataPackets",        optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("GBT_TrackingLinkErrors", optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("GBT_DataPackets",        optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
}
