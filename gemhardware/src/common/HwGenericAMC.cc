#include "gem/hw/HwGenericAMC.h"

#include <iomanip>

// // define the consts
// const unsigned gem::hw::HwGenericAMC::N_GTX;

gem::hw::HwGenericAMC::HwGenericAMC(std::string const& amcDevice,
                                    std::string const& connectionFile) :
  gem::hw::GEMHwDevice::GEMHwDevice(amcDevice, connectionFile),
  m_links(0),
  m_maxLinks(gem::hw::utils::N_GTX),
  m_crate(-1),
  m_slot(-1)
{
  CMSGEMOS_INFO("HwGenericAMC ctor");
  this->setup(amcDevice);
  this->setDeviceBaseNode("GEM_AMC");

  for (unsigned li = 0; li < gem::hw::utils::N_GTX; ++li) {
    b_links[li] = false;
    AMCIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }

  CMSGEMOS_INFO("HwGenericAMC ctor done, deviceBaseNode is " << getDeviceBaseNode());
}

gem::hw::HwGenericAMC::HwGenericAMC(std::string const& amcDevice,
                                    std::string const& connectionURI,
                                    std::string const& addressTable) :
  gem::hw::GEMHwDevice::GEMHwDevice(amcDevice, connectionURI, addressTable),
  m_links(0),
  m_maxLinks(gem::hw::utils::N_GTX),
  m_crate(-1),
  m_slot(-1)
{
  CMSGEMOS_INFO("trying to create HwGenericAMC(" << amcDevice << "," << connectionURI << "," <<addressTable);
  this->setup(amcDevice);
  this->setDeviceBaseNode("GEM_AMC");
  for (unsigned li = 0; li < gem::hw::utils::N_GTX; ++li) {
    b_links[li] = false;
    AMCIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }

  CMSGEMOS_INFO("HwGenericAMC ctor done, deviceBaseNode is " << getDeviceBaseNode());
}

gem::hw::HwGenericAMC::HwGenericAMC(std::string const& amcDevice,
                                    uhal::HwInterface& uhalDevice) :
  gem::hw::GEMHwDevice::GEMHwDevice(amcDevice,uhalDevice),
  m_links(0),
  m_maxLinks(gem::hw::utils::N_GTX),
  m_crate(-1),
  m_slot(-1)
{
  this->setup(amcDevice);
  this->setDeviceBaseNode("GEM_AMC"); // needed?
  if (isConnected) {
    this->loadModule("amc", "amc v1.0.1");
    CMSGEMOS_DEBUG("HwGenericAMC::HwGenericAMC amc module loaded");
  } else {
    CMSGEMOS_WARN("HwGenericAMC::HwGenericAMC RPC interface failed to connect");
  }

  for (unsigned li = 0; li < gem::hw::utils::N_GTX; ++li) {
    b_links[li] = false;
    AMCIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }

  CMSGEMOS_INFO("HwGenericAMC::HwGenericAMC ctor done, deviceBaseNode is " << getDeviceBaseNode());
}

// gem::hw::HwGenericAMC::HwGenericAMC(int const& shelf,
//                                     int const& slot,
//                                     bool       uhalNative) :
//   gem::hw::HwGenericAMC::HwGenericAMC(toolbox::toString("gem-shelf%02d-amc%02d",shelf,slot),
//                                       connectionURI,
//                                       addressTable
//                                     )
// {
// }

gem::hw::HwGenericAMC::~HwGenericAMC()
{
}

void gem::hw::HwGenericAMC::connectRPC(bool reconnect)
{
  if (isConnected) {
    // TODO: find better way than hardcoded versions
    this->loadModule("amc", "amc v1.0.1");
    this->loadModule("amc", "amc v1.0.1");
    CMSGEMOS_DEBUG("HwGenericAMC::connectRPC modules loaded");
  } else {
    CMSGEMOS_WARN("HwGenericAMC::connectRPC RPC interface failed to connect");
  }
}

