/**
 *   General structure taken blatantly from tcds::utils::HwDeviceTCA as we're using the same card
 *   11.2018: structure modified to inherit from both uhal::HwInterface and xhal::client::XHALInterface
 */

#include "gem/hw/devices/GEMHwDevice.h"

#include <boost/algorithm/string.hpp>

gem::hw::GEMHwDevice::GEMHwDevice(std::string const& deviceName,
                                  std::string const& connectionFile) :
  xhal::client::XHALInterface(deviceName.substr(0,deviceName.rfind("-optohybrid"))),
  uhal::HwInterface(std::shared_ptr<uhal::ConnectionManager>(new uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/"+connectionFile))->getDevice(deviceName)),
  // b_is_connected(false),
  m_gemLogger(log4cplus::Logger::getInstance(deviceName)),
  m_hwLock(toolbox::BSem::FULL, true),
  m_crate(-1),
  m_slot(-1)
{
  CMSGEMOS_DEBUG("GEMHwDevice(std::string, std::string) ctor");

  CMSGEMOS_DEBUG("GEMHwDevice::ctor done");
}

gem::hw::GEMHwDevice::GEMHwDevice(std::string const& deviceName,
                                  std::string const& connectionURI,
                                  std::string const& addressTable) :
  xhal::client::XHALInterface(deviceName.substr(0,deviceName.rfind("-optohybrid"))),
  uhal::HwInterface(uhal::ConnectionManager::getDevice(deviceName, connectionURI, addressTable)),
  // b_is_connected(false),
  m_gemLogger(log4cplus::Logger::getInstance(deviceName)),
  m_hwLock(toolbox::BSem::FULL, true),
  m_crate(-1),
  m_slot(-1)
{
  CMSGEMOS_DEBUG("GEMHwDevice(std::string, std::string, std::string) ctor");

  CMSGEMOS_DEBUG("GEMHwDevice::ctor done");
}

gem::hw::GEMHwDevice::GEMHwDevice(std::string const& deviceName,
                                  uhal::HwInterface const& uhalDevice) :
  xhal::client::XHALInterface(deviceName.substr(0,deviceName.rfind("-optohybrid"))),
  uhal::HwInterface(uhalDevice),
  // b_is_connected(false),
  m_gemLogger(log4cplus::Logger::getInstance(deviceName)),
  m_hwLock(toolbox::BSem::FULL, true),
  m_crate(-1),
  m_slot(-1)
{
  CMSGEMOS_DEBUG("GEMHwDevice(std::string, uhal::HwInterface) ctor");

  CMSGEMOS_DEBUG("GEMHwDevice::ctor done");
}

gem::hw::GEMHwDevice::~GEMHwDevice()
{
}

std::string gem::hw::GEMHwDevice::printErrorCounts() const
{
  std::stringstream errstream;
  errstream << "errors while accessing registers:"                << std::endl
            << "Bad header:  "       << m_ipBusErrs.BadHeader     << std::endl
            << "Read errors: "       << m_ipBusErrs.ReadError     << std::endl
            << "Timeouts:    "       << m_ipBusErrs.Timeout       << std::endl
            << "Controlhub errors: " << m_ipBusErrs.ControlHubErr << std::endl;
  CMSGEMOS_TRACE(errstream.str());
  return errstream.str();
}

void gem::hw::GEMHwDevice::connectRPC(bool reconnect)
{
  if (isConnected) {
    // TODO: find better way than hardcoded versions
    this->loadModule("utils", "utils v1.0.1");
    CMSGEMOS_DEBUG("GEMHwDevice::connectRPC modules loaded");
  } else {
    CMSGEMOS_WARN("GEMHwDevice::connectRPC RPC interface failed to connect");
  }
}

void gem::hw::GEMHwDevice::setup(std::string const& deviceName)
{
  b_is_connected = false;
  // if the root logger doesn't exist, need to create it, othewise, just get the instance
  // set up the logging (do we have a parent logger or not?)
  // log4cplus::SharedAppenderPtr appender(new log4cplus::NullAppender());
  // default to console?
  //  log to console? file? xml? port?
  // log4cplus::SharedAppenderPtr appender(new log4cplus::ConsoleAppender(true, true));
  // appender->setName(deviceName);
  // log4cplus::Logger::getRoot().addAppender(appender);

  // m_gemLogger = log4cplus::Logger::getInstance(deviceName);

  setDeviceBaseNode("");
  setDeviceID(deviceName);

  m_crate = extractDeviceID(deviceName,1);
  m_slot  = extractDeviceID(deviceName,2);

  m_ipBusErrs.BadHeader     = 0;
  m_ipBusErrs.ReadError     = 0;
  m_ipBusErrs.Timeout       = 0;
  m_ipBusErrs.ControlHubErr = 0;

  // uhal setup
  setLogLevelTo(uhal::Error());

  // xhal setup
  connectRPC();
  setLogLevel(1/*FIXME add enums to xhal, e.g., xhal::WARN*/);
}

///////////////////////////////////////////////////////////////////////////////////////
//****************Methods implemented for convenience on uhal devices****************//
///////////////////////////////////////////////////////////////////////////////////////

uint32_t gem::hw::GEMHwDevice::readReg(std::string const& name)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);
  uint32_t res = 0;
  try {
    res = xhal::common::rpc::call<::utils::readRemoteReg>(rpc, name);
  } catch (std::exception const& err) {
    //FIXME
  }

  CMSGEMOS_TRACE("GEMHwDevice::Successfully read register " << name.c_str() << " with value 0x"
                 << std::setfill('0') << std::setw(8) << std::hex << res);
  return res;
}

