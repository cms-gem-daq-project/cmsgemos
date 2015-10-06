#include <iomanip>

#include "gem/hw/glib/HwGLIB.h"

gem::hw::glib::HwGLIB::HwGLIB() :
  gem::hw::GEMHwDevice::GEMHwDevice("HwGLIB"),
  //monGLIB_(0),
  m_controlLink(-1),
  m_crate(-1),
  m_slot(-1)
{
  INFO("HwGLIB ctor");
  //use a connection file and connection manager?
  setDeviceID("GLIBHw");
  setAddressTableFileName("glib_address_table.xml");
  setDeviceBaseNode("GLIB");
  //gem::hw::glib::HwGLIB::initDevice();

  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    GLIBIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }
  
  INFO("HwGLIB ctor done " << isHwConnected());
}

gem::hw::glib::HwGLIB::HwGLIB(std::string const& glibDevice,
                              std::string const& connectionFile) :
  gem::hw::GEMHwDevice::GEMHwDevice(glibDevice, connectionFile),
  m_controlLink(-1),
  m_crate(-1),
  m_slot(-1)
{
  setDeviceBaseNode("GLIB");
  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    GLIBIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }
  
  INFO("HwGLIB ctor done " << isHwConnected());
}

gem::hw::glib::HwGLIB::HwGLIB(std::string const& glibDevice,
                              std::string const& connectionURI,
                              std::string const& addressTable) :
  gem::hw::GEMHwDevice::GEMHwDevice(glibDevice, connectionURI, addressTable),
  m_controlLink(-1),
  m_crate(-1),
  m_slot(-1)

{
  INFO("trying to create HwGLIB(" << glibDevice << "," << connectionURI << "," <<addressTable);
  setDeviceBaseNode("GLIB");
  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    GLIBIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }
  
  INFO("HwGLIB ctor done " << isHwConnected());
}

gem::hw::glib::HwGLIB::HwGLIB(std::string const& glibDevice,
                              uhal::HwInterface& uhalDevice) :
  gem::hw::GEMHwDevice::GEMHwDevice(glibDevice,uhalDevice),
  m_controlLink(-1),
  m_crate(-1),
  m_slot(-1)

{
  setDeviceBaseNode("GLIB");
  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    GLIBIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }
  
  INFO("HwGLIB ctor done " << isHwConnected());
}

gem::hw::glib::HwGLIB::HwGLIB(const int& crate, const int& slot) :
  gem::hw::GEMHwDevice::GEMHwDevice(toolbox::toString("gem.shelf%02d.glib%02d",crate,slot)),
  //monGLIB_(0),
  m_controlLink(-1),
  m_crate(crate),
  m_slot(slot)
{
  INFO("HwGLIB ctor");
  //use a connection file and connection manager?
  setDeviceID(toolbox::toString("gem.shelf%02d.glib%02d",crate,slot));
  
  //uhal::ConnectionManager manager ( "file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml" );
  INFO("getting the ConnectionManager pointer");
  //p_gemConnectionManager.reset(new uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml"));
  p_gemConnectionManager.reset(new uhal::ConnectionManager("file://../data/connections_ch.xml"));
  INFO("getting HwInterface " << getDeviceID() << " pointer from ConnectionManager");
  p_gemHW.reset(new uhal::HwInterface(p_gemConnectionManager->getDevice(this->getDeviceID())));
  //p_gemConnectionManager = new uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml");
  //p_gemHW = new uhal::HwInterface(p_gemConnectionManager->getDevice(this->getDeviceID()));
  //setAddressTableFileName("glib_address_table.xml");
  //setDeviceIPAddress(toolbox::toString("192.168.0.%d",160+slot));
  INFO("setting the device base node");
  setDeviceBaseNode("GLIB");
  //gem::hw::glib::HwGLIB::initDevice();
  //  
  //  ipBusErrs.badHeader_     = 0;
  //  ipBusErrs.readError_     = 0;
  //  ipBusErrs.timeouts_      = 0;
  //  ipBusErrs.controlHubErr_ = 0;
  //  
  //setLogLevelTo(uhal::Error());  // Minimise uHAL logging
  
  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    GLIBIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }
  
  INFO("HwGLIB ctor done " << isHwConnected());
}

