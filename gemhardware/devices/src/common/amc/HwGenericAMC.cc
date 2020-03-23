#include "gem/hw/devices/amc/HwGenericAMC.h"

#include <iomanip>

gem::hw::amc::HwGenericAMC::HwGenericAMC(std::string const& amcDevice) :
  gem::hw::GEMHwDevice::GEMHwDevice(amcDevice),
  m_links(0),
  m_maxLinks(gem::hw::utils::N_GTX)
{
  CMSGEMOS_INFO("HwGenericAMC ctor");
  this->setup(amcDevice);
  CMSGEMOS_INFO("HwGenericAMC ctor done");
}

gem::hw::amc::HwGenericAMC::~HwGenericAMC()
{
}

void gem::hw::amc::HwGenericAMC::connectRPC(bool reconnect)
{
  if (isConnected) {
    // TODO: find better way than hardcoded versions
    this->loadModule("amc", "amc v1.0.1");
    CMSGEMOS_DEBUG("HwGenericAMC::connectRPC modules loaded");
  } else {
    CMSGEMOS_WARN("HwGenericAMC::connectRPC RPC interface failed to connect");
  }
}

bool gem::hw::amc::HwGenericAMC::isHwConnected()
{
  // DO NOT LIKE THIS FUNCTION FIXME!!!
  if (b_is_connected) {
    CMSGEMOS_INFO("basic check: HwGenericAMC connection good");
    return true;
  } else if (gem::hw::GEMHwDevice::isHwConnected()) {
    CMSGEMOS_INFO("basic check: HwGenericAMC pointer valid");
    m_maxLinks = this->getSupportedOptoHybrids();
    // FIXME needs update for V3
    if (((this->getBoardIDString()).rfind("GLIB") != std::string::npos) ||
        ((this->getBoardIDString()).rfind("beef") != std::string::npos)) {
      CMSGEMOS_INFO("HwGenericAMC found boardID");

      for (int gtx = 0; gtx < 12; ++gtx) { //FIXME removed hardcode
        // FIXME OBSOLETE!!! somehow need to actually check that the specified link is present
        // check GBT status?
        CMSGEMOS_INFO("m_links 0x" << std::hex << std::setw(8) << std::setfill('0')
                      << m_links
                      << " 0x1 << gtx = " << std::setw(8) << std::setfill('0') << (0x1<<gtx)
                      << std::dec);
        m_links |= (0x1<<gtx);
        CMSGEMOS_INFO("gtx" << static_cast<int>(gtx) << " present("
                      << this->getFirmwareVer() << ")"); // FIXME get link FW?
        CMSGEMOS_INFO("m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
                      << m_links << std::dec);
        CMSGEMOS_INFO("m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
                      << m_links << std::dec);
      }

      return true;
    } else {
      CMSGEMOS_WARN("Device not reachable (unable to find 'GLIB' in the board ID)"
                    << " board ID "              << this->getBoardIDString()
                    << " user firmware version " << this->getFirmwareVer());
      return false;
    }
  } else {
    return false;
  }
}

std::string gem::hw::amc::HwGenericAMC::getBoardIDString()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The board ID consists of four characters encoded as a 32-bit unsigned int
  std::string res = "N/A";
  uint32_t brdID = getBoardID();
  res = toolbox::toString("%x",brdID);
  return res;
}

uint32_t gem::hw::amc::HwGenericAMC::getBoardID()
{
  return readReg("GEM_AMC.GEM_SYSTEM.BOARD_ID");
}

std::string gem::hw::amc::HwGenericAMC::getBoardTypeString()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The board ID consists of four characters encoded as a 32-bit unsigned int
  std::string res = "N/A";
  uint32_t brdID = getBoardType();
  res = toolbox::toString("%x",brdID);
  return res;
}

uint32_t gem::hw::amc::HwGenericAMC::getBoardType()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The board type consists of four characters encoded as a 32-bit unsigned int
  return readReg("GEM_AMC.GEM_SYSTEM.BOARD_TYPE");
}

std::string gem::hw::amc::HwGenericAMC::getSystemIDString()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The system ID consists of four characters encoded as a 32-bit unsigned int
  std::string res = "N/A";
  uint32_t sysID = getSystemID();
  res = toolbox::toString("%x",sysID);
  return res;
}

uint32_t gem::hw::amc::HwGenericAMC::getSystemID()
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The system ID consists of four characters encoded as a 32-bit unsigned int
  return readReg("GEM_AMC.GEM_SYSTEM.BOARD_TYPE");
}

uint32_t gem::hw::amc::HwGenericAMC::getSupportedOptoHybrids()
{
  return readReg("GEM_AMC.GEM_SYSTEM.CONFIG.NUM_OF_OH");
}

uint32_t gem::hw::amc::HwGenericAMC::supportsTriggerLink()
{
  return readReg("GEM_AMC.GEM_SYSTEM.CONFIG.USE_TRIG_LINKS");
}

