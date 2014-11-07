//General structure taken blatantly from tcds::utils::HwDeviceTCA/HwGLIB

#include "gem/hw/vfat/HwVFAT2.h"

#define DEBUG(MSG) LOG4CPLUS_DEBUG(logGEMHw_ , MSG)
#define INFO( MSG) LOG4CPLUS_INFO( logGEMHw_ , MSG)
#define WARN( MSG) LOG4CPLUS_WARN( logGEMHw_ , MSG)
#define ERROR(MSG) LOG4CPLUS_ERROR(logGEMHw_ , MSG)
#define FATAL(MSG) LOG4CPLUS_FATAL(logGEMHw_ , MSG)

gem::hw::vfat::HwVFAT2::HwVFAT2(xdaq::Application* vfat2App,
				std::string const& vfatDevice):
  gem::hw::GEMHwDevice::GEMHwDevice(vfat2App)
//logVFAT2_(vfat2App->getApplicationLogger()),
//hwVFAT2_(0)
//monVFAT2_(0)
{
  //this->gem::hw::GEMHwDevice::GEMHwDevice();
  //gem::hw::vfat::HwVFAT2::initDevice();
  //can use a different address table for the VFAT access
  setAddressTableFileName("allregsnonfram.xml");
  setIPbusProtocolVersion("2.0");
  setDeviceID("VFAT2Hw");
  setDeviceBaseNode("user_regs.vfats."+vfatDevice);

  //what's the difference between connect, init, enable for VFAT?
  //check that register values are hardware default values, if not, something may be amiss
  
  //set register values to sw default values

  //hardware is enabled!

  //set register values to desired values

  //hardware is configured!

  //set run bit

  //hardware is running
}

gem::hw::vfat::HwVFAT2::~HwVFAT2()
{
  //what does release device do to the hardware?
  //does it turn it off?  it probably shouldn't, in case code crashes but
  //hardware should still take data
  releaseDevice();
}

void gem::hw::vfat::HwVFAT2::configureDevice()
{
  
}

uint32_t gem::hw::vfat::HwVFAT2::readReg(std::string const& regName)
{
  //Maybe want to use a lock to prevent hammering the HW device
  uint32_t regValue;
  regValue = gem::hw::GEMHwDevice::readReg(getDeviceBaseNode()+"."+regName);
  //sleep(1);
  //regValue = gem::hw::GEMHwDevice::readReg("user_regs.vfats.vfat_response");
  //sleep(1);
  return regValue;
}

//void gem::hw::vfat::HwVFAT2::readVFAT2Counters(gem::hw::vfat::VFAT2ControlParams &params)
void gem::hw::vfat::HwVFAT2::readVFAT2Counters()
{
  vfatParams_.chipID       = getChipID();
  vfatParams_.upsetCounter = getUpsetCount();
  vfatParams_.hitCounter   = getHitCount();
}

//void gem::hw::vfat::HwVFAT2::readVFAT2Channel(gem::hw::vfat::VFAT2ControlParams &params, uint8_t channel)
void gem::hw::vfat::HwVFAT2::readVFAT2Channel(uint8_t channel)
{
  uint8_t chanSettings = getChannelSettings(channel);
  if (channel>1)
    vfatParams_.activeChannel = (unsigned)channel;
  vfatParams_.channels[channel-1].fullChannelReg = chanSettings;
  vfatParams_.channels[channel-1].calPulse0 = false;
  if (channel == 1)
    vfatParams_.channels[channel-1].calPulse0 = ((chanSettings&VFAT2ChannelBitMasks::CHANCAL0)>>VFAT2ChannelBitShifts::CHANCAL0);
  vfatParams_.channels[channel-1].calPulse    = ((chanSettings&VFAT2ChannelBitMasks::CHANCAL )>>VFAT2ChannelBitShifts::CHANCAL );
  vfatParams_.channels[channel-1].mask        = ((chanSettings&VFAT2ChannelBitMasks::ISMASKED)>>VFAT2ChannelBitShifts::ISMASKED);
  vfatParams_.channels[channel-1].trimDAC     = ((chanSettings&VFAT2ChannelBitMasks::TRIMDAC )>>VFAT2ChannelBitShifts::TRIMDAC );
  INFO("readVFAT2Channel " << (unsigned)channel << " - 0x"
       <<std::hex<<static_cast<unsigned>(vfatParams_.channels[channel-1].fullChannelReg)<<std::dec<<"::<"
       <<std::hex<<static_cast<unsigned>(vfatParams_.channels[channel-1].calPulse0     )<<std::dec<<":"
       <<std::hex<<static_cast<unsigned>(vfatParams_.channels[channel-1].calPulse      )<<std::dec<<":"
       <<std::hex<<static_cast<unsigned>(vfatParams_.channels[channel-1].mask          )<<std::dec<<":"
       <<std::hex<<static_cast<unsigned>(vfatParams_.channels[channel-1].trimDAC       )<<std::dec<<">"
       << std::endl);
  
}

