/**
 * class: OptoHybridManager
 * description: Manager application for OptoHybrid cards
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date:
 */

#include "gem/hw/managers/optohybrid/OptoHybridManager.h"

#include "gem/hw/devices/optohybrid/HwOptoHybrid.h"
#include "gem/hw/managers/optohybrid/OptoHybridManagerWeb.h"

#include "gem/hw/managers/optohybrid/exception/Exception.h"
#include "gem/hw/managers/exception/Exception.h"
#include "gem/hw/devices/exception/Exception.h"
#include "gem/utils/exception/Exception.h"

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
}

void gem::hw::optohybrid::OptoHybridManager::OptoHybridInfo::registerFields(xdata::Bag<gem::hw::optohybrid::OptoHybridManager::OptoHybridInfo>* bag) {
  bag->addField("crateID",  &crateID);
  bag->addField("slot",     &slotID);
  bag->addField("link",     &linkID);
  bag->addField("present",  &present);
}

gem::hw::optohybrid::OptoHybridManager::OptoHybridManager(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub)
{
  m_optohybridInfo.setSize(MAX_OPTOHYBRIDS_PER_AMC*MAX_AMCS_PER_CRATE);

  p_appInfoSpace->fireItemAvailable("AllOptoHybridsInfo", &m_optohybridInfo);

  p_appInfoSpace->addItemRetrieveListener("AllOptoHybridsInfo", this);
  p_appInfoSpace->addItemChangedListener( "AllOptoHybridsInfo", this);

  // initialize the OptoHybrid application objects
  CMSGEMOS_DEBUG("OptoHybridManager::Connecting to the OptoHybridManagerWeb interface");
  p_gemWebInterface = new gem::hw::optohybrid::OptoHybridManagerWeb(this);
  CMSGEMOS_DEBUG("OptoHybridManager::done");

  // set up the info hwCfgInfoSpace
  init();

}

gem::hw::optohybrid::OptoHybridManager::~OptoHybridManager() {
  // memory management, maybe not necessary here?
}

// This is the callback used for handling xdata:Event objects
void gem::hw::optohybrid::OptoHybridManager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    CMSGEMOS_DEBUG("OptoHybridManager::actionPerformed() setDefaultValues"
                   << "Default configuration values have been loaded from xml profile");

    // how to handle passing in various values nested in a vector in a bag
    for (auto& board : m_optohybridInfo) {
      // if (board->bag.present.value_) {
      auto& cfgbag = board.bag;
      if (cfgbag.crateID.value_ > -1) {
        cfgbag.present = true;
        CMSGEMOS_INFO("OptoHybridManager::Found attribute:" << cfgbag.toString());
      }
    }
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::optohybrid::OptoHybridManager::init()
{
}

// state transitions
void gem::hw::optohybrid::OptoHybridManager::initializeAction()
{
  CMSGEMOS_DEBUG("OptoHybridManager::initializeAction begin");
  // FIXME make me more streamlined
  for (size_t slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    CMSGEMOS_DEBUG("OptoHybridManager::initializeAction looping over slots("
                   << (slot+1) << ") and finding expected cards");
    for (size_t link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      CMSGEMOS_DEBUG("OptoHybridManager::initializeAction looping over links("
                     << link << ") and finding expected cards");
      size_t index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
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
      std::string deviceName = toolbox::toString("gem-shelf%02d-amc%02d-optohybrid%02d",
                                                 info.crateID.value_,
                                                 info.slotID.value_,
                                                 info.linkID.value_);

      toolbox::net::URN hwCfgURN("urn:gem:hw:"+deviceName);

      if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
        CMSGEMOS_DEBUG("OptoHybridManager::initializeAction::infospace "
                       << hwCfgURN.toString() << " already exists, getting");
        is_optohybrids.at(slot).at(link) = std::make_shared<gem::base::utils::GEMInfoSpaceToolBox>(this,
                                                                                                   hwCfgURN.toString(),
                                                                                                   true);

      } else {
        CMSGEMOS_DEBUG("OptoHybridManager::initializeAction::infospace "
                       << hwCfgURN.toString() << " does not exist, creating");
        is_optohybrids.at(slot).at(link) = std::make_shared<gem::base::utils::GEMInfoSpaceToolBox>(this,
                                                                                                   hwCfgURN.toString(),
                                                                                                   true);
      }

      try {
        CMSGEMOS_DEBUG("OptoHybridManager::initializeAction obtaining pointer to HwOptoHybrid " << deviceName
                       << " (slot " << slot+1 << ")"
                       << " (link " << link   << ")");
        m_optohybrids.at(slot).at(link) = std::make_shared<gem::hw::optohybrid::HwOptoHybrid>(deviceName);
        auto&& optohybrid = m_optohybrids.at(slot).at(link);
        createOptoHybridInfoSpaceItems(is_optohybrids.at(slot).at(link), optohybrid);
      } GEM_HW_TRANSITION_CATCH("OptoHybridManager::initializeAction",gem::hw::devices::exception::Exception); 
      // set the web view to be empty or grey
      // if (!info.present.value_) continue;
      // p_gemWebInterface->optohybridInSlot(slot);

    }
  }
  CMSGEMOS_INFO("OptoHybridManager::initializeAction end");
}