std::string gem::hw::amc::HwGenericAMC::getFirmwareDateString(bool const& system)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream res;
  std::stringstream regName;
  uint32_t fwid = getFirmwareDate(system);

  // GLIB system FW 0x1f0222b7
  res <<         std::setfill('0') <<std::setw(2) << (fwid&0x1f)       // day
      << "-"  << std::setfill('0') <<std::setw(2) << ((fwid>>5)&0x0f)  // month
      << "-"  << std::setw(4) << 2000+((fwid>>9)&0x7f);                // year
  // GEM system FW date 0xYYYYMMDD
  return res.str();
}

uint32_t gem::hw::amc::HwGenericAMC::getFirmwareDate(bool const& system)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  if (system)
    return readReg("GEM_AMC.GEM_SYSTEM.RELEASE.DATE");
  else
    return readReg("GEM_AMC.GEM_SYSTEM.RELEASE.DATE");
}

std::string gem::hw::amc::HwGenericAMC::getFirmwareVerString(bool const& system)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream res;
  std::stringstream regName;
  uint32_t fwid;

  // GLIB system FW 0x1f0222b7
  fwid = getFirmwareVer(system);
  res << ((fwid>>16)&0xff) << "."
      << ((fwid>>8) &0xff) << "."
      << ((fwid)    &0xff);
  // GEM system FW version 0xMMmmpppp
  return res.str();
}

uint32_t gem::hw::amc::HwGenericAMC::getFirmwareVer(bool const& system)
{
  // gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  if (system)
    return readReg("GEM_AMC.GEM_SYSTEM.RELEASE");
  else
    return readReg("GEM_AMC.GEM_SYSTEM.RELEASE");
}

/** User core functionality **/
uint32_t gem::hw::amc::HwGenericAMC::getUserFirmware()
{
  // This returns the firmware register (V2 removed the user firmware specific).
  return readReg("GEM_AMC.GEM_SYSTEM.RELEASE");
}

std::string gem::hw::amc::HwGenericAMC::getUserFirmwareDate()
{
  // This returns the user firmware build date.
  std::stringstream res;
  res << "0x"<< std::hex << getUserFirmware() << std::dec;
  return res.str();
}

bool gem::hw::amc::HwGenericAMC::linkCheck(uint8_t const& gtx, std::string const& opMsg)
{
  CMSGEMOS_INFO("linkCheck:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
                << m_links << std::dec);
  if (gtx > m_maxLinks) {
    std::string msg = toolbox::toString("%s requested for gtx (%d): outside expectation (0-%d)",
                                        opMsg.c_str(), gtx, m_maxLinks);
    CMSGEMOS_ERROR(msg);
    // XCEPT_RAISE(gem::hw::devices::exception::InvalidLink,msg);
    return false;
  } else if (!((m_links>>gtx)&0x1)) {
    CMSGEMOS_INFO("linkCheck:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
                  << m_links << std::dec);
    std::string msg = toolbox::toString("%s requested inactive gtx (%d, 0x%08x, 0x%08x)",
                                        opMsg.c_str(), gtx,m_links,m_links>>gtx);
    CMSGEMOS_ERROR(msg);
    // XCEPT_RAISE(gem::hw::devices::exception::InvalidLink,msg);
    return false;
  }
  CMSGEMOS_INFO("linkCheck:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
                << m_links << std::dec);
  return true;
}

// FIXME UPDATE //
// gem::hw::GEMHwDevice::OpticalLinkStatus gem::hw::amc::HwGenericAMC::LinkStatus(uint8_t const& gtx)
// {
//   gem::hw::GEMHwDevice::OpticalLinkStatus linkStatus;
//   // FIXME moved to OH GEM_AMC.OH.OH0.FPGA.GBT.RX.CNT_LINK_ERR
//   CMSGEMOS_INFO("LinkStatus:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
//                 << m_links << std::dec);
//   if (linkCheck(gtx, "Link status")) {
//     linkStatus.GTX_TRK_Errors   = readReg("GEM_AMC."+toolbox::toString("OH_LINKS.OH%d.TRACK_LINK_ERROR_CNT", gtx));
//     linkStatus.GTX_TRG_Errors   = readReg("GEM_AMC."+toolbox::toString("TRIGGER.OH%d.LINK0_MISSED_COMMA_CNT",gtx));
//     linkStatus.GTX_Data_Packets = readReg("GEM_AMC."+toolbox::toString("OH_LINKS.OH%d.VFAT_BLOCK_CNT",       gtx));
//     linkStatus.GBT_TRK_Errors   = readReg("GEM_AMC."+toolbox::toString("OH_LINKS.OH%d.TRACK_LINK_ERROR_CNT", gtx));
//     linkStatus.GBT_Data_Packets = readReg("GEM_AMC."+toolbox::toString("OH_LINKS.OH%d.VFAT_BLOCK_CNT",       gtx));
//   }
//   CMSGEMOS_INFO("LinkStatus:: m_links 0x" << std::hex <<std::setw(8) << std::setfill('0')
//                 << m_links << std::dec);
//   return linkStatus;
// }