gem::hw::glib::HwGLIB::~HwGLIB()
{
  //releaseDevice();
}

//void gem::hw::glib::HwGLIB::configureDevice(std::string const& xmlSettings)
//{
//  //here load the xml file settings onto the board
//}
//
//void gem::hw::glib::HwGLIB::configureDevice()
//{
//  //determine the manner in which to configure the device (XML or DB parameters)
//}
//
//void gem::hw::glib::HwGLIB::connectDevice()
//{
//  
//}
//
//void gem::hw::glib::HwGLIB::releaseDevice()
//{
//
//}
//
//void gem::hw::glib::HwGLIB::enableDevice()
//{
//
//}
//
//void gem::hw::glib::HwGLIB::disableDevice()
//{
//
//}
//
//void gem::hw::glib::HwGLIB::pauseDevice()
//{
//
//}
//
//void gem::hw::glib::HwGLIB::startDevice()
//{
//
//}
//
//void gem::hw::glib::HwGLIB::stopDevice()
//{
//
//}
//
//void gem::hw::glib::HwGLIB::resumeDevice()
//{
//
//}
//
//void gem::hw::glib::HwGLIB::haltDevice()
//{
//
//}
//

bool gem::hw::glib::HwGLIB::isHwConnected() 
{
  if ( b_is_connected ) {
    INFO("basic check: HwGLIB connection good");
    return true;
  } else if (gem::hw::GEMHwDevice::isHwConnected()) {
    std::vector<linkStatus> tmp_activeLinks;
    tmp_activeLinks.reserve(N_GTX);
    for (unsigned int link = 0; link < N_GTX; ++link) {
      //need to make sure that this works only for "valid" FW results
      // for the moment we can do a check to see that 2015 appears in the string
      // this no longer will work as desired, how to get whether the GTX is active?
      if ((this->getFirmwareVer()).rfind("5.") != std::string::npos) {
        b_links[link] = true;
        INFO("link" << link << " present(" << this->getFirmwareVer() << ")");
        tmp_activeLinks.push_back(std::make_pair(link,this->LinkStatus(link)));
      } else {
        b_links[link] = false;
        INFO("link" << link << " not reachable (unable to find 5 in the firmware string)"
             << " user firmware version " << this->getFirmwareVer());
      }
    }
    v_activeLinks = tmp_activeLinks;
    if (!v_activeLinks.empty()) {
      b_is_connected = true;
      m_controlLink = (v_activeLinks.begin())->first;
      INFO("connected - control link" << (int)m_controlLink);
      INFO("checked links: HwGLIB connection good");
      return true;
    } else {
      b_is_connected = false;
      INFO("not connected - control link" << (int)m_controlLink);
      return false;
    }
  } else if (m_controlLink < 0)
    return false;
  else
    return false;
}

std::string gem::hw::glib::HwGLIB::getBoardID()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The board ID consists of four characters encoded as a 32-bit unsigned int
  std::string res = "???";
  uint32_t val = readReg(getDeviceBaseNode(),"SYSTEM.BOARD_ID");
  res = uint32ToString(val);
  return res;
}

std::string gem::hw::glib::HwGLIB::getSystemID()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The system ID consists of four characters encoded as a 32-bit unsigned int
  std::string res = "???";
  uint32_t val = readReg(getDeviceBaseNode(),"SYSTEM.SYSTEM_ID");
  res = uint32ToString(val);
  return res;
}

std::string gem::hw::glib::HwGLIB::getIPAddress()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::string res = "N/A";
  uint32_t val = readReg(getDeviceBaseNode(),"SYSTEM.IP_INFO");
  res = uint32ToDottedQuad(val);
  return res;
}

std::string gem::hw::glib::HwGLIB::getMACAddress()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::string res = "N/A";
  uint32_t val1 = readReg(getDeviceBaseNode(),"SYSTEM.MAC.UPPER");
  uint32_t val2 = readReg(getDeviceBaseNode(),"SYSTEM.MAC.LOWER");
  res = uint32ToGroupedHex(val1,val2);
  return res;
}

