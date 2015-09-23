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
  controlHubIPAddress = "";
  deviceIPAddress     = "";
  ipBusProtocol       = "";
  addressTable        = "";
  controlHubPort      = 0;
  ipBusPort           = 0;
  
  triggerSource = 0;
  sbitSource    = 0;
}

void gem::hw::optohybrid::OptoHybridManager::OptoHybridInfo::registerFields(xdata::Bag<gem::hw::optohybrid::OptoHybridManager::OptoHybridInfo>* bag) {
  bag->addField("crateID",       &crateID);
  bag->addField("slot",          &slotID);
  bag->addField("present",       &present);

  bag->addField("ControlHubIPAddress", &controlHubIPAddress);
  bag->addField("DeviceIPAddress",     &deviceIPAddress);
  bag->addField("IPBusProtocol",       &ipBusProtocol);
  bag->addField("AddressTable",        &addressTable);
  bag->addField("ControlHubPort",      &controlHubPort);
  bag->addField("IPBusPort",           &ipBusPort);
            
  bag->addField("triggerSource", &triggerSource);
  bag->addField("sbitSource",    &sbitSource);
}

gem::hw::optohybrid::OptoHybridManager::OptoHybridManager(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub)
{
  m_optohybridInfo.setSize(MAX_AMCS_PER_CRATE);
  //maybe we put this type of stuff into a per OptoHybrid infospace, in the monitor?
  //p_appInfoSpace->addItemRetrieveListener("AllOptoHybridsInfo", this);
  //p_appInfoSpace->addItemChangedListener( "AllOptoHybridsInfo", this);
  //p_appInfoSpace->fireItemValueRetrieve("AllOptoHybridsInfo", &m_optohybridInfo);

  //p_appInfoSpace->fireItemAvailable("crateID", &m_crateID);
  //p_appInfoSpace->fireItemAvailable("slot",    &m_slot);
  p_appInfoSpace->fireItemAvailable("AllOptoHybridsInfo",  &m_optohybridInfo);
  p_appInfoSpace->fireItemAvailable("ConnectionFile",&m_connectionFile);


  //initialize the OptoHybrid application objects
  DEBUG("Connecting to the OptoHybridManagerWeb interface");
  p_gemWebInterface = new gem::hw::optohybrid::OptoHybridManagerWeb(this);
  //p_gemMonitor      = new gem::hw::optohybrid::OptoHybridHwMonitor(this);
  DEBUG("done");
  
  for (int slot=1; slot <= MAX_AMCS_PER_CRATE; slot++) {
    if (m_optohybrids[slot-1])
      delete m_optohybrids[slot-1];
    m_optohybrids[slot-1] = 0;
  }
  //init();
  //getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemhardware/images/optohybrid/OptoHybridManager.png");
}

gem::hw::optohybrid::OptoHybridManager::~OptoHybridManager() {
  //memory management, maybe not necessary here?
  for (int slot=1; slot <= MAX_AMCS_PER_CRATE; slot++) {
    if (m_optohybrids[slot-1])
      delete m_optohybrids[slot-1];
    m_optohybrids[slot-1] = 0;
  }  
}

// This is the callback used for handling xdata:Event objects
void gem::hw::optohybrid::OptoHybridManager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("OptoHybridManager::actionPerformed() setDefaultValues" << 
          "Default configuration values have been loaded from xml profile");
    
    //how to handle passing in various values nested in a vector in a bag
    //for (int slot = 0; slot < MAX_AMCS_PER_CRATE; slot++) {
    for (auto slot = m_optohybridInfo.begin(); slot != m_optohybridInfo.end(); ++slot) {
      if (slot->bag.present.value_)
        INFO("Found attribute:" << slot->bag.toString());
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
  INFO("gem::hw::optohybrid::OptoHybridManager::initializeAction begin");
  INFO("gem::hw::optohybrid::OptoHybridManager::initializeAction end");
}

void gem::hw::optohybrid::OptoHybridManager::configureAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
}

void gem::hw::optohybrid::OptoHybridManager::startAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
}

void gem::hw::optohybrid::OptoHybridManager::pauseAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
}

void gem::hw::optohybrid::OptoHybridManager::resumeAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
}

void gem::hw::optohybrid::OptoHybridManager::stopAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
}

void gem::hw::optohybrid::OptoHybridManager::haltAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
}

void gem::hw::optohybrid::OptoHybridManager::resetAction()
  throw (gem::hw::optohybrid::exception::Exception)
{
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
