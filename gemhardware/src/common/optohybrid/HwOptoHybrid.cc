#include <bitset>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <functional>

#include "gem/hw/optohybrid/HwOptoHybrid.h"

// gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid() :
//   gem::hw::GEMHwDevice::GEMHwDevice("HwOptoHybrid"),
//   //monOptoHybrid_(0)
//   b_links{false,false,false},
//   m_controlLink(-1)
// {
//   setDeviceID("OptoHybridHw");
//   setAddressTableFileName("uhal_gem_amc_glib_link00.xml");
//   //need to know which device this is 0 or 1?
//   //need to fix the hard coded '0', how to get it in from the constructor in a sensible way? /**JS Oct 8**/
//   setDeviceBaseNode("GEM_AMC.OH.OH0");
//   //gem::hw::optohybrid::HwOptoHybrid::initDevice();
//   //set up which links are active, so that the control can be done without specifying a link
//   INFO("HwOptoHybrid ctor done " << isHwConnected());
// }

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(std::string const& optohybridDevice,
                                                std::string const& connectionFile) :
  gem::hw::GEMHwDevice::GEMHwDevice(optohybridDevice, connectionFile),
  //monOptoHybrid_(0)
  b_links{false,false,false},
  b_is_initial(true),
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
  b_links{false,false,false},
  b_is_initial(true),
  m_controlLink(-1)
{
  setAddressTableFileName(toolbox::toString("uhal_gem_amc_glib_link%02d.xml",*optohybridDevice.rbegin()));
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
  b_links{false,false,false},
  b_is_initial(true),
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
  b_links{false,false,false},
  b_is_initial(true),
  m_controlLink(-1),
  m_slot((int)slot)
{
  INFO("HwOptoHybrid creating OptoHybrid device from GLIB device " << glibDevice.getLoggerName());
  //use a connection file and connection manager?
  setDeviceID(toolbox::toString("%s.optohybrid%02d",glibDevice.getDeviceID().c_str(),slot));
  //uhal::ConnectionManager manager ( "file://${GEM_ADDRESS_TABLE_PATH}/connections.xml" );
  p_gemConnectionManager.reset(new uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections.xml"));
  p_gemHW.reset(new uhal::HwInterface(p_gemConnectionManager->getDevice(this->getDeviceID())));
  //p_gemConnectionManager = std::shared_ptr<uhal::ConnectionManager>(uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections.xml"));
  //p_gemHW = std::shared_ptr<uhal::HwInterface>(p_gemConnectionManager->getDevice(this->getDeviceID()));
  std::stringstream basenode;
  basenode << "GEM_AMC.OH.OH" << (int)slot;
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
  if (b_is_connected) {
    DEBUG("HwOptoHybrid connection good");
    return true;
  } else if (gem::hw::GEMHwDevice::isHwConnected()) {
    DEBUG("Checking hardware connection");

    // FIXME IN FIRMWARE, need better check of connectivity...
    if ((this->getFirmwareDateString()).rfind("15") != std::string::npos ||
        (this->getFirmwareDateString()).rfind("16") != std::string::npos ||
        (this->getFirmwareDateString()).rfind("17") != std::string::npos) {
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

  linkStatus.GTX_TRK_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX_LINK.TRK_ERR"));
  linkStatus.GTX_TRG_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX_LINK.TRG_ERR"));
  linkStatus.GTX_Data_Packets = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX_LINK.DATA_Packets"));
  linkStatus.GBT_TRK_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GBT_LINK.TRK_ERR"));
  linkStatus.GBT_Data_Packets = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GBT_LINK.DATA_Packets"));
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
  if (resets&0x8)
    writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX.TRK_ERR.Reset"),0x1);
  if (resets&0x10)
    writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GBT.DATA_Packets.Reset"),0x1);
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
    TRACE("HwOptoHybrid::broadcastRead transaction on "
          << name << " is still running...");
    usleep(10);
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
    usleep(10);
  }
  auto t2 = std::chrono::high_resolution_clock::now();
  TRACE("HwOptoHybrid::broadcastWrite transaction on " << name << " lasted "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() << "ns");
}


std::vector<std::pair<uint8_t,uint32_t> > gem::hw::optohybrid::HwOptoHybrid::getConnectedVFATs(bool update)
{
  if (update || b_is_initial) {
    std::vector<uint32_t> chips0 = broadcastRead("ChipID0",ALL_VFATS_BCAST_MASK,false);
    std::vector<uint32_t> chips1 = broadcastRead("ChipID1",ALL_VFATS_BCAST_MASK,false);
    DEBUG("HwOptoHybrid::getConnectedVFATs chips0 size:" << chips0.size() <<  ", chips1 size:" << chips1.size());

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
        DEBUG("HwOptoHybrid::getConnectedVFATs GEB slot: " << (int)slot
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

     m_disabledMask  = connectedMask;
     m_connectedMask = disabledMask;
  }
  return m_connectedMask;
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

//////// T1 Controller Module \\\\\\\\*
void gem::hw::optohybrid::HwOptoHybrid::configureT1Generator(uint8_t const& mode, uint8_t const& type,
                                                             T1Sequence sequence, bool reset)
{
  if (reset)
    resetT1Generator();

  writeReg(getDeviceBaseNode(),"T1Controller.MODE",mode);
  if (mode == 0x0) {
    writeReg(getDeviceBaseNode(),"T1Controller.TYPE",type);
  } else if (mode == 0x2) {
    writeReg(getDeviceBaseNode(),"T1Controller.Sequence.L1A.MSB",     sequence.l1a_seq>>32);
    writeReg(getDeviceBaseNode(),"T1Controller.Sequence.L1A.LSB",     sequence.l1a_seq&0xffffffff);
    writeReg(getDeviceBaseNode(),"T1Controller.Sequence.CalPulse.MSB",sequence.cal_seq>>32);
    writeReg(getDeviceBaseNode(),"T1Controller.Sequence.CalPulse.LSB",sequence.cal_seq&0xffffffff);
    writeReg(getDeviceBaseNode(),"T1Controller.Sequence.Resync.MSB",  sequence.rsy_seq>>32);
    writeReg(getDeviceBaseNode(),"T1Controller.Sequence.Resync.LSB",  sequence.rsy_seq&0xffffffff);
    writeReg(getDeviceBaseNode(),"T1Controller.Sequence.BCO.MSB",     sequence.bc0_seq>>32);
    writeReg(getDeviceBaseNode(),"T1Controller.Sequence.BC0.LSB",     sequence.bc0_seq&0xffffffff);
  }
}

void gem::hw::optohybrid::HwOptoHybrid::startT1Generator(uint32_t const& ntrigs, uint32_t const& rate, uint32_t const& delay)
{
  uint32_t interval = 1/(rate*0.000000025);

  writeReg(getDeviceBaseNode(),"T1Controller.NUMBER"  ,ntrigs  );
  writeReg(getDeviceBaseNode(),"T1Controller.INTERVAL",interval);
  writeReg(getDeviceBaseNode(),"T1Controller.DELAY"   ,delay   );

  //don't toggle off if the generator is currently running
  if (!getT1GeneratorStatus())
    writeReg(getDeviceBaseNode(),"T1Controller.TOGGLE",0x1);
}

void gem::hw::optohybrid::HwOptoHybrid::stopT1Generator(bool reset)
{
  //don't toggle on if the generator is currently not running
  if (getT1GeneratorStatus())
    writeReg(getDeviceBaseNode(),"T1Controller.TOGGLE",0x1);
  if (reset)
    resetT1Generator();
}

void gem::hw::optohybrid::HwOptoHybrid::sendL1A(uint32_t const& ntrigs, uint32_t const& rate)
{
  T1Sequence sequence;
  configureT1Generator(0x0, 0x0, sequence, true);
  startT1Generator(ntrigs,rate, 0);
}

void gem::hw::optohybrid::HwOptoHybrid::sendCalPulse(uint32_t const& npulse, uint32_t const& rate)
{
  T1Sequence sequence;
  configureT1Generator(0x0, 0x1, sequence, true);
  startT1Generator(npulse, rate, 0);
}

void gem::hw::optohybrid::HwOptoHybrid::sendL1ACalPulse(uint32_t const& npulse, uint32_t const& delay, uint32_t const& rate)
{
  T1Sequence sequence;
  configureT1Generator(0x1, 0x0, sequence, true);
  startT1Generator(npulse,rate, delay);
}

void gem::hw::optohybrid::HwOptoHybrid::sendResync(uint32_t const& nresync, uint32_t const& rate)
{
  // OBSOLETE writeReg(getDeviceBaseNode(), "CONTROL.TRIGGER.SOURCE",0x0);
  // OBSOLETE writeReg(getDeviceBaseNode(), "CONTROL.CLOCK.REF_CLK",0x1);
  T1Sequence sequence;
  configureT1Generator(0x0, 0x2, sequence, true);
  startT1Generator(nresync, rate, 0);
}


void gem::hw::optohybrid::HwOptoHybrid::sendBC0(uint32_t const& nbc0, uint32_t const& rate)
{
  T1Sequence sequence;
  configureT1Generator(0x0, 0x3, sequence, true);
  startT1Generator(nbc0, rate, 0);
}

//////// Optohybrid ADC Interface \\\\\\\\*
uint32_t gem::hw::optohybrid::HwOptoHybrid::getADCVAUX(uint8_t const& vaux)
{
  if (vaux > 15) {
    std::string msg = toolbox::toString("Invalid VAUX requested (%d): outside expectation (0-15)",int(vaux));
    ERROR(msg);
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
    ERROR(msg);
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
    ERROR(msg);
    return 0;
  }
  return readReg(getDeviceBaseNode(),vauxInput.str());
}

uint32_t gem::hw::optohybrid::HwOptoHybrid::getVFATDACOutI(uint8_t const& column)
{
  if (column > 2) {
    std::string msg = toolbox::toString("Invalid column requested (%d): outside expectation (0-2)",int(column));
    ERROR(msg);
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
    ERROR(msg);
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
  std::stringstream scanBase;
  scanBase << "ScanController";
  if (useUltra)
    scanBase << ".ULTRA";
  else
    scanBase << ".THLAT";

  if (reset)
    writeReg(getDeviceBaseNode(),scanBase.str()+".RESET",0x1);

  writeReg(getDeviceBaseNode(),scanBase.str()+".CONF.MODE", mode);
  writeReg(getDeviceBaseNode(),scanBase.str()+".CONF.MIN",  min);
  writeReg(getDeviceBaseNode(),scanBase.str()+".CONF.MAX",  max);
  writeReg(getDeviceBaseNode(),scanBase.str()+".CONF.STEP", step);

  // need also to enable this chip and disable all others, use a broadcast write?
  if (useUltra)
    writeReg(getDeviceBaseNode(),scanBase.str()+".CONF.MASK", chip);
  else
    writeReg(getDeviceBaseNode(),scanBase.str()+".CONF.CHIP", chip);
  if (mode == 0x1 || mode == 0x3) {
    // protect for non-existent channels?
    // need also to enable this channel and disable all others
    writeReg(getDeviceBaseNode(),scanBase.str()+".CONF.CHAN",channel);
    if (mode == 0x3) {
      // need also to enable cal pulse to this channel and disable all others
    }
  }
}

void gem::hw::optohybrid::HwOptoHybrid::startScanModule(uint32_t const& nevts, bool useUltra)
{
  std::stringstream scanBase;
  scanBase << "ScanController";
  if (useUltra)
    scanBase << ".ULTRA";
  else
    scanBase << ".THLAT";

  writeReg(getDeviceBaseNode(),scanBase.str()+".CONF.NTRIGS", nevts);

  //don't toggle off if the module is currently running
  if (!getScanStatus())
    writeReg(getDeviceBaseNode(),scanBase.str()+".START",0x1);
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
    INFO("Scan still running, not returning results");
    usleep(10);
  }
  std::stringstream regname;
  regname << getDeviceBaseNode() << ".ScanController.THLAT.RESULTS";
  return readBlock(regname.str(), npoints);
}

std::vector<std::vector<uint32_t> > gem::hw::optohybrid::HwOptoHybrid::getUltraScanResults(uint32_t const& npoints)
{
  while (readReg(getDeviceBaseNode(),"ScanController.ULTRA.MONITOR.STATUS") > 0) {
    INFO("Scan still running, not returning results");
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
void gem::hw::optohybrid::HwOptoHybrid::updateWBMasterCounters()
{
  std::stringstream regName;
  regName << getDeviceBaseNode() << ".COUNTERS.WB.MASTER";
  std::vector<std::pair<std::string,uint32_t> > wishboneRegisters;
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Strobe.GTX"   ,regName.str().c_str()),
                                             m_wbMasterCounters.GTX.first));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Ack.GTX"      ,regName.str().c_str()),
                                             m_wbMasterCounters.GTX.second));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Strobe.GBT"   ,regName.str().c_str()),
                                             m_wbMasterCounters.GBT.first));
  wishboneRegisters.push_back(std::make_pair(toolbox::toString("%s.Ack.GBT"      ,regName.str().c_str()),
                                             m_wbMasterCounters.GBT.second));
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
  wishboneRegisters.push_back(toolbox::toString("%s.Strobe.GBT.Reset"   ,regName.str().c_str()));
  wishboneRegisters.push_back(toolbox::toString("%s.Ack.GBT.Reset"      ,regName.str().c_str()));
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

std::pair<uint32_t,uint32_t> gem::hw::optohybrid::HwOptoHybrid::getVFATCRCCount(uint8_t const& chip)
{
  std::stringstream vfatCRC;
  vfatCRC << getDeviceBaseNode() << ".COUNTERS.CRC.";
  std::vector<std::pair<std::string,uint32_t> > vfatCRCRegs;
  uint32_t valid(0x0), incorrect(0x0);
  vfatCRCRegs.push_back(std::make_pair(toolbox::toString("%s.VALID.VFAT%d.Reset",vfatCRC.str().c_str(),(int)chip),
                                       valid));
  vfatCRCRegs.push_back(std::make_pair(toolbox::toString("%s.INCORRECT.VFAT%d.Reset",vfatCRC.str().c_str(),(int)chip),
                                       incorrect));
  readRegs(vfatCRCRegs);
  return std::make_pair(valid,incorrect);
}


void gem::hw::optohybrid::HwOptoHybrid::resetVFATCRCCount(uint8_t const& chip)
{
  std::stringstream vfatCRC;
  vfatCRC << getDeviceBaseNode() << ".COUNTERS.CRC.";
  std::vector<std::string> vfatCRCRegs;
  vfatCRCRegs.push_back(toolbox::toString("%s.VALID.VFAT%d.Reset",vfatCRC.str().c_str(),(int)chip));
  vfatCRCRegs.push_back(toolbox::toString("%s.INCORRECT.VFAT%d.Reset",vfatCRC.str().c_str(),(int)chip));
  writeValueToRegs(vfatCRCRegs, 0x1);
  return;
}

void gem::hw::optohybrid::HwOptoHybrid::resetVFATCRCCount()
{
  std::stringstream vfatCRC;
  vfatCRC << getDeviceBaseNode() << ".COUNTERS.CRC";
  std::vector<std::string> vfatCRCRegs;
  for (int vfat = 0; vfat < 24; ++vfat) {
    vfatCRCRegs.push_back(toolbox::toString("%s.VALID.VFAT%d.Reset",vfatCRC.str().c_str(),vfat));
    vfatCRCRegs.push_back(toolbox::toString("%s.INCORRECT.VFAT%d.Reset",vfatCRC.str().c_str(),vfat));
  }
  writeValueToRegs(vfatCRCRegs, 0x1);
  return;
}

//// T1 Counters \\\\*
void gem::hw::optohybrid::HwOptoHybrid::updateT1Counters()
{
  for (unsigned signal = 0; signal < 4; ++signal) {
    m_t1Counters.GTX_TTC.at(signal)  = getT1Count(signal, 0x0);
    m_t1Counters.Firmware.at(signal) = getT1Count(signal, 0x1);
    m_t1Counters.External.at(signal) = getT1Count(signal, 0x2);
    m_t1Counters.Loopback.at(signal) = getT1Count(signal, 0x3);
    m_t1Counters.Sent.at(    signal) = getT1Count(signal, 0x4);
    m_t1Counters.GBT_TTC.at( signal) = getT1Count(signal, 0x5);
  }
}

void gem::hw::optohybrid::HwOptoHybrid::resetT1Counters()
{
  resetT1Count(0x0, 0x6); //reset all L1A counters
  resetT1Count(0x1, 0x6); //reset all CalPulse counters
  resetT1Count(0x2, 0x6); //reset all Resync counters
  resetT1Count(0x3, 0x6); //reset all BC0 counters
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

uint32_t gem::hw::optohybrid::HwOptoHybrid::getT1Count(uint8_t const& signal, uint8_t const& mode)
{
  std::stringstream t1Signal;
  if (signal == 0x0)
    t1Signal << "L1A";
  if (signal == 0x1)
    t1Signal << "CalPulse";
  if (signal == 0x2)
    t1Signal << "Resync";
  if (signal == 0x3)
    t1Signal << "BC0";

  switch(mode) {
  case(OptoHybridTTCMode::GTX_TTC):
    return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.GTX_TTC.%s", (t1Signal.str()).c_str()));
  case(OptoHybridTTCMode::INTERNAL):
    return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.INTERNAL.%s",(t1Signal.str()).c_str()));
  case(OptoHybridTTCMode::EXTERNAL):
    return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.EXTERNAL.%s",(t1Signal.str()).c_str()));
  case(OptoHybridTTCMode::LOOPBACK):
    return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.LOOPBACK.%s",(t1Signal.str()).c_str()));
  case(OptoHybridTTCMode::SENT):
    return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.SENT.%s",    (t1Signal.str()).c_str()));
  case(OptoHybridTTCMode::GBT_TTC):
    return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.GBT_TTC.%s", (t1Signal.str()).c_str()));
  default:
    return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.SENT.%s",    (t1Signal.str()).c_str()));
  }
}

void gem::hw::optohybrid::HwOptoHybrid::resetT1Count(uint8_t const& signal, uint8_t const& mode)
{
  std::stringstream t1Signal;
  if (signal == 0x0)
    t1Signal << "L1A";
  if (signal == 0x1)
    t1Signal << "CalPulse";
  if (signal == 0x2)
    t1Signal << "Resync";
  if (signal == 0x3)
    t1Signal << "BC0";

  std::stringstream regName;
  regName << getDeviceBaseNode() << ".COUNTERS.T1";
  std::vector<std::string> l1aCounterRegisters;

  switch(mode) {
  case(OptoHybridTTCMode::GTX_TTC):
    l1aCounterRegisters.push_back(toolbox::toString("%s.GTX_TTC.%s.Reset", regName.str().c_str(),t1Signal.str().c_str()));
    break;
  case(OptoHybridTTCMode::INTERNAL):
    l1aCounterRegisters.push_back(toolbox::toString("%s.INTERNAL.%s.Reset",regName.str().c_str(),t1Signal.str().c_str()));
    break;
  case(OptoHybridTTCMode::EXTERNAL):
    l1aCounterRegisters.push_back(toolbox::toString("%s.EXTERNAL.%s.Reset",regName.str().c_str(),t1Signal.str().c_str()));
    break;
 case(OptoHybridTTCMode::LOOPBACK):
    l1aCounterRegisters.push_back(toolbox::toString("%s.LOOPBACK.%s.Reset",regName.str().c_str(),t1Signal.str().c_str()));
    break;
  case(OptoHybridTTCMode::SENT):
    l1aCounterRegisters.push_back(toolbox::toString("%s.SENT.%s.Reset",    regName.str().c_str(),t1Signal.str().c_str()));
    break;
  case(OptoHybridTTCMode::GBT_TTC):
    l1aCounterRegisters.push_back(toolbox::toString("%s.GBT_TTC.%s.Reset", regName.str().c_str(),t1Signal.str().c_str()));
    break;
  case(OptoHybridTTCMode::RESET):
    l1aCounterRegisters.push_back(toolbox::toString("%s.GTX_TTC.%s.Reset", regName.str().c_str(),t1Signal.str().c_str()));
    l1aCounterRegisters.push_back(toolbox::toString("%s.INTERNAL.%s.Reset",regName.str().c_str(),t1Signal.str().c_str()));
    l1aCounterRegisters.push_back(toolbox::toString("%s.EXTERNAL.%s.Reset",regName.str().c_str(),t1Signal.str().c_str()));
    l1aCounterRegisters.push_back(toolbox::toString("%s.LOOPBACK.%s.Reset",regName.str().c_str(),t1Signal.str().c_str()));
    l1aCounterRegisters.push_back(toolbox::toString("%s.SENT.%s.Reset",    regName.str().c_str(),t1Signal.str().c_str()));
    l1aCounterRegisters.push_back(toolbox::toString("%s.GBT_TTC.%s.Reset", regName.str().c_str(),t1Signal.str().c_str()));
    break;
  default:
    l1aCounterRegisters.push_back(toolbox::toString("%s.GTX_TTC.%s.Reset", regName.str().c_str(),t1Signal.str().c_str()));
    l1aCounterRegisters.push_back(toolbox::toString("%s.INTERNAL.%s.Reset",regName.str().c_str(),t1Signal.str().c_str()));
    l1aCounterRegisters.push_back(toolbox::toString("%s.EXTERNAL.%s.Reset",regName.str().c_str(),t1Signal.str().c_str()));
    l1aCounterRegisters.push_back(toolbox::toString("%s.LOOPBACK.%s.Reset",regName.str().c_str(),t1Signal.str().c_str()));
    l1aCounterRegisters.push_back(toolbox::toString("%s.SENT.%s.Reset",    regName.str().c_str(),t1Signal.str().c_str()));
    l1aCounterRegisters.push_back(toolbox::toString("%s.GBT_TTC.%s.Reset", regName.str().c_str(),t1Signal.str().c_str()));
    break;
  }
  writeValueToRegs(l1aCounterRegisters, 0x1);
}
