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
  xoap::bind(this, &gem::hw::optohybrid::OptoHybridManager::updateScanValueCalib, "updateScanValueCalib",  XDAQ_NS_URI);
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
        //FIXME what's the purpose of this 
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

        uint32_t vfatMask = m_broadcastList.at(slot).at(link);
        
        
        if (m_scanInfo.bag.scanType.value_ == 2) {
            //TODO:set calibration mode (atm set to false)
            if (m_scanInfo.bag.calMode.value_)
                optohybrid->configureOHCalDataFormat(vfatMask);
            
            CMSGEMOS_DEBUG("OptoHybridManager::configureAction configureAction: Latency scan with value " << m_scanInfo.bag.scanMin.value_);
            optohybrid->broadcastWrite("CFG_LATENCY", m_scanInfo.bag.scanMin.value_,vfatMask, false);
            optohybrid->broadcastWrite("CFG_PULSE_STRETCH", m_scanInfo.bag.mspl.value_,vfatMask, false);
            
            if(  m_scanInfo.bag.signalSourceType.value_<1) {//CG: Cal pulse
                optohybrid->broadcastWrite("CFG_CAL_MODE", 0x1 ,vfatMask, false);
            }///for latency scans use the Voltage pulse(CAL_MODE 0x1), low CFG_CAL_DAC is a high injected charge amount  
            optohybrid->broadcastWrite("CFG_CAL_DAC", 6, vfatMask, false);
            CMSGEMOS_DEBUG("OptoHybridManager::configureAction: reading  specifics for latency on OH0VFAT23 MPSL " <<  m_scanInfo.bag.mspl.value_ << " reg " << optohybrid->readReg("GEM_AMC.OH.OH0.GEB.VFAT23.CFG_PULSE_STRETCH") << " CAL_DAC " <<  (uint32_t) optohybrid->readReg("GEM_AMC.OH.OH0.GEB.VFAT23.CFG_CAL_DAC"));
        
          // enabling and disabling calpulse for channels on vfat 
          for (int ch=0; ch< 128 ; ++ch){
              char reg [100] ;
              if (ch < m_scanInfo.bag.vfatChMin.value_ || ch >  m_scanInfo.bag.vfatChMax.value_) { 
                  sprintf(reg, "VFAT_CHANNELS.CHANNEL%i.CALPULSE_ENABLE", ch);
                  optohybrid->broadcastWrite(reg, 0, vfatMask, false);
              } else {
                  sprintf(reg, "VFAT_CHANNELS.CHANNEL%i.CALPULSE_ENABLE", ch);
                  optohybrid->broadcastWrite(reg, 1, vfatMask, false);
              }
          }
          
          CMSGEMOS_INFO("OptoHybridManager::configureAction: ending  specifics for latency ");   

    
        } else if (m_scanInfo.bag.scanType.value_ == 3) { //S-curve
            // FIXME OBSOLETE

            optohybrid->broadcastWrite("CFG_LATENCY", m_scanInfo.bag.latency.value_, vfatMask, false);  
            optohybrid->broadcastWrite("CFG_PULSE_STRETCH", m_scanInfo.bag.mspl.value_, vfatMask, false);
            optohybrid->broadcastWrite("CFG_CAL_PHI", m_scanInfo.bag.pulseDelay.value_, vfatMask, false);
            ///for Scurve scans use Current pulse, high CFG_CAL_DAC is a high injected charge amount
            int caldac = 250;
            optohybrid->broadcastWrite("CFG_CAL_DAC", caldac, vfatMask, false);

            uint32_t initialVT1 =  m_scanInfo.bag.scanMin.value_; //m_scanMin.value_;
         
            uint32_t initialVT2 = 0; //std::max(0,(uint32_t)m_scanMax.value_);

            
        } else {
          // FIXME:should configure VFATbefore and not here.
            CMSGEMOS_INFO("OptoHybridManager::configureAction configureVFATs()");
            optohybrid->configureVFATs(); ///CG;
        }

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
    
  if (m_scanType.value_ == 2) {
    m_lastLatency = m_scanInfo.bag.scanMin.value_;
    m_lastVT1 = 0;
  } else if (m_scanType.value_ == 3) {
    m_lastLatency = 0;
    m_lastVT1 = m_scanInfo.bag.scanMin.value_ ;
  }

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
        // FIXME reset counters is doing nothing at the moment!
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

