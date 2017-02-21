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

  controlHubAddress = "N/A";
  deviceIPAddress   = "N/A";
  ipBusProtocol     = "N/A";
  addressTable      = "N/A";
  controlHubPort    = 0;
  ipBusPort         = 0;

  vfatBroadcastList = "0-23";
  vfatBroadcastMask = 0xff000000;

  vfatSBitList = "0-23";
  vfatSBitMask = 0xff000000;

  triggerSource = 0;
  //sbitSource    = 0;
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
  //bag->addField("sbitSource",    &sbitSource);
  bag->addField("refClkSrc",     &refClkSrc);
  //bag->addField("vfatClkSrc",    &vfatClkSrc);
  //bag->addField("cdceClkSrc",    &cdceClkSrc);

  bag->addField("SBitConfig",         &sbitConfig);
  bag->addField("CommonVFATSettings", &commonVFATSettings);
}

gem::hw::optohybrid::OptoHybridManager::SBitConfig::SBitConfig() {
  Mode       = 0;
  Output0Src = 0;
  Output1Src = 1;
  Output2Src = 2;
  Output3Src = 3;
  Output4Src = 4;
  Output5Src = 5;
  Outputs.setSize(6);
  Outputs.at(0) = Output0Src.value_;
  Outputs.at(1) = Output1Src.value_;
  Outputs.at(2) = Output2Src.value_;
  Outputs.at(3) = Output3Src.value_;
  Outputs.at(4) = Output4Src.value_;
  Outputs.at(5) = Output5Src.value_;
}

void gem::hw::optohybrid::OptoHybridManager::SBitConfig::registerFields(xdata::Bag<gem::hw::optohybrid::OptoHybridManager::SBitConfig>* bag) {
  bag->addField("Mode",       &Mode      );
  bag->addField("Output0Src", &Output0Src);
  bag->addField("Output1Src", &Output1Src);
  bag->addField("Output2Src", &Output2Src);
  bag->addField("Output3Src", &Output3Src);
  bag->addField("Output4Src", &Output4Src);
  bag->addField("Output5Src", &Output5Src);
  bag->addField("Outputs",    &Outputs);
}

gem::hw::optohybrid::OptoHybridManager::CommonVFATSettings::CommonVFATSettings() {
  ContReg0    = 0x36;
  ContReg2    = 0x30;
  IPreampIn   =  168;
  IPreampFeed =   80;
  IPreampOut  =  150;
  IShaper     =  150;
  IShaperFeed =  100;
  IComp       =   75;
  Latency     =  157;
  VThreshold1 =   50;
  VThreshold2 =    0;
}

