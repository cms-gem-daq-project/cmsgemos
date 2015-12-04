/**
 * class: OptoHybridManager
 * description: Manager application for OptoHybrid cards
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date: 
 */

#include "gem/hw/optohybrid/OptoHybridManagerWeb.h"
#include "gem/hw/optohybrid/OptoHybridManager.h"

#include "gem/hw/optohybrid/HwOptoHybrid.h"
//#include "gem/hw/optohybrid/exception/Exception.h"

XDAQ_INSTANTIATOR_IMPL(gem::hw::optohybrid::OptoHybridManager);

gem::hw::optohybrid::OptoHybridManager::OptoHybridInfo::OptoHybridInfo() {
  present = false;
  crateID = -1;
  slotID  = -1;
  linkID  = -1;

  controlHubAddress = "";
  deviceIPAddress     = "";
  ipBusProtocol       = "";
  addressTable        = "";
  controlHubPort      = 0;
  ipBusPort           = 0;
  
  triggerSource = 0;
  sbitSource    = 0;
  refClkSrc     = 0;
  vfatClkSrc    = 0;
  cdceClkSrc    = 0;
}

void gem::hw::optohybrid::OptoHybridManager::OptoHybridInfo::registerFields(xdata::Bag<gem::hw::optohybrid::OptoHybridManager::OptoHybridInfo>* bag) {
  bag->addField("crateID",       &crateID);
  bag->addField("slot",          &slotID);
  bag->addField("link",          &linkID);
  bag->addField("present",       &present);

  bag->addField("ControlHubAddress", &controlHubAddress);
  bag->addField("DeviceIPAddress",   &deviceIPAddress);
  bag->addField("IPBusProtocol",     &ipBusProtocol);
  bag->addField("AddressTable",      &addressTable);
  bag->addField("ControlHubPort",    &controlHubPort);
  bag->addField("IPBusPort",         &ipBusPort);

  bag->addField("VFATBroadcastList", &vfatBroadcastList);
  bag->addField("VFATBroadcastMask", &vfatBroadcastMask);
            
  bag->addField("triggerSource", &triggerSource);
  bag->addField("sbitSource",    &sbitSource);
  bag->addField("refClkSrc",     &refClkSrc);
  bag->addField("vfatClkSrc",    &vfatClkSrc);
  bag->addField("cdceClkSrc",    &cdceClkSrc);
}

gem::hw::optohybrid::OptoHybridManager::OptoHybridManager(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub)
{
  m_optohybridInfo.setSize(MAX_OPTOHYBRIDS_PER_AMC*MAX_AMCS_PER_CRATE);

  p_appInfoSpace->fireItemAvailable("AllOptoHybridsInfo", &m_optohybridInfo);
  p_appInfoSpace->fireItemAvailable("ConnectionFile",     &m_connectionFile);

  p_appInfoSpace->addItemRetrieveListener("AllOptoHybridsInfo", this);
  p_appInfoSpace->addItemRetrieveListener("ConnectionFile",     this);
  p_appInfoSpace->addItemChangedListener( "AllOptoHybridsInfo", this);
  p_appInfoSpace->addItemChangedListener( "ConnectionFile",     this);

  //initialize the OptoHybrid application objects
  DEBUG("Connecting to the OptoHybridManagerWeb interface");
  p_gemWebInterface = new gem::hw::optohybrid::OptoHybridManagerWeb(this);
  //p_gemMonitor      = new gem::hw::optohybrid::OptoHybridHwMonitor(this);
  DEBUG("done");

  //set up the info hwCfgInfoSpace 
  init();

  //getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemhardware/images/optohybrid/OptoHybridManager.png");
}

gem::hw::optohybrid::OptoHybridManager::~OptoHybridManager() {
  //memory management, maybe not necessary here?
}