xoap::MessageReference gem::hw::optohybrid::OptoHybridManager::updateScanValueCalib(xoap::MessageReference msg){
    std::string commandName = "updateScanValueCalib";
    CMSGEMOS_DEBUG("OptohybridManager::updateScanValueCalib");
    try {
        CMSGEMOS_INFO("Optohybrid::updateScanValueCalib trying");
        for (size_t slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {

            for (size_t link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
                size_t index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
                
                OptoHybridInfo& info = m_optohybridInfo[index].bag;

                if (!info.present)
                    continue;
                
                auto&& optohybrid = m_optohybrids.at(slot).at(link);

                if (optohybrid->isHwConnected()) {
                    // turn on all VFATs? or should they always be on?
                    uint32_t vfatMask = m_broadcastList.at(slot).at(link);
                    if (m_scanInfo.bag.scanType.value_ == 2) { 
                        uint32_t updatedLatency = m_lastLatency + m_stepSize.value_;
                        CMSGEMOS_DEBUG("OptoHybridManager::updateScanValueCalib LatencyScan OptoHybrid on link " << static_cast<uint32_t>(link)
                                      << " AMC slot " << (slot+1) << " Latency  " << static_cast<uint32_t>(updatedLatency));
                        
                        optohybrid->broadcastWrite("CFG_LATENCY", updatedLatency, vfatMask);
                    } else if (m_scanInfo.bag.scanType.value_ == 3) { // FIXME OBSOLETE
                        uint8_t updatedVT1 = m_lastVT1 + m_scanInfo.bag.stepSize.value_;
                        uint8_t VT2 = 0;  // std::max(0,static_cast<uint32_t>(m_scanMax.value_));
                        CMSGEMOS_INFO("OptoHybridManager::ThresholdScan OptoHybrid on link " << static_cast<uint32_t>(link)
                                      << " AMC slot " << (slot+1) << " VT1 " << static_cast<uint32_t>(updatedVT1)
                                      << " VT2 " << VT2 << " StepSize " << m_scanInfo.bag.stepSize.value_);

                        // optohybrid->broadcastWrite("CFG_THR_ARM_DAC", updatedVT1, vfatMask);
                        // optohybrid->broadcastWrite("CFG_THR_ZCC_DAC", VT2, vfatMask);
                    }
                    // what resets to do
                } else {
                    std::stringstream errmsg;
                    errmsg << "OptoHybridManager::updateScanValueCalib OptoHybrid connected on link " << static_cast<uint32_t>(link)
                           << " to AMC in slot " << static_cast<uint32_t>(slot+1) << " is not responding";
                    CMSGEMOS_ERROR(errmsg.str());
                    // fireEvent("Fail");
                    XCEPT_RAISE(gem::hw::managers::optohybrid::exception::Exception, errmsg.str());
                }
            }
        }
        // Update the scan parameters
        if (m_scanInfo.bag.scanType.value_ == 2) {
            CMSGEMOS_INFO("OptoHybridManager::pauseAction LatencyScan old Latency " << static_cast<uint32_t>(m_lastLatency));
            m_lastLatency += m_scanInfo.bag.stepSize.value_;
            CMSGEMOS_INFO("OptoHybridManager::pauseAction LatencyScan new Latency " << static_cast<uint32_t>(m_lastLatency));
        } else if (m_scanInfo.bag.scanType.value_ == 3) {
            CMSGEMOS_INFO("OptoHybridManager::pauseAction ThresholdScan old VT1 " << static_cast<uint32_t>(m_lastVT1));
            m_lastVT1 += m_scanInfo.bag.stepSize.value_;
            CMSGEMOS_INFO("OptoHybridManager::pauseAction ThresholdScan new VT1 " << static_cast<uint32_t>(m_lastVT1));
        }
        CMSGEMOS_INFO("OptoHybridManager::updateScanValueCalib end");
        
        return
            gem::utils::soap::GEMSOAPToolBox::makeSOAPReply(commandName, "Done");
        
    } catch(xcept::Exception& err) {
        std::string msgBase = toolbox::toString("Failed to create SOAP reply for command '%s'",
                                                commandName.c_str());
        CMSGEMOS_ERROR(toolbox::toString("%s: %s.", msgBase.c_str(), xcept::stdformat_exception_history(err).c_str()));
    }
    return
        gem::utils::soap::GEMSOAPToolBox::makeSOAPReply(commandName, "NotDone");
    
}
    
void gem::hw::optohybrid::OptoHybridManager::pauseAction()
{

  // FIXME put all connected VFATs into run/pause  mode?

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
  is_optohybrid->createUInt32("CLK_STATUS",       optohybrid->getClockStatus(),  NULL, GEMUpdateType::HW32); //CG FIXME supposed to work on xdaq15 and coffin but FW not wired up correctly, register not set.

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
