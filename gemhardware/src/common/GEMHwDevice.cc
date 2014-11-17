//General structure taken blatantly from tcds::utils::HwDeviceTCA/HwGLIB as we're using the same card

#include "gem/hw/GEMHwDevice.h"

#define DEBUG(MSG) LOG4CPLUS_DEBUG(logGEMHw_ , MSG)
#define INFO( MSG) LOG4CPLUS_INFO( logGEMHw_ , MSG)
#define WARN( MSG) LOG4CPLUS_WARN( logGEMHw_ , MSG)
#define ERROR(MSG) LOG4CPLUS_ERROR(logGEMHw_ , MSG)
#define FATAL(MSG) LOG4CPLUS_FATAL(logGEMHw_ , MSG)

gem::hw::GEMHwDevice::GEMHwDevice(xdaq::Application* gemApp):
  logGEMHw_(gemApp->getApplicationLogger()),
  gemHWP_(0)
  //monGEMHw_(0)

{
  //need to grab these parameters from the xml file or from some configuration space/file/db
  setAddressTableFileName("allregsnonfram.xml");
  setIPbusProtocolVersion("2.0");
  setDeviceBaseNode("");
  setDeviceID("GEMHwDevice");

  ipBusErrs.badHeader_     = 0;
  ipBusErrs.readError_     = 0;
  ipBusErrs.timeouts_      = 0;
  ipBusErrs.controlHubErr_ = 0;

  setLogLevelTo(uhal::Error());  // Minimise uHAL logging
  //gem::hw::GEMHwDevice::initDevice();
  /** 
   * what's the difference between connect, init, enable for GLIB, VFAT, other devices?
   * are all options necessary?
   * steps from nothing to running:
   * initDevice:
   * check that register values are hardware default values, if not, something may be amiss
   * enableDevice:
   * set register values to sw default values -> hardware is enabled!
   * configureDevice:
   * set register values to desired values -> hardware is configured!
   * startDevice:
   * set run bit -> hardware is running

   * in this model, a device can be running while the C++ object no longer exists
   * is this a good thing?  one can always at a later time connect again and turn the device off
   * however, if we define the sequences as init->enable->configure->start
   * then it will be non-trivial to connect to a running chip and set enable to off without
   * repeating the steps...
   **/
}

gem::hw::GEMHwDevice::~GEMHwDevice()
{
  releaseDevice();
}

std::string gem::hw::GEMHwDevice::printErrorCounts() {
  std::stringstream errstream;
  errstream << "errors while accessing registers:"    << std::endl 
	    << "Bad header:  " <<ipBusErrs.badHeader_ << std::endl
	    << "Read errors: " <<ipBusErrs.readError_ << std::endl
	    << "Timeouts:    " <<ipBusErrs.timeouts_  << std::endl
	    << "Controlhub errors: " <<ipBusErrs.controlHubErr_ << std::endl;
  INFO(errstream);
  return errstream.str();
}