void gem::hw::optohybrid::OptoHybridManager::CommonVFATSettings::registerFields(xdata::Bag<gem::hw::optohybrid::OptoHybridManager::CommonVFATSettings>* bag) {
  bag->addField("ContReg0",          &ContReg0   );
  bag->addField("ContReg1",          &ContReg1   );
  bag->addField("ContReg2",          &ContReg2   );
  bag->addField("ContReg3",          &ContReg3   );
  bag->addField("IPreampIn",         &IPreampIn  );
  bag->addField("IPreampFeed",       &IPreampFeed);
  bag->addField("IPreampOut",        &IPreampOut );
  bag->addField("IShaper",           &IShaper    );
  bag->addField("IShaperFeed",       &IShaperFeed);
  bag->addField("IComp",             &IComp      );
  bag->addField("Latency",           &Latency    );
  bag->addField("VThreshold1",       &VThreshold1);
  bag->addField("VThreshold2",       &VThreshold2);
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
    DEBUG("OptoHybridManager::initializeAction looping over slots(" << (slot+1) << ") and finding expected cards");
    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      DEBUG("OptoHybridManager::initializeAction looping over links(" << link << ") and finding expected cards");
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      DEBUG("OptoHybridManager::initializeAction index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;
      DEBUG("OptoHybridManager::initializeAction bag"
            << "crate " << info.crateID.value_
            << " slot " << info.slotID.value_
            << " link " << info.linkID.value_);

      if (!info.present)
        continue;

      DEBUG("OptoHybridManager::initializeAction: info is: " << info.toString());
      DEBUG("OptoHybridManager::initializeAction creating pointer to board connected on link "
            << link << " to AMC in slot " << (slot+1));
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

      DEBUG("OptoHybridManager::initializeAction exporting config parameters into infospace");
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

      DEBUG("OptoHybridManager::initializeAction InfoSpace found item: ControlHubAddress "
            << is_optohybrids.at(slot).at(link)->getString("ControlHubAddress"));
      DEBUG("OptoHybridManager::initializeAction InfoSpace found item: IPBusProtocol "
            << is_optohybrids.at(slot).at(link)->getString("IPBusProtocol")    );
      DEBUG("OptoHybridManager::initializeAction InfoSpace found item: DeviceIPAddress "
            << is_optohybrids.at(slot).at(link)->getString("DeviceIPAddress")  );
      DEBUG("OptoHybridManager::initializeAction InfoSpace found item: AddressTable "
            << is_optohybrids.at(slot).at(link)->getString("AddressTable")     );
      DEBUG("OptoHybridManager::initializeAction InfoSpace found item: ControlHubPort "
            << is_optohybrids.at(slot).at(link)->getUInt32("ControlHubPort")   );
      DEBUG("OptoHybridManager::initializeAction InfoSpace found item: IPBusPort "
            << is_optohybrids.at(slot).at(link)->getUInt32("IPBusPort")        );

      try {
        DEBUG("OptoHybridManager::initializeAction obtaining pointer to HwOptoHybrid " << deviceName
              << " (slot " << slot+1 << ")"
              << " (link " << link   << ")");
        m_optohybrids.at(slot).at(link) = optohybrid_shared_ptr(new gem::hw::optohybrid::HwOptoHybrid(deviceName,m_connectionFile.toString()));
      } catch (gem::hw::optohybrid::exception::Exception const& e) {
        std::stringstream msg;
        msg << "OptoHybridManager::initializeAction caught exception " << e.what();
        ERROR(msg.str());
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, msg.str());
      } catch (toolbox::net::exception::MalformedURN const& e) {
        std::stringstream msg;
        msg << "OptoHybridManager::initializeAction caught exception " << e.what();
        ERROR(msg.str());
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, msg.str());
      } catch (std::exception const& e) {
        std::stringstream msg;
        msg << "OptoHybridManager::initializeAction caught exception " << e.what();
        ERROR(msg.str());
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, msg.str());
      }
      DEBUG("OptoHybridManager::initializeAction connected");
      // set the web view to be empty or grey
      // if (!info.present.value_) continue;
      // p_gemWebInterface->optohybridInSlot(slot);

      DEBUG("OptoHybridManager::initializeAction grabbing pointer to hardware device");
      // optohybrid_shared_ptr optohybrid = m_optohybrids.at(slot).at(link);
      if (m_optohybrids.at(slot).at(link)->isHwConnected()) {
        // get connected VFATs
        m_vfatMapping.at(slot).at(link)   = m_optohybrids.at(slot).at(link)->getConnectedVFATs();
        m_trackingMask.at(slot).at(link)  = m_optohybrids.at(slot).at(link)->getConnectedVFATMask();
        m_broadcastList.at(slot).at(link) = m_optohybrids.at(slot).at(link)->getConnectedVFATMask();
        m_sbitMask.at(slot).at(link)      = m_optohybrids.at(slot).at(link)->getConnectedVFATMask();

        createOptoHybridInfoSpaceItems(is_optohybrids.at(slot).at(link), m_optohybrids.at(slot).at(link));
        INFO("OptoHybridManager::initializeAction looping over created VFAT devices");
        for (auto mapit = m_vfatMapping.at(slot).at(link).begin();
             mapit != m_vfatMapping.at(slot).at(link).end(); ++mapit) {
          INFO("OptoHybridManager::initializeAction VFAT" << (int)mapit->first << " has chipID "
               << std::hex << (int)mapit->second << std::dec << " (from map)");
          gem::hw::vfat::HwVFAT2& vfatDevice = m_optohybrids.at(slot).at(link)->getVFATDevice(mapit->first);
          INFO("OptoHybridManager::initializeAction VFAT" << (int)mapit->first << " has chipID "
               << std::hex << (int)vfatDevice.getChipID() << std::dec << " (from HW device) ");
        }

        m_optohybridMonitors.at(slot).at(link) = std::shared_ptr<OptoHybridMonitor>(new OptoHybridMonitor(m_optohybrids.at(slot).at(link), this, index));
        m_optohybridMonitors.at(slot).at(link)->addInfoSpace("HWMonitoring", is_optohybrids.at(slot).at(link));
        m_optohybridMonitors.at(slot).at(link)->setupHwMonitoring();
        m_optohybridMonitors.at(slot).at(link)->startMonitoring();

        INFO("OptoHybridManager::initializeAction OptoHybrid connected on link "
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
        m_optohybrids.at(slot).at(link)->setVFATMask(m_trackingMask.at(slot).at(link));
        m_optohybrids.at(slot).at(link)->setSBitMask(m_sbitMask.at(slot).at(link));
        // turn off any that are excluded by the additional mask?
      } else {
        std::stringstream msg;
        msg << "OptoHybridManager::initializeAction OptoHybrid connected on link "
            << link << " to AMC in slot " << (slot+1) << " is not responding";
        ERROR(msg.str());
        //fireEvent("Fail");
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, msg.str());
      }
      // FOR MISHA
      // hardware should be connected, can update ldqm_db for teststand/local runs
    }
  }
  INFO("OptoHybridManager::initializeAction end");
}

