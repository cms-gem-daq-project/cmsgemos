#include <iomanip>

#include "gem/hw/HwGenericAMC.h"

gem::hw::HwGenericAMC::HwGenericAMC() :
  gem::hw::GEMHwDevice::GEMHwDevice("HwGenericAMC"),
  m_crate(-1),
  m_slot(-1)
{
  INFO("HwGenericAMC ctor");

  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    AMCIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }

  INFO("HwGenericAMC ctor done");
}

gem::hw::HwGenericAMC::HwGenericAMC(std::string const& amcDevice) :
  gem::hw::GEMHwDevice::GEMHwDevice(amcDevice),
  m_crate(-1),
  m_slot(-1)
{
  INFO("HwGenericAMC ctor");

  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    AMCIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }

  INFO("HwGenericAMC ctor done");
}

gem::hw::HwGenericAMC::HwGenericAMC(std::string const& amcDevice,
                                    int const& crate,
                                    int const& slot) :
  gem::hw::GEMHwDevice::GEMHwDevice(amcDevice),
  m_crate(crate),
  m_slot(slot)
{
  INFO("HwGenericAMC ctor");

  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    AMCIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }

  INFO("HwGenericAMC ctor done");
}

gem::hw::HwGenericAMC::HwGenericAMC(std::string const& amcDevice,
                                    std::string const& connectionFile) :
  gem::hw::GEMHwDevice::GEMHwDevice(amcDevice, connectionFile),
  m_crate(-1),
  m_slot(-1)
{
  INFO("HwGenericAMC ctor");

  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    AMCIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }

  INFO("HwGenericAMC ctor done");
}

gem::hw::HwGenericAMC::HwGenericAMC(std::string const& amcDevice,
                                    std::string const& connectionURI,
                                    std::string const& addressTable) :
  gem::hw::GEMHwDevice::GEMHwDevice(amcDevice, connectionURI, addressTable),
  m_crate(-1),
  m_slot(-1)

{
  INFO("trying to create HwGenericAMC(" << amcDevice << "," << connectionURI << "," <<addressTable);
  setDeviceBaseNode(amcDevice);
  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    AMCIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }

  INFO("HwGenericAMC ctor done");
}

gem::hw::HwGenericAMC::HwGenericAMC(std::string const& amcDevice,
                                    uhal::HwInterface& uhalDevice) :
  gem::hw::GEMHwDevice::GEMHwDevice(amcDevice,uhalDevice),
  m_crate(-1),
  m_slot(-1)

{
  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    AMCIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }

  INFO("HwGenericAMC ctor done");
}

gem::hw::HwGenericAMC::~HwGenericAMC()
{
}

bool gem::hw::HwGenericAMC::isHwConnected()
{
  if ( b_is_connected ) {
    INFO("basic check: HwGenericAMC connection good");
    return true;
  } else if (gem::hw::GEMHwDevice::isHwConnected()) {
    INFO("basic check: HwGenericAMC pointer valid");
    std::vector<linkStatus> tmp_activeLinks;
    tmp_activeLinks.reserve(this->getSupportedOptoHybrids());
    if ((this->getBoardID()).rfind("GLIB") != std::string::npos ) {
      INFO("HwGenericAMC found boardID");
      for (unsigned int gtx = 0; gtx < this->getSupportedOptoHybrids(); ++gtx) {
        // somehow need to actually check that the specified link is present
        b_links[gtx] = true;
        DEBUG("gtx" << gtx << " present(" << this->getFirmwareVer() << ")");
        tmp_activeLinks.push_back(std::make_pair(gtx,this->LinkStatus(gtx)));
      }
    } else {
      INFO("Device not reachable (unable to find 'GenericAMC' in the board ID)"
           << " board ID "              << this->getBoardID()
           << " user firmware version " << this->getFirmwareVer());
    }

    v_activeLinks = tmp_activeLinks;
    if (!v_activeLinks.empty()) {
      b_is_connected = true;
      DEBUG("checked gtxs: HwGenericAMC connection good");
      return true;
    } else {
      b_is_connected = false;
      return false;
    }
  } else {
    return false;
  }
}