bool gem::hw::HwGenericAMC::isHwConnected()
{
  // DO NOT LIKE THIS FUNCTION FIXME!!!
  if (b_is_connected) {
    CMSGEMOS_INFO("basic check: HwGenericAMC connection good");
    return true;
  } else if (gem::hw::GEMHwDevice::isHwConnected()) {
    CMSGEMOS_INFO("basic check: HwGenericAMC pointer valid");
    std::vector<linkStatus> tmp_activeLinks; // FIXME SHOULD BE READ FROM FW, WHAT IS THIS USED FOR?
    m_maxLinks = this->getSupportedOptoHybrids();
    tmp_activeLinks.reserve(m_maxLinks);
    if ((this->getBoardID()).rfind("GLIB") != std::string::npos ) {
      CMSGEMOS_INFO("HwGenericAMC found boardID");
      for (unsigned int gtx = 0; gtx < m_maxLinks; ++gtx) {
        // FIXME!!! somehow need to actually check that the specified link is present
        b_links[gtx] = true;
        CMSGEMOS_INFO("m_links 0x" << std::hex << std::setw(8) << std::setfill('0')
             << m_links
             << " 0x1 << gtx = " << std::setw(8) << std::setfill('0') << (0x1<<gtx)
             << std::dec);
        m_links |= (0x1<<gtx);
        CMSGEMOS_INFO("gtx" << gtx << " present(" << this->getFirmwareVer() << ")");
        CMSGEMOS_INFO("m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
              << m_links << std::dec);
        tmp_activeLinks.push_back(std::make_pair(gtx,this->LinkStatus(gtx)));
        CMSGEMOS_INFO("m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
              << m_links << std::dec);
      }
    } else {
      CMSGEMOS_WARN("Device not reachable (unable to find 'GLIB' in the board ID)"
           << " board ID "              << this->getBoardID()
           << " user firmware version " << this->getFirmwareVer());
      return false;
    }

    v_activeLinks = tmp_activeLinks;
    if (!v_activeLinks.empty()) {
      b_is_connected = true;
      CMSGEMOS_DEBUG("checked gtxs: HwGenericAMC connection good");
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
  res = gem::utils::uint32ToString(getBoardIDRaw());
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
  uint32_t fwid = getFirmwareDateRaw(system);

  // GLIB system FW 0x1f0222b7
  res <<         std::setfill('0') <<std::setw(2) << (fwid&0x1f)       // day
      << "-"  << std::setfill('0') <<std::setw(2) << ((fwid>>5)&0x0f)  // month
      << "-"  << std::setw(4) << 2000+((fwid>>9)&0x7f);                // year
  // GEM system FW date 0xYYYYMMDD
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

  // GLIB system FW 0x1f0222b7
  fwid = getFirmwareVerRaw(system);
  res << ((fwid>>16)&0xff) << "."
      << ((fwid>>8) &0xff) << "."
      << ((fwid)    &0xff);
  // GEM system FW version 0xMMmmpppp
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
  CMSGEMOS_INFO("linkCheck:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
       << m_links << std::dec);
  if (gtx > m_maxLinks) {
    std::string msg = toolbox::toString("%s requested for gtx (%d): outside expectation (0-%d)",
                                        opMsg.c_str(), gtx, m_maxLinks);
    CMSGEMOS_ERROR(msg);
    // XCEPT_RAISE(gem::hw::exception::InvalidLink,msg);
    return false;
    //  } else if (!b_links[gtx]) {
  } else if (!((m_links>>gtx)&0x1)) {
    CMSGEMOS_INFO("linkCheck:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
         << m_links << std::dec);
    std::string msg = toolbox::toString("%s requested inactive gtx (%d, 0x%08x, 0x%08x)",opMsg.c_str(), gtx,m_links,m_links>>gtx);
    CMSGEMOS_ERROR(msg);
    // XCEPT_RAISE(gem::hw::exception::InvalidLink,msg);
    return false;
  }
  CMSGEMOS_INFO("linkCheck:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
       << m_links << std::dec);
  return true;
}

gem::hw::GEMHwDevice::OpticalLinkStatus gem::hw::HwGenericAMC::LinkStatus(uint8_t const& gtx)
{
  gem::hw::GEMHwDevice::OpticalLinkStatus linkStatus;
  // FIXME moved to OH GEM_AMC.OH.OH0.FPGA.GBT.RX.CNT_LINK_ERR
  CMSGEMOS_INFO("LinkStatus:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
       << m_links << std::dec);
  if (linkCheck(gtx, "Link status")) {
    linkStatus.GTX_TRK_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("OH_LINKS.OH%d.TRACK_LINK_ERROR_CNT", gtx));
    linkStatus.GTX_TRG_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("TRIGGER.OH%d.LINK0_MISSED_COMMA_CNT",gtx));
    linkStatus.GTX_Data_Packets = readReg(getDeviceBaseNode(),toolbox::toString("OH_LINKS.OH%d.VFAT_BLOCK_CNT",       gtx));
    linkStatus.GBT_TRK_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("OH_LINKS.OH%d.TRACK_LINK_ERROR_CNT", gtx));
    linkStatus.GBT_Data_Packets = readReg(getDeviceBaseNode(),toolbox::toString("OH_LINKS.OH%d.VFAT_BLOCK_CNT",       gtx));
  }
  CMSGEMOS_INFO("LinkStatus:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
       << m_links << std::dec);
  return linkStatus;
}

