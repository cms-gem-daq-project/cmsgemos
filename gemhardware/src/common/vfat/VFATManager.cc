/**
 * class: VFATManager
 * description: Manager application for Vfat cards
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date: 
 */

#include "gem/hw/vfat/VFATManagerWeb.h"
#include "gem/hw/vfat/VFATManager.h"

#include "gem/hw/vfat/HwVFAT2.h"
//#include "gem/hw/vfat/exception/Exception.h"

XDAQ_INSTANTIATOR_IMPL(gem::hw::vfat::VFATManager);

gem::hw::vfat::VFATManager::VFATInfo::VFATInfo() {
  present = false;
  slotID  = -1;
  controlHubAddress = "";
  deviceIPAddress     = "";
  ipBusProtocol       = "";
  addressTable        = "";
  controlHubPort      = 0;
  ipBusPort           = 0;
  
  triggerSource = 0;
}

void gem::hw::vfat::VFATManager::VFATInfo::registerFields(xdata::Bag<gem::hw::vfat::VFATManager::VFATInfo>* bag) {
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
}

gem::hw::vfat::VFATManager::VFATManager(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub)
{
  m_vfatInfo.setSize(MAX_VFATS_PER_AMC*MAX_AMCS_PER_CRATE);

  p_appInfoSpace->fireItemAvailable("AllVFATsInfo", &m_vfatInfo);
  p_appInfoSpace->fireItemAvailable("ConnectionFile",     &m_connectionFile);

  p_appInfoSpace->addItemRetrieveListener("AllVFATsInfo", this);
  p_appInfoSpace->addItemRetrieveListener("ConnectionFile",     this);
  p_appInfoSpace->addItemChangedListener( "AllVFATsInfo", this);
  p_appInfoSpace->addItemChangedListener( "ConnectionFile",     this);

  //initialize the VFAT application objects
  DEBUG("Connecting to the VFATManagerWeb interface");
  p_gemWebInterface = new gem::hw::vfat::VFATManagerWeb(this);
  //p_gemMonitor      = new gem::hw::vfat::VFATHwMonitor(this);
  DEBUG("done");

  //set up the info hwCfgInfoSpace 
  init();

  //getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemhardware/images/vfat/VFATManager.png");
}

gem::hw::vfat::VFATManager::~VFATManager() {
  //memory management, maybe not necessary here?
}

