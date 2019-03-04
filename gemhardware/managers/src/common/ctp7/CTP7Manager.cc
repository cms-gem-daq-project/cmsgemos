/**
 * class: CTP7Manager
 * description: Manager application for CTP7 cards
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date:
 */

#include "gem/hw/ctp7/CTP7Manager.h"

#include "gem/hw/ctp7/HwCTP7.h"
#include "gem/hw/ctp7/CTP7Monitor.h"
#include "gem/hw/ctp7/CTP7ManagerWeb.h"

#include "gem/hw/ctp7/exception/Exception.h"

typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

XDAQ_INSTANTIATOR_IMPL(gem::hw::ctp7::CTP7Manager);

gem::hw::ctp7::CTP7Manager::CTP7Info::CTP7Info()
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

  sbitSource    = 0;
}

void gem::hw::ctp7::CTP7Manager::CTP7Info::registerFields(xdata::Bag<gem::hw::ctp7::CTP7Manager::CTP7Info>* bag)
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

  bag->addField("sbitSource",    &sbitSource);
}

gem::hw::ctp7::CTP7Manager::CTP7Manager(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub),
  m_amcEnableMask(0)
{
  m_ctp7Info.setSize(MAX_AMCS_PER_CRATE);

  p_appInfoSpace->fireItemAvailable("AllCTP7sInfo",   &m_ctp7Info);
  p_appInfoSpace->fireItemAvailable("AMCSlots",       &m_amcSlots);
  p_appInfoSpace->fireItemAvailable("ConnectionFile", &m_connectionFile);

  p_appInfoSpace->addItemRetrieveListener("AllCTP7sInfo",   this);
  p_appInfoSpace->addItemRetrieveListener("AMCSlots",       this);
  p_appInfoSpace->addItemRetrieveListener("ConnectionFile", this);
  p_appInfoSpace->addItemChangedListener( "AllCTP7sInfo",   this);
  p_appInfoSpace->addItemChangedListener( "AMCSlots",       this);
  p_appInfoSpace->addItemChangedListener( "ConnectionFile", this);

  xgi::bind(this, &CTP7Manager::dumpCTP7FIFO, "dumpCTP7FIFO");

  // initialize the CTP7 application objects
  CMSGEMOS_DEBUG("CTP7Manager::Connecting to the CTP7ManagerWeb interface");
  p_gemWebInterface = new gem::hw::ctp7::CTP7ManagerWeb(this);
  // p_gemMonitor      = new gem::hw::ctp7::CTP7HwMonitor(this);
  CMSGEMOS_DEBUG("CTP7Manager::done");

  // set up the info hwCfgInfoSpace
  init();

  // getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemhardware/images/ctp7/CTP7Manager.png");
}

gem::hw::ctp7::CTP7Manager::~CTP7Manager()
{
  // memory management, maybe not necessary here?
}

std::vector<uint32_t> gem::hw::ctp7::CTP7Manager::dumpCTP7FIFO(int const& ctp7)
{
  std::vector<uint32_t> dump;
  if (ctp7 < 0 || ctp7 > 11) {
    CMSGEMOS_WARN("CTP7Manager::dumpCTP7FIFO Specified invalid CTP7 card " << ctp7+1);
    return dump;
  } else if (!m_ctp7s.at(ctp7)) {
    CMSGEMOS_WARN("CTP7Manager::dumpCTP7FIFO Specified CTP7 card " << ctp7+1
         << " is not connected");
    return dump;
  //} else if (!(m_ctp7s.at(ctp7)->hasTrackingData(0))) {
  //  CMSGEMOS_WARN("CTP7Manager::dumpCTP7FIFO Specified CTP7 card " << ctp7
  //       << " has no tracking data in the FIFO");
  //  return dump;
  }

  try {
    CMSGEMOS_INFO("CTP7Manager::dumpCTP7FIFO Dumping FIFO for specified CTP7 card " << ctp7+1);
    return m_ctp7s.at(ctp7)->getTrackingData(0, 24);
  } catch (gem::hw::ctp7::exception::Exception const& ex) {
    CMSGEMOS_ERROR("CTP7Manager::dumpCTP7FIFO Unable to read tracking data from CTP7 " << ctp7+1
          << " FIFO, caught exception " << ex.what());
    return dump;
  } catch (std::exception const& ex) {
    CMSGEMOS_ERROR("CTP7Manager::dumpCTP7FIFO Unable to read tracking data from CTP7 " << ctp7+1
          << " FIFO,  caught exception " << ex.what());
    return dump;
  } catch (...) {
    CMSGEMOS_ERROR("CTP7Manager::dumpCTP7FIFO Unable to read tracking data from CTP7 " << ctp7+1
          << " FIFO");
    return dump;
  }
}