// FIXME UPDATE //
// void gem::hw::amc::HwGenericAMC::LinkReset(uint8_t const& gtx, uint8_t const& resets)
// {
//   // FIXME right now this just resets the counters, but we need to be able to "reset" the link too
//   if (linkCheck(gtx, "Link reset")) {
//     if (resets&0x1)
//       writeReg("GEM_AMC."+toolbox::toString("OH_LINKS.CTRL.CNT_RESET"), gtx);
//     if (resets&0x2)
//       writeReg("GEM_AMC."+toolbox::toString("TRIGGER.CTRL.CNT_RESET"),  gtx);
//     if (resets&0x4)
//       writeReg("GEM_AMC."+toolbox::toString("OH_LINKS.CTRL.CNT_RESET"), gtx);
//   }
// }


uint32_t gem::hw::amc::HwGenericAMC::readTriggerFIFO(uint8_t const& gtx)
{
  // V2 firmware hasn't got trigger fifo yet
  return 0;
}

void gem::hw::amc::HwGenericAMC::flushTriggerFIFO(uint8_t const& gtx)
{
  // V2 firmware hasn't got trigger fifo yet
  return;
}

/** DAQ link module functions **/
void gem::hw::amc::HwGenericAMC::configureDAQModule(bool enableZS, bool doPhaseShift, uint32_t const& runType, uint32_t const& marker, bool relock, bool bc0LockPSMode)
{
  try {
    req = wisc::RPCMsg("amc.configureDAQModule");
    req.set_word("enableZS",     enableZS);
    req.set_word("doPhaseShift", doPhaseShift);
    req.set_word("runType",      runType);
    req.set_word("marker",       marker);
    req.set_word("relock",       relock);
    req.set_word("bc0LockPSMode",bc0LockPSMode);
    try {
      rsp = rpc.call_method(req);
      try {
        if (rsp.get_key_exists("error")) {
          std::stringstream errmsg;
          errmsg << rsp.get_string("error");
          CMSGEMOS_ERROR("HwGenericAMC::configureDAQModule: " << errmsg.str());
          XCEPT_RAISE(gem::hw::devices::exception::RPCMethodError, errmsg.str());
        }
      } STANDARD_CATCH;
    } STANDARD_CATCH;
  } GEM_CATCH_RPC_ERROR("HwGenericAMC::enableDAQLink", gem::hw::devices::exception::Exception);
}

void gem::hw::amc::HwGenericAMC::enableDAQLink(uint32_t const& enableMask)
{
  try {
    req = wisc::RPCMsg("amc.enableDAQLink");
    req.set_word("enableMask",enableMask);
    try {
      rsp = rpc.call_method(req);
      try {
        if (rsp.get_key_exists("error")) {
          std::stringstream errmsg;
          errmsg << rsp.get_string("error");
          CMSGEMOS_ERROR("HwGenericAMC::enableDAQLink: " << errmsg.str());
          XCEPT_RAISE(gem::hw::devices::exception::RPCMethodError, errmsg.str());
        }
      } STANDARD_CATCH;
    } STANDARD_CATCH;
  } GEM_CATCH_RPC_ERROR("HwGenericAMC::enableDAQLink", gem::hw::devices::exception::Exception);
}

void gem::hw::amc::HwGenericAMC::disableDAQLink()
{
  try {
    req = wisc::RPCMsg("amc.disableDAQLink");
    try {
      rsp = rpc.call_method(req);
      try {
        if (rsp.get_key_exists("error")) {
          std::stringstream errmsg;
          errmsg << rsp.get_string("error");
          CMSGEMOS_ERROR("HwGenericAMC::disableDAQLink: " << errmsg.str());
          XCEPT_RAISE(gem::hw::devices::exception::RPCMethodError, errmsg.str());
        }
      } STANDARD_CATCH;
    } STANDARD_CATCH;
  } GEM_CATCH_RPC_ERROR("HwGenericAMC::enableDAQLink", gem::hw::devices::exception::Exception);
}

void gem::hw::amc::HwGenericAMC::setZS(bool en)
{
  writeReg("GEM_AMC.DAQ.CONTROL.ZERO_SUPPRESSION_EN", uint32_t(en));
}

