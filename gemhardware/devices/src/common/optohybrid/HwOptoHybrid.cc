#include "gem/hw/optohybrid/HwOptoHybrid.h"

#include "gem/hw/HwGenericAMC.h"

#include <bitset>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <functional>

#include "gem/hw/devices/optohybrid/HwOptoHybrid.h"

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(std::string const& optohybridDevice,
                                                std::string const& connectionFile) :
  gem::hw::GEMHwDevice::GEMHwDevice(optohybridDevice, connectionFile),
  b_is_initial(true),
  m_link(extractDeviceID(optohybridDevice,3))
{
  this->setup(optohybridDevice);
  std::stringstream basenode;
  basenode << "GEM_AMC.OH.OH" << static_cast<uint32_t>(m_link);
  this->setDeviceBaseNode(basenode.str());
  CMSGEMOS_INFO("HwOptoHybrid ctor done (basenode "
                << basenode.str() << ") " << isHwConnected());
}

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(std::string const& optohybridDevice,
                                                std::string const& connectionURI,
                                                std::string const& addressTable) :
  gem::hw::GEMHwDevice::GEMHwDevice(optohybridDevice, connectionURI, addressTable),
  b_is_initial(true),
  m_link(extractDeviceID(optohybridDevice,3))
{
  this->setup(optohybridDevice);
  // FIXME HARDCODED OBSOLETE
  this->setAddressTableFileName(toolbox::toString("uhal_gem_amc_glib_link%02d.xml",m_link));
  std::stringstream basenode;
  basenode << "GEM_AMC.OH.OH" << static_cast<uint32_t>(m_link);
  this->setDeviceBaseNode(basenode.str());
  CMSGEMOS_INFO("HwOptoHybrid ctor done (basenode "
                << basenode.str() << ") " << isHwConnected());
}

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(std::string const& optohybridDevice,
                                                uhal::HwInterface& uhalDevice) :
  gem::hw::GEMHwDevice::GEMHwDevice(optohybridDevice,uhalDevice),
  b_is_initial(true),
  m_link(extractDeviceID(optohybridDevice,3))
{
  this->setup(optohybridDevice);
  std::stringstream basenode;
  basenode << "GEM_AMC.OH.OH" << static_cast<uint32_t>(m_link);
  this->setDeviceBaseNode(basenode.str());
  CMSGEMOS_INFO("HwOptoHybrid ctor done (basenode "
                << basenode.str() << ") " << isHwConnected());
}

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(gem::hw::HwGenericAMC const& amcDevice,
                                                uint8_t const& slot) :
  gem::hw::GEMHwDevice::GEMHwDevice(toolbox::toString("%s.OptoHybrid_%d",(amcDevice.getLoggerName()).c_str(),
                                                      static_cast<uint32_t>(slot)),
                                    dynamic_cast<uhal::HwInterface const&>(amcDevice)),
  b_is_initial(true),
  m_link(static_cast<uint32_t>(slot))
{
  CMSGEMOS_INFO("HwOptoHybrid creating OptoHybrid device from AMC device " << amcDevice.getLoggerName());
  // Use a connection file and connection manager?
  setDeviceID(toolbox::toString("%s.optohybrid%02d",amcDevice.getDeviceID().c_str(),static_cast<uint32_t>(m_link)));
  // uhal::ConnectionManager manager ( "file://${GEM_ADDRESS_TABLE_PATH}/connections.xml" );
  // p_gemConnectionManager.reset(new uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections.xml"));
  // p_gemHW.reset(new uhal::HwInterface(p_gemConnectionManager->getDevice(this->getDeviceID())));
  // p_gemConnectionManager = std::shared_ptr<uhal::ConnectionManager>(uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections.xml"));
  // p_gemHW = std::shared_ptr<uhal::HwInterface>(p_gemConnectionManager->getDevice(this->getDeviceID()));
  std::stringstream basenode;
  basenode << "GEM_AMC.OH.OH" << static_cast<uint32_t>(m_link);
  this->setDeviceBaseNode(basenode.str());
  CMSGEMOS_INFO("HwOptoHybrid ctor done (basenode "
                << basenode.str() << ") " << isHwConnected());
}