uint16_t gem::hw::ctp7::CTP7Manager::parseAMCEnableList(std::string const& enableList)
{
  uint16_t slotMask = 0x0;
  std::vector<std::string> slots;

  boost::split(slots, enableList, boost::is_any_of(", "), boost::token_compress_on);
  CMSGEMOS_DEBUG("CTP7Manager::AMC input enable list is " << enableList);
  // would be great to multithread this portion
  for (auto slot = slots.begin(); slot != slots.end(); ++slot) {
    CMSGEMOS_DEBUG("CTP7Manager::slot is " << *slot);
    if (slot->find('-') != std::string::npos) {  // found a possible range
      CMSGEMOS_DEBUG("CTP7Manager::found a hyphen in " << *slot);
      std::vector<std::string> range;
      boost::split(range, *slot, boost::is_any_of("-"), boost::token_compress_on);
      if (range.size() > 2) {
        CMSGEMOS_WARN("CTP7Manager::parseAMCEnableList::Found poorly formatted range " << *slot);
        continue;
      }
      if (isValidSlotNumber(range.at(0)) && isValidSlotNumber(range.at(1))) {
        std::stringstream ss0(range.at(0));
        std::stringstream ss1(range.at(1));
        int min, max;
        ss0 >> min;
        ss1 >> max;

        if (min == max) {
          CMSGEMOS_WARN("CTP7Manager::parseAMCEnableList::Found poorly formatted range " << *slot);
          continue;
        }
        if (min > max) {  // elements in the wrong order
          CMSGEMOS_WARN("CTP7Manager::parseAMCEnableList::Found poorly formatted range " << *slot);
          continue;
        }

        for (int islot = min; islot <= max; ++islot) {
          slotMask |= (0x1 << (islot-1));
        }  //  end loop over range of list
      }  // end check on valid values
    } else {  //not a range
      CMSGEMOS_DEBUG("CTP7Manager::found no hyphen in " << *slot);
      if (slot->length() > 2) {
        CMSGEMOS_WARN("CTP7Manager::parseAMCEnableList::Found longer value than expected (1-12) " << *slot);
        continue;
      }

      if (!isValidSlotNumber(*slot)) {
        CMSGEMOS_WARN("CTP7Manager::parseAMCEnableList::Found invalid value " << *slot);
        continue;
      }
      std::stringstream ss(*slot);
      int slotNum = -1;
      ss >> slotNum;
      slotMask |= (0x1 << (slotNum-1));
    }  // done processing single values
  }  // done looping over extracted values
  CMSGEMOS_DEBUG("CTP7Manager::parseAMCEnableList::Parsed enabled list 0x" << std::hex << slotMask << std::dec);
  return slotMask;
}

bool gem::hw::ctp7::CTP7Manager::isValidSlotNumber(std::string const& s)
{
  try {
    int i_val;
    i_val = std::stoi(s);
    if (!(i_val > 0 && i_val < 13)) {
      CMSGEMOS_ERROR("CTP7Manager::isValidSlotNumber::Found value outside expected (1-12) " << i_val);
      return false;
    }
  } catch (std::invalid_argument const& err) {
    CMSGEMOS_ERROR("CTP7Manager::isValidSlotNumber::Unable to convert to integer type " << s << std::endl << err.what());
    return false;
  } catch (std::out_of_range const& err) {
    CMSGEMOS_ERROR("CTP7Manager::isValidSlotNumber::Unable to convert to integer type " << s << std::endl << err.what());
    return false;
  }
  // if you get here, should be possible to parse as an integer in the range [1,12]
  return true;
}

// This is the callback used for handling xdata:Event objects
void gem::hw::ctp7::CTP7Manager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    CMSGEMOS_DEBUG("CTP7Manager::actionPerformed() setDefaultValues" <<
          "Default configuration values have been loaded from xml profile");
    m_amcEnableMask = parseAMCEnableList(m_amcSlots.toString());
    CMSGEMOS_INFO("CTP7Manager::Parsed AMCEnableList m_amcSlots = " << m_amcSlots.toString()
         << " to slotMask 0x" << std::hex << m_amcEnableMask << std::dec);

    // how to handle passing in various values nested in a vector in a bag
    for (auto slot = m_ctp7Info.begin(); slot != m_ctp7Info.end(); ++slot) {
      if (slot->bag.present.value_)
        CMSGEMOS_DEBUG("CTP7Manager::Found attribute:" << slot->bag.toString());
    }
    // p_gemMonitor->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::ctp7::CTP7Manager::init()
{
  // anything needed here?
}

