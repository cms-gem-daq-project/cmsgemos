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

#include "gem/hw/utils/GEMCrateUtils.h"

XDAQ_INSTANTIATOR_IMPL(gem::hw::optohybrid::OptoHybridManager);

gem::hw::optohybrid::OptoHybridManager::OptoHybridInfo::OptoHybridInfo() {
  present  = false;
  crateID  = -1;
  slotID   = -1;
  linkID   = -1;
  cardName = "";

  vfatBroadcastList = "0-23";
  vfatBroadcastMask = 0xff000000;

  vfatSBitList = "0-23";
  vfatSBitMask = 0xff000000;

  controlHubAddress = "";
  deviceIPAddress   = "";
  ipBusProtocol     = "";
  addressTable      = "";
  controlHubPort    = 0;
  ipBusPort         = 0;

  triggerSource = 0;
  sbitSource    = 0;
  refClkSrc     = 0;
  //vfatClkSrc    = 0;
  //cdceClkSrc    = 0;
}

void gem::hw::optohybrid::OptoHybridManager::OptoHybridInfo::registerFields(xdata::Bag<gem::hw::optohybrid::OptoHybridManager::OptoHybridInfo>* bag) {
  bag->addField("crateID",  &crateID);
  bag->addField("slot",     &slotID);
  bag->addField("link",     &linkID);
  bag->addField("present",  &present);
  bag->addField("CardName", &cardName);

  bag->addField("ControlHubAddress", &controlHubAddress);
  bag->addField("DeviceIPAddress",   &deviceIPAddress);
  bag->addField("IPBusProtocol",     &ipBusProtocol);
  bag->addField("AddressTable",      &addressTable);
  bag->addField("ControlHubPort",    &controlHubPort);
  bag->addField("IPBusPort",         &ipBusPort);

  bag->addField("VFATBroadcastList", &vfatBroadcastList);
  bag->addField("VFATBroadcastMask", &vfatBroadcastMask);

  bag->addField("VFATSBitList", &vfatSBitList);
  bag->addField("VFATSBitMask", &vfatSBitMask);

  bag->addField("triggerSource", &triggerSource);
  bag->addField("sbitSource",    &sbitSource);
  bag->addField("refClkSrc",     &refClkSrc);
  //bag->addField("vfatClkSrc",    &vfatClkSrc);
  //bag->addField("cdceClkSrc",    &cdceClkSrc);
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

  //initialize the OptoHybrid application objects
  DEBUG("OptoHybridManager::Connecting to the OptoHybridManagerWeb interface");
  p_gemWebInterface = new gem::hw::optohybrid::OptoHybridManagerWeb(this);
  //p_gemMonitor      = new gem::hw::optohybrid::OptoHybridHwMonitor(this);
  DEBUG("OptoHybridManager::done");

  //set up the info hwCfgInfoSpace
  init();

  //getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemhardware/images/optohybrid/OptoHybridManager.png");
}

gem::hw::optohybrid::OptoHybridManager::~OptoHybridManager() {
  //memory management, maybe not necessary here?
}

// This is the callback used for handling xdata:Event objects
void gem::hw::optohybrid::OptoHybridManager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("OptoHybridManager::actionPerformed() setDefaultValues" <<
          "Default configuration values have been loaded from xml profile");

    //how to handle passing in various values nested in a vector in a bag
    for (auto board = m_optohybridInfo.begin(); board != m_optohybridInfo.end(); ++board) {
      // if (board->bag.present.value_) {
      if (board->bag.crateID.value_ > -1) {
        board->bag.present = true;
        INFO("OptoHybridManager::Found attribute:" << board->bag.toString());
        uint32_t tmpBroadcastMask = gem::hw::utils::parseVFATMaskList(board->bag.vfatBroadcastList.toString());
        INFO("OptoHybridManager::Parsed vfatBroadcastList = " << board->bag.vfatBroadcastList.toString()
             << " to broadcastMask 0x" << std::hex << tmpBroadcastMask << std::dec);
        board->bag.vfatBroadcastMask = tmpBroadcastMask;
        //board->bag.vfatBroadcastMask.push_back(parseVFATMaskList(board->bag.vfatBroadcastList.toString()));

        uint32_t tmpSBitMask = gem::hw::utils::parseVFATMaskList(board->bag.vfatSBitList.toString());
        INFO("OptoHybridManager::Parsed vfatSBitList = " << board->bag.vfatSBitList.toString()
             << " to sbitMask 0x" << std::hex << tmpSBitMask << std::dec);
        board->bag.vfatSBitMask = tmpSBitMask;
      }
    }
    //p_gemMonitor->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::optohybrid::OptoHybridManager::init()
{
}

