#include <bitset>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <functional>

#include "gem/hw/devices/optohybrid/HwOptoHybrid.h"
#include "gem/hw/devices/amc/HwGenericAMC.h"

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(std::string const& optohybridDevice) :
  gem::hw::GEMHwDevice::GEMHwDevice(optohybridDevice),
  b_is_initial(true),
  m_link(extractDeviceID(optohybridDevice,3))
{
  this->setup(optohybridDevice);
  std::stringstream basenode;
  basenode << "GEM_AMC.OH.OH" << static_cast<uint32_t>(m_link);
  CMSGEMOS_INFO("HwOptoHybrid ctor done (basenode "
                << basenode.str() << ") " << isHwConnected());
}

gem::hw::optohybrid::HwOptoHybrid::~HwOptoHybrid()
{
  // releaseDevice();
  // Disable connection to RPC service?
}

void gem::hw::optohybrid::HwOptoHybrid::connectRPC(bool reconnect)
{
  if (isConnected) {
    this->loadModule("utils", "utils v1.0.1");
    this->loadModule("optohybrid", "optohybrid v1.0.1");
    this->loadModule("vfat3",      "vfat3 v1.0.1");
    this->loadModule("gbt",        "gbt v1.0.1");
    CMSGEMOS_DEBUG("HwOptoHybrid::connectRPC modules loaded");
  } else {
    // FIXME, should we throw? fail?
    CMSGEMOS_WARN("HwOptoHybrid::connectRPC RPC interface failed to connect");
  }
}

bool gem::hw::optohybrid::HwOptoHybrid::isHwConnected()
{
  if (b_is_connected) {
    CMSGEMOS_DEBUG("HwOptoHybrid connection good");
    return true;
  } else if (gem::hw::GEMHwDevice::isHwConnected()) {
    CMSGEMOS_DEBUG("Checking hardware connection");

    // FIXME, need better check of connectivity...
    if ((this->getFirmwareDateString()).rfind("15") != std::string::npos ||
        (this->getFirmwareDateString()).rfind("16") != std::string::npos ||
        (this->getFirmwareDateString()).rfind("17") != std::string::npos ||
        (this->getFirmwareDateString()).rfind("18") != std::string::npos ||
        (this->getFirmwareDateString()).rfind("19") != std::string::npos) {
      b_is_connected = true;
      CMSGEMOS_INFO("OptoHybrid present ("
                    << this->getFirmwareVersionString() << "/0x"
                    << std::hex << this->getFirmwareDate() << std::dec << ")");
      return true;
    } else {
      b_is_connected = false;
      CMSGEMOS_DEBUG("OptoHybrid not reachable (unable to find 15, 16, 17, 18, or 19 in the firmware string)."
                     << " Obviously we need a better strategy to check connectivity");
      return false;
    }
  }
  // shouldn't get to here unless HW isn't connected
  CMSGEMOS_DEBUG("OptoHybrid not reachable (!b_is_connected && !GEMHwDevice::isHwConnnected)");
  return false;
}


// std::string gem::hw::optohybrid::HwOptoHybrid::getBoardID() const;


//uint32_t gem::hw::optohybrid::HwOptoHybrid::readTriggerData()
//{
//  return uint32_t value;
//}


uint32_t gem::hw::optohybrid::HwOptoHybrid::getFirmwareVersion()
{
  uint32_t fwver = readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".FPGA.CONTROL.RELEASE.VERSION");
  CMSGEMOS_TRACE("OH has firmware version 0x" << std::hex << fwver << std::dec << std::endl);
  return fwver;
}

std::string gem::hw::optohybrid::HwOptoHybrid::getFirmwareVersionString()
{
  std::stringstream retval;
  uint32_t fwver = getFirmwareVersion();
  retval << std::hex
         << ((fwver)     & 0xff) << "."
         << ((fwver>>8)  & 0xff) << "."
         << ((fwver>>16) & 0xff) << "."
         << ((fwver>>24) & 0xff)
         << std::dec;
  return retval.str();
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFirmwareDate()
{
  uint32_t fwver = readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".FPGA.CONTROL.RELEASE.DATE");
  CMSGEMOS_TRACE("OH has firmware date 0x" << std::hex << fwver << std::dec << std::endl);
  return fwver;
}

