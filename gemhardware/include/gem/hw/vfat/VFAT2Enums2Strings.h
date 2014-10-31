#ifndef gem_hw_vfat_VFAT2Enums2Strings_h
#define gem_hw_vfat_VFAT2Enums2Strings_h

#include "gem/hw/vfat/VFAT2SettingsEnums.h"
#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>
#include <iostream>

namespace gem {
  namespace hw {
    namespace vfat {
      //VFAT2ChannelSettings
      typedef boost::unordered_map<VFAT2ChannelSettings::ChannelBitMasks::EChannelBitMasks, std::string> enumChBitMaskToString;
      typedef boost::unordered_map<VFAT2Settings::ContRegBitMasks::EContRegBitMasks       , std::string> enumContRegBitMaskToString;
      typedef boost::unordered_map<VFAT2Settings::ContRegBitShifts::EContRegBitShifts     , std::string> enumContRegBitShiftToString;
      typedef boost::unordered_map<VFAT2Settings::RunMode::ERunMode                       , std::string> enumRunModeToString;
      typedef boost::unordered_map<VFAT2Settings::TriggerMode::ETriggerMode               , std::string> enumTrigModeToString;
      typedef boost::unordered_map<VFAT2Settings::CalibrationMode::ECalibrationMode       , std::string> enumCalibModeToString;
      typedef boost::unordered_map<VFAT2Settings::MSPolarity::EMSPolarity                 , std::string> enumMSPolToString;
      typedef boost::unordered_map<VFAT2Settings::CalPolarity::ECalPolarity               , std::string> enumCalPolToString;
      typedef boost::unordered_map<VFAT2Settings::ProbeMode::EProbeMode                   , std::string> enumProbeModeToString;
      typedef boost::unordered_map<VFAT2Settings::LVDSPowerSave::ELVDSPowerSave           , std::string> enumLVDSPSToString;
      typedef boost::unordered_map<VFAT2Settings::DACMode::EDACMode                       , std::string> enumDACModeToString;
      typedef boost::unordered_map<VFAT2Settings::HitCountMode::EHitCountMode             , std::string> enumHitCountModeToString;
      typedef boost::unordered_map<VFAT2Settings::MSPulseLength::EMSPulseLength           , std::string> enumMSPulseLenToString;
      typedef boost::unordered_map<VFAT2Settings::ReHitCT::EReHitCT                       , std::string> enumReHitCTToString;
      typedef boost::unordered_map<VFAT2Settings::DigInSel::EDigInSel                     , std::string> enumDigInSelToString;
      typedef boost::unordered_map<VFAT2Settings::TrimDACRange::ETrimDACRange             , std::string> enumTrimDACRangeToString;
      typedef boost::unordered_map<VFAT2Settings::PbBG::EPbBG                             , std::string> enumPbBGToString;
      typedef boost::unordered_map<VFAT2Settings::DFTestPattern::EDFTestPattern           , std::string> enumDFTestToString;

      const enumChBitMaskToString ChBitMaskToString = boost::assign::map_list_of
	(VFAT2ChannelSettings::ChannelBitMasks::TRIMDAC , "TRIMDAC" )
	(VFAT2ChannelSettings::ChannelBitMasks::ISMASKED, "ISMASKED")
	(VFAT2ChannelSettings::ChannelBitMasks::CHANCAL0, "CHANCAL0")
	(VFAT2ChannelSettings::ChannelBitMasks::CHANCAL , "CHANCAL" );

      //VFAT2Settings 
      const enumContRegBitMaskToString ContRegBitMaskToString = boost::assign::map_list_of
	(VFAT2Settings::ContRegBitMasks::RUNMODE       , "RUNMODE"       )
	(VFAT2Settings::ContRegBitMasks::TRIGMODE      , "TRIGMODE"      )
	(VFAT2Settings::ContRegBitMasks::MSPOL         , "MSPOL"         )
	(VFAT2Settings::ContRegBitMasks::CALPOL        , "CALPOL"        )
	(VFAT2Settings::ContRegBitMasks::CALMODE       , "CALMODE"       )
					              	               