uint32_t gem::hw::optohybrid::OptoHybridManager::parseVFATMaskList(std::string const& enableList)
{
  //nothing masked, return the negation of the mask that includes the enable list
  uint32_t broadcastMask = 0x00000000;
  //everything masked, return the mask that doesn't include the enable list
  //uint32_t broadcastMask = 0xffffffff;
  std::vector<std::string> slots;

  boost::split(slots, enableList, boost::is_any_of(", "), boost::token_compress_on);  
  DEBUG("VFAT broadcast enable list is " << enableList);
  for (auto slot = slots.begin(); slot != slots.end(); ++slot) {
    DEBUG("slot is " << *slot);
    if (slot->find('-') != std::string::npos) { // found a possible range
      DEBUG("found a hyphen in " << *slot);
      std::vector<std::string> range;
      boost::split(range, *slot, boost::is_any_of("-"), boost::token_compress_on);
      if (range.size() > 2) {
        WARN("parseVFATMaskList::Found poorly formatted range " << *slot);
        continue;
      }
      if (isValidSlotNumber(range.at(0)) && isValidSlotNumber(range.at(1))) {
        std::stringstream ss0(range.at(0));
        std::stringstream ss1(range.at(1));
        int min, max;
        ss0 >> min;
        ss1 >> max;
        
        if (min == max) {
          WARN("parseVFATMaskList::Found poorly formatted range " << *slot);
          continue;
        }
        if (min > max) { // elements in the wrong order
          WARN("parseVFATMaskList::Found poorly formatted range " << *slot);
          continue;
        }
        
        for (int islot = min; islot <= max; ++islot) {
          broadcastMask |= (0x1 << (islot));
          //broadcastMask ^= (0x1 << (islot));
        } //  end loop over range of list
      } // end check on valid values
    } else { //not a range
      DEBUG("found no hyphen in " << *slot);
      if (slot->length() > 2) {
        WARN("parseVFATMaskList::Found longer value than expected (0-23) " << *slot);
        continue;
      }
      
      if (!isValidSlotNumber(*slot)) {
        WARN("parseVFATMaskList::Found invalid value " << *slot);
        continue;
      }
      std::stringstream ss(*slot);
      int slotNum = -1;
      ss >> slotNum;
      broadcastMask |= (0x1 << (slotNum));
      //broadcastMask ^= (0x1 << (slotNum));
    } //done processing single values
  } //done looping over extracted values
  DEBUG("parseVFATMaskList::Parsed enabled list 0x" << std::hex << broadcastMask << std::dec
        //<< " bits set " << std::bitset<32>(broadcastMask).count()
        << " inverted: 0x" << std::hex << ~broadcastMask << std::dec
        //<< " bits set " << std::bitset<32>(~broadcastMask).count()
        );
  return ~broadcastMask;
}

bool gem::hw::optohybrid::OptoHybridManager::isValidSlotNumber(std::string const& s)
{
  try {
    int i_val;
    i_val = std::stoi(s);
    if (!(i_val >= 0 && i_val < 24)) {
      ERROR("isValidSlotNumber::Found value outside expected (0-23) " << i_val);
      return false;
    }
  } catch (std::invalid_argument const& err) {
    ERROR("isValidSlotNumber::Unable to convert to integer type " << s << std::endl << err.what());
    return false;
  } catch (std::out_of_range const& err) {
    ERROR("isValidSlotNumber::Unable to convert to integer type " << s << std::endl << err.what());
    return false;
  }
  
  return true; //if you get here, should be possible to parse as an integer in the range [1,12]
}

