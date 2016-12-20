#include <bitset>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <functional>

#include "gem/hw/optohybrid/HwOptoHybrid.h"

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid() :
  gem::hw::GEMHwDevice::GEMHwDevice("HwOptoHybrid"),
  //monOptoHybrid_(0)
  b_links({false,false,false}),
  m_controlLink(-1)
{
  setDeviceID("OptoHybridHw");
  setAddressTableFileName("uhal_gem_amc_glib.xml");
  //need to know which device this is 0 or 1?
  //need to fix the hard coded '0', how to get it in from the constructor in a sensible way? /**JS Oct 8**/
  setDeviceBaseNode("GEM_AMC.OH.OH0");
  //gem::hw::optohybrid::HwOptoHybrid::initDevice();
  //set up which links are active, so that the control can be done without specifying a link
  INFO("HwOptoHybrid ctor done " << isHwConnected());
}

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(std::string const& optohybridDevice,
                                                std::string const& connectionFile) :
  gem::hw::GEMHwDevice::GEMHwDevice(optohybridDevice, connectionFile),
  //monOptoHybrid_(0)
  b_links({false,false,false}),
  m_controlLink(-1)
{
  std::stringstream basenode;
  basenode << "GEM_AMC.OH.OH" << *optohybridDevice.rbegin();
  setDeviceBaseNode(basenode.str());
  INFO("HwOptoHybrid ctor done (basenode "
       << basenode.str() << ") " << isHwConnected());
}

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(std::string const& optohybridDevice,
                                                std::string const& connectionURI,
                                                std::string const& addressTable) :
  gem::hw::GEMHwDevice::GEMHwDevice(optohybridDevice, connectionURI, addressTable),
  //monOptoHybrid_(0)
  b_links({false,false,false}),
  m_controlLink(-1)
{
  setAddressTableFileName("uhal_gem_amc_glib.xml");
  std::stringstream basenode;
  basenode << "GEM_AMC.OH.OH" << *optohybridDevice.rbegin();
  setDeviceBaseNode(basenode.str());
  INFO("HwOptoHybrid ctor done (basenode "
       << basenode.str() << ") " << isHwConnected());
}

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(std::string const& optohybridDevice,
                                                uhal::HwInterface& uhalDevice) :
  gem::hw::GEMHwDevice::GEMHwDevice(optohybridDevice,uhalDevice),
  //monOptoHybrid_(0)
  b_links({false,false,false}),
  m_controlLink(-1)
{
  std::stringstream basenode;
  basenode << "GEM_AMC.OH.OH" << *optohybridDevice.rbegin();
  setDeviceBaseNode(basenode.str());
  INFO("HwOptoHybrid ctor done (basenode "
       << basenode.str() << ") " << isHwConnected());
}

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(gem::hw::glib::HwGLIB const& glibDevice,
                                                uint8_t const& slot) :
  gem::hw::GEMHwDevice::GEMHwDevice(toolbox::toString("%s.OptoHybrid_%d",(glibDevice.getLoggerName()).c_str(),(int)slot),
                                    glibDevice.getGEMHwInterface()),
  //monOptoHybrid_(0),
  b_links({false,false,false}),
  m_controlLink(-1),
  m_slot((int)slot)
{
  INFO("HwOptoHybrid creating OptoHybrid device from GLIB device " << glibDevice.getLoggerName());
  //use a connection file and connection manager?
  setDeviceID(toolbox::toString("%s.optohybrid%02d",glibDevice.getDeviceID().c_str(),slot));
  //uhal::ConnectionManager manager ( "file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml" );
  p_gemConnectionManager.reset(new uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml"));
  p_gemHW.reset(new uhal::HwInterface(p_gemConnectionManager->getDevice(this->getDeviceID())));
  //p_gemConnectionManager = std::shared_ptr<uhal::ConnectionManager>(uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml"));
  //p_gemHW = std::shared_ptr<uhal::HwInterface>(p_gemConnectionManager->getDevice(this->getDeviceID()));
  std::stringstream basenode;
  basenode << "GLIB.OptoHybrid_" << (int)slot << ".OptoHybrid";
  setDeviceBaseNode(basenode.str());
  INFO("HwOptoHybrid ctor done (basenode "
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

bool gem::hw::optohybrid::HwOptoHybrid::isHwConnected()
{
  if ( b_is_connected ) {
    DEBUG("HwOptoHybrid connection good");
    return true;
  } else if (gem::hw::GEMHwDevice::isHwConnected()) {
    DEBUG("Checking hardware connection");

    if ((this->getFirmwareDateString()).rfind("15") != std::string::npos ||
        (this->getFirmwareDateString()).rfind("16") != std::string::npos) {
      b_is_connected = true;
      INFO("OptoHybrid present ("
           << this->getFirmwareVersionString() << "/0x"
           << std::hex << this->getFirmwareDate() << std::dec << ")");
      return true;
    } else {
      b_is_connected = false;
      DEBUG("OptoHybrid not reachable (unable to find 15 or 16 in the firmware string)."
            << " Obviously we need a better strategy to check connectivity");
      return false;
    }
  }
  //shouldn't get to here unless HW isn't connected
  DEBUG("OptoHybrid not reachable (!b_is_connected && !GEMHwDevice::isHwConnnected)");
  return false;
}


gem::hw::GEMHwDevice::OpticalLinkStatus gem::hw::optohybrid::HwOptoHybrid::LinkStatus()
{
  gem::hw::GEMHwDevice::OpticalLinkStatus linkStatus;

  linkStatus.TRK_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX.TRK_ERR"));
  linkStatus.TRG_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX.TRG_ERR"));
  linkStatus.Data_Packets = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX.DATA_Packets"));
  return linkStatus;
}

void gem::hw::optohybrid::HwOptoHybrid::LinkReset(uint8_t const& resets)
{
  if (resets&0x1)
    writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX.TRK_ERR.Reset"),0x1);
  if (resets&0x2)
    writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX.TRG_ERR.Reset"),0x1);
  if (resets&0x4)
    writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX.DATA_Packets.Reset"),0x1);
}

//uint32_t gem::hw::optohybrid::HwOptoHybrid::readTriggerData() {
//  return uint32_t value;
//}

void gem::hw::optohybrid::HwOptoHybrid::updateWBMasterCounters()
{
  std::stringstream regName;
  regName << getDeviceBaseNode() << ".COUNTERS.WB.MASTER";
  std::vector<std::pair<std::string,uint32_t> > wishboneRegisters;
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Strobe.GTX"   ,regName.str().c_str()),
                                             m_wbMasterCounters.GTX.first));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Ack.GTX"      ,regName.str().c_str()),
                                             m_wbMasterCounters.GTX.second));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Strobe.ExtI2C",regName.str().c_str()),
                                             m_wbMasterCounters.ExtI2C.first));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Ack.ExtI2C"   ,regName.str().c_str()),
                                             m_wbMasterCounters.ExtI2C.second));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Strobe.Scan"  ,regName.str().c_str()),
                                             m_wbMasterCounters.Scan.first));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Ack.Scan"     ,regName.str().c_str()),
                                             m_wbMasterCounters.Scan.second));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Strobe.DAC"   ,regName.str().c_str()),
                                             m_wbMasterCounters.DAC.first));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Ack.DAC"      ,regName.str().c_str()),
                                             m_wbMasterCounters.DAC.second));
  readRegs(wishboneRegisters);
}

