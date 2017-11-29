#include "gem/hw/HwGenericAMC.h"

#include <iomanip>

// gem::hw::HwGenericAMC::HwGenericAMC() :
//   gem::hw::GEMHwDevice::GEMHwDevice("HwGenericAMC"),
//   m_links(0),
//   m_crate(-1),
//   m_slot(-1)
// {
//   INFO("HwGenericAMC ctor");

//   for (unsigned li = 0; li < N_GTX; ++li) {
//     b_links[li] = false;
//     AMCIPBusCounters tmpGTXCounter;
//     m_ipBusCounters.push_back(tmpGTXCounter);
//   }

//   INFO("HwGenericAMC ctor done, deviceBaseNode is " << getDeviceBaseNode());
// }

// gem::hw::HwGenericAMC::HwGenericAMC(std::string const& amcDevice) :
//   gem::hw::GEMHwDevice::GEMHwDevice(amcDevice),
//   m_links(0),
//   m_crate(-1),
//   m_slot(-1)
// {
//   INFO("HwGenericAMC ctor");

//   for (unsigned li = 0; li < N_GTX; ++li) {
//     b_links[li] = false;
//     AMCIPBusCounters tmpGTXCounter;
//     m_ipBusCounters.push_back(tmpGTXCounter);
//   }

//   INFO("HwGenericAMC ctor done, deviceBaseNode is " << getDeviceBaseNode());
// }

// gem::hw::HwGenericAMC::HwGenericAMC(std::string const& amcDevice,
//                                     int const& crate,
//                                     int const& slot) :
//   gem::hw::GEMHwDevice::GEMHwDevice(amcDevice),
//   m_links(0),
//   m_crate(crate),
//   m_slot(slot)
// {
//   INFO("HwGenericAMC ctor");

//   for (unsigned li = 0; li < N_GTX; ++li) {
//     b_links[li] = false;
//     AMCIPBusCounters tmpGTXCounter;
//     m_ipBusCounters.push_back(tmpGTXCounter);
//   }

//   INFO("HwGenericAMC ctor done, deviceBaseNode is " << getDeviceBaseNode());
// }

gem::hw::HwGenericAMC::HwGenericAMC(std::string const& amcDevice,
                                    std::string const& connectionFile) :
  gem::hw::GEMHwDevice::GEMHwDevice(amcDevice, connectionFile),
  m_links(0),
  m_crate(-1),
  m_slot(-1)
{
  INFO("HwGenericAMC ctor");

  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    AMCIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }

  INFO("HwGenericAMC ctor done, deviceBaseNode is " << getDeviceBaseNode());
}

gem::hw::HwGenericAMC::HwGenericAMC(std::string const& amcDevice,
                                    std::string const& connectionURI,
                                    std::string const& addressTable) :
  gem::hw::GEMHwDevice::GEMHwDevice(amcDevice, connectionURI, addressTable),
  m_links(0),
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

  INFO("HwGenericAMC ctor done, deviceBaseNode is " << getDeviceBaseNode());
}

gem::hw::HwGenericAMC::HwGenericAMC(std::string const& amcDevice,
                                    uhal::HwInterface& uhalDevice) :
  gem::hw::GEMHwDevice::GEMHwDevice(amcDevice,uhalDevice),
  m_links(0),
  m_crate(-1),
  m_slot(-1)

{
  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    AMCIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }

  INFO("HwGenericAMC ctor done, deviceBaseNode is " << getDeviceBaseNode());
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
        // FIXME!!! somehow need to actually check that the specified link is present
        b_links[gtx] = true;
        INFO("m_links 0x" << std::hex << std::setw(8) << std::setfill('0')
             << m_links
             << " 0x1 << gtx = " << std::setw(8) << std::setfill('0') << (0x1<<gtx)
             << std::dec);
        m_links |= (0x1<<gtx);
        INFO("gtx" << gtx << " present(" << this->getFirmwareVer() << ")");
        INFO("m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
              << m_links << std::dec);
        tmp_activeLinks.push_back(std::make_pair(gtx,this->LinkStatus(gtx)));
        INFO("m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
              << m_links << std::dec);
      }
    } else {
      WARN("Device not reachable (unable to find 'GLIB' in the board ID)"
           << " board ID "              << this->getBoardID()
           << " user firmware version " << this->getFirmwareVer());
      return false;
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
  uint32_t val = readReg(getDeviceBaseNode(), "GEM_SYSTEM.BOARD_ID");
  res = gem::utils::uint32ToString(val);
  return res;
}