std::string gem::hw::glib::HwGLIB::getFirmwareDate()
{
  // This returns the firmware build date. 
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream res;
  std::stringstream regName;
  /*
    uint32_t yy = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.YY");
    uint32_t mm = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.MM");
    uint32_t dd = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.DD");
    res << "20" << std::setfill('0') << std::setw(2) << yy
    << "-"      << std::setw(2) << mm
    << "-"      << std::setw(2) << dd;
  */
  uint32_t fwid = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE");
  res << "20" << std::setfill('0') << std::setw(2) << (fwid&0x1f)
      << "-"  << std::setw(2) << ((fwid>>5)&0x0f)
      << "-"  << std::setw(2) << ((fwid>>9)&0x7f);
  return res.str();
}

std::string gem::hw::glib::HwGLIB::getFirmwareVer()
{
  // This returns the firmware version number. 
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream res;
  std::stringstream regName;
  /*
    uint32_t versionMajor = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.MAJOR");
    uint32_t versionMinor = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.MINOR");
    uint32_t versionBuild = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.BUILD");
    res << versionMajor << "." << versionMinor << "." << versionBuild;
  */

  uint32_t fwid = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE");
  res << ((fwid>>28)&0x0f) << "." 
      << ((fwid>>24)&0x0f) << "."
      << ((fwid>>16)&0xff);
  return res.str();
}

void gem::hw::glib::HwGLIB::XPointControl(bool xpoint2, uint8_t const& input, uint8_t const& output)
{
  if (xpoint2 && (input > 2 || output > 0)) {
    std::string msg = toolbox::toString("Invalid clock routing for XPoint2 %d -> %d",input,output);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidXPoint2Routing,msg);
    return;
  }
  
  if ((input > 3 || output > 3)) {
    std::string msg = toolbox::toString( "Invalid clock routing for XPoint%d %d -> %d",xpoint2,input,output);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidXPointRouting,msg);
    return;
  }
  
  std::stringstream regName;
  if (xpoint2)
    regName << "SYSTEM.CLK_CTRL.XPOINT2";
  else
    regName << "SYSTEM.CLK_CTRL.XPOINT1";
  
  switch(output) {
  case (0) :
    regName << ".S1";
  case (1) :
    regName << ".S2";
  case (2) :
    regName << ".S3";
  case (3) :
    regName << ".S4";
  }
  //input = b7b6b5b4b3b2b1b0 and all that matter are b1 and b0 -> 1 and 0 of, eg., S1
  // input == 0 -> b1b0 == 00
  // input == 1 -> b1b0 == 01
  // input == 2 -> b1b0 == 10
  // input == 3 -> b1b0 == 11
  // but the xpoint switch inverts b0 and b1 when routing outputs
  // thus to select input 3 for output 1, one sets S10=1 and S11=0
  writeReg(getDeviceBaseNode(),regName.str()+"1",input&0x01);
  writeReg(getDeviceBaseNode(),regName.str()+"0",(input&0x10)>>1);
}

uint8_t gem::hw::glib::HwGLIB::XPointControl(bool xpoint2, uint8_t const& output)
{
  /*
    if (xpoint2 && output > 0) {
    std::string msg = toolbox::toString("Invalid clock output for XPoint2 %d",output);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidXPoint2Routing,msg);
    return output;
    }
  */
  
  if (output > 3) {
    std::string msg = toolbox::toString( "Invalid clock output for XPoint%d %d",xpoint2,output);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidXPointRouting,msg);
    return output;
  }
  
  std::stringstream regName;
  if (xpoint2)
    regName << "SYSTEM.CLK_CTRL.XPOINT2";
  else
    regName << "SYSTEM.CLK_CTRL.XPOINT1";
  
  switch(output) {
  case (0) :
    regName << ".S1";
  case (1) :
    regName << ".S2";
  case (2) :
    regName << ".S3";
  case (3) :
    regName << ".S4";
  }
  uint8_t input = 0x0;
  input |= (readReg(getDeviceBaseNode(),regName.str()+"0")&0x1)<<1;
  //input = input << 1;
  input |= (readReg(getDeviceBaseNode(),regName.str()+"1")&0x1);
  return input;
}