// This is the callback used for handling xdata:Event objects
void gem::hw::optohybrid::OptoHybridManager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("OptoHybridManager::actionPerformed() setDefaultValues" << 
          "Default configuration values have been loaded from xml profile");
    
    //how to handle passing in various values nested in a vector in a bag
    for (auto board = m_optohybridInfo.begin(); board != m_optohybridInfo.end(); ++board) {
      if (board->bag.present.value_) {
        INFO("Found attribute:" << board->bag.toString());
        v_vfatBroadcastMask.push_back(parseVFATMaskList(board->bag.vfatBroadcastList.toString()));
        INFO("Parsed AMCEnableList vfatBroadcastMask = " << board->bag.vfatBroadcastList.toString()
             << " to broadcastMask 0x" << std::hex << v_vfatBroadcastMask.back() << std::dec);
        board->bag.vfatBroadcastMask = v_vfatBroadcastMask.back();
      }
    }
    //p_gemMonitor->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::optohybrid::OptoHybridManager::init()
{
  /*
  INFO("gem::hw::optohybrid::OptoHybridManager::init begin");
  for (int slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    INFO("OptoHybridManager::looping over slots(" << (slot+1) << ") and finding expected cards");
    for (int link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      INFO("OptoHybridManager::looping over links(" << link << ") and finding expected cards");
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      OptoHybridInfo& info = m_optohybridInfo[index].bag;
      
      if (!info.present)
        continue;
      // set up the info space here rather than in initialize (where it can then get unset in reset?
      // should a value be set up for all of them by default?

      INFO("creating hwCfgInfoSpace items for board connected on link " << link << " to GLIB in slot " << (slot+1));
      toolbox::net::URN hwCfgURN("urn:gem:hw:"+toolbox::toString("gem.shelf%02d.glib%02d.optohybrid%02d",
                                                                 info.crateID.value_,
                                                                 info.slotID.value_,
                                                                 info.linkID.value_));
      if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
        INFO("init::infospace " << hwCfgURN.toString() << " already exists, getting");
        is_optohybrids[slot] = xdata::getInfoSpaceFactory()->get(hwCfgURN.toString());
      } else {
        INFO("init::infospace " << hwCfgURN.toString() << " does not exist, creating");
        is_optohybrids[slot] = xdata::getInfoSpaceFactory()->create(hwCfgURN.toString());
      }
      
      INFO("exporting config parameters into infospace");
      is_optohybrids[slot]->fireItemAvailable("ControlHubAddress", &info.controlHubAddress);
      is_optohybrids[slot]->fireItemAvailable("IPBusProtocol",     &info.ipBusProtocol);
      is_optohybrids[slot]->fireItemAvailable("DeviceIPAddress",   &info.deviceIPAddress);
      is_optohybrids[slot]->fireItemAvailable("AddressTable",      &info.addressTable);
      is_optohybrids[slot]->fireItemAvailable("ControlHubPort",    &info.controlHubPort);
      is_optohybrids[slot]->fireItemAvailable("IPBusPort",         &info.ipBusPort);
      
      is_optohybrids[slot]->fireItemValueChanged("ControlHubAddress");
      is_optohybrids[slot]->fireItemValueChanged("IPBusProtocol");
      is_optohybrids[slot]->fireItemValueChanged("DeviceIPAddress");
      is_optohybrids[slot]->fireItemValueChanged("AddressTable");
      is_optohybrids[slot]->fireItemValueChanged("ControlHubPort");
      is_optohybrids[slot]->fireItemValueChanged("IPBusPort");
      
      INFO("InfoSpace found item: ControlHubAddress " << is_optohybrids[slot]->find("ControlHubAddress"));
      INFO("InfoSpace found item: IPBusProtocol "     << is_optohybrids[slot]->find("IPBusProtocol")    );
      INFO("InfoSpace found item: DeviceIPAddress "   << is_optohybrids[slot]->find("DeviceIPAddress")  );
      INFO("InfoSpace found item: AddressTable "      << is_optohybrids[slot]->find("AddressTable")     );
      INFO("InfoSpace found item: ControlHubPort "    << is_optohybrids[slot]->find("ControlHubPort")   );
      INFO("InfoSpace found item: IPBusPort "         << is_optohybrids[slot]->find("IPBusPort")        );
      
      INFO("info:" << info.toString());
    
      INFO("InfoSpace item value: ControlHubAddress " << info.controlHubAddress.toString());
      INFO("InfoSpace item value: IPBusProtocol "     << info.ipBusProtocol.toString()    );
      INFO("InfoSpace item value: DeviceIPAddress "   << info.deviceIPAddress.toString()  );
      INFO("InfoSpace item value: AddressTable "      << info.addressTable.toString()     );
      INFO("InfoSpace item value: ControlHubPort "    << info.controlHubPort.toString()   );
      INFO("InfoSpace item value: IPBusPort "         << info.ipBusPort.toString()        );
    }
  }
  */
}