void gem::hw::optohybrid::HwOptoHybrid::resetWBMasterCounters()
{
  std::stringstream regName;
  regName << getDeviceBaseNode() << ".COUNTERS.WB.MASTER";
  std::vector<std::string> wishboneRegisters;
  wishboneRegisters.push_back(toolbox::toString("%s.Strobe.GTX.Reset"   ,regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Ack.GTX.Reset"      ,regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Strobe.ExtI2C.Reset",regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Ack.ExtI2C.Reset"   ,regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Strobe.Scan.Reset"  ,regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Ack.Scan.Reset"     ,regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Strobe.DAC.Reset"   ,regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Ack.DAC.Reset"      ,regName.str().c_str()));
  writeValueToRegs(wishboneRegisters, 0x1);
  m_wbMasterCounters.reset();
}

void gem::hw::optohybrid::HwOptoHybrid::updateWBSlaveCounters()
{
  std::stringstream regName;
  regName << getDeviceBaseNode() << ".COUNTERS.WB.SLAVE";
  std::vector<std::pair<std::string,uint32_t> > wishboneRegisters;
  for (unsigned i2c = 0; i2c < 6; ++i2c) {
    wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Strobe.I2C%d.Reset",regName.str().c_str(),i2c),
                                               m_wbSlaveCounters.I2C.at(i2c).first));
    wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Ack.I2C%d.Reset",   regName.str().c_str(),i2c),
                                               m_wbSlaveCounters.I2C.at(i2c).second));
  }
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Strobe.ExtI2C"  ,regName.str().c_str()),
                                             m_wbSlaveCounters.ExtI2C.first));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Ack.ExtI2C"     ,regName.str().c_str()),
                                             m_wbSlaveCounters.ExtI2C.second));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Strobe.Scan"    ,regName.str().c_str()),
                                             m_wbSlaveCounters.Scan.first));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Ack.Scan"       ,regName.str().c_str()),
                                             m_wbSlaveCounters.Scan.second));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Strobe.T1"      ,regName.str().c_str()),
                                             m_wbSlaveCounters.T1.first));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Ack.T1"         ,regName.str().c_str()),
                                             m_wbSlaveCounters.T1.second));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Strobe.DAC"     ,regName.str().c_str()),
                                             m_wbSlaveCounters.DAC.first));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Ack.DAC"        ,regName.str().c_str()),
                                             m_wbSlaveCounters.DAC.second));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Strobe.ADC"     ,regName.str().c_str()),
                                             m_wbSlaveCounters.ADC.first));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Ack.ADC"        ,regName.str().c_str()),
                                             m_wbSlaveCounters.ADC.second));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Strobe.Clocking",regName.str().c_str()),
                                             m_wbSlaveCounters.Clocking.first));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Ack.Clocking"   ,regName.str().c_str()),
                                             m_wbSlaveCounters.Clocking.second));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Strobe.Counters",regName.str().c_str()),
                                             m_wbSlaveCounters.Counters.first));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Ack.Counters"   ,regName.str().c_str()),
                                             m_wbSlaveCounters.Counters.second));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Strobe.System"  ,regName.str().c_str()),
                                             m_wbSlaveCounters.System.first));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Ack.System"     ,regName.str().c_str()),
                                             m_wbSlaveCounters.System.second));
}