// This is the callback used for handling xdata:Event objects
void gem::hw::vfat::VFATManager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("VFATManager::actionPerformed() setDefaultValues" << 
          "Default configuration values have been loaded from xml profile");
    
    //how to handle passing in various values nested in a vector in a bag
    for (auto board = m_vfatInfo.begin(); board != m_vfatInfo.end(); ++board) {
      if (board->bag.present.value_)
        INFO("Found attribute:" << board->bag.toString());
    }
    //p_gemMonitor->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::vfat::VFATManager::init()
{
  /*
  INFO("gem::hw::vfat::VFATManager::init begin");
  for (int slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    INFO("VFATManager::looping over slots(" << (slot+1) << ") and finding expected cards");
    for (int link = 0; link < MAX_VFATS_PER_AMC; ++link) {
      INFO("VFATManager::looping over links(" << link << ") and finding expected cards");
      unsigned int index = (slot*MAX_VFATS_PER_AMC)+link;
      VFATInfo& info = m_vfatInfo[index].bag;
      
      if (!info.present)
        continue;
      // set up the info space here rather than in initialize (where it can then get unset in reset?
      // should a value be set up for all of them by default?

      INFO("creating hwCfgInfoSpace items for board connected on link " << link << " to GLIB in slot " << (slot+1));
      toolbox::net::URN hwCfgURN("urn:gem:hw:"+toolbox::toString("gem.shelf%02d.glib%02d.vfat%02d",
                                                                 info.crateID.value_,
                                                                 info.slotID.value_,
                                                                 info.linkID.value_));
      if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
        INFO("init::infospace " << hwCfgURN.toString() << " already exists, getting");
        is_vfats[slot] = xdata::getInfoSpaceFactory()->get(hwCfgURN.toString());
      } else {
        INFO("init::infospace " << hwCfgURN.toString() << " does not exist, creating");
        is_vfats[slot] = xdata::getInfoSpaceFactory()->create(hwCfgURN.toString());
      }
      
      INFO("exporting config parameters into infospace");
      is_vfats[slot]->fireItemAvailable("ControlHubAddress", &info.controlHubAddress);
      is_vfats[slot]->fireItemAvailable("IPBusProtocol",     &info.ipBusProtocol);
      is_vfats[slot]->fireItemAvailable("DeviceIPAddress",   &info.deviceIPAddress);
      is_vfats[slot]->fireItemAvailable("AddressTable",      &info.addressTable);
      is_vfats[slot]->fireItemAvailable("ControlHubPort",    &info.controlHubPort);
      is_vfats[slot]->fireItemAvailable("IPBusPort",         &info.ipBusPort);
      
      is_vfats[slot]->fireItemValueChanged("ControlHubAddress");
      is_vfats[slot]->fireItemValueChanged("IPBusProtocol");
      is_vfats[slot]->fireItemValueChanged("DeviceIPAddress");
      is_vfats[slot]->fireItemValueChanged("AddressTable");
      is_vfats[slot]->fireItemValueChanged("ControlHubPort");
      is_vfats[slot]->fireItemValueChanged("IPBusPort");
      
      INFO("InfoSpace found item: ControlHubAddress " << is_vfats[slot]->find("ControlHubAddress"));
      INFO("InfoSpace found item: IPBusProtocol "     << is_vfats[slot]->find("IPBusProtocol")    );
      INFO("InfoSpace found item: DeviceIPAddress "   << is_vfats[slot]->find("DeviceIPAddress")  );
      INFO("InfoSpace found item: AddressTable "      << is_vfats[slot]->find("AddressTable")     );
      INFO("InfoSpace found item: ControlHubPort "    << is_vfats[slot]->find("ControlHubPort")   );
      INFO("InfoSpace found item: IPBusPort "         << is_vfats[slot]->find("IPBusPort")        );
      
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
void gem::hw::vfat::VFATManager::initializeAction()
  throw (gem::hw::vfat::exception::Exception)
{
  INFO("gem::hw::vfat::VFATManager::initializeAction begin");
  for (int slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    usleep(1000);
    INFO("VFATManager::looping over slots(" << (slot+1) << ") and finding expected cards");
    for (int link = 0; link < MAX_VFATS_PER_AMC; ++link) {
      usleep(1000);
      INFO("VFATManager::looping over links(" << link << ") and finding expected cards");
      unsigned int index = (slot*MAX_VFATS_PER_AMC)+link;
      VFATInfo& info = m_vfatInfo[index].bag;

      if (!info.present)
        continue;

      INFO("line 118: info is: " << info.toString());
      INFO("creating pointer to board connected on link " << link << " to GLIB in slot " << (slot+1));
      std::string deviceName = toolbox::toString("gem.shelf%02d.glib%02d.vfat%02d",
                                                 info.crateID.value_,
                                                 info.slotID.value_,
                                                 info.linkID.value_);
      toolbox::net::URN hwCfgURN("urn:gem:hw:"+deviceName);

      if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
        INFO("initializeAction::infospace " << hwCfgURN.toString() << " already exists, getting");
        is_vfats[slot] = xdata::getInfoSpaceFactory()->get(hwCfgURN.toString());
      } else {
        INFO("initializeAction::infospace " << hwCfgURN.toString() << " does not exist, creating");
        is_vfats[slot] = xdata::getInfoSpaceFactory()->create(hwCfgURN.toString());
      }
      
      INFO("exporting config parameters into infospace");
      /* figure out how to make it work like this
         probably just have to define begin/end for VFATInfo class and iterators
      for (auto monitorable = info.begin(); monitorable != info.end(); ++monitorable)
        if (is_vfats[slot]->hasItem(monitorable->getName()))
          is_vfats[slot]->fireItemAvailable(monitorable->getName(), monitorable->getSerializableValue());
      */
      if (!is_vfats[slot]->hasItem("ControlHubAddress"))
        is_vfats[slot]->fireItemAvailable("ControlHubAddress", &info.controlHubAddress);
      
      if (!is_vfats[slot]->hasItem("IPBusProtocol"))
        is_vfats[slot]->fireItemAvailable("IPBusProtocol",     &info.ipBusProtocol);
      
      if (!is_vfats[slot]->hasItem("DeviceIPAddress"))
        is_vfats[slot]->fireItemAvailable("DeviceIPAddress",   &info.deviceIPAddress);
      
      if (!is_vfats[slot]->hasItem("AddressTable"))
        is_vfats[slot]->fireItemAvailable("AddressTable",      &info.addressTable);
      
      if (!is_vfats[slot]->hasItem("ControlHubPort"))
        is_vfats[slot]->fireItemAvailable("ControlHubPort",    &info.controlHubPort);
      
      if (!is_vfats[slot]->hasItem("IPBusPort"))
        is_vfats[slot]->fireItemAvailable("IPBusPort",         &info.ipBusPort);

      INFO("InfoSpace found item: ControlHubAddress " << is_vfats[slot]->find("ControlHubAddress"));
      INFO("InfoSpace found item: IPBusProtocol "     << is_vfats[slot]->find("IPBusProtocol")    );
      INFO("InfoSpace found item: DeviceIPAddress "   << is_vfats[slot]->find("DeviceIPAddress")  );
      INFO("InfoSpace found item: AddressTable "      << is_vfats[slot]->find("AddressTable")     );
      INFO("InfoSpace found item: ControlHubPort "    << is_vfats[slot]->find("ControlHubPort")   );
      INFO("InfoSpace found item: IPBusPort "         << is_vfats[slot]->find("IPBusPort")        );
    
      is_vfats[slot]->fireItemValueRetrieve("ControlHubAddress");
      is_vfats[slot]->fireItemValueRetrieve("IPBusProtocol");
      is_vfats[slot]->fireItemValueRetrieve("DeviceIPAddress");
      is_vfats[slot]->fireItemValueRetrieve("AddressTable");
      is_vfats[slot]->fireItemValueRetrieve("ControlHubPort");
      is_vfats[slot]->fireItemValueRetrieve("IPBusPort");

      is_vfats[slot]->fireItemValueChanged("ControlHubAddress");
      is_vfats[slot]->fireItemValueChanged("IPBusProtocol");
      is_vfats[slot]->fireItemValueChanged("DeviceIPAddress");
      is_vfats[slot]->fireItemValueChanged("AddressTable");
      is_vfats[slot]->fireItemValueChanged("ControlHubPort");
      is_vfats[slot]->fireItemValueChanged("IPBusPort");

      INFO("initializeAction::info:" << info.toString());
      INFO("InfoSpace item value: ControlHubAddress " << info.controlHubAddress.toString());
      INFO("InfoSpace item value: IPBusProtocol "     << info.ipBusProtocol.toString()    );
      INFO("InfoSpace item value: DeviceIPAddress "   << info.deviceIPAddress.toString()  );
      INFO("InfoSpace item value: AddressTable "      << info.addressTable.toString()     );
      INFO("InfoSpace item value: ControlHubPort "    << info.controlHubPort.toString()   );
      INFO("InfoSpace item value: IPBusPort "         << info.ipBusPort.toString()        );
    
      try {
        INFO("obtaining pointer to HwVFAT2 " << deviceName
             << " (slot " << slot+1 << ")"
             << " (link " << link   << ")");
        m_vfats[index] = vfat_shared_ptr(new gem::hw::vfat::HwVFAT2(deviceName,m_connectionFile.toString()));
        
        /*
        //still uses the above method behind the scenes
        //m_vfats[index] = vfat_shared_ptr(new gem::hw::vfat::HwVFAT2(info.crateID.value_,info.slotID.value_,info.linkID.value_));
        
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
        m_vfats[index] = vfat_shared_ptr(new gem::hw::vfat::HwVFAT2(deviceName, tmpURI.str(), "file://setup/etc/addresstables/"+info.addressTable.toString()));
        //INFO("connecting to device");
        //m_vfats[index]->connectDevice();
        */
        INFO("connected");
      } catch (gem::hw::vfat::exception::Exception const& ex) {
        ERROR("caught exception " << ex.what());
        XCEPT_RAISE(gem::hw::vfat::exception::Exception, "initializeAction failed");
      } catch (toolbox::net::exception::MalformedURN const& ex) {
        ERROR("caught exception " << ex.what());
        XCEPT_RAISE(gem::hw::vfat::exception::Exception, "initializeAction failed");
      } catch (std::exception const& ex) {
        ERROR("caught exception " << ex.what());
        XCEPT_RAISE(gem::hw::vfat::exception::Exception, "initializeAction failed");
      }
      //set the web view to be empty or grey
      //if (!info.present.value_) continue;
      //p_gemWebInterface->vfatInSlot(slot);

      INFO("grabbing pointer to hardware device");
      vfat_shared_ptr vfat = m_vfats[index];

      if (vfat->isHwConnected()) {
        //return;
        INFO("VFAT connected on link " << link << " to GLIB in slot " << (slot+1));
      } else {
        WARN("VFAT connected on link " << link << " to GLIB in slot " << (slot+1) << " is not responding");
      }
    }
  }  
  INFO("gem::hw::vfat::VFATManager::initializeAction end");
}