std::string gem::hw::optohybrid::HwOptoHybrid::getFirmwareDateString()
{
  std::stringstream retval;
  uint32_t rawDate = getFirmwareDate();
  retval << std::hex << std::setw(2) << std::setfill('0') << ((rawDate)&0xff)       << std::dec << "/"
         << std::hex << std::setw(2) << std::setfill('0') << ((rawDate>>8)&0xff)    << std::dec << "/"
         << std::hex << std::setw(4) << std::setfill('0') << ((rawDate>>16)&0xffff) << std::dec;
  return retval.str();
}

std::vector<uint32_t> gem::hw::optohybrid::HwOptoHybrid::broadcastRead(std::string const& name,
                                                                       uint32_t    const& mask,
                                                                       bool        const& reset)
{
  try {
    auto res = xhal::common::rpc::call<::oh::broadcastRead>(rpc, static_cast<uint32_t>(m_link), name, mask); 
    return res;
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::broadcastRead", gem::hw::devices::exception::Exception);
}

void gem::hw::optohybrid::HwOptoHybrid::broadcastWrite(std::string const& name,
                                                       uint32_t    const& value,
                                                       uint32_t    const& mask,
                                                       bool        const& reset)
{
  try {
    xhal::common::rpc::call<::oh::broadcastWrite>(rpc, static_cast<uint32_t>(m_link), name, value, mask); 
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::broadcastWrite", gem::hw::devices::exception::Exception);
}


std::vector<std::pair<uint8_t, uint32_t> > gem::hw::optohybrid::HwOptoHybrid::getConnectedVFATs(bool update)
{
  // FIXME UPDATE WITH NEW COMM PROTOCOL
  if (update || b_is_initial) {
    std::vector<uint32_t> loc_chipIDs(::oh::VFATS_PER_OH,0xdeaddead);
    try {
      loc_chipIDs = xhal::common::rpc::call<::vfat3::getVFAT3ChipIDs>(rpc, static_cast<uint32_t>(m_link), 0x0, false); //FIXME hardcoded mask (obsolete param) and rawID
    } GEM_CATCH_RPC_ERROR("HwOptoHybrid::getVFAT3ChipIDs", gem::hw::devices::exception::Exception);

    // std::vector<uint32_t> loc_chipIDs = broadcastRead("HW_CHIP_ID", gem::hw::utils::ALL_VFATS_BCAST_MASK, false);

    std::vector<std::pair<uint8_t, uint32_t> > chipIDs;

    size_t idx = 0;
    for (auto const& chip : loc_chipIDs) {
      uint32_t chipID = chip;
      CMSGEMOS_DEBUG("HwOptoHybrid::getConnectedVFATs GEB slot: " << static_cast<uint32_t>(idx)
                     << ", chipID: 0x" << std::hex << std::setw(8) << std::setfill('0') << chipID << std::dec);
      chipIDs.push_back(std::make_pair(idx, chipID));
      ++idx;
    }
    m_chipIDs    = chipIDs;
    b_is_initial = false;
  }
  return m_chipIDs;
}


uint32_t gem::hw::optohybrid::HwOptoHybrid::getConnectedVFATMask(bool update)
{
  if (update || b_is_initial) {
    // change to vfatSyncCheck?
    uint32_t goodVFATs = 0x0;
    try {
      goodVFATs = xhal::common::rpc::call<::vfat3::vfatSyncCheck>(rpc, static_cast<uint32_t>(m_link), 0x0); //FIXME hardcoded mask
    } GEM_CATCH_RPC_ERROR("HwOptoHybrid::getConnectedVFATMask", gem::hw::devices::exception::Exception);

    uint32_t connectedMask = goodVFATs;      // FIXME high means broadcast, present
    uint32_t disabledMask  = ~connectedMask; // FIXME high means ignore data

    connectedMask |= gem::hw::utils::ALL_VFATS_BCAST_MASK;
    disabledMask  |= gem::hw::utils::ALL_VFATS_BCAST_MASK;

    CMSGEMOS_DEBUG("HwOptoHybrid::getConnectedVFATMask final mask is 0x"
                   << std::setw(8) << std::setfill('0') << std::hex << connectedMask << std::dec);

    m_connectedMask = connectedMask;
  }
  return m_connectedMask;
}


void gem::hw::optohybrid::HwOptoHybrid::setVFATsToDefaults()
{
  CMSGEMOS_WARN("HwOptoHybrid::setVFATsToDefaults functionality is replaced with HwOptoHybrid::configureVFATs");
  configureVFATs();
}


void gem::hw::optohybrid::HwOptoHybrid::configureVFATs()
{
  try {
    xhal::common::rpc::call<::vfat3::configureVFAT3s>(rpc, static_cast<uint32_t>(m_link), ~m_connectedMask); //FIXME remove requirement for mask TODO implement option for RAM loading
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::configureVFATs", gem::hw::devices::exception::Exception);
}


void gem::hw::optohybrid::HwOptoHybrid::setVFATsToDefaults(std::map<std::string, uint16_t> const& regvals)
{
  CMSGEMOS_WARN("HwOptoHybrid::setVFATsToDefaults functionality is replaced with HwOptoHybrid::configureVFATs");
  configureVFATs(regvals);
}

void gem::hw::optohybrid::HwOptoHybrid::configureVFATs(std::map<std::string, uint16_t> const& regvals)
{
  //FIXME not implemented. Do we actually need it??
  //try {
  //  req = wisc::RPCMsg("vfat3.configureVFAT3s");
  //  req.set_word("ohN", static_cast<uint32_t>(m_link));
  //  // req.set_word("vfatMask", m_connectedMask); // FIXME REMOVE
  //  req.set_word("useRAM", false);
  //  std::array<uint32_t,24*74> cfgdata{0};
  //  req.set_binarydata("config", cfgdata.data(), cfgdata.size());
  //  try {
  //    rsp = rpc.call_method(req);
  //  } STANDARD_CATCH;
  //  checkRPCResponse("HwOptoHybrid::configureVFATs");
  //} GEM_CATCH_RPC_ERROR("HwOptoHybrid::configureVFATs", gem::hw::devices::exception::Exception);
}


void gem::hw::optohybrid::HwOptoHybrid::configureGBT(uint8_t const& gbtID, uint32_t const* gbtcfg)
{
  gbt::config_t t_gbtconfig = {*(uint8_t*) gbtcfg};
  try {
    xhal::common::rpc::call<::gbt::writeGBTConfig>(rpc, static_cast<uint32_t>(m_link), static_cast<uint32_t>(gbtID), t_gbtconfig);
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::configureAllGBTs", gem::hw::devices::exception::Exception);
}


void gem::hw::optohybrid::HwOptoHybrid::configureGBT(uint8_t const& gbtID, std::array<const uint32_t, 92> const& gbtcfg)
{
  configureGBT(gbtID, gbtcfg.data());
}


void gem::hw::optohybrid::HwOptoHybrid::configureAllGBTs(uint32_t const* gbtcfg)
{
  //FIXME not implemented on the RPC side, also not used anywhere in cmsgemos
//  try {
//    req = wisc::RPCMsg("gbt.writeAllGBTConfigs");
//    req.set_word("ohN", static_cast<uint32_t>(m_link));
//    req.set_word("useRAM", false);
//    // std::array<uint32_t, 3*92> cfgdata{0};
//    // cfgdata.resize(3*92);
//    // req.set_binarydata("config", cfgdata.data(), cfgdata.size());
//    req.set_binarydata("config", gbtcfg, 3*92); // FIXME how to guarantee size?
//    /**
//    // or, with one blob per GBTx
//    std::array<std::array<uint32_t, 92>, 3> cfgdata{};
//    req.set_binarydata("gbt0", cfgdata.at(0).data(), cfgdata.at(0).size());
//    req.set_binarydata("gbt1", cfgdata.at(1).data(), cfgdata.at(1).size());
//    req.set_binarydata("gbt2", cfgdata.at(2).data(), cfgdata.at(2).size());
//    */
//    try {
//      rsp = rpc.call_method(req);
//    } STANDARD_CATCH;
//    checkRPCResponse("HwOptoHybrid::configureAllGBTs");
//  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::configureAllGBTs", gem::hw::devices::exception::Exception);
}


void gem::hw::optohybrid::HwOptoHybrid::configureAllGBTs(std::array<const uint32_t, 3*92> const& gbtcfg)
{
  //FIXME not implemented on the RPC side, also not used anywhere in cmsgemos
  configureAllGBTs(gbtcfg.data());
}


void gem::hw::optohybrid::HwOptoHybrid::generalReset()
{
  CMSGEMOS_WARN("HwOptoHybrid::generalReset functionality is not fully implemented");
  counterReset();
  return;
}

void gem::hw::optohybrid::HwOptoHybrid::counterReset()
{
  CMSGEMOS_WARN("HwOptoHybrid::counterReset functionality is not fully implemented");
  resetTTCCounters();
  // other counter resets?
  return;
}

void gem::hw::optohybrid::HwOptoHybrid::linkReset(uint8_t const& link)
{
  CMSGEMOS_WARN("HwOptoHybrid::linkReset functionality is not implemented");
  return;
}

//////// VFAT \\\\\\\\*
uint32_t gem::hw::optohybrid::HwOptoHybrid::getVFATMask()
{
  // FIXME candidate for HwGenericAMC?
  std::stringstream regName;
  regName << "GEM_AMC.OH_LINKS.OH" << static_cast<int>(m_link) << ".VFAT_MASK";
  return readReg(regName.str());
}

void gem::hw::optohybrid::HwOptoHybrid::setVFATMask(uint32_t const mask)
{
  // FIXME candidate for HwGenericAMC?
  CMSGEMOS_DEBUG("HwOptoHybrid::setVFATMask setting VFAT mask to "
                 << std::hex << std::setw(8) << std::setfill('0') << mask << std::dec);
  std::stringstream regName;
  regName << "GEM_AMC.OH_LINKS.OH" << static_cast<int>(m_link) << ".VFAT_MASK";
  return writeReg(regName.str(), mask&0x00ffffff);
}

void gem::hw::optohybrid::HwOptoHybrid::resetVFATs()
{
  return writeReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".FPGA.CONTROL.VFAT.RESET",0x1);
}

void gem::hw::optohybrid::HwOptoHybrid::setSBitMask(uint32_t const mask)
{
  writeReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".FPGA.TRIG.CTRL.VFAT_MASK", mask&0x00ffffff);
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getSBitMask()
{
  return readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".FPGA.TRIG.CTRL.VFAT_MASK");
}

//// HDMI_OUTPUT \\\\*
void gem::hw::optohybrid::HwOptoHybrid::setHDMISBitSource(uint8_t const& bit, uint8_t const& mode)
{
  writeReg("GEM_AMC.OH.OH"+std::to_string(m_link)+"."+toolbox::toString("FPGA.CONTROL.HDMI.SBIT_SEL%i",static_cast<uint32_t>(bit)),mode);
}

void gem::hw::optohybrid::HwOptoHybrid::setHDMISBitSource(std::array<uint8_t, 8> const& sources)
{
  for (uint8_t i = 0; i < 8; ++i)
    setHDMISBitSource(i, sources[i]);
};

uint32_t gem::hw::optohybrid::HwOptoHybrid::getHDMISBitSource(uint8_t const& bit)
{
  return readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+"."+toolbox::toString("FPGA.CONTROL.HDMI.SBIT_SEL%i",static_cast<uint32_t>(bit)));
}

void gem::hw::optohybrid::HwOptoHybrid::setHDMISBitMode(uint8_t const& bit, uint8_t const& mode)
{
  writeReg("GEM_AMC.OH.OH"+std::to_string(m_link)+"."+toolbox::toString("FPGA.CONTROL.HDMI.SBIT_MODE%i",static_cast<uint32_t>(bit)), mode);
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getHDMISBitMode(uint8_t const& bit)
{
  return readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+"."+toolbox::toString("FPGA.CONTROL.HDMI.SBIT_MODE%i",static_cast<uint32_t>(bit)));
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getClockStatus()
{
  return readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".FPGA.CLOCKING");
}

//////// Firmware Status \\\\\\\\*
bool gem::hw::optohybrid::HwOptoHybrid::hasSEUError()
{
  return readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".STATUS.SEU");
}

