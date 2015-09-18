/**
 * class: GLIBManager
 * description: Manager application for GLIB cards
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date: 
 */

#include "gem/hw/glib/GLIBManagerWeb.h"
#include "gem/hw/glib/GLIBManager.h"

#include "gem/hw/glib/HwGLIB.h"
//#include "gem/hw/glib/exception/Exception.h"

XDAQ_INSTANTIATOR_IMPL(gem::hw::glib::GLIBManager);

gem::hw::glib::GLIBManager::GLIBInfo::GLIBInfo() {
  present = false;
  crateID = -1;
  slotID  = -1;
}

void gem::hw::glib::GLIBManager::GLIBInfo::registerFields(xdata::Bag<gem::hw::glib::GLIBManager::GLIBInfo>* bag) {
  bag->addField("crateID", &crateID);
  bag->addField("slot",    &slotID);
  bag->addField("present", &present);
}

gem::hw::glib::GLIBManager::GLIBManager(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub),
  m_amcEnableMask(0)
{
  //maybe we put this type of stuff into a per GLIB infospace, in the monitor?
  // getApplicationInfoSpace()->fireItemAvailable("crateID", &m_crateID);
  // getApplicationInfoSpace()->fireItemAvailable("slot",    &m_slot);
  /*
    p_appInfoSpace->fireItemAvailable("AllGLIBsInfo",     &m_glibInfo);
    p_appInfoSpace->fireItemValueRetrieve("AllGLIBsInfo", &m_glibInfo);
  */
  p_appInfoSpace->fireItemAvailable("AMCSlots",     &m_amcSlots);
  p_appInfoSpace->fireItemValueRetrieve("AMCSlots", &m_amcSlots);

  //initialize the GLIB application objects
  DEBUG("Connecting to the GLIBManagerWeb interface");
  p_gemWebInterface = new gem::hw::glib::GLIBManagerWeb(this);
  //p_gemMonitor      = new gem::hw::glib::GLIBHwMonitor(this);
  DEBUG("done");
  
  for (int slot=1; slot <= MAX_AMCS_PER_CRATE; slot++) {
    if (m_glibs[slot-1])
      delete m_glibs[slot-1];
    m_glibs[slot-1] = 0;
  }
  //init();
  //getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemhardware/images/glib/GLIBManager.png");
}

gem::hw::glib::GLIBManager::~GLIBManager() {
  //memory management, maybe not necessary here?
  for (int slot=1; slot <= MAX_AMCS_PER_CRATE; slot++) {
    if (m_glibs[slot-1])
      delete m_glibs[slot-1];
    m_glibs[slot-1] = 0;
  }  
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
      WARN("isValidSlotNumber::Found value outside expected (1-12) " << i_val);
      return false;
    }
  } catch (std::invalid_argument const& err) {
    WARN("isValidSlotNumber::Unable to convert to integer type " << s << std::endl << err.what());
    return false;
  } catch (std::out_of_range const& err) {
    WARN("isValidSlotNumber::Unable to convert to integer type " << s << std::endl << err.what());
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
    //p_gemMonitor->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::glib::GLIBManager::preInit()
  throw (gem::base::exception::Exception)
{
  
  for (int slot = 1; slot <= MAX_AMCS_PER_CRATE; slot++) {    
    GLIBInfo& info = m_glibInfo[slot].bag;
    if ((m_amcEnableMask >> (slot-1)) & 0x1) {
      info.slotID  = slot;
      info.present = true;
      //actually check presence? this just says that we expect it to be there
      //check if there is a GLIB in the specified slot, if not, do not initialize
      //set the web view to be empty or grey
      //if (!info.present.value_) continue;
      // needs .value_?
      //p_gemWebInterface->glibInSlot(slot);
    }
  }  
}

void gem::hw::glib::GLIBManager::init()
  throw (gem::base::exception::Exception)
{
  gem::base::GEMFSMApplication::init();

  uhal::setLogLevelTo( uhal::ErrorLevel() );
  
  int gemCrate = 1;
  
  for (int slot = 0; slot < MAX_AMCS_PER_CRATE; slot++) {
    GLIBInfo& info = m_glibInfo[slot].bag;
    
    //check the config file if there should be a GLIB in the specified slot, if not, do not initialize
    if (!info.present)
      continue;
    
    //info.present = true;
    //info.slotID  = slot+1;
    info.crateID = gemCrate;
    
    m_glibs[slot] = new gem::hw::glib::HwGLIB(info.crateID,info.slotID);
    m_glibs[slot]->connectDevice();
    //set the web view to be empty or grey
    //if (!info.present.value_) continue;
    //p_gemWebInterface->glibInSlot(slot);
  }

  for (int slot = 0; slot < MAX_AMCS_PER_CRATE; slot++) {
    GLIBInfo& info = m_glibInfo[slot].bag;

    if (!info.present)
      continue;
    
    gem::hw::glib::HwGLIB* glib = m_glibs[slot];
    
    if (glib->isHwConnected())
      return;
  }
}

void gem::hw::glib::GLIBManager::enable()
  throw (gem::base::exception::Exception) {
  DEBUG("Entering gem::hw::glib::GLIBManager::enable()");
  //gem::base::GEMFSMApplication::enable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_deviceLock);
  //m_glibs[0]->startRun();
}

void gem::hw::glib::GLIBManager::disable()
  throw (gem::base::exception::Exception) {
  DEBUG("Entering gem::hw::glib::GLIBManager::disable()");
  //gem::base::GEMFSMApplication::disable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_deviceLock);
  //m_glibs[0]->endRun();
}

//state transitions
void gem::hw::glib::GLIBManager::initializeAction() throw (gem::hw::glib::exception::Exception) {}
void gem::hw::glib::GLIBManager::configureAction()  throw (gem::hw::glib::exception::Exception) {}
void gem::hw::glib::GLIBManager::startAction()      throw (gem::hw::glib::exception::Exception) {}
void gem::hw::glib::GLIBManager::pauseAction()      throw (gem::hw::glib::exception::Exception) {}
void gem::hw::glib::GLIBManager::resumeAction()     throw (gem::hw::glib::exception::Exception) {}
void gem::hw::glib::GLIBManager::stopAction()       throw (gem::hw::glib::exception::Exception) {}
void gem::hw::glib::GLIBManager::haltAction()       throw (gem::hw::glib::exception::Exception) {}
void gem::hw::glib::GLIBManager::resetAction()      throw (gem::hw::glib::exception::Exception) {}
//void gem::hw::glib::GLIBManager::noAction()         throw (gem::hw::glib::exception::Exception) {}

void gem::hw::glib::GLIBManager::failAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}

void gem::hw::glib::GLIBManager::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}