// state transitions
void gem::hw::ctp7::CTP7Manager::initializeAction()
  throw (gem::hw::ctp7::exception::Exception)
{
  CMSGEMOS_DEBUG("CTP7Manager::initializeAction begin");
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    CMSGEMOS_DEBUG("CTP7Manager::looping over slots(" << (slot+1) << ") and finding expected cards");
    CTP7Info& info = m_ctp7Info[slot].bag;
    if ((m_amcEnableMask >> (slot)) & 0x1) {
      CMSGEMOS_DEBUG("CTP7Manager::info:" << info.toString());
      CMSGEMOS_DEBUG("CTP7Manager::expect a card in slot " << (slot+1));
      info.slotID  = slot+1;
      info.present = true;
      // actually check presence? this just says that we expect it to be there
      // check if there is a CTP7 in the specified slot, if not, do not initialize
      // set the web view to be empty or grey
      // if (!info.present.value_) continue;
      // needs .value_?
      // p_gemWebInterface->ctp7InSlot(slot);
    }
  }

  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    CTP7Info& info = m_ctp7Info[slot].bag;

    // check the config file if there should be a CTP7 in the specified slot, if not, do not initialize
    if (!info.present)
      continue;

    CMSGEMOS_DEBUG("CTP7Manager::info:" << info.toString());
    CMSGEMOS_DEBUG("CTP7Manager::creating pointer to card in slot " << (slot+1));

    // create the cfgInfoSpace object (qualified vs non?)
    std::string deviceName = toolbox::toString("gem.shelf%02d.ctp7%02d",
                                               info.crateID.value_,
                                               info.slotID.value_);
    toolbox::net::URN hwCfgURN("urn:gem:hw:"+deviceName);

    if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
      CMSGEMOS_DEBUG("CTP7Manager::initializeAction::infospace " << hwCfgURN.toString() << " already exists, getting");
      is_ctp7s.at(slot) = is_toolbox_ptr(new gem::base::utils::GEMInfoSpaceToolBox(this,
                                                                                   xdata::getInfoSpaceFactory()->get(hwCfgURN.toString()),
                                                                                   true));
    } else {
      CMSGEMOS_DEBUG("CTP7Manager::initializeAction::infospace " << hwCfgURN.toString() << " does not exist, creating");
      // is_ctp7s.at(slot) = xdata::getInfoSpaceFactory()->create(hwCfgURN.toString());
      is_ctp7s.at(slot) = is_toolbox_ptr(new gem::base::utils::GEMInfoSpaceToolBox(this,
                                                                                   hwCfgURN.toString(),
                                                                                   true));
    }

    CMSGEMOS_DEBUG("CTP7Manager::exporting config parameters into infospace");
    is_ctp7s.at(slot)->createString("ControlHubAddress", info.controlHubAddress.value_, &(info.controlHubAddress),
                                 GEMUpdateType::NOUPDATE);
    is_ctp7s.at(slot)->createString("IPBusProtocol",     info.ipBusProtocol.value_    , &(info.ipBusProtocol),
                                 GEMUpdateType::NOUPDATE);
    is_ctp7s.at(slot)->createString("DeviceIPAddress",   info.deviceIPAddress.value_  , &(info.deviceIPAddress),
                                 GEMUpdateType::NOUPDATE);
    is_ctp7s.at(slot)->createString("AddressTable",      info.addressTable.value_     , &(info.addressTable),
                                 GEMUpdateType::NOUPDATE);
    is_ctp7s.at(slot)->createUInt32("ControlHubPort",    info.controlHubPort.value_   , &(info.controlHubPort),
                                 GEMUpdateType::NOUPDATE);
    is_ctp7s.at(slot)->createUInt32("IPBusPort",         info.ipBusPort.value_        , &(info.ipBusPort),
                                 GEMUpdateType::NOUPDATE);

    CMSGEMOS_DEBUG("CTP7Manager::InfoSpace found item: ControlHubAddress " << is_ctp7s.at(slot)->getString("ControlHubAddress"));
    CMSGEMOS_DEBUG("CTP7Manager::InfoSpace found item: IPBusProtocol "     << is_ctp7s.at(slot)->getString("IPBusProtocol")    );
    CMSGEMOS_DEBUG("CTP7Manager::InfoSpace found item: DeviceIPAddress "   << is_ctp7s.at(slot)->getString("DeviceIPAddress")  );
    CMSGEMOS_DEBUG("CTP7Manager::InfoSpace found item: AddressTable "      << is_ctp7s.at(slot)->getString("AddressTable")     );
    CMSGEMOS_DEBUG("CTP7Manager::InfoSpace found item: ControlHubPort "    << is_ctp7s.at(slot)->getUInt32("ControlHubPort")   );
    CMSGEMOS_DEBUG("CTP7Manager::InfoSpace found item: IPBusPort "         << is_ctp7s.at(slot)->getUInt32("IPBusPort")        );

    try {
      CMSGEMOS_DEBUG("CTP7Manager::obtaining pointer to HwCTP7");
      // m_ctp7s.at(slot) = ctp7_shared_ptr(new gem::hw::ctp7::HwCTP7(info.crateID.value_,info.slotID.value_));
      m_ctp7s.at(slot) = ctp7_shared_ptr(new gem::hw::ctp7::HwCTP7(deviceName, m_connectionFile.toString()));
      if (m_ctp7s.at(slot)->isHwConnected()) {
        // maybe better to rais exception here and fail if not connected, as we expected the card to be here?
        createCTP7InfoSpaceItems(is_ctp7s.at(slot), m_ctp7s.at(slot));

        m_ctp7Monitors.at(slot) = std::shared_ptr<CTP7Monitor>(new CTP7Monitor(m_ctp7s.at(slot), this, slot+1));
        m_ctp7Monitors.at(slot)->addInfoSpace("HWMonitoring", is_ctp7s.at(slot));
        m_ctp7Monitors.at(slot)->setupHwMonitoring();
        m_ctp7Monitors.at(slot)->startMonitoring();
      } else {
        CMSGEMOS_ERROR("CTP7Manager:: unable to communicate with CTP7 in slot " << slot);
        XCEPT_RAISE(gem::hw::ctp7::exception::Exception, "initializeAction failed");
      }
    } catch (gem::hw::ctp7::exception::Exception const& ex) {
      CMSGEMOS_ERROR("CTP7Manager::caught exception " << ex.what());
      XCEPT_RAISE(gem::hw::ctp7::exception::Exception, "initializeAction failed");
    } catch (toolbox::net::exception::MalformedURN const& ex) {
      CMSGEMOS_ERROR("CTP7Manager::caught exception " << ex.what());
      XCEPT_RAISE(gem::hw::ctp7::exception::Exception, "initializeAction failed");
    } catch (std::exception const& ex) {
      CMSGEMOS_ERROR("CTP7Manager::caught exception " << ex.what());
      XCEPT_RAISE(gem::hw::ctp7::exception::Exception, "initializeAction failed");
    }
    CMSGEMOS_DEBUG("CTP7Manager::connected");
    // set the web view to be empty or grey
    // if (!info.present.value_) continue;
    // p_gemWebInterface->ctp7InSlot(slot);
  }

  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    CTP7Info& info = m_ctp7Info[slot].bag;

    if (!info.present)
      continue;

    if (m_ctp7s.at(slot)->isHwConnected()) {
      CMSGEMOS_DEBUG("CTP7Manager::connected a card in slot " << (slot+1));
    } else {
      CMSGEMOS_ERROR("CTP7Manager::CTP7 in slot " << (slot+1) << " is not connected");
      fireEvent("Fail");
      // maybe raise exception so as to not continue with other cards? let's just return for the moment
      return;
    }
  }
  usleep(10000); // just for testing the timing of different applications
  CMSGEMOS_DEBUG("CTP7Manager::initializeAction end");
}

