/**
 *   General structure taken blatantly from tcds::utils::HwDeviceTCA as we're using the same card
 *   11.2018: structure modified to inherit from both uhal::HwInterface and xhal::client::XHALInterface
 *   23.2020: structure modified to drop uhal::HwInterface dependency and depends only on xhal::client::XHALInterface
 */

#include "gem/hw/devices/GEMHwDevice.h"

#include <boost/algorithm/string.hpp>

gem::hw::GEMHwDevice::GEMHwDevice(std::string const& deviceName) :
  xhal::client::XHALInterface(deviceName.substr(0,deviceName.rfind("-optohybrid"))),
  m_gemLogger(log4cplus::Logger::getInstance(deviceName)),
  m_hwLock(toolbox::BSem::FULL, true),
  m_crate(-1),
  m_slot(-1)
{
  CMSGEMOS_DEBUG("GEMHwDevice(std::string) ctor");

  CMSGEMOS_DEBUG("GEMHwDevice::ctor done");
}

gem::hw::GEMHwDevice::~GEMHwDevice()
{
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
  setDeviceID(deviceName);

  m_crate = extractDeviceID(deviceName,1);
  m_slot  = extractDeviceID(deviceName,2);

  // xhal setup
  connectRPC();
  setLogLevel(1/*FIXME add enums to xhal, e.g., xhal::WARN*/);
}

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
