/**
 * class: GLIBManager
 * description: Manager application for GLIB cards
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date: 
 */

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPConstants.h"
#include "xoap/SOAPBody.h"
#include "xoap/Method.h"
#include "xoap/AttachmentPart.h"

#include "gem/hw/glib/GLIBManager.h"
#include "gem/hw/glib/GLIBMonitor.h"
#include "gem/hw/glib/GLIBManagerWeb.h"

#include "gem/hw/glib/HwGLIB.h"
#include "gem/hw/glib/exception/Exception.h"

typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

XDAQ_INSTANTIATOR_IMPL(gem::hw::glib::GLIBManager);

gem::hw::glib::GLIBManager::GLIBInfo::GLIBInfo()
{
  present = false;
  crateID = -1;
  slotID  = -1;
  controlHubAddress = "";
  deviceIPAddress   = "";
  ipBusProtocol     = "";
  addressTable      = "";
  controlHubPort    = 0;
  ipBusPort         = 0;
  
  triggerSource = 0;
  sbitSource    = 0;
}

void gem::hw::glib::GLIBManager::GLIBInfo::registerFields(xdata::Bag<gem::hw::glib::GLIBManager::GLIBInfo>* bag)
{
  bag->addField("crateID", &crateID);
  bag->addField("slot",    &slotID);
  bag->addField("present", &present);

  bag->addField("ControlHubAddress", &controlHubAddress);
  bag->addField("DeviceIPAddress",   &deviceIPAddress);
  bag->addField("IPBusProtocol",     &ipBusProtocol);
  bag->addField("AddressTable",      &addressTable);
  bag->addField("ControlHubPort",    &controlHubPort);
  bag->addField("IPBusPort",         &ipBusPort);
            
  bag->addField("triggerSource", &triggerSource);
  bag->addField("sbitSource",    &sbitSource);
}

gem::hw::glib::GLIBManager::GLIBManager(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub),
  m_amcEnableMask(0)
{
  m_glibInfo.setSize(MAX_AMCS_PER_CRATE);

  p_appInfoSpace->fireItemAvailable("AllGLIBsInfo",   &m_glibInfo);
  p_appInfoSpace->fireItemAvailable("AMCSlots",       &m_amcSlots);
  p_appInfoSpace->fireItemAvailable("ConnectionFile", &m_connectionFile);

  p_appInfoSpace->addItemRetrieveListener("AllGLIBsInfo",   this);
  p_appInfoSpace->addItemRetrieveListener("AMCSlots",       this);
  p_appInfoSpace->addItemRetrieveListener("ConnectionFile", this);
  p_appInfoSpace->addItemChangedListener( "AllGLIBsInfo",   this);
  p_appInfoSpace->addItemChangedListener( "AMCSlots",       this);
  p_appInfoSpace->addItemChangedListener( "ConnectionFile", this);

  // initialize the GLIB application objects
  DEBUG("GLIBManager::Connecting to the GLIBManagerWeb interface");
  p_gemWebInterface = new gem::hw::glib::GLIBManagerWeb(this);
  // p_gemMonitor      = new gem::hw::glib::GLIBHwMonitor(this);
  DEBUG("GLIBManager::done");
  
  // set up the info hwCfgInfoSpace 
  init();

  // getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemhardware/images/glib/GLIBManager.png");
}

gem::hw::glib::GLIBManager::~GLIBManager()
{
  // memory management, maybe not necessary here?
}

