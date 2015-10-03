#include <iomanip>

#include "gem/hw/optohybrid/HwOptoHybrid.h"

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid() :
  gem::hw::GEMHwDevice::GEMHwDevice("HwOptoHybrid"),
  //monOptoHybrid_(0)
  b_links({false,false,false}),
  m_controlLink(-1)  
{
  setDeviceID("OptoHybridHw");
  setAddressTableFileName("glib_address_table.xml");
  //need to know which device this is 0 or 1?
  setDeviceBaseNode("GLIB.OptoHybrid_0.OptoHybrid");
  //gem::hw::optohybrid::HwOptoHybrid::initDevice();
  //set up which links are active, so that the control can be done without specifying a link
  INFO("HwOptoHybrid ctor done " << isHwConnected());
}

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(std::string const& optohybridDevice,
                                                std::string const& connectionFile) :
  gem::hw::GEMHwDevice::GEMHwDevice(optohybridDevice, connectionFile),
  //monOptoHybrid_(0)
  b_links({false,false,false}),
  m_controlLink(-1)  
{
  std::stringstream basenode;
  basenode << "GLIB.OptoHybrid_" << *optohybridDevice.rbegin() << ".OptoHybrid";
  setDeviceBaseNode(basenode.str());
  INFO("HwOptoHybrid ctor done " << isHwConnected());
}

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(std::string const& optohybridDevice,
                                                std::string const& connectionURI,
                                                std::string const& addressTable) :
  gem::hw::GEMHwDevice::GEMHwDevice(optohybridDevice, connectionURI, addressTable),
  //monOptoHybrid_(0)
  b_links({false,false,false}),
  m_controlLink(-1)  
{
  std::stringstream basenode;
  basenode << "GLIB.OptoHybrid_" << *optohybridDevice.rbegin() << ".OptoHybrid";
  setDeviceBaseNode(basenode.str());
  INFO("HwOptoHybrid ctor done " << isHwConnected());
}

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(std::string const& optohybridDevice,
                                                uhal::HwInterface& uhalDevice) :
  gem::hw::GEMHwDevice::GEMHwDevice(optohybridDevice,uhalDevice),
  //monOptoHybrid_(0)
  b_links({false,false,false}),
  m_controlLink(-1)  
{
  std::stringstream basenode;
  basenode << "GLIB.OptoHybrid_" << *optohybridDevice.rbegin() << ".OptoHybrid";
  setDeviceBaseNode(basenode.str());
  INFO("HwOptoHybrid ctor done " << isHwConnected());
}
/*
gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(gem::hw::glib::HwGLIB const& glib,
                                                int const& slot) :
  gem::hw::GEMHwDevice::GEMHwDevice("HwOptoHybrid"),
  //monOptoHybrid_(0),
  b_links({false,false,false}),
  m_controlLink(-1),
  m_slot(slot)
{
  //use a connection file and connection manager?
  setDeviceID(toolbox::toString("%s.optohybrid%02d",glib.getDeviceID().c_str(),slot));
  //uhal::ConnectionManager manager ( "file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml" );
  p_gemConnectionManager.reset(new uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml"));
  p_gemHW.reset(new uhal::HwInterface(p_gemConnectionManager->getDevice(this->getDeviceID())));
  //p_gemConnectionManager = std::shared_ptr<uhal::ConnectionManager>(uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml"));
  //p_gemHW = std::shared_ptr<uhal::HwInterface>(p_gemConnectionManager->getDevice(this->getDeviceID()));
  //setAddressTableFileName("glib_address_table.xml");
  //setDeviceIPAddress(toolbox::toString("192.168.0.%d",160+slot));
  setDeviceBaseNode("OptoHybrid");
  //gem::hw::optohybrid::HwOptoHybrid::initDevice();
}
*/
gem::hw::optohybrid::HwOptoHybrid::~HwOptoHybrid()
{
  //releaseDevice();
}

//void gem::hw::optohybrid::HwOptoHybrid::configureDevice(std::string const& xmlSettings)
//{
//  //here load the xml file settings onto the board
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::configureDevice()
//{
//  //determine the manner in which to configure the device (XML or DB parameters)
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::releaseDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::enableDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::disableDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::pauseDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::startDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::stopDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::resumeDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::haltDevice()
//{
//
//}
//

bool gem::hw::optohybrid::HwOptoHybrid::isHwConnected() 
{
  if ( b_is_connected ) {
    DEBUG("HwOptoHybrid connection good");
    return true;
  } else if (gem::hw::GEMHwDevice::isHwConnected()) {
    DEBUG("Checking hardware connection");

    if ((this->getFirmwareDate()).rfind("15") != std::string::npos) {
      b_is_connected = true;
      INFO("OptoHybrid present(0x" << std::hex << this->getFirmware() << std::dec << ")");
      return true;
    } else {
      b_is_connected = false;
      DEBUG("OptoHybrid not reachable (unable to find 15 in the firmware string)");
      return false;
    }
  }
}


gem::hw::GEMHwDevice::OpticalLinkStatus gem::hw::optohybrid::HwOptoHybrid::LinkStatus()
{
  gem::hw::GEMHwDevice::OpticalLinkStatus linkStatus;
  
  linkStatus.TRK_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX.TRK_ERR"));
  linkStatus.TRG_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX.TRG_ERR"));
  linkStatus.Data_Packets = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX.DATA_Packets"));
  return linkStatus;
}

void gem::hw::optohybrid::HwOptoHybrid::LinkReset(uint8_t const& resets)
{
  if (resets&0x1)
    writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX.TRK_ERR.Reset"),0x1);
  if (resets&0x2)
    writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX.TRG_ERR.Reset"),0x1);
  if (resets&0x4)
    writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX.DATA_Packets.Reset"),0x1);
}

//uint32_t gem::hw::optohybrid::HwOptoHybrid::readTriggerData() {
//  return uint32_t value;
//}

std::vector<uint32_t> gem::hw::optohybrid::HwOptoHybrid::broadcastRead(std::string const& name,
                                                                       uint32_t const& mask,
                                                                       bool reset=false)
{
  if (reset)
    writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Reset"),0x1);
  writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Mask"),mask);
  uint32_t tmp = readReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.%s", name.c_str()));
  
  std::stringstream regName;
  regName << getDeviceBaseNode() << "GEB.Broadcast.Results";
  return readBlock(regName.str(),24);
}

void gem::hw::optohybrid::HwOptoHybrid::broadcastWrite(std::string const& name,
                                                       uint32_t const& mask,
                                                       uint32_t const& value,
                                                       bool reset=false)
{
  if (reset)
    writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Reset"),0x1);
  writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Mask"),mask);
  writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.%s", name.c_str()),value);
  //return readBlock(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Results"),24);
}