uint32_t gem::hw::HwGenericAMC::getBoardIDRaw()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The board ID consists of four characters encoded as a 32-bit unsigned int
  uint32_t val = readReg(getDeviceBaseNode(), "GEM_SYSTEM.BOARD_ID");
  return val;
}

std::string gem::hw::HwGenericAMC::getFirmwareDate(bool const& system)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream res;
  std::stringstream regName;
  uint32_t fwid = readReg(getDeviceBaseNode(), "GEM_SYSTEM.RELEASE.DATE");
  res <<         std::setfill('0') <<std::setw(2) << (fwid&0x1f)       // day
      << "-"  << std::setfill('0') <<std::setw(2) << ((fwid>>5)&0x0f)  // month
      << "-"  << std::setw(4) << 2000+((fwid>>9)&0x7f);                // year
  return res.str();
}

uint32_t gem::hw::HwGenericAMC::getFirmwareDateRaw(bool const& system)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  if (system)
    return readReg(getDeviceBaseNode(), "GEM_SYSTEM.RELEASE.DATE");
  else
    return readReg(getDeviceBaseNode(), "GEM_SYSTEM.RELEASE.DATE");
}

std::string gem::hw::HwGenericAMC::getFirmwareVer(bool const& system)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream res;
  std::stringstream regName;
  uint32_t fwid;

  if (system)
    fwid = readReg(getDeviceBaseNode(), "GEM_SYSTEM.RELEASE");
  else
    fwid = readReg(getDeviceBaseNode(), "GEM_SYSTEM.RELEASE");
  res << ((fwid>>16)&0xff) << "."
      << ((fwid>>8) &0xff) << "."
      << ((fwid)    &0xff);
  return res.str();
}

uint32_t gem::hw::HwGenericAMC::getFirmwareVerRaw(bool const& system)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  if (system)
    return readReg(getDeviceBaseNode(), "GEM_SYSTEM.RELEASE");
  else
    return readReg(getDeviceBaseNode(), "GEM_SYSTEM.RELEASE");
}

/** User core functionality **/
uint32_t gem::hw::HwGenericAMC::getUserFirmware()
{
  // This returns the firmware register (V2 removed the user firmware specific).
  return readReg(getDeviceBaseNode(), "GEM_SYSTEM.RELEASE");
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
  INFO("linkCheck:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
       << m_links << std::dec);
  if (gtx > this->getSupportedOptoHybrids()) {
    std::string msg = toolbox::toString("%s requested for gtx (%d): outside expectation (0-%d)",
                                        opMsg.c_str(), gtx, this->getSupportedOptoHybrids());
    ERROR(msg);
    // XCEPT_RAISE(gem::hw::exception::InvalidLink,msg);
    return false;
    //  } else if (!b_links[gtx]) {
  } else if (!((m_links>>gtx)&0x1)) {
    INFO("linkCheck:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
         << m_links << std::dec);
    std::string msg = toolbox::toString("%s requested inactive gtx (%d, 0x%08x, 0x%08x)",opMsg.c_str(), gtx,m_links,m_links>>gtx);
    ERROR(msg);
    // XCEPT_RAISE(gem::hw::exception::InvalidLink,msg);
    return false;
  }
  INFO("linkCheck:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
       << m_links << std::dec);
  return true;
}