//state transitions
void gem::hw::optohybrid::OptoHybridManager::initializeAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  INFO("gem::hw::optohybrid::OptoHybridManager::initializeAction begin");
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    usleep(1000);
    INFO("OptoHybridManager::looping over slots(" << (slot+1) << ") and finding expected cards");
    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      usleep(1000);
      INFO("OptoHybridManager::looping over links(" << link << ") and finding expected cards");
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      if (!info.present)
        continue;

      INFO("line 118: info is: " << info.toString());
      INFO("creating pointer to board connected on link " << link << " to GLIB in slot " << (slot+1));
      std::string deviceName = toolbox::toString("gem.shelf%02d.glib%02d.optohybrid%02d",
                                                 info.crateID.value_,
                                                 info.slotID.value_,
                                                 info.linkID.value_);
      toolbox::net::URN hwCfgURN("urn:gem:hw:"+deviceName);

      if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
        INFO("initializeAction::infospace " << hwCfgURN.toString() << " already exists, getting");
        is_optohybrids[index] = xdata::getInfoSpaceFactory()->get(hwCfgURN.toString());

        /*
        createOptoHybridInfoSpaceItems(is_optohybrids[index], m_optohybrids[index]);
        
        m_optohybridMonitors[index] = std::shared_ptr<OptoHybridMonitor>(new OptoHybridMonitor(m_optohybrids[index], this, slot+1));
        m_optohybridMonitors[index]->addInfoSpace("HWMonitoring", is_optohybrids[index]);
        m_optohybridMonitors[index]->setupHwMonitoring();
        m_optohybridMonitors[index]->startMonitoring();
        */
      } else {
        INFO("initializeAction::infospace " << hwCfgURN.toString() << " does not exist, creating");
        is_optohybrids[index] = xdata::getInfoSpaceFactory()->create(hwCfgURN.toString());
      }
      
      INFO("exporting config parameters into infospace");
      if (!is_optohybrids[index]->hasItem("ControlHubAddress"))
        is_optohybrids[index]->fireItemAvailable("ControlHubAddress", &info.controlHubAddress);
      
      if (!is_optohybrids[index]->hasItem("IPBusProtocol"))
        is_optohybrids[index]->fireItemAvailable("IPBusProtocol",     &info.ipBusProtocol);
      
      if (!is_optohybrids[index]->hasItem("DeviceIPAddress"))
        is_optohybrids[index]->fireItemAvailable("DeviceIPAddress",   &info.deviceIPAddress);
      
      if (!is_optohybrids[index]->hasItem("AddressTable"))
        is_optohybrids[index]->fireItemAvailable("AddressTable",      &info.addressTable);
      
      if (!is_optohybrids[index]->hasItem("ControlHubPort"))
        is_optohybrids[index]->fireItemAvailable("ControlHubPort",    &info.controlHubPort);
      
      if (!is_optohybrids[index]->hasItem("IPBusPort"))
        is_optohybrids[index]->fireItemAvailable("IPBusPort",         &info.ipBusPort);

      INFO("InfoSpace found item: ControlHubAddress " << is_optohybrids[index]->find("ControlHubAddress"));
      INFO("InfoSpace found item: IPBusProtocol "     << is_optohybrids[index]->find("IPBusProtocol")    );
      INFO("InfoSpace found item: DeviceIPAddress "   << is_optohybrids[index]->find("DeviceIPAddress")  );
      INFO("InfoSpace found item: AddressTable "      << is_optohybrids[index]->find("AddressTable")     );
      INFO("InfoSpace found item: ControlHubPort "    << is_optohybrids[index]->find("ControlHubPort")   );
      INFO("InfoSpace found item: IPBusPort "         << is_optohybrids[index]->find("IPBusPort")        );
    
      is_optohybrids[index]->fireItemValueRetrieve("ControlHubAddress");
      is_optohybrids[index]->fireItemValueRetrieve("IPBusProtocol");
      is_optohybrids[index]->fireItemValueRetrieve("DeviceIPAddress");
      is_optohybrids[index]->fireItemValueRetrieve("AddressTable");
      is_optohybrids[index]->fireItemValueRetrieve("ControlHubPort");
      is_optohybrids[index]->fireItemValueRetrieve("IPBusPort");

      is_optohybrids[index]->fireItemValueChanged("ControlHubAddress");
      is_optohybrids[index]->fireItemValueChanged("IPBusProtocol");
      is_optohybrids[index]->fireItemValueChanged("DeviceIPAddress");
      is_optohybrids[index]->fireItemValueChanged("AddressTable");
      is_optohybrids[index]->fireItemValueChanged("ControlHubPort");
      is_optohybrids[index]->fireItemValueChanged("IPBusPort");

      INFO("initializeAction::info:" << info.toString());
      INFO("InfoSpace item value: ControlHubAddress " << info.controlHubAddress.toString());
      INFO("InfoSpace item value: IPBusProtocol "     << info.ipBusProtocol.toString()    );
      INFO("InfoSpace item value: DeviceIPAddress "   << info.deviceIPAddress.toString()  );
      INFO("InfoSpace item value: AddressTable "      << info.addressTable.toString()     );
      INFO("InfoSpace item value: ControlHubPort "    << info.controlHubPort.toString()   );
      INFO("InfoSpace item value: IPBusPort "         << info.ipBusPort.toString()        );
    
      try {
        DEBUG("obtaining pointer to HwOptoHybrid " << deviceName
              << " (slot " << slot+1 << ")"
              << " (link " << link   << ")");
        m_optohybrids[index] = optohybrid_shared_ptr(new gem::hw::optohybrid::HwOptoHybrid(deviceName,m_connectionFile.toString()));
      } catch (gem::hw::optohybrid::exception::Exception const& ex) {
        ERROR("caught exception " << ex.what());
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, "initializeAction failed");
      } catch (toolbox::net::exception::MalformedURN const& ex) {
        ERROR("caught exception " << ex.what());
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, "initializeAction failed");
      } catch (std::exception const& ex) {
        ERROR("caught exception " << ex.what());
        XCEPT_RAISE(gem::hw::optohybrid::exception::Exception, "initializeAction failed");
      }
      DEBUG("connected");
      //set the web view to be empty or grey
      //if (!info.present.value_) continue;
      //p_gemWebInterface->optohybridInSlot(slot);

      DEBUG("grabbing pointer to hardware device");
      //optohybrid_shared_ptr optohybrid = m_optohybrids[index];

      if (m_optohybrids[index]->isHwConnected()) {
        INFO("OptoHybrid connected on link " << link << " to GLIB in slot " << (slot+1));
      } else {
        ERROR("OptoHybrid connected on link " << link << " to GLIB in slot " << (slot+1) << " is not responding");
        fireEvent("Fail");
        //maybe raise exception so as to not continue with other cards? let's just return for the moment
        return;
      }
    }
  }  
  INFO("gem::hw::optohybrid::OptoHybridManager::initializeAction end");
}

