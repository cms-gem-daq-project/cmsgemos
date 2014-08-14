#ifndef _gem_hw_vfat_VFAT2EnumStrings_h
#define _gem_hw_vfat_VFAT2EnumStrings_h

#include "gem/hw/vfat/VFAT2SettingsEnums.h"
#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>
#include <iostream>

namespace gem {
  namespace hw {
    namespace vfat {
      //VFAT2ChannelSettings
      typedef boost::unordered_map<VFAT2ChannelSettings::ChannelBitMasks::EChannelBitMasks, const char*> enumChBitMaskToString;
      typedef boost::unordered_map<VFAT2Settings::ContRegBitMasks::EContRegBitMasks       , const char*> enumContRegBitMaskToString;
      typedef boost::unordered_map<VFAT2Settings::ContRegBitShifts::EContRegBitShifts     , const char*> enumContRegBitShiftToString;
      typedef boost::unordered_map<VFAT2Settings::RunMode::ERunMode                       , const char*> enumRunModeToString;
      typedef boost::unordered_map<VFAT2Settings::TriggerMode::ETriggerMode               , const char*> enumTrigModeToString;
      typedef boost::unordered_map<VFAT2Settings::CalibrationMode::ECalibrationMode       , const char*> enumCalibModeToString;
      typedef boost::unordered_map<VFAT2Settings::MSPolarity::EMSPolarity                 , const char*> enumMSPolToString;
      typedef boost::unordered_map<VFAT2Settings::CalPolarity::ECalPolarity               , const char*> enumCalPolToString;
      typedef boost::unordered_map<VFAT2Settings::ProbeMode::EProbeMode                   , const char*> enumProbeModeToString;
      typedef boost::unordered_map<VFAT2Settings::LVDSPowerSave::ELVDSPowerSave           , const char*> enumLVDSPSToString;
      typedef boost::unordered_map<VFAT2Settings::DACMode::EDACMode                       , const char*> enumDACModeToString;
      typedef boost::unordered_map<VFAT2Settings::HitCountMode::EHitCountMode             , const char*> enumHitCountModeToString;
      typedef boost::unordered_map<VFAT2Settings::MSPulseLength::EMSPulseLength           , const char*> enumMSPulseLenToString;
      typedef boost::unordered_map<VFAT2Settings::ReHitCT::EReHitCT                       , const char*> enumReHitCTToString;
      typedef boost::unordered_map<VFAT2Settings::DigInSel::EDigInSel                     , const char*> enumDigInSelToString;
      typedef boost::unordered_map<VFAT2Settings::TrimDACRange::ETrimDACRange             , const char*> enumTrimDACRangeToString;
      typedef boost::unordered_map<VFAT2Settings::PbBG::EPbBG                             , const char*> enumPbBGToString;
      typedef boost::unordered_map<VFAT2Settings::DFTestPattern::EDFTestPattern           , const char*> enumDFTestToString;

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
					              	               
	(VFAT2Settings::ContRegBitMasks::DACSEL        , "DACSEL"        )
	(VFAT2Settings::ContRegBitMasks::PROBEMODE     , "PROBEMODE"     )
	(VFAT2Settings::ContRegBitMasks::LVDSMODE      , "LVDSMODE"      )
	(VFAT2Settings::ContRegBitMasks::REHITCT       , "REHITCT"       )
					              	               
	(VFAT2Settings::ContRegBitMasks::HITCOUNTSEL   , "HITCOUNTSEL"   )
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
					              	               
	(VFAT2Settings::ContRegBitShifts::DACSEL        , "DACSEL"        )
	(VFAT2Settings::ContRegBitShifts::PROBEMODE     , "PROBEMODE"     )
	(VFAT2Settings::ContRegBitShifts::LVDSMODE      , "LVDSMODE"      )
	(VFAT2Settings::ContRegBitShifts::REHITCT       , "REHITCT"       )
					              	               
	(VFAT2Settings::ContRegBitShifts::HITCOUNTSEL   , "HITCOUNTSEL"   )
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
	(VFAT2Settings::TriggerMode::GEMMODE   , "GEMMODE"   );
      
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
	(VFAT2Settings::DACMode::NORMAL    , "NORMAL"    )
	(VFAT2Settings::DACMode::IPREIN    , "IPREIN"    )
	(VFAT2Settings::DACMode::IPREFEED  , "IPREFEED"  )
	(VFAT2Settings::DACMode::IPREOUT   , "IPREOUT"   )
	(VFAT2Settings::DACMode::ISHAPER   , "ISHAPER"   )
	(VFAT2Settings::DACMode::ISHAPFEED , "ISHAPFEED" )
	(VFAT2Settings::DACMode::ICOMP     , "ICOMP"     )
	(VFAT2Settings::DACMode::ITHRESH1  , "ITHRESH1"  )
	(VFAT2Settings::DACMode::ITHRESH2  , "ITHRESH2"  )
	(VFAT2Settings::DACMode::VCAL      , "VCAL  "    )
	(VFAT2Settings::DACMode::CALOUT    , "CALOUT"    );

      const enumHitCountModeToString HitCountModeToString = boost::assign::map_list_of
	(VFAT2Settings::HitCountMode::FASTOR128 , "FASTOR128" )
	(VFAT2Settings::HitCountMode::COUNTS1   , "COUNTS1"   )
	(VFAT2Settings::HitCountMode::COUNTS2   , "COUNTS2"   )
	(VFAT2Settings::HitCountMode::COUNTS3   , "COUNTS3"   )
	(VFAT2Settings::HitCountMode::COUNTS4   , "COUNTS4"   )
	(VFAT2Settings::HitCountMode::COUNTS5   , "COUNTS5"   )
	(VFAT2Settings::HitCountMode::COUNTS6   , "COUNTS6"   )
	(VFAT2Settings::HitCountMode::COUNTS7   , "COUNTS7"   )
	(VFAT2Settings::HitCountMode::COUNTS8   , "COUNTS8"   );
	
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