void gem::hw::amc::HwGenericAMC::resetDAQLink(uint32_t const& davTO, uint32_t const& ttsOverride)
{
  try {
    req = wisc::RPCMsg("amc.resetDAQLink");
    req.set_word("davTO",       davTO);
    req.set_word("ttsOverride", ttsOverride);
    try {
      rsp = rpc.call_method(req);
      try {
        if (rsp.get_key_exists("error")) {
          std::stringstream errmsg;
          errmsg << rsp.get_string("error");
          XCEPT_RAISE(gem::hw::devices::exception::RPCMethodError, errmsg.str());
        }
      } STANDARD_CATCH;
    } STANDARD_CATCH;
    /*  FIXME alternative
    // should reraise gem::hw::devices::exception::RPCMethodError as a result of the STANDARD_CATCH
    // turn this into a macro?
    } catch (...) {
      handleRPCError("HwGenericAMC::enableDAQLink");
      // https://stackoverflow.com/questions/3561659/how-can-i-abstract-out-a-repeating-try-catch-pattern-in-c
      // https://stackoverflow.com/questions/2466131/is-re-throwing-an-exception-legal-in-a-nested-try
      // https://stackoverflow.com/questions/13007793/is-a-macro-to-catch-a-set-of-exceptions-at-different-places-fine
      // with lambdas
      // https://codereview.stackexchange.com/questions/2484/generic-c-exception-catch-handler-macro
    }
     */
  } GEM_CATCH_RPC_ERROR("HwGenericAMC::enableDAQLink", gem::hw::devices::exception::Exception);
}

uint32_t gem::hw::amc::HwGenericAMC::getDAQLinkControl()
{
  return readReg("GEM_AMC.DAQ.CONTROL");
}

uint32_t gem::hw::amc::HwGenericAMC::getDAQLinkStatus()
{
  return readReg("GEM_AMC.DAQ.STATUS");
}

bool gem::hw::amc::HwGenericAMC::daqLinkReady()
{
  return readReg("GEM_AMC.DAQ.STATUS.DAQ_LINK_RDY");
}

bool gem::hw::amc::HwGenericAMC::daqClockLocked()
{
  return readReg("GEM_AMC.DAQ.STATUS.DAQ_CLK_LOCKED");
}

bool gem::hw::amc::HwGenericAMC::daqTTCReady()
{
  return readReg("GEM_AMC.DAQ.STATUS.TTC_RDY");
}

uint8_t gem::hw::amc::HwGenericAMC::daqTTSState()
{
  return readReg("GEM_AMC.DAQ.STATUS.TTS_STATE");
}

bool gem::hw::amc::HwGenericAMC::daqAlmostFull()
{
  return readReg("GEM_AMC.DAQ.STATUS.DAQ_AFULL");
}

bool gem::hw::amc::HwGenericAMC::l1aFIFOIsEmpty()
{
  return readReg("GEM_AMC.DAQ.STATUS.L1A_FIFO_IS_EMPTY");
}

bool gem::hw::amc::HwGenericAMC::l1aFIFOIsAlmostFull()
{
  return readReg("GEM_AMC.DAQ.STATUS.L1A_FIFO_IS_NEAR_FULL");
}

bool gem::hw::amc::HwGenericAMC::l1aFIFOIsFull()
{
  return readReg("GEM_AMC.DAQ.STATUS.L1A_FIFO_IS_FULL");
}

bool gem::hw::amc::HwGenericAMC::l1aFIFOIsUnderflow()
{
  return readReg("GEM_AMC.DAQ.STATUS.L1A_FIFO_IS_UNDERFLOW");
}

uint32_t gem::hw::amc::HwGenericAMC::getDAQLinkEventsSent()
{
  return readReg("GEM_AMC.DAQ.EXT_STATUS.EVT_SENT");
}

uint32_t gem::hw::amc::HwGenericAMC::getDAQLinkL1AID()
{
  return readReg("GEM_AMC.DAQ.EXT_STATUS.L1AID");
}

uint32_t gem::hw::amc::HwGenericAMC::getDAQLinkDisperErrors()
{
  return readReg("GEM_AMC.DAQ.EXT_STATUS.DISPER_ERR");
}

uint32_t gem::hw::amc::HwGenericAMC::getDAQLinkNonidentifiableErrors()
{
  return readReg("GEM_AMC.DAQ.EXT_STATUS.NOTINTABLE_ERR");
}

uint32_t gem::hw::amc::HwGenericAMC::getDAQLinkInputMask()
{
  return readReg("GEM_AMC.DAQ.CONTROL.INPUT_ENABLE_MASK");
}

uint32_t gem::hw::amc::HwGenericAMC::getDAQLinkDAVTimeout()
{
  return readReg("GEM_AMC.DAQ.CONTROL.DAV_TIMEOUT");
}

uint32_t gem::hw::amc::HwGenericAMC::getDAQLinkDAVTimer(bool const& max)
{
  if (max)
    return readReg("GEM_AMC.DAQ.EXT_STATUS.MAX_DAV_TIMER");
  else
    return readReg("GEM_AMC.DAQ.EXT_STATUS.LAST_DAV_TIMER");
}