void gem::hw::optohybrid::OptoHybridManager::configureAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  DEBUG("OptoHybridManager::configureAction");
  //std::ofstream of

  std::map<int,std::set<int> > hwMapping;
  //will the manager operate for all connected optohybrids, or only those connected to certain AMCs?
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    // usleep(100); // just for testing the timing of different applications
    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      // usleep(100); // just for testing the timing of different applications
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      DEBUG("OptoHybridManager::configureAction::info is: " << info.toString());
      if (!info.present)
        continue;

      DEBUG("OptoHybridManager::configureAction::grabbing pointer to hardware device");
      optohybrid_shared_ptr optohybrid = m_optohybrids.at(slot).at(link);

      if (optohybrid->isHwConnected()) {
        hwMapping[slot+1].insert(link);

        DEBUG("OptoHybridManager::configureAction::setting trigger source to 0x"
             << std::hex << info.triggerSource.value_ << std::dec);
        optohybrid->setTrigSource(info.triggerSource.value_);

        // DEBUG("OptoHybridManager::configureAction::setting sbit source to 0x"
        //      << std::hex << info.sbitSource.value_ << std::dec);
        // optohybrid->setSBitSource(info.sbitSource.value_);
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
        for (unsigned olink = 0; olink < HwAMC::N_GTX; ++olink) {
        }
        */

        DEBUG("OptoHybridManager::configureAction Setting output s-bit configuration parameters");
        optohybrid->setSBitMode(info.sbitConfig.bag.Mode.value_);

        std::array<uint8_t, 6> sbitSources = {{
            static_cast<uint8_t>(info.sbitConfig.bag.Output0Src.value_ & 0x1f),
            static_cast<uint8_t>(info.sbitConfig.bag.Output1Src.value_ & 0x1f),
            static_cast<uint8_t>(info.sbitConfig.bag.Output2Src.value_ & 0x1f),
            static_cast<uint8_t>(info.sbitConfig.bag.Output3Src.value_ & 0x1f),
            static_cast<uint8_t>(info.sbitConfig.bag.Output4Src.value_ & 0x1f),
            static_cast<uint8_t>(info.sbitConfig.bag.Output5Src.value_ & 0x1f),
          }};

        optohybrid->setHDMISBitSource(sbitSources);

        std::vector<std::pair<uint8_t,uint32_t> > chipIDs = optohybrid->getConnectedVFATs();

        for (auto chip = chipIDs.begin(); chip != chipIDs.end(); ++chip)
          if (chip->second)
            INFO("VFAT found in GEB slot " << std::setw(2) << (int)chip->first << " has ChipID "
                 << "0x" << std::hex << std::setw(4) << chip->second << std::dec);
          else
            INFO("No VFAT found in GEB slot " << std::setw(2) << (int)chip->first);

        uint32_t vfatMask = m_broadcastList.at(slot).at(link);
        INFO("Setting VFAT parameters with broadcast write using mask " << std::hex << vfatMask << std::dec);

        std::map<std::string, uint8_t > vfatSettings;
        vfatSettings["ContReg0"   ] = (uint8_t)(info.commonVFATSettings.bag.ContReg0.value_);
        vfatSettings["ContReg1"   ] = (uint8_t)(info.commonVFATSettings.bag.ContReg1.value_);
        vfatSettings["ContReg2"   ] = (uint8_t)(info.commonVFATSettings.bag.ContReg2.value_);
        vfatSettings["ContReg3"   ] = (uint8_t)(info.commonVFATSettings.bag.ContReg3.value_);
        vfatSettings["IPreampIn"  ] = (uint8_t)(info.commonVFATSettings.bag.IPreampIn.value_);
        vfatSettings["IPreampFeed"] = (uint8_t)(info.commonVFATSettings.bag.IPreampFeed.value_);
        vfatSettings["IPreampOut" ] = (uint8_t)(info.commonVFATSettings.bag.IPreampOut.value_);
        vfatSettings["IShaper"    ] = (uint8_t)(info.commonVFATSettings.bag.IShaper.value_);
        vfatSettings["IShaperFeed"] = (uint8_t)(info.commonVFATSettings.bag.IShaperFeed.value_);
        vfatSettings["IComp"      ] = (uint8_t)(info.commonVFATSettings.bag.IComp.value_);
        vfatSettings["VThreshold1"] = (uint8_t)(info.commonVFATSettings.bag.VThreshold1.value_);
        vfatSettings["VThreshold2"] = (uint8_t)(info.commonVFATSettings.bag.VThreshold2.value_);
        vfatSettings["Latency"    ] = (uint8_t)(info.commonVFATSettings.bag.Latency.value_);

	if (m_scanType.value_ == 2) {
	  INFO("OptoHybridManager::configureAction configureAction: FIRST Latency  " << m_scanMin.value_);
          vfatSettings["Latency"    ] = (uint8_t)(m_scanMin.value_);
	  optohybrid->setVFATsToDefaults(vfatSettings, vfatMask);
          // HACK
          // have to enable the pulse to the channel if using cal pulse latency scan
          // but shouldn't mess with other settings... not possible here, so just a hack
          // optohybrid->broadcastWrite("VFATChannels.ChanReg23",  0x40, vfatMask);
          // optohybrid->broadcastWrite("VFATChannels.ChanReg124", 0x40, vfatMask);
          // optohybrid->broadcastWrite("VFATChannels.ChanReg65",  0x40, vfatMask);
          // optohybrid->broadcastWrite("VCal",                    0xaf, vfatMask);
	} else if (m_scanType.value_ == 3) {
	  uint32_t initialVT1 = m_scanMin.value_;
	  //	  uint32_t VT1 = (m_scanMax.value_ - m_scanMin.value_);
	  uint32_t initialVT2 = 0; //std::max(0,(uint32_t)m_scanMax.value_);
	  INFO("OptoHybridManager::configureAction FIRST VT1 " << initialVT1 << " VT2 " << initialVT2);
          vfatSettings["VThreshold1"] = (uint8_t)(initialVT1&0xff);
          vfatSettings["VThreshold2"] = (uint8_t)(initialVT2&0xff);
	  optohybrid->setVFATsToDefaults(vfatSettings, vfatMask);
	} else {
	  optohybrid->setVFATsToDefaults(vfatSettings, vfatMask);
	}

	std::array<std::string, 11> setupregs = {{"ContReg0", "ContReg2", "IPreampIn", "IPreampFeed", "IPreampOut",
						  "IShaper", "IShaperFeed", "IComp", "Latency",
						  "VThreshold1", "VThreshold2"}};

        INFO("Reading back values after setting defaults:");
        for (auto reg = setupregs.begin(); reg != setupregs.end(); ++reg) {
          std::vector<uint32_t> res = optohybrid->broadcastRead(*reg,vfatMask);
          INFO(*reg);
          for (auto r = res.begin(); r != res.end(); ++r) {
            INFO(" 0x" << std::hex << std::setw(8) << std::setfill('0') << *r << std::dec);
          }
        }

        //what else is required for configuring the OptoHybrid?
        //need to reset optical links?
        //reset counters?
        uint32_t gtxMask = optohybrid->readReg("GEM_AMC.DAQ.CONTROL.INPUT_ENABLE_MASK");
        gtxMask |= (0x1<<link);
        optohybrid->writeReg("GEM_AMC.DAQ.CONTROL.INPUT_ENABLE_MASK", gtxMask);
      } else {
        std::stringstream msg;
        msg << "OptoHybridManager::configureAction::OptoHybrid connected on link " << (int)link
            << " to AMC in slot " << (int)(slot+1) << " is not responding";
        ERROR(msg.str());
        //fireEvent("Fail");
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, msg.str());
      }
    }
  }
  INFO("OptoHybridManager::configureAction end");
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

  DEBUG("OptoHybridManager::startAction");
  //will the manager operate for all connected optohybrids, or only those connected to certain AMCs?
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    // usleep(100); // just for testing the timing of different applications
    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      // usleep(100); // just for testing the timing of different applications
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      if (!info.present)
        continue;

      DEBUG("OptoHybridManager::startAction::grabbing pointer to hardware device");
      optohybrid_shared_ptr optohybrid = m_optohybrids.at(slot).at(link);

      if (optohybrid->isHwConnected()) {
        // turn on all VFATs? or should they always be on?
        uint32_t vfatMask = m_broadcastList.at(slot).at(link);
        // reset counters
        optohybrid->counterReset();
        // // reset VFAT counters
        // optohybrid->resetVFATCRCCount();

        std::vector<uint32_t> res = optohybrid->broadcastRead("ContReg0",vfatMask);
        INFO("ContReg0: vfatMask = " << std::hex << std::setw(8) << std::setfill('0') << vfatMask);
        for (auto r = res.begin(); r != res.end(); ++r)
          INFO(" 0x" << std::hex << std::setw(8) << std::setfill('0') << *r << std::dec);

        // perhaps don't hardcode the full control register here, simply turn on the run bit?
        optohybrid->broadcastWrite("ContReg0", 0x37, vfatMask);
        res.clear();
        res = optohybrid->broadcastRead("ContReg0",vfatMask);
        INFO("OptoHybridManager::startAction ContReg0");
        for (auto r = res.begin(); r != res.end(); ++r)
          INFO(" 0x" << std::hex << std::setw(8) << std::setfill('0') << *r << std::dec);

        // what resets to do
      } else {
        std::stringstream msg;
        msg << "OptoHybridManager::startAction::OptoHybrid connected on link " << (int)link
            << " to AMC in slot " << (int)(slot+1) << " is not responding";
        ERROR(msg.str());
        //fireEvent("Fail");
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, msg.str());
      }
    }
  }
  INFO("OptoHybridManager::startAction end");
}