void gem::hw::ctp7::CTP7Manager::configureAction()
  throw (gem::hw::ctp7::exception::Exception)
{
  CMSGEMOS_DEBUG("CTP7Manager::configureAction");

  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    usleep(5000); // just for testing the timing of different applications
    CTP7Info& info = m_ctp7Info[slot].bag;

    if (!info.present)
      continue;

    if (m_ctp7s.at(slot)->isHwConnected()) {
      m_ctp7s.at(slot)->resetL1ACount();
      m_ctp7s.at(slot)->resetCalPulseCount();

      // reset the DAQ
      m_ctp7s.at(slot)->setL1AInhibit(0x1);
      m_ctp7s.at(slot)->resetDAQLink();
      m_ctp7s.at(slot)->setDAQLinkRunType(0x3);
      m_ctp7s.at(slot)->setDAQLinkRunParameters(0xfaac);

      // should FIFOs be emptied in configure or at start?
      CMSGEMOS_DEBUG("CTP7Manager::emptying trigger/tracking data FIFOs");
      for (unsigned gtx = 0; gtx < HwCTP7::N_GTX; ++gtx) {
        // m_ctp7s.at(slot)->flushTriggerFIFO(gtx);
        m_ctp7s.at(slot)->flushFIFO(gtx);
      }
      // what else is required for configuring the CTP7?
      // need to reset optical links?
      // reset counters?
      // setup run mode?
      // setup DAQ mode?
    } else {
      CMSGEMOS_ERROR("CTP7Manager::CTP7 in slot " << (slot+1) << " is not connected");
      fireEvent("Fail");
      // maybe raise exception so as to not continue with other cards?
    }
  }

  CMSGEMOS_DEBUG("CTP7Manager::configureAction end");
}

