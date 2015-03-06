#include "gem/hw/glib/HwGLIB.h"

gem::hw::glib::HwGLIB::HwGLIB(xdaq::Application* glibApp):
  gem::hw::GEMHwDevice::GEMHwDevice(glibApp)
  //logGLIB_(glibApp->getApplicationLogger()),
  //hwGLIB_(0),
  //monGLIB_(0)
{
  setDeviceID("GLIBHw");
  setAddressTableFileName("optohybrid_address_table.xml");
  setIPbusProtocolVersion("2.0");
  setDeviceBaseNode("GLIB");
  //gem::hw::glib::HwGLIB::initDevice();
}

gem::hw::glib::HwGLIB::~Device()
{
  releaseDevice();
}

void gem::hw::glib::HwGLIB::connectDevice()
{
  std::string const controlhubAddress = cfgInfoSpaceP_->getString("controlhubAddress");
  std::string const device1Address    = cfgInfoSpaceP_->getString("deviceAddress");
  uint32_t const    controlhubPort    = cfgInfoSpaceP_->getUInt32("controlhubPort");
  uint32_t const    ipbusPort         = cfgInfoSpaceP_->getUInt32("ipbusPort");
  
  std::stringstream tmpUri;
  if (controlhubAddress.size() > 0)
    {
      INFO("Using control hub at address '" << controlhubAddress
           << ", port number " << controlhubPort << "'.");
      tmpUri << "chtcp-"
             << getIPbusProtocolVersion()
             << "://"
             << controlhubAddress
             << ":"
             << controlhubPort
             << "?target="
             << deviceAddress
             << ":"
             << ipbusPort;
    }
  else
    {
      INFO("No control hub address specified -> "
           "continuing with a direct connection.");
      tmpUri << "ipbusudp-"
             << getIPbusProtocolVersion()
             << "://"
             << deviceAddress
             << ":"
             << ipbusPort;
    }
  std::string const uri = tmpUri.str();
  std::string const id = "HwGLIB";
  
}

void gem::hw::glib::HwGLIB::releaseDevice()
{

}

void gem::hw::glib::HwGLIB::enableDevice()
{

}

void gem::hw::glib::HwGLIB::disableDevice()
{

}

void gem::hw::glib::HwGLIB::pauseDevice()
{

}

void gem::hw::glib::HwGLIB::startDevice()
{

}

void gem::hw::glib::HwGLIB::stopDevice()
{

}

void gem::hw::glib::HwGLIB::resumeDevice()
{

}

void gem::hw::glib::HwGLIB::haltDevice()
{

}

void gem::hw::glib::HwGLIB::initDevice() 
{
  char * val;
  val = std::getenv( "BUILD_HOME" );
  std::string dirVal = "";
  if (val != NULL) {
    dirVal = val;
  }
  else {
    std::cout<<"$BUILD_HOME not set, exiting"<<std::endl;
    exit(1);
  }
  
  //setLogLevelTo(uhal::Debug());  // Maximise uHAL logging
  setLogLevelTo(uhal::Error());  // Minimise uHAL logging
  char connectionPath[128];
  try {
    sprintf(connectionPath,"file://%s/data/myconnections.xml;",dirVal.c_str());
    manageGLIBConnection = new uhal::ConnectionManager( connectionPath );
  }
  catch (const std::exception& e) {
    std::cout << "Something went wrong initializing the connection: " << e.what() << std::endl;
  }
}


std::string gem::hw::glib:HwGLIB::getBoardID()
{
  //LockGuard<Lock> guardedLock(lock_);
  // The board ID consists of four characters encoded as a 32-bit
  // something.
  std::string res = "???";
  uint32_t val = readReg("glib_regs.sysregs.board_id");
  res = uint32ToString(val);
  return res;
}

std::string gem::hw::glib:HwGLIB::getSystemID()
{
  //LockGuard<Lock> guardedLock(lock_);
  std::string res = "???";
  uint32_t val = readReg("glib_regs.sysregs.system_id");
  res = uint32ToString(val);
  return res;
}

std::string gem::hw::glib:HwGLIB::getFirmwareDate(std::string const& regNamePrefix)
{
  // This returns the firmware build date. If no register name prefix
  // is given, 'glib' is used, and the build date returned is that of
  // the system logic (as opposed to the user logic)..
  //LockGuard<Lock> guardedLock(lock_);
  std::stringstream res;
  std::stringstream regName;
  /**
     regName << regNamePrefix << ".firmware_id.date_yy";
     uint32_t yy = readReg(regName.str());
     regName.str("");
     regName << regNamePrefix << ".firmware_id.date_mm";
     uint32_t mm = readReg(regName.str());
     regName.str("");
     regName << regNamePrefix << ".firmware_id.date_dd";
     uint32_t dd = readReg(regName.str());
     res << "20" << std::setfill('0') << std::setw(2) << yy
     << "-"
     << std::setw(2) << mm
     << "-"
     << std::setw(2) << dd;
  **/
  regName.str("");
  regName << regNamePrefix << ".firmware_id";
  uint32_t fwid = readReg(regName.str());
  res << "20" << std::setfill('0') << std::setw(2) << (fwid&0x1f)
      << "-"
      << std::setw(2) << ((fwid>>5)&0x0f)
      << "-"
      << std::setw(2) << ((fwid>>9)&0x7f);
  return res.str();
}

std::string gem::hw::glib:HwGLIB::getFirmwareVer(std::string const& regNamePrefix)
{
  // This returns the firmware version number. If no register name
  // prefix is given, 'glib' is used, and the version number returned
  // is that of the system logic (as opposed to the user logic)..
  //LockGuard<Lock> guardedLock(lock_);
  std::stringstream res;
  std::stringstream regName;
  /***
      regName << regNamePrefix << ".firmware_id.ver_major";
      uint32_t versionMajor = readReg(regName.str());
      regName.str("");
      regName << regNamePrefix << ".firmware_id.ver_minor";
      uint32_t versionMinor = readReg(regName.str());
      regName.str("");
      regName << regNamePrefix << ".firmware_id.ver_build";
      uint32_t versionBuild = readReg(regName.str());
      res << versionMajor << "." << versionMinor << "." << versionBuild;
  ***/

  regName.str("");
  regName << regNamePrefix << ".firmware_id";
  uint32_t fwid = readReg(regName.str());
  res << ((fwid>>28)&0x0f)
      << "." 
      << ((fwid>>24)&0x0f)
      << "."
      << ((fwid>>16)&0xff);
  return res.str();
}