void gem::hw::optohybrid::OptoHybridManager::pauseAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  // put all connected VFATs into sleep mode?

  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    // usleep(100); // just for testing the timing of different applications
    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      // usleep(100); // just for testing the timing of different applications
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      if (!info.present)
        continue;

      DEBUG("OptoHybridManager::pauseAction::grabbing pointer to hardware device");
      optohybrid_shared_ptr optohybrid = m_optohybrids.at(slot).at(link);

      if (optohybrid->isHwConnected()) {
        // turn on all VFATs? or should they always be on?
        uint32_t vfatMask = m_broadcastList.at(slot).at(link);
	if (m_scanType.value_ == 2) {
	  uint8_t updatedLatency = m_lastLatency + m_stepSize.value_;
	  INFO("OptoHybridManager::LatencyScan OptoHybrid on link " << (int)link
	       << " AMC slot " << (slot+1) << " Latency  " << (int)updatedLatency);

          optohybrid->broadcastWrite("Latency", updatedLatency, vfatMask);
      } else if (m_scanType.value_ == 3) {
	  uint8_t updatedVT1 = m_lastVT1 + m_stepSize.value_;
	  uint8_t VT2 = 0; //std::max(0,(int)m_scanMax.value_);
	  INFO("OptoHybridManager::ThresholdScan OptoHybrid on link " << (int)link
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
        ERROR(msg.str());
        //fireEvent("Fail");
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, msg.str());
      }
    }
  }
  // Update the scan parameters
  if (m_scanType.value_ == 2) {
    INFO("OptoHybridManager::pauseAction LatencyScan old Latency " << (int)m_lastLatency);
    m_lastLatency += m_stepSize.value_;
    INFO("OptoHybridManager::pauseAction LatencyScan new Latency " << (int)m_lastLatency);
  } else if (m_scanType.value_ == 3) {
    INFO("OptoHybridManager::pauseAction ThresholdScan old VT1 " << (int)m_lastVT1);
    m_lastVT1 += m_stepSize.value_;
    INFO("OptoHybridManager::pauseAction ThresholdScan new VT1 " << (int)m_lastVT1);
  }
  INFO("OptoHybridManager::pauseAction end");
}

