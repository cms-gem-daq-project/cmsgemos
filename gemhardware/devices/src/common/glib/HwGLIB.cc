#include <iomanip>

#include "gem/hw/devices/glib/HwGLIB.h"

#include "gem/hw/devices/exception/Exception.h"

gem::hw::glib::HwGLIB::HwGLIB(std::string const& glibDevice,
                              std::string const& connectionFile) :
  gem::hw::HwGenericAMC::HwGenericAMC(glibDevice, connectionFile)
{
  CMSGEMOS_INFO("HwGLIB::HwGLIB trying to create HwGLIB(" << glibDevice << "," << connectionFile);

  // this->setDeviceBaseNode("GEM_AMC");
  this->setExpectedBoardID("GLIB");

  CMSGEMOS_INFO("HwGLIB::HwGLIB ctor done " << isHwConnected());
}

gem::hw::glib::HwGLIB::HwGLIB(std::string const& glibDevice,
                              std::string const& connectionURI,
                              std::string const& addressTable) :
  gem::hw::HwGenericAMC::HwGenericAMC(glibDevice, connectionURI, addressTable)

{
  CMSGEMOS_INFO("HwGLIB::HwGLIB trying to create HwGLIB(" << glibDevice << "," << connectionURI << "," <<addressTable);

  // this->setDeviceBaseNode("GEM_AMC");
  this->setExpectedBoardID("GLIB");

  CMSGEMOS_INFO("HwGLIB::HwGLIB ctor done " << isHwConnected());
}

gem::hw::glib::HwGLIB::HwGLIB(std::string const& glibDevice,
                              uhal::HwInterface& uhalDevice) :
  gem::hw::HwGenericAMC::HwGenericAMC(glibDevice,uhalDevice)
{
  CMSGEMOS_INFO("HwGLIB::HwGLIB ctor");

  // this->setDeviceBaseNode("GEM_AMC");
  this->setExpectedBoardID("GLIB");

  CMSGEMOS_INFO("HwGLIB::HwGLIB ctor done " << isHwConnected());
}

// gem::hw::glib::HwGLIB::HwGLIB(const int& crate, const int& slot) :
//   gem::hw::HwGenericAMC(toolbox::toString("gem.shelf%02d.glib%02d",crate,slot),crate,slot)
//   // monGLIB_(0),
// {
//   CMSGEMOS_INFO("HwGLIB ctor");

//   // use a connection file and connection manager?
//   setDeviceID(toolbox::toString("gem.shelf%02d.glib%02d",crate,slot));

//   // uhal::ConnectionManager manager ( "file://${GEM_ADDRESS_TABLE_PATH}/connections.xml" );
//   CMSGEMOS_INFO("getting the ConnectionManager pointer");
//   p_gemConnectionManager.reset(new uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections.xml"));
//   CMSGEMOS_INFO("getting HwInterface " << getDeviceID() << " pointer from ConnectionManager");
//   p_gemHW.reset(new uhal::HwInterface(p_gemConnectionManager->getDevice(this->getDeviceID())));
//   CMSGEMOS_INFO("setting the device base node");
//   setDeviceBaseNode("GEM_AMC");
//   setExpectedBoardID("GLIB");

//   // setLogLevelTo(uhal::Error());  // Minimise uHAL logging

//   for (unsigned li = 0; li < N_GTX; ++li) {
//     AMCIPBusCounters tmpGTXCounter;
//     m_ipBusCounters.push_back(tmpGTXCounter);
//   }

//   CMSGEMOS_INFO("HwGLIB ctor done " << isHwConnected());
// }

gem::hw::glib::HwGLIB::~HwGLIB()
{
}

std::string gem::hw::glib::HwGLIB::getBoardIDString()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The board ID consists of four characters encoded as a 32-bit unsigned int
  std::string res = "N/A";
  res = gem::utils::uint32ToString(getBoardID());
  return res;
}

uint32_t gem::hw::glib::HwGLIB::getBoardID()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The board ID consists of four characters encoded as a 32-bit unsigned int
  uint32_t val = readReg(getDeviceBaseNode(), "GLIB_SYSTEM.SYSTEM.BOARD_ID");
  return val;
}

std::string gem::hw::glib::HwGLIB::getSystemIDString()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The system ID consists of four characters encoded as a 32-bit unsigned int
  std::string res = "N/A";
  res = gem::utils::uint32ToString(getSystemID());
  return res;
}

uint32_t gem::hw::glib::HwGLIB::getSystemID()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The system ID consists of four characters encoded as a 32-bit unsigned int
  uint32_t val = readReg(getDeviceBaseNode(), "GLIB_SYSTEM.SYSTEM.SYSTEM_ID");
  return val;
}

std::string gem::hw::glib::HwGLIB::getIPAddressString()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::string res = "N/A";
  res = gem::utils::uint32ToDottedQuad(getIPAddress());
  return res;
}

uint32_t gem::hw::glib::HwGLIB::getIPAddress()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  uint32_t val = readReg(getDeviceBaseNode(), "GLIB_SYSTEM.SYSTEM.IP_INFO");
  return val;
}

std::string gem::hw::glib::HwGLIB::getMACAddressString()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::string res = "N/A";
  uint64_t val = getMACAddress();
  res = gem::utils::uint32ToGroupedHex((val>>32)&0xffffffff,val&0xffffffff);
  // res = gem::utils::uint64ToGroupedHex(val);
  return res;
}