void gem::hw::vfat::VFATManager::configureAction()
  throw (gem::hw::vfat::exception::Exception)
{
  INFO("gem::hw::vfat::VFATManager::configureAction");
  //will the manager operate for all connected vfats, or only those connected to certain GLIBs?
  for (int slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    usleep(1000);
    for (int link = 0; link < MAX_VFATS_PER_AMC; ++link) {
      usleep(1000);
      unsigned int index = (slot*MAX_VFATS_PER_AMC)+link;
      VFATInfo& info = m_vfatInfo[index].bag;

      INFO("line 231: info is: " << info.toString());
      if (!info.present)
        continue;
      
      INFO("grabbing pointer to hardware device");
      vfat_shared_ptr vfat = m_vfats[index];
      
      if (vfat->isHwConnected()) {
        /*
        INFO("setting cdce clock source to 0x" << std::hex << info.cdceClkSrc.value_ << std::dec);
        vfat->setSBitSource(info.cdceClkSrc.value_);
        */
        for (unsigned olink = 0; olink < 3; ++olink) {
        }
        //what else is required for configuring the VFAT?
        //need to reset optical links?
        //reset counters?
      } else {
        WARN("VFAT connected on link " << link << " to GLIB in slot " << (slot+1) << " is not responding");
      }
    }
  }
  
  INFO("gem::hw::vfat::VFATManager::configureAction end");
}