void gem::hw::optohybrid::OptoHybridManager::configureAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  INFO("gem::hw::optohybrid::OptoHybridManager::configureAction");
  //will the manager operate for all connected optohybrids, or only those connected to certain GLIBs?
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    usleep(1000);
    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      usleep(1000);
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      OptoHybridInfo& info = m_optohybridInfo[index].bag;

      DEBUG("configureAction::info is: " << info.toString());
      if (!info.present)
        continue;
      
      DEBUG("configureAction::grabbing pointer to hardware device");
      optohybrid_shared_ptr optohybrid = m_optohybrids[index];
      
      if (optohybrid->isHwConnected()) {
        INFO("configureAction::setting trigger source to 0x" 
             << std::hex << info.triggerSource.value_ << std::dec);
        optohybrid->setTrigSource(info.triggerSource.value_);
        INFO("configureAction::setting sbit source to 0x"
             << std::hex << info.sbitSource.value_ << std::dec);
        optohybrid->setSBitSource(info.sbitSource.value_);
        INFO("setting reference clock source to 0x"
             << std::hex << info.refClkSrc.value_ << std::dec);
        optohybrid->setReferenceClock(info.refClkSrc.value_);
        /*
        INFO("setting vfat clock source to 0x" << std::hex << info.vfatClkSrc.value_ << std::dec);
        optohybrid->setVFATClock(info.vfatClkSrc.value_,);
        INFO("setting cdce clock source to 0x" << std::hex << info.cdceClkSrc.value_ << std::dec);
        optohybrid->setSBitSource(info.cdceClkSrc.value_);
        */
        /*
        for (unsigned olink = 0; olink < HwGLIB::N_GTX; ++olink) {
        }
        */
        
        std::vector<uint32_t> connectedChipID0 = optohybrid->broadcastRead("ChipID0",info.vfatBroadcastMask);
        std::vector<uint32_t> connectedChipID1 = optohybrid->broadcastRead("ChipID1",info.vfatBroadcastMask);
        {
          auto id0 = connectedChipID0.begin();
          auto id1 = connectedChipID1.begin();
          INFO(std::setw(12) << "ChipID 1" << std::setw(12) << "ChipID 0");
          for (; id0 != connectedChipID0.end(); ++id0, ++id1) {
            INFO(std::setw(10) << "0x" << std::hex << *id1 << std::dec << 
                 std::setw(10) << "0x" << std::hex << *id0 << std::dec);
          }
        }
        //what else is required for configuring the OptoHybrid?
        //need to reset optical links?
        //reset counters?
      } else {
        ERROR("OptoHybrid connected on link " << link << " to GLIB in slot " << (slot+1) << " is not responding");
        fireEvent("Fail");
        //maybe raise exception so as to not continue with other cards?
      }
    }
  }
  
  INFO("gem::hw::optohybrid::OptoHybridManager::configureAction end");
}