std::string gem::hw::HwGenericAMC::getBoardID()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The board ID consists of four characters encoded as a 32-bit unsigned int
  std::string res = "???";
  uint32_t val = readReg(getDeviceBaseNode(), "SYSTEM.BOARD_ID");
  res = gem::utils::uint32ToString(val);
  return res;
}

uint32_t gem::hw::HwGenericAMC::getBoardIDRaw()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The board ID consists of four characters encoded as a 32-bit unsigned int
  uint32_t val = readReg(getDeviceBaseNode(), "SYSTEM.BOARD_ID");
  return val;
}

std::string gem::hw::HwGenericAMC::getFirmwareDate(bool const& system)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream res;
  std::stringstream regName;
  uint32_t fwid = readReg(getDeviceBaseNode(), "SYSTEM.FIRMWARE.DATE");
  res <<         std::setfill('0') <<std::setw(2) << (fwid&0x1f)       // day
      << "-"  << std::setfill('0') <<std::setw(2) << ((fwid>>5)&0x0f)  // month
      << "-"  << std::setw(4) << 2000+((fwid>>9)&0x7f);                // year
  return res.str();
}

uint32_t gem::hw::HwGenericAMC::getFirmwareDateRaw(bool const& system)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  if (system)
    return readReg(getDeviceBaseNode(), "SYSTEM.FIRMWARE.DATE");
  else
    return readReg(getDeviceBaseNode(), "SYSTEM.FIRMWARE.DATE");
}

std::string gem::hw::HwGenericAMC::getFirmwareVer(bool const& system)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream res;
  std::stringstream regName;
  uint32_t fwid;

  if (system)
    fwid = readReg(getDeviceBaseNode(), "SYSTEM.FIRMWARE.ID");
  else
    fwid = readReg(getDeviceBaseNode(), "SYSTEM.FIRMWARE.ID");
  res << ((fwid>>12)&0x0f) << "."
      << ((fwid>>8) &0x0f) << "."
      << ((fwid)    &0xff);
  return res.str();
}

uint32_t gem::hw::HwGenericAMC::getFirmwareVerRaw(bool const& system)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  if (system)
    return readReg(getDeviceBaseNode(), "SYSTEM.FIRMWARE.ID");
  else
    return readReg(getDeviceBaseNode(), "SYSTEM.FIRMWARE.ID");
}

/** User core functionality **/
uint32_t gem::hw::HwGenericAMC::getUserFirmware()
{
  // This returns the firmware register (V2 removed the user firmware specific).
  return readReg(getDeviceBaseNode(), "SYSTEM.FIRMWARE");
}

std::string gem::hw::HwGenericAMC::getUserFirmwareDate()
{
  // This returns the user firmware build date.
  std::stringstream res;
  res << "0x"<< std::hex << getUserFirmware() << std::dec;
  return res.str();
}

bool gem::hw::HwGenericAMC::linkCheck(uint8_t const& gtx, std::string const& opMsg)
{
  if (gtx > this->getSupportedOptoHybrids()) {
    std::string msg = toolbox::toString("%s requested for gtx (%d): outside expectation (0-%d)",
                                        opMsg.c_str(), gtx, this->getSupportedOptoHybrids());
    ERROR(msg);
    // XCEPT_RAISE(gem::hw::exception::InvalidLink,msg);
    return false;
  } else if (!b_links[gtx]) {
    std::string msg = toolbox::toString("%s requested inactive gtx (%d)",opMsg.c_str(), gtx);
    ERROR(msg);
    // XCEPT_RAISE(gem::hw::exception::InvalidLink,msg);
    return false;
  }
  return true;
}

gem::hw::GEMHwDevice::OpticalLinkStatus gem::hw::HwGenericAMC::LinkStatus(uint8_t const& gtx)
{
  gem::hw::GEMHwDevice::OpticalLinkStatus linkStatus;

  if (linkCheck(gtx, "Link status")) {
    linkStatus.TRK_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.TRK_ERR",gtx));
    linkStatus.TRG_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.TRG_ERR",gtx));
    linkStatus.Data_Packets = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.DATA_Packets",gtx));
  }
  return linkStatus;
}