/** GTX specific DAQ link information **/
// TODO: should rename, "DAQ link" is a back end FW term, not corresponding to a front end link...
uint32_t gem::hw::amc::HwGenericAMC::getLinkDAQStatus(uint8_t const& gtx)
{
  // do link protections here...
  std::stringstream regBase;
  regBase << "DAQ.OH" << (int)gtx;
  return readReg("GEM_AMC."+regBase.str()+".STATUS");
}

uint32_t gem::hw::amc::HwGenericAMC::getLinkDAQCounters(uint8_t const& gtx, uint8_t const& mode)
{
  std::stringstream regBase;
  regBase << "DAQ.OH" << (int)gtx << ".COUNTERS";
  if (mode == 0)
    return readReg("GEM_AMC."+regBase.str()+".CORRUPT_VFAT_BLK_CNT");
  else
    return readReg("GEM_AMC."+regBase.str()+".EVN");
}

uint32_t gem::hw::amc::HwGenericAMC::getLinkLastDAQBlock(uint8_t const& gtx)
{
  std::stringstream regBase;
  regBase << "DAQ.OH" << (int)gtx;
  return readReg("GEM_AMC."+regBase.str()+".LASTBLOCK");
}

uint32_t gem::hw::amc::HwGenericAMC::getDAQLinkInputTimeout()
{
  // OBSOLETE, NO LONGER PRESENT
  return readReg("GEM_AMC.DAQ.EXT_CONTROL.INPUT_TIMEOUT");
}

uint32_t gem::hw::amc::HwGenericAMC::getDAQLinkRunType()
{
  return readReg("GEM_AMC.DAQ.EXT_CONTROL.RUN_TYPE");
}

uint32_t gem::hw::amc::HwGenericAMC::getDAQLinkRunParameters()
{
  return readReg("GEM_AMC.DAQ.EXT_CONTROL.RUN_PARAMS");
}

uint32_t gem::hw::amc::HwGenericAMC::getDAQLinkRunParameter(uint8_t const& parameter)
{
  std::stringstream regBase;
  regBase << "DAQ.EXT_CONTROL.RUN_PARAM" << (int) parameter;
  return readReg("GEM_AMC."+regBase.str());
}

void gem::hw::amc::HwGenericAMC::setDAQLinkInputTimeout(uint32_t const& value)
{
  // for (unsigned li = 0; li < m_maxLinks; ++li) {
  // for (unsigned li =  m_maxLinks - 1; li > -1; --li) {
  //   writeReg("GEM_AMC."+toolbox::toString("DAQ.OH%d.CONTROL.EOE_TIMEOUT", li), value);
  // }
  // return writeReg("GEM_AMC.DAQ.EXT_CONTROL.INPUT_TIMEOUT",value);
}

void gem::hw::amc::HwGenericAMC::setDAQLinkRunType(uint32_t const& value)
{
  return writeReg("GEM_AMC.DAQ.EXT_CONTROL.RUN_TYPE",value);
}

void gem::hw::amc::HwGenericAMC::setDAQLinkRunParameters(uint32_t const& value)
{
  return writeReg("GEM_AMC.DAQ.EXT_CONTROL.RUN_PARAMS",value);
}

void gem::hw::amc::HwGenericAMC::setDAQLinkRunParameter(uint8_t const& parameter, uint8_t const& value)
{
  if (parameter < 1 || parameter > 3) {
    std::string msg = toolbox::toString("Attempting to set DAQ link run parameter %d: outside expectation (1-%d)",
                                        (int)parameter,3);
    CMSGEMOS_ERROR(msg);
    return;
  }
  std::stringstream regBase;
  regBase << "DAQ.EXT_CONTROL.RUN_PARAM" << (int) parameter;
  writeReg("GEM_AMC."+regBase.str(),value);
}

/********************************/
/** TTC module information **/
/********************************/

/** TTC module functions **/
void gem::hw::amc::HwGenericAMC::ttcModuleReset()
{
  writeReg("GEM_AMC.TTC.CTRL.MODULE_RESET", 0x1);
}

void gem::hw::amc::HwGenericAMC::ttcMMCMReset()
{
  writeReg("GEM_AMC.TTC.CTRL.MMCM_RESET", 0x1);
  // writeReg("GEM_AMC.TTC.CTRL.PHASE_ALIGNMENT_RESET", 0x1);
}

void gem::hw::amc::HwGenericAMC::ttcMMCMPhaseShift(bool relock, bool modeBC0, bool scan)
{
  try {
    req = wisc::RPCMsg("amc.ttcMMCMPhaseShift");
    req.set_word("relock",  relock);
    req.set_word("modeBC0", modeBC0);
    req.set_word("scan",    scan);
    try {
      rsp = rpc.call_method(req);
      try {
        if (rsp.get_key_exists("error")) {
          std::stringstream errmsg;
          errmsg << rsp.get_string("error");
          XCEPT_RAISE(gem::hw::devices::exception::RPCMethodError, errmsg.str());
        }
      } STANDARD_CATCH;
    } STANDARD_CATCH;
  } GEM_CATCH_RPC_ERROR("HwGenericAMC::ttcMMCMPhaseShift", gem::hw::devices::exception::Exception);
}