	(VFAT2Settings::ContRegBitMasks::DACMODE       , "DACMODE"       )
	(VFAT2Settings::ContRegBitMasks::PROBEMODE     , "PROBEMODE"     )
	(VFAT2Settings::ContRegBitMasks::LVDSMODE      , "LVDSMODE"      )
	(VFAT2Settings::ContRegBitMasks::REHITCT       , "REHITCT"       )
					              	               
	(VFAT2Settings::ContRegBitMasks::HITCOUNTMODE  , "HITCOUNTMODE"  )
	(VFAT2Settings::ContRegBitMasks::MSPULSELENGTH , "MSPULSELENGTH" )
	(VFAT2Settings::ContRegBitMasks::DIGINSEL      , "DIGINSEL"      )

	(VFAT2Settings::ContRegBitMasks::TRIMDACRANGE  , "TRIMDACRANGE"  )
	(VFAT2Settings::ContRegBitMasks::PADBANDGAP    , "PADBANDGAP"    )
	(VFAT2Settings::ContRegBitMasks::DFTESTMODE    , "DFTESTMODE"    )
	(VFAT2Settings::ContRegBitMasks::SPARE         , "SPARE"         );

      const enumContRegBitShiftToString ContRegBitShiftToString = boost::assign::map_list_of
	(VFAT2Settings::ContRegBitShifts::RUNMODE       , "RUNMODE"       )
	(VFAT2Settings::ContRegBitShifts::TRIGMODE      , "TRIGMODE"      )
	(VFAT2Settings::ContRegBitShifts::MSPOL         , "MSPOL"         )
	(VFAT2Settings::ContRegBitShifts::CALPOL        , "CALPOL"        )
	(VFAT2Settings::ContRegBitShifts::CALMODE       , "CALMODE"       )
					              	               
	(VFAT2Settings::ContRegBitShifts::DACMODE       , "DACMODE"       )
	(VFAT2Settings::ContRegBitShifts::PROBEMODE     , "PROBEMODE"     )
	(VFAT2Settings::ContRegBitShifts::LVDSMODE      , "LVDSMODE"      )
	(VFAT2Settings::ContRegBitShifts::REHITCT       , "REHITCT"       )
					              	               
	(VFAT2Settings::ContRegBitShifts::HITCOUNTMODE  , "HITCOUNTMODE"  )
	(VFAT2Settings::ContRegBitShifts::MSPULSELENGTH , "MSPULSELENGTH" )
	(VFAT2Settings::ContRegBitShifts::DIGINSEL      , "DIGINSEL"      )

	(VFAT2Settings::ContRegBitShifts::TRIMDACRANGE  , "TRIMDACRANGE"  )
	(VFAT2Settings::ContRegBitShifts::PADBANDGAP    , "PADBANDGAP"    )
	(VFAT2Settings::ContRegBitShifts::DFTESTMODE    , "DFTESTMODE"    )
	(VFAT2Settings::ContRegBitShifts::SPARE         , "SPARE"         );


      const enumRunModeToString RunModeToString = boost::assign::map_list_of
	(VFAT2Settings::RunMode::SLEEP , "SLEEP" )
	(VFAT2Settings::RunMode::RUN   , "RUN"   );

      const enumTrigModeToString TriggerModeToString = boost::assign::map_list_of
	(VFAT2Settings::TriggerMode::NOTRIGGER , "NOTRIGGER" )
	(VFAT2Settings::TriggerMode::ONEMODE   , "ONEMODE"   )
	(VFAT2Settings::TriggerMode::FOURMODE  , "FOURMODE"  )
	(VFAT2Settings::TriggerMode::EIGHTMODE , "EIGHTMODE" )
	(VFAT2Settings::TriggerMode::GEMMODE   , "GEMMODE"   )
	//need logic to handle these, to remap to GEMMODE
        (VFAT2Settings::TriggerMode::SPARE0    , "GEMMODE"    )
	(VFAT2Settings::TriggerMode::SPARE1    , "GEMMODE"    )
	(VFAT2Settings::TriggerMode::SPARE2    , "GEMMODE"    );
      //(VFAT2Settings::TriggerMode::SPARE3    , "GEMMODE"    );
      