void gem::hw::HwGenericAMC::LinkReset(uint8_t const& gtx, uint8_t const& resets)
{

  // right now this just resets the counters, but we need to be able to "reset" the link too
  if (linkCheck(gtx, "Link reset")) {
    if (resets&0x1)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.TRK_ERR.Reset",gtx),0x1);
    if (resets&0x2)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.TRG_ERR.Reset",gtx),0x1);
    if (resets&0x4)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.DATA_Packets.Reset",gtx),0x1);
  }
}


gem::hw::HwGenericAMC::AMCIPBusCounters gem::hw::HwGenericAMC::getIPBusCounters(uint8_t const& gtx,
                                                                                uint8_t const& mode)
{

  if (linkCheck(gtx, "IPBus counter")) {
    if (mode&0x01)
      m_ipBusCounters.at(gtx).OptoHybridStrobe = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.OptoHybrid_%d",gtx));
    if (mode&0x02)
      m_ipBusCounters.at(gtx).OptoHybridAck    = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.OptoHybrid_%d",gtx));
    if (mode&0x04)
      m_ipBusCounters.at(gtx).TrackingStrobe   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.OptoHybrid_%d",gtx));
    if (mode&0x08)
      m_ipBusCounters.at(gtx).TrackingAck      = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.OptoHybrid_%d",gtx));
    if (mode&0x10)
      m_ipBusCounters.at(gtx).CounterStrobe    = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.Counters"));
    if (mode&0x20)
      m_ipBusCounters.at(gtx).CounterAck       = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.Counters"));
  }
  return m_ipBusCounters.at(gtx);
}

void gem::hw::HwGenericAMC::resetIPBusCounters(uint8_t const& gtx, uint8_t const& resets)
{
  if (linkCheck(gtx, "Reset IPBus counters")) {
    if (resets&0x01)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.OptoHybrid_%d.Reset",gtx), 0x1);
    if (resets&0x02)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.OptoHybrid_%d.Reset",   gtx), 0x1);
    if (resets&0x04)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.TRK_%d.Reset",       gtx), 0x1);
    if (resets&0x08)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.TRK_%d.Reset",          gtx), 0x1);
    if (resets&0x10)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.Counters.Reset"),          0x1);
    if (resets&0x20)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.Counters.Reset"),             0x1);
  }
}

uint32_t gem::hw::HwGenericAMC::readTriggerFIFO(uint8_t const& gtx)
{
  // V2 firmware hasn't got trigger fifo yet
  return 0;
}

void gem::hw::HwGenericAMC::flushTriggerFIFO(uint8_t const& gtx)
{
  // V2 firmware hasn't got trigger fifo yet
  return;
}

uint32_t gem::hw::HwGenericAMC::getFIFOOccupancy(uint8_t const& gtx)
{
  uint32_t fifocc = 0;
  if (linkCheck(gtx, "FIFO occupancy")) {
    std::stringstream regName;
    regName << "TRK_DATA.OptoHybrid_" << (int)gtx;
    fifocc = readReg(getDeviceBaseNode(),regName.str()+".DEPTH");
    DEBUG(toolbox::toString("getFIFOOccupancy(%d) %s.%s%s:: %d", gtx, getDeviceBaseNode().c_str(),
                            regName.str().c_str(), ".DEPTH", fifocc));
  }
  // the fifo occupancy is in number of 32 bit words
  return fifocc;
}

uint32_t gem::hw::HwGenericAMC::getFIFOVFATBlockOccupancy(uint8_t const& gtx)
{
  // what to return when the occupancy is not a full VFAT block?
  return getFIFOOccupancy(gtx)/7;
}

bool gem::hw::HwGenericAMC::hasTrackingData(uint8_t const& gtx)
{
  bool hasData = false;
  if (linkCheck(gtx, "Tracking data")) {
    std::stringstream regName;
    regName << "TRK_DATA.OptoHybrid_" << (int)gtx << ".ISEMPTY";
    hasData = !readReg(getDeviceBaseNode(),regName.str());
  }
  // if the FIFO is fragmented, this will return true but we won't read a full block
  // what to do in this case?
  return hasData;
}