//void gem::hw::vfat::HwVFAT2::readVFAT2Channels(gem::hw::vfat::VFAT2ControlParams &params)
void gem::hw::vfat::HwVFAT2::readVFAT2Channels()
{
  for (uint8_t chan = 1; chan < 129; ++chan) {
    //readVFAT2Channel(vfatParams_,chan);
    readVFAT2Channel(chan);
    INFO("chan = "<< (unsigned)chan << "; activeChannel = " <<(unsigned)vfatParams_.activeChannel << std::endl);
  }
}

/////////************************///////////////
void gem::hw::vfat::HwVFAT2::setAllSettings(const gem::hw::vfat::VFAT2ControlParams &params) {
  //want to lock the hardware while performing this operation
  //when sending all these requests, might it be better to do a single transaction?
  
  //check that the hardware is alive
  //check that the settings are non-empty?
  uint8_t cont0 = 0x0;
  uint8_t cont1 = 0x0;
  uint8_t cont2 = 0x0;
  uint8_t cont3 = 0x0;
  
  setRunMode(        params.runMode   ,cont0);
  setTriggerMode(    params.trigMode  ,cont0);
  setMSPolarity(     params.msPol     ,cont0);
  setCalPolarity(    params.calPol    ,cont0);
  setCalibrationMode(params.calibMode ,cont0);
  writeReg("ContReg0",cont0);
  
  setDACMode(          params.dacMode  ,cont1);
  setProbeMode(        params.probeMode,cont1);
  setLVDSMode(         params.lvdsMode ,cont1);
  setHitCountCycleTime(params.reHitCT  ,cont1);
  writeReg("ContReg1",cont1);
  
  setHitCountMode( params.hitCountMode,cont2);
  setMSPulseLength(params.msPulseLen  ,cont2);
  setInputPadMode( params.digInSel    ,cont2);
  writeReg("ContReg2",cont2);
  
  setTrimDACRange(   params.trimDACRange   ,cont3);
  setBandgapPad(     params.padBandGap     ,cont3);
  sendTestPattern   (params.sendTestPattern,cont3);
  writeReg("ContReg3",cont3);

  setIPreampIn(  params.iPreampIn  );
  setIPreampFeed(params.iPreampFeed);
  setIPreampOut( params.iPreampOut );
  setIShaper(    params.iShaper    );
  setIShaperFeed(params.iShaperFeed);
  setIComp(      params.iComp      );
  
  setVCal(       params.vCal    );
  setVThreshold1(params.vThresh1);
  setVThreshold2(params.vThresh2);
  setCalPhase(   params.calPhase);

  //set the channel settings here
  //channel 0/1
  uint8_t chanReg = 0x0;
  chanReg|=(params.channels[0].trimDAC  <<VFAT2ChannelBitShifts::TRIMDAC );
  chanReg|=(params.channels[0].mask     <<VFAT2ChannelBitShifts::ISMASKED);
  chanReg|=(params.channels[0].calPulse <<VFAT2ChannelBitShifts::CHANCAL );
  chanReg|=(params.channels[0].calPulse0<<VFAT2ChannelBitShifts::CHANCAL0);
  writeReg("VFATChannels.ChanReg1",chanReg);

  for (uint8_t chan = 2; chan < N_VFAT2_CHANNELS+1; ++chan) {
    chanReg = 0x0;
    chanReg|=(params.channels[chan-1].trimDAC <<VFAT2ChannelBitShifts::TRIMDAC );
    chanReg|=(params.channels[chan-1].mask    <<VFAT2ChannelBitShifts::ISMASKED);
    chanReg|=(params.channels[chan-1].calPulse<<VFAT2ChannelBitShifts::CHANCAL );
    writeReg(toolbox::toString("VFATChannels.ChanReg%d",(unsigned)chan),(unsigned)chanReg);
  }
}

