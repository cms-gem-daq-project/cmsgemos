/**
 *   General structure taken blatantly from tcds::utils::HwDeviceTCA as we're using the same card
 *   11.2018: structure modified to inherit from both uhal::HwInterface and xhal::XHALInterface
 */

#include "gem/hw/GEMHwDevice.h"
#include <boost/algorithm/string.hpp>

gem::hw::GEMHwDevice::GEMHwDevice(std::string const& deviceName,
                                  std::string const& connectionFile) :
  xhal::XHALInterface(deviceName),
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
  xhal::XHALInterface(deviceName),
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
  xhal::XHALInterface(deviceName),
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
  CMSGEMOS_TRACE(errstream);
  return errstream.str();
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

  std::vector<std::string> subs;
  // deviceName should be of the format: gem-shelfXX-amcYY
  // optionally including -optohybridZZ
  // boost::split(subs, deviceName, boost::is_any_of('-'));
  // m_crate = stoull(subs[1].substr(subs[1].find_first_of("0123456789"),2),nullptr,10);
  // m_slot  = stoull(subs[2].substr(subs[2].find_first_of("0123456789"),2),nullptr,10);

  m_crate = gem::utils::extractDeviceID(deviceName,1);
  m_slot  = gem::utils::extractDeviceID(deviceName,2);

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

  unsigned retryCount = 0;
  uint32_t res = 0x0;
  CMSGEMOS_DEBUG("GEMHwDevice::readReg " << name << std::endl
                 << "Path  "      << this->getNode(name).getPath()                << std::endl
                 << "Address 0x"  << std::hex << this->getNode(name).getAddress() << std::dec << std::endl
                 << "Mask 0x"     << std::hex << this->getNode(name).getMask()    << std::dec << std::endl
                 << "Permission " << this->getNode(name).getPermission()          << std::endl
                 << "Mode "       << this->getNode(name).getMode()                << std::endl
                 << "Size "       << this->getNode(name).getSize()                << std::endl
                 << std::endl);
  while (retryCount < MAX_IPBUS_RETRIES) {
    ++retryCount;
    try {
      uhal::ValWord<uint32_t> val = this->getNode(name).read();
      this->dispatch();
      res = val.value();
      CMSGEMOS_TRACE("GEMHwDevice::Successfully read register " << name.c_str() << " with value 0x"
                     << std::setfill('0') << std::setw(8) << std::hex << res << std::dec
                     << " retry count is " << retryCount << ". Should move on to next operation");
      return res;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = toolbox::toString("Could not read register '%s' (uHAL)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
        ++retryCount;
        if (retryCount > (MAX_IPBUS_RETRIES-1))
          CMSGEMOS_DEBUG("GEMHwDevice::Failed to read register " << name
                         << ". retryCount("<<retryCount<<")"
                         << std::endl);
        updateErrorCounters(errCode);
        continue;
      } else {
        CMSGEMOS_ERROR("GEMHwDevice::" << msg);
        // XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (std::exception const& err) {
      std::string msgBase = toolbox::toString("Could not read register '%s' (std)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      CMSGEMOS_ERROR("GEMHwDevice::" << msg);
      // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
  std::string msg = toolbox::toString("Maximum number of retries reached, unable to read register %s",name.c_str());
  CMSGEMOS_ERROR("GEMHwDevice::" << msg);
  // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
  return res;
}

uint32_t gem::hw::GEMHwDevice::readReg(uint32_t const& address)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);

  unsigned retryCount = 0;
  uint32_t res = 0x0;
  CMSGEMOS_TRACE("GEMHwDevice::gem::hw::GEMHwDevice::readReg 0x" << std::setfill('0') << std::setw(8)
                 << std::hex << address << std::dec << std::endl);
  while (retryCount < MAX_IPBUS_RETRIES) {
    ++retryCount;
    try {
      uhal::ValWord<uint32_t> val = this->getClient().read(address);
      this->dispatch();
      res = val.value();
      CMSGEMOS_TRACE("GEMHwDevice::Successfully read register 0x" << std::setfill('0') << std::setw(8)
                     << std::hex << address << std::dec << " with value 0x"
                     << std::setfill('0') << std::setw(8) << std::hex << res << std::dec
                     << " retry count is " << retryCount << ". Should move on to next operation");
      return res;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = toolbox::toString("Could not read register '0x%08x' (uHAL)", address);
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
        ++retryCount;
        if (retryCount > (MAX_IPBUS_RETRIES-1))
          CMSGEMOS_DEBUG("GEMHwDevice::Failed to read register 0x" << std::setfill('0') << std::setw(8)
                         << std::hex << address << std::dec
                         << ". retryCount("<<retryCount<<")"
                         << std::endl);
        updateErrorCounters(errCode);
        continue;
      } else {
        CMSGEMOS_ERROR("GEMHwDevice::" << msg);
        // XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (std::exception const& err) {
      std::string msgBase = toolbox::toString("Could not read register '0x%08x' (std)", address);
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      CMSGEMOS_ERROR("GEMHwDevice::" << msg);
      // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
  std::string msg = toolbox::toString("Maximum number of retries reached, unable to read register 0x%08x",
                                      address);
  CMSGEMOS_ERROR("GEMHwDevice::" << msg);
  // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
  return res;
}

uint32_t gem::hw::GEMHwDevice::readReg(uint32_t const& address, uint32_t const& mask)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);

  unsigned retryCount = 0;
  uint32_t res = 0x0;
  CMSGEMOS_TRACE("GEMHwDevice::gem::hw::GEMHwDevice::readReg 0x" << std::setfill('0') << std::setw(8)
                 << std::hex << address << std::dec << std::endl);
  while (retryCount < MAX_IPBUS_RETRIES) {
    ++retryCount;
    try {
      uhal::ValWord<uint32_t> val = this->getClient().read(address,mask);
      this->dispatch();
      res = val.value();
      CMSGEMOS_TRACE("GEMHwDevice::Successfully read register 0x" << std::setfill('0') << std::setw(8)
                     << std::hex << address << std::dec << " with mask "
                     << std::hex << mask << std::dec << " with value "
                     << std::setfill('0') << std::setw(8) << std::hex << res << std::dec
                     << " retry count is " << retryCount << ". Should move on to next operation");
      return res;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = toolbox::toString("Could not read register '0x%08x' (uHAL)", address);
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
        ++retryCount;
        if (retryCount > (MAX_IPBUS_RETRIES-1))
          CMSGEMOS_DEBUG("GEMHwDevice::Failed to read register 0x" << std::setfill('0') << std::setw(8)
                         << std::hex << address << std::dec << " with mask "
                         << std::hex << address << std::dec
                         << ". retryCount("<<retryCount<<")"
                         << std::endl);
        updateErrorCounters(errCode);
        continue;
      } else {
        CMSGEMOS_ERROR("GEMHwDevice::" << msg);
        // XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (std::exception const& err) {
      std::string msgBase = toolbox::toString("Could not read register '0x%08x' (std)", address);
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      CMSGEMOS_ERROR("GEMHwDevice::" << msg);
      // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
  std::string msg = toolbox::toString("Maximum number of retries reached, unable to read register 0x%08x",
                                      address);
  CMSGEMOS_ERROR("GEMHwDevice::" << msg);
  // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
  return res;
}

uint32_t gem::hw::GEMHwDevice::readMaskedAddress(std::string const& name)
{
  uint32_t address = this->getNode(name).getAddress();
  uint32_t mask    = this->getNode(name).getMask();
  return readReg(address,mask);
}

void gem::hw::GEMHwDevice::readRegs(register_pair_list &regList, int const& freq)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);

  unsigned retryCount = 0;
  while (retryCount < MAX_IPBUS_RETRIES) {
    ++retryCount;
    try {
      std::vector<std::pair<std::string,uhal::ValWord<uint32_t> > > vals;
      // vals.reserve(regList.size());
      int counter{0}, dispatchcounter{0};
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg) {
        vals.push_back(std::make_pair(curReg->first,this->getNode(curReg->first).read()));
        ++counter;
        if (freq > 0 && counter%freq == 0) {
          this->dispatch();
          ++dispatchcounter;
        }
      }
      if (freq < 0 || counter%freq != 0) {
        this->dispatch();
          ++dispatchcounter;
      }

      CMSGEMOS_DEBUG("GEMHwDevice::readRegs dispatched " << dispatchcounter
                     << " calls for " << counter << " registers");
      // would like to have these local to the loop, how to do...?
      auto curVal = vals.begin();
      auto curReg = regList.begin();
      for ( ; curReg != regList.end(); ++curVal,++curReg)
        curReg->second = (curVal->second).value();
      return;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = "Could not read from register in list:";
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg)
        msgBase += toolbox::toString(" '%s'", curReg->first.c_str());
      std::string msg     = toolbox::toString("%s (uHAL): %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
        updateErrorCounters(errCode);
        continue;
      } else {
        CMSGEMOS_ERROR("GEMHwDevice::" << msg);
        // XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (std::exception const& err) {
      std::string msgBase = "Could not read from register in list:";
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg)
        msgBase += toolbox::toString(" '%s'", curReg->first.c_str());
      std::string msg = toolbox::toString("%s (std): %s.", msgBase.c_str(), err.what());
      CMSGEMOS_ERROR("GEMHwDevice::" << msg);
      // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
  std::string msg = toolbox::toString("Maximum number of retries reached, unable to read registers");
  CMSGEMOS_ERROR("GEMHwDevice::" << msg);
  // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
}

void gem::hw::GEMHwDevice::readRegs(addressed_register_pair_list &regList, int const& freq)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);

  unsigned retryCount = 0;
  while (retryCount < MAX_IPBUS_RETRIES) {
    ++retryCount;
    try {
      std::vector<std::pair<uint32_t, uhal::ValWord<uint32_t> > > vals;
      // vals.reserve(regList.size());
      int counter{0}, dispatchcounter{0};
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg) {
        vals.push_back(std::make_pair(curReg->first,this->getClient().read(curReg->first)));
        ++counter;
        if (freq > 0 && counter%freq == 0) {
          this->dispatch();
          ++dispatchcounter;
        }
      }
      if (freq < 0 || counter%freq != 0) {
        this->dispatch();
          ++dispatchcounter;
      }

      CMSGEMOS_DEBUG("GEMHwDevice::readRegs dispatched " << dispatchcounter
                     << " calls for " << counter << " registers");
      // would like to have these local to the loop, how to do...?
      auto curVal = vals.begin();
      auto curReg = regList.begin();
      for ( ; curReg != regList.end(); ++curVal,++curReg)
        curReg->second = (curVal->second).value();
      return;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = "Could not read from register in list:";
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg)
        msgBase += toolbox::toString(" '0x%08x mask 0x%08x'", curReg->first);
      std::string msg     = toolbox::toString("%s (uHAL): %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
        updateErrorCounters(errCode);
        continue;
      } else {
        CMSGEMOS_ERROR("GEMHwDevice::" << msg);
        // XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (std::exception const& err) {
      std::string msgBase = "Could not read from register in list:";
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg)
        msgBase += toolbox::toString(" '0x%08x mask 0x%08x'", curReg->first);
      std::string msg = toolbox::toString("%s (std): %s.", msgBase.c_str(), err.what());
      CMSGEMOS_ERROR("GEMHwDevice::" << msg);
      // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
  std::string msg = toolbox::toString("Maximum number of retries reached, unable to read registers");
  CMSGEMOS_ERROR("GEMHwDevice::" << msg);
  // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
}