gem::hw::GEMHwDevice::OpticalLinkStatus gem::hw::HwGenericAMC::LinkStatus(uint8_t const& gtx)
{
  gem::hw::GEMHwDevice::OpticalLinkStatus linkStatus;

  INFO("LinkStatus:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
       << m_links << std::dec);
  if (linkCheck(gtx, "Link status")) {
    linkStatus.GTX_TRK_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("OH_LINKS.OH%d.TRACK_LINK_ERROR_CNT", gtx));
    linkStatus.GTX_TRG_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("TRIGGER.OH%d.LINK0_MISSED_COMMA_CNT",gtx));
    linkStatus.GTX_Data_Packets = readReg(getDeviceBaseNode(),toolbox::toString("OH_LINKS.OH%d.VFAT_BLOCK_CNT",       gtx));
    linkStatus.GBT_TRK_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("OH_LINKS.OH%d.TRACK_LINK_ERROR_CNT", gtx));
    linkStatus.GBT_Data_Packets = readReg(getDeviceBaseNode(),toolbox::toString("OH_LINKS.OH%d.VFAT_BLOCK_CNT",       gtx));
  }
  INFO("LinkStatus:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
       << m_links << std::dec);
  return linkStatus;
}

void gem::hw::HwGenericAMC::LinkReset(uint8_t const& gtx, uint8_t const& resets)
{

  // right now this just resets the counters, but we need to be able to "reset" the link too
  if (linkCheck(gtx, "Link reset")) {
    if (resets&0x1)
      writeReg(getDeviceBaseNode(),toolbox::toString("OH_LINKS.CTRL.CNT_RESET"), gtx);
    if (resets&0x2)
      writeReg(getDeviceBaseNode(),toolbox::toString("TRIGGER.CTRL.CNT_RESET"),  gtx);
    if (resets&0x4)
      writeReg(getDeviceBaseNode(),toolbox::toString("OH_LINKS.CTRL.CNT_RESET"), gtx);
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

/** obsolete in generic AMC firmware **
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
**/

/** DAQ link module functions **/
void gem::hw::HwGenericAMC::enableDAQLink(uint32_t const& enableMask)
{
  // move enabling of input mask to OH manager
  // writeReg(getDeviceBaseNode(), "DAQ.CONTROL.INPUT_ENABLE_MASK", enableMask);
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.DAQ_ENABLE", 0x1);
}

void gem::hw::HwGenericAMC::disableDAQLink()
{
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.INPUT_ENABLE_MASK", 0x0);
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.DAQ_ENABLE",        0x0);
}

void gem::hw::HwGenericAMC::enableZeroSuppression(bool en)
{
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.ZERO_SUPPRESSION_EN", uint32_t(en));
}

void gem::hw::HwGenericAMC::disableZeroSuppression()
{
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.ZERO_SUPPRESSION_EN", 0x0);
}

void gem::hw::HwGenericAMC::resetDAQLink(uint32_t const& davTO, uint32_t const& ttsOverride)
{
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.RESET", 0x1);
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.RESET", 0x0);
  disableDAQLink();
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.DAV_TIMEOUT", davTO);
  setDAQLinkInputTimeout();  // default value is 0x100
  // setDAQLinkInputTimeout(davTO);
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.TTS_OVERRIDE", ttsOverride);/*HACK to be fixed?*/
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

uint8_t gem::hw::HwGenericAMC::daqTTSState()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.TTS_STATE");
}

bool gem::hw::HwGenericAMC::daqAlmostFull()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.DAQ_AFULL");
}

bool gem::hw::HwGenericAMC::l1aFIFOIsEmpty()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.L1A_FIFO_IS_EMPTY");
}

bool gem::hw::HwGenericAMC::l1aFIFOIsAlmostFull()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.L1A_FIFO_IS_NEAR_FULL");
}

bool gem::hw::HwGenericAMC::l1aFIFOIsFull()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.L1A_FIFO_IS_FULL");
}

