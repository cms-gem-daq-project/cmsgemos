#ifndef gem_hw_vfat_VFAT2Settings_h
#define gem_hw_vfat_VFAT2Settings_h

#include "gem/hw/vfat/VFAT2SettingsEnums.h"
#include "gem/hw/vfat/VFAT2Enums2Strings.h"
#include "gem/hw/vfat/VFAT2Strings2Enums.h"

namespace gem {
  namespace hw {
    namespace vfat {
      typedef struct {
	uint8_t fullChannelReg;
	uint8_t trimDAC;
	bool    mask;
	bool    calPulse;
	bool    calPulse0;
	
      } VFAT2ChannelParams;
      
      inline std::ostream& operator<<(std::ostream& os, const VFAT2ChannelParams& chanParams)
      {
	// write obj to stream
	os << "0x"<<std::hex<<(unsigned)chanParams.fullChannelReg<<std::dec<<"::<"
	   <<std::hex<<(unsigned)chanParams.calPulse0<<std::dec<<":"
	   <<std::hex<<(unsigned)chanParams.calPulse <<std::dec<<":"
	   <<std::hex<<(unsigned)chanParams.mask     <<std::dec<<":"
	   <<std::hex<<(unsigned)chanParams.trimDAC  <<std::dec<<">"
	   << std::endl;
	return os;
      };
      
      //class VFAT2Settings;

      typedef struct {
	/*
	  VFAT2Settings::RunMode::ERunMode                 runMode;
	  VFAT2Settings::TriggerMode::ETriggerMode         trigMode;
	  VFAT2Settings::MSPolarity::EMSPolarity           msPol;
	  VFAT2Settings::CalPolarity::ECalPolarity         calPol;
	  VFAT2Settings::CalibrationMode::ECalibrationMode calibMode;
	
	  VFAT2Settings::DACMode::EDACMode             dacMode;
	  VFAT2Settings::ProbeMode::EProbeMode         probeMode;
	  VFAT2Settings::LVDSPowerSave::ELVDSPowerSave lvdsMode;
	  VFAT2Settings::ReHitCT::EReHitCT             reHitCT;

	  VFAT2Settings::HitCountMode::EHitCountMode   hitCountMode;
	  VFAT2Settings::MSPulseLength::EMSPulseLength msPulseLen;
	  VFAT2Settings::DigInSel::EDigInSel           digInSel;

	  VFAT2Settings::TrimDACRange::ETrimDACRange   trimDACRange;
	  VFAT2Settings::PbBG::EPbBG                   padBandGap;
	  VFAT2Settings::DFTestPattern::EDFTestPattern sendTestPattern;
	*/
	VFAT2RunMode   runMode;
	VFAT2TrigMode  trigMode;
	VFAT2MSPol     msPol;
	VFAT2CalPol    calPol;
	VFAT2CalibMode calibMode;
	
	VFAT2DACMode   dacMode;
	VFAT2ProbeMode probeMode;
	VFAT2LVDSMode  lvdsMode;
	VFAT2ReHitCT   reHitCT;

	VFAT2HitCountMode  hitCountMode;
	VFAT2MSPulseLength msPulseLen;
	VFAT2DigInSel      digInSel;

	VFAT2TrimDACRange  trimDACRange;
	VFAT2PadBandgap    padBandGap;
	VFAT2DFTestPattern sendTestPattern;
	
	//any reason not to include these too???
	uint8_t control0;
	uint8_t control1;
	uint8_t control2;
	uint8_t control3;
	
	uint8_t latency;

	uint8_t iPreampIn;
	uint8_t iPreampFeed;
	uint8_t iPreampOut;
	uint8_t iShaper;
	uint8_t iShaperFeed;
	uint8_t iComp;

	uint8_t vCal;
	uint8_t vThresh1;
	uint8_t vThresh2;
	uint8_t calPhase;
	
	//counters
	uint16_t chipID;
	uint8_t  upsetCounter;
	uint32_t hitCounter;
	
	gem::hw::vfat::VFAT2ChannelParams channels[128];
	uint8_t activeChannel;

      } VFAT2ControlParams;