uint64_t gem::hw::glib::HwGLIB::getMACAddress()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  uint32_t val1 = readReg(getDeviceBaseNode(), "GLIB_SYSTEM.SYSTEM.MAC.UPPER");
  uint32_t val2 = readReg(getDeviceBaseNode(), "GLIB_SYSTEM.SYSTEM.MAC.LOWER");
  return ((uint64_t)val1 << 32) + val2;
}

void gem::hw::glib::HwGLIB::XPointControl(bool xpoint2, uint8_t const& input, uint8_t const& output)
{
  if (xpoint2 && (input > 2 || output > 0)) {
    std::string msg = toolbox::toString("Invalid clock routing for XPoint2 %d -> %d",input,output);
    CMSGEMOS_ERROR(msg);
    // XCEPT_RAISE(gem::hw::devices::exception::InvalidXPoint2Routing,msg);
    return;
  }

  if ((input > 3 || output > 3)) {
    std::string msg = toolbox::toString( "Invalid clock routing for XPoint%d %d -> %d",xpoint2,input,output);
    CMSGEMOS_ERROR(msg);
    // XCEPT_RAISE(gem::hw::devices::exception::InvalidXPointRouting,msg);
    return;
  }

  std::stringstream regName;
  if (xpoint2)
    regName << "GLIB_SYSTEM.SYSTEM.CLK_CTRL.XPOINT2";
  else
    regName << "GLIB_SYSTEM.SYSTEM.CLK_CTRL.XPOINT1";

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
  // input = b7b6b5b4b3b2b1b0 and all that matter are b1 and b0 -> 1 and 0 of, eg., S1
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
    CMSGEMOS_ERROR(msg);
    // XCEPT_RAISE(gem::hw::devices::exception::InvalidXPoint2Routing,msg);
    return output;
    }
  */

  if (output > 3) {
    std::string msg = toolbox::toString( "Invalid clock output for XPoint%d %d",xpoint2,output);
    CMSGEMOS_ERROR(msg);
    // XCEPT_RAISE(gem::hw::devices::exception::InvalidXPointRouting,msg);
    return output;
  }

  std::stringstream regName;
  if (xpoint2)
    regName << "GLIB_SYSTEM.SYSTEM.CLK_CTRL.XPOINT2";
  else
    regName << "GLIB_SYSTEM.SYSTEM.CLK_CTRL.XPOINT1";

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
  // input = input << 1;
  input |= (readReg(getDeviceBaseNode(),regName.str()+"1")&0x1);
  return input;
}

uint8_t gem::hw::glib::HwGLIB::SFPStatus(uint8_t const& sfpcage)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  if (sfpcage < 1 || sfpcage > 4) {
    std::string msg = toolbox::toString("Status requested for SFP (%d): outside expectation (1,4)", sfpcage);
    CMSGEMOS_ERROR(msg);
    // XCEPT_RAISE(gem::hw::devices::exception::InvalidLink,msg);
    return 0;
  }
  std::stringstream regName;
  regName << "GLIB_SYSTEM.SYSTEM.STATUS.SFP" << (int)sfpcage << ".STATUS";
  return (uint8_t)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::FMCPresence(bool fmc2)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "GLIB_SYSTEM.SYSTEM.STATUS.FMC" << (int)fmc2+1 << "_PRESENT";
  return (bool)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::GbEInterrupt()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "GLIB_SYSTEM.SYSTEM.STATUS.GBE_INT";
  return (bool)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::FPGAResetStatus()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "GLIB_SYSTEM.SYSTEM.STATUS.FPGA_RESET";
  return (bool)readReg(getDeviceBaseNode(),regName.str());
}

uint8_t gem::hw::glib::HwGLIB::V6CPLDStatus()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "GLIB_SYSTEM.SYSTEM.STATUS.V6_CPLD";
  return (uint8_t)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::CDCELockStatus()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "GLIB_SYSTEM.SYSTEM.STATUS.CDCE_LOCK";
  return static_cast<bool>(readReg(getDeviceBaseNode(),regName.str()));
}

/** only for compatibility with some functions, will be removed **/
uint32_t gem::hw::glib::HwGLIB::getFIFOOccupancy(uint8_t const& gtx)
{
  // FIXME OBSOLETE!!!
  return 0x0;
}

uint32_t gem::hw::glib::HwGLIB::getFIFOVFATBlockOccupancy(uint8_t const& gtx)
{
  // FIXME OBSOLETE!!!
  return 0x0;
}

bool gem::hw::glib::HwGLIB::hasTrackingData(uint8_t const& gtx)
{
  // FIXME OBSOLETE!!!
  return false;
}

std::vector<uint32_t> gem::hw::glib::HwGLIB::getTrackingData(uint8_t const& gtx, size_t const& nBlocks)
{
  // FIXME OBSOLETE!!!
  std::vector<uint32_t> res;
  return res;
}

uint32_t gem::hw::glib::HwGLIB::getTrackingData(uint8_t const& gtx, uint32_t* data, size_t const& nBlocks)
{
  // FIXME OBSOLETE!!!
  return 0x0;
}

// uint32_t gem::hw::glib::HwGLIB::getTrackingData(uint8_t const& gtx, std::vector<toolbox::mem::Reference*>& data,
//                                                 size_t const& nBlocks)
// {
//   // FIXME OBSOLETE!!!
//   return 0x0;
// }

void gem::hw::glib::HwGLIB::flushFIFO(uint8_t const& gtx)
{
  // FIXME OBSOLETE!!!
  return;
}