void gem::hw::ctp7::CTP7Manager::startAction()
  throw (gem::hw::ctp7::exception::Exception)
{

  CMSGEMOS_INFO("gem::hw::ctp7::CTP7Manager::startAction begin");
  // what is required for starting the CTP7?
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    usleep(500);
    CMSGEMOS_DEBUG("CTP7Manager::looping over slots(" << (slot+1) << ") and finding infospace items");
    CTP7Info& info = m_ctp7Info[slot].bag;

    if (!info.present)
      continue;

    if (m_ctp7s.at(slot)->isHwConnected()) {
      CMSGEMOS_DEBUG("connected a card in slot " << (slot+1));
      // enable the DAQ
      m_ctp7s.at(slot)->enableDAQLink();
      m_ctp7s.at(slot)->setL1AInhibit(0x0);
      usleep(100); // just for testing the timing of different applications
    } else {
      CMSGEMOS_ERROR("CTP7 in slot " << (slot+1) << " is not connected");
      fireEvent("Fail");
      // maybe raise exception so as to not continue with other cards? let's just return for the moment
      return;
    }

    /*
    // reset the hw monitor, this was in release-v2 but not in integrated-application-framework, may have forgotten something
    if (m_ctp7Monitors.at(slot))
      m_ctp7Monitors.at(slot)->reset();
    */
  }
  usleep(10000);
  CMSGEMOS_INFO("gem::hw::ctp7::CTP7Manager::startAction end");
}

void gem::hw::ctp7::CTP7Manager::pauseAction()
  throw (gem::hw::ctp7::exception::Exception)
{
  // what is required for pausing the CTP7?
  usleep(10000);  // just for testing the timing of different applications
}

void gem::hw::ctp7::CTP7Manager::resumeAction()
  throw (gem::hw::ctp7::exception::Exception)
{
  // what is required for resuming the CTP7?
  usleep(10000);  // just for testing the timing of different applications
}

void gem::hw::ctp7::CTP7Manager::stopAction()
  throw (gem::hw::ctp7::exception::Exception)
{
  CMSGEMOS_INFO("gem::hw::ctp7::CTP7Manager::stopAction begin");
  // what is required for stopping the CTP7?
  usleep(10000);  // just for testing the timing of different applications
}

void gem::hw::ctp7::CTP7Manager::haltAction()
  throw (gem::hw::ctp7::exception::Exception)
{
  // what is required for halting the CTP7?
  usleep(10000);  // just for testing the timing of different applications
}