int gem::hw::amc::HwGenericAMC::checkPLLLock(uint32_t readAttempts)
{
  try {
    req = wisc::RPCMsg("amc.checkPLLLock");
    req.set_word("readAttempts", readAttempts);
    try {
      rsp = rpc.call_method(req);
      try {
        if (rsp.get_key_exists("error")) {
          std::stringstream errmsg;
          errmsg << rsp.get_string("error");
          XCEPT_RAISE(gem::hw::devices::exception::RPCMethodError, errmsg.str());
        }
      } STANDARD_CATCH;
      return rsp.get_word("lockCnt");
    } STANDARD_CATCH;
  } GEM_CATCH_RPC_ERROR("HwGenericAMC::checkPLLLock", gem::hw::devices::exception::Exception);
}

double gem::hw::amc::HwGenericAMC::getMMCMPhaseMean(uint32_t readAttempts)
{
  if (readAttempts == 1) {
    return double(readReg("GEM_AMC.TTC.STATUS.CLK.TTC_PM_PHASE_MEAN"));
  } else {
    try {
      req = wisc::RPCMsg("amc.getMMCMPhaseMean");
      req.set_word("reads", readAttempts);
      try {
        rsp = rpc.call_method(req);
        try {
          if (rsp.get_key_exists("error")) {
            std::stringstream errmsg;
            errmsg << rsp.get_string("error");
            XCEPT_RAISE(gem::hw::devices::exception::RPCMethodError, errmsg.str());
          }
        } STANDARD_CATCH;
        return rsp.get_word("phase");
      } STANDARD_CATCH;
    } GEM_CATCH_RPC_ERROR("HwGenericAMC::getMMCMPhaseMean", gem::hw::devices::exception::Exception);
  }
}

double gem::hw::amc::HwGenericAMC::getMMCMPhaseMedian(uint32_t readAttempts)
{
  CMSGEMOS_WARN("HwGenericAMC::getMMCMPhaseMedian is not implemented, returning the mean");
  return getMMCMPhaseMean(readAttempts);
}

double gem::hw::amc::HwGenericAMC::getGTHPhaseMean(uint32_t readAttempts)
{
  if (readAttempts == 1) {
    return readReg("GEM_AMC.TTC.STATUS.CLK.GTH_PM_PHASE_MEAN");
  } else {
    try {
      req = wisc::RPCMsg("amc.getGTHPhaseMean");
      req.set_word("reads", readAttempts);
      try {
        rsp = rpc.call_method(req);
        try {
          if (rsp.get_key_exists("error")) {
            std::stringstream errmsg;
            errmsg << rsp.get_string("error");
            XCEPT_RAISE(gem::hw::devices::exception::RPCMethodError, errmsg.str());
          }
        } STANDARD_CATCH;
        return rsp.get_word("phase");
      } STANDARD_CATCH;
    } GEM_CATCH_RPC_ERROR("HwGenericAMC::getGTHPhaseMean", gem::hw::devices::exception::Exception);
  }
}

double gem::hw::amc::HwGenericAMC::getGTHPhaseMedian(uint32_t readAttempts)
{
  CMSGEMOS_WARN("HwGenericAMC::getGTHPhaseMedian is not implemented, returning the mean");
  return getGTHPhaseMean(readAttempts);
}

void gem::hw::amc::HwGenericAMC::ttcCounterReset()
{
  writeReg("GEM_AMC.TTC.CTRL.CNT_RESET", 0x1);
}

bool gem::hw::amc::HwGenericAMC::getL1AEnable()
{
  return readReg("GEM_AMC.TTC.CTRL.L1A_ENABLE");
}

void gem::hw::amc::HwGenericAMC::setL1AEnable(bool enable)
{
  // uint32_t safeEnable = 0xa4a2c200+int(enable);
  writeReg("GEM_AMC.TTC.CTRL.L1A_ENABLE", uint32_t(enable));
}

uint32_t gem::hw::amc::HwGenericAMC::getTTCConfig(AMCTTCCommandT const& cmd)
{
  CMSGEMOS_WARN("HwGenericAMC::getTTCConfig: not implemented");
  return 0x0;
}

void gem::hw::amc::HwGenericAMC::setTTCConfig(AMCTTCCommandT const& cmd, uint8_t const& value)
{
  CMSGEMOS_WARN("HwGenericAMC::setTTCConfig: not implemented");
  return;
}