uint16_t gem::hw::glib::GLIBManager::parseAMCEnableList(std::string const& enableList)
{
  uint16_t slotMask = 0x0;
  std::vector<std::string> slots;

  boost::split(slots, enableList, boost::is_any_of(", "), boost::token_compress_on);  
  DEBUG("GLIBManager::AMC input enable list is " << enableList);
  // would be great to multithread this portion
  for (auto slot = slots.begin(); slot != slots.end(); ++slot) {
    DEBUG("GLIBManager::slot is " << *slot);
    if (slot->find('-') != std::string::npos) {  // found a possible range
      DEBUG("GLIBManager::found a hyphen in " << *slot);
      std::vector<std::string> range;
      boost::split(range, *slot, boost::is_any_of("-"), boost::token_compress_on);
      if (range.size() > 2) {
        WARN("GLIBManager::parseAMCEnableList::Found poorly formatted range " << *slot);
        continue;
      }
      if (isValidSlotNumber(range.at(0)) && isValidSlotNumber(range.at(1))) {
        std::stringstream ss0(range.at(0));
        std::stringstream ss1(range.at(1));
        int min, max;
        ss0 >> min;
        ss1 >> max;
        
        if (min == max) {
          WARN("GLIBManager::parseAMCEnableList::Found poorly formatted range " << *slot);
          continue;
        }
        if (min > max) {  // elements in the wrong order
          WARN("GLIBManager::parseAMCEnableList::Found poorly formatted range " << *slot);
          continue;
        }
        
        for (int islot = min; islot <= max; ++islot) {
          slotMask |= (0x1 << (islot-1));
        }  //  end loop over range of list
      }  // end check on valid values
    } else {  //not a range
      DEBUG("GLIBManager::found no hyphen in " << *slot);
      if (slot->length() > 2) {
        WARN("GLIBManager::parseAMCEnableList::Found longer value than expected (1-12) " << *slot);
        continue;
      }
      
      if (!isValidSlotNumber(*slot)) {
        WARN("GLIBManager::parseAMCEnableList::Found invalid value " << *slot);
        continue;
      }
      std::stringstream ss(*slot);
      int slotNum = -1;
      ss >> slotNum;
      slotMask |= (0x1 << (slotNum-1));
    }  // done processing single values
  }  // done looping over extracted values
  DEBUG("GLIBManager::parseAMCEnableList::Parsed enabled list 0x" << std::hex << slotMask << std::dec);
  return slotMask;
}

bool gem::hw::glib::GLIBManager::isValidSlotNumber(std::string const& s)
{
  try {
    int i_val;
    i_val = std::stoi(s);
    if (!(i_val > 0 && i_val < 13)) {
      ERROR("GLIBManager::isValidSlotNumber::Found value outside expected (1-12) " << i_val);
      return false;
    }
  } catch (std::invalid_argument const& err) {
    ERROR("GLIBManager::isValidSlotNumber::Unable to convert to integer type " << s << std::endl << err.what());
    return false;
  } catch (std::out_of_range const& err) {
    ERROR("GLIBManager::isValidSlotNumber::Unable to convert to integer type " << s << std::endl << err.what());
    return false;
  }
  // if you get here, should be possible to parse as an integer in the range [1,12]  
  return true;
}

// This is the callback used for handling xdata:Event objects
void gem::hw::glib::GLIBManager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("GLIBManager::actionPerformed() setDefaultValues" << 
          "Default configuration values have been loaded from xml profile");
    m_amcEnableMask = parseAMCEnableList(m_amcSlots.toString());
    INFO("GLIBManager::Parsed AMCEnableList m_amcSlots = " << m_amcSlots.toString()
         << " to slotMask 0x" << std::hex << m_amcEnableMask << std::dec);
    
    // how to handle passing in various values nested in a vector in a bag
    for (auto slot = m_glibInfo.begin(); slot != m_glibInfo.end(); ++slot) {
      if (slot->bag.present.value_)
        DEBUG("GLIBManager::Found attribute:" << slot->bag.toString());
    }
    // p_gemMonitor->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::glib::GLIBManager::init()
{
  // anything needed here?
}