void gem::hw::vfat::VFATManager::startAction()
  throw (gem::hw::vfat::exception::Exception)
{
  usleep(1000);
}

void gem::hw::vfat::VFATManager::pauseAction()
  throw (gem::hw::vfat::exception::Exception)
{
  usleep(1000);
}

void gem::hw::vfat::VFATManager::resumeAction()
  throw (gem::hw::vfat::exception::Exception)
{
  usleep(1000);
}

void gem::hw::vfat::VFATManager::stopAction()
  throw (gem::hw::vfat::exception::Exception)
{
  usleep(1000);
}

void gem::hw::vfat::VFATManager::haltAction()
  throw (gem::hw::vfat::exception::Exception)
{
  usleep(1000);
}

void gem::hw::vfat::VFATManager::resetAction()
  throw (gem::hw::vfat::exception::Exception)
{
  //unregister listeners and items in info spaces
  INFO("gem::hw::vfat::VFATManager::resetAction begin");
  for (int slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    usleep(1000);
    INFO("VFATManager::looping over slots(" << (slot+1) << ") and finding expected cards");
    for (int link = 0; link < MAX_VFATS_PER_AMC; ++link) {
      usleep(1000);
      INFO("VFATManager::looping over links(" << link << ") and finding expected cards");
      unsigned int index = (slot*MAX_VFATS_PER_AMC)+link;
      VFATInfo& info = m_vfatInfo[index].bag;
      
      if (!info.present)
        continue;
      // set up the info space here rather than in initialize (where it can then get unset in reset?
      // should a value be set up for all of them by default?

      INFO("revoking hwCfgInfoSpace items for board connected on link " << link << " to GLIB in slot " << (slot+1));
      toolbox::net::URN hwCfgURN("urn:gem:hw:"+toolbox::toString("gem.shelf%02d.glib%02d.vfat%02d",
                                                                 info.crateID.value_,
                                                                 info.slotID.value_,
                                                                 info.linkID.value_));
      if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
        INFO("resetAction::infospace " << hwCfgURN.toString() << " already exists, getting");
        is_vfats[slot] = xdata::getInfoSpaceFactory()->get(hwCfgURN.toString());
      } else {
        INFO("resetAction::infospace " << hwCfgURN.toString() << " does not exist, no further action");
        continue;
      }
      
      INFO("revoking config parameters into infospace");
      if (is_vfats[slot]->hasItem("ControlHubAddress"))
        is_vfats[slot]->fireItemRevoked("ControlHubAddress");
      
      if (is_vfats[slot]->hasItem("IPBusProtocol"))
        is_vfats[slot]->fireItemRevoked("IPBusProtocol");
      
      if (is_vfats[slot]->hasItem("DeviceIPAddress"))
        is_vfats[slot]->fireItemRevoked("DeviceIPAddress");
      
      if (is_vfats[slot]->hasItem("AddressTable"))
        is_vfats[slot]->fireItemRevoked("AddressTable");
      
      if (is_vfats[slot]->hasItem("ControlHubPort"))
        is_vfats[slot]->fireItemRevoked("ControlHubPort");
      
      if (is_vfats[slot]->hasItem("IPBusPort"))
        is_vfats[slot]->fireItemRevoked("IPBusPort");
    }
  }
}

/*
void gem::hw::vfat::VFATManager::noAction()
  throw (gem::hw::vfat::exception::Exception)
{
}
*/

void gem::hw::vfat::VFATManager::failAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}

void gem::hw::vfat::VFATManager::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}