      const enumCalibModeToString CalibrationModeToString = boost::assign::map_list_of
	(VFAT2Settings::CalibrationMode::NORMAL   , "NORMAL"   )
	(VFAT2Settings::CalibrationMode::VCAL     , "VCAL"     )
	(VFAT2Settings::CalibrationMode::BASELINE , "BASELINE" )
	(VFAT2Settings::CalibrationMode::EXTERNAL , "EXTERNAL" );

      const enumMSPolToString MSPolarityToString = boost::assign::map_list_of
	(VFAT2Settings::MSPolarity::POSITIVE , "POSITIVE" )
	(VFAT2Settings::MSPolarity::NEGATIVE , "NEGATIVE" );

      const enumCalPolToString CalPolarityToString = boost::assign::map_list_of
	(VFAT2Settings::CalPolarity::POSITIVE , "POSITIVE" )
	(VFAT2Settings::CalPolarity::NEGATIVE , "NEGATIVE" );

      const enumProbeModeToString ProbeModeToString = boost::assign::map_list_of
	(VFAT2Settings::ProbeMode::OFF , "OFF" )
	(VFAT2Settings::ProbeMode::ON  , "ON"  );

      const enumLVDSPSToString LVDSPowerSaveToString = boost::assign::map_list_of
	(VFAT2Settings::LVDSPowerSave::ENABLEALL , "ENABLEALL" )
	(VFAT2Settings::LVDSPowerSave::POWERSAVE , "POWERSAVE" );

      const enumDACModeToString DACModeToString = boost::assign::map_list_of
	(VFAT2Settings::DACMode::NORMAL      , "NORMAL"      )
	(VFAT2Settings::DACMode::IPREAMPIN   , "IPREAMPIN"   )
	(VFAT2Settings::DACMode::IPREAMPFEED , "IPREAMPFEED" )
	(VFAT2Settings::DACMode::IPREAMPOUT  , "IPREAMPOUT"  )
	(VFAT2Settings::DACMode::ISHAPER     , "ISHAPER"     )
	(VFAT2Settings::DACMode::ISHAPERFEED , "ISHAPERFEED" )
	(VFAT2Settings::DACMode::ICOMP       , "ICOMP"       )
	(VFAT2Settings::DACMode::ITHRESHOLD1 , "ITHRESHOLD1" )
	(VFAT2Settings::DACMode::ITHRESHOLD2 , "ITHRESHOLD2" )
	(VFAT2Settings::DACMode::VCAL        , "VCAL"        )
	(VFAT2Settings::DACMode::CALOUT      , "CALOUT"      )
	//need logic to prevent these from being set until they are actually used
	(VFAT2Settings::DACMode::SPARE0    , "SPARE0"    )
	(VFAT2Settings::DACMode::SPARE1    , "SPARE1"    )
	(VFAT2Settings::DACMode::SPARE2    , "SPARE2"    )
	(VFAT2Settings::DACMode::SPARE3    , "SPARE3"    )
	(VFAT2Settings::DACMode::SPARE4    , "SPARE4"    );