void gem::hw::ctp7::CTP7Manager::resetAction()
  throw (gem::hw::ctp7::exception::Exception)
{
  // what is required for resetting the CTP7?
  // unregister listeners and items in info spaces

  CMSGEMOS_DEBUG("CTP7Manager::resetAction begin");
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    usleep(500);  // just for testing the timing of different applications
    CMSGEMOS_DEBUG("CTP7Manager::looping over slots(" << (slot+1) << ") and finding infospace items");
    CTP7Info& info = m_ctp7Info[slot].bag;

    if (!info.present)
      continue;

    // reset the hw monitor
    if (m_ctp7Monitors.at(slot))
      m_ctp7Monitors.at(slot)->reset();

    CMSGEMOS_DEBUG("CTP7Manager::looking for hwCfgInfoSpace items for CTP7 in slot " << (slot+1));
    toolbox::net::URN hwCfgURN("urn:gem:hw:"+toolbox::toString("gem.shelf%02d.ctp7%02d",
                                                               info.crateID.value_,
                                                               info.slotID.value_));

    if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
      CMSGEMOS_DEBUG("CTP7Manager::revoking config parameters infospace");

      // reset the hw infospace toolbox
      is_ctp7s.at(slot)->reset();

      // these should now be gone from the reset call..., holdover from the old way
      if (is_ctp7s.at(slot)->getInfoSpace()->hasItem("ControlHubAddress"))
        is_ctp7s.at(slot)->getInfoSpace()->fireItemRevoked("ControlHubAddress");

      if (is_ctp7s.at(slot)->getInfoSpace()->hasItem("IPBusProtocol"))
        is_ctp7s.at(slot)->getInfoSpace()->fireItemRevoked("IPBusProtocol");

      if (is_ctp7s.at(slot)->getInfoSpace()->hasItem("DeviceIPAddress"))
        is_ctp7s.at(slot)->getInfoSpace()->fireItemRevoked("DeviceIPAddress");

      if (is_ctp7s.at(slot)->getInfoSpace()->hasItem("AddressTable"))
        is_ctp7s.at(slot)->getInfoSpace()->fireItemRevoked("AddressTable");

      if (is_ctp7s.at(slot)->getInfoSpace()->hasItem("ControlHubPort"))
        is_ctp7s.at(slot)->getInfoSpace()->fireItemRevoked("ControlHubPort");

      if (is_ctp7s.at(slot)->getInfoSpace()->hasItem("IPBusPort"))
        is_ctp7s.at(slot)->getInfoSpace()->fireItemRevoked("IPBusPort");
    } else {
      CMSGEMOS_DEBUG("CTP7Manager::resetAction::infospace " << hwCfgURN.toString() << " does not exist, no further action");
      continue;
    }
  }
  // gem::base::GEMFSMApplication::resetAction();
}

/*
  void gem::hw::ctp7::CTP7Manager::noAction()
  throw (gem::hw::ctp7::exception::Exception)
  {
  }
*/

void gem::hw::ctp7::CTP7Manager::failAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}

void gem::hw::ctp7::CTP7Manager::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}