uint8_t gem::hw::glib::HwGLIB::SFPStatus(uint8_t const& sfpcage)
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.SFP" << (int)sfpcage << ".STATUS";
  return (uint8_t)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::FMCPresence(bool fmc2)
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.FMC" << (int)fmc2 << "_PRESENT";
  return (bool)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::GbEInterrupt()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.GBE_INT";
  return (bool)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::FPGAResetStatus()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.FPGA_RESET";
  return (bool)readReg(getDeviceBaseNode(),regName.str());
}

uint8_t gem::hw::glib::HwGLIB::V6CPLDStatus()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.V6_CPLD";
  return (uint8_t)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::CDCELockStatus()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.CDCE_LOCK";
  return static_cast<bool>(readReg(getDeviceBaseNode(),regName.str()));
}

/** User core functionality **/
uint32_t gem::hw::glib::HwGLIB::getUserFirmware()
{
  // This returns the user firmware build date. 
  return getUserFirmware(m_controlLink);
}

uint32_t gem::hw::glib::HwGLIB::getUserFirmware(uint8_t const& link)
{
  // This returns the user firmware build date. 
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "GLIB_LINKS.LINK" << (int)link << ".USER_FW";
  uint32_t userfw = readReg(getDeviceBaseNode(),regName.str());
  DEBUG("GLIB link" << (int)link << " has firmware version 0x" 
        << std::hex << userfw << std::dec << std::endl);
  return userfw;
}

std::string gem::hw::glib::HwGLIB::getUserFirmwareDate()
{
  // This returns the user firmware build date. 
  std::stringstream res;
  res << "0x"<< std::hex << getUserFirmware(m_controlLink) << std::dec;
  return res.str();
}

std::string gem::hw::glib::HwGLIB::getUserFirmwareDate(uint8_t const& link)
{
  // This returns the user firmware build date. 
  std::stringstream res;
  res << "0x"<< std::hex << getUserFirmware(link) << std::dec;
  return res.str();
}

bool gem::hw::glib::HwGLIB::linkCheck(uint8_t const& link, std::string const& opMsg)
{
  if (link > N_GTX) {
    std::string msg = toolbox::toString("%s requested for link (%d): outside expectation (0-%d)",
                                        opMsg.c_str(), link, N_GTX);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidLink,msg);
    return false;
  } else if (!b_links[link]) {
    std::string msg = toolbox::toString("%s requested inactive link (%d)",opMsg.c_str(), link);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::optohybrid::exception::InvalidLink,msg);
    return false;
  }
  return true;
}

gem::hw::GEMHwDevice::OpticalLinkStatus gem::hw::glib::HwGLIB::LinkStatus(uint8_t const& link)
{
  gem::hw::GEMHwDevice::OpticalLinkStatus linkStatus;

  if (linkCheck(link, "Link status")) {
    linkStatus.TRK_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.TRK_ERR",link));
    linkStatus.TRG_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.TRG_ERR",link));
    linkStatus.Data_Packets = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.DATA_Packets",link));
  }
  return linkStatus;
}

void gem::hw::glib::HwGLIB::LinkReset(uint8_t const& link, uint8_t const& resets)
{

  //right now this just resets the counters, but we need to be able to "reset" the link too
  if (linkCheck(link, "Link reset")) {
    if (resets&0x1)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.TRK_ERR.Reset",link),0x1);
    if (resets&0x2)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.TRG_ERR.Reset",link),0x1);
    if (resets&0x4)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.DATA_Packets.Reset",link),0x1);
  }
}