void gem::hw::optohybrid::OptoHybridManager::configureAction()
{
  CMSGEMOS_DEBUG("OptoHybridManager::configureAction");
  // FIXME make me more streamlined
  for (size_t slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    for (size_t link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      size_t index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      CMSGEMOS_DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      CMSGEMOS_DEBUG("OptoHybridManager::configureAction::info is: " << info.toString());
      if (!info.present)
        continue;

      CMSGEMOS_DEBUG("OptoHybridManager::configureAction::grabbing pointer to hardware device");
      auto&& optohybrid = m_optohybrids.at(slot).at(link);

      if (optohybrid->isHwConnected()) {
        CMSGEMOS_INFO("OptoHybridManager::configureAction:: configuring VFATs");
        optohybrid->configureVFATs();
        // what else is required for configuring the OptoHybrid?
        // need to reset optical links?
        // reset counters?
      } else {
        std::stringstream errmsg;
        errmsg << "OptoHybridManager::configureAction::OptoHybrid connected on link " << static_cast<uint32_t>(link)
               << " to AMC in slot " << static_cast<uint32_t>(slot+1) << " is not responding";
        CMSGEMOS_ERROR(errmsg.str());
        // fireEvent("Fail");
        XCEPT_RAISE(gem::hw::managers::optohybrid::exception::Exception, errmsg.str());
      }
    }
  }
  CMSGEMOS_INFO("OptoHybridManager::configureAction end");
}

void gem::hw::optohybrid::OptoHybridManager::startAction()
{
  CMSGEMOS_DEBUG("OptoHybridManager::startAction");
  // will the manager operate for all connected optohybrids, or only those connected to certain AMCs?
  // FIXME make me more streamlined
  for (size_t slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    for (size_t link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      size_t index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      CMSGEMOS_DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;
      if (!info.present)
        continue;
 
      CMSGEMOS_DEBUG("OptoHybridManager::startAction::grabbing pointer to hardware device");
      auto&& optohybrid = m_optohybrids.at(slot).at(link);

      if (optohybrid->isHwConnected()) {
        // reset counters
        optohybrid->counterReset();
        //FIXME set VFATs to run mode
        //Potentially merge the counter reset in one RPC call

      } else {
        std::stringstream errmsg;
        errmsg << "OptoHybridManager::startAction::OptoHybrid connected on link " << static_cast<uint32_t>(link)
               << " to AMC in slot " << static_cast<uint32_t>(slot+1) << " is not responding";
        CMSGEMOS_ERROR(errmsg.str());
        // fireEvent("Fail");
        XCEPT_RAISE(gem::hw::managers::optohybrid::exception::Exception, errmsg.str());
      }
    }
  }
  CMSGEMOS_INFO("OptoHybridManager::startAction end");
}

void gem::hw::optohybrid::OptoHybridManager::pauseAction()
{
  // FIXME put all connected VFATs into run mode?
  CMSGEMOS_INFO("OptoHybridManager::pauseAction end");
}

void gem::hw::optohybrid::OptoHybridManager::resumeAction()
{
  // FIXME put all connected VFATs into run mode?
  CMSGEMOS_INFO("OptoHybridManager::resumeAction end");
}

void gem::hw::optohybrid::OptoHybridManager::stopAction()
{
  CMSGEMOS_DEBUG("OptoHybridManager::stopAction");
  // FIXME will the manager operate for all connected optohybrids, or only those connected to certain AMCs?
  // FIXME TODO make me more streamlined
  for (size_t slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {

    for (size_t link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      size_t index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      CMSGEMOS_DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      if (!info.present)
        continue;

      CMSGEMOS_DEBUG("OptoHybridManager::stopAction::grabbing pointer to hardware device");
      auto&& optohybrid = m_optohybrids.at(slot).at(link);

      if (optohybrid->isHwConnected()) {
        uint32_t vfatMask = m_broadcastList.at(slot).at(link);
        optohybrid->broadcastWrite("CFG_RUN", 0x0, vfatMask);
        // FIXME what resets to do
      }
    }
  }
  CMSGEMOS_INFO("OptoHybridManager::stopAction end");
}

void gem::hw::optohybrid::OptoHybridManager::haltAction()
{
  // put all connected VFATs into sleep mode?
  usleep(10);
  CMSGEMOS_INFO("OptoHybridManager::haltAction end");
}

void gem::hw::optohybrid::OptoHybridManager::resetAction()
{
  // unregister listeners and items in info spaces
  CMSGEMOS_DEBUG("OptoHybridManager::resetAction begin");
  // FIXME make me more streamlined
  for (size_t slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    CMSGEMOS_DEBUG("OptoHybridManager::looping over slots(" << (slot+1) << ") and finding expected cards");

    for (size_t link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      CMSGEMOS_DEBUG("OptoHybridManager::looping over links(" << link << ") and finding expected cards");
      size_t index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      CMSGEMOS_DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      if (!info.present)
        continue;
      // set up the info space here rather than in initialize (where it can then get unset in reset?
      // should a value be set up for all of them by default?

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
{
}
*/

void gem::hw::optohybrid::OptoHybridManager::failAction(toolbox::Event::Reference e)
{
}

void gem::hw::optohybrid::OptoHybridManager::resetAction(toolbox::Event::Reference e)
{
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

  for (auto const& ttcsrc : std::array<std::string const, 2>({{"GBT_TTC","LOCAL_TTC"}}) ) {
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