      const enumHitCountModeToString HitCountModeToString = boost::assign::map_list_of
	(VFAT2Settings::HitCountMode::FASTOR128 , "FASTOR128" )
	(VFAT2Settings::HitCountMode::COUNTS1   , "COUNTS1"   )
	(VFAT2Settings::HitCountMode::COUNTS2   , "COUNTS2"   )
	(VFAT2Settings::HitCountMode::COUNTS3   , "COUNTS3"   )
	(VFAT2Settings::HitCountMode::COUNTS4   , "COUNTS4"   )
	(VFAT2Settings::HitCountMode::COUNTS5   , "COUNTS5"   )
	(VFAT2Settings::HitCountMode::COUNTS6   , "COUNTS6"   )
	(VFAT2Settings::HitCountMode::COUNTS7   , "COUNTS7"   )
	(VFAT2Settings::HitCountMode::COUNTS8   , "COUNTS8"   )
	//need logic to handle these, as they should be mapped to COUNTS8
	(VFAT2Settings::HitCountMode::REDUNDANT0   , "COUNTS8"   )
	(VFAT2Settings::HitCountMode::REDUNDANT1   , "COUNTS8"   )
	(VFAT2Settings::HitCountMode::REDUNDANT2   , "COUNTS8"   )
	(VFAT2Settings::HitCountMode::REDUNDANT3   , "COUNTS8"   )
	(VFAT2Settings::HitCountMode::REDUNDANT4   , "COUNTS8"   )
	(VFAT2Settings::HitCountMode::REDUNDANT5   , "COUNTS8"   )
	(VFAT2Settings::HitCountMode::REDUNDANT6   , "COUNTS8"   );
	
      const enumMSPulseLenToString MSPulseLengthToString = boost::assign::map_list_of
	(VFAT2Settings::MSPulseLength::CLOCK1 , "CLOCK1" )
	(VFAT2Settings::MSPulseLength::CLOCK2 , "CLOCK2" )
	(VFAT2Settings::MSPulseLength::CLOCK3 , "CLOCK3" )
	(VFAT2Settings::MSPulseLength::CLOCK4 , "CLOCK4" )
	(VFAT2Settings::MSPulseLength::CLOCK5 , "CLOCK5" )
	(VFAT2Settings::MSPulseLength::CLOCK6 , "CLOCK6" )
	(VFAT2Settings::MSPulseLength::CLOCK7 , "CLOCK7" )
	(VFAT2Settings::MSPulseLength::CLOCK8 , "CLOCK8" );

      const enumReHitCTToString ReHitCTToString = boost::assign::map_list_of
	(VFAT2Settings::ReHitCT::CYCLE0 , "CYCLE0" )
	(VFAT2Settings::ReHitCT::CYCLE1 , "CYCLE1" )
	(VFAT2Settings::ReHitCT::CYCLE2 , "CYCLE2" )
	(VFAT2Settings::ReHitCT::CYCLE3 , "CYCLE3" );
	
      const enumDigInSelToString DigInSelToString = boost::assign::map_list_of
	(VFAT2Settings::DigInSel::ANALOG  , "ANALOG"  )
	(VFAT2Settings::DigInSel::DIGITAL , "DIGITAL" );

      const enumTrimDACRangeToString TrimDACRangeToString = boost::assign::map_list_of
	(VFAT2Settings::TrimDACRange::DEFAULT , "DEFAULT" )
	(VFAT2Settings::TrimDACRange::VALUE1  , "VALUE1"  )
	(VFAT2Settings::TrimDACRange::VALUE2  , "VALUE2"  )
	(VFAT2Settings::TrimDACRange::VALUE3  , "VALUE3"  )
	(VFAT2Settings::TrimDACRange::VALUE4  , "VALUE4"  )
	(VFAT2Settings::TrimDACRange::VALUE5  , "VALUE5"  )
	(VFAT2Settings::TrimDACRange::VALUE6  , "VALUE6"  )
	(VFAT2Settings::TrimDACRange::VALUE7  , "VALUE7"  );

      const enumPbBGToString PbBGToString = boost::assign::map_list_of
	(VFAT2Settings::PbBG::UNCONNECTED , "UNCONNECTED" )
	(VFAT2Settings::PbBG::CONNECTED   , "CONNECTED"   );

      const enumDFTestToString DFTestPatternToString = boost::assign::map_list_of
	(VFAT2Settings::DFTestPattern::IDLE , "IDLE" )
	(VFAT2Settings::DFTestPattern::SEND , "SEND" );
      
    }//end namespace gem::hw::vfat
    
  }//end namespace gem::hw
  
}//end namespace gem

#endif