void gem::hw::optohybrid::OptoHybridManager::resumeAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  // put all connected VFATs into run mode?
  usleep(100);
  INFO("OptoHybridManager::resumeAction end");
}

void gem::hw::optohybrid::OptoHybridManager::stopAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  DEBUG("OptoHybridManager::stopAction");
  //will the manager operate for all connected optohybrids, or only those connected to certain AMCs?
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    // usleep(100); // just for testing the timing of different applications
    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      // usleep(100); // just for testing the timing of different applications
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      if (!info.present)
        continue;

      DEBUG("OptoHybridManager::stopAction::grabbing pointer to hardware device");
      optohybrid_shared_ptr optohybrid = m_optohybrids.at(slot).at(link);

      if (optohybrid->isHwConnected()) {
        // put all connected VFATs into sleep mode?
        uint32_t vfatMask = m_broadcastList.at(slot).at(link);
        // perhaps don't hardcode the full control register here, simply turn off the run bit?
        optohybrid->broadcastWrite("ContReg0", 0x36, vfatMask);
        // what resets to do

        std::map<std::string, uint8_t > vfatSettings;
        vfatSettings["ContReg0"   ] = (uint8_t)(info.commonVFATSettings.bag.ContReg0.value_);
        vfatSettings["ContReg1"   ] = (uint8_t)(info.commonVFATSettings.bag.ContReg1.value_);
        vfatSettings["ContReg2"   ] = (uint8_t)(info.commonVFATSettings.bag.ContReg2.value_);
        vfatSettings["ContReg3"   ] = (uint8_t)(info.commonVFATSettings.bag.ContReg3.value_);
        vfatSettings["IPreampIn"  ] = (uint8_t)(info.commonVFATSettings.bag.IPreampIn.value_);
        vfatSettings["IPreampFeed"] = (uint8_t)(info.commonVFATSettings.bag.IPreampFeed.value_);
        vfatSettings["IPreampOut" ] = (uint8_t)(info.commonVFATSettings.bag.IPreampOut.value_);
        vfatSettings["IShaper"    ] = (uint8_t)(info.commonVFATSettings.bag.IShaper.value_);
        vfatSettings["IShaperFeed"] = (uint8_t)(info.commonVFATSettings.bag.IShaperFeed.value_);
        vfatSettings["IComp"      ] = (uint8_t)(info.commonVFATSettings.bag.IComp.value_);
        vfatSettings["VThreshold1"] = (uint8_t)(info.commonVFATSettings.bag.VThreshold1.value_);
        vfatSettings["VThreshold2"] = (uint8_t)(info.commonVFATSettings.bag.VThreshold2.value_);
        vfatSettings["Latency"    ] = (uint8_t)(info.commonVFATSettings.bag.Latency.value_);

	if (m_scanType.value_ == 2) {
	  optohybrid->setVFATsToDefaults(vfatSettings, vfatMask);
          // HACK
          // have to disable the pulse to the channel if using cal pulse latency scan
          // but shouldn't mess with other settings... not possible here, so just a hack
          // optohybrid->broadcastWrite("VFATChannels.ChanReg23",  0x00, vfatMask);
          // optohybrid->broadcastWrite("VFATChannels.ChanReg124", 0x00, vfatMask);
          // optohybrid->broadcastWrite("VFATChannels.ChanReg65",  0x00, vfatMask);
          // optohybrid->broadcastWrite("VCal",                    0x00, vfatMask);
	} else if (m_scanType.value_ == 3) {
	  optohybrid->setVFATsToDefaults(vfatSettings, vfatMask);
        }
      } else {
        std::stringstream msg;
        msg << "OptoHybridManager::stopAction::OptoHybrid connected on link " << (int)link
            << " to AMC in slot " << (int)(slot+1) << " is not responding";
        ERROR(msg.str());
        //fireEvent("Fail");
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, msg.str());
      }
    }
  }
  INFO("OptoHybridManager::stopAction end");
}