uint32_t gem::hw::GEMHwDevice::readReg(uint32_t const& address)
{
  //Deprecated
  return 0;
}

uint32_t gem::hw::GEMHwDevice::readReg(uint32_t const& address, uint32_t const& mask)
{
  //Deprecated
  return 0;
}

uint32_t gem::hw::GEMHwDevice::readMaskedAddress(std::string const& name)
{
  //Deprecated
  return 0;
}

void gem::hw::GEMHwDevice::readRegs(register_pair_list &regList, int const& freq)
{
  //Deprecated
}

void gem::hw::GEMHwDevice::readRegs(addressed_register_pair_list &regList, int const& freq)
{
  //Deprecated
}

void gem::hw::GEMHwDevice::readRegs(masked_register_pair_list &regList, int const& freq)
{
  //Deprecated
}

void gem::hw::GEMHwDevice::writeReg(std::string const& name, uint32_t const val)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);
  try {
    xhal::common::rpc::call<::utils::writeRemoteReg>(rpc, name, val);
  } catch (std::exception const& err) {
    //FIXME
  }

  CMSGEMOS_TRACE("GEMHwDevice::Successfully wrote register " << name.c_str() << " with value 0x"
                 << std::setfill('0') << std::setw(8) << std::hex << val);
}

void gem::hw::GEMHwDevice::writeReg(uint32_t const& address, uint32_t const val)
{
  //Deprecated
}

void gem::hw::GEMHwDevice::writeRegs(register_pair_list const& regList, int const& freq)
{
  //Deprecated
}

void gem::hw::GEMHwDevice::writeValueToRegs(std::vector<std::string> const& regNames, uint32_t const& regValue, int const& freq)
{
  //Deprecated
}

void gem::hw::GEMHwDevice::zeroRegs(std::vector<std::string> const& regNames, int const& freq)
{
  //Deprecated
}

std::vector<uint32_t> gem::hw::GEMHwDevice::readBlock(std::string const& name)
{
  //FIXME Not supported without uHAL. TBD
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);
  size_t numWords = this->getNode(name).getSize();
  CMSGEMOS_TRACE("GEMHwDevice::reading block " << name << " which has size "<<numWords);
  return readBlock(name, numWords);
}

std::vector<uint32_t> gem::hw::GEMHwDevice::readBlock(std::string const& name, size_t const& numWords)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);
  std::vector<uint32_t> res(numWords); 
  try {
    res = xhal::common::rpc::call<::utils::readRemoteBlock>(rpc, name, numWords, 0);
  } catch (std::exception const& err) {
    //FIXME
  }

  CMSGEMOS_TRACE("GEMHwDevice::Successfully read block " << name.c_str());

  return res;
}

uint32_t gem::hw::GEMHwDevice::readBlock(std::string const& name, uint32_t* buffer,
                                         size_t const& numWords)
{
  // not yet implemented
  return 0;
}