      inline std::ostream& operator<<(std::ostream& os, const VFAT2ControlParams& controlParams)
      {
	// write obj to stream
	os << "CR0::0x"<<std::hex<<(unsigned)controlParams.control0<<std::dec<<"<"
	   <<CalibrationModeToString.at(controlParams.calibMode)
	   <<(unsigned)(controlParams.calibMode)
	   <<":"
	   <<CalPolarityToString.at(controlParams.calPol)
	   <<(unsigned)(controlParams.calPol)
	   <<":"
	   <<MSPolarityToString.at(controlParams.msPol)
	   <<(unsigned)(controlParams.msPol)
	   <<":"
	   <<TriggerModeToString.at(controlParams.trigMode)
	   <<(unsigned)(controlParams.trigMode)
	   <<":"
	   <<RunModeToString.at(controlParams.runMode)
	   <<(unsigned)(controlParams.runMode)
	   <<">"<< std::endl;

	os << "CR1::0x"<<std::hex<<(unsigned)controlParams.control1<<std::dec<<"<"
	   <<ReHitCTToString.at(controlParams.reHitCT)
	   <<(unsigned)(controlParams.reHitCT)
	   <<":"
	   <<LVDSPowerSaveToString.at(controlParams.lvdsMode)
	   <<(unsigned)(controlParams.lvdsMode)
	   <<":"
	   <<ProbeModeToString.at(controlParams.probeMode)
	   <<(unsigned)(controlParams.probeMode)
	   <<":"
	   <<DACModeToString.at(controlParams.dacMode)
	   <<(unsigned)(controlParams.dacMode)
	   <<">"<< std::endl;
	  
	os << "CR2::0x"<<std::hex<<(unsigned)controlParams.control2<<std::dec<<"<"
	   <<DigInSelToString.at(controlParams.digInSel)
	   <<(unsigned)(controlParams.digInSel)
	   <<":"
	   <<MSPulseLengthToString.at(controlParams.msPulseLen)
	   <<(unsigned)(controlParams.msPulseLen)
	   <<":"
	   <<HitCountModeToString.at(controlParams.hitCountMode)
	   <<(unsigned)(controlParams.hitCountMode)
	   <<">"<< std::endl;
	  
	os << "CR3::0x"<<std::hex<<(unsigned)controlParams.control3<<std::dec<<"<"
	   <<DFTestPatternToString.at(controlParams.sendTestPattern)
	   <<(unsigned)(controlParams.sendTestPattern)
	   <<":"
	   <<PbBGToString.at(controlParams.padBandGap)
	   <<(unsigned)(controlParams.padBandGap)
	   <<":"
	   <<TrimDACRangeToString.at(controlParams.trimDACRange)
	   <<(unsigned)(controlParams.trimDACRange)
	   <<">"<< std::endl;

	os << "Latency::0x"<<std::hex<<(unsigned)controlParams.latency<<std::dec<<std::endl

	   << "IPreampIn::0x"<<std::hex<<(unsigned)controlParams.iPreampIn<<std::dec<<std::endl
	   << "IPreampFeed::0x"<<std::hex<<(unsigned)controlParams.iPreampFeed<<std::dec<<std::endl
	   << "IPreampOut::0x"<<std::hex<<(unsigned)controlParams.iPreampOut<<std::dec<<std::endl

	   << "IShaper::0x"<<std::hex<<(unsigned)controlParams.iShaper<<std::dec<<std::endl
	   << "IShaperFeed::0x"<<std::hex<<(unsigned)controlParams.iShaperFeed<<std::dec<<std::endl

	   << "IComp::0x"<<std::hex<<(unsigned)controlParams.iComp<<std::dec<<std::endl

	   << "VCal::0x"<<std::hex<<(unsigned)controlParams.vCal<<std::dec<<std::endl

	   << "VThreshold1::0x"<<std::hex<<(unsigned)controlParams.vThresh1<<std::dec<<std::endl
	   << "VThreshold2::0x"<<std::hex<<(unsigned)controlParams.vThresh2<<std::dec<<std::endl

	   << "CalPhase::0x"<<std::hex<<(unsigned)controlParams.calPhase<<std::dec<<std::endl

	   << "ChipID::0x"<<std::hex<<(unsigned)controlParams.chipID<<std::dec<<std::endl
	   << "UpsetCounter::0x"<<std::hex<<(unsigned)controlParams.upsetCounter<<std::dec<<std::endl
	   << "HitCounter::0x"<<std::hex<<(unsigned)controlParams.hitCounter<<std::dec<<std::endl

	   << "active display channel: "<<(unsigned)controlParams.activeChannel<<std::endl;
	  
	for (uint8_t chan = 1; chan < 129; ++chan)
	  os << "Channel" << (unsigned)chan << "::" << controlParams.channels[chan-1];
	  
	return os;
      };
      
    }//end namespace gem::hw::vfat
    
  }//end namespace gem::hw
  
}//end namespace gem

#endif