//////// OptoHybrid ADC Interface \\\\\\\\*
// FIXME UPDATE FOR V3
uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGATemp()
{
  return readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".ADC.TEMP");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGAMaxTemp()
{
  return readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".ADC.TEMP.MAX");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGAMinTemp()
{
  return readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".ADC.TEMP.MIN");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGAVccInt()
{
  return readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".ADC.VCCINT");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGAMaxVccInt()
{
  return readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".ADC.VCCINT.MAX");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGAMinVccInt()
{
  return readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".ADC.VCCINT.MIN");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGAVccAux()
{
  return readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".ADC.VCCAUX");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGAMaxVccAux()
{
  return readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".ADC.VCCAUX.MAX");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGAMinVccAux()
{
  return readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".ADC.VCCAUX.MIN");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getADCVPVN() const
{
  CMSGEMOS_WARN("HwOptoHybrid::getADCVPVN functionality is not implemented");
  return 0xdead;
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getADCVAUX(uint8_t const& vaux) const
{
  CMSGEMOS_WARN("HwOptoHybrid::getADCVAUX functionality is not implemented");
  return 0xdead;
}

// FIXME UPDATE FOR V3
uint32_t gem::hw::optohybrid::HwOptoHybrid::getVFATDACOutV(uint8_t const& column) const
{
  CMSGEMOS_WARN("HwOptoHybrid::getVFATDACOutV functionality is not implemented");
  return 0xdead;
}

// FIXME OBSOLETE IN V3
uint32_t gem::hw::optohybrid::HwOptoHybrid::getVFATDACOutI(uint8_t const& column) const
{
  CMSGEMOS_WARN("HwOptoHybrid::getVFATDACOutI functionality is not implemented");
  return 0xdead;
}

//////// Scan Modules \\\\\\\\*
void gem::hw::optohybrid::HwOptoHybrid::configureScanModule(uint8_t const& mode, uint32_t const& chip, uint8_t const& channel,
                                                            uint8_t const& min,  uint8_t const& max,
                                                            uint8_t const& step, uint32_t const& nevts,
                                                            bool useUltra, bool reset)
{
  try {
    xhal::common::rpc::call<::oh::configureScanModule>(rpc, static_cast<uint32_t>(m_link), chip, mode, useUltra, chip, channel, nevts, min, max, step); //FIXME chip is used twice, need a redesign. Probably won't work, but will compile at the moment.
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::startScanModule", gem::hw::devices::exception::Exception);
}

//FIXME review passed arguments: nevts is not used here.
void gem::hw::optohybrid::HwOptoHybrid::startScanModule(uint32_t const& nevts, bool useUltra)
{
  try {
    xhal::common::rpc::call<::oh::startScanModule>(rpc, static_cast<uint32_t>(m_link), useUltra);
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::startScanModule", gem::hw::devices::exception::Exception);
}

std::vector<uint32_t> gem::hw::optohybrid::HwOptoHybrid::getScanResults(uint32_t const& nevts,
                                                                        uint32_t const& min,
                                                                        uint32_t const& max,
                                                                        uint32_t const& step)
{
  // FIXME OBSOLETE IN V3
  CMSGEMOS_WARN("HwOptoHybrid::getScanResults is obsolete");
  return getUltraScanResults(nevts, min, max, step).at(0);
}

std::vector<std::vector<uint32_t> > gem::hw::optohybrid::HwOptoHybrid::getUltraScanResults(uint32_t const& nevts,
                                                                                           uint32_t const& min,
                                                                                           uint32_t const& max,
                                                                                           uint32_t const& step)
{
  //FIXME this method is deprecated at ctp7 modules side and return types do not match. Won't work
  //try {
  //  auto res = xhal::common::rpc::call<::oh::getUltraScanResults>(rpc, static_cast<uint32_t>(m_link), nevts, min, max, step);
  //  return res;
  //} GEM_CATCH_RPC_ERROR("HwOptoHybrid::getUltraScanResults", gem::hw::devices::exception::Exception);
  std::vector<std::vector<uint32_t>> res;
  return res;
}


//////// Firmware forced TTC commands \\\\\\\\*
void gem::hw::optohybrid::HwOptoHybrid::sendL1A(uint32_t const& ntrigs, uint32_t const& rate)
{
  writeReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".FPGA.GBT.FORCE_L1A",0x1);
}

void gem::hw::optohybrid::HwOptoHybrid::sendResync(uint32_t const& nresync, uint32_t const& rate)
{
  writeReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".FPGA.GBT.FORCE_RESYNC",0x1);
}


void gem::hw::optohybrid::HwOptoHybrid::sendBC0(uint32_t const& nbc0, uint32_t const& rate)
{
  writeReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".FPGA.GBT.FORCE_BC0",0x1);
}

