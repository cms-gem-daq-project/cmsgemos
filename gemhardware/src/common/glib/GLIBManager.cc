/**
 * class: GLIBManager
 * description: Manager application for GLIB cards
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date: 
 */

#include "gem/hw/glib/GLIBManagerWeb.h"
#include "gem/hw/glib/GLIBManager.h"
#include "gem/hw/glib/GLIBMonitor.h"

#include "gem/hw/glib/HwGLIB.h"
//#include "gem/hw/glib/exception/Exception.h"

XDAQ_INSTANTIATOR_IMPL(gem::hw::glib::GLIBManager);

gem::hw::glib::GLIBManager::GLIBInfo::GLIBInfo() {
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

void gem::hw::glib::GLIBManager::GLIBInfo::registerFields(xdata::Bag<gem::hw::glib::GLIBManager::GLIBInfo>* bag) {
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

  p_appInfoSpace->fireItemAvailable("AllGLIBsInfo",  &m_glibInfo);
  p_appInfoSpace->fireItemAvailable("AMCSlots",      &m_amcSlots);
  p_appInfoSpace->fireItemAvailable("ConnectionFile",&m_connectionFile);

  p_appInfoSpace->addItemRetrieveListener("AllGLIBsInfo",   this);
  p_appInfoSpace->addItemRetrieveListener("AMCSlots",       this);
  p_appInfoSpace->addItemRetrieveListener("ConnectionFile", this);
  p_appInfoSpace->addItemChangedListener( "AllGLIBsInfo",   this);
  p_appInfoSpace->addItemChangedListener( "AMCSlots",       this);
  p_appInfoSpace->addItemChangedListener( "ConnectionFile", this);

  //initialize the GLIB application objects
  DEBUG("Connecting to the GLIBManagerWeb interface");
  p_gemWebInterface = new gem::hw::glib::GLIBManagerWeb(this);
  //p_gemMonitor      = new gem::hw::glib::GLIBHwMonitor(this);
  DEBUG("done");
  
  //set up the info hwCfgInfoSpace 
  init();

  //getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemhardware/images/glib/GLIBManager.png");
}

gem::hw::glib::GLIBManager::~GLIBManager() {
  //memory management, maybe not necessary here?
}

uint16_t gem::hw::glib::GLIBManager::parseAMCEnableList(std::string const& enableList)
{
  uint16_t slotMask = 0x0;
  std::vector<std::string> slots;

  boost::split(slots, enableList, boost::is_any_of(", "), boost::token_compress_on);  
  DEBUG("AMC input enable list is " << enableList);
  for (auto slot = slots.begin(); slot != slots.end(); ++slot) {
    DEBUG("slot is " << *slot);
    if (slot->find('-') != std::string::npos) { // found a possible range
      DEBUG("found a hyphen in " << *slot);
      std::vector<std::string> range;
      boost::split(range, *slot, boost::is_any_of("-"), boost::token_compress_on);
      if (range.size() > 2) {
        WARN("parseAMCEnableList::Found poorly formatted range " << *slot);
        continue;
      }
      if (isValidSlotNumber(range.at(0)) && isValidSlotNumber(range.at(1))) {
        std::stringstream ss0(range.at(0));
        std::stringstream ss1(range.at(1));
        int min, max;
        ss0 >> min;
        ss1 >> max;
        
        if (min == max) {
          WARN("parseAMCEnableList::Found poorly formatted range " << *slot);
          continue;
        }
        if (min > max) { // elements in the wrong order
          WARN("parseAMCEnableList::Found poorly formatted range " << *slot);
          continue;
        }
        
        for (int islot = min; islot <= max; ++islot) {
          slotMask |= (0x1 << (islot-1));
        } //  end loop over range of list
      } // end check on valid values
    } else { //not a range
      DEBUG("found no hyphen in " << *slot);
      if (slot->length() > 2) {
        WARN("parseAMCEnableList::Found longer value than expected (1-12) " << *slot);
        continue;
      }
      
      if (!isValidSlotNumber(*slot)) {
        WARN("parseAMCEnableList::Found invalid value " << *slot);
        continue;
      }
      std::stringstream ss(*slot);
      int slotNum = -1;
      ss >> slotNum;
      slotMask |= (0x1 << (slotNum-1));
    } //done processing single values
  } //done looping over extracted values
  DEBUG("parseAMCEnableList::Parsed enabled list 0x" << std::hex << slotMask << std::dec);
  return slotMask;
}

bool gem::hw::glib::GLIBManager::isValidSlotNumber(std::string const& s)
{
  try {
    int i_val;
    i_val = std::stoi(s);
    if (!(i_val > 0 && i_val < 13)) {
      ERROR("isValidSlotNumber::Found value outside expected (1-12) " << i_val);
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
void gem::hw::glib::GLIBManager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("GLIBManager::actionPerformed() setDefaultValues" << 
          "Default configuration values have been loaded from xml profile");
    m_amcEnableMask = parseAMCEnableList(m_amcSlots.toString());
    INFO("Parsed AMCEnableList m_amcSlots = " << m_amcSlots.toString()
         << " to slotMask 0x" << std::hex << m_amcEnableMask << std::dec);
    
    //how to handle passing in various values nested in a vector in a bag
    for (auto slot = m_glibInfo.begin(); slot != m_glibInfo.end(); ++slot) {
      if (slot->bag.present.value_)
        INFO("line 183::Found attribute:" << slot->bag.toString());
    }
    //p_gemMonitor->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::glib::GLIBManager::init()
{
  /*
  INFO("gem::hw::glib::GLIBManager::init begin");
  for (int slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {    
    INFO("GLIBManager::looping over slots(" << (slot+1) << ") and finding infospace items");
    GLIBInfo& info = m_glibInfo[slot].bag;
    
    if (!info.present)
      continue;
    
    toolbox::net::URN hwCfgURN("urn:gem:hw:"+toolbox::toString("gem.shelf%02d.glib%02d",
                                                               info.crateID.value_,
                                                               info.slotID.value_));

    INFO("creating hwCfgInfoSpace items for GLIB in slot " << (slot+1) << " with URN " << hwCfgURN.toString());
    if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
      INFO("init::infospace " << hwCfgURN.toString() << " already exists, getting");
      is_glibs[slot] = xdata::getInfoSpaceFactory()->get(hwCfgURN.toString());
    } else {
      INFO("init::infospace " << hwCfgURN.toString() << " does not exist, creating");
      is_glibs[slot] = xdata::getInfoSpaceFactory()->create(hwCfgURN.toString());
    }
    
    INFO("exporting config parameters into infospace");
    is_glibs[slot]->fireItemAvailable("ControlHubAddress", &info.controlHubAddress);
    is_glibs[slot]->fireItemAvailable("IPBusProtocol",     &info.ipBusProtocol);
    is_glibs[slot]->fireItemAvailable("DeviceIPAddress",   &info.deviceIPAddress);
    is_glibs[slot]->fireItemAvailable("AddressTable",      &info.addressTable);
    is_glibs[slot]->fireItemAvailable("ControlHubPort",    &info.controlHubPort);
    is_glibs[slot]->fireItemAvailable("IPBusPort",         &info.ipBusPort);
    
    is_glibs[slot]->fireItemValueChanged("ControlHubAddress");
    is_glibs[slot]->fireItemValueChanged("IPBusProtocol");
    is_glibs[slot]->fireItemValueChanged("DeviceIPAddress");
    is_glibs[slot]->fireItemValueChanged("AddressTable");
    is_glibs[slot]->fireItemValueChanged("ControlHubPort");
    is_glibs[slot]->fireItemValueChanged("IPBusPort");
    
    INFO("InfoSpace found item: ControlHubAddress " << is_glibs[slot]->find("ControlHubAddress"));
    INFO("InfoSpace found item: IPBusProtocol "     << is_glibs[slot]->find("IPBusProtocol")    );
    INFO("InfoSpace found item: DeviceIPAddress "   << is_glibs[slot]->find("DeviceIPAddress")  );
    INFO("InfoSpace found item: AddressTable "      << is_glibs[slot]->find("AddressTable")     );
    INFO("InfoSpace found item: ControlHubPort "    << is_glibs[slot]->find("ControlHubPort")   );
    INFO("InfoSpace found item: IPBusPort "         << is_glibs[slot]->find("IPBusPort")        );
    
    INFO("info:" << info.toString());

    INFO("InfoSpace item value: ControlHubAddress " << info.controlHubAddress.toString());
    INFO("InfoSpace item value: IPBusProtocol "     << info.ipBusProtocol.toString()    );
    INFO("InfoSpace item value: DeviceIPAddress "   << info.deviceIPAddress.toString()  );
    INFO("InfoSpace item value: AddressTable "      << info.addressTable.toString()     );
    INFO("InfoSpace item value: ControlHubPort "    << info.controlHubPort.toString()   );
    INFO("InfoSpace item value: IPBusPort "         << info.ipBusPort.toString()        );
  }
  */
}

//state transitions
void gem::hw::glib::GLIBManager::initializeAction()
  throw (gem::hw::glib::exception::Exception)
{
  INFO("gem::hw::glib::GLIBManager::initializeAction begin");
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {    
    INFO("GLIBManager::looping over slots(" << (slot+1) << ") and finding expected cards");
    GLIBInfo& info = m_glibInfo[slot].bag;
    if ((m_amcEnableMask >> (slot)) & 0x1) {
      INFO("line 204::info:" << info.toString());
      INFO("expect a card in slot " << (slot+1));
      info.slotID  = slot+1;
      info.present = true;
      //actually check presence? this just says that we expect it to be there
      //check if there is a GLIB in the specified slot, if not, do not initialize
      //set the web view to be empty or grey
      //if (!info.present.value_) continue;
      // needs .value_?
      //p_gemWebInterface->glibInSlot(slot);
    }
  }  
  
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    GLIBInfo& info = m_glibInfo[slot].bag;
    
    //check the config file if there should be a GLIB in the specified slot, if not, do not initialize
    if (!info.present)
      continue;
    
    INFO("line 228::info:" << info.toString());
    INFO("creating pointer to card in slot " << (slot+1));
    
    //create the cfgInfoSpace object (qualified vs non?)
    //toolbox::net::URN hwCfgURN = this->createQualifiedInfoSpace("urn:gem:hw:"+toolbox::toString("gem.shelf%02d.glib%02d",info.crateID.value_,info.slotID.value_));
    std::string deviceName = toolbox::toString("gem.shelf%02d.glib%02d",
                                               info.crateID.value_,
                                               info.slotID.value_);
    toolbox::net::URN hwCfgURN("urn:gem:hw:"+deviceName);

    if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
      INFO("initializeAction::infospace " << hwCfgURN.toString() << " already exists, getting");
      is_glibs[slot] = xdata::getInfoSpaceFactory()->get(hwCfgURN.toString());
    } else {
      INFO("initializeAction::infospace " << hwCfgURN.toString() << " does not exist, creating");
      is_glibs[slot] = xdata::getInfoSpaceFactory()->create(hwCfgURN.toString());
    }

    INFO("exporting config parameters into infospace");
      /* figure out how to make it work like this
         probably just have to define begin/end for OptoHybridInfo class and iterators
      for (auto monitorable = info.begin(); monitorable != info.end(); ++monitorable)
        if (is_optohybrids[slot]->hasItem(monitorable->getName()))
          is_optohybrids[slot]->fireItemAvailable(monitorable->getName(), monitorable->getSerializableValue());
      */
    if (!is_glibs[slot]->hasItem("ControlHubAddress"))
      is_glibs[slot]->fireItemAvailable("ControlHubAddress", &info.controlHubAddress);
    
    if (!is_glibs[slot]->hasItem("IPBusProtocol"))
      is_glibs[slot]->fireItemAvailable("IPBusProtocol",     &info.ipBusProtocol);
    
    if (!is_glibs[slot]->hasItem("DeviceIPAddress"))
      is_glibs[slot]->fireItemAvailable("DeviceIPAddress",   &info.deviceIPAddress);
    
    if (!is_glibs[slot]->hasItem("AddressTable"))
      is_glibs[slot]->fireItemAvailable("AddressTable",      &info.addressTable);
    
    if (!is_glibs[slot]->hasItem("ControlHubPort"))
      is_glibs[slot]->fireItemAvailable("ControlHubPort",    &info.controlHubPort);
    
    if (!is_glibs[slot]->hasItem("IPBusPort"))
      is_glibs[slot]->fireItemAvailable("IPBusPort",         &info.ipBusPort);
    //is_glibs[slot]->fireItemAvailable("ControlHubAddress", &info.controlHubAddress);
    //is_glibs[slot]->fireItemAvailable("IPBusProtocol",     &info.ipBusProtocol);
    //is_glibs[slot]->fireItemAvailable("DeviceIPAddress",   &info.deviceIPAddress);
    //is_glibs[slot]->fireItemAvailable("AddressTable",      &info.addressTable);
    //is_glibs[slot]->fireItemAvailable("ControlHubPort",    &info.controlHubPort);
    //is_glibs[slot]->fireItemAvailable("IPBusPort",         &info.ipBusPort);
    
    INFO("InfoSpace found item: ControlHubAddress " << is_glibs[slot]->find("ControlHubAddress"));
    INFO("InfoSpace found item: IPBusProtocol "     << is_glibs[slot]->find("IPBusProtocol")    );
    INFO("InfoSpace found item: DeviceIPAddress "   << is_glibs[slot]->find("DeviceIPAddress")  );
    INFO("InfoSpace found item: AddressTable "      << is_glibs[slot]->find("AddressTable")     );
    INFO("InfoSpace found item: ControlHubPort "    << is_glibs[slot]->find("ControlHubPort")   );
    INFO("InfoSpace found item: IPBusPort "         << is_glibs[slot]->find("IPBusPort")        );

    is_glibs[slot]->fireItemValueRetrieve("ControlHubAddress");
    is_glibs[slot]->fireItemValueRetrieve("IPBusProtocol");
    is_glibs[slot]->fireItemValueRetrieve("DeviceIPAddress");
    is_glibs[slot]->fireItemValueRetrieve("AddressTable");
    is_glibs[slot]->fireItemValueRetrieve("ControlHubPort");
    is_glibs[slot]->fireItemValueRetrieve("IPBusPort");

    is_glibs[slot]->fireItemValueChanged("ControlHubAddress");
    is_glibs[slot]->fireItemValueChanged("IPBusProtocol");
    is_glibs[slot]->fireItemValueChanged("DeviceIPAddress");
    is_glibs[slot]->fireItemValueChanged("AddressTable");
    is_glibs[slot]->fireItemValueChanged("ControlHubPort");
    is_glibs[slot]->fireItemValueChanged("IPBusPort");

    
    INFO("initializeAction::info:" << info.toString());
    INFO("InfoSpace item value: ControlHubAddress " << info.controlHubAddress.toString());
    INFO("InfoSpace item value: IPBusProtocol "     << info.ipBusProtocol.toString()    );
    INFO("InfoSpace item value: DeviceIPAddress "   << info.deviceIPAddress.toString()  );
    INFO("InfoSpace item value: AddressTable "      << info.addressTable.toString()     );
    INFO("InfoSpace item value: ControlHubPort "    << info.controlHubPort.toString()   );
    INFO("InfoSpace item value: IPBusPort "         << info.ipBusPort.toString()        );
    
    try {
      INFO("obtaining pointer to HwGLIB");
      /*this constructor is not sensible, as the connection file is expected to be found from the
        running directory of the application, and not based on environment variables
      */
      //still uses the above method behind the scenes
      //m_glibs[slot] = glib_shared_ptr(new gem::hw::glib::HwGLIB(info.crateID.value_,info.slotID.value_));
      m_glibs[slot] = glib_shared_ptr(new gem::hw::glib::HwGLIB(deviceName, m_connectionFile.toString()));
      m_glibMonitors[slot] = std::make_shared<GLIBMonitor>(GLIBMonitor(m_glibs[slot], this));
      // m_glibMonitors[slot]->addInfoSpace("");
      // m_glibMonitors[slot]->startMonitoring();
      /*
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
      m_glibs[slot] = glib_shared_ptr(new gem::hw::glib::HwGLIB(deviceName, tmpURI.str(), "file://${GEM_ADDRESS_TABLE_PATH}/"+info.addressTable.toString()));
      //INFO("connecting to device");
      //m_glibs[slot]->connectDevice();
      */
      INFO("connected");
    } catch (gem::hw::glib::exception::Exception const& ex) {
      ERROR("caught exception " << ex.what());
      XCEPT_RAISE(gem::hw::glib::exception::Exception, "initializeAction failed");
    } catch (toolbox::net::exception::MalformedURN const& ex) {
      ERROR("caught exception " << ex.what());
      XCEPT_RAISE(gem::hw::glib::exception::Exception, "initializeAction failed");
    } catch (std::exception const& ex) {
      ERROR("caught exception " << ex.what());
      XCEPT_RAISE(gem::hw::glib::exception::Exception, "initializeAction failed");
    }
    //set the web view to be empty or grey
    //if (!info.present.value_) continue;
    //p_gemWebInterface->glibInSlot(slot);
  }

  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    GLIBInfo& info = m_glibInfo[slot].bag;

    if (!info.present)
      continue;
    
    INFO("grabbing pointer to hardware device");
    glib_shared_ptr glib = m_glibs[slot];
    
    if (glib->isHwConnected()) {
      //return;
      INFO("connected a card in slot " << (slot+1));
    } else {
      WARN("GLIB in slot " << (slot+1) << " is not connected");
    }
  }
  INFO("gem::hw::glib::GLIBManager::initializeAction end");
}

void gem::hw::glib::GLIBManager::configureAction()
  throw (gem::hw::glib::exception::Exception)
{
  INFO("gem::hw::glib::GLIBManager::configureAction");

  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {
    usleep(100);
    GLIBInfo& info = m_glibInfo[slot].bag;

    if (!info.present)
      continue;
    
    INFO("grabbing pointer to hardware device");
    glib_shared_ptr glib = m_glibs[slot];
    
    if (glib->isHwConnected()) {
      INFO("setting trigger source to 0x" << std::hex << info.triggerSource.value_ << std::dec);
      glib->setTrigSource(info.triggerSource.value_);
      
      //should FIFOs be emptied in configure or at start?
      INFO("emptying trigger/tracking data FIFOs");
      for (unsigned link = 0; link < HwGLIB::N_GTX; ++link) {
        //glib->flushTriggerFIFO(link);
        glib->flushFIFO(link);
      }
      //what else is required for configuring the GLIB?
      //need to reset optical links?
      //reset counters?
    } else {
      WARN("GLIB in slot " << (slot+1) << " is not connected");
    }
  }
  
  INFO("gem::hw::glib::GLIBManager::configureAction end");
}

void gem::hw::glib::GLIBManager::startAction()
  throw (gem::hw::glib::exception::Exception)
{
  //what is required for starting the GLIB?
  usleep(100);
}

void gem::hw::glib::GLIBManager::pauseAction()
  throw (gem::hw::glib::exception::Exception)
{
  //what is required for pausing the GLIB?
  usleep(100);
}

void gem::hw::glib::GLIBManager::resumeAction()
  throw (gem::hw::glib::exception::Exception)
{
  //what is required for resuming the GLIB?
  usleep(100);
}

void gem::hw::glib::GLIBManager::stopAction()
  throw (gem::hw::glib::exception::Exception)
{
  //what is required for stopping the GLIB?
  usleep(100);
}

void gem::hw::glib::GLIBManager::haltAction()
  throw (gem::hw::glib::exception::Exception)
{
  //what is required for halting the GLIB?
  usleep(100);
}

void gem::hw::glib::GLIBManager::resetAction()
  throw (gem::hw::glib::exception::Exception)
{
  //what is required for resetting the GLIB?
  //unregister listeners and items in info spaces
  
  INFO("gem::hw::glib::GLIBManager::resetAction begin");
  for (unsigned slot = 0; slot < MAX_AMCS_PER_CRATE; ++slot) {    
    usleep(100);
    INFO("GLIBManager::looping over slots(" << (slot+1) << ") and finding infospace items");
    GLIBInfo& info = m_glibInfo[slot].bag;
    
    if (!info.present)
      continue;
    
    INFO("looking for hwCfgInfoSpace items for GLIB in slot " << (slot+1));
    toolbox::net::URN hwCfgURN("urn:gem:hw:"+toolbox::toString("gem.shelf%02d.glib%02d",
                                                               info.crateID.value_,
                                                               info.slotID.value_));
    
    if (xdata::getInfoSpaceFactory()->hasItem(hwCfgURN.toString())) {
      INFO("resetAction::infospace " << hwCfgURN.toString() << " already exists, getting");
      is_glibs[slot] = xdata::getInfoSpaceFactory()->get(hwCfgURN.toString());
    } else {
      INFO("resetAction::infospace " << hwCfgURN.toString() << " does not exist, no further action");
      continue;
    }
    
    INFO("revoking config parameters infospace");
    if (is_glibs[slot]->hasItem("ControlHubAddress"))
      is_glibs[slot]->fireItemRevoked("ControlHubAddress");
    
    if (is_glibs[slot]->hasItem("IPBusProtocol"))
      is_glibs[slot]->fireItemRevoked("IPBusProtocol");
    
    if (is_glibs[slot]->hasItem("DeviceIPAddress"))
      is_glibs[slot]->fireItemRevoked("DeviceIPAddress");
    
    if (is_glibs[slot]->hasItem("AddressTable"))
      is_glibs[slot]->fireItemRevoked("AddressTable");
    
    if (is_glibs[slot]->hasItem("ControlHubPort"))
      is_glibs[slot]->fireItemRevoked("ControlHubPort");
    
    if (is_glibs[slot]->hasItem("IPBusPort"))
      is_glibs[slot]->fireItemRevoked("IPBusPort");
  }
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