// state transitions
void gem::hw::glib::GLIBManager::initializeAction()
  throw (gem::hw::glib::exception::Exception)
{
  DEBUG("GLIBManager::initializeAction begin");
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {    
    DEBUG("GLIBManager::looping over slots(" << (slot+1) << ") and finding expected cards");
    GLIBInfo& info = m_glibInfo[slot].bag;
    if ((m_amcEnableMask >> (slot)) & 0x1) {
      DEBUG("GLIBManager::info:" << info.toString());
      DEBUG("GLIBManager::expect a card in slot " << (slot+1));
      info.slotID  = slot+1;
      info.present = true;
      // actually check presence? this just says that we expect it to be there
      // check if there is a GLIB in the specified slot, if not, do not initialize
      // set the web view to be empty or grey
      // if (!info.present.value_) continue;
      // needs .value_?
      // p_gemWebInterface->glibInSlot(slot);
    }
  }  
  
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    GLIBInfo& info = m_glibInfo[slot].bag;
    
    // check the config file if there should be a GLIB in the specified slot, if not, do not initialize
    if (!info.present)
      continue;
    
    DEBUG("GLIBManager::info:" << info.toString());
    DEBUG("GLIBManager::creating pointer to card in slot " << (slot+1));
    
    // create the cfgInfoSpace object (qualified vs non?)
    std::string deviceName = toolbox::toString("gem.shelf%02d.glib%02d",
                                               info.crateID.value_,
                                               info.slotID.value_);
    toolbox::net::URN hwCfgURN("urn:gem:hw:"+deviceName);

    if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
      DEBUG("GLIBManager::initializeAction::infospace " << hwCfgURN.toString() << " already exists, getting");
      is_glibs[slot] = is_toolbox_ptr(new gem::base::utils::GEMInfoSpaceToolBox(this,
                                                                                xdata::getInfoSpaceFactory()->get(hwCfgURN.toString()),
                                                                                true));
    } else {
      DEBUG("GLIBManager::initializeAction::infospace " << hwCfgURN.toString() << " does not exist, creating");
      // is_glibs[slot] = xdata::getInfoSpaceFactory()->create(hwCfgURN.toString());
      is_glibs[slot] = is_toolbox_ptr(new gem::base::utils::GEMInfoSpaceToolBox(this,
                                                                                hwCfgURN.toString(),
                                                                                true));
    }
    DEBUG("GLIBManager::exporting config parameters into infospace");
    is_glibs[slot]->createString("ControlHubAddress", info.controlHubAddress.value_, &(info.controlHubAddress),
                                 GEMUpdateType::NOUPDATE);
    is_glibs[slot]->createString("IPBusProtocol",     info.ipBusProtocol.value_    , &(info.ipBusProtocol),
                                 GEMUpdateType::NOUPDATE);
    is_glibs[slot]->createString("DeviceIPAddress",   info.deviceIPAddress.value_  , &(info.deviceIPAddress),
                                 GEMUpdateType::NOUPDATE);
    is_glibs[slot]->createString("AddressTable",      info.addressTable.value_     , &(info.addressTable),
                                 GEMUpdateType::NOUPDATE);
    is_glibs[slot]->createUInt32("ControlHubPort",    info.controlHubPort.value_   , &(info.controlHubPort),
                                 GEMUpdateType::NOUPDATE);
    is_glibs[slot]->createUInt32("IPBusPort",         info.ipBusPort.value_        , &(info.ipBusPort),
                                 GEMUpdateType::NOUPDATE);
    
    DEBUG("GLIBManager::InfoSpace found item: ControlHubAddress " << is_glibs[slot]->getString("ControlHubAddress"));
    DEBUG("GLIBManager::InfoSpace found item: IPBusProtocol "     << is_glibs[slot]->getString("IPBusProtocol")    );
    DEBUG("GLIBManager::InfoSpace found item: DeviceIPAddress "   << is_glibs[slot]->getString("DeviceIPAddress")  );
    DEBUG("GLIBManager::InfoSpace found item: AddressTable "      << is_glibs[slot]->getString("AddressTable")     );
    DEBUG("GLIBManager::InfoSpace found item: ControlHubPort "    << is_glibs[slot]->getUInt32("ControlHubPort")   );
    DEBUG("GLIBManager::InfoSpace found item: IPBusPort "         << is_glibs[slot]->getUInt32("IPBusPort")        );

    try {
      DEBUG("GLIBManager::obtaining pointer to HwGLIB");
      // m_glibs[slot] = glib_shared_ptr(new gem::hw::glib::HwGLIB(info.crateID.value_,info.slotID.value_));
      m_glibs[slot] = glib_shared_ptr(new gem::hw::glib::HwGLIB(deviceName, m_connectionFile.toString()));
      if (m_glibs[slot]->isHwConnected()) {
        // maybe better to rais exception here and fail if not connected, as we expected the card to be here?
        createGLIBInfoSpaceItems(is_glibs[slot], m_glibs[slot]);
        
        m_glibMonitors[slot] = std::shared_ptr<GLIBMonitor>(new GLIBMonitor(m_glibs[slot], this, slot+1));
        m_glibMonitors[slot]->addInfoSpace("HWMonitoring", is_glibs[slot]);
        m_glibMonitors[slot]->setupHwMonitoring();
        m_glibMonitors[slot]->startMonitoring();
      } else {
        ERROR("GLIBManager:: unable to communicate with GLIB in slot " << slot);
        XCEPT_RAISE(gem::hw::glib::exception::Exception, "initializeAction failed");
      }
    } catch (uhalException const& ex) {
      ERROR("GLIBManager::caught uHAL exception " << ex.what());
      XCEPT_RAISE(gem::hw::glib::exception::Exception, "initializeAction failed");
    } catch (gem::hw::glib::exception::Exception const& ex) {
      ERROR("GLIBManager::caught exception " << ex.what());
      XCEPT_RAISE(gem::hw::glib::exception::Exception, "initializeAction failed");
    } catch (toolbox::net::exception::MalformedURN const& ex) {
      ERROR("GLIBManager::caught exception " << ex.what());
      XCEPT_RAISE(gem::hw::glib::exception::Exception, "initializeAction failed");
    } catch (std::exception const& ex) {
      ERROR("GLIBManager::caught exception " << ex.what());
      XCEPT_RAISE(gem::hw::glib::exception::Exception, "initializeAction failed");
    }
    DEBUG("GLIBManager::connected");
    // set the web view to be empty or grey
    // if (!info.present.value_) continue;
    // p_gemWebInterface->glibInSlot(slot);
  }

  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    GLIBInfo& info = m_glibInfo[slot].bag;

    if (!info.present)
      continue;
    
    if (m_glibs[slot]->isHwConnected()) {
      DEBUG("GLIBManager::connected a card in slot " << (slot+1));
    } else {
      ERROR("GLIBManager::GLIB in slot " << (slot+1) << " is not connected");
      fireEvent("Fail");
      // maybe raise exception so as to not continue with other cards? let's just return for the moment
      return;
    }
  }
  usleep(10000); // just for testing the timing of different applications
  DEBUG("GLIBManager::initializeAction end");
}