void gem::hw::GEMHwDevice::readRegs(masked_register_pair_list &regList, int const& freq)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);

  unsigned retryCount = 0;
  while (retryCount < MAX_IPBUS_RETRIES) {
    ++retryCount;
    try {
      std::vector<std::pair<std::pair<uint32_t,uint32_t>,uhal::ValWord<uint32_t> > > vals;
      // vals.reserve(regList.size());
      int counter{0}, dispatchcounter{0};
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg) {
        vals.push_back(std::make_pair(std::make_pair(curReg->first.first,curReg->first.second),
                                      this->getClient().read(curReg->first.first,curReg->second)));
        ++counter;
        if (freq > 0 && counter%freq == 0) {
          this->dispatch();
          ++dispatchcounter;
        }
      }
      if (freq < 0 || counter%freq != 0) {
        this->dispatch();
          ++dispatchcounter;
      }

      CMSGEMOS_DEBUG("GEMHwDevice::readRegs dispatched " << dispatchcounter
                     << " calls for " << counter << " registers");
      // would like to have these local to the loop, how to do...?
      auto curVal = vals.begin();
      auto curReg = regList.begin();
      for ( ; curReg != regList.end(); ++curVal,++curReg)
        curReg->second = (curVal->second).value();
      return;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = "Could not read from register in list:";
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg)
        msgBase += toolbox::toString(" '0x%08x mask 0x%08x'", curReg->first.first, curReg->first.second);
      std::string msg     = toolbox::toString("%s (uHAL): %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
        updateErrorCounters(errCode);
        continue;
      } else {
        CMSGEMOS_ERROR("GEMHwDevice::" << msg);
        // XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (std::exception const& err) {
      std::string msgBase = "Could not read from register in list:";
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg)
        msgBase += toolbox::toString(" '0x%08x mask 0x%08x'", curReg->first.first, curReg->first.second);
      std::string msg = toolbox::toString("%s (std): %s.", msgBase.c_str(), err.what());
      CMSGEMOS_ERROR("GEMHwDevice::" << msg);
      // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
  std::string msg = toolbox::toString("Maximum number of retries reached, unable to read registers");
  CMSGEMOS_ERROR("GEMHwDevice::" << msg);
  // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
}