void gem::hw::optohybrid::OptoHybridManager::startAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  usleep(1000);
}

void gem::hw::optohybrid::OptoHybridManager::pauseAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  usleep(1000);
}

void gem::hw::optohybrid::OptoHybridManager::resumeAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  usleep(1000);
}

void gem::hw::optohybrid::OptoHybridManager::stopAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  usleep(1000);
}

void gem::hw::optohybrid::OptoHybridManager::haltAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  usleep(1000);
}

void gem::hw::optohybrid::OptoHybridManager::resetAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
  //unregister listeners and items in info spaces
  INFO("gem::hw::optohybrid::OptoHybridManager::resetAction begin");
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    usleep(1000);
    INFO("OptoHybridManager::looping over slots(" << (slot+1) << ") and finding expected cards");
    for (unsigned link = 0; link < MAX_OPTOHYBRIDS_PER_AMC; ++link) {
      usleep(1000);
      INFO("OptoHybridManager::looping over links(" << link << ") and finding expected cards");
      unsigned int index = (slot*MAX_OPTOHYBRIDS_PER_AMC)+link;
      OptoHybridInfo& info = m_optohybridInfo[index].bag;
      
      if (!info.present)
        continue;
      // set up the info space here rather than in initialize (where it can then get unset in reset?
      // should a value be set up for all of them by default?

      DEBUG("revoking hwCfgInfoSpace items for board connected on link " << link << " to GLIB in slot " << (slot+1));
      toolbox::net::URN hwCfgURN("urn:gem:hw:"+toolbox::toString("gem.shelf%02d.glib%02d.optohybrid%02d",
                                                                 info.crateID.value_,
                                                                 info.slotID.value_,
                                                                 info.linkID.value_));
      if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
        DEBUG("resetAction::infospace " << hwCfgURN.toString() << " already exists, getting");
        is_optohybrids[index] = xdata::getInfoSpaceFactory()->get(hwCfgURN.toString());
        
        DEBUG("revoking config parameters into infospace");
        // reset the hw infospace toolbox
        // is_optohybrids[index]->reset();
        if (is_optohybrids[index]->hasItem("ControlHubAddress"))
          is_optohybrids[index]->fireItemRevoked("ControlHubAddress");
        
        if (is_optohybrids[index]->hasItem("IPBusProtocol"))
          is_optohybrids[index]->fireItemRevoked("IPBusProtocol");
        
        if (is_optohybrids[index]->hasItem("DeviceIPAddress"))
          is_optohybrids[index]->fireItemRevoked("DeviceIPAddress");
        
        if (is_optohybrids[index]->hasItem("AddressTable"))
          is_optohybrids[index]->fireItemRevoked("AddressTable");
        
        if (is_optohybrids[index]->hasItem("ControlHubPort"))
          is_optohybrids[index]->fireItemRevoked("ControlHubPort");
        
        if (is_optohybrids[index]->hasItem("IPBusPort"))
          is_optohybrids[index]->fireItemRevoked("IPBusPort");
      } else {
        INFO("resetAction::infospace " << hwCfgURN.toString() << " does not exist, no further action");
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
