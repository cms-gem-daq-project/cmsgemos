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
            
  bag->addField("triggerSource", &triggerSource);
  bag->addField("sbitSource",    &sbitSource);
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

// This is the callback used for handling xdata:Event objects
void gem::hw::optohybrid::OptoHybridManager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("OptoHybridManager::actionPerformed() setDefaultValues" << 
          "Default configuration values have been loaded from xml profile");
    
    //how to handle passing in various values nested in a vector in a bag
    for (auto board = m_optohybridInfo.begin(); board != m_optohybridInfo.end(); ++board) {
      if (board->bag.present.value_)
        INFO("Found attribute:" << board->bag.toString());
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
        is_optohybrids[slot] = xdata::getInfoSpaceFactory()->get(hwCfgURN.toString());
      } else {
        INFO("initializeAction::infospace " << hwCfgURN.toString() << " does not exist, creating");
        is_optohybrids[slot] = xdata::getInfoSpaceFactory()->create(hwCfgURN.toString());
      }
      
      INFO("exporting config parameters into infospace");
      /* figure out how to make it work like this
         probably just have to define begin/end for OptoHybridInfo class and iterators
      for (auto monitorable = info.begin(); monitorable != info.end(); ++monitorable)
        if (is_optohybrids[slot]->hasItem(monitorable->getName()))
          is_optohybrids[slot]->fireItemAvailable(monitorable->getName(), monitorable->getSerializableValue());
      */
      if (!is_optohybrids[slot]->hasItem("ControlHubAddress"))
        is_optohybrids[slot]->fireItemAvailable("ControlHubAddress", &info.controlHubAddress);
      
      if (!is_optohybrids[slot]->hasItem("IPBusProtocol"))
        is_optohybrids[slot]->fireItemAvailable("IPBusProtocol",     &info.ipBusProtocol);
      
      if (!is_optohybrids[slot]->hasItem("DeviceIPAddress"))
        is_optohybrids[slot]->fireItemAvailable("DeviceIPAddress",   &info.deviceIPAddress);
      
      if (!is_optohybrids[slot]->hasItem("AddressTable"))
        is_optohybrids[slot]->fireItemAvailable("AddressTable",      &info.addressTable);
      
      if (!is_optohybrids[slot]->hasItem("ControlHubPort"))
        is_optohybrids[slot]->fireItemAvailable("ControlHubPort",    &info.controlHubPort);
      
      if (!is_optohybrids[slot]->hasItem("IPBusPort"))
        is_optohybrids[slot]->fireItemAvailable("IPBusPort",         &info.ipBusPort);

      INFO("InfoSpace found item: ControlHubAddress " << is_optohybrids[slot]->find("ControlHubAddress"));
      INFO("InfoSpace found item: IPBusProtocol "     << is_optohybrids[slot]->find("IPBusProtocol")    );
      INFO("InfoSpace found item: DeviceIPAddress "   << is_optohybrids[slot]->find("DeviceIPAddress")  );
      INFO("InfoSpace found item: AddressTable "      << is_optohybrids[slot]->find("AddressTable")     );
      INFO("InfoSpace found item: ControlHubPort "    << is_optohybrids[slot]->find("ControlHubPort")   );
      INFO("InfoSpace found item: IPBusPort "         << is_optohybrids[slot]->find("IPBusPort")        );
    
      is_optohybrids[slot]->fireItemValueRetrieve("ControlHubAddress");
      is_optohybrids[slot]->fireItemValueRetrieve("IPBusProtocol");
      is_optohybrids[slot]->fireItemValueRetrieve("DeviceIPAddress");
      is_optohybrids[slot]->fireItemValueRetrieve("AddressTable");
      is_optohybrids[slot]->fireItemValueRetrieve("ControlHubPort");
      is_optohybrids[slot]->fireItemValueRetrieve("IPBusPort");

      is_optohybrids[slot]->fireItemValueChanged("ControlHubAddress");
      is_optohybrids[slot]->fireItemValueChanged("IPBusProtocol");
      is_optohybrids[slot]->fireItemValueChanged("DeviceIPAddress");
      is_optohybrids[slot]->fireItemValueChanged("AddressTable");
      is_optohybrids[slot]->fireItemValueChanged("ControlHubPort");
      is_optohybrids[slot]->fireItemValueChanged("IPBusPort");

      INFO("initializeAction::info:" << info.toString());
      INFO("InfoSpace item value: ControlHubAddress " << info.controlHubAddress.toString());
      INFO("InfoSpace item value: IPBusProtocol "     << info.ipBusProtocol.toString()    );
      INFO("InfoSpace item value: DeviceIPAddress "   << info.deviceIPAddress.toString()  );
      INFO("InfoSpace item value: AddressTable "      << info.addressTable.toString()     );
      INFO("InfoSpace item value: ControlHubPort "    << info.controlHubPort.toString()   );
      INFO("InfoSpace item value: IPBusPort "         << info.ipBusPort.toString()        );
    
      try {
        INFO("obtaining pointer to HwOptoHybrid " << deviceName
             << " (slot " << slot+1 << ")"
             << " (link " << link   << ")");
        m_optohybrids[index] = optohybrid_shared_ptr(new gem::hw::optohybrid::HwOptoHybrid(deviceName,m_connectionFile.toString()));
        
        /*
        //still uses the above method behind the scenes
        //m_optohybrids[index] = optohybrid_shared_ptr(new gem::hw::optohybrid::HwOptoHybrid(info.crateID.value_,info.slotID.value_,info.linkID.value_));
        
        //maybe make this into a commonly used function? createHwURI(what though)
        //std::string tmpURI = toolbox::toString();
        std::stringstream tmpURI;
        if (info.controlHubAddress.toString().size() > 0) {
          INFO("Using control hub at address '" << info.controlHubAddress.toString()
               << ", port number "              << info.controlHubPort.toString() << "'.");
          tmpURI << "chtcp-"<< info.ipBusProtocol.toString() << "://"
                 << info.controlHubAddress.toString() << ":" << info.controlHubPort.toString()
                 << "?target=" << info.deviceIPAddress.toString() << ":" << info.ipBusPort.toString();
        } else {
          INFO("No control hub address specified -> continuing with a direct connection.");
          tmpURI << "ipbusudp-" << info.ipBusProtocol.toString() << "://"
                 << info.deviceIPAddress.toString() << ":" << info.ipBusPort.toString();
        }
        //std::string const uri = tmpURI.str();
        m_optohybrids[index] = optohybrid_shared_ptr(new gem::hw::optohybrid::HwOptoHybrid(deviceName, tmpURI.str(), "file://setup/etc/addresstables/"+info.addressTable.toString()));
        //INFO("connecting to device");
        //m_optohybrids[index]->connectDevice();
        */
        INFO("connected");
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
      //set the web view to be empty or grey
      //if (!info.present.value_) continue;
      //p_gemWebInterface->optohybridInSlot(slot);

      INFO("grabbing pointer to hardware device");
      optohybrid_shared_ptr optohybrid = m_optohybrids[index];

      if (optohybrid->isHwConnected()) {
        //return;
        INFO("OptoHybrid connected on link " << link << " to GLIB in slot " << (slot+1));
      } else {
        WARN("OptoHybrid connected on link " << link << " to GLIB in slot " << (slot+1) << " is not responding");
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
        /*
        INFO("setting vfat clock source to 0x" << std::hex << info.vfatClkSrc.value_ << std::dec);
        optohybrid->SetVFATClock(info.vfatClkSrc.value_,);
        INFO("setting cdce clock source to 0x" << std::hex << info.cdceClkSrc.value_ << std::dec);
        optohybrid->setSBitSource(info.cdceClkSrc.value_);
        */
        /*
        for (unsigned olink = 0; olink < HwGLIB::N_GTX; ++olink) {
        }
        */
        
        std::vector<uint32_t> connectedChipID0 = optohybrid->broadcastRead("ChipID0",0xffffffff);
        std::vector<uint32_t> connectedChipID1 = optohybrid->broadcastRead("ChipID1",0xffffffff);
        {
          auto id0 = connectedChipID0.begin();
          auto id1 = connectedChipID0.begin();
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
        WARN("OptoHybrid connected on link " << link << " to GLIB in slot " << (slot+1) << " is not responding");
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

      INFO("revoking hwCfgInfoSpace items for board connected on link " << link << " to GLIB in slot " << (slot+1));
      toolbox::net::URN hwCfgURN("urn:gem:hw:"+toolbox::toString("gem.shelf%02d.glib%02d.optohybrid%02d",
                                                                 info.crateID.value_,
                                                                 info.slotID.value_,
                                                                 info.linkID.value_));
      if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
        INFO("resetAction::infospace " << hwCfgURN.toString() << " already exists, getting");
        is_optohybrids[slot] = xdata::getInfoSpaceFactory()->get(hwCfgURN.toString());
      } else {
        INFO("resetAction::infospace " << hwCfgURN.toString() << " does not exist, no further action");
        continue;
      }
      
      INFO("revoking config parameters into infospace");
      if (is_optohybrids[slot]->hasItem("ControlHubAddress"))
        is_optohybrids[slot]->fireItemRevoked("ControlHubAddress");
      
      if (is_optohybrids[slot]->hasItem("IPBusProtocol"))
        is_optohybrids[slot]->fireItemRevoked("IPBusProtocol");
      
      if (is_optohybrids[slot]->hasItem("DeviceIPAddress"))
        is_optohybrids[slot]->fireItemRevoked("DeviceIPAddress");
      
      if (is_optohybrids[slot]->hasItem("AddressTable"))
        is_optohybrids[slot]->fireItemRevoked("AddressTable");
      
      if (is_optohybrids[slot]->hasItem("ControlHubPort"))
        is_optohybrids[slot]->fireItemRevoked("ControlHubPort");
      
      if (is_optohybrids[slot]->hasItem("IPBusPort"))
        is_optohybrids[slot]->fireItemRevoked("IPBusPort");
    }
  }
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