void gem::hw::GEMHwDevice::writeReg(std::string const& name, uint32_t const val)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);

  unsigned retryCount = 0;
  CMSGEMOS_DEBUG("GEMHwDevice::writeReg " << name << std::endl
                 << "Path  "      << this->getNode(name).getPath() << std::endl
                 << "Address 0x"  << std::hex << this->getNode(name).getAddress() << std::dec << std::endl
                 << "Mask 0x"     << std::hex << this->getNode(name).getMask()    << std::dec << std::endl
                 << "Permission " << this->getNode(name).getPermission() << std::endl
                 << "Mode "       << this->getNode(name).getMode() << std::endl
                 << "Size "       << this->getNode(name).getSize() << std::endl
                 << std::endl);
  while (retryCount < MAX_IPBUS_RETRIES) {
    ++retryCount;
    try {
      uhal::ValWord<uint32_t> ival;
      if (this->getNode(name).getPermission() != uhal::defs::WRITE)
        ival = this->getNode(name).read();
      this->getNode(name).write(val);
      uhal::ValWord<uint32_t> rval;
      if (this->getNode(name).getPermission() != uhal::defs::WRITE)
        rval = this->getNode(name).read();
      this->dispatch();
      if (this->getNode(name).getPermission() != uhal::defs::WRITE)
        CMSGEMOS_DEBUG("GEMHwDevice::writeReg initial: "
                       << std::hex << ival.value() << std::dec
                       << ", write val: " << std::hex << val << std::dec
                       << ", readback: "  << std::hex << rval.value() << std::dec
                       << std::endl);
      if (this->getNode(name).getPermission() != uhal::defs::WRITE)
        if (rval.value() != val) {
          std::string msgBase = toolbox::toString("WriteValueMismatch write (0x%x) to register '%s' resulted in 0x%x (uHAL)",
                                                  val, name.c_str(),rval.value());
          XCEPT_RAISE(gem::hw::exception::WriteValueMismatch, toolbox::toString("%s.", msgBase.c_str()));
        }
      return;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = toolbox::toString("Could not write to register '%s' (uHAL)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
        ++retryCount;
        if (retryCount > (MAX_IPBUS_RETRIES-1))
          CMSGEMOS_DEBUG("GEMHwDevice::Failed to write value 0x" << std::hex<< val << std::dec << " to register " << name
                         << ". retryCount("<<retryCount<<")"
                         << std::endl);
        updateErrorCounters(errCode);
        continue;
      } else {
        CMSGEMOS_ERROR("GEMHwDevice::" << msg);
        // XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (gem::hw::exception::WriteValueMismatch const& err) {
      std::string msgBase = toolbox::toString("Could not write to register '%s' (uHAL)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      CMSGEMOS_ERROR("GEMHwDevice::" << msg);
    } catch (std::exception const& err) {
      std::string msgBase = toolbox::toString("Could not write to register '%s' (std)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      CMSGEMOS_ERROR("GEMHwDevice::" << msg);
      // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
  std::string msg = toolbox::toString("Maximum number of retries reached, unable to write to register %s",name.c_str());
  CMSGEMOS_ERROR("GEMHwDevice::" << msg);
  // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
}

void gem::hw::GEMHwDevice::writeReg(uint32_t const& address, uint32_t const val)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);

  unsigned retryCount = 0;
  while (retryCount < MAX_IPBUS_RETRIES) {
    ++retryCount;
    try {
      uhal::ValWord<uint32_t> ival = this->getClient().read(address);
      this->getClient().write(address, val);
      uhal::ValWord<uint32_t> rval = this->getClient().read(address);
      this->dispatch();
      CMSGEMOS_DEBUG("GEMHwDevice::writeReg initial: "
                     << std::hex << ival.value() << std::dec
                     << ", write val: " << std::hex << val << std::dec
                     << ", readback: "  << std::hex << rval.value() << std::dec
                     << std::endl);
      if (rval.value() != val) {
        std::string msgBase = toolbox::toString("WriteValueMismatch write (0x%x) to register '0x%08x' resulted in 0x%x (uHAL)",
                                                val,address,rval.value());
        XCEPT_RAISE(gem::hw::exception::WriteValueMismatch, toolbox::toString("%s.", msgBase.c_str()));
      }
      return;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = toolbox::toString("Could not write to register '0x%08x' (uHAL)", address);
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
        ++retryCount;
        if (retryCount > (MAX_IPBUS_RETRIES-1))
          CMSGEMOS_DEBUG("GEMHwDevice::Failed to write value 0x" << std::hex<< val << std::dec << " to register 0x"
                         << std::setfill('0') << std::setw(8) << std::hex << address << std::dec
                         << ". retryCount("<<retryCount<<")"
                         << std::endl);
        updateErrorCounters(errCode);
        continue;
      } else {
        CMSGEMOS_ERROR("GEMHwDevice::" << msg);
        // XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (gem::hw::exception::WriteValueMismatch const& err) {
      std::string msgBase = toolbox::toString("Could not write to register '0x%08x' (uHAL)", address);
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      CMSGEMOS_ERROR("GEMHwDevice::" << msg);
    } catch (std::exception const& err) {
      std::string msgBase = toolbox::toString("Could not write to register '0x%08x' (std)", address);
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      CMSGEMOS_ERROR("GEMHwDevice::" << msg);
      // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
  std::string msg = toolbox::toString("Maximum number of retries reached, unable to write to register 0x%08x",
                                      address);
  CMSGEMOS_ERROR("GEMHwDevice::" << msg);
  // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
}

void gem::hw::GEMHwDevice::writeRegs(register_pair_list const& regList, int const& freq)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);

  unsigned retryCount = 0;
  while (retryCount < MAX_IPBUS_RETRIES) {
    ++retryCount;
    try {
      int counter{0}, dispatchcounter{0};
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg) {
        this->getNode(curReg->first).write(curReg->second);
        ++counter;
        if (freq > 0 && counter%freq == 0) {
          this->dispatch();
          ++dispatchcounter;
        }
      }
      if (freq < 0 || counter%freq != 0) {
        this->dispatch();
          ++dispatchcounter;
      }
      CMSGEMOS_DEBUG("GEMHwDevice::writeRegs dispatched " << dispatchcounter
                     << " calls for " << counter << " registers");
      return;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = "Could not write to register in list:";
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg)
        msgBase += toolbox::toString(" '%s'", curReg->first.c_str());
      std::string msg     = toolbox::toString("%s (uHAL): %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
        updateErrorCounters(errCode);
        continue;
      } else {
        CMSGEMOS_ERROR("GEMHwDevice::" << msg);
        // XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (std::exception const& err) {
      std::string msgBase = "Could not write to register in list:";
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg)
        msgBase += toolbox::toString(" '%s'", curReg->first.c_str());
      std::string msg = toolbox::toString("%s (std): %s.", msgBase.c_str(), err.what());
      CMSGEMOS_ERROR("GEMHwDevice::" << msg);
      // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
}

void gem::hw::GEMHwDevice::writeValueToRegs(std::vector<std::string> const& regNames, uint32_t const& regValue, int const& freq)
{
  register_pair_list regsToWrite;
  for (auto curReg = regNames.begin(); curReg != regNames.end(); ++curReg)
    regsToWrite.push_back(std::make_pair(*curReg,regValue));
  writeRegs(regsToWrite,freq);
}

/*
  void gem::hw::GEMHwDevice::zeroReg(std::string const& name)
  {
  writeReg(name,0);
  }
*/

void gem::hw::GEMHwDevice::zeroRegs(std::vector<std::string> const& regNames, int const& freq)
{
  register_pair_list regsToZero;
  for (auto curReg = regNames.begin(); curReg != regNames.end(); ++curReg)
    regsToZero.push_back(std::make_pair(*curReg,0x0));
  writeRegs(regsToZero,freq);
}

std::vector<uint32_t> gem::hw::GEMHwDevice::readBlock(std::string const& name)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);
  size_t numWords = this->getNode(name).getSize();
  CMSGEMOS_TRACE("GEMHwDevice::reading block " << name << " which has size "<<numWords);
  return readBlock(name, numWords);
}