std::vector<uint32_t> gem::hw::HwGenericAMC::getTrackingData(uint8_t const& gtx, size_t const& nBlocks)
{
  if (!linkCheck(gtx, "Tracking data")) {
    // do we really want to return a huge vector of 0s in the case that the link is not up?
    std::vector<uint32_t> data(7*nBlocks,0x0);
    return data;
  }

  std::stringstream regName;
  regName << getDeviceBaseNode() << ".TRK_DATA.OptoHybrid_" << (int)gtx << ".FIFO";
  // best way to read a real block? make getTrackingData ask for N blocks?
  // can we return the memory another way, rather than a vector?
  return readBlock(regName.str(),7*nBlocks);
}

uint32_t gem::hw::HwGenericAMC::getTrackingData(uint8_t const& gtx, uint32_t* data, size_t const& nBlocks)
{
  if (data==NULL) {
    std::string msg = toolbox::toString("Block read requested for null pointer");
    ERROR(msg);
    XCEPT_RAISE(gem::hw::exception::NULLReadoutPointer,msg);
  } else if (!linkCheck(gtx, "Tracking data")) {
    return 0;
  }

  std::stringstream regName;
  regName << getDeviceBaseNode() << ".TRK_DATA.OptoHybrid_" << (int)gtx << ".FIFO";
  // best way to read a real block? make getTrackingData ask for N blocks?
  // can we return the memory another way, rather than a vector?
  // readBlock(regName.str(),7*nBlocks);
  return nBlocks;
}

uint32_t gem::hw::HwGenericAMC::getTrackingData(uint8_t const& gtx, std::vector<toolbox::mem::Reference*>& data,
                                                size_t const& nBlocks)
{
  if (!linkCheck(gtx, "Tracking data")) {
    return 0;
  }

  std::stringstream regName;
  regName << getDeviceBaseNode() << ".TRK_DATA.OptoHybrid_" << (int)gtx << ".FIFO";
  // best way to read a real block? make getTrackingData ask for N blocks?
  // can we return the memory another way, rather than a vector?
  // readBlock(regName.str(),7*nBlocks);
  return nBlocks;
}

void gem::hw::HwGenericAMC::flushFIFO(uint8_t const& gtx)
{
  if (linkCheck(gtx, "Flush FIFO")) {
    std::stringstream regName;
    regName << "TRK_DATA.OptoHybrid_" << (int)gtx;
    INFO("Tracking FIFO" << (int)gtx << ":"
         << " ISFULL  0x" << std::hex << readReg(getDeviceBaseNode(),regName.str()+".ISFULL")  << std::dec
         << " ISEMPTY 0x" << std::hex << readReg(getDeviceBaseNode(),regName.str()+".ISEMPTY") << std::dec
         << " Depth   0x" << std::hex << getFIFOOccupancy(gtx) << std::dec);
    writeReg(getDeviceBaseNode(),regName.str()+".FLUSH",0x1);
    INFO("Tracking FIFO" << (int)gtx << ":"
         << " ISFULL  0x" << std::hex << readReg(getDeviceBaseNode(),regName.str()+".ISFULL")  << std::dec
         << " ISEMPTY 0x" << std::hex << readReg(getDeviceBaseNode(),regName.str()+".ISEMPTY") << std::dec
         << " Depth   0x" << std::hex << getFIFOOccupancy(gtx) << std::dec);
  }
}


/** DAQ link module functions **/
void gem::hw::HwGenericAMC::enableDAQLink(uint32_t const& enableMask)
{
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.INPUT_ENABLE_MASK", enableMask);
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.DAQ_ENABLE", 0x1);
}

void gem::hw::HwGenericAMC::disableDAQLink()
{
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.INPUT_ENABLE_MASK", 0x0);
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.DAQ_ENABLE",        0x0);
}