gem::hw::optohybrid::HwOptoHybrid::~HwOptoHybrid()
{
  // releaseDevice();
  // Disable connection to RPC service?
}

//void gem::hw::optohybrid::HwOptoHybrid::configureDevice(std::string const& xmlSettings)
//{
//  //here load the xml file settings onto the board
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::configureDevice(gem::config::OptoHybridBLOB const& blob)
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::configureDevice()
//{
//  // determine the manner in which to configure the device (XML or DB parameters)
//}
//

void gem::hw::optohybrid::HwOptoHybrid::connectRPC(bool reconnect)
{
  if (isConnected) {
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
  uint32_t fwver = readReg(getDeviceBaseNode(),"FPGA.CONTROL.RELEASE.VERSION");
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
  uint32_t fwver = readReg(getDeviceBaseNode(),"FPGA.CONTROL.RELEASE.DATE");
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
    req = wisc::RPCMsg("vfat3.broadcastRead");
    req.set_string("reg_name", name);
    req.set_word("ohN",      static_cast<uint32_t>(m_link));
    req.set_word("mask",     mask);
    try {
      rsp = rpc.call_method(req);
    } STANDARD_CATCH;
    checkRPCResponse("HwOptoHybrid::broadcastRead");
    uint32_t size = rsp.get_word_array_size("data");
    std::vector<uint32_t> data;
    data.resize(size);
    rsp.get_word_array("data", data.data());
    return data;
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::broadcastRead", gem::hw::optohybrid::exception::Exception);
}

void gem::hw::optohybrid::HwOptoHybrid::broadcastWrite(std::string const& name,
                                                       uint32_t    const& value,
                                                       uint32_t    const& mask,
                                                       bool        const& reset)
{
  try {
    req = wisc::RPCMsg("vfat3.broadcastWrite");
    req.set_string("reg_name", name);
    req.set_word("ohN",      static_cast<uint32_t>(m_link));
    req.set_word("mask",     mask);
    req.set_word("value",    value);
    try {
      rsp = rpc.call_method(req);
    } STANDARD_CATCH;
    checkRPCResponse("HwOptoHybrid::broadcastWrite");
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::broadcastWrite", gem::hw::optohybrid::exception::Exception);
}


std::vector<std::pair<uint8_t, uint32_t> > gem::hw::optohybrid::HwOptoHybrid::getConnectedVFATs(bool update)
{
  // FIXME UPDATE WITH NEW COMM PROTOCOL
  if (update || b_is_initial) {
    std::vector<uint32_t> loc_chipIDs;
    try {
      req = wisc::RPCMsg("vfat3.getVFAT3ChipIDs");
      req.set_word("ohN",   static_cast<uint32_t>(m_link));
      req.set_word("mask",  0x0); // FIXME mandatory, need to redesign ctp7_modules
      req.set_word("rawID", false);
      try {
        rsp = rpc.call_method(req);
      } STANDARD_CATCH;
      checkRPCResponse("HwOptoHybrid::getVFAT3ChipIDs");
      loc_chipIDs.resize(rsp.get_word_array_size("chipIDs")); // FIXME should be 24
      rsp.get_word_array("chipIDs", loc_chipIDs.data());
    } GEM_CATCH_RPC_ERROR("HwOptoHybrid::getVFAT3ChipIDs", gem::hw::optohybrid::exception::Exception);

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
      req = wisc::RPCMsg("vfat3.vfatSyncCheck");
      req.set_word("ohN",   static_cast<uint32_t>(m_link));
      try {
        rsp = rpc.call_method(req);
      } STANDARD_CATCH;
      checkRPCResponse("HwOptoHybrid::getConnectedVFATMask");
      goodVFATs = rsp.get_word("goodVFATs");
    } GEM_CATCH_RPC_ERROR("HwOptoHybrid::getConnectedVFATMask", gem::hw::optohybrid::exception::Exception);

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
    req = wisc::RPCMsg("vfat3.configureVFAT3s");
    req.set_word("ohN", static_cast<uint32_t>(m_link));
    req.set_word("vfatMask", ~m_connectedMask); // FIXME REMOVE
    req.set_word("useRAM", false); // FIXME false until RAM loading is implemented
    try {
      rsp = rpc.call_method(req);
    } STANDARD_CATCH;
    checkRPCResponse("HwOptoHybrid::configureVFATs");
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::configureVFATs", gem::hw::optohybrid::exception::Exception);
}


void gem::hw::optohybrid::HwOptoHybrid::setVFATsToDefaults(std::map<std::string, uint16_t> const& regvals)
{
  CMSGEMOS_WARN("HwOptoHybrid::setVFATsToDefaults functionality is replaced with HwOptoHybrid::configureVFATs");
  configureVFATs(regvals);
}

void gem::hw::optohybrid::HwOptoHybrid::configureVFATs(std::map<std::string, uint16_t> const& regvals)
{
  try {
    req = wisc::RPCMsg("vfat3.configureVFAT3s");
    req.set_word("ohN", static_cast<uint32_t>(m_link));
    // req.set_word("vfatMask", m_connectedMask); // FIXME REMOVE
    req.set_word("useRAM", false);
    std::array<uint32_t,24*74> cfgdata{0};
    req.set_binarydata("config", cfgdata.data(), cfgdata.size());
    try {
      rsp = rpc.call_method(req);
    } STANDARD_CATCH;
    checkRPCResponse("HwOptoHybrid::configureVFATs");
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::configureVFATs", gem::hw::optohybrid::exception::Exception);
}


void gem::hw::optohybrid::HwOptoHybrid::configureGBT(uint8_t const& gbtID, uint32_t const* gbtcfg)
{
  try {
    req = wisc::RPCMsg("gbt.writeGBTConfig");
    req.set_word("ohN",  static_cast<uint32_t>(m_link));
    req.set_word("gbtN", static_cast<uint32_t>(gbtID));
    req.set_word("useRAM", false);
    // std::array<uint32_t, 92> cfgdata{0};
    // cfgdata.resize(3*92);
    // req.set_binarydata("config", cfgdata.data(), cfgdata.size());
    req.set_binarydata("config", gbtcfg, 92); // FIXME how to guarantee size?
    /**
    // or, with one blob per GBTx
    std::array<std::array<uint32_t, 92>, 3> cfgdata{};
    req.set_binarydata("gbt0", cfgdata.at(0).data(), cfgdata.at(0).size());
    req.set_binarydata("gbt1", cfgdata.at(1).data(), cfgdata.at(1).size());
    req.set_binarydata("gbt2", cfgdata.at(2).data(), cfgdata.at(2).size());
    */
    try {
      rsp = rpc.call_method(req);
    } STANDARD_CATCH;
    checkRPCResponse("HwOptoHybrid::configureAllGBTs");
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::configureAllGBTs", gem::hw::optohybrid::exception::Exception);
}


void gem::hw::optohybrid::HwOptoHybrid::configureGBT(uint8_t const& gbtID, std::array<const uint32_t, 92> const& gbtcfg)
{
  configureGBT(gbtID, gbtcfg.data());
}


void gem::hw::optohybrid::HwOptoHybrid::configureAllGBTs(uint32_t const* gbtcfg)
{
  try {
    req = wisc::RPCMsg("gbt.writeAllGBTConfigs");
    req.set_word("ohN", static_cast<uint32_t>(m_link));
    req.set_word("useRAM", false);
    // std::array<uint32_t, 3*92> cfgdata{0};
    // cfgdata.resize(3*92);
    // req.set_binarydata("config", cfgdata.data(), cfgdata.size());
    req.set_binarydata("config", gbtcfg, 3*92); // FIXME how to guarantee size?
    /**
    // or, with one blob per GBTx
    std::array<std::array<uint32_t, 92>, 3> cfgdata{};
    req.set_binarydata("gbt0", cfgdata.at(0).data(), cfgdata.at(0).size());
    req.set_binarydata("gbt1", cfgdata.at(1).data(), cfgdata.at(1).size());
    req.set_binarydata("gbt2", cfgdata.at(2).data(), cfgdata.at(2).size());
    */
    try {
      rsp = rpc.call_method(req);
    } STANDARD_CATCH;
    checkRPCResponse("HwOptoHybrid::configureAllGBTs");
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::configureAllGBTs", gem::hw::optohybrid::exception::Exception);
}


void gem::hw::optohybrid::HwOptoHybrid::configureAllGBTs(std::array<const uint32_t, 3*92> const& gbtcfg)
{
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
  return writeReg(getDeviceBaseNode(),"FPGA.CONTROL.VFAT.RESET",0x1);
}

void gem::hw::optohybrid::HwOptoHybrid::setSBitMask(uint32_t const mask)
{
  writeReg(getDeviceBaseNode(),"FPGA.TRIG.CTRL.VFAT_MASK", mask&0x00ffffff);
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getSBitMask()
{
  return readReg(getDeviceBaseNode(),"FPGA.TRIG.CTRL.VFAT_MASK");
}

//// HDMI_OUTPUT \\\\*
void gem::hw::optohybrid::HwOptoHybrid::setHDMISBitSource(uint8_t const& bit, uint8_t const& mode)
{
  writeReg(getDeviceBaseNode(),toolbox::toString("FPGA.CONTROL.HDMI.SBIT_SEL%i",static_cast<uint32_t>(bit)),mode);
}

void gem::hw::optohybrid::HwOptoHybrid::setHDMISBitSource(std::array<uint8_t, 8> const& sources)
{
  for (uint8_t i = 0; i < 8; ++i)
    setHDMISBitSource(i, sources[i]);
};

uint32_t gem::hw::optohybrid::HwOptoHybrid::getHDMISBitSource(uint8_t const& bit)
{
  return readReg(getDeviceBaseNode(),toolbox::toString("FPGA.CONTROL.HDMI.SBIT_SEL%i",static_cast<uint32_t>(bit)));
}

void gem::hw::optohybrid::HwOptoHybrid::setHDMISBitMode(uint8_t const& bit, uint8_t const& mode)
{
  writeReg(getDeviceBaseNode(),toolbox::toString("FPGA.CONTROL.HDMI.SBIT_MODE%i",static_cast<uint32_t>(bit)), mode);
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getHDMISBitMode(uint8_t const& bit)
{
  return readReg(getDeviceBaseNode(),toolbox::toString("FPGA.CONTROL.HDMI.SBIT_MODE%i",static_cast<uint32_t>(bit)));
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getClockStatus()
{
  return readReg(getDeviceBaseNode(),"FPGA.CLOCKING");
}

//////// Firmware Status \\\\\\\\*
bool gem::hw::optohybrid::HwOptoHybrid::hasSEUError()
{
  return readReg(getDeviceBaseNode(),"STATUS.SEU");
}

//////// OptoHybrid ADC Interface \\\\\\\\*
// FIXME UPDATE FOR V3
uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGATemp()
{
  return readReg(getDeviceBaseNode(),"ADC.TEMP");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGAMaxTemp()
{
  return readReg(getDeviceBaseNode(),"ADC.TEMP.MAX");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGAMinTemp()
{
  return readReg(getDeviceBaseNode(),"ADC.TEMP.MIN");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGAVccInt()
{
  return readReg(getDeviceBaseNode(),"ADC.VCCINT");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGAMaxVccInt()
{
  return readReg(getDeviceBaseNode(),"ADC.VCCINT.MAX");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGAMinVccInt()
{
  return readReg(getDeviceBaseNode(),"ADC.VCCINT.MIN");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGAVccAux()
{
  return readReg(getDeviceBaseNode(),"ADC.VCCAUX");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGAMaxVccAux()
{
  return readReg(getDeviceBaseNode(),"ADC.VCCAUX.MAX");
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getFPGAMinVccAux()
{
  return readReg(getDeviceBaseNode(),"ADC.VCCAUX.MIN");
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
    req = wisc::RPCMsg("optohybrid.configureScanModule");
    req.set_word("ohN",static_cast<uint32_t>(m_link));
    req.set_word("useUltra",useUltra);

    if (useUltra) {
      req.set_word("mask",chip);
    } else {
      req.set_word("vfat",chip);
    }

    req.set_word("mode",   mode);
    req.set_word("ch",     channel);
    req.set_word("nevts",  nevts);
    req.set_word("dacMin", min);
    req.set_word("dacMax", max);
    req.set_word("dacStep",step);
    // req.set_word("reset",reset);

    try {
      rsp = rpc.call_method(req);
    } STANDARD_CATCH;
    checkRPCResponse("HwOptoHybrid::broadcastWrite");
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::startScanModule", gem::hw::optohybrid::exception::Exception);
}

void gem::hw::optohybrid::HwOptoHybrid::startScanModule(uint32_t const& nevts, bool useUltra)
{
  try {
    req = wisc::RPCMsg("optohybrid.startScanModule");
    req.set_word("ohN", static_cast<uint32_t>(m_link));
    // req.set_word("nevts", nevts);
    req.set_word("useUltra", useUltra);

    try {
      rsp = rpc.call_method(req);
    } STANDARD_CATCH;
    checkRPCResponse("HwOptoHybrid::broadcastWrite");
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::startScanModule", gem::hw::optohybrid::exception::Exception);
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
  try {
    req = wisc::RPCMsg("optohybrid.getUltraScanResults");
    req.set_word("ohN",     static_cast<uint32_t>(m_link));
    req.set_word("nevts",   nevts);
    req.set_word("dacMin",  min);  // FIXME not present, should come from configuration
    req.set_word("dacMax",  max);  // FIXME not present, should come from configuration
    req.set_word("dacStep", step); // FIXME not present, should come from configuration

    try {
      rsp = rpc.call_method(req);
    } STANDARD_CATCH;
    checkRPCResponse("HwOptoHybrid::broadcastWrite");
    uint32_t size = rsp.get_word_array_size("data");
    std::vector<uint32_t> data;
    data.resize(size);
    // results.resize(NVFATS*(max-min+1)/step); // FIXME should be the same as above, throw error if not?
    // need to transform to std::vector<(NVFATS), std::vector<((max-min)/step)uint32_t> >
    rsp.get_word_array("data", data.data());
    std::vector<std::vector<uint32_t> > results;
    results.resize(24);
    size_t idx = 0;
    for (size_t vfat = 0; vfat < 24; ++vfat) {
      auto vres = results.at(vfat);
      for (size_t dac = 0; dac <= max-min; ++dac) {
        vres.push_back(data.at(idx));
        ++idx;
      }
    }
    return results;
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::getUltraScanResults", gem::hw::optohybrid::exception::Exception);
}


//////// Firmware forced TTC commands \\\\\\\\*
void gem::hw::optohybrid::HwOptoHybrid::sendL1A(uint32_t const& ntrigs, uint32_t const& rate)
{
  writeReg(getDeviceBaseNode(), "FPGA.GBT.FORCE_L1A",0x1);
}

void gem::hw::optohybrid::HwOptoHybrid::sendResync(uint32_t const& nresync, uint32_t const& rate)
{
  writeReg(getDeviceBaseNode(), "FPGA.GBT.FORCE_RESYNC",0x1);
}


void gem::hw::optohybrid::HwOptoHybrid::sendBC0(uint32_t const& nbc0, uint32_t const& rate)
{
  writeReg(getDeviceBaseNode(), "FPGA.GBT.FORCE_BC0",0x1);
}

void gem::hw::optohybrid::HwOptoHybrid::sendBXN(uint32_t const& nbc0, uint32_t const& rate)
{
  writeReg(getDeviceBaseNode(), "FPGA.GBT.FORCE_BXN",0x1);
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
    return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.TTC.GTX_TTC.%s", (ttcSignal.str()).c_str()));
  case(OptoHybridTTCMode::RESET):
    return 0x0;
  default:
    return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.TTC.SENT.%s",    (ttcSignal.str()).c_str()));
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