gem::hw::glib::HwGLIB::GLIBIPBusCounters gem::hw::glib::HwGLIB::getIPBusCounters(uint8_t const& link,
                                                                                 uint8_t const& mode)
{
  
  if (linkCheck(link, "IPBus counter")) {
    if (mode&0x01)
      m_ipBusCounters.at(link).OptoHybridStrobe = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.OptoHybrid_%d",link));
    if (mode&0x02)
      m_ipBusCounters.at(link).OptoHybridAck    = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.OptoHybrid_%d",link));
    if (mode&0x04)
      m_ipBusCounters.at(link).TrackingStrobe   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.OptoHybrid_%d",link));
    if (mode&0x08)
      m_ipBusCounters.at(link).TrackingAck      = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.OptoHybrid_%d",link));
    if (mode&0x10)
      m_ipBusCounters.at(link).CounterStrobe    = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.Counters"));
    if (mode&0x20)
      m_ipBusCounters.at(link).CounterAck       = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.Counters"));
  }
  return m_ipBusCounters.at(link);
}

void gem::hw::glib::HwGLIB::resetIPBusCounters(uint8_t const& link, uint8_t const& resets)
{  
  if (linkCheck(link, "Reset IPBus counters")) {
    if (resets&0x01)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.OptoHybrid_%d.Reset",link),0x1);
    if (resets&0x02)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.OptoHybrid_%d.Reset",link),0x1);
    if (resets&0x04)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.TRK_%d.Reset",link),0x1);
    if (resets&0x08)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.TRK_%d.Reset",link),0x1);
    if (resets&0x10)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.Counters.Reset"),0x1);
    if (resets&0x20)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.Counters.Reset"),0x1);
  }
}

uint32_t gem::hw::glib::HwGLIB::readTriggerFIFO(uint8_t const& link)
{
  std::stringstream regName;
  regName << "GLIB_LINKS.TRG_DATA";
  uint32_t trgword = readReg(getDeviceBaseNode(),regName.str()+".DATA");
  return trgword;
}

void gem::hw::glib::HwGLIB::flushTriggerFIFO(uint8_t const& link)
{
  std::stringstream regName;
  regName << "GLIB_LINKS.LINK" << (int)link << ".TRIGGER";
  writeReg(getDeviceBaseNode(),regName.str()+".FIFO_FLUSH",0x1);
}

uint32_t gem::hw::glib::HwGLIB::getFIFOOccupancy(uint8_t const& link)
{
  uint32_t fifocc = 0;
  if (linkCheck(link, "FIFO occupancy")) {
    std::stringstream regName;
    regName << "TRK_DATA.OptoHybrid_" << (int)link;
    fifocc = readReg(getDeviceBaseNode(),regName.str()+".DEPTH");
    DEBUG(toolbox::toString("getFIFOOccupancy(%d) %s.%s%s:: %d", link, getDeviceBaseNode().c_str(),
                            regName.str().c_str(), ".DEPTH", fifocc));
  }
  //the fifo occupancy is in number of 32 bit words
  return fifocc/7;
}

bool gem::hw::glib::HwGLIB::hasTrackingData(uint8_t const& link)
{
  bool hasData = false;
  if (linkCheck(link, "Tracking data")) {
    std::stringstream regName;
    regName << "TRK_DATA.OptoHybrid_" << (int)link << ".ISEMPTY";
    hasData = !readReg(getDeviceBaseNode(),regName.str());
  }
  return hasData;
}

std::vector<uint32_t> gem::hw::glib::HwGLIB::getTrackingData(uint8_t const& link)
{
  if (!linkCheck(link, "Tracking data")) {
    std::vector<uint32_t> data(7,0x0);
    return data;
  } 
  
  std::stringstream regName;
  regName << getDeviceBaseNode() << "TRK_DATA.OptoHybrid_" << (int)link << ".FIFO";
  //return single VFAT block 7x32 bits
  return readFIFO(regName.str(),7);
  //return readBlock(regName.str(),7);
}

void gem::hw::glib::HwGLIB::flushFIFO(uint8_t const& link)
{
  if (linkCheck(link, "Flush FIFO")) {
    std::stringstream regName;
    regName << "TRK_DATA.OptoHybrid_" << (int)link;
    writeReg(getDeviceBaseNode(),regName.str()+".FLUSH",0x1);
  }
}