std::vector<uint32_t> gem::hw::GEMHwDevice::readBlock(std::string const& name, size_t const& numWords)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);

  std::vector<uint32_t> res(numWords);

  unsigned retryCount = 0;
  if (numWords < 1)
    return res;

  while (retryCount < MAX_IPBUS_RETRIES) {
    ++retryCount;
    try {
      uhal::ValVector<uint32_t> values = this->getNode(name).readBlock(numWords);
      this->dispatch();
      std::copy(values.begin(), values.end(), res.begin());
      return res;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = toolbox::toString("Could not read block '%s' (uHAL)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
        ++retryCount;
        if (retryCount > (MAX_IPBUS_RETRIES-1))
          CMSGEMOS_DEBUG("GEMHwDevice::Failed to read block " << name << " with " << numWords << " words"
                         << ". retryCount("<<retryCount<<")" << std::endl
                         << "error was " << errCode
                         << std::endl);
        updateErrorCounters(errCode);
        continue;
      } else {
        CMSGEMOS_ERROR("GEMHwDevice::" << msg);
        // XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (std::exception const& err) {
      std::string msgBase = toolbox::toString("Could not read block '%s' (std)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      CMSGEMOS_ERROR("GEMHwDevice::" << msg);
      // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
  std::string msg = toolbox::toString("Maximum number of retries reached, unable to read block");
  CMSGEMOS_ERROR("GEMHwDevice::" << msg);
  // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
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
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);
  if (values.size() < 1)
    return;

  unsigned retryCount = 0;
  while (retryCount < MAX_IPBUS_RETRIES) {
    ++retryCount;
    try {
      this->getNode(name).writeBlock(values);
      this->dispatch();
      return;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = toolbox::toString("Could not write to block '%s' (uHAL)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
        ++retryCount;
        if (retryCount > (MAX_IPBUS_RETRIES-1))
          CMSGEMOS_DEBUG("GEMHwDevice::Failed to write block " << name
                         << ". retryCount("<<retryCount<<")"
                         << std::endl);
        updateErrorCounters(errCode);
        continue;
      } else {
        CMSGEMOS_ERROR("GEMHwDevice::" << msg);
        // XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (std::exception const& err) {
      std::string msgBase = toolbox::toString("Could not write to block '%s' (std)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      CMSGEMOS_ERROR("GEMHwDevice::" << msg);
      // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
  std::string msg = toolbox::toString("Maximum number of retries reached, unable to write block %s",name.c_str());
  CMSGEMOS_ERROR("GEMHwDevice::" << msg);
  // XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
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

bool gem::hw::GEMHwDevice::knownErrorCode(std::string const& errCode) const {
  return ((errCode.find("amount of data")              != std::string::npos) ||
          (errCode.find("INFO CODE = 0x4L")            != std::string::npos) ||
          (errCode.find("INFO CODE = 0x6L")            != std::string::npos) ||
          (errCode.find("timed out")                   != std::string::npos) ||
          (errCode.find("had response field = 0x04")   != std::string::npos) ||
          (errCode.find("had response field = 0x06")   != std::string::npos) ||
          (errCode.find("ControlHub error code is: 3") != std::string::npos) ||
          (errCode.find("ControlHub error code is: 4") != std::string::npos));
}


void gem::hw::GEMHwDevice::updateErrorCounters(std::string const& errCode) {
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
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_hwLock);

  size_t numWords = this->getNode(name).getSize();
  std::vector<uint32_t> zeros(numWords, 0);
  return writeBlock(name, zeros);
}

void gem::hw::GEMHwDevice::generalReset()
{
  return;
}

void gem::hw::GEMHwDevice::counterReset()
{
  return;
}

void gem::hw::GEMHwDevice::linkReset(uint8_t const& link)
{
  return;
}