void gem::hw::HwGenericAMC::resetDAQLink(uint32_t const& davTO)
{
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.RESET", 0x1);
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.RESET", 0x0);
  disableDAQLink();
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.DAV_TIMEOUT", davTO);
  // set each link input timeout to 0x30d4 (160MHz clock cycles, 0xc35 40MHz clock cycles)
  for (unsigned li = 0; li < this->getSupportedOptoHybrids(); ++li) {
    writeReg(getDeviceBaseNode(), toolbox::toString("DAQ.GTX%d.CONTROL.DAV_TIMEOUT", li), 0x30D4);
  }
  // setDAQLinkInputTimeout(davTO);
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.TTS_OVERRIDE", 0x8);/*HACK to be fixed?*/
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkControl()
{
  return readReg(getDeviceBaseNode(), "DAQ.CONTROL");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkStatus()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS");
}

bool gem::hw::HwGenericAMC::daqLinkReady()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.DAQ_LINK_RDY");
}

bool gem::hw::HwGenericAMC::daqClockLocked()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.DAQ_CLK_LOCKED");
}

bool gem::hw::HwGenericAMC::daqTTCReady()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.TTC_RDY");
}

bool gem::hw::HwGenericAMC::daqAlmostFull()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.DAQ_AFULL");
}

uint8_t gem::hw::HwGenericAMC::daqTTSState()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.TTS_STATE");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkEventsSent()
{
  return readReg(getDeviceBaseNode(), "DAQ.EXT_STATUS.EVT_SENT");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkL1AID()
{
  return readReg(getDeviceBaseNode(), "DAQ.EXT_STATUS.L1AID");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkDisperErrors()
{
  return readReg(getDeviceBaseNode(), "DAQ.EXT_STATUS.DISPER_ERR");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkNonidentifiableErrors()
{
  return readReg(getDeviceBaseNode(), "DAQ.EXT_STATUS.NOTINTABLE_ERR");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkInputMask()
{
  return readReg(getDeviceBaseNode(), "DAQ.CONTROL.INPUT_ENABLE_MASK");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkDAVTimeout()
{
  return readReg(getDeviceBaseNode(), "DAQ.CONTROL.DAV_TIMEOUT");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkDAVTimer(bool const& max)
{
  if (max)
    return readReg(getDeviceBaseNode(), "DAQ.EXT_STATUS.MAX_DAV_TIMER");
  else
    return readReg(getDeviceBaseNode(), "DAQ.EXT_STATUS.LAST_DAV_TIMER");
}

/** GTX specific DAQ link information **/
uint32_t gem::hw::HwGenericAMC::getDAQLinkStatus(uint8_t const& gtx)
{
  // do link protections here...
  std::stringstream regBase;
  regBase << "DAQ.GTX" << (int)gtx;
  return readReg(getDeviceBaseNode(),regBase.str()+".STATUS");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkCounters(uint8_t const& gtx, uint8_t const& mode)
{
  std::stringstream regBase;
  regBase << "DAQ.GTX" << (int)gtx << ".COUNTERS";
  if (mode == 0)
    return readReg(getDeviceBaseNode(),regBase.str()+".CORRUPT_VFAT_BLK_CNT");
  else
    return readReg(getDeviceBaseNode(),regBase.str()+".EVN");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkLastBlock(uint8_t const& gtx)
{
  std::stringstream regBase;
  regBase << "DAQ.GTX" << (int)gtx;
  return readReg(getDeviceBaseNode(),regBase.str()+".LASTBLOCK");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkInputTimeout()
{
  return readReg(getDeviceBaseNode(), "DAQ.EXT_CONTROL.INPUT_TIMEOUT");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkRunType()
{
  return readReg(getDeviceBaseNode(), "DAQ.EXT_CONTROL.RUN_TYPE");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkRunParameters()
{
  return readReg(getDeviceBaseNode(), "DAQ.EXT_CONTROL.RUN_PARAMS");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkRunParameter(uint8_t const& parameter)
{
  std::stringstream regBase;
  regBase << "DAQ.EXT_CONTROL.RUN_PARAM" << (int) parameter;
  return readReg(getDeviceBaseNode(),regBase.str());
}

void gem::hw::HwGenericAMC::setDAQLinkInputTimeout(uint32_t const& value)
{
  // set each link input timeout to 0x30d4 (160MHz clock cycles, 0xc35 40MHz clock cycles)
  for (unsigned li = 0; li < this->getSupportedOptoHybrids(); ++li) {
    writeReg(getDeviceBaseNode(), toolbox::toString("DAQ.GTX%d.CONTROL.DAV_TIMEOUT", li), 0x30D4);
  }
  // return writeReg(getDeviceBaseNode(), "DAQ.EXT_CONTROL.INPUT_TIMEOUT",value);
}

void gem::hw::HwGenericAMC::setDAQLinkRunType(uint32_t const& value)
{
  return writeReg(getDeviceBaseNode(), "DAQ.EXT_CONTROL.RUN_TYPE",value);
}

void gem::hw::HwGenericAMC::setDAQLinkRunParameters(uint32_t const& value)
{
  return writeReg(getDeviceBaseNode(), "DAQ.EXT_CONTROL.RUN_PARAMS",value);
}

void gem::hw::HwGenericAMC::setDAQLinkRunParameter(uint8_t const& parameter, uint8_t const& value)
{
  if (parameter < 1 || parameter > 3) {
    std::string msg = toolbox::toString("Attempting to set DAQ link run parameter %d: outside expectation (1-%d)",
                                        (int)parameter,3);
    ERROR(msg);
    return;
  }
  std::stringstream regBase;
  regBase << "DAQ.EXT_CONTROL.RUN_PARAM" << (int) parameter;
  writeReg(getDeviceBaseNode(),regBase.str(),value);
}

/** TTC module functions **/
uint32_t gem::hw::HwGenericAMC::getTTCControl()
{
  return readReg(getDeviceBaseNode(), "TTC.CONTROL");
}

gem::AMCTTCEncoding gem::hw::HwGenericAMC::getTTCEncoding()
{
  return (AMCTTCEncoding)readReg(getDeviceBaseNode(), "TTC.CONTROL.GEMFORMAT");
}

bool gem::hw::HwGenericAMC::getL1AInhibit()
{
  return readReg(getDeviceBaseNode(), "TTC.CONTROL.INHIBIT_L1A");
}

uint32_t gem::hw::HwGenericAMC::getTTCSpyBuffer()
{
  return readReg(getDeviceBaseNode(), "TTC.SPY");
}

void gem::hw::HwGenericAMC::setTTCEncoding(AMCTTCEncoding ttc_enc)
{
  return writeReg(getDeviceBaseNode(), "TTC.CONTROL.GEMFORMAT", (uint32_t)ttc_enc);
}

void gem::hw::HwGenericAMC::setL1AInhibit(bool inhibit)
{
  return writeReg(getDeviceBaseNode(), "TTC.CONTROL.INHIBIT_L1A", (uint32_t)inhibit);
}

void gem::hw::HwGenericAMC::resetTTC()
{
  return writeReg(getDeviceBaseNode(), "TTC.CONTROL.RESET",0x1);
}

void gem::hw::HwGenericAMC::generalReset()
{
  // reset all counters
  counterReset();

  for (unsigned gtx = 0; gtx < this->getSupportedOptoHybrids(); ++gtx)
    linkReset(gtx);

  // other resets

  return;
}

void gem::hw::HwGenericAMC::counterReset()
{
  // reset all counters
  resetT1Counters();

  for (unsigned gtx = 0; gtx < this->getSupportedOptoHybrids(); ++gtx)
    resetIPBusCounters(gtx, 0xff);

  resetLinkCounters();

  return;
}

void gem::hw::HwGenericAMC::resetT1Counters()
{
  writeReg(getDeviceBaseNode(), "T1.L1A.RESET",      0x1);
  writeReg(getDeviceBaseNode(), "T1.CalPulse.RESET", 0x1);
  writeReg(getDeviceBaseNode(), "T1.Resync.RESET",   0x1);
  writeReg(getDeviceBaseNode(), "T1.BC0.RESET",      0x1);
  return;
}

void gem::hw::HwGenericAMC::resetLinkCounters()
{
  return;
}

void gem::hw::HwGenericAMC::linkReset(uint8_t const& gtx)
{
  return;
}