//void gem::hw::vfat::HwVFAT2::getAllSettings(gem::hw::vfat::VFAT2ControlParams &params) {
void gem::hw::vfat::HwVFAT2::getAllSettings() {
  //want to lock the hardware (and the params variable) while performing this operation
  //when sending all these requests, might it be better to do a single transaction?
  //check that the hardware is alive
  //maybe find a way to do a single transaction, using the vector read/write?
  //what is the performance cost/benefit

  INFO("getting all settings in HwVFAT2.cc");
  uint8_t cont0 = readVFATReg("ContReg0");
  uint8_t cont1 = readVFATReg("ContReg1");
  uint8_t cont2 = readVFATReg("ContReg2");
  uint8_t cont3 = readVFATReg("ContReg3");
  
  INFO("getting all settings in HwVFAT2.cc");
  vfatParams_.control0 = static_cast<unsigned>(cont0);
  INFO("getting all settings in HwVFAT2.cc");
  vfatParams_.control1 = static_cast<unsigned>(cont1);
  INFO("getting all settings in HwVFAT2.cc");
  vfatParams_.control2 = static_cast<unsigned>(cont2);
  INFO("getting all settings in HwVFAT2.cc");
  vfatParams_.control3 = static_cast<unsigned>(cont3);

  INFO("storing RunMode in HwVFAT2.cc");
  vfatParams_.runMode   = static_cast<VFAT2RunMode  >(getRunMode(        cont0));
  INFO("storing TriggerMode in HwVFAT2.cc");
  vfatParams_.trigMode  = static_cast<VFAT2TrigMode >(getTriggerMode(    cont0));
  INFO("storing MSPolarity in HwVFAT2.cc");
  vfatParams_.msPol     = static_cast<VFAT2MSPol    >(getMSPolarity(     cont0));
  INFO("storing CalPolarity in HwVFAT2.cc");
  vfatParams_.calPol    = static_cast<VFAT2CalPol   >(getCalPolarity(    cont0));
  INFO("storing CalibrationMode in HwVFAT2.cc");
  vfatParams_.calibMode = static_cast<VFAT2CalibMode>(getCalibrationMode(cont0));
  
  INFO("storing DACMode in HwVFAT2.cc");
  vfatParams_.dacMode   = static_cast<VFAT2DACMode  >(getDACMode(          cont1));
  INFO("storing ProbeMode in HwVFAT2.cc");
  vfatParams_.probeMode = static_cast<VFAT2ProbeMode>(getProbeMode(        cont1));
  INFO("storing LVDSMode in HwVFAT2.cc");
  vfatParams_.lvdsMode  = static_cast<VFAT2LVDSMode >(getLVDSMode(         cont1));
  INFO("storing HitCycleTime in HwVFAT2.cc");
  vfatParams_.reHitCT   = static_cast<VFAT2ReHitCT  >(getHitCountCycleTime(cont1));
  
  INFO("storing HitCountMode in HwVFAT2.cc");
  vfatParams_.hitCountMode = static_cast<VFAT2HitCountMode >(getHitCountMode( cont2));
  INFO("storing MSPulseLength in HwVFAT2.cc");
  vfatParams_.msPulseLen   = static_cast<VFAT2MSPulseLength>(getMSPulseLength(cont2));
  INFO("storing DigInSel in HwVFAT2.cc");
  vfatParams_.digInSel     = static_cast<VFAT2DigInSel     >(getInputPadMode( cont2));
  
  INFO("storing TrimDACRange in HwVFAT2.cc");
  vfatParams_.trimDACRange    = static_cast<VFAT2TrimDACRange >(getTrimDACRange(   cont3));
  INFO("storing PadBandgap in HwVFAT2.cc");
  vfatParams_.padBandGap      = static_cast<VFAT2PadBandgap   >(getBandgapPad(     cont3));
  INFO("storing DFTestPattern HwVFAT2.cc");
  vfatParams_.sendTestPattern = static_cast<VFAT2DFTestPattern>(getTestPatternMode(cont3));
  
  vfatParams_.latency = getLatency();

  vfatParams_.iPreampIn   = getIPreampIn()  ;
  vfatParams_.iPreampFeed = getIPreampFeed();
  vfatParams_.iPreampOut  = getIPreampOut() ;
  vfatParams_.iShaper     = getIShaper()    ;
  vfatParams_.iShaperFeed = getIShaperFeed();
  vfatParams_.iComp       = getIComp()      ;
  
  vfatParams_.vCal     = getVCal()       ;
  vfatParams_.vThresh1 = getVThreshold1();
  vfatParams_.vThresh2 = getVThreshold2();
  vfatParams_.calPhase = getCalPhase()   ;

  //counters
  INFO("getting all counters in HwVFAT2.cc");
  //readVFAT2Counters(params);
  readVFAT2Counters();
  
  //set the channel settings here
  INFO("getting all channel settings in HwVFAT2.cc");
  //readVFAT2Channels(params);
  readVFAT2Channels();
  INFO("done getting all settings in HwVFAT2.cc");
}