// uint32_t gem::hw::GEMHwDevice::readBlock(std::string const& name, std::vector<toolbox::mem::Reference*>& buffer,
//                                          size_t const& numWords)
// {
//   // not yet implemented
//   return 0;
// }

void gem::hw::GEMHwDevice::writeBlock(std::string const& name, std::vector<uint32_t> const values)
{
  //Deprecated
}

std::vector<uint32_t> gem::hw::GEMHwDevice::readFIFO(std::string const& name)
{
  return readBlock(name);
}

std::vector<uint32_t> gem::hw::GEMHwDevice::readFIFO(std::string const& name, size_t const& numWords)
{
  std::vector<uint32_t> result;
  for (size_t word = 0; word < numWords; ++word)
    result.push_back(readReg(name));

  return result;
}

void gem::hw::GEMHwDevice::writeFIFO(std::string const& name, std::vector<uint32_t> const values)
{
  for (auto word = values.begin(); word != values.end(); ++word)
    writeReg(name,*word);

  return;
}

void gem::hw::GEMHwDevice::zeroFIFO(std::string const& name)
{
  return writeReg(name+".FLUSH",0x0);
}

bool gem::hw::GEMHwDevice::knownErrorCode(std::string const& errCode) const
{
  //FIXME Deprecated
  return ((errCode.find("amount of data")              != std::string::npos) ||
          (errCode.find("INFO CODE = 0x4L")            != std::string::npos) ||
          (errCode.find("INFO CODE = 0x6L")            != std::string::npos) ||
          (errCode.find("timed out")                   != std::string::npos) ||
          (errCode.find("had response field = 0x04")   != std::string::npos) ||
          (errCode.find("had response field = 0x06")   != std::string::npos) ||
          (errCode.find("ControlHub error code is: 3") != std::string::npos) ||
          (errCode.find("ControlHub error code is: 4") != std::string::npos));
}


void gem::hw::GEMHwDevice::updateErrorCounters(std::string const& errCode) const
{
  //FIXME Deprecated
  if (errCode.find("amount of data")    != std::string::npos)
    ++m_ipBusErrs.BadHeader;
  if (errCode.find("INFO CODE = 0x4L")  != std::string::npos)
    ++m_ipBusErrs.ReadError;
  if ((errCode.find("INFO CODE = 0x6L") != std::string::npos) ||
      (errCode.find("timed out")        != std::string::npos))
    ++m_ipBusErrs.Timeout;
  if (errCode.find("ControlHub error code") != std::string::npos)
    ++m_ipBusErrs.ControlHubErr;
  if ((errCode.find("had response field = 0x04") != std::string::npos) ||
      (errCode.find("had response field = 0x06") != std::string::npos))
    ++m_ipBusErrs.ControlHubErr;
}

void gem::hw::GEMHwDevice::zeroBlock(std::string const& name)
{
  //FIXME Deprecated
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);

  size_t numWords = this->getNode(name).getSize();
  std::vector<uint32_t> zeros(numWords, 0);
  return writeBlock(name, zeros);
}

void gem::hw::GEMHwDevice::checkRPCResponse(std::string const& caller) const
{
  if (rsp.get_key_exists("error")) {
    std::stringstream errmsg;
    errmsg << rsp.get_string("error");
    CMSGEMOS_ERROR(caller << ": " << errmsg.str());
    XCEPT_RAISE(gem::hw::devices::exception::RPCMethodError, errmsg.str());
  } else if (rsp.get_key_exists("rpcerror")) {
    std::stringstream errmsg;
    errmsg << rsp.get_string("rpcerror");
    CMSGEMOS_ERROR(caller << ": " << errmsg.str());
    XCEPT_RAISE(gem::hw::devices::exception::RPCMethodError, errmsg.str());
  }
}

uint8_t gem::hw::GEMHwDevice::extractDeviceID(std::string const& deviceName, uint8_t const& index)
{
  std::vector<std::string> subs;
  boost::split(subs, deviceName, boost::is_any_of("-"));
  if (index < subs.size()) {
    return stoull(subs[index].substr(subs[index].find_first_of("0123456789"),2),nullptr,10);
  } else {
    std::stringstream errmsg;
    errmsg << "Unable to extract parameter " << static_cast<int>(index)
           << " value from provided device name: " << deviceName;
    XCEPT_RAISE(gem::hw::devices::exception::DeviceNameParseError, errmsg.str());
  }
}