uint32_t gem::hw::amc::HwGenericAMC::getTTCStatus()
{
  CMSGEMOS_WARN("HwGenericAMC::getTTCStatus: not fully implemented");
  return readReg("GEM_AMC.TTC.STATUS");
}

uint32_t gem::hw::amc::HwGenericAMC::getTTCErrorCount(bool const& single)
{
  if (single)
    return readReg("GEM_AMC.TTC.STATUS.TTC_SINGLE_ERROR_CNT");
  else
    return readReg("GEM_AMC.TTC.STATUS.TTC_DOUBLE_ERROR_CNT");
}

uint32_t gem::hw::amc::HwGenericAMC::getTTCCounter(AMCTTCCommandT const& cmd)
{
  switch(cmd) {
  case(AMCTTCCommand::TTC_L1A) :
    return readReg("GEM_AMC.TTC.CMD_COUNTERS.L1A");
  case(AMCTTCCommand::TTC_BC0) :
    return readReg("GEM_AMC.TTC.CMD_COUNTERS.BC0");
  case(AMCTTCCommand::TTC_EC0) :
    return readReg("GEM_AMC.TTC.CMD_COUNTERS.EC0");
  case(AMCTTCCommand::TTC_RESYNC) :
    return readReg("GEM_AMC.TTC.CMD_COUNTERS.RESYNC");
  case(AMCTTCCommand::TTC_OC0) :
    return readReg("GEM_AMC.TTC.CMD_COUNTERS.OC0");
  case(AMCTTCCommand::TTC_HARD_RESET) :
    return readReg("GEM_AMC.TTC.CMD_COUNTERS.HARD_RESET");
  case(AMCTTCCommand::TTC_CALPULSE) :
    return readReg("GEM_AMC.TTC.CMD_COUNTERS.CALPULSE");
  case(AMCTTCCommand::TTC_START) :
    return readReg("GEM_AMC.TTC.CMD_COUNTERS.START");
  case(AMCTTCCommand::TTC_STOP) :
    return readReg("GEM_AMC.TTC.CMD_COUNTERS.STOP");
  case(AMCTTCCommand::TTC_TEST_SYNC) :
    return readReg("GEM_AMC.TTC.CMD_COUNTERS.TEST_SYNC");
  default :
    return readReg("GEM_AMC.TTC.CMD_COUNTERS.L1A");
  }
}

uint32_t gem::hw::amc::HwGenericAMC::getL1AID()
{
  return readReg("GEM_AMC.TTC.L1A_ID");
}

uint32_t gem::hw::amc::HwGenericAMC::getL1ARate()
{
  return readReg("GEM_AMC.TTC.L1A_RATE");
}

uint32_t gem::hw::amc::HwGenericAMC::getTTCSpyBuffer()
{
  // FIXME: OBSOLETE in V3?
  CMSGEMOS_WARN("HwGenericAMC::getTTCSpyBuffer: TTC.TTC_SPY_BUFFER is obsolete and will be removed in a future release");
  return 0x0;
  // return readReg("GEM_AMC.TTC.TTC_SPY_BUFFER");
}

/********************************/
/** SLOW_CONTROL module information **/
/********************************/

/*** SCA submodule ***/
void gem::hw::amc::HwGenericAMC::scaHardResetEnable(bool const& en)
{
  writeReg("GEM_AMC.SLOW_CONTROL.SCA.CTRL.TTC_HARD_RESET_EN", uint32_t(en));
}

/********************************/
/** TRIGGER module information **/
/********************************/

void gem::hw::amc::HwGenericAMC::triggerReset()
{
  writeReg("GEM_AMC.TRIGGER.CTRL.MODULE_RESET", 0x1);
}

void gem::hw::amc::HwGenericAMC::triggerCounterReset()
{
  writeReg("GEM_AMC.TRIGGER.CTRL.CNT_RESET", 0x1);
}

uint32_t gem::hw::amc::HwGenericAMC::getOptoHybridKillMask()
{
  return readReg("GEM_AMC.TRIGGER.CTRL.OH_KILL_MASK");
}

void gem::hw::amc::HwGenericAMC::setOptoHybridKillMask(uint32_t const& mask)
{
  writeReg("GEM_AMC.TRIGGER.CTRL.OH_KILL_MASK", mask);
}

/*** STATUS submodule ***/
uint32_t gem::hw::amc::HwGenericAMC::getORTriggerRate()
{
  return readReg("GEM_AMC.TRIGGER.STATUS.OR_TRIGGER_RATE");
}

uint32_t gem::hw::amc::HwGenericAMC::getORTriggerCount()
{
  return readReg("GEM_AMC.TRIGGER.STATUS.TRIGGER_SINGLE_ERROR_CNT");
}

/*** OH{IDXX} submodule ***/
uint32_t gem::hw::amc::HwGenericAMC::getOptoHybridTriggerRate(uint8_t const& oh)
{
  return readReg("GEM_AMC."+toolbox::toString("TRIGGER.OH%d.TRIGGER_RATE",(int)oh));
}

