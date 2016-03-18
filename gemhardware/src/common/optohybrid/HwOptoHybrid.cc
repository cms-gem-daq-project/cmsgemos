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
  setAddressTableFileName("glib_address_table.xml");
  //need to know which device this is 0 or 1?
  //need to fix the hard coded '0', how to get it in from the constructor in a sensible way? /**JS Oct 8**/
  setDeviceBaseNode("GLIB.OptoHybrid_0.OptoHybrid");
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
  basenode << "GLIB.OptoHybrid_" << *optohybridDevice.rbegin() << ".OptoHybrid";
  setDeviceBaseNode(basenode.str());
  INFO("HwOptoHybrid ctor done " << isHwConnected());
}

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(std::string const& optohybridDevice,
                                                std::string const& connectionURI,
                                                std::string const& addressTable) :
  gem::hw::GEMHwDevice::GEMHwDevice(optohybridDevice, connectionURI, addressTable),
  //monOptoHybrid_(0)
  b_links({false,false,false}),
  m_controlLink(-1)  
{
  setAddressTableFileName("glib_address_table.xml");
  std::stringstream basenode;
  basenode << "GLIB.OptoHybrid_" << *optohybridDevice.rbegin() << ".OptoHybrid";
  setDeviceBaseNode(basenode.str());
  INFO("HwOptoHybrid ctor done " << isHwConnected());
}

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(std::string const& optohybridDevice,
                                                uhal::HwInterface& uhalDevice) :
  gem::hw::GEMHwDevice::GEMHwDevice(optohybridDevice,uhalDevice),
  //monOptoHybrid_(0)
  b_links({false,false,false}),
  m_controlLink(-1)  
{
  std::stringstream basenode;
  basenode << "GLIB.OptoHybrid_" << *optohybridDevice.rbegin() << ".OptoHybrid";
  setDeviceBaseNode(basenode.str());
  INFO("HwOptoHybrid ctor done " << isHwConnected());
}
/*
gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(gem::hw::glib::HwGLIB const& glib,
                                                int const& slot) :
  gem::hw::GEMHwDevice::GEMHwDevice("HwOptoHybrid"),
  //monOptoHybrid_(0),
  b_links({false,false,false}),
  m_controlLink(-1),
  m_slot(slot)
{
  //use a connection file and connection manager?
  setDeviceID(toolbox::toString("%s.optohybrid%02d",glib.getDeviceID().c_str(),slot));
  //uhal::ConnectionManager manager ( "file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml" );
  p_gemConnectionManager.reset(new uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml"));
  p_gemHW.reset(new uhal::HwInterface(p_gemConnectionManager->getDevice(this->getDeviceID())));
  //p_gemConnectionManager = std::shared_ptr<uhal::ConnectionManager>(uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml"));
  //p_gemHW = std::shared_ptr<uhal::HwInterface>(p_gemConnectionManager->getDevice(this->getDeviceID()));
  //setAddressTableFileName("glib_address_table.xml");
  //setDeviceIPAddress(toolbox::toString("192.168.0.%d",160+slot));
  setDeviceBaseNode("OptoHybrid");
  //gem::hw::optohybrid::HwOptoHybrid::initDevice();
}
*/
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

    if ((this->getFirmwareDate()).rfind("15") != std::string::npos ||
        (this->getFirmwareDate()).rfind("16") != std::string::npos) {
      b_is_connected = true;
      INFO("OptoHybrid present(0x" << std::hex << this->getFirmware() << std::dec << ")");
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
  regName << "COUNTERS.WB.MASTER";
  m_wbMasterCounters.GTX.first     = readReg(getDeviceBaseNode(),regName.str() + ".Strobe.GTX"   );
  m_wbMasterCounters.GTX.second    = readReg(getDeviceBaseNode(),regName.str() + ".Ack.GTX"      );
  m_wbMasterCounters.ExtI2C.first  = readReg(getDeviceBaseNode(),regName.str() + ".Strobe.ExtI2C");
  m_wbMasterCounters.ExtI2C.second = readReg(getDeviceBaseNode(),regName.str() + ".Ack.ExtI2C"   );
  m_wbMasterCounters.Scan.first    = readReg(getDeviceBaseNode(),regName.str() + ".Strobe.Scan"  );
  m_wbMasterCounters.Scan.second   = readReg(getDeviceBaseNode(),regName.str() + ".Ack.Scan"     );
  m_wbMasterCounters.DAC.first     = readReg(getDeviceBaseNode(),regName.str() + ".Strobe.DAC"   );
  m_wbMasterCounters.DAC.second    = readReg(getDeviceBaseNode(),regName.str() + ".Ack.DAC"      );
}