void gem::hw::optohybrid::HwOptoHybrid::sendBXN(uint32_t const& nbc0, uint32_t const& rate)
{
  writeReg("GEM_AMC.OH.OH"+std::to_string(m_link)+".FPGA.GBT.FORCE_BXN",0x1);
}

//////// Counters \\\\\\\\*
//// TTC Counters \\\\*
void gem::hw::optohybrid::HwOptoHybrid::updateTTCCounters()
{
  for (unsigned signal = 0; signal < 4; ++signal) {
    getTTCCount(signal, 0x0);
  }
}

void gem::hw::optohybrid::HwOptoHybrid::resetTTCCounters()
{
  resetTTCCount(0x0, 0x6); //reset all L1A counters
  resetTTCCount(0x1, 0x6); //reset all Resync counters
  resetTTCCount(0x2, 0x6); //reset all BC0 counters
  resetTTCCount(0x3, 0x6); //reset all BXN counters
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getTTCCount(uint8_t const& signal, uint8_t const& mode)
{
  std::stringstream ttcSignal;
  if (signal == 0x0)
    ttcSignal << "L1A";
  if (signal == 0x1)
    ttcSignal << "Resync";
  if (signal == 0x2)
    ttcSignal << "BC0";
  if (signal == 0x3)
    ttcSignal << "BXN";

  switch(mode) {
  case(OptoHybridTTCMode::LOCAL_TTC):
    return readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+"."+toolbox::toString("COUNTERS.TTC.GTX_TTC.%s", (ttcSignal.str()).c_str()));
  case(OptoHybridTTCMode::RESET):
    return 0x0;
  default:
    return readReg("GEM_AMC.OH.OH"+std::to_string(m_link)+"."+toolbox::toString("COUNTERS.TTC.SENT.%s",    (ttcSignal.str()).c_str()));
  }
}

void gem::hw::optohybrid::HwOptoHybrid::resetTTCCount(uint8_t const& signal, uint8_t const& mode)
{
  std::stringstream ttcSignal;
  if (signal == 0x0)
    ttcSignal << "L1A";
  if (signal == 0x1)
    ttcSignal << "Resync";
  if (signal == 0x2)
    ttcSignal << "BC0";
  if (signal == 0x3)
    ttcSignal << "BXN";

  // CTP7 module candidate?
}