uint32_t gem::hw::amc::HwGenericAMC::getOptoHybridTriggerCount(uint8_t const& oh)
{
  return readReg("GEM_AMC."+toolbox::toString("TRIGGER.OH%d.TRIGGER_CNT",(int)oh));
}

uint32_t gem::hw::amc::HwGenericAMC::getOptoHybridClusterRate(uint8_t const& oh, uint8_t const& cs)
{
  return readReg("GEM_AMC."+toolbox::toString("TRIGGER.OH%d.CLUSTER_SIZE_%d_RATE",(int)oh,(int)cs));
}

uint32_t gem::hw::amc::HwGenericAMC::getOptoHybridClusterCount(uint8_t const& oh, uint8_t const& cs)
{
  return readReg("GEM_AMC."+toolbox::toString("TRIGGER.OH%d.CLUSTER_SIZE_%d_CNT",(int)oh,(int)cs));
}

uint32_t gem::hw::amc::HwGenericAMC::getOptoHybridDebugLastCluster(uint8_t const& oh, uint8_t const& cs)
{
  return readReg("GEM_AMC."+toolbox::toString("TRIGGER.OH%d.DEBUG_LAST_CLUSTER_%d",(int)oh,(int)cs));
}

uint32_t gem::hw::amc::HwGenericAMC::getOptoHybridTriggerLinkCount(uint8_t const& oh, uint8_t const& link, AMCOHLinkCountT const& count)
{
  switch(count) {
  case(AMCOHLinkCount::LINK_NOT_VALID) :
    return readReg("GEM_AMC."+toolbox::toString("TRIGGER.OH%d.LINK%d_NOT_VALID_CNT",(int)oh,(int)link));
  case(AMCOHLinkCount::LINK_MISSED_COMMA) :
    return readReg("GEM_AMC."+toolbox::toString("TRIGGER.OH%d.LINK%d_MISSED_COMMA_CNT",(int)oh,(int)link));
  case(AMCOHLinkCount::LINK_OVERFLOW) :
    return readReg("GEM_AMC."+toolbox::toString("TRIGGER.OH%d.LINK%d_OVERFLOW_CNT",(int)oh,(int)link));
  case(AMCOHLinkCount::LINK_UNDERFLOW) :
    return readReg("GEM_AMC."+toolbox::toString("TRIGGER.OH%d.LINK%d_UNDERFLOW_CNT",(int)oh,(int)link));
  case(AMCOHLinkCount::LINK_SYNC_WORD) :
    return readReg("GEM_AMC."+toolbox::toString("TRIGGER.OH%d.LINK%d_SYNC_WORD_CNT",(int)oh,(int)link));
  default :
    return readReg("GEM_AMC."+toolbox::toString("TRIGGER.OH%d.LINK%d_MISSED_COMMA_CNT",(int)oh,(int)link));
  }
}


// general resets
void gem::hw::amc::HwGenericAMC::generalReset()
{
  // TODO: CTP7 module candidate
  // reset all counters
  counterReset();

  for (unsigned gtx = 0; gtx < m_maxLinks; ++gtx)
    linkReset(gtx);

  // other resets

  return;
}

void gem::hw::amc::HwGenericAMC::counterReset()
{
  // TODO: CTP7 module candidate
  // reset all counters
  resetT1Counters();

  linkCounterReset();

  return;
}

void gem::hw::amc::HwGenericAMC::resetT1Counters()
{
  // TODO: CTP7 module candidate
  // FIXME: OBSOLETE in V3
  CMSGEMOS_WARN("HwGenericAMC::resetT1Counters is obsolete and will be removed in a future release");
  return;
}

void gem::hw::amc::HwGenericAMC::linkCounterReset()
{
  // TODO: CTP7 module candidate
  // FIXME: OBSOLETE in V3?
  CMSGEMOS_WARN("HwGenericAMC::linkCounterReset: not yet implemented");
  return;
}

void gem::hw::amc::HwGenericAMC::linkReset(uint8_t const& gtx)
{
  // TODO: CTP7 module candidate
  // req = wisc::RPCMsg("amc.linkReset");
  // req.set_word("NOH", static_cast<uint32_t>(gtx));
  // try {
  //   rsp = rpc.call_method(req);
  // } STANDARD_CATCH;

  // try {
  //   if (rsp.get_key_exists("error")) {
  //     ERROR("LinkReset error: " << rsp.get_string("error").c_str());
  //     //throw xhal::common::utils::XHALException("DAQ_TRIGGER_MAIN update failed");
  //   }
  // } STANDARD_CATCH;

  CMSGEMOS_WARN("HwGenericAMC::linkReset: not yet implemented");
  linkCounterReset();
  return;
}