//channel specific settings
void gem::hw::vfat::HwVFAT2::sendCalPulseToChannel(uint8_t channel, bool on) {
  INFO(toolbox::toString("setting cal pulse for channel %d HwVFAT2.cc",(unsigned)channel));
  //channel should be 0 to 128
  if (channel > 128) {
    std::string msg =
      toolbox::toString("Channel specified (%d) larger than expected (128)",channel);
    XCEPT_RAISE(gem::hw::vfat::exception::NonexistentChannel,msg);
    return;
  }
  
  std::string registerName = "VFATChannels.ChanReg1";
  //registerName = toolbox::toString("VFATChannels.ChanReg%d",(unsigned)channel);
  if (channel > 1)
    registerName = toolbox::toString("VFATChannels.ChanReg%d",(unsigned)channel);
  //registerName = "VFATChannels.ChanReg1";
  
  uint8_t channelSettings = readVFATReg(registerName);
  
  if (channel == 0) 
    writeReg(registerName,(channelSettings&~VFAT2ChannelBitMasks::CHANCAL0)|(on ? 0x80 : 0x0));
  else
    writeReg(registerName,(channelSettings&~VFAT2ChannelBitMasks::CHANCAL)|(on ? 0x40 : 0x0));
}

void gem::hw::vfat::HwVFAT2::maskChannel(uint8_t channel, bool on) {
  INFO(toolbox::toString("setting mask for channel %d HwVFAT2.cc",(unsigned)channel));
  //channel should be 1 to 128
  if ((channel > 128) || (channel < 1)) {
    std::string msg =
      toolbox::toString("Channel specified (%d) outside expectation (1-128)",channel);
    XCEPT_RAISE(gem::hw::vfat::exception::NonexistentChannel,msg);
    return;
  }
  std::string registerName = toolbox::toString("VFATChannels.ChanReg%d",(unsigned)channel);
  uint8_t channelSettings = (readVFATReg(registerName)&~VFAT2ChannelBitMasks::ISMASKED);
  writeReg(registerName,channelSettings|(on ? 0x20 : 0x0));
}

uint8_t gem::hw::vfat::HwVFAT2::getChannelTrimDAC(uint8_t channel) {
  INFO(toolbox::toString("getting trim DAC for channel %d HwVFAT2.cc",(unsigned)channel));
  //channel should be 1 to 128
  if ((channel > 128) || (channel < 1)) {
    std::string msg =
      toolbox::toString("Channel specified (%d) outside expectation (1-128)",channel);
    XCEPT_RAISE(gem::hw::vfat::exception::NonexistentChannel,msg);
    return 0xff;
  }
  std::string registerName = toolbox::toString("VFATChannels.ChanReg%d",(unsigned)channel);
  return (readVFATReg(registerName)&VFAT2ChannelBitMasks::TRIMDAC);
}

void gem::hw::vfat::HwVFAT2::setChannelTrimDAC(uint8_t channel, uint8_t trimDAC) {
  INFO(toolbox::toString("setting trim DAC for channel %d HwVFAT2.cc",(unsigned)channel));
  //channel should be 1 to 128
  if ((channel > 128) || (channel < 1)) {
    std::string msg =
      toolbox::toString("Channel specified (%d) outside expectation (1-128)",channel);
    XCEPT_RAISE(gem::hw::vfat::exception::NonexistentChannel,msg);
    return;
  }
  std::string registerName = toolbox::toString("VFATChannels.ChanReg%d",channel);
  uint8_t channelSettings = (readVFATReg(registerName)&~VFAT2ChannelBitMasks::TRIMDAC)|trimDAC;
  writeReg(registerName,channelSettings|trimDAC);
}

/***
void gem::hw::vfat::HwVFAT2::setChannelTrimDAC(uint8_t channel, double trimDAC) {
  //channel should be 1 to 128
  if ((channel > 128) || (channel < 1)) {
    std::string msg =
      toolbox::toString("Channel specified (%d) outside expectation (1-128)",channel);
    XCEPT_RAISE(gem::hw::vfat::exception::NonexistentChannel,msg);
    return;
  }
  std::string registerName = "VFATChannels.ChanReg"+channel;
  uint8_t channelSettings = readVFATReg(registerName);
  writeReg(registerName,channelSettings|0x20);
}
***/