void gem::hw::optohybrid::HwOptoHybrid::resetWBMasterCounters()
{
  std::stringstream regName;
  regName << "COUNTERS.WB.MASTER";
  writeReg(getDeviceBaseNode(),regName.str() + ".Strobe.GTX.Reset",   0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Ack.GTX.Reset",      0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Strobe.ExtI2C.Reset",0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Ack.ExtI2C.Reset",   0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Strobe.Scan.Reset",  0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Ack.Scan.Reset",     0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Strobe.DAC.Reset",   0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Ack.DAC.Reset",      0x1);
  m_wbMasterCounters.reset();
}

void gem::hw::optohybrid::HwOptoHybrid::updateWBSlaveCounters()
{
  std::stringstream regName;
  regName << "COUNTERS.WB.SLAVE";
  for (unsigned i2c = 0; i2c < 6; ++i2c) {
    m_wbSlaveCounters.I2C.at(i2c).first     = readReg(getDeviceBaseNode(),
                                                      regName.str() +
                                                      toolbox::toString(".Strobe.I2C%d.Reset",i2c));
    m_wbSlaveCounters.I2C.at(i2c).second    = readReg(getDeviceBaseNode(),
                                                      regName.str() +
                                                      toolbox::toString(".Ack.I2C%d.Reset",i2c)   );
  }
  m_wbSlaveCounters.ExtI2C.first    = readReg(getDeviceBaseNode(),regName.str() + ".Strobe.ExtI2C"  );
  m_wbSlaveCounters.ExtI2C.second   = readReg(getDeviceBaseNode(),regName.str() + ".Ack.ExtI2C"     );
  m_wbSlaveCounters.Scan.first      = readReg(getDeviceBaseNode(),regName.str() + ".Strobe.Scan"    );
  m_wbSlaveCounters.Scan.second     = readReg(getDeviceBaseNode(),regName.str() + ".Ack.Scan"       );
  m_wbSlaveCounters.T1.first        = readReg(getDeviceBaseNode(),regName.str() + ".Strobe.T1"      );
  m_wbSlaveCounters.T1.second       = readReg(getDeviceBaseNode(),regName.str() + ".Ack.T1"         );
  m_wbSlaveCounters.DAC.first       = readReg(getDeviceBaseNode(),regName.str() + ".Strobe.DAC"     );
  m_wbSlaveCounters.DAC.second      = readReg(getDeviceBaseNode(),regName.str() + ".Ack.DAC"        );
  m_wbSlaveCounters.ADC.first       = readReg(getDeviceBaseNode(),regName.str() + ".Strobe.ADC"     );
  m_wbSlaveCounters.ADC.second      = readReg(getDeviceBaseNode(),regName.str() + ".Ack.ADC"        );
  m_wbSlaveCounters.Clocking.first  = readReg(getDeviceBaseNode(),regName.str() + ".Strobe.Clocking");
  m_wbSlaveCounters.Clocking.second = readReg(getDeviceBaseNode(),regName.str() + ".Ack.Clocking"   );
  m_wbSlaveCounters.Counters.first  = readReg(getDeviceBaseNode(),regName.str() + ".Strobe.Counters");
  m_wbSlaveCounters.Counters.second = readReg(getDeviceBaseNode(),regName.str() + ".Ack.Counters"   );
  m_wbSlaveCounters.System.first    = readReg(getDeviceBaseNode(),regName.str() + ".Strobe.System"  );
  m_wbSlaveCounters.System.second   = readReg(getDeviceBaseNode(),regName.str() + ".Ack.System"     );
}

void gem::hw::optohybrid::HwOptoHybrid::resetWBSlaveCounters()
{
  std::stringstream regName;
  regName << "COUNTERS.WB.SLAVE";
  for (unsigned i2c = 0; i2c < 6; ++i2c) {
    writeReg(getDeviceBaseNode(),regName.str() + toolbox::toString(".Strobe.GTX%d.Reset",i2c),0x1);
    writeReg(getDeviceBaseNode(),regName.str() + toolbox::toString(".Ack.GTX%d.Reset",   i2c),0x1);
  }
  writeReg(getDeviceBaseNode(),regName.str() + ".Strobe.ExtI2C.Reset",  0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Ack.ExtI2C.Reset",     0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Strobe.Scan.Reset",    0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Ack.Scan.Reset",       0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Strobe.T1.Reset",      0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Ack.T1.Reset",         0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Strobe.DAC.Reset",     0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Ack.DAC.Reset",        0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Strobe.ADC.Reset",     0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Ack.ADC.Reset",        0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Strobe.Clocking.Reset",0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Ack.Clocking.Reset",   0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Strobe.Counters.Reset",0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Ack.Counters.Reset",   0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Strobe.System.Reset",  0x1);
  writeReg(getDeviceBaseNode(),regName.str() + ".Ack.System.Reset",     0x1);
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
  for (unsigned slot = 0; slot < 24; ++slot)
    resetVFATCRCCount(slot);
  m_vfatCRCCounters.reset();
}

std::vector<uint32_t> gem::hw::optohybrid::HwOptoHybrid::broadcastRead(std::string const& name,
                                                                       uint32_t const mask,
                                                                       bool reset)
{
  if (reset)
    writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Reset"),0x1);
  writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Mask"),mask);
  uint32_t tmp = readReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Request.%s", name.c_str()));
  
  std::stringstream regName;
  regName << getDeviceBaseNode() << ".GEB.Broadcast.Results";
  std::vector<uint32_t> results;
  //need to compute the number of required reads based on the mask
  return readBlock(regName.str(),std::bitset<32>(~mask).count());
}

void gem::hw::optohybrid::HwOptoHybrid::broadcastWrite(std::string const& name,
                                                       uint32_t    const& mask,
                                                       uint32_t    const& value,
                                                       bool reset)
{
  if (reset)
    writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Reset"),0x1);
  writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Mask"),mask);
  writeReg(getDeviceBaseNode(),toolbox::toString("GEB.Broadcast.Request.%s", name.c_str()),value);
}


std::vector<std::pair<uint8_t,uint32_t> > gem::hw::optohybrid::HwOptoHybrid::getConnectedVFATs()
{
  std::vector<uint32_t> chips0 = broadcastRead("ChipID0",ALL_VFATS_BCAST_MASK,false);
  std::vector<uint32_t> chips1 = broadcastRead("ChipID1",ALL_VFATS_BCAST_MASK,false);
  DEBUG("chips0 size:" << chips0.size() <<  ", chips1 size:" << chips1.size());
  
  std::vector<std::pair<uint8_t, uint32_t> > chipIDs;
  std::vector<std::pair<uint32_t,uint32_t> > chipPairs;
  chipPairs.reserve(chips0.size());
  
  std::transform(chips1.begin(), chips1.end(), chips0.begin(),
                 std::back_inserter(chipPairs),
                 std::make_pair<uint32_t,uint32_t>);
  
  for (auto chip = chipPairs.begin(); chip != chipPairs.end(); ++chip) {
    uint8_t slot = ((chip->first)>>8)&0xff;
    uint32_t chipID = (((chip->first)&0xff)<<8)+((chip->second)&0xff);
    DEBUG("GEB slot: " << (int)slot
          << ", chipID1: 0x" << std::hex << chip->first   << std::dec
          << ", chipID2: 0x" << std::hex << chip->second  << std::dec
          << ", chipID: 0x"  << std::hex << chipID        << std::dec);
    chipIDs.push_back(std::make_pair(slot,chipID));
  }
  return chipIDs;
}


uint32_t gem::hw::optohybrid::HwOptoHybrid::getConnectedVFATMask()
{
  std::vector<uint32_t> allChips = broadcastRead("ChipID0",0xff000000);
  uint32_t connectedMask = 0x0; // high means don't broadcast
  uint32_t disabledMask  = 0x0; // high means ignore data
  DEBUG("Reading ChipID0 from all possible slots");
  for (auto id = allChips.begin(); id != allChips.end(); ++id) {
    // 0x00XXYYZZ
    // XX = status (00000EVR)
    // YY = chip number
    // ZZ = register contents
    DEBUG("result 0x" << std::setw(8) << std::setfill('0') << std::hex << *id << std::dec);
    bool e_bit(((*id)>>18)&0x1),v_bit(((*id)>>17)&0x1),r_bit(((*id)>>16)&0x1);
    
    if (v_bit && !e_bit) {
      uint8_t shift = ((*id)>>8)&0xff;
      connectedMask |= (0x1 << shift);
      disabledMask  |= (0x1 << shift);
    }
    DEBUG("mask is " << std::hex << connectedMask << std::dec);
  }
  
  connectedMask = ~connectedMask;
  disabledMask  = ~disabledMask ;
  DEBUG("final mask is 0x" << std::setw(8) << std::setfill('0') << std::hex << connectedMask << std::dec);
  return connectedMask;
}