//state transitions
void gem::hw::optohybrid::OptoHybridManager::initializeAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  DEBUG("OptoHybridManager::initializeAction begin");
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    usleep(1000);
    DEBUG("OptoHybridManager::looping over slots(" << (slot+1) << ") and finding expected cards");
    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      usleep(1000);
      DEBUG("OptoHybridManager::looping over links(" << link << ") and finding expected cards");
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;
      DEBUG("OptoHybridManager::bag"
            << "crate " << info.crateID.value_
            << " slot " << info.slotID.value_
            << " link " << info.linkID.value_);

      if (!info.present)
        continue;

      DEBUG("OptoHybridManager::line 118: info is: " << info.toString());
      DEBUG("OptoHybridManager::creating pointer to board connected on link " << link << " to GLIB in slot " << (slot+1));
      std::string deviceName = info.cardName.toString();
      if (deviceName.empty())
        deviceName = toolbox::toString("gem.shelf%02d.glib%02d.optohybrid%02d",
                                       info.crateID.value_,
                                       info.slotID.value_,
                                       info.linkID.value_);
      toolbox::net::URN hwCfgURN("urn:gem:hw:"+deviceName);

      if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
        DEBUG("OptoHybridManager::initializeAction::infospace " << hwCfgURN.toString() << " already exists, getting");
        is_optohybrids.at(slot).at(link) = is_toolbox_ptr(new gem::base::utils::GEMInfoSpaceToolBox(this,
                                                                                                    hwCfgURN.toString(),
                                                                                                    true));
      } else {
        DEBUG("OptoHybridManager::initializeAction::infospace " << hwCfgURN.toString() << " does not exist, creating");
        is_optohybrids.at(slot).at(link) = is_toolbox_ptr(new gem::base::utils::GEMInfoSpaceToolBox(this,
                                                                                                    hwCfgURN.toString(),
                                                                                                    true));
      }

      DEBUG("OptoHybridManager::exporting config parameters into infospace");
      is_optohybrids.at(slot).at(link)->createString("ControlHubAddress", info.controlHubAddress.value_, &(info.controlHubAddress),
                                                     GEMUpdateType::NOUPDATE);
      is_optohybrids.at(slot).at(link)->createString("IPBusProtocol",     info.ipBusProtocol.value_    , &(info.ipBusProtocol),
                                                     GEMUpdateType::NOUPDATE);
      is_optohybrids.at(slot).at(link)->createString("DeviceIPAddress",   info.deviceIPAddress.value_  , &(info.deviceIPAddress),
                                                     GEMUpdateType::NOUPDATE);
      is_optohybrids.at(slot).at(link)->createString("AddressTable",      info.addressTable.value_     , &(info.addressTable),
                                                     GEMUpdateType::NOUPDATE);
      is_optohybrids.at(slot).at(link)->createUInt32("ControlHubPort",    info.controlHubPort.value_   , &(info.controlHubPort),
                                                     GEMUpdateType::NOUPDATE);
      is_optohybrids.at(slot).at(link)->createUInt32("IPBusPort",         info.ipBusPort.value_        , &(info.ipBusPort),
                                                     GEMUpdateType::NOUPDATE);

      DEBUG("OptoHybridManager::InfoSpace found item: ControlHubAddress "
            << is_optohybrids.at(slot).at(link)->getString("ControlHubAddress"));
      DEBUG("OptoHybridManager::InfoSpace found item: IPBusProtocol "
            << is_optohybrids.at(slot).at(link)->getString("IPBusProtocol")    );
      DEBUG("OptoHybridManager::InfoSpace found item: DeviceIPAddress "
            << is_optohybrids.at(slot).at(link)->getString("DeviceIPAddress")  );
      DEBUG("OptoHybridManager::InfoSpace found item: AddressTable "
            << is_optohybrids.at(slot).at(link)->getString("AddressTable")     );
      DEBUG("OptoHybridManager::InfoSpace found item: ControlHubPort "
            << is_optohybrids.at(slot).at(link)->getUInt32("ControlHubPort")   );
      DEBUG("OptoHybridManager::InfoSpace found item: IPBusPort "
            << is_optohybrids.at(slot).at(link)->getUInt32("IPBusPort")        );

      try {
        DEBUG("OptoHybridManager::obtaining pointer to HwOptoHybrid " << deviceName
              << " (slot " << slot+1 << ")"
              << " (link " << link   << ")");
        m_optohybrids.at(slot).at(link) = optohybrid_shared_ptr(new gem::hw::optohybrid::HwOptoHybrid(deviceName,m_connectionFile.toString()));
      } catch (gem::hw::optohybrid::exception::Exception const& ex) {
        ERROR("OptoHybridManager::caught exception " << ex.what());
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, "initializeAction failed");
      } catch (toolbox::net::exception::MalformedURN const& ex) {
        ERROR("OptoHybridManager::caught exception " << ex.what());
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, "initializeAction failed");
      } catch (std::exception const& ex) {
        ERROR("OptoHybridManager::caught exception " << ex.what());
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, "initializeAction failed");
      }
      DEBUG("OptoHybridManager::connected");
      // set the web view to be empty or grey
      // if (!info.present.value_) continue;
      // p_gemWebInterface->optohybridInSlot(slot);

      DEBUG("OptoHybridManager::grabbing pointer to hardware device");
      // optohybrid_shared_ptr optohybrid = m_optohybrids.at(slot).at(link);
      if (m_optohybrids.at(slot).at(link)->isHwConnected()) {
        // get connected VFATs
        m_vfatMapping.at(slot).at(link)   = m_optohybrids.at(slot).at(link)->getConnectedVFATs();
        m_trackingMask.at(slot).at(link)  = m_optohybrids.at(slot).at(link)->getConnectedVFATMask();
        m_broadcastList.at(slot).at(link) = m_optohybrids.at(slot).at(link)->getConnectedVFATMask();
        m_sbitMask.at(slot).at(link)      = m_optohybrids.at(slot).at(link)->getConnectedVFATMask();

        createOptoHybridInfoSpaceItems(is_optohybrids.at(slot).at(link), m_optohybrids.at(slot).at(link));

        m_optohybridMonitors.at(slot).at(link) = std::shared_ptr<OptoHybridMonitor>(new OptoHybridMonitor(m_optohybrids.at(slot).at(link), this, index));
        m_optohybridMonitors.at(slot).at(link)->addInfoSpace("HWMonitoring", is_optohybrids.at(slot).at(link));
        m_optohybridMonitors.at(slot).at(link)->setupHwMonitoring();
        m_optohybridMonitors.at(slot).at(link)->startMonitoring();

        DEBUG("OptoHybridManager::OptoHybrid connected on link " << link << " to GLIB in slot " << (slot+1) << std::endl
              << "Tracking mask: 0x" << std::hex << std::setw(8) << std::setfill('0') << m_trackingMask.at(slot).at(link)
              << std::dec << std::endl
              << "Broadcst mask: 0x" << std::hex << std::setw(8) << std::setfill('0') << m_broadcastList.at(slot).at(link)
              << std::dec << std::endl
              << "    SBit mask: 0x" << std::hex << std::setw(8) << std::setfill('0') << m_sbitMask.at(slot).at(link)
              << std::dec << std::endl
              );
        // turn off any that are excluded by the additional mask?
      } else {
        ERROR("OptoHybridManager::OptoHybrid connected on link " << link << " to GLIB in slot " << (slot+1) << " is not responding");
        fireEvent("Fail");
        //maybe raise exception so as to not continue with other cards? let's just return for the moment
        return;
      }
    }
  }
  DEBUG("OptoHybridManager::initializeAction end");
}