void gem::hw::ctp7::CTP7Manager::createCTP7InfoSpaceItems(is_toolbox_ptr is_ctp7, ctp7_shared_ptr ctp7)
{
  // system registers
  is_ctp7->createUInt32("BOARD_ID",      ctp7->getBoardID(),      NULL, GEMUpdateType::NOUPDATE, "docstring", "id");
  is_ctp7->createUInt32("SYSTEM_ID",     ctp7->getSystemID(),     NULL, GEMUpdateType::NOUPDATE, "docstring", "id");
  is_ctp7->createUInt32("FIRMWARE_ID",   ctp7->getFirmwareVer(),  NULL, GEMUpdateType::PROCESS,  "docstring", "fwver");
  is_ctp7->createUInt32("FIRMWARE_DATE", ctp7->getFirmwareDate(), NULL, GEMUpdateType::PROCESS,  "docstring", "date");
  is_ctp7->createUInt32("IP_ADDRESS",    ctp7->getIPAddress(),    NULL, GEMUpdateType::NOUPDATE, "docstring", "ip");
  is_ctp7->createUInt64("MAC_ADDRESS",   ctp7->getMACAddress(),   NULL, GEMUpdateType::NOUPDATE, "docstring", "mac");
  is_ctp7->createUInt32("SFP1_STATUS",   ctp7->SFPStatus(1),      NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("SFP2_STATUS",   ctp7->SFPStatus(2),      NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("SFP3_STATUS",   ctp7->SFPStatus(3),      NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("SFP4_STATUS",   ctp7->SFPStatus(4),      NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("FMC1_STATUS",   ctp7->FMCPresence(0),    NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("FMC2_STATUS",   ctp7->FMCPresence(1),    NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("FPGA_RESET",    ctp7->FPGAResetStatus(), NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("GBE_INT",       ctp7->GbEInterrupt(),    NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("V6_CPLD",       ctp7->V6CPLDStatus(),    NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("CPLD_LOCK",     ctp7->CDCELockStatus(),  NULL, GEMUpdateType::HW32);

  // ttc registers
  is_ctp7->createUInt32("L1A",      ctp7->getL1ACount(),      NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("CalPulse", ctp7->getCalPulseCount(), NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("Resync",   ctp7->getResyncCount(),   NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("BC0",      ctp7->getBC0Count(),      NULL, GEMUpdateType::HW32);

  // DAQ link registers
  is_ctp7->createUInt32("CONTROL",           ctp7->getDAQLinkControl(),               NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("STATUS",            ctp7->getDAQLinkStatus(),                NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("INPUT_ENABLE_MASK", ctp7->getDAQLinkInputMask(),             NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("DAV_TIMEOUT",       ctp7->getDAQLinkDAVTimeout(),            NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("MAX_DAV_TIMER",     ctp7->getDAQLinkDAVTimer(0),             NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("LAST_DAV_TIMER",    ctp7->getDAQLinkDAVTimer(1),             NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("NOTINTABLE_ERR",    ctp7->getDAQLinkNonidentifiableErrors(), NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("DISPER_ERR",        ctp7->getDAQLinkDisperErrors(),          NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("EVT_SENT",          ctp7->getDAQLinkEventsSent(),            NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("L1AID",             ctp7->getDAQLinkL1AID(),                 NULL, GEMUpdateType::HW32);

  is_ctp7->createUInt32("GTX0_DAQ_STATUS",               ctp7->getDAQLinkStatus(0),      NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("GTX0_DAQ_CORRUPT_VFAT_BLK_CNT", ctp7->getDAQLinkCounters(0, 0), NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("GTX0_DAQ_EVN",                  ctp7->getDAQLinkCounters(0, 1), NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("GTX1_DAQ_STATUS",               ctp7->getDAQLinkStatus(1),      NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("GTX1_DAQ_CORRUPT_VFAT_BLK_CNT", ctp7->getDAQLinkCounters(1, 0), NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("GTX1_DAQ_EVN",                  ctp7->getDAQLinkCounters(1, 1), NULL, GEMUpdateType::HW32);

  // request counters
  is_ctp7->createUInt64("OptoHybrid_0", 0, NULL, GEMUpdateType::I2CSTAT, "docstring", "i2c/hex");
  is_ctp7->createUInt64("OptoHybrid_1", 0, NULL, GEMUpdateType::I2CSTAT, "docstring", "i2c/hex");
  is_ctp7->createUInt64("TRK_0",        0, NULL, GEMUpdateType::I2CSTAT, "docstring", "i2c/hex");
  is_ctp7->createUInt64("TRK_1",        0, NULL, GEMUpdateType::I2CSTAT, "docstring", "i2c/hex");
  is_ctp7->createUInt64("Counters",     0, NULL, GEMUpdateType::I2CSTAT, "docstring", "i2c/hex");

  // link status registers
  is_ctp7->createUInt32("GTX0_TRG_ERR",      0, NULL, GEMUpdateType::PROCESS, "docstring", "raw/rate");
  is_ctp7->createUInt32("GTX0_TRK_ERR",      0, NULL, GEMUpdateType::PROCESS, "docstring", "raw/rate");
  is_ctp7->createUInt32("GTX0_DATA_Packets", 0, NULL, GEMUpdateType::PROCESS, "docstring", "raw/rate");
  is_ctp7->createUInt32("GTX1_TRG_ERR",      0, NULL, GEMUpdateType::PROCESS, "docstring", "raw/rate");
  is_ctp7->createUInt32("GTX1_TRK_ERR",      0, NULL, GEMUpdateType::PROCESS, "docstring", "raw/rate");
  is_ctp7->createUInt32("GTX1_DATA_Packets", 0, NULL, GEMUpdateType::PROCESS, "docstring", "raw/rate");

  // TTC registers
  is_ctp7->createUInt32("TTC_CONTROL", ctp7->getTTCControl(),   NULL, GEMUpdateType::HW32);
  is_ctp7->createUInt32("TTC_SPY",     ctp7->getTTCSpyBuffer(), NULL, GEMUpdateType::HW32);
}

void gem::hw::ctp7::CTP7Manager::dumpCTP7FIFO(xgi::Input* in, xgi::Output* out)
{
  dynamic_cast<CTP7ManagerWeb*>(p_gemWebInterface)->dumpCTP7FIFO(in, out);
}
