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
      typedef boost::unordered_map<const char*, VFAT2ChannelSettings::ChannelBitMasks::EChannelBitMasks > stringToChBitMask;
      typedef boost::unordered_map<const char*, VFAT2Settings::ContRegBitMasks::EContRegBitMasks        > stringToContRegBitMask;
      typedef boost::unordered_map<const char*, VFAT2Settings::ContRegBitShifts::EContRegBitShifts      > stringToContRegBitShift;
      typedef boost::unordered_map<const char*, VFAT2Settings::RunMode::ERunMode                        > stringToRunMode;
      typedef boost::unordered_map<const char*, VFAT2Settings::TriggerMode::ETriggerMode                > stringToTrigMode;
      typedef boost::unordered_map<const char*, VFAT2Settings::CalibrationMode::ECalibrationMode        > stringToCalibMode;
      typedef boost::unordered_map<const char*, VFAT2Settings::MSPolarity::EMSPolarity                  > stringToMSPol;
      typedef boost::unordered_map<const char*, VFAT2Settings::CalPolarity::ECalPolarity                > stringToCalPol;
      typedef boost::unordered_map<const char*, VFAT2Settings::ProbeMode::EProbeMode                    > stringToProbeMode;
      typedef boost::unordered_map<const char*, VFAT2Settings::LVDSPowerSave::ELVDSPowerSave            > stringToLVDSPS;
      typedef boost::unordered_map<const char*, VFAT2Settings::DACMode::EDACMode                        > stringToDACMode;
      typedef boost::unordered_map<const char*, VFAT2Settings::HitCountMode::EHitCountMode              > stringToHitCountMode;
      typedef boost::unordered_map<const char*, VFAT2Settings::MSPulseLength::EMSPulseLength            > stringToMSPulseLen;
      typedef boost::unordered_map<const char*, VFAT2Settings::ReHitCT::EReHitCT                        > stringToReHitCT;
      typedef boost::unordered_map<const char*, VFAT2Settings::DigInSel::EDigInSel                      > stringToDigInSel;
      typedef boost::unordered_map<const char*, VFAT2Settings::TrimDACRange::ETrimDACRange              > stringToTrimDACRange;
      typedef boost::unordered_map<const char*, VFAT2Settings::PbBG::EPbBG                              > stringToPbBG;
      typedef boost::unordered_map<const char*, VFAT2Settings::DFTestPattern::EDFTestPattern            > stringToDFTest;

      const stringToChBitMask StringToChBitMask = boost::assign::map_list_of
	("TRIMDAC"  , VFAT2ChannelSettings::ChannelBitMasks::TRIMDAC )
	("ISMASKED" , VFAT2ChannelSettings::ChannelBitMasks::ISMASKED)
	("CHANCAL0" , VFAT2ChannelSettings::ChannelBitMasks::CHANCAL0)
	("CHANCAL"  , VFAT2ChannelSettings::ChannelBitMasks::CHANCAL );
      
      //VFAT2Settings 
      const stringToContRegBitMask StringToContRegBitMask = boost::assign::map_list_of
	("RUNMODE"      , VFAT2Settings::ContRegBitMasks::RUNMODE       )
	("TRIGMODE"     , VFAT2Settings::ContRegBitMasks::TRIGMODE      )
	("MSPOL"        , VFAT2Settings::ContRegBitMasks::MSPOL         )
	("CALPOL"       , VFAT2Settings::ContRegBitMasks::CALPOL        )
	("CALMODE"      , VFAT2Settings::ContRegBitMasks::CALMODE       )
	               	                                                
	("DACSEL"       , VFAT2Settings::ContRegBitMasks::DACSEL        )
	("PROBEMODE"    , VFAT2Settings::ContRegBitMasks::PROBEMODE     )
	("LVDSMODE"     , VFAT2Settings::ContRegBitMasks::LVDSMODE      )
	("REHITCT"      , VFAT2Settings::ContRegBitMasks::REHITCT       )
	               	                                                
	("HITCOUNTSEL"  , VFAT2Settings::ContRegBitMasks::HITCOUNTSEL   )
	("MSPULSELENGTH", VFAT2Settings::ContRegBitMasks::MSPULSELENGTH )
	("DIGINSEL"     , VFAT2Settings::ContRegBitMasks::DIGINSEL      )
			                                                
	("TRIMDACRANGE" , VFAT2Settings::ContRegBitMasks::TRIMDACRANGE  )
	("PADBANDGAP"   , VFAT2Settings::ContRegBitMasks::PADBANDGAP    )
	("DFTESTMODE"   , VFAT2Settings::ContRegBitMasks::DFTESTMODE    )
	("SPARE"        , VFAT2Settings::ContRegBitMasks::SPARE         );

      const stringToContRegBitShift StringToContRegBitShift = boost::assign::map_list_of
	("RUNMODE"       , VFAT2Settings::ContRegBitShifts::RUNMODE       )
	("TRIGMODE"      , VFAT2Settings::ContRegBitShifts::TRIGMODE      )
	("MSPOL"         , VFAT2Settings::ContRegBitShifts::MSPOL         )
	("CALPOL"        , VFAT2Settings::ContRegBitShifts::CALPOL        )
	("CALMODE"       , VFAT2Settings::ContRegBitShifts::CALMODE       )
	              	                                                  
	("DACSEL"        , VFAT2Settings::ContRegBitShifts::DACSEL        )
	("PROBEMODE"     , VFAT2Settings::ContRegBitShifts::PROBEMODE     )
	("LVDSMODE"      , VFAT2Settings::ContRegBitShifts::LVDSMODE      )
	("REHITCT"       , VFAT2Settings::ContRegBitShifts::REHITCT       )
	              	                                                  
	("HITCOUNTSEL"   , VFAT2Settings::ContRegBitShifts::HITCOUNTSEL   )
	("MSPULSELENGTH" , VFAT2Settings::ContRegBitShifts::MSPULSELENGTH )
	("DIGINSEL"      , VFAT2Settings::ContRegBitShifts::DIGINSEL      )
			                                                  
	("TRIMDACRANGE"  , VFAT2Settings::ContRegBitShifts::TRIMDACRANGE  )
	("PADBANDGAP"    , VFAT2Settings::ContRegBitShifts::PADBANDGAP    )
	("DFTESTMODE"    , VFAT2Settings::ContRegBitShifts::DFTESTMODE    )
	("SPARE"         , VFAT2Settings::ContRegBitShifts::SPARE         );


      const stringToRunMode StringToRunMode = boost::assign::map_list_of
	("SLEEP" , VFAT2Settings::RunMode::SLEEP )
	("RUN"   , VFAT2Settings::RunMode::RUN   );

      const stringToTrigMode StringToTriggerMode = boost::assign::map_list_of
	("NOTRIGGER" , VFAT2Settings::TriggerMode::NOTRIGGER )
	("ONEMODE"   , VFAT2Settings::TriggerMode::ONEMODE   )
	("FOURMODE"  , VFAT2Settings::TriggerMode::FOURMODE  )
	("EIGHTMODE" , VFAT2Settings::TriggerMode::EIGHTMODE )
	("GEMMODE"   , VFAT2Settings::TriggerMode::GEMMODE   );
      
      const stringToCalibMode StringToCalibrationMode = boost::assign::map_list_of
	("NORMAL"   , VFAT2Settings::CalibrationMode::NORMAL   )
	("VCAL"     , VFAT2Settings::CalibrationMode::VCAL     )
	("BASELINE" , VFAT2Settings::CalibrationMode::BASELINE )
	("EXTERNAL" , VFAT2Settings::CalibrationMode::EXTERNAL );

      const stringToMSPol StringToMSPolarity = boost::assign::map_list_of
	("POSITIVE" , VFAT2Settings::MSPolarity::POSITIVE )
	("NEGATIVE" , VFAT2Settings::MSPolarity::NEGATIVE );

      const stringToCalPol StringToCalPolarity = boost::assign::map_list_of
	("POSITIVE" , VFAT2Settings::CalPolarity::POSITIVE )
	("NEGATIVE" , VFAT2Settings::CalPolarity::NEGATIVE );

      const stringToProbeMode StringToProbeMode = boost::assign::map_list_of
	("OFF" , VFAT2Settings::ProbeMode::OFF )
	("ON"  , VFAT2Settings::ProbeMode::ON  );

      const stringToLVDSPS StringToLVDSPowerSave = boost::assign::map_list_of
	("ENABLEALL" , VFAT2Settings::LVDSPowerSave::ENABLEALL )
	("POWERSAVE" , VFAT2Settings::LVDSPowerSave::POWERSAVE );

      const stringToDACMode StringToDACMode = boost::assign::map_list_of
	("NORMAL"    , VFAT2Settings::DACMode::NORMAL    )
	("IPREIN"    , VFAT2Settings::DACMode::IPREIN    )
	("IPREFEED"  , VFAT2Settings::DACMode::IPREFEED  )
	("IPREOUT"   , VFAT2Settings::DACMode::IPREOUT   )
	("ISHAPER"   , VFAT2Settings::DACMode::ISHAPER   )
	("ISHAPFEED" , VFAT2Settings::DACMode::ISHAPFEED )
	("ICOMP"     , VFAT2Settings::DACMode::ICOMP     )
	("ITHRESH1"  , VFAT2Settings::DACMode::ITHRESH1  )
	("ITHRESH2"  , VFAT2Settings::DACMode::ITHRESH2  )
	("VCAL"      , VFAT2Settings::DACMode::VCAL      )
	("CALOUT"    , VFAT2Settings::DACMode::CALOUT    );

      const stringToHitCountMode StringToHitCountMode = boost::assign::map_list_of
	("FASTOR128" , VFAT2Settings::HitCountMode::FASTOR128 )
	("COUNTS1"   , VFAT2Settings::HitCountMode::COUNTS1   )
	("COUNTS2"   , VFAT2Settings::HitCountMode::COUNTS2   )
	("COUNTS3"   , VFAT2Settings::HitCountMode::COUNTS3   )
	("COUNTS4"   , VFAT2Settings::HitCountMode::COUNTS4   )
	("COUNTS5"   , VFAT2Settings::HitCountMode::COUNTS5   )
	("COUNTS6"   , VFAT2Settings::HitCountMode::COUNTS6   )
	("COUNTS7"   , VFAT2Settings::HitCountMode::COUNTS7   )
	("COUNTS8"   , VFAT2Settings::HitCountMode::COUNTS8   );
	
      const stringToMSPulseLen StringToMSPulseLength = boost::assign::map_list_of
	("CLOCK1" , VFAT2Settings::MSPulseLength::CLOCK1 )
	("CLOCK2" , VFAT2Settings::MSPulseLength::CLOCK2 )
	("CLOCK3" , VFAT2Settings::MSPulseLength::CLOCK3 )
	("CLOCK4" , VFAT2Settings::MSPulseLength::CLOCK4 )
	("CLOCK5" , VFAT2Settings::MSPulseLength::CLOCK5 )
	("CLOCK6" , VFAT2Settings::MSPulseLength::CLOCK6 )
	("CLOCK7" , VFAT2Settings::MSPulseLength::CLOCK7 )
	("CLOCK8" , VFAT2Settings::MSPulseLength::CLOCK8 );

      const stringToReHitCT StringToReHitCT = boost::assign::map_list_of
	("CYCLE0" , VFAT2Settings::ReHitCT::CYCLE0 )
	("CYCLE1" , VFAT2Settings::ReHitCT::CYCLE1 )
	("CYCLE2" , VFAT2Settings::ReHitCT::CYCLE2 )
	("CYCLE3" , VFAT2Settings::ReHitCT::CYCLE3 );
	
      const stringToDigInSel StringToDigInSel = boost::assign::map_list_of
	("ANALOG"  , VFAT2Settings::DigInSel::ANALOG  )
	("DIGITAL" , VFAT2Settings::DigInSel::DIGITAL );

      const stringToTrimDACRange StringToTrimDACRange = boost::assign::map_list_of
	("DEFAULT" , VFAT2Settings::TrimDACRange::DEFAULT )
	("VALUE1"  , VFAT2Settings::TrimDACRange::VALUE1  )
	("VALUE2"  , VFAT2Settings::TrimDACRange::VALUE2  )
	("VALUE3"  , VFAT2Settings::TrimDACRange::VALUE3  )
	("VALUE4"  , VFAT2Settings::TrimDACRange::VALUE4  )
	("VALUE5"  , VFAT2Settings::TrimDACRange::VALUE5  )
	("VALUE6"  , VFAT2Settings::TrimDACRange::VALUE6  )
	("VALUE7"  , VFAT2Settings::TrimDACRange::VALUE7  );

      const stringToPbBG StringToPbBG = boost::assign::map_list_of
	("UNCONNECTED" , VFAT2Settings::PbBG::UNCONNECTED )
	("CONNECTED"   , VFAT2Settings::PbBG::CONNECTED   );

      const stringToDFTest StringToDFTestPattern = boost::assign::map_list_of
	("IDLE" , VFAT2Settings::DFTestPattern::IDLE )
	("SEND" , VFAT2Settings::DFTestPattern::SEND );
      
    }//end namespace gem::hw::vfat
    
  }//end namespace gem::hw
  
}//end namespace gem

#endif