void gem::hw::glib::GLIBManager::configureAction()
  throw (gem::hw::glib::exception::Exception)
{
  DEBUG("GLIBManager::configureAction");

  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    usleep(5000); // just for testing the timing of different applications
    GLIBInfo& info = m_glibInfo[slot].bag;

    if (!info.present)
      continue;
    
    if (m_glibs[slot]->isHwConnected()) {
      DEBUG("GLIBManager::setting trigger source to 0x" << std::hex << info.triggerSource.value_ << std::dec);
      m_glibs[slot]->setTrigSource(info.triggerSource.value_);
      
      m_glibs[slot]->resetL1ACount();
      m_glibs[slot]->resetCalPulseCount();

      // reset the DAQ
      m_glibs[slot]->setL1AInhibit(0x1);
      m_glibs[slot]->resetDAQLink();
      m_glibs[slot]->setDAQLinkRunType(0x3);
      m_glibs[slot]->setDAQLinkRunParameters(0xfaac);
      
      // should FIFOs be emptied in configure or at start?
      DEBUG("GLIBManager::emptying trigger/tracking data FIFOs");
      for (unsigned link = 0; link < HwGLIB::N_GTX; ++link) {
        // m_glibs[slot]->flushTriggerFIFO(link);
        m_glibs[slot]->flushFIFO(link);
      }
      // what else is required for configuring the GLIB?
      // need to reset optical links?
      // reset counters?
      // setup run mode?
      // setup DAQ mode?
    } else {
      ERROR("GLIBManager::GLIB in slot " << (slot+1) << " is not connected");
      fireEvent("Fail");
      // maybe raise exception so as to not continue with other cards?
    }
  }
  
  DEBUG("GLIBManager::configureAction end");
}