void gem::hw::optohybrid::HwOptoHybrid::resetWBSlaveCounters()
{
  std::stringstream regName;
  regName << getDeviceBaseNode() << ".COUNTERS.WB.SLAVE";
  std::vector<std::string> wishboneRegisters;
  for (unsigned i2c = 0; i2c < 6; ++i2c) {
    wishboneRegisters.push_back(toolbox::toString("%s.Strobe.I2C%d.Reset",regName.str().c_str(),i2c));
    wishboneRegisters.push_back(toolbox::toString("%s.Ack.I2C%d.Reset",   regName.str().c_str(),i2c));
  }
  wishboneRegisters.push_back(toolbox::toString("%s.Strobe.ExtI2C.Reset",  regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Ack.ExtI2C.Reset",     regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Strobe.Scan.Reset",    regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Ack.Scan.Reset",       regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Strobe.T1.Reset",      regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Ack.T1.Reset",         regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Strobe.DAC.Reset",     regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Ack.DAC.Reset",        regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Strobe.ADC.Reset",     regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Ack.ADC.Reset",        regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Strobe.Clocking.Reset",regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Ack.Clocking.Reset",   regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Strobe.Counters.Reset",regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Ack.Counters.Reset",   regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Strobe.System.Reset",  regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Ack.System.Reset",     regName.str().c_str()));
  writeValueToRegs(wishboneRegisters, 0x1);
  m_wbSlaveCounters.reset();
}


void gem::hw::optohybrid::HwOptoHybrid::updateT1Counters()
{
  for (unsigned signal = 0; signal < 4; ++signal) {
    m_t1Counters.AMC13.at(   signal) = getT1Count(signal, 0x0);
    m_t1Counters.Firmware.at(signal) = getT1Count(signal, 0x1);
    m_t1Counters.External.at(signal) = getT1Count(signal, 0x2);
    m_t1Counters.Loopback.at(signal) = getT1Count(signal, 0x3);
    m_t1Counters.Sent.at(    signal) = getT1Count(signal, 0x4);
  }
}

void gem::hw::optohybrid::HwOptoHybrid::resetT1Counters()
{
  resetT1Count(0x0, 0x5); //reset all L1A counters
  resetT1Count(0x1, 0x5); //reset all CalPulse counters
  resetT1Count(0x2, 0x5); //reset all Resync counters
  resetT1Count(0x3, 0x5); //reset all BC0 counters
  m_t1Counters.reset();
}

void gem::hw::optohybrid::HwOptoHybrid::updateVFATCRCCounters()
{
  for (unsigned slot = 0; slot < 24; ++slot)
    m_vfatCRCCounters.CRCCounters.at(slot) = getVFATCRCCount(slot);
}

void gem::hw::optohybrid::HwOptoHybrid::resetVFATCRCCounters()
{
  resetVFATCRCCount();
  // for (unsigned slot = 0; slot < 24; ++slot)
  //   resetVFATCRCCount(slot);
  m_vfatCRCCounters.reset();
}

std::vector<uint32_t> gem::hw::optohybrid::HwOptoHybrid::broadcastRead(std::string const& name,
                                                                       uint32_t    const& mask,
                                                                       bool               reset)
{
  auto t1 = std::chrono::high_resolution_clock::now();
  if (reset)
    writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Reset"),0x1);
  writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Mask"),mask);
  uint32_t tmp = readReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Request.%s", name.c_str()));

  while (readReg(getDeviceBaseNode(),"GEB.Broadcast.Running")) {
    TRACE("HwOptoHybrid::broadcastRead transaction on "
          << name << " is still running...");
    usleep(100);
  }
  auto t2 = std::chrono::high_resolution_clock::now();
  TRACE("HwOptoHybrid::broadcastRead transaction on " << name << " lasted "
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
    TRACE("HwOptoHybrid::broadcastWrite transaction on "
          << name << " is still running...");
    usleep(100);
  }
  auto t2 = std::chrono::high_resolution_clock::now();
  TRACE("HwOptoHybrid::broadcastWrite transaction on " << name << " lasted "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() << "ns");
}


std::vector<std::pair<uint8_t,uint32_t> > gem::hw::optohybrid::HwOptoHybrid::getConnectedVFATs()
{
  std::vector<uint32_t> chips0 = broadcastRead("ChipID0",ALL_VFATS_BCAST_MASK,false);
  std::vector<uint32_t> chips1 = broadcastRead("ChipID1",ALL_VFATS_BCAST_MASK,false);
  DEBUG("HwOptoHybrid::getConnectedVFATs chips0 size:" << chips0.size() <<  ", chips1 size:" << chips1.size());

  std::vector<std::pair<uint8_t, uint32_t> > chipIDs;
  std::vector<std::pair<uint32_t,uint32_t> > chipPairs;
  chipPairs.reserve(chips0.size());

  std::transform(chips1.begin(), chips1.end(), chips0.begin(),
                 std::back_inserter(chipPairs),
                 std::make_pair<uint32_t,uint32_t>);

  for (auto chip = chipPairs.begin(); chip != chipPairs.end(); ++chip) {
    if (((chip->first) >> 16) != 0x3) {
      uint8_t slot = ((chip->first)>>8)&0xff;
      uint32_t chipID = (((chip->first)&0xff)<<8)+((chip->second)&0xff);
      DEBUG("HwOptoHybrid::getConnectedVFATs GEB slot: " << (int)slot
            << ", chipID1: 0x" << std::hex << chip->first   << std::dec
            << ", chipID2: 0x" << std::hex << chip->second  << std::dec
            << ", chipID: 0x"  << std::hex << chipID        << std::dec);
      chipIDs.push_back(std::make_pair(slot,chipID));
    }
  }
  return chipIDs;
}


uint32_t gem::hw::optohybrid::HwOptoHybrid::getConnectedVFATMask()
{
  std::vector<uint32_t> allChips = broadcastRead("ChipID0",ALL_VFATS_BCAST_MASK);
  uint32_t connectedMask = 0x0; // high means don't broadcast
  uint32_t disabledMask  = 0x0; // high means ignore data
  DEBUG("HwOptoHybrid::getConnectedVFATMask Reading ChipID0 from all possible slots");
  for (auto id = allChips.begin(); id != allChips.end(); ++id) {
    // 0x00XXYYZZ
    // XX = status (00000EVR)
    // YY = chip number
    // ZZ = register contents
    INFO("HwOptoHybrid::getConnectedVFATMask result 0x" << std::setw(8) << std::setfill('0') << std::hex << *id << std::dec);
    // bool e_bit(((*id)>>18)&0x1),v_bit(((*id)>>17)&0x1),r_bit(((*id)>>16)&0x1);

    // if (v_bit && !e_bit) {
    if (((*id) >> 16) != 0x3) {
      uint8_t shift = ((*id)>>8)&0xff;
      connectedMask |= (0x1 << shift);
      disabledMask  |= (0x1 << shift);
    }
    DEBUG("HwOptoHybrid::getConnectedVFATMask mask is " << std::hex << connectedMask << std::dec);
  }

  DEBUG("HwOptoHybrid::getConnectedVFATMask previous mask is 0x" << std::setw(8) << std::setfill('0')
        << std::hex << connectedMask << std::dec);
  connectedMask = ~connectedMask;
  disabledMask  = ~disabledMask ;
  DEBUG("HwOptoHybrid::getConnectedVFATMask intermediate mask is 0x" << std::setw(8) << std::setfill('0')
        << std::hex << connectedMask << std::dec);
  connectedMask |= ALL_VFATS_BCAST_MASK;
  disabledMask  |= ALL_VFATS_BCAST_MASK;
  DEBUG("HwOptoHybrid::getConnectedVFATMask final mask is 0x" << std::setw(8) << std::setfill('0')
        << std::hex << connectedMask << std::dec);

  return connectedMask;
}


void gem::hw::optohybrid::HwOptoHybrid::setVFATsToDefaults(uint8_t  const& vt1,
                                                           uint8_t  const& vt2,
                                                           uint8_t  const& latency,
                                                           uint32_t const& broadcastMask)
{
  // std::stringstream regName;
  // regName << getDeviceBaseNode() << ".GEB.Broadcast.Results";
  // std::vector<uint32_t> res;
  // res = readBlock(regName.str(),std::bitset<32>(~broadcastMask).count());
  // WARN("HwOptoHybrid::setVFATsToDefaults::Latency");
  // for (auto r = res.begin(); r != res.end(); ++r) {
  //   WARN(" 0x" << std::hex << std::setw(8) << std::setfill('0') << *r << std::dec);
  // }

  broadcastWrite("ContReg0",   0x36, broadcastMask);
  broadcastWrite("ContReg1",   0x00, broadcastMask);
  broadcastWrite("ContReg2",   0x30, broadcastMask);
  broadcastWrite("ContReg3",   0x00, broadcastMask);
  broadcastWrite("IPreampIn",   168, broadcastMask);
  broadcastWrite("IPreampFeed",  80, broadcastMask);
  broadcastWrite("IPreampOut",  150, broadcastMask);
  broadcastWrite("IShaper",     150, broadcastMask);
  broadcastWrite("IShaperFeed", 100, broadcastMask);
  broadcastWrite("IComp",        90, broadcastMask);

  broadcastWrite("VThreshold1", vt1,     broadcastMask);
  broadcastWrite("VThreshold2", vt2,     broadcastMask);
  broadcastWrite("Latency",     latency, broadcastMask);
}


void gem::hw::optohybrid::HwOptoHybrid::setVFATsToDefaults(std::map<std::string, uint8_t> const& regvals,
                                                           uint32_t const& broadcastMask)
{
  for (auto reg = regvals.begin(); reg != regvals.end(); ++reg) {
    // check that reg->first is a valid VFAT register?
    broadcastWrite(reg->first,   reg->second, broadcastMask);
  }
}


void gem::hw::optohybrid::HwOptoHybrid::generalReset()
{
  return;
}

void gem::hw::optohybrid::HwOptoHybrid::counterReset()
{
  resetT1Counters();
  resetVFATCRCCounters();
  resetWBSlaveCounters();
  resetWBMasterCounters();
  return;
}

void gem::hw::optohybrid::HwOptoHybrid::linkReset(uint8_t const& link)
{
  return;
}