void gem::hw::optohybrid::OptoHybridManager::configureAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  DEBUG("OptoHybridManager::configureAction");
  //will the manager operate for all connected optohybrids, or only those connected to certain GLIBs?
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    usleep(1000); // just for testing the timing of different applications
    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      usleep(1000); // just for testing the timing of different applications
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      DEBUG("OptoHybridManager::configureAction::info is: " << info.toString());
      if (!info.present)
        continue;

      DEBUG("OptoHybridManager::configureAction::grabbing pointer to hardware device");
      optohybrid_shared_ptr optohybrid = m_optohybrids.at(slot).at(link);

      if (optohybrid->isHwConnected()) {
        DEBUG("OptoHybridManager::configureAction::setting trigger source to 0x"
             << std::hex << info.triggerSource.value_ << std::dec);
        optohybrid->setTrigSource(info.triggerSource.value_);
        DEBUG("OptoHybridManager::configureAction::setting sbit source to 0x"
             << std::hex << info.sbitSource.value_ << std::dec);
        optohybrid->setSBitSource(info.sbitSource.value_);
        DEBUG("OptoHybridManager::setting reference clock source to 0x"
             << std::hex << info.refClkSrc.value_ << std::dec);
        optohybrid->setReferenceClock(info.refClkSrc.value_);
        /*
        DEBUG("OptoHybridManager::setting vfat clock source to 0x" << std::hex << info.vfatClkSrc.value_ << std::dec);
        optohybrid->setVFATClock(info.vfatClkSrc.value_,);
        DEBUG("OptoHybridManager::setting cdce clock source to 0x" << std::hex << info.cdceClkSrc.value_ << std::dec);
        optohybrid->setSBitSource(info.cdceClkSrc.value_);
        */
        /*
        for (unsigned olink = 0; olink < HwGLIB::N_GTX; ++olink) {
        }
        */

        std::vector<std::pair<uint8_t,uint32_t> > chipIDs = optohybrid->getConnectedVFATs();
        for (auto chip = chipIDs.begin(); chip != chipIDs.end(); ++chip)
          if (chip->second)
            INFO("VFAT found in GEB slot " << std::setw(2) << (int)chip->first << " has ChipID "
                 << "0x" << std::hex << std::setw(4) << chip->second << std::dec);
          else
            INFO("No VFAT found in GEB slot " << std::setw(2) << (int)chip->first);

        uint32_t vfatMask = optohybrid->getConnectedVFATMask();

        //optohybrid->broadcastWrite("Latency",     ~vfatMask, 157);
        //optohybrid->broadcastWrite("VThreshold1", ~vfatMask, 50);

        //what else is required for configuring the OptoHybrid?
        //need to reset optical links?
        //reset counters?
      } else {
        ERROR("configureAction::OptoHybrid connected on link " << (int)link << " to GLIB in slot " << (int)(slot+1)
              << " is not responding");
        fireEvent("Fail");
        //maybe raise exception so as to not continue with other cards?
      }
    }
  }

  DEBUG("OptoHybridManager::configureAction end");
}