void gem::hw::GEMHwDevice::connectDevice()
{
  //std::string const addressTable      = "allregsnonfram.xml";    //cfgInfoSpaceP_->getString("addressTable");
  std::string const controlhubAddress = "localhost";    //cfgInfoSpaceP_->getString("controlhubAddress");
  std::string const deviceAddress     = "192.168.0.115";//cfgInfoSpaceP_->getString("deviceAddress");
  uint32_t    const controlhubPort    = 10203;          //cfgInfoSpaceP_->getUInt32("controlhubPort");
  uint32_t    const ipbusPort         = 50001;          //cfgInfoSpaceP_->getUInt32("ipbusPort");
  
  std::stringstream tmpUri;
  if (controlhubAddress.size() > 0)
    {
      DEBUG("Using control hub at address '" << controlhubAddress
	   << ", port number " << controlhubPort << "'.");
      tmpUri << "chtcp-"<< getIPbusProtocolVersion() << "://" << controlhubAddress << ":" << controlhubPort
             << "?target=" << deviceAddress << ":" << ipbusPort;
    }
  else
    {
      DEBUG("No control hub address specified -> "
	   "continuing with a direct connection.");
      tmpUri << "ipbusudp-" << getIPbusProtocolVersion() << "://"
             << deviceAddress << ":" << ipbusPort;
    }
  std::string const uri = tmpUri.str();
  std::string const id  = getDeviceID();
  std::string const addressTable = getAddressTableFileName();
  
  //int retryCount = 0;
  
  uhal::HwInterface* tmpHWP = 0;
  setLogLevelTo(uhal::Error());  // Minimise uHAL logging

  try {
    tmpHWP = new uhal::HwInterface(uhal::ConnectionManager::getDevice(id, uri, addressTable));
  }
  catch (uhal::exception::FileNotFound const& err)
    {
      std::string msg =
        toolbox::toString("Could not find uhal address table file '%s' "
                          "(or one of its included address table modules).",
                          addressTable.c_str());
      FATAL(msg);
      XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  catch (uhal::exception::exception const& err)
    {
      std::string msgBase =
        "Could not obtain the uhal device from the connection manager";
      std::string msg = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      FATAL(msg);
      XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  catch (std::exception const& err)
    {
      ERROR("Unknown std::exception caught from uhal");
      std::string msgBase = "Could not connect to th e hardware";
      std::string msg = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      FATAL(msg);
      XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  
  gemHWP_ = tmpHWP;
  
  INFO("Successfully connected to the hardware.");

}

void gem::hw::GEMHwDevice::configureDevice()
{
  
}

void gem::hw::GEMHwDevice::releaseDevice()
{
  if (gemHWP_ != 0) {
    delete gemHWP_;
    gemHWP_ = 0;
  }
}

void gem::hw::GEMHwDevice::enableDevice()
{
  if (!isGEMHwDeviceConnected()){
    std::string msg = "Could not enable the hardware. " \
      "(No hardware is connected.)";
    FATAL(msg);
    XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
  }
}

//void gem::hw::GEMHwDevice::disableDevice()
//{
//
//}
//
//void gem::hw::GEMHwDevice::pauseDevice()
//{
//
//}
//
//void gem::hw::GEMHwDevice::startDevice()
//{
//
//}
//
//void gem::hw::GEMHwDevice::stopDevice()
//{
//
//}
//
//void gem::hw::GEMHwDevice::resumeDevice()
//{
//
//}
//
//void gem::hw::GEMHwDevice::haltDevice()
//{
//
//}

/**
   void gem::hw::GEMHwDevice::initDevice() 
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
**/

uhal::HwInterface& gem::hw::GEMHwDevice::getGEMHwInterface()// const
{
  if (gemHWP_ == 0)
    {
      std::string msg = "Trying to access hardware before connecting.";
      ERROR(msg);
      XCEPT_RAISE(gem::hw::exception::SoftwareProblem, msg);
    }
  else
    {
      uhal::HwInterface& hw = static_cast<uhal::HwInterface&>(*gemHWP_);
      return hw;
    }
}

std::string gem::hw::GEMHwDevice::getBoardID()
{
  //LockGuard<Lock> guardedLock(lock_);
  // The board ID consists of four characters encoded as a 32-bit
  // something.
  std::string res = "???";
  uint32_t val = readReg("glib_regs.sysregs.board_id");
  res = uint32ToString(val);
  return res;
}

std::string gem::hw::GEMHwDevice::getSystemID()
{
  //LockGuard<Lock> guardedLock(lock_);
  std::string res = "???";
  uint32_t val = readReg("glib_regs.sysregs.system_id");
  res = uint32ToString(val);
  return res;
}

std::string gem::hw::GEMHwDevice::getFirmwareDate(std::string const& regNamePrefix)
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

std::string gem::hw::GEMHwDevice::getFirmwareVer(std::string const& regNamePrefix)
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

uint32_t gem::hw::GEMHwDevice::readReg(std::string const& name)
{
  //LockGuard<Lock> guardedLock(lock_);
  uhal::HwInterface& hw = getGEMHwInterface();

  int retryCount = 0;
  uint32_t res;
  while (retryCount < MAX_VFAT_RETRIES) {
    try {
      uhal::ValWord<uint32_t> val = hw.getNode(name).read();
      hw.dispatch();
      res = val.value();
      break;
    }
    catch (uhal::exception::exception const& err) {
      std::string msgBase = toolbox::toString("Could not read register '%s' (uHAL)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if ((errCode.find("amount of data")              != std::string::npos) ||
	  (errCode.find("INFO CODE = 0x4L")            != std::string::npos) ||
	  (errCode.find("INFO CODE = 0x6L")            != std::string::npos) ||
	  (errCode.find("timed out")                   != std::string::npos) ||
	  (errCode.find("had response field = 0x04")   != std::string::npos) ||
	  (errCode.find("ControlHub error code is: 4") != std::string::npos)) {
	++retryCount;
	INFO("Failed to read register " << name <<
	     ", retrying. retryCount("<<retryCount<<")"
	     << std::endl);
	gem::hw::GEMHwDevice::updateErrorCounters(errCode);
	continue;
      }
      else {
	FATAL(msg);
	XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    }
    catch (std::exception const& err) {
      std::string msgBase = toolbox::toString("Could not read register '%s' (std)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      FATAL(msg);
      XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
  return res;
}

void gem::hw::GEMHwDevice::readRegs(std::vector<std::pair<std::string, uint32_t> > &regList)
{
  //LockGuard<Lock> guardedLock(lock_);
  uhal::HwInterface& hw = getGEMHwInterface();

  int retryCount = 0;
  while (retryCount < MAX_VFAT_RETRIES) {
    try {
      std::vector<std::pair<std::string, uint32_t> >::iterator curReg = regList.begin();
      std::vector<std::pair<std::string,uhal::ValWord<uint32_t> > > vals;
      //vals.reserve(regList.size());
      for (; curReg != regList.end(); ++curReg) 
	vals.push_back(std::make_pair(curReg->first,hw.getNode(curReg->first).read()));
      hw.dispatch();

      std::vector<std::pair<std::string, uhal::ValWord<uint32_t> > >::const_iterator curVal = vals.begin();
      curReg = regList.begin();
      for (; curReg != regList.end(); ++curReg,++curVal) 
	curReg->second = (curVal->second).value();
      break;
    }
    catch (uhal::exception::exception const& err) {
      std::string msgBase = "Could not read from register in list:";
      std::vector<std::pair<std::string, uint32_t> >::const_iterator curReg = regList.begin();
      for (; curReg != regList.end(); ++curReg) 
	msgBase += toolbox::toString(" '%s'", curReg->first.c_str());
      std::string msg     = toolbox::toString("%s (uHAL): %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if ((errCode.find("amount of data")              != std::string::npos) ||
	  (errCode.find("INFO CODE = 0x4L")            != std::string::npos) ||
	  (errCode.find("INFO CODE = 0x6L")            != std::string::npos) ||
	  (errCode.find("timed out")                   != std::string::npos) ||
	  (errCode.find("had response field = 0x04")   != std::string::npos) ||
	  (errCode.find("ControlHub error code is: 4") != std::string::npos)) {
	++retryCount;
	INFO("Failed to read register " << curReg->first <<
	     ", retrying. retryCount("<<retryCount<<")"
	     << std::endl);
	gem::hw::GEMHwDevice::updateErrorCounters(errCode);
	continue;
      }
      else {
	FATAL(msg);
	XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    }
    catch (std::exception const& err) {
      std::string msgBase = "Could not read from register in list:";
      std::vector<std::pair<std::string, uint32_t> >::const_iterator curReg = regList.begin();
      for (; curReg != regList.end(); ++curReg) 
	msgBase += toolbox::toString(" '%s'", curReg->first.c_str());
      std::string msg = toolbox::toString("%s (std): %s.", msgBase.c_str(), err.what());
      FATAL(msg);
      XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
}

void gem::hw::GEMHwDevice::writeReg(std::string const& name,
				    uint32_t const val)
{
  //LockGuard<Lock> guardedLock(lock_);
  uhal::HwInterface& hw = getGEMHwInterface();
  int retryCount = 0;
  while (retryCount < MAX_VFAT_RETRIES) {
    try {
      hw.getNode(name).write(val);
      hw.dispatch();
      break;
    }
    catch (uhal::exception::exception const& err) {
      std::string msgBase = toolbox::toString("Could not write to register '%s' (uHAL)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if ((errCode.find("amount of data")              != std::string::npos) ||
	  (errCode.find("INFO CODE = 0x4L")            != std::string::npos) ||
	  (errCode.find("INFO CODE = 0x6L")            != std::string::npos) ||
	  (errCode.find("timed out")                   != std::string::npos) ||
	  (errCode.find("had response field = 0x04")   != std::string::npos) ||
	  (errCode.find("ControlHub error code is: 4") != std::string::npos)) {
	++retryCount;
	INFO("Failed to write value 0x" << std::hex<< val << std::dec << " to register " << name <<
	     ", retrying. retryCount("<<retryCount<<")"
	     << std::endl);
	gem::hw::GEMHwDevice::updateErrorCounters(errCode);
	continue;
      }
      else {
	FATAL(msg);
	XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    }
    catch (std::exception const& err) {
      std::string msgBase = toolbox::toString("Could not write to register '%s' (std)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      FATAL(msg);
      XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
}

void gem::hw::GEMHwDevice::writeRegs(std::vector<std::pair<std::string, uint32_t> > const& regList)
{
  //LockGuard<Lock> guardedLock(lock_);
  uhal::HwInterface& hw = getGEMHwInterface();
  int retryCount = 0;
  while (retryCount < MAX_VFAT_RETRIES) {
    try {
      std::vector<std::pair<std::string, uint32_t> >::const_iterator curReg = regList.begin();
      for (; curReg != regList.end(); ++curReg) 
	hw.getNode(curReg->first).write(curReg->second);
      hw.dispatch();
      break;
    }
    catch (uhal::exception::exception const& err) {
      std::string msgBase = "Could not write to register in list:";
      std::vector<std::pair<std::string, uint32_t> >::const_iterator curReg = regList.begin();
      for (; curReg != regList.end(); ++curReg) 
	msgBase += toolbox::toString(" '%s'", curReg->first.c_str());
      std::string msg     = toolbox::toString("%s (uHAL): %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if ((errCode.find("amount of data")              != std::string::npos) ||
	  (errCode.find("INFO CODE = 0x4L")            != std::string::npos) ||
	  (errCode.find("INFO CODE = 0x6L")            != std::string::npos) ||
	  (errCode.find("timed out")                   != std::string::npos) ||
	  (errCode.find("had response field = 0x04")   != std::string::npos) ||
	  (errCode.find("ControlHub error code is: 4") != std::string::npos)) {
	++retryCount;
	INFO("Failed to write value 0x" << std::hex <<
	     curReg->second << std::dec <<
	     " to register " << curReg->first <<
	     ", retrying. retryCount("<<retryCount<<")"
	     << std::endl);
	gem::hw::GEMHwDevice::updateErrorCounters(errCode);
	continue;
      }
      else {
	FATAL(msg);
	XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    }
    catch (std::exception const& err) {
      std::string msgBase = "Could not write to register in list:";
      std::vector<std::pair<std::string, uint32_t> >::const_iterator curReg = regList.begin();
      for (; curReg != regList.end(); ++curReg) 
	msgBase += toolbox::toString(" '%s'", curReg->first.c_str());
      std::string msg = toolbox::toString("%s (std): %s.", msgBase.c_str(), err.what());
      FATAL(msg);
      XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
}

void gem::hw::GEMHwDevice::writeValueToRegs(std::vector<std::string> const& regNames, uint32_t const& regValue)
{
  std::vector<std::pair<std::string, uint32_t> > regsToWrite;
  std::vector<std::string>::const_iterator curReg = regNames.begin();
  for (; curReg != regNames.end(); ++curReg)
    regsToWrite.push_back(std::make_pair(*curReg,regValue));
  writeRegs(regsToWrite);
}

void gem::hw::GEMHwDevice::zeroReg(std::string const& name)
{
  writeReg(name,0);
}

void gem::hw::GEMHwDevice::zeroRegs(std::vector<std::string> const& regNames)
{
  std::vector<std::pair<std::string, uint32_t> > regsToZero;
  std::vector<std::string>::const_iterator curReg = regNames.begin();
  for (; curReg != regNames.end(); ++curReg)
    regsToZero.push_back(std::make_pair(*curReg,0x0));
  writeRegs(regsToZero);
}

std::vector<uint32_t> gem::hw::GEMHwDevice::readBlock(std::string const& name)
{
  //LockGuard<Lock> guardedLock(lock_);
  uhal::HwInterface& hw = getGEMHwInterface();
  size_t numWords       = hw.getNode(name).getSize();
  return readBlock(name, numWords);
}

std::vector<uint32_t> gem::hw::GEMHwDevice::readBlock(std::string const& name,
						      size_t const numWords)
{
  //LockGuard<Lock> guardedLock(lock_);
  uhal::HwInterface& hw = getGEMHwInterface();

  std::vector<uint32_t> res(numWords);

  int retryCount = 0;
  if (numWords < 1) {
    return res;
  }

  while (retryCount < MAX_VFAT_RETRIES) {
    try {
      uhal::ValVector<uint32_t> values = hw.getNode(name).readBlock(numWords);
      hw.dispatch();
      std::copy(values.begin(), values.end(), res.begin());
      break;
    }
    catch (uhal::exception::exception const& err) {
      std::string msgBase = toolbox::toString("Could not read block '%s' (uHAL)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if ((errCode.find("amount of data")              != std::string::npos) ||
	  (errCode.find("INFO CODE = 0x4L")            != std::string::npos) ||
	  (errCode.find("INFO CODE = 0x6L")            != std::string::npos) ||
	  (errCode.find("timed out")                   != std::string::npos) ||
	  (errCode.find("had response field = 0x04")   != std::string::npos) ||
	  (errCode.find("ControlHub error code is: 4") != std::string::npos)) {
	++retryCount;
	INFO("Failed to block " << name << " with " << numWords << " words" <<
	     ", retrying. retryCount("<<retryCount<<")"
	     << std::endl);
	gem::hw::GEMHwDevice::updateErrorCounters(errCode);
	continue;
      }
      else {
	FATAL(msg);
	XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    }
    catch (std::exception const& err) {
      std::string msgBase = toolbox::toString("Could not read block '%s' (std)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      FATAL(msg);
      XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
  return res;
}

void gem::hw::GEMHwDevice::writeBlock(std::string const& name,
				      std::vector<uint32_t> const values)
{
  //LockGuard<Lock> guardedLock(lock_);
  if (values.size() < 1) {
    return;
  }
  
  uhal::HwInterface& hw = getGEMHwInterface();
  int retryCount = 0;
  while (retryCount < MAX_VFAT_RETRIES) {
    try {
      hw.getNode(name).writeBlock(values);
      hw.dispatch();
      break;
    }
    catch (uhal::exception::exception const& err) {
      std::string msgBase = toolbox::toString("Could not write to block '%s' (uHAL)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if ((errCode.find("amount of data")              != std::string::npos) ||
	  (errCode.find("INFO CODE = 0x4L")            != std::string::npos) ||
	  (errCode.find("INFO CODE = 0x6L")            != std::string::npos) ||
	  (errCode.find("timed out")                   != std::string::npos) ||
	  (errCode.find("had response field = 0x04")   != std::string::npos) ||
	  (errCode.find("ControlHub error code is: 4") != std::string::npos)) {
	++retryCount;
	INFO("Failed to write block " << name <<
	     ", retrying. retryCount("<<retryCount<<")"
	     << std::endl);
	gem::hw::GEMHwDevice::updateErrorCounters(errCode);
	continue;
      }
      else {
	FATAL(msg);
	XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    }
    catch (std::exception const& err) {
      std::string msgBase = toolbox::toString("Could not write to block '%s' (std)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      FATAL(msg);
      XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
}

void gem::hw::GEMHwDevice::updateErrorCounters(std::string const& errCode) {
  if ((errCode.find("amount of data")    != std::string::npos) ||
      (errCode.find("had response field = 0x04")   != std::string::npos))
    ++ipBusErrs.badHeader_;
  if (errCode.find("INFO CODE = 0x4L")  != std::string::npos)
    ++ipBusErrs.readError_;
  if ((errCode.find("INFO CODE = 0x6L") != std::string::npos) ||
      (errCode.find("timed out")        != std::string::npos))
    ++ipBusErrs.timeouts_;
  if (errCode.find("ControlHub error code is: 4") != std::string::npos)
    ++ipBusErrs.controlHubErr_;
  if (errCode.find("had response field = 0x04") != std::string::npos)
    ++ipBusErrs.controlHubErr_;
}

//void gem::hw::GEMHwDevice::zeroBlock(std::string const& name)
//{
//  //LockGuard<Lock> guardedLock(lock_);
//  uhal::HwInterface& hw = getGEMHwInterface();
//  size_t numWords = hw.getNode(name).getSize();
//  std::vector<uint32_t> zeros(numWords, 0);
//  return writeBlock(name, zeros);
//}

/***********my versions
uint32_t gem::hw::GEMHwDevice::readReg(std::string const& regName)
{
//Maybe want to use a lock to prevent hammering the HW device
  uhal::HwInterface& hw = getGEMHwInterface();
  uint32_t regValue;
  try {
    uhal::ValWord<uint32_t> regVal = hw.getNode(regName).read();
    hw.dispatch();
    regValue = regVal.value();
  }
  catch (uhalException const& e) {
    std::stringstream msgStream;
    msgStream << "Could not read register ";
    msgStream << regName;
    msgStream << ": ";
    msgStream << e.what();
    std::string message = msgStream.str();
    FATAL(message);
    XCEPT_RAISE(gem::hw::exception::HardwareProblem, message);
  }
  catch (std::exception const& e) {
    std::stringstream msgStream;
    msgStream << "Could not read register ";
    msgStream << regName;
    msgStream << ": ";
    msgStream << e.what();
    std::string message = msgStream.str();
    FATAL(message);
    XCEPT_RAISE(gem::hw::exception::HardwareProblem, message);
  }
  return regValue;
}

void gem::hw::vfat::HwVFAT2::writeReg(std::string const& regName
				      uint32_t const writeValue)
{
//Maybe want to use a lock to prevent hammering the HW device
  uhal::HwInterface& hw = getGEMHwInterface();
  try {
    hw.getNode(regName).write(writeValue);
    hw.dispatch();
  }
  catch (uhalException const& e) {
    std::stringstream msgStream;
    msgStream << "Could not write to register ";
    msgStream << regName;
    msgStream << ": ";
    msgStream << e.what();
    std::string message = msgStream.str();
    FATAL(message);
    XCEPT_RAISE(gem::hw::exception::HardwareProblem, message);
  }
  catch (std::exception const& e) {
    std::stringstream msgStream;
    msgStream << "Could not write to register ";
    msgStream << regName;
    msgStream << ": ";
    msgStream << e.what();
    std::string message = msgStream.str();
    FATAL(message);
    XCEPT_RAISE(gem::hw::exception::HardwareProblem, message);
  }
}

void gem::hw::GEMHwDevice::main()
{
  uhal::HwInterface hw=manager->getDevice ( "gemsupervisor.udp.0" );
  //  *hw = manager->getDevice ( "gemsupervisor.udp.0" );

  // print out basic information
  std::cout << "current Value of myParameter_ = " << myParameter_ << std::endl;
  //std::cout << "System ID: " << formatSystemID(systemID_,0) << std::endl;
  //std::cout << "Board ID: "  << formatBoardID(boardID_,0)   << std::endl;
  //std::cout << "System firmware version: " << formatFW(firmwareID_,0) << std::endl;
  //std::cout << "System firmware date: "    << formatFW(firmwareID_,1) << std::endl;
  std::cout << "Current value of test register_ = " << testReg_       << std::endl;
  
  gem::hw::GEMHwDevice::getTestReg();
  std::cout << "Current value of test register_ = " << testReg_       << std::endl;
  gem::hw::GEMHwDevice::setTestReg(32);
  std::cout << "Current value of test register_ = " << testReg_       << std::endl;
}
***/
/***
    void gem::hw::GEMHwDevice::getTestReg()
    //uhal::HwInterface &hw, uhal::ValWord< uint32_t> &mem)
    {
    uhal::HwInterface hw=manager->getDevice ( "gemsupervisor.udp.0" );
    try {
    r_test = hw.getNode ( "test" ).read();
    hw.dispatch();
    testReg_ = r_test.value();
    }
    catch (const std::exception& e) {
    std::cout << "Something went wrong reading the test register: " << e.what() << std::endl;
    }
  
    }
    void gem::hw::GEMHwDevice::setTestReg(uint32_t setVal)
    //uhal::HwInterface &hw, uhal::ValWord< uint32_t> &mem)
    {
    testReg_ = setVal;
  
    uhal::HwInterface hw=manager->getDevice ( "gemsupervisor.udp.0" );
    try {
    hw.getNode ( "test" ).write(testReg_);
    hw.dispatch();
    }
    catch (const std::exception& e) {
    std::cout << "Something went wrong writing the test register: " << e.what() << std::endl;
    }
  
    try {
    r_test = hw.getNode ( "test" ).read();
    hw.dispatch();
    testReg_ = r_test.value();
    }
    catch (const std::exception& e) {
    std::cout << "Something went wrong reading the test register: " << e.what() << std::endl;
    }
    }
***/