void gem::hw::glib::GLIBManager::startAction()
  throw (gem::hw::glib::exception::Exception)
{

  INFO("gem::hw::glib::GLIBManager::startAction begin");
  // what is required for starting the GLIB?
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    usleep(500);
    DEBUG("GLIBManager::looping over slots(" << (slot+1) << ") and finding infospace items");
    GLIBInfo& info = m_glibInfo[slot].bag;

    if (!info.present)
      continue;
    
    if (m_glibs[slot]->isHwConnected()) {
      DEBUG("connected a card in slot " << (slot+1));
      // enable the DAQ
      m_glibs[slot]->enableDAQLink();
      m_glibs[slot]->setL1AInhibit(0x0);
      usleep(100); // just for testing the timing of different applications
    } else {
      ERROR("GLIB in slot " << (slot+1) << " is not connected");
      fireEvent("Fail");
      // maybe raise exception so as to not continue with other cards? let's just return for the moment
      return;
    }

    /*
    // reset the hw monitor, this was in release-v2 but not in integrated-application-framework, may have forgotten something
    if (m_glibMonitors[slot])
      m_glibMonitors[slot]->reset();
    */
  }
  usleep(10000);
  INFO("gem::hw::glib::GLIBManager::startAction end");
}

void gem::hw::glib::GLIBManager::pauseAction()
  throw (gem::hw::glib::exception::Exception)
{
  // what is required for pausing the GLIB?
  usleep(10000);  // just for testing the timing of different applications
}

void gem::hw::glib::GLIBManager::resumeAction()
  throw (gem::hw::glib::exception::Exception)
{
  // what is required for resuming the GLIB?
  usleep(10000);  // just for testing the timing of different applications
}

void gem::hw::glib::GLIBManager::stopAction()
  throw (gem::hw::glib::exception::Exception)
{
  INFO("gem::hw::glib::GLIBManager::stopAction begin");
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    usleep(500);
    DEBUG("GLIBManager::looping over slots(" << (slot+1) << ") and finding infospace items");
    GLIBInfo& info = m_glibInfo[slot].bag;

    if (!info.present)
      continue;
    
    if (m_glibs[slot]->isHwConnected()) {
      // what is required for stopping the GLIB?
      // FIXME temporarily inhibit triggers at the GLIB
      m_glibs[slot]->setL1AInhibit(0x1);
    }
  }
  usleep(10000);  // just for testing the timing of different applications
}

void gem::hw::glib::GLIBManager::haltAction()
  throw (gem::hw::glib::exception::Exception)
{
  // what is required for halting the GLIB?
  usleep(10000);  // just for testing the timing of different applications
}

void gem::hw::glib::GLIBManager::resetAction()
  throw (gem::hw::glib::exception::Exception)
{
  // what is required for resetting the GLIB?
  // unregister listeners and items in info spaces
  
  DEBUG("GLIBManager::resetAction begin");
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {    
    usleep(500);  // just for testing the timing of different applications
    DEBUG("GLIBManager::looping over slots(" << (slot+1) << ") and finding infospace items");
    GLIBInfo& info = m_glibInfo[slot].bag;
    
    if (!info.present)
      continue;

    // reset the hw monitor
    if (m_glibMonitors[slot])
      m_glibMonitors[slot]->reset();
    
    DEBUG("GLIBManager::looking for hwCfgInfoSpace items for GLIB in slot " << (slot+1));
    toolbox::net::URN hwCfgURN("urn:gem:hw:"+toolbox::toString("gem.shelf%02d.glib%02d",
                                                               info.crateID.value_,
                                                               info.slotID.value_));
    
    if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
      DEBUG("GLIBManager::revoking config parameters infospace");
      // reset the hw infospace toolbox
      is_glibs[slot]->reset();
      // these should now be gone from the reset call..., holdover from the old way
      if (is_glibs[slot]->getInfoSpace()->hasItem("ControlHubAddress"))
        is_glibs[slot]->getInfoSpace()->fireItemRevoked("ControlHubAddress");
      
      if (is_glibs[slot]->getInfoSpace()->hasItem("IPBusProtocol"))
        is_glibs[slot]->getInfoSpace()->fireItemRevoked("IPBusProtocol");
      
      if (is_glibs[slot]->getInfoSpace()->hasItem("DeviceIPAddress"))
        is_glibs[slot]->getInfoSpace()->fireItemRevoked("DeviceIPAddress");
      
      if (is_glibs[slot]->getInfoSpace()->hasItem("AddressTable"))
        is_glibs[slot]->getInfoSpace()->fireItemRevoked("AddressTable");
      
      if (is_glibs[slot]->getInfoSpace()->hasItem("ControlHubPort"))
        is_glibs[slot]->getInfoSpace()->fireItemRevoked("ControlHubPort");
      
      if (is_glibs[slot]->getInfoSpace()->hasItem("IPBusPort"))
        is_glibs[slot]->getInfoSpace()->fireItemRevoked("IPBusPort");
    } else {
      DEBUG("GLIBManager::resetAction::infospace " << hwCfgURN.toString() << " does not exist, no further action");
      continue;
    }
  }
  // gem::base::GEMFSMApplication::resetAction();
}