void gem::hw::optohybrid::OptoHybridManager::startAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  // put all connected VFATs into run mode?
  usleep(1000);
}

void gem::hw::optohybrid::OptoHybridManager::pauseAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  // put all connected VFATs into sleep mode?
  usleep(1000);
}

void gem::hw::optohybrid::OptoHybridManager::resumeAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  // put all connected VFATs into run mode?
  usleep(1000);
}

void gem::hw::optohybrid::OptoHybridManager::stopAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  // put all connected VFATs into sleep mode?
  usleep(1000);
}

void gem::hw::optohybrid::OptoHybridManager::haltAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  // put all connected VFATs into sleep mode?
  usleep(1000);
}

void gem::hw::optohybrid::OptoHybridManager::resetAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  //unregister listeners and items in info spaces
  DEBUG("OptoHybridManager::resetAction begin");
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    usleep(1000);
    DEBUG("OptoHybridManager::looping over slots(" << (slot+1) << ") and finding expected cards");
    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      usleep(1000);
      DEBUG("OptoHybridManager::looping over links(" << link << ") and finding expected cards");
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      if (!info.present)
        continue;
      // set up the info space here rather than in initialize (where it can then get unset in reset?
      // should a value be set up for all of them by default?

      DEBUG("OptoHybridManager::revoking hwCfgInfoSpace items for board connected on link "
            << link << " to GLIB in slot " << (slot+1));
      toolbox::net::URN hwCfgURN("urn:gem:hw:"+toolbox::toString("gem.shelf%02d.glib%02d.optohybrid%02d",
                                                                 info.crateID.value_,
                                                                 info.slotID.value_,
                                                                 info.linkID.value_));
      if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
        DEBUG("OptoHybridManager::revoking config parameters into infospace");

        // reset the hw infospace toolbox
        is_optohybrids.at(slot).at(link)->reset();

        // these should now be gone from the reset call..., holdover from the old way
        if (is_optohybrids.at(slot).at(link)->getInfoSpace()->hasItem("ControlHubAddress"))
          is_optohybrids.at(slot).at(link)->getInfoSpace()->fireItemRevoked("ControlHubAddress");

        if (is_optohybrids.at(slot).at(link)->getInfoSpace()->hasItem("IPBusProtocol"))
          is_optohybrids.at(slot).at(link)->getInfoSpace()->fireItemRevoked("IPBusProtocol");

        if (is_optohybrids.at(slot).at(link)->getInfoSpace()->hasItem("DeviceIPAddress"))
          is_optohybrids.at(slot).at(link)->getInfoSpace()->fireItemRevoked("DeviceIPAddress");

        if (is_optohybrids.at(slot).at(link)->getInfoSpace()->hasItem("AddressTable"))
          is_optohybrids.at(slot).at(link)->getInfoSpace()->fireItemRevoked("AddressTable");

        if (is_optohybrids.at(slot).at(link)->getInfoSpace()->hasItem("ControlHubPort"))
          is_optohybrids.at(slot).at(link)->getInfoSpace()->fireItemRevoked("ControlHubPort");

        if (is_optohybrids.at(slot).at(link)->getInfoSpace()->hasItem("IPBusPort"))
          is_optohybrids.at(slot).at(link)->getInfoSpace()->fireItemRevoked("IPBusPort");
      } else {
        DEBUG("OptoHybridManager::resetAction::infospace " << hwCfgURN.toString() << " does not exist, no further action");
        continue;
      }
    } // end loop on link < MAX_OPTOHYBRIDS_PER_AMC
  } // end loop on slot < MAX_AMCS_PER_CRATE
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
  is_optohybrid->createUInt32("TrgSource",    optohybrid->getTrigSource(),      NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("SBitLoopback", optohybrid->getFirmware(),        NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("Ref_clk",      optohybrid->getReferenceClock(),  NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("SBit_Mask",    optohybrid->getSBitMask(),        NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("SBitsOut",     optohybrid->getSBitSource(),      NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("TrgThrottle",  optohybrid->getFirmware(),        NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("ZS",           optohybrid->getFirmware(),        NULL, GEMUpdateType::HW32);

  is_optohybrid->createUInt32("FIRMWARE_ID", optohybrid->getFirmware(),
                              NULL, GEMUpdateType::PROCESS, "docstring", "fwver");
  is_optohybrid->createUInt32("FPGA_PLL_IS_LOCKED",      optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("EXT_PLL_IS_LOCKED",       optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("CDCE_IS_LOCKED",          optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("GTX_IS_LOCKED",           optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("QPLL_IS_LOCKED",          optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("QPLL_FPGA_PLL_IS_LOCKED", optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);

  std::array<std::string, 4> wbMasters = {{"GTX","ExtI2C","Scan","DAC"}};
  for (auto master = wbMasters.begin(); master != wbMasters.end(); ++master) {
    is_optohybrid->createUInt32("Master:"+(*master)+"Strobe", optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32("Master:"+(*master)+"Ack",    optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
  }

  for (int i2c = 0; i2c < 6; ++i2c) {
    std::stringstream ss;
    ss << "I2C" << i2c;
    is_optohybrid->createUInt32("Slave:"+ss.str()+"Strobe", optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32("Slave:"+ss.str()+"Ack",    optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
  }

  std::array<std::string, 8> wbSlaves = {{"ExtI2C","Scan","T1","DAC","ADC","Clocking","Counters","System"}};
  for (auto slave = wbSlaves.begin(); slave != wbSlaves.end(); ++slave) {
    is_optohybrid->createUInt32("Slave:"+(*slave)+"Strobe", optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32("Slave:"+(*slave)+"Ack",    optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
  }

  for (int vfat = 0; vfat < 24; ++vfat) {
    std::stringstream ss;
    ss << "VFAT" << vfat;
    is_optohybrid->createUInt32(ss.str()+"_Incorrect", optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32(ss.str()+"_Valid",     optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
  }

  std::array<std::string, 5> t1sources = {{"TTC","INTERNAL","EXTERNAL","LOOPBACK","SENT"}};
  for (auto t1src = t1sources.begin(); t1src != t1sources.end(); ++t1src) {
    is_optohybrid->createUInt32((*t1src)+"L1A",      optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32((*t1src)+"CalPulse", optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32((*t1src)+"Resync",   optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32((*t1src)+"BC0",      optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
  }

  is_optohybrid->createUInt32("TrackingLinkErrors", optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("TriggerLinkErrors",  optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("DataPackets",        optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("QPLL_LOCK",          optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("QPLL_FPGA_PLL_LOCK", optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);

  /** Firmware based scan routines **/
  std::array<std::string, 2> scans = {{"Threshold/Latency","DAC"}};
  std::array<std::string, 9> scanregs = {{"START","MODE","CHIP","CHAN","MIN","MAX","STEP","NTRIGS","MONITOR"}};
  for (auto scan = scans.begin(); scan != scans.end(); ++scan) {
    for (auto scanreg = scanregs.begin(); scanreg != scanregs.end(); ++scanreg) {
      if ((*scan) == "DAC" && (*scanreg) == "CHAN")
        continue;

      is_optohybrid->createUInt32((*scan)+(*scanreg), optohybrid->getFirmware(), NULL, GEMUpdateType::HW32);
    }
  }
}
