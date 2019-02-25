#include "gem/hw/optohybrid/HwOptoHybrid.h"

#include <bitset>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <functional>

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(std::string const& optohybridDevice,
                                                std::string const& connectionFile) :
  gem::hw::GEMHwDevice::GEMHwDevice(optohybridDevice, connectionFile),
  //monOptoHybrid_(0)
  b_is_initial(true),
  m_link(-1)
{
  this->setup(optohybridDevice);
  m_link = gem::utils::extractDeviceID(optohybridDevice,3);
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
  //monOptoHybrid_(0)
  b_is_initial(true),
  m_link(-1)
{
  this->setup(optohybridDevice);
  m_link = gem::utils::extractDeviceID(optohybridDevice,3);
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
  //monOptoHybrid_(0)
  b_is_initial(true),
  m_link(-1)
{
  this->setup(optohybridDevice);
  m_link = gem::utils::extractDeviceID(optohybridDevice,3);
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
  //monOptoHybrid_(0),
  b_is_initial(true),
  m_link(static_cast<uint32_t>(slot))
{
  CMSGEMOS_INFO("HwOptoHybrid creating OptoHybrid device from AMC device " << amcDevice.getLoggerName());
  //use a connection file and connection manager?
  setDeviceID(toolbox::toString("%s.optohybrid%02d",amcDevice.getDeviceID().c_str(),static_cast<uint32_t>(m_link)));
  //uhal::ConnectionManager manager ( "file://${GEM_ADDRESS_TABLE_PATH}/connections.xml" );
  // p_gemConnectionManager.reset(new uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections.xml"));
  // p_gemHW.reset(new uhal::HwInterface(p_gemConnectionManager->getDevice(this->getDeviceID())));
  //p_gemConnectionManager = std::shared_ptr<uhal::ConnectionManager>(uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections.xml"));
  //p_gemHW = std::shared_ptr<uhal::HwInterface>(p_gemConnectionManager->getDevice(this->getDeviceID()));
  std::stringstream basenode;
  basenode << "GEM_AMC.OH.OH" << static_cast<uint32_t>(m_link);
  this->setDeviceBaseNode(basenode.str());
  CMSGEMOS_INFO("HwOptoHybrid ctor done (basenode "
                << basenode.str() << ") " << isHwConnected());
}

gem::hw::optohybrid::HwOptoHybrid::~HwOptoHybrid()
{
  //releaseDevice();
}

//void gem::hw::optohybrid::HwOptoHybrid::configureDevice(std::string const& xmlSettings)
//{
//  //here load the xml file settings onto the board
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::configureDevice()
//{
//  //determine the manner in which to configure the device (XML or DB parameters)
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::releaseDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::enableDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::disableDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::pauseDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::startDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::stopDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::resumeDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::haltDevice()
//{
//
//}
//

void gem::hw::optohybrid::HwOptoHybrid::connectRPC(bool reconnect)
{
  if (isConnected) {
    this->loadModule("optohybrid", "optohybrid v1.0.1");
    CMSGEMOS_DEBUG("HwOptoHybrid::connectRPC modules loaded");
  } else {
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

    // FIXME IN FIRMWARE, need better check of connectivity...
    if ((this->getFirmwareDateString()).rfind("15") != std::string::npos ||
        (this->getFirmwareDateString()).rfind("16") != std::string::npos ||
        (this->getFirmwareDateString()).rfind("17") != std::string::npos ||
        (this->getFirmwareDateString()).rfind("18") != std::string::npos) {
      b_is_connected = true;
      CMSGEMOS_INFO("OptoHybrid present ("
           << this->getFirmwareVersionString() << "/0x"
           << std::hex << this->getFirmwareDate() << std::dec << ")");
      return true;
    } else {
      b_is_connected = false;
      CMSGEMOS_DEBUG("OptoHybrid not reachable (unable to find 15 or 16 in the firmware string)."
            << " Obviously we need a better strategy to check connectivity");
      return false;
    }
  }
  //shouldn't get to here unless HW isn't connected
  CMSGEMOS_DEBUG("OptoHybrid not reachable (!b_is_connected && !GEMHwDevice::isHwConnnected)");
  return false;
}


//uint32_t gem::hw::optohybrid::HwOptoHybrid::readTriggerData() {
//  return uint32_t value;
//}


std::vector<uint32_t> gem::hw::optohybrid::HwOptoHybrid::broadcastRead(std::string const& name,
                                                                       uint32_t    const& mask,
                                                                       bool               reset)
{
  auto t1 = std::chrono::high_resolution_clock::now();
  if (reset)
    writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Reset"),0x1);
  writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Mask"),mask);
  readReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Request.%s", name.c_str()));

  while (readReg(getDeviceBaseNode(),"GEB.Broadcast.Running")) {
    CMSGEMOS_TRACE("HwOptoHybrid::broadcastRead transaction on "
          << name << " is still running...");
    usleep(10);
  }
  auto t2 = std::chrono::high_resolution_clock::now();
  CMSGEMOS_TRACE("HwOptoHybrid::broadcastRead transaction on " << name << " lasted "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() << "ns");
  std::stringstream regName;
  regName << getDeviceBaseNode() << ".GEB.Broadcast.Results";
  std::vector<uint32_t> results;
  //need to compute the number of required reads based on the mask
  return readBlock(regName.str(),std::bitset<32>(~mask).count());
}

void gem::hw::optohybrid::HwOptoHybrid::broadcastWrite(std::string const& name,
                                                       uint32_t    const& value,
                                                       uint32_t    const& mask,
                                                       bool reset)
{
  auto t1 = std::chrono::high_resolution_clock::now();
  if (reset)
    writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Reset"),0x1);
  writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Mask"),mask);
  writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Request.%s", name.c_str()),value);
  while (readReg(getDeviceBaseNode(),"GEB.Broadcast.Running")) {
    CMSGEMOS_TRACE("HwOptoHybrid::broadcastWrite transaction on "
          << name << " is still running...");
    usleep(10);
  }
  auto t2 = std::chrono::high_resolution_clock::now();
  CMSGEMOS_TRACE("HwOptoHybrid::broadcastWrite transaction on " << name << " lasted "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() << "ns");
}


std::vector<std::pair<uint8_t,uint32_t> > gem::hw::optohybrid::HwOptoHybrid::getConnectedVFATs(bool update)
{
  if (update || b_is_initial) {
    std::vector<uint32_t> chips0 = broadcastRead("ChipID0",gem::hw::utils::ALL_VFATS_BCAST_MASK,false);
    std::vector<uint32_t> chips1 = broadcastRead("ChipID1",gem::hw::utils::ALL_VFATS_BCAST_MASK,false);
    CMSGEMOS_DEBUG("HwOptoHybrid::getConnectedVFATs chips0 size:" << chips0.size() <<  ", chips1 size:" << chips1.size());

    std::vector<std::pair<uint8_t, uint32_t> > chipIDs;
    std::vector<std::pair<uint32_t,uint32_t> > chipPairs;
    chipPairs.reserve(chips0.size());

    std::transform(chips1.begin(), chips1.end(), chips0.begin(),
                   std::back_inserter(chipPairs),
                   std::make_pair<const uint32_t&, const uint32_t&>);

    for (auto chip = chipPairs.begin(); chip != chipPairs.end(); ++chip) {
      if (((chip->first) >> 16) != 0x3) {
        uint8_t slot = ((chip->first)>>8)&0xff;
        uint32_t chipID = (((chip->first)&0xff)<<8)+((chip->second)&0xff);
        CMSGEMOS_DEBUG("HwOptoHybrid::getConnectedVFATs GEB slot: " << (int)slot
              << ", chipID1: 0x" << std::hex << chip->first   << std::dec
              << ", chipID2: 0x" << std::hex << chip->second  << std::dec
              << ", chipID: 0x"  << std::hex << chipID        << std::dec);
        chipIDs.push_back(std::make_pair(slot,chipID));
      }
    }
    m_chipIDs = chipIDs;
    b_is_initial = false;
  }
  return m_chipIDs;
}


uint32_t gem::hw::optohybrid::HwOptoHybrid::getConnectedVFATMask(bool update)
{
  if (update || b_is_initial) {
    std::vector<uint32_t> allChips = broadcastRead("ChipID0",gem::hw::utils::ALL_VFATS_BCAST_MASK);
    uint32_t connectedMask = 0x0; // high means don't broadcast
    uint32_t disabledMask  = 0x0; // high means ignore data
    CMSGEMOS_DEBUG("HwOptoHybrid::getConnectedVFATMask Reading ChipID0 from all possible slots");
    for (auto id = allChips.begin(); id != allChips.end(); ++id) {
      // 0x00XXYYZZ
      // XX = status (00000EVR)
      // YY = chip number
      // ZZ = register contents
      CMSGEMOS_INFO("HwOptoHybrid::getConnectedVFATMask result 0x" << std::setw(8) << std::setfill('0') << std::hex << *id << std::dec);
      // bool e_bit(((*id)>>18)&0x1),v_bit(((*id)>>17)&0x1),r_bit(((*id)>>16)&0x1);

      // if (v_bit && !e_bit) {
      if (((*id) >> 16) != 0x3) {
        uint8_t shift = ((*id)>>8)&0xff;
        connectedMask |= (0x1 << shift);
        disabledMask  |= (0x1 << shift);
      }
      CMSGEMOS_DEBUG("HwOptoHybrid::getConnectedVFATMask mask is " << std::hex << connectedMask << std::dec);
  }

    CMSGEMOS_DEBUG("HwOptoHybrid::getConnectedVFATMask previous mask is 0x" << std::setw(8) << std::setfill('0')
          << std::hex << connectedMask << std::dec);
    connectedMask = ~connectedMask;
    disabledMask  = ~disabledMask ;
    CMSGEMOS_DEBUG("HwOptoHybrid::getConnectedVFATMask intermediate mask is 0x" << std::setw(8) << std::setfill('0')
          << std::hex << connectedMask << std::dec);
    connectedMask |= gem::hw::utils::ALL_VFATS_BCAST_MASK;
    disabledMask  |= gem::hw::utils::ALL_VFATS_BCAST_MASK;
    CMSGEMOS_DEBUG("HwOptoHybrid::getConnectedVFATMask final mask is 0x" << std::setw(8) << std::setfill('0')
          << std::hex << connectedMask << std::dec);

     m_disabledMask  = connectedMask;
     m_connectedMask = disabledMask;
  }
  return m_connectedMask;
}


void gem::hw::optohybrid::HwOptoHybrid::setVFATsToDefaults()
{
  // CTP7 module candidate
}


void gem::hw::optohybrid::HwOptoHybrid::setVFATsToDefaults(std::map<std::string, uint16_t> const& regvals)
{
  // CTP7 module candidate
}


void gem::hw::optohybrid::HwOptoHybrid::generalReset()
{
  return;
}

void gem::hw::optohybrid::HwOptoHybrid::counterReset()
{
  resetTTCCounters();
  // other counter resets?
  return;
}

void gem::hw::optohybrid::HwOptoHybrid::linkReset(uint8_t const& link)
{
  return;
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

//////// Optohybrid ADC Interface \\\\\\\\*
uint32_t gem::hw::optohybrid::HwOptoHybrid::getADCVAUX(uint8_t const& vaux)
{
  if (vaux > 15) {
    std::string msg = toolbox::toString("Invalid VAUX requested (%d): outside expectation (0-15)",int(vaux));
    CMSGEMOS_ERROR(msg);
    XCEPT_RAISE(gem::hw::optohybrid::exception::ValueError,msg);
  }
  std::stringstream vauxInput;
  vauxInput << "ADC.VAUX.VAL_" << int(vaux);
  return readReg(getDeviceBaseNode(),vauxInput.str());
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getVFATDACOutV(uint8_t const& column)
{
  if (column > 2) {
    std::string msg = toolbox::toString("Invalid column requested (%d): outside expectation (0-2)",int(column));
    CMSGEMOS_ERROR(msg);
    XCEPT_RAISE(gem::hw::optohybrid::exception::ValueError,msg);
  }

  std::stringstream vauxInput;
  switch(column) {
  case(0):
    vauxInput << "ADC.VAUX.VAL_1";
  case(1):
    vauxInput << "ADC.VAUX.VAL_5";
  case(2):
    vauxInput << "ADC.VPVN";
  default:
    std::string msg = toolbox::toString("Invalid column requested (%d): outside expectation (0-2)",int(column));
    CMSGEMOS_ERROR(msg);
    return 0;
  }
  return readReg(getDeviceBaseNode(),vauxInput.str());
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getVFATDACOutI(uint8_t const& column)
{
  if (column > 2) {
    std::string msg = toolbox::toString("Invalid column requested (%d): outside expectation (0-2)",int(column));
    CMSGEMOS_ERROR(msg);
    XCEPT_RAISE(gem::hw::optohybrid::exception::ValueError,msg);
  }

  std::stringstream vauxInput;
  switch(column) {
  case(0):
    vauxInput << "ADC.VAUX.VAL_4";
  case(1):
    vauxInput << "ADC.VAUX.VAL_6";
  case(2):
    vauxInput << "ADC.VAUX.VAL_13";
  default:
    std::string msg = toolbox::toString("Invalid column requested (%d): outside expectation (0-2)",int(column));
    CMSGEMOS_ERROR(msg);
    return 0;
  }
  return readReg(getDeviceBaseNode(),vauxInput.str());
}

//////// Scan Modules \\\\\\\\*
void gem::hw::optohybrid::HwOptoHybrid::configureScanModule(uint8_t const& mode, uint8_t const& chip, uint8_t const& channel,
                                                            uint8_t const& min,  uint8_t const& max,
                                                            uint8_t const& step, uint32_t const& nevts,
                                                            bool useUltra, bool reset)
{
  try {
    req = wisc::RPCMsg("amc.linkReset");
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

    try {
      if (rsp.get_key_exists("error")) {
        std::stringstream errmsg;
        errmsg << rsp.get_string("error");
        CMSGEMOS_ERROR("HwOptoHybrid::configureScanModule error: " << errmsg.str());
        XCEPT_RAISE(gem::hw::exception::RPCMethodError, errmsg.str());
      }
    } STANDARD_CATCH;
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::startScanModule", gem::hw::optohybrid::exception::Exception);
}

void gem::hw::optohybrid::HwOptoHybrid::startScanModule(uint32_t const& nevts, bool useUltra)
{
  try {
    req = wisc::RPCMsg("amc.linkReset");
    req.set_word("ohN", static_cast<uint32_t>(m_link));
    // req.set_word("nevts", nevts);
    req.set_word("useUltra", useUltra);

    try {
      rsp = rpc.call_method(req);
    } STANDARD_CATCH;

    try {
      if (rsp.get_key_exists("error")) {
        std::stringstream errmsg;
        errmsg << rsp.get_string("error");
        CMSGEMOS_ERROR("HwOptoHybrid::startScanModule error: " << errmsg.str());
        XCEPT_RAISE(gem::hw::exception::RPCMethodError, errmsg.str());
      }
    } STANDARD_CATCH;
  } GEM_CATCH_RPC_ERROR("HwOptoHybrid::startScanModule", gem::hw::optohybrid::exception::Exception);
}

void gem::hw::optohybrid::HwOptoHybrid::stopScanModule(bool useUltra, bool reset)
{
  std::stringstream scanBase;
  scanBase << "ScanController";
  if (useUltra)
    scanBase << ".ULTRA";
  else
    scanBase << ".THLAT";

  //don't toggle on if the module is currently not running
  //if (getScanStatus())
  //  writeReg(getDeviceBaseNode(),scanBase.str()+".TOGGLE",0x1);
  if (reset)
    writeReg(getDeviceBaseNode(),scanBase.str()+".RESET",0x1);
}

uint8_t gem::hw::optohybrid::HwOptoHybrid::getScanStatus(bool useUltra)
{
  std::stringstream scanBase;
  scanBase << "ScanController";
  if (useUltra)
    scanBase << ".ULTRA";
  else
    scanBase << ".THLAT";
  return readReg(getDeviceBaseNode(),scanBase.str()+".MONITOR.STATUS");
}

std::vector<uint32_t> gem::hw::optohybrid::HwOptoHybrid::getScanResults(uint32_t const& npoints)
{
  while (readReg(getDeviceBaseNode(),"ScanController.THLAT.MONITOR.STATUS") > 0) {
    CMSGEMOS_INFO("Scan still running, not returning results");
    usleep(10);
  }
  std::stringstream regname;
  regname << getDeviceBaseNode() << ".ScanController.THLAT.RESULTS";
  return readBlock(regname.str(), npoints);
}

std::vector<std::vector<uint32_t> > gem::hw::optohybrid::HwOptoHybrid::getUltraScanResults(uint32_t const& npoints)
{
  while (readReg(getDeviceBaseNode(),"ScanController.ULTRA.MONITOR.STATUS") > 0) {
    CMSGEMOS_INFO("Scan still running, not returning results");
    usleep(10);
  }
  std::vector<std::vector<uint32_t> > results;
  for (int vfat = 0; vfat < 24; ++vfat) {
    std::stringstream regname;
    regname << getDeviceBaseNode() << ".ScanController.ULTRA.RESULTS.VFAT" << vfat;
    results.push_back(readBlock(regname.str(), npoints));
  }
  return results;
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