/*
  void gem::hw::glib::GLIBManager::noAction()
  throw (gem::hw::glib::exception::Exception)
  {
  }
*/

void gem::hw::glib::GLIBManager::failAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}

void gem::hw::glib::GLIBManager::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}


void gem::hw::glib::GLIBManager::createGLIBInfoSpaceItems(is_toolbox_ptr is_glib, glib_shared_ptr glib)
{
  // system registers
  is_glib->createUInt32("BOARD_ID",      glib->getBoardIDRaw(),      NULL, GEMUpdateType::NOUPDATE, "docstring", "id");
  is_glib->createUInt32("SYSTEM_ID",     glib->getSystemIDRaw(),     NULL, GEMUpdateType::NOUPDATE, "docstring", "id");
  is_glib->createUInt32("FIRMWARE_ID",   glib->getFirmwareVerRaw(),  NULL, GEMUpdateType::PROCESS,  "docstring", "fwver");
  is_glib->createUInt32("FIRMWARE_DATE", glib->getFirmwareDateRaw(), NULL, GEMUpdateType::PROCESS,  "docstring", "date");
  is_glib->createUInt32("IP_ADDRESS",    glib->getIPAddressRaw(),    NULL, GEMUpdateType::NOUPDATE, "docstring", "ip");
  is_glib->createUInt64("MAC_ADDRESS",   glib->getMACAddressRaw(),   NULL, GEMUpdateType::NOUPDATE, "docstring", "mac");
  is_glib->createUInt32("SFP1_STATUS",   glib->SFPStatus(1),         NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("SFP2_STATUS",   glib->SFPStatus(2),         NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("SFP3_STATUS",   glib->SFPStatus(3),         NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("SFP4_STATUS",   glib->SFPStatus(4),         NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("FMC1_STATUS",   glib->FMCPresence(0),       NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("FMC2_STATUS",   glib->FMCPresence(1),       NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("FPGA_RESET",    glib->FPGAResetStatus(),    NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("GBE_INT",       glib->GbEInterrupt(),       NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("V6_CPLD",       glib->V6CPLDStatus(),       NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("CPLD_LOCK",     glib->CDCELockStatus(),     NULL, GEMUpdateType::HW32);

  // ttc registers
  is_glib->createUInt32("L1A",      glib->getL1ACount(),      NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("CalPulse", glib->getCalPulseCount(), NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("Resync",   glib->getResyncCount(),   NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("BC0",      glib->getBC0Count(),      NULL, GEMUpdateType::HW32);

  // DAQ link registers
  is_glib->createUInt32("CONTROL",           glib->getDAQLinkControl(),               NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("STATUS",            glib->getDAQLinkStatus(),                NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("INPUT_ENABLE_MASK", glib->getDAQLinkInputMask(),             NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("DAV_TIMEOUT",       glib->getDAQLinkDAVTimeout(),            NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("MAX_DAV_TIMER",     glib->getDAQLinkDAVTimer(0),             NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("LAST_DAV_TIMER",    glib->getDAQLinkDAVTimer(1),             NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("NOTINTABLE_ERR",    glib->getDAQLinkNonidentifiableErrors(), NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("DISPER_ERR",        glib->getDAQLinkDisperErrors(),          NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("EVT_SENT",          glib->getDAQLinkEventsSent(),            NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("L1AID",             glib->getDAQLinkL1AID(),                 NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("INPUT_TIMEOUT",     glib->getDAQLinkL1AID(),                 NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("RUN_TYPE",          glib->getDAQLinkL1AID(),                 NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("RUN_PARAMS",        glib->getDAQLinkL1AID(),                 NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("SBIT_RATE",         glib->getDAQLinkL1AID(),                 NULL, GEMUpdateType::HW32);

  is_glib->createUInt32("GTX0_STATUS",               glib->getDAQLinkStatus(0),      NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("GTX0_CORRUPT_VFAT_BLK_CNT", glib->getDAQLinkCounters(0, 0), NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("GTX0_EVN",                  glib->getDAQLinkCounters(0, 1), NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("GTX0_DAV_TIMEOUT",          glib->getDAQLinkDAVTimer(0),    NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("GTX0_MAX_DAV_TIMER",        glib->getDAQLinkDAVTimer(0),    NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("GTX0_LAST_DAV_TIMER",       glib->getDAQLinkDAVTimer(1),    NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("GTX0_CLUSTER_01",           glib->getDAQLinkCounters(0, 1), NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("GTX0_CLUSTER_23",           glib->getDAQLinkCounters(0, 1), NULL, GEMUpdateType::HW32);

  is_glib->createUInt32("GTX1_STATUS",               glib->getDAQLinkStatus(1),      NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("GTX1_CORRUPT_VFAT_BLK_CNT", glib->getDAQLinkCounters(1, 0), NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("GTX1_EVN",                  glib->getDAQLinkCounters(1, 1), NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("GTX1_DAV_TIMEOUT",          glib->getDAQLinkDAVTimer(0),    NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("GTX1_MAX_DAV_TIMER",        glib->getDAQLinkDAVTimer(0),    NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("GTX1_LAST_DAV_TIMER",       glib->getDAQLinkDAVTimer(1),    NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("GTX1_CLUSTER_01",           glib->getDAQLinkCounters(1, 1), NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("GTX1_CLUSTER_23",           glib->getDAQLinkCounters(1, 1), NULL, GEMUpdateType::HW32);

  // request counters
  is_glib->createUInt64("OptoHybrid_0", 0, NULL, GEMUpdateType::I2CSTAT, "docstring", "i2c/hex");
  is_glib->createUInt64("OptoHybrid_1", 0, NULL, GEMUpdateType::I2CSTAT, "docstring", "i2c/hex");
  is_glib->createUInt64("TRK_0",        0, NULL, GEMUpdateType::I2CSTAT, "docstring", "i2c/hex");
  is_glib->createUInt64("TRK_1",        0, NULL, GEMUpdateType::I2CSTAT, "docstring", "i2c/hex");
  is_glib->createUInt64("Counters",     0, NULL, GEMUpdateType::I2CSTAT, "docstring", "i2c/hex");

  // link status registers
  is_glib->createUInt32("GTX0_TRG_ERR",      0, NULL, GEMUpdateType::PROCESS, "docstring", "raw/rate");
  is_glib->createUInt32("GTX0_TRK_ERR",      0, NULL, GEMUpdateType::PROCESS, "docstring", "raw/rate");
  is_glib->createUInt32("GTX0_DATA_Packets", 0, NULL, GEMUpdateType::PROCESS, "docstring", "raw/rate");
  is_glib->createUInt32("GTX1_TRG_ERR",      0, NULL, GEMUpdateType::PROCESS, "docstring", "raw/rate");
  is_glib->createUInt32("GTX1_TRK_ERR",      0, NULL, GEMUpdateType::PROCESS, "docstring", "raw/rate");
  is_glib->createUInt32("GTX1_DATA_Packets", 0, NULL, GEMUpdateType::PROCESS, "docstring", "raw/rate");

  // TTC registers
  is_glib->createUInt32("TTC_CONTROL", glib->getTTCControl(),   NULL, GEMUpdateType::HW32);
  is_glib->createUInt32("TTC_SPY",     glib->getTTCSpyBuffer(), NULL, GEMUpdateType::HW32);
}