void gem::hw::optohybrid::OptoHybridManager::haltAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  // put all connected VFATs into sleep mode?
  usleep(100);
  INFO("OptoHybridManager::haltAction end");
}

void gem::hw::optohybrid::OptoHybridManager::resetAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  //unregister listeners and items in info spaces
  DEBUG("OptoHybridManager::resetAction begin");
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    // usleep(100);
    DEBUG("OptoHybridManager::looping over slots(" << (slot+1) << ") and finding expected cards");
    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      // usleep(100);
      DEBUG("OptoHybridManager::looping over links(" << link << ") and finding expected cards");
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      DEBUG("OptoHybridManager::index = " << index);
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      if (!info.present)
        continue;
      // set up the info space here rather than in initialize (where it can then get unset in reset?
      // should a value be set up for all of them by default?

    // reset the hw monitor
    if (m_optohybridMonitors.at(slot).at(link))
      m_optohybridMonitors.at(slot).at(link)->reset();

      DEBUG("OptoHybridManager::revoking hwCfgInfoSpace items for board connected on link "
            << link << " to AMC in slot " << (slot+1));
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
  INFO("OptoHybridManager::resetAction end");
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
  is_optohybrid->createUInt32("SBitLoopback", optohybrid->getFirmwareVersion(),        NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("Ref_clk",      optohybrid->getReferenceClock(),  NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("SBit_Mask",    optohybrid->getSBitMask(),        NULL, GEMUpdateType::HW32);
  // probably want this to be a set of 6
  is_optohybrid->createUInt32("SBitsOut",     optohybrid->getSBitSource(),      NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("SBitOutMode",  optohybrid->getSBitMode(),        NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("TrgThrottle",  optohybrid->getFirmwareVersion(),        NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("ZS",           optohybrid->getFirmwareVersion(),        NULL, GEMUpdateType::HW32);

  is_optohybrid->createUInt32("FIRMWARE_DATE", optohybrid->getFirmwareDate(),
                              NULL, GEMUpdateType::PROCESS, "docstring", "fwdateoh");
  is_optohybrid->createUInt32("FIRMWARE_VERSION", optohybrid->getFirmwareVersion(),
                              NULL, GEMUpdateType::PROCESS, "docstring", "fwveroh");
  is_optohybrid->createUInt32("FPGA_PLL_IS_LOCKED",      optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("EXT_PLL_IS_LOCKED",       optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("CDCE_IS_LOCKED",          optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("GTX_IS_LOCKED",           optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("QPLL_IS_LOCKED",          optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("QPLL_FPGA_PLL_IS_LOCKED", optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);

  std::array<std::string, 4> wbMasters = {{"GTX","ExtI2C","Scan","DAC"}};
  for (auto master = wbMasters.begin(); master != wbMasters.end(); ++master) {
    is_optohybrid->createUInt32("Master:"+(*master)+"Strobe", optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32("Master:"+(*master)+"Ack",    optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  }

  for (int i2c = 0; i2c < 6; ++i2c) {
    std::stringstream ss;
    ss << "I2C" << i2c;
    is_optohybrid->createUInt32("Slave:"+ss.str()+"Strobe", optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32("Slave:"+ss.str()+"Ack",    optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  }

  std::array<std::string, 8> wbSlaves = {{"ExtI2C","Scan","T1","DAC","ADC","Clocking","Counters","System"}};
  for (auto slave = wbSlaves.begin(); slave != wbSlaves.end(); ++slave) {
    is_optohybrid->createUInt32("Slave:"+(*slave)+"Strobe", optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32("Slave:"+(*slave)+"Ack",    optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  }

  for (int vfat = 0; vfat < 24; ++vfat) {
    std::stringstream ss;
    ss << "VFAT" << vfat;
    is_optohybrid->createUInt32(ss.str()+"_Incorrect", optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32(ss.str()+"_Valid",     optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  }

  std::array<std::string, 5> t1sources = {{"TTC","INTERNAL","EXTERNAL","LOOPBACK","SENT"}};
  for (auto t1src = t1sources.begin(); t1src != t1sources.end(); ++t1src) {
    is_optohybrid->createUInt32((*t1src)+"L1A",      optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32((*t1src)+"CalPulse", optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32((*t1src)+"Resync",   optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
    is_optohybrid->createUInt32((*t1src)+"BC0",      optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  }

  is_optohybrid->createUInt32("TrackingLinkErrors", optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("TriggerLinkErrors",  optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("DataPackets",        optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("QPLL_LOCK",          optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
  is_optohybrid->createUInt32("QPLL_FPGA_PLL_LOCK", optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);

  /** Firmware based scan routines **/
  std::array<std::string, 3> scans = {{"Single VFAT Threshold/Latency/SCurve", "Ultra VFATs Threshold/Latency/SCurve","DAC"}};
  std::array<std::string, 8> scanregs = {{"MODE","CHIP","CHAN","MIN","MAX","STEP","NTRIGS","MONITOR"}};
  for (auto scan = scans.begin(); scan != scans.end(); ++scan) {
    for (auto scanreg = scanregs.begin(); scanreg != scanregs.end(); ++scanreg) {
      if ((*scan) == "DAC" && (*scanreg) == "CHAN")
        continue;
      if (scan->rfind("Ultra") != std::string::npos && (*scanreg) == "CHIP")
        is_optohybrid->createUInt32((*scan)+"MASK", optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
      else
        is_optohybrid->createUInt32((*scan)+(*scanreg), optohybrid->getFirmwareVersion(), NULL, GEMUpdateType::HW32);
    }
  }
}