bool gem::hw::HwGenericAMC::l1aFIFOIsUnderflow()
{
  return readReg(getDeviceBaseNode(), "DAQ.STATUS.L1A_FIFO_IS_UNDERFLOW");
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
  regBase << "DAQ.OH" << (int)gtx;
  return readReg(getDeviceBaseNode(),regBase.str()+".STATUS");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkCounters(uint8_t const& gtx, uint8_t const& mode)
{
  std::stringstream regBase;
  regBase << "DAQ.OH" << (int)gtx << ".COUNTERS";
  if (mode == 0)
    return readReg(getDeviceBaseNode(),regBase.str()+".CORRUPT_VFAT_BLK_CNT");
  else
    return readReg(getDeviceBaseNode(),regBase.str()+".EVN");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkLastBlock(uint8_t const& gtx)
{
  std::stringstream regBase;
  regBase << "DAQ.OH" << (int)gtx;
  return readReg(getDeviceBaseNode(),regBase.str()+".LASTBLOCK");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkInputTimeout()
{
  // OBSOLETE, but will likely be recovered post 1.11.3
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
  for (unsigned li = 0; li < this->getSupportedOptoHybrids(); ++li) {
    writeReg(getDeviceBaseNode(), toolbox::toString("DAQ.OH%d.CONTROL.EOE_TIMEOUT", li), value);
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

/********************************/
/** TTC module information **/
/********************************/

/** TTC module functions **/
void gem::hw::HwGenericAMC::ttcReset()
{
  writeReg(getDeviceBaseNode(), "TTC.CTRL.MODULE_RESET", 0x1);
}

void gem::hw::HwGenericAMC::ttcMMCMReset()
{
  writeReg(getDeviceBaseNode(), "TTC.CTRL.MMCM_RESET", 0x1);
}

void gem::hw::HwGenericAMC::ttcMMCMPhaseShift()
 {
  writeReg(getDeviceBaseNode(), "TTC.CTRL.MMCM_PHASE_SHIFT", 0x1);
}

void gem::hw::HwGenericAMC::ttcCounterReset()
{
  writeReg(getDeviceBaseNode(), "TTC.CTRL.CNT_RESET", 0x1);
}

bool gem::hw::HwGenericAMC::getL1AEnable()
{
  return readReg(getDeviceBaseNode(), "TTC.CTRL.L1A_ENABLE");
}

void gem::hw::HwGenericAMC::setL1AEnable(bool enable)
{
  writeReg(getDeviceBaseNode(), "TTC.CTRL.L1A_ENABLE", (uint32_t)enable);
}

uint32_t gem::hw::HwGenericAMC::getTTCConfig(AMCTTCCommand const& cmd)
{
  return 0x0;
}

void gem::hw::HwGenericAMC::setTTCConfig(AMCTTCCommand const& cmd, uint8_t const& value)
{
  return;
}

uint32_t gem::hw::HwGenericAMC::getTTCStatus()
{
  return readReg(getDeviceBaseNode(), "TTC.STATUS.MODULE");
}

uint32_t gem::hw::HwGenericAMC::getTTCErrorCount(bool const& single)
{
  if (single)
    return readReg(getDeviceBaseNode(), "TTC.STATUS.TTC_SINGLE_ERROR_CNT");
  else
    return readReg(getDeviceBaseNode(), "TTC.STATUS.TTC_DOUBLE_ERROR_CNT");
}

uint32_t gem::hw::HwGenericAMC::getTTCCounter(AMCTTCCommand const& cmd)
{
  switch(cmd) {
  case(AMCTTCCommand::TTC_L1A) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.L1A");
  case(AMCTTCCommand::TTC_BC0) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.BC0");
  case(AMCTTCCommand::TTC_EC0) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.EC0");
  case(AMCTTCCommand::TTC_RESYNC) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.RESYNC");
  case(AMCTTCCommand::TTC_OC0) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.OC0");
  case(AMCTTCCommand::TTC_HARD_RESET) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.HARD_RESET");
  case(AMCTTCCommand::TTC_CALPULSE) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.CALPULSE");
  case(AMCTTCCommand::TTC_START) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.START");
  case(AMCTTCCommand::TTC_STOP) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.STOP");
  case(AMCTTCCommand::TTC_TEST_SYNC) :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.TEST_SYNC");
  default :
    return readReg(getDeviceBaseNode(), "TTC.CMD_COUNTERS.L1A");
  }
}

uint32_t gem::hw::HwGenericAMC::getL1AID()
{
  return readReg(getDeviceBaseNode(), "TTC.L1A_ID");
}

uint32_t gem::hw::HwGenericAMC::getTTCSpyBuffer()
{
  return readReg(getDeviceBaseNode(), "TTC.TTC_SPY_BUFFER");
}

/********************************/
/** SLOW_CONTROL module information **/
/********************************/

/*** SCA submodule ***/
void gem::hw::HwGenericAMC::scaHardResetEnable(bool const& en)
{
  writeReg(getDeviceBaseNode(), "SLOW_CONTROL.SCA.CTRL.TTC_HARD_RESET_EN", uint32_t(en));
}

/********************************/
/** TRIGGER module information **/
/********************************/

void gem::hw::HwGenericAMC::triggerReset()
{
  writeReg(getDeviceBaseNode(), "TRIGGER.CTRL.MODULE_RESET", 0x1);
}

void gem::hw::HwGenericAMC::triggerCounterReset()
{
  writeReg(getDeviceBaseNode(), "TRIGGER.CTRL.CNT_RESET", 0x1);
}

uint32_t gem::hw::HwGenericAMC::getOptoHybridKillMask()
{
  return readReg(getDeviceBaseNode(), "TRIGGER.CTRL.OH_KILL_MASK");
}

void gem::hw::HwGenericAMC::setOptoHybridKillMask(uint32_t const& mask)
{
  writeReg(getDeviceBaseNode(), "TRIGGER.CTRL.OH_KILL_MASK", mask);
}

/*** STATUS submodule ***/
uint32_t gem::hw::HwGenericAMC::getORTriggerRate()
{
  return readReg(getDeviceBaseNode(), "TRIGGER.STATUS.OR_TRIGGER_RATE");
}

uint32_t gem::hw::HwGenericAMC::getORTriggerCount()
{
  return readReg(getDeviceBaseNode(), "TRIGGER.STATUS.TRIGGER_SINGLE_ERROR_CNT");
}

/*** OH{IDXX} submodule ***/
uint32_t gem::hw::HwGenericAMC::getOptoHybridTriggerRate(uint8_t const& oh)
{
  return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.TRIGGER_RATE",(int)oh));
}

uint32_t gem::hw::HwGenericAMC::getOptoHybridTriggerCount(uint8_t const& oh)
{
  return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.TRIGGER_CNT",(int)oh));
}

uint32_t gem::hw::HwGenericAMC::getOptoHybridClusterRate(uint8_t const& oh, uint8_t const& cs)
{
  return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.CLUSTER_SIZE_%d_RATE",(int)oh,(int)cs));
}

uint32_t gem::hw::HwGenericAMC::getOptoHybridClusterCount(uint8_t const& oh, uint8_t const& cs)
{
  return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.CLUSTER_SIZE_%d_CNT",(int)oh,(int)cs));
}

uint32_t gem::hw::HwGenericAMC::getOptoHybridDebugLastCluster(uint8_t const& oh, uint8_t const& cs)
{
  return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.DEBUG_LAST_CLUSTER_%d",(int)oh,(int)cs));
}

uint32_t gem::hw::HwGenericAMC::getOptoHybridTriggerLinkCount(uint8_t const& oh, uint8_t const& link, AMCOHLinkCount const& count)
{
  switch(count) {
  case(AMCOHLinkCount::LINK_NOT_VALID) :
    return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.LINK%d_NOT_VALID_CNT",(int)oh,(int)link));
  case(AMCOHLinkCount::LINK_MISSED_COMMA) :
    return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.LINK%d_MISSED_COMMA_CNT",(int)oh,(int)link));
  case(AMCOHLinkCount::LINK_OVERFLOW) :
    return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.LINK%d_OVERFLOW_CNT",(int)oh,(int)link));
  case(AMCOHLinkCount::LINK_UNDERFLOW) :
    return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.LINK%d_UNDERFLOW_CNT",(int)oh,(int)link));
  case(AMCOHLinkCount::LINK_SYNC_WORD) :
    return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.LINK%d_SYNC_WORD_CNT",(int)oh,(int)link));
  default :
    return readReg(getDeviceBaseNode(), toolbox::toString("TRIGGER.OH%d.LINK%d_MISSED_COMMA_CNT",(int)oh,(int)link));
  }
}


// general resets
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