void gem::hw::HwGenericAMC::LinkReset(uint8_t const& gtx, uint8_t const& resets)
{
  // FIXME changed in V3 firmware
  // req = wisc::RPCMsg("amc.linkReset");
  // req.set_word("NOH",NOH);
  // try {
  //   rsp = rpc.call_method(req);
  // } STANDARD_CATCH;

  // try {
  //   if (rsp.get_key_exists("error")) {
  //     ERROR("LinkReset error: " << rsp.get_string("error").c_str());
  //     //throw xhal::utils::XHALException("DAQ_TRIGGER_MAIN update failed");
  //   }
  // } STANDARD_CATCH;

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
  // FIXME removed in V3 firmware
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
  // FIXME removed in V3 firmware
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

/** DAQ link module functions **/
void gem::hw::HwGenericAMC::enableDAQLink(uint32_t const& enableMask)
{
  // move enabling of input mask to OH manager
  // writeReg(getDeviceBaseNode(), "DAQ.CONTROL.INPUT_ENABLE_MASK", enableMask);
  writeReg(getDeviceBaseNode(), "DAQ.CONTROL.DAQ_ENABLE", 0x1);

  // or, in the needlessly more complicated way of the RPC message (really not necessary at the AMC level):
  req = wisc::RPCMsg("amc.enableDAQLink");
  req.set_word("enableMask",enableMask);
  try {
    rsp = rpc.call_method(req);
    try {
      if (rsp.get_key_exists("error")) {
        CMSGEMOS_ERROR("HwGenericAMC::enableDAQLink: " << rsp.get_string("error").c_str());
        //throw xhal::utils::XHALException("DAQ_OH_MAIN update failed");
      }
    } STANDARD_CATCH;
  } STANDARD_CATCH;

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
  // 7 individual calls, probably makes sense to use the wrapped RPC
  req = wisc::RPCMsg("amc.resetDAQLink");
  req.set_word("davTO",       davTO);
  req.set_word("ttsOverride", ttsOverride);
  try {
    rsp = rpc.call_method(req);
    try {  // what is excepting here?
      if (rsp.get_key_exists("error")) {
        CMSGEMOS_ERROR("HwGenericAMC::resetDAQLink error: "
                       << rsp.get_string("error").c_str());
        // throw xhal::utils::XHALException("HwGenericAMC::resetDAQLink error: " <<rsp.get_string("error").c_str());
      }
    } STANDARD_CATCH;
  } STANDARD_CATCH;

  // writeReg(getDeviceBaseNode(), "DAQ.CONTROL.RESET", 0x1);
  // writeReg(getDeviceBaseNode(), "DAQ.CONTROL.RESET", 0x0);
  // // disableDAQLink();
  // writeReg(getDeviceBaseNode(), "DAQ.CONTROL.DAV_TIMEOUT", davTO);
  // setDAQLinkInputTimeout();  // default value is 0x100
  // // setDAQLinkInputTimeout(davTO);
  // // writeReg(getDeviceBaseNode(), "DAQ.CONTROL.TTS_OVERRIDE", ttsOverride);/*HACK to be fixed?*/
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
// TODO: should rename, "DAQ link" is a back end FW term, not corresponding to a front end link...
uint32_t gem::hw::HwGenericAMC::getLinkDAQStatus(uint8_t const& gtx)
{
  // do link protections here...
  std::stringstream regBase;
  regBase << "DAQ.OH" << (int)gtx;
  return readReg(getDeviceBaseNode(),regBase.str()+".STATUS");
}

uint32_t gem::hw::HwGenericAMC::getLinkDAQCounters(uint8_t const& gtx, uint8_t const& mode)
{
  std::stringstream regBase;
  regBase << "DAQ.OH" << (int)gtx << ".COUNTERS";
  if (mode == 0)
    return readReg(getDeviceBaseNode(),regBase.str()+".CORRUPT_VFAT_BLK_CNT");
  else
    return readReg(getDeviceBaseNode(),regBase.str()+".EVN");
}

uint32_t gem::hw::HwGenericAMC::getLinkLastDAQBlock(uint8_t const& gtx)
{
  std::stringstream regBase;
  regBase << "DAQ.OH" << (int)gtx;
  return readReg(getDeviceBaseNode(),regBase.str()+".LASTBLOCK");
}

uint32_t gem::hw::HwGenericAMC::getDAQLinkInputTimeout()
{
  // OBSOLETE, NO LONGER PRESENT
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
  // for (unsigned li = 0; li < m_maxLinks; ++li) {
  // for (unsigned li =  m_maxLinks - 1; li > -1; --li) {
  //   writeReg(getDeviceBaseNode(), toolbox::toString("DAQ.OH%d.CONTROL.EOE_TIMEOUT", li), value);
  // }
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
    CMSGEMOS_ERROR(msg);
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
  // writeReg(getDeviceBaseNode(), "TTC.CTRL.PHASE_ALIGNMENT_RESET", 0x1);
}

void gem::hw::HwGenericAMC::ttcMMCMPhaseShift(bool shiftOutOfLockFirst, bool useBC0Locked, bool doScan)
{
  // FIXME CANDIDATE for CTP7 module
  req = wisc::RPCMsg("amc.ttcMMCMPhaseShift");
  req.set_word("shiftOutOfLockFirst", shiftOutOfLockFirst);
  req.set_word("useBC0Locked",        useBC0Locked);
  req.set_word("doScan",              doScan);
  try {
    rsp = rpc.call_method(req);
    try {  // what is excepting here?
      if (rsp.get_key_exists("error")) {
        CMSGEMOS_ERROR("HwGenericAMC::ttcMMCMPhaseShift error: "
                       << rsp.get_string("error").c_str());
        // throw xhal::utils::XHALException("HwGenericAMC::resetDAQLink error: " <<rsp.get_string("error").c_str());
      }
    } STANDARD_CATCH;
  } STANDARD_CATCH;

  /** Description of phase alignment algorithm
      shiftOutOfLockFirst controls whether the procedure will force a relock
      useBC0Locked controls whether the procedure will use the BC0_LOCKED or the PLL_LOCKED register
      * BC0_LOCKED doesn't work in GEM_AMC FW > 1.13.0
      doScan tells the procedure to run through the full possibility of phases several times, and just
      logs the places where it found a lock

      Locking procedure:
      * 3840 shifts is the full width of one good + bad region
      * if shiftOutOfLockFirst
      * * shift into bad region, not on the edge
      * * find next good lock status
      * * shift halfway through the region
      * * * 1920 for BC0_LOCKED,
      * * * 1000 for PLL_LOCKED,
      * if !shiftOutOfLockFirst
      * * find X consecutive "good" locks
      * * * 200 for BC0_LOCKED,
      * * * 50 for PLL_LOCKED,
      * * reverse direction
      * * shift backwards halfway
      * * if a bad lock is encountered, reset and try again
      * * else take the phase at the back half point
   */
//   const int PHASE_CHECK_AVERAGE_CNT = 100;
//   const int PLL_LOCK_READ_ATTEMPTS  = 10;
//   const double PLL_LOCK_WAIT_TIME   = 0.00001; // wait 100us to allow the PLL to lock

//   CMSGEMOS_INFO("HwGenericAMC::ttcMMCMPhaseShift: starting phase shifting procedure");
//   // writeReg(getDeviceBaseNode(), "TTC.CTRL.MMCM_PHASE_SHIFT", 0x1);

//   /*
//   // FIXME block these into one transaction, handicapped by CTP7
//   register_pair_list regList = {
//   {"GEM_AMC.TTC.CTRL.DISABLE_PHASE_ALIGNMENT",       0x1},
//   {"GEM_AMC.TTC.CTRL.PA_DISABLE_GTH_PHASE_TRACKING", 0x1},
//   {"GEM_AMC.TTC.CTRL.PA_MANUAL_OVERRIDE",            0x1},
//   {"GEM_AMC.TTC.CTRL.PA_MANUAL_SHIFT_DIR",           0x1},
//   {"GEM_AMC.TTC.CTRL.PA_GTH_MANUAL_OVERRIDE",        0x1},
//   {"GEM_AMC.TTC.CTRL.PA_GTH_MANUAL_SHIFT_DIR",       0x0},
//   {"GEM_AMC.TTC.CTRL.PA_GTH_MANUAL_SHIFT_STEP",      0x1},
//   {"GEM_AMC.TTC.CTRL.PA_GTH_MANUAL_SEL_OVERRIDE",    0x1},
//   {"GEM_AMC.TTC.CTRL.PA_GTH_MANUAL_COMBINED",        0x1},
//   {"GEM_AMC.TTC.CTRL.GTH_TXDLYBYPASS",               0x1},
//   {"GEM_AMC.TTC.CTRL.PA_MANUAL_PLL_RESET",           0x1},
//   {"GEM_AMC.TTC.CTRL.CNT_RESET",                     0x1}
//   };
//   writeRegs(regList);
//   */
//   // FIXME clean up with multiple dispatch
//   writeReg(getDeviceBaseNode(), "TTC.CTRL.DISABLE_PHASE_ALIGNMENT",       0x1);
//   writeReg(getDeviceBaseNode(), "TTC.CTRL.PA_DISABLE_GTH_PHASE_TRACKING", 0x1);
//   writeReg(getDeviceBaseNode(), "TTC.CTRL.PA_MANUAL_OVERRIDE",            0x1);
//   writeReg(getDeviceBaseNode(), "TTC.CTRL.PA_MANUAL_SHIFT_DIR",           0x1);
//   writeReg(getDeviceBaseNode(), "TTC.CTRL.PA_GTH_MANUAL_OVERRIDE",        0x1);
//   writeReg(getDeviceBaseNode(), "TTC.CTRL.PA_GTH_MANUAL_SHIFT_DIR",       0x0);
//   writeReg(getDeviceBaseNode(), "TTC.CTRL.PA_GTH_MANUAL_SHIFT_STEP",      0x1);
//   writeReg(getDeviceBaseNode(), "TTC.CTRL.PA_GTH_MANUAL_SEL_OVERRIDE",    0x1);
//   writeReg(getDeviceBaseNode(), "TTC.CTRL.PA_GTH_MANUAL_COMBINED",        0x1);
//   writeReg(getDeviceBaseNode(), "TTC.CTRL.GTH_TXDLYBYPASS",               0x1);
//   writeReg(getDeviceBaseNode(), "TTC.CTRL.PA_MANUAL_PLL_RESET",           0x1);
//   writeReg(getDeviceBaseNode(), "TTC.CTRL.CNT_RESET",                     0x1);

//   // add readback of aforementioned registers

//   if (readReg(getDeviceBaseNode(),"TTC.CTRL.DISABLE_PHASE_ALIGNMENT") == 0x0) {
//     CMSGEMOS_WARN("HwGeneircAMC::ttcMMCMPhaseShift automatic phase alignment is turned off!!");
//     // EXCEPT_RAISE
//     return;
//   }

//   int readAttempts = 1;
//   int maxShift     = 7680+(7680/2);

//   if (!useBC0Locked) {
//     readAttempts = PLL_LOCK_READ_ATTEMPTS;
//   }
//   if (doScan) {
//     readAttempts = PLL_LOCK_READ_ATTEMPTS;
//     maxShift     = 23040;
//   }

//   uint32_t mmcmShiftCnt = readReg(getDeviceBaseNode(),"TTC.STATUS.CLK.PA_MANUAL_SHIFT_CNT");
//   uint32_t gthShiftCnt  = readReg(getDeviceBaseNode(),"TTC.STATUS.CLK.PA_MANUAL_GTH_SHIFT_CNT");
//   int  pllLockCnt       = checkPllLock(readAttempts);
//   bool firstUnlockFound = false;
//   bool nextLockFound    = false;
//   bool bestLockFound    = false;
//   bool reversingForLock = false;
//   uint32_t phase = 0;
//   double phaseNs = 0.0;

//   bool mmcmShiftTable[] = {false, false, false, true, false, false, false,
//                            false, false, true, false, false, false, false,
//                            false, true, false, false, false, false, true,
//                            false, false, false, false, false, true, false,
//                            false, false, false, false, true, false, false,
//                            false, false, false, true, false, false};

//   int nGoodLocks       = 0;
//   int nShiftsSinceLock = 0;
//   int nBadLocks        = 0;
//   int totalShiftCount  = 0;

//   for (int i = 0; i < maxShift; ++i) {
//     // FIXME clean up with multiple dispatch
//     this->getNode( "GEM_AMC.TTC.CTRL.PA_GTH_MANUAL_SHIFT_EN").write(0x1);
//     uhal::ValWord<uint32_t> tmpGthShiftCnt  = this->getNode("GEM_AMC.TTC.STATUS.CLK.PA_MANUAL_GTH_SHIFT_CNT").read();
//     uhal::ValWord<uint32_t> tmpMmcmShiftCnt = this->getNode("GEM_AMC.TTC.STATUS.CLK.PA_MANUAL_SHIFT_CNT").read();
//     this->dispatch();

//     // added based on current eagle61 script 08.11.2018
//     bool mmcmShiftRequired = false;
//     if (reversingForLock) {
//       mmcmShiftRequired = mmcmShiftTable[gthShiftCnt];
//     } else {
//       mmcmShiftRequired = mmcmShiftTable[gthShiftCnt+1];
//     }

//     CMSGEMOS_DEBUG("HwGeneircAMC::ttcMMCMPhaseShift BEFORE: "
//                    << "mmcmShift: "  << mmcmShiftCnt << "(" << tmpMmcmShiftCnt << ")"
//                    << ", gthShift: " << gthShiftCnt  << "(" << tmpGthShiftCnt  << ")"
//                    << ", mmcmShiftRequired: "               << mmcmShiftRequired);

//     if (!reversingForLock && (gthShiftCnt == 39)) {
//       CMSGEMOS_DEBUG("HwGeneircAMC::ttcMMCMPhaseShift: normal GTH shift rollover 39->0");
//       gthShiftCnt = 0;
//     } else if (reversingForLock && (gthShiftCnt == 0)){
//       CMSGEMOS_DEBUG("HwGeneircAMC::ttcMMCMPhaseShift: rerversed GTH shift rollover 0->39");
//       gthShiftCnt = 39;
//     } else {
//       if (reversingForLock) {
//         gthShiftCnt -= 1;
//       } else {
//         gthShiftCnt += 1;
//       }
//     }

//     while (gthShiftCnt != tmpGthShiftCnt) {
//       CMSGEMOS_INFO("HwGeneircAMC::ttcMMCMPhaseShift Repeating a GTH PI shift because the shift count doesn't"
//                     << " match the expected value."
//                     << " Expected shift cnt = " << gthShiftCnt
//                     << ", ctp7 returned "       << tmpGthShiftCnt);
//       this->getNode( "GEM_AMC.TTC.CTRL.PA_GTH_MANUAL_SHIFT_EN").write(0x1);
//       tmpGthShiftCnt = this->getNode("GEM_AMC.TTC.STATUS.CLK.PA_MANUAL_GTH_SHIFT_CNT").read();
//       this->dispatch();
//     }

//     if (mmcmShiftRequired) {
//       if (!reversingForLock && (mmcmShiftCnt == 0xffff)) {
//         mmcmShiftCnt = 0;
//       } else if (reversingForLock && (mmcmShiftCnt == 0x0)) {
//         mmcmShiftCnt = 0xffff;
//       } else {
//         if (reversingForLock) {
//           mmcmShiftCnt -= 1;
//         } else {
//           mmcmShiftCnt += 1;
//         }
//       }
//     }

//     CMSGEMOS_DEBUG("HwGeneircAMC::ttcMMCMPhaseShift AFTER: "
//                    << "mmcmShift: "  << mmcmShiftCnt << "(" << tmpMmcmShiftCnt << ")"
//                    << ", gthShift: " << gthShiftCnt  << "(" << tmpGthShiftCnt  << ")"
//                    << ", mmcmShiftRequired: "               << mmcmShiftRequired);

//     tmpMmcmShiftCnt = readReg(getDeviceBaseNode(),"TTC.STATUS.CLK.PA_MANUAL_SHIFT_CNT");
//     if (mmcmShiftCnt != tmpMmcmShiftCnt)
//       CMSGEMOS_WARN("HwGeneircAMC::ttcMMCMPhaseShift Reported MMCM shift count doesn't match the expected MMCM shift count."
//                     << " Expected shift cnt = " << mmcmShiftCnt
//                     << " , ctp7 returned "      << tmpMmcmShiftCnt);

//     // FIXME clean up with multiple dispatch
//     pllLockCnt = checkPllLock(readAttempts);
//     phase      = getMMCMPhaseMean();
//     phaseNs    = phase * 0.01860119;
//     uint32_t gthPhase = getGTHPhaseMean();
//     double gthPhaseNs = gthPhase * 0.01860119;

//     uhal::ValWord<uint32_t> bc0Locked  = this->getNode("GEM_AMC.TTC.STATUS.BC0.LOCKED").read();
//     uhal::ValWord<uint32_t> bc0UnlkCnt = this->getNode("GEM_AMC.TTC.STATUS.BC0.UNLOCK_CNT").read();
//     uhal::ValWord<uint32_t> sglErrCnt  = this->getNode("GEM_AMC.TTC.STATUS.TTC_SINGLE_ERROR_CNT").read();
//     uhal::ValWord<uint32_t> dblErrCnt  = this->getNode("GEM_AMC.TTC.STATUS.TTC_DOUBLE_ERROR_CNT").read();
//     this->dispatch();

//     CMSGEMOS_DEBUG("HwGeneircAMC::ttcMMCMPhaseShift GTH shift #" << i
//                    << ": mmcm shift cnt = "     << mmcmShiftCnt
//                    << ", mmcm phase counts = "  << phase
//                    << ", mmcm phase = "         << phaseNs
//                    << "ns, gth phase counts = " << gthPhase
//                    << ", gth phase = "          << gthPhaseNs
//                    << ", PLL lock count = "     << pllLockCnt);

//     if (useBC0Locked) {
//       if (!firstUnlockFound) {
//         bestLockFound = false;
//         if (bc0Locked == 0) {
//           nBadLocks += 1;
//           nGoodLocks = 0;
//         } else {
//           nBadLocks   = 0;
//           nGoodLocks += 1;
//         }

//         if (shiftOutOfLockFirst) {
//           if (nBadLocks > 100) {
//             firstUnlockFound = true;
//             CMSGEMOS_DEBUG("HwGenericAMC::ttcMMCMPhaseShift 100 unlocks found after " << (i+1) << " shifts:"
//                            << " bad locks "           << nBadLocks
//                            << ", good locks "         << nGoodLocks
//                            << ", mmcm phase count = " << phase
//                            << ", mmcm phase ns = "    << phaseNs << "ns");
//           }
//         } else {
//           if (reversingForLock && (nBadLocks > 0)) {
//             CMSGEMOS_DEBUG("HwGenericAMC::ttcMMCMPhaseShift Bad BC0 lock found:"
//                            << " phase count = " << phase
//                            << ", phase ns = "   << phaseNs << "ns"
//                            << ", returning to normal search");
//             this->getNode("GEM_AMC.TTC.CTRL.PA_MANUAL_SHIFT_DIR").write(1);
//             this->getNode("GEM_AMC.TTC.CTRL.PA_GTH_MANUAL_SHIFT_DIR").write(0);
//             this->dispatch();
//             bestLockFound    = false;
//             reversingForLock = false;
//             nGoodLocks       = 0;
//           } else if (nGoodLocks == 200) {
//             reversingForLock = true;
//             CMSGEMOS_INFO("HwGenericAMC::ttcMMCMPhaseShift 200 consecutive good BC0 locks found:"
//                           << " phase count = " << phase
//                           << ", phase ns = "   << phaseNs << "ns"
//                           << ", reversing scan direction");
//             this->getNode("GEM_AMC.TTC.CTRL.PA_MANUAL_SHIFT_DIR").write(0);
//             this->getNode("GEM_AMC.TTC.CTRL.PA_GTH_MANUAL_SHIFT_DIR").write(1);
//             this->dispatch();
//           }
//           if (reversingForLock && (nGoodLocks == 300)) {
//             CMSGEMOS_INFO("HwGenericAMC::ttcMMCMPhaseShift Best lock found after reversing:"
//                           << " phase count = " << phase
//                           << ", phase ns = "   << phaseNs << "ns");
//             bestLockFound    = true;
//             if (doScan) {
//               this->getNode("GEM_AMC.TTC.CTRL.PA_MANUAL_SHIFT_DIR").write(1);
//               this->getNode("GEM_AMC.TTC.CTRL.PA_GTH_MANUAL_SHIFT_DIR").write(0);
//               this->dispatch();
//               bestLockFound    = false;
//               reversingForLock = false;
//               nGoodLocks       = 0;
//             } else {
//               break;
//             }
//           }
//         }
//       } else { // shift to first good BC0 locked
//         if (bc0Locked == 0) {
//           if (nextLockFound) {
//             CMSGEMOS_DEBUG("HwGenericAMC::ttcMMCMPhaseShift Unexpected unlock after " << (i+1) << " shifts:"
//                            << " bad locks "           << nBadLocks
//                            << ", good locks "         << nGoodLocks
//                            << ", mmcm phase count = " << phase
//                            << ", mmcm phase ns = "    << phaseNs << "ns");
//           }
//           nBadLocks += 1;
//           // nGoodLocks = 0;
//         } else {
//           if (!nextLockFound) {
//             CMSGEMOS_INFO("HwGenericAMC::ttcMMCMPhaseShift Found next lock after "  << (i+1) << " shifts:"
//                           << " bad locks "           << nBadLocks
//                           << ", good locks "         << nGoodLocks
//                           << ", mmcm phase count = " << phase
//                           << ", mmcm phase ns = "    << phaseNs << "ns");
//             nextLockFound = true;
//             nBadLocks   = 0;
//           }
//           nGoodLocks += 1;
//         }
//         if (nGoodLocks == 1920) {
//           CMSGEMOS_INFO("HwGenericAMC::ttcMMCMPhaseShift Finished 1920 shifts after first good lock: "
//                         << "bad locks "   << nBadLocks
//                         << " good locks " <<  nGoodLocks);
//           bestLockFound = true;
//           if (doScan) {
//             nextLockFound    = false;
//             firstUnlockFound = false;
//             nGoodLocks       = 0;
//             nBadLocks        = 0;
//             nShiftsSinceLock = 0;
//           } else {
//             break;
//           }
//         }
//       }
//     } else if (true) { // using the PLL lock counter, but the method as for the BC0 lock
//       if (!firstUnlockFound) {
//         bestLockFound = false;
//         if (pllLockCnt < PLL_LOCK_READ_ATTEMPTS) {
//           nBadLocks += 1;
//           nGoodLocks = 0;
//         } else {
//           nBadLocks   = 0;
//           nGoodLocks += 1;
//         }
//         if (shiftOutOfLockFirst) {
//           if (nBadLocks > 500) {
//             firstUnlockFound = true;
//             CMSGEMOS_DEBUG("HwGenericAMC::ttcMMCMPhaseShift 500 unlocks found after " << i+1 << " shifts:"
//                            << " bad locks "           << nBadLocks
//                            << ", good locks "         << nGoodLocks
//                            << ", mmcm phase count = " << phase
//                            << ", mmcm phase ns = "    << phaseNs << "ns");
//           }
//         } else {
//           if (reversingForLock && (nBadLocks > 0)) {
//             CMSGEMOS_DEBUG("HwGenericAMC::ttcMMCMPhaseShift Bad BC0 lock found:"
//                            << " phase count = " << phase
//                            << ", phase ns = "   << phaseNs << "ns"
//                            << ", returning to normal search");
//             this->getNode("GEM_AMC.TTC.CTRL.PA_MANUAL_SHIFT_DIR").write(1);
//             this->getNode("GEM_AMC.TTC.CTRL.PA_GTH_MANUAL_SHIFT_DIR").write(0);
//             this->dispatch();
//             bestLockFound    = false;
//             reversingForLock = false;
//             nGoodLocks       = 0;
//           } else if (nGoodLocks == 50) {
//             reversingForLock = true;
//             CMSGEMOS_INFO("HwGenericAMC::ttcMMCMPhaseShift 50 consecutive good PLL locks found:"
//                           << " phase count = " << phase
//                           << ", phase ns = "   << phaseNs << "ns"
//                           << ", reversing scan direction");
//             this->getNode("GEM_AMC.TTC.CTRL.PA_MANUAL_SHIFT_DIR").write(0);
//             this->getNode("GEM_AMC.TTC.CTRL.PA_GTH_MANUAL_SHIFT_DIR").write(1);
//             this->dispatch();
//           }
//           if (reversingForLock &&(nGoodLocks == 75)) {
//             CMSGEMOS_INFO("HwGenericAMC::ttcMMCMPhaseShift Best lock found after reversing:"
//                           << " phase count = " << phase
//                           << ", phase ns = "   << phaseNs << "ns.");
//             bestLockFound = true;
//             if (doScan) {
//               this->getNode("GEM_AMC.TTC.CTRL.PA_MANUAL_SHIFT_DIR").write(1);
//               this->getNode("GEM_AMC.TTC.CTRL.PA_GTH_MANUAL_SHIFT_DIR").write(0);
//               this->dispatch();
//               bestLockFound    = false;
//               reversingForLock = false;
//               nGoodLocks       = 0;
//             } else {
//               break;
//             }
//           }
//         }
//       } else { // shift to first good PLL locked
//         if (pllLockCnt < PLL_LOCK_READ_ATTEMPTS) {
//           if (nextLockFound) {
//             CMSGEMOS_WARN("HwGenericAMC::ttcMMCMPhaseShift Unexpected unlock after " << i+1 << " shifts:"
//                           << " bad locks "           << nBadLocks
//                           << ", good locks "         << nGoodLocks
//                           << ", mmcm phase count = " << phase
//                           << ", mmcm phase ns = "    << phaseNs << "ns");
//             nBadLocks += 1;
//             // nGoodLocks = 0;
//           } else {
//             if (!nextLockFound) {
//               CMSGEMOS_INFO("HwGenericAMC::ttcMMCMPhaseShift Found next lock after " << i+1 << " shifts:"
//                             << " bad locks "           << nBadLocks
//                             << ", good locks "         << nGoodLocks
//                             << ", mmcm phase count = " << phase
//                             << ", mmcm phase ns = "    << phaseNs << "ns");
//               nextLockFound = true;
//               nBadLocks     = 0;
//             }
//             nGoodLocks += 1;
//           }
//           if (nShiftsSinceLock == 1000) {
//             CMSGEMOS_INFO("HwGenericAMC::ttcMMCMPhaseShift Finished 1000 shifts after first good lock:"
//                           << " bad locks "   << nBadLocks
//                           << ", good locks " << nGoodLocks);
//             bestLockFound = true;
//             if (doScan) {
//               nextLockFound    = false;
//               firstUnlockFound = false;
//               nGoodLocks       = 0;
//               nBadLocks        = 0;
//               nShiftsSinceLock = 0;
//             } else {
//               break;
//             }
//           }
//         }
//       }
//     } else {
//       if (shiftOutOfLockFirst && (pllLockCnt < PLL_LOCK_READ_ATTEMPTS) && !firstUnlockFound) {
//         firstUnlockFound = true;
//         CMSGEMOS_WARN("HwGenericAMC::ttcMMCMPhaseShift Unlocked after " << i+1 << "shifts:"
//                       << " mmcm phase count = "     << phase
//                       << ", mmcm phase ns = "       << phaseNs << "ns"
//                       << ", pllLockCnt = "          << pllLockCnt
//                       << ", firstUnlockFound = "    << firstUnlockFound
//                       << ", shiftOutOfLockFirst = " << shiftOutOfLockFirst);
//       }

//       if (pllLockCnt == PLL_LOCK_READ_ATTEMPTS) {
//         if (!shiftOutOfLockFirst) {
//           if (nGoodLocks == 50) {
//             reversingForLock = true;
//             CMSGEMOS_INFO("HwGenericAMC::ttcMMCMPhaseShift 200 consecutive good PLL locks found:"
//                           << " phase count = " << phase
//                           << ", phase ns = "   << phaseNs << "ns"
//                           << ", reversing scan direction");
//             this->getNode("GEM_AMC.TTC.CTRL.PA_MANUAL_SHIFT_DIR").write(0);
//             this->getNode("GEM_AMC.TTC.CTRL.PA_GTH_MANUAL_SHIFT_DIR").write(1);
//             this->dispatch();
//           }
//           if (reversingForLock && (nGoodLocks == 75)) {
//             CMSGEMOS_INFO("HwGenericAMC::ttcMMCMPhaseShift Best lock found after reversing:"
//                           << " phase count = " << phase
//                           << ", phase ns = "   << phaseNs << "ns.");
//             bestLockFound    = true;
//             if (doScan) {
//               this->getNode("GEM_AMC.TTC.CTRL.PA_MANUAL_SHIFT_DIR").write(1);
//               this->getNode("GEM_AMC.TTC.CTRL.PA_GTH_MANUAL_SHIFT_DIR").write(0);
//               this->dispatch();
//               bestLockFound    = false;
//               reversingForLock = false;
//               nGoodLocks       = 0;
//               nShiftsSinceLock = 0;
//             } else {
//               break;
//             }
//           }
//         } else if (firstUnlockFound || !shiftOutOfLockFirst) {
//           if (!nextLockFound) {
//             CMSGEMOS_DEBUG("HwGenericAMC::ttcMMCMPhaseShift Found next lock after " << i+1 << " shifts:"
//                            << " bad locks "           << nBadLocks
//                            << ", good locks "         << nGoodLocks
//                            << ", mmcm phase count = " << phase
//                            << ", mmcm phase ns = "    << phaseNs << "ns");
//             nextLockFound = true;
//           }
//           if (nShiftsSinceLock > 500) {
//             bestLockFound = true;
//             if (!doScan)
//               break;
//             nextLockFound    = false;
//             firstUnlockFound = false;
//             bestLockFound    = false;
//             nGoodLocks       = 0;
//             nShiftsSinceLock = 0;
//           }
//         } else {
//           nGoodLocks += 1;
//         }
//       } else if (nextLockFound) {
//         if (nShiftsSinceLock > 500) {
//           bestLockFound = true;
//           if (!doScan)
//             break;
//           nextLockFound    = false;
//           firstUnlockFound = false;
//           bestLockFound    = false;
//           nGoodLocks       = 0;
//           nShiftsSinceLock = 0;
//         }
//       } else {
//         bestLockFound = false;
//         nBadLocks += 1;
//         // nGoodLocks = 0;
//       }
//     }
//     if (nextLockFound)
//       nShiftsSinceLock += 1;
//     if (reversingForLock)
//       totalShiftCount -= 1;
//     else
//       totalShiftCount += 1;
//   }

//   if (bestLockFound) {
//     ttcReset();
//     CMSGEMOS_INFO("HwGeneircAMC::ttcMMCMPhaseShift Lock was found:"
//                   << " phase count " << phase
//                   << ", phase "      << phaseNs << "ns");
//   } else {
//     std::stringstream msg;
//     msg << "HwGeneircAMC::ttcMMCMPhaseShift Unable to find lock";
//     CMSGEMOS_ERROR(msg);
//     // XCEPT_RAISE(gem::hw::exception::MMCMLockFailed,msg);
//   }
}

int gem::hw::HwGenericAMC::checkPllLock(int readAttempts)
{
  // FIXME CANDIDATE for CTP7 module
  const int PHASE_CHECK_AVERAGE_CNT = 100;
  const int PLL_LOCK_READ_ATTEMPTS  = 10;
  const double PLL_LOCK_WAIT_TIME   = 0.00001; // wait 100us to allow the PLL to lock
  int lockCnt = 0;
  for (int i = 0; i < readAttempts; ++i ) {
    writeReg(getDeviceBaseNode(),"TTC.CTRL.PA_MANUAL_PLL_RESET", 0x1);
    sleep(PLL_LOCK_WAIT_TIME);
    if (readReg(getDeviceBaseNode(),"TTC.STATUS.CLK.PHASE_LOCKED") != 0)
      lockCnt += 1;
  }
  return lockCnt;
  /*
  const double PLL_LOCK_WAIT_TIME = 0.0001; // wait 100us to allow the PLL to lock
  writeReg(getDeviceBaseNode(), "TTC.CTRL.PA_MANUAL_PLL_RESET", 0x1);
  sleep(PLL_LOCK_WAIT_TIME);
  if (((readReg(getDeviceBaseNode(), "TTC.STATUS.CLK.PHASE_LOCKED") & 0x4) >> 2) == 0)
    return false;
  else
    return true;
   */
}

uint32_t gem::hw::HwGenericAMC::getMMCMPhaseMean()
{
  return readReg(getDeviceBaseNode(), "TTC.STATUS.CLK.TTC_PM_PHASE_MEAN");
}

uint32_t gem::hw::HwGenericAMC::getGTHPhaseMean()
{
  return readReg(getDeviceBaseNode(), "TTC.STATUS.CLK.GTH_PM_PHASE_MEAN");
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
  // uint32_t safeEnable = 0xa4a2c200+int(enable);
  writeReg(getDeviceBaseNode(), "TTC.CTRL.L1A_ENABLE", uint32_t(enable));
}

uint32_t gem::hw::HwGenericAMC::getTTCConfig(AMCTTCCommandT const& cmd)
{
  CMSGEMOS_WARN("HwGenericAMC::getTTCConfig: not implemented");
  return 0x0;
}

void gem::hw::HwGenericAMC::setTTCConfig(AMCTTCCommandT const& cmd, uint8_t const& value)
{
  CMSGEMOS_WARN("HwGenericAMC::setTTCConfig: not implemented");
  return;
}

uint32_t gem::hw::HwGenericAMC::getTTCStatus()
{
  CMSGEMOS_WARN("HwGenericAMC::getTTCStatus: not fully implemented");
  return readReg(getDeviceBaseNode(), "TTC.STATUS");
}

uint32_t gem::hw::HwGenericAMC::getTTCErrorCount(bool const& single)
{
  if (single)
    return readReg(getDeviceBaseNode(), "TTC.STATUS.TTC_SINGLE_ERROR_CNT");
  else
    return readReg(getDeviceBaseNode(), "TTC.STATUS.TTC_DOUBLE_ERROR_CNT");
}

uint32_t gem::hw::HwGenericAMC::getTTCCounter(AMCTTCCommandT const& cmd)
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

uint32_t gem::hw::HwGenericAMC::getL1ARate()
{
  return readReg(getDeviceBaseNode(), "TTC.L1A_RATE");
}

uint32_t gem::hw::HwGenericAMC::getTTCSpyBuffer()
{
  // FIXME: OBSOLETE in V3?
  CMSGEMOS_WARN("HwGenericAMC::getTTCSpyBuffer: TTC.TTC_SPY_BUFFER is obsolete and will be removed in a future release");
  return 0x0;
  // return readReg(getDeviceBaseNode(), "TTC.TTC_SPY_BUFFER");
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

uint32_t gem::hw::HwGenericAMC::getOptoHybridTriggerLinkCount(uint8_t const& oh, uint8_t const& link, AMCOHLinkCountT const& count)
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
  // TODO: CTP7 module candidate
  // reset all counters
  counterReset();

  for (unsigned gtx = 0; gtx < m_maxLinks; ++gtx)
    linkReset(gtx);

  // other resets

  return;
}

void gem::hw::HwGenericAMC::counterReset()
{
  // TODO: CTP7 module candidate
  // reset all counters
  resetT1Counters();

  for (unsigned gtx = 0; gtx < m_maxLinks; ++gtx)
    resetIPBusCounters(gtx, 0xff);

  linkCounterReset();

  return;
}

void gem::hw::HwGenericAMC::resetT1Counters()
{
  // TODO: CTP7 module candidate
  // FIXME: OBSOLETE in V3
  CMSGEMOS_WARN("HwGenericAMC::resetT1Counters is obsolete and will be removed in a future release");
  writeReg(getDeviceBaseNode(), "T1.L1A.RESET",      0x1);
  writeReg(getDeviceBaseNode(), "T1.CalPulse.RESET", 0x1);
  writeReg(getDeviceBaseNode(), "T1.Resync.RESET",   0x1);
  writeReg(getDeviceBaseNode(), "T1.BC0.RESET",      0x1);
  return;
}

void gem::hw::HwGenericAMC::linkCounterReset()
{
  // TODO: CTP7 module candidate
  // FIXME: OBSOLETE in V3?
  CMSGEMOS_WARN("HwGenericAMC::linkCounterReset: not yet implemented");
  return;
}

void gem::hw::HwGenericAMC::linkReset(uint8_t const& gtx)
{
  // TODO: CTP7 module candidate
  // FIXME: OBSOLETE in V3?
  CMSGEMOS_WARN("HwGenericAMC::linkReset: not yet implemented");
  linkCounterReset();
  return;
}
