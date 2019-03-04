/** @file VFAT2Enums2Strings.h */

#ifndef GEM_HW_VFAT_VFAT2ENUMS2STRINGS_H
#define GEM_HW_VFAT_VFAT2ENUMS2STRINGS_H

#include "gem/hw/vfat/VFAT2SettingsEnums.h"

#include <unordered_map> // TODO: migrate to std::unordered_map if/when possible
#include <iostream>

#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>

namespace gem {
  namespace hw {
    namespace vfat {
      //VFAT2ChannelSettings
      typedef boost::unordered_map<gem::VFAT2ChannelBitMasksT,  std::string> enumChBitMaskToString;
      typedef boost::unordered_map<gem::VFAT2ChannelBitShiftsT, std::string> enumChBitShiftToString;
      typedef boost::unordered_map<gem::VFAT2ContRegBitMasksT,  std::string> enumContRegBitMaskToString;
      typedef boost::unordered_map<gem::VFAT2ContRegBitShiftsT, std::string> enumContRegBitShiftToString;
      typedef boost::unordered_map<gem::VFAT2RunModeT,          std::string> enumRunModeToString;
      typedef boost::unordered_map<gem::VFAT2TrigModeT,         std::string> enumTrigModeToString;
      typedef boost::unordered_map<gem::VFAT2CalModeT,          std::string> enumCalibModeToString;
      typedef boost::unordered_map<gem::VFAT2MSPolT,            std::string> enumMSPolToString;
      typedef boost::unordered_map<gem::VFAT2CalPolT,           std::string> enumCalPolToString;
      typedef boost::unordered_map<gem::VFAT2ProbeModeT,        std::string> enumProbeModeToString;
      typedef boost::unordered_map<gem::VFAT2LVDSModeT,         std::string> enumLVDSModeToString;
      typedef boost::unordered_map<gem::VFAT2DACModeT,          std::string> enumDACModeToString;
      typedef boost::unordered_map<gem::VFAT2HitCountModeT,     std::string> enumHitCountModeToString;
      typedef boost::unordered_map<gem::VFAT2MSPulseLengthT,    std::string> enumMSPulseLenToString;
      typedef boost::unordered_map<gem::VFAT2ReHitCTT,          std::string> enumReHitCTToString;
      typedef boost::unordered_map<gem::VFAT2DigInSelT,         std::string> enumDigInSelToString;
      typedef boost::unordered_map<gem::VFAT2TrimDACRangeT,     std::string> enumTrimDACRangeToString;
      typedef boost::unordered_map<gem::VFAT2PbBGT,             std::string> enumPbBGToString;
      typedef boost::unordered_map<gem::VFAT2DFTestPatternT,    std::string> enumDFTestToString;

      const enumChBitMaskToString ChBitMaskToString = boost::assign::map_list_of
        (gem::VFAT2ChannelBitMasks::TRIMDAC , "TRIMDAC" )
        (gem::VFAT2ChannelBitMasks::ISMASKED, "ISMASKED")
        (gem::VFAT2ChannelBitMasks::CHANCAL0, "CHANCAL0")
        (gem::VFAT2ChannelBitMasks::CHANCAL , "CHANCAL" );

      const enumChBitShiftToString ChBitShiftToString = boost::assign::map_list_of
        (gem::VFAT2ChannelBitShifts::TRIMDAC , "TRIMDAC" )
        (gem::VFAT2ChannelBitShifts::ISMASKED, "ISMASKED")
        (gem::VFAT2ChannelBitShifts::CHANCAL0, "CHANCAL0")
        (gem::VFAT2ChannelBitShifts::CHANCAL , "CHANCAL" );

      //VFAT2Settings
      const enumContRegBitMaskToString ContRegBitMaskToString = boost::assign::map_list_of
        (gem::VFAT2ContRegBitMasks::RUNMODE       , "RUNMODE"       )
        (gem::VFAT2ContRegBitMasks::TRIGMODE      , "TRIGMODE"      )
        (gem::VFAT2ContRegBitMasks::MSPOL         , "MSPOL"         )
        (gem::VFAT2ContRegBitMasks::CALPOL        , "CALPOL"        )
        (gem::VFAT2ContRegBitMasks::CALMODE       , "CALMODE"       )

        (gem::VFAT2ContRegBitMasks::DACMODE       , "DACMODE"       )
        (gem::VFAT2ContRegBitMasks::PROBEMODE     , "PROBEMODE"     )
        (gem::VFAT2ContRegBitMasks::LVDSMODE      , "LVDSMODE"      )
        (gem::VFAT2ContRegBitMasks::REHITCT       , "REHITCT"       )

        (gem::VFAT2ContRegBitMasks::HITCOUNTMODE  , "HITCOUNTMODE"  )
        (gem::VFAT2ContRegBitMasks::MSPULSELENGTH , "MSPULSELENGTH" )
        (gem::VFAT2ContRegBitMasks::DIGINSEL      , "DIGINSEL"      )

        (gem::VFAT2ContRegBitMasks::TRIMDACRANGE  , "TRIMDACRANGE"  )
        (gem::VFAT2ContRegBitMasks::PADBANDGAP    , "PADBANDGAP"    )
        (gem::VFAT2ContRegBitMasks::DFTESTMODE    , "DFTESTMODE"    )
        (gem::VFAT2ContRegBitMasks::SPARE         , "SPARE"         );

      const enumContRegBitShiftToString ContRegBitShiftToString = boost::assign::map_list_of
        (gem::VFAT2ContRegBitShifts::RUNMODE       , "RUNMODE"       )
        (gem::VFAT2ContRegBitShifts::TRIGMODE      , "TRIGMODE"      )
        (gem::VFAT2ContRegBitShifts::MSPOL         , "MSPOL"         )
        (gem::VFAT2ContRegBitShifts::CALPOL        , "CALPOL"        )
        (gem::VFAT2ContRegBitShifts::CALMODE       , "CALMODE"       )

        (gem::VFAT2ContRegBitShifts::DACMODE       , "DACMODE"       )
        (gem::VFAT2ContRegBitShifts::PROBEMODE     , "PROBEMODE"     )
        (gem::VFAT2ContRegBitShifts::LVDSMODE      , "LVDSMODE"      )
        (gem::VFAT2ContRegBitShifts::REHITCT       , "REHITCT"       )

        (gem::VFAT2ContRegBitShifts::HITCOUNTMODE  , "HITCOUNTMODE"  )
        (gem::VFAT2ContRegBitShifts::MSPULSELENGTH , "MSPULSELENGTH" )
        (gem::VFAT2ContRegBitShifts::DIGINSEL      , "DIGINSEL"      )

        (gem::VFAT2ContRegBitShifts::TRIMDACRANGE  , "TRIMDACRANGE"  )
        (gem::VFAT2ContRegBitShifts::PADBANDGAP    , "PADBANDGAP"    )
        (gem::VFAT2ContRegBitShifts::DFTESTMODE    , "DFTESTMODE"    )
        (gem::VFAT2ContRegBitShifts::SPARE         , "SPARE"         );


      const enumRunModeToString RunModeToString = boost::assign::map_list_of
        (gem::VFAT2RunMode::SLEEP , "SLEEP" )
        (gem::VFAT2RunMode::RUN   , "RUN"   );

      const enumTrigModeToString TriggerModeToString = boost::assign::map_list_of
        (gem::VFAT2TrigMode::NOTRIGGER , "NOTRIGGER" )
        (gem::VFAT2TrigMode::ONEMODE   , "S1"        )//ONEMODE
        (gem::VFAT2TrigMode::FOURMODE  , "S1-S4"     )//FOURMODE
        (gem::VFAT2TrigMode::EIGHTMODE , "S1-S8"     )//EIGHTMODE
        (gem::VFAT2TrigMode::GEMMODE   , "GEMMODE"   )
        //need logic to handle these, to remap to GEMMODE
        (gem::VFAT2TrigMode::SPARE0    , "GEMMODE"    )
        (gem::VFAT2TrigMode::SPARE1    , "GEMMODE"    )
        (gem::VFAT2TrigMode::SPARE2    , "GEMMODE"    );
      //(gem::VFAT2TrigMode::SPARE3    , "GEMMODE"    );

      const enumCalibModeToString CalibrationModeToString = boost::assign::map_list_of
        (gem::VFAT2CalMode::NORMAL   , "NORMAL"   )
        (gem::VFAT2CalMode::VCAL     , "VHI"      )//VCAL
        (gem::VFAT2CalMode::BASELINE , "VLOW"     )//BASELINE
        (gem::VFAT2CalMode::EXTERNAL , "EXTERNAL" );

      const enumMSPolToString MSPolarityToString = boost::assign::map_list_of
        (gem::VFAT2MSPol::POSITIVE , "POSITIVE" )
        (gem::VFAT2MSPol::NEGATIVE , "NEGATIVE" );

      const enumCalPolToString CalPolarityToString = boost::assign::map_list_of
        (gem::VFAT2CalPol::POSITIVE , "POSITIVE" )
        (gem::VFAT2CalPol::NEGATIVE , "NEGATIVE" );

      const enumProbeModeToString ProbeModeToString = boost::assign::map_list_of
        (gem::VFAT2ProbeMode::OFF , "OFF" )
        (gem::VFAT2ProbeMode::ON  , "ON"  );

      const enumLVDSModeToString LVDSModeToString = boost::assign::map_list_of
        (gem::VFAT2LVDSMode::ENABLEALL , "OFF" ) //ENABLEALL
        (gem::VFAT2LVDSMode::POWERSAVE , "ON"  );//POWERSAVE

      const enumDACModeToString DACModeToString = boost::assign::map_list_of
        (gem::VFAT2DACMode::NORMAL      , "OFF"         )//NORMAL
        (gem::VFAT2DACMode::IPREAMPIN   , "IPREAMPIN"   )
        (gem::VFAT2DACMode::IPREAMPFEED , "IPREAMPFEED" )
        (gem::VFAT2DACMode::IPREAMPOUT  , "IPREAMPOUT"  )
        (gem::VFAT2DACMode::ISHAPER     , "ISHAPER"     )
        (gem::VFAT2DACMode::ISHAPERFEED , "ISHAPERFEED" )
        (gem::VFAT2DACMode::ICOMP       , "ICOMP"       )
        (gem::VFAT2DACMode::ITHRESHOLD1 , "VTHRESHOLD1" )//ITHRESHOLD1
        (gem::VFAT2DACMode::ITHRESHOLD2 , "VTHRESHOLD2" )//ITHRESHOLD2
        (gem::VFAT2DACMode::VCAL        , "VCAL"        )
        (gem::VFAT2DACMode::CALOUT      , "CALOUT"      )
        //need logic to prevent these from being set until they are actually used
        (gem::VFAT2DACMode::SPARE0    , "SPARE0"    )
        (gem::VFAT2DACMode::SPARE1    , "SPARE1"    )
        (gem::VFAT2DACMode::SPARE2    , "SPARE2"    )
        (gem::VFAT2DACMode::SPARE3    , "SPARE3"    )
        (gem::VFAT2DACMode::SPARE4    , "SPARE4"    );

      const enumHitCountModeToString HitCountModeToString = boost::assign::map_list_of
        (gem::VFAT2HitCountMode::FASTOR128 , "FASTOR" )//FASTOR128
        (gem::VFAT2HitCountMode::COUNTS1   , "S1"     )//COUNTS1
        (gem::VFAT2HitCountMode::COUNTS2   , "S2"     )//COUNTS2
        (gem::VFAT2HitCountMode::COUNTS3   , "S3"     )//COUNTS3
        (gem::VFAT2HitCountMode::COUNTS4   , "S4"     )//COUNTS4
        (gem::VFAT2HitCountMode::COUNTS5   , "S5"     )//COUNTS5
        (gem::VFAT2HitCountMode::COUNTS6   , "S6"     )//COUNTS6
        (gem::VFAT2HitCountMode::COUNTS7   , "S7"     )//COUNTS7
        (gem::VFAT2HitCountMode::COUNTS8   , "S8"     )//COUNTS8
        //need logic to handle these, as they should be mapped to COUNTS8
        (gem::VFAT2HitCountMode::REDUNDANT0   , "S8"   ) //COUNTS8
        (gem::VFAT2HitCountMode::REDUNDANT1   , "S8"   ) //COUNTS8
        (gem::VFAT2HitCountMode::REDUNDANT2   , "S8"   ) //COUNTS8
        (gem::VFAT2HitCountMode::REDUNDANT3   , "S8"   ) //COUNTS8
        (gem::VFAT2HitCountMode::REDUNDANT4   , "S8"   ) //COUNTS8
        (gem::VFAT2HitCountMode::REDUNDANT5   , "S8"   ) //COUNTS8
        (gem::VFAT2HitCountMode::REDUNDANT6   , "S8"   );//COUNTS8

      const enumMSPulseLenToString MSPulseLengthToString = boost::assign::map_list_of
        (gem::VFAT2MSPulseLength::CLOCK1 , "1" ) //CLOCK1
        (gem::VFAT2MSPulseLength::CLOCK2 , "2" ) //CLOCK2
        (gem::VFAT2MSPulseLength::CLOCK3 , "3" ) //CLOCK3
        (gem::VFAT2MSPulseLength::CLOCK4 , "4" ) //CLOCK4
        (gem::VFAT2MSPulseLength::CLOCK5 , "5" ) //CLOCK5
        (gem::VFAT2MSPulseLength::CLOCK6 , "6" ) //CLOCK6
        (gem::VFAT2MSPulseLength::CLOCK7 , "7" ) //CLOCK7
        (gem::VFAT2MSPulseLength::CLOCK8 , "8" );//CLOCK8

      const enumReHitCTToString ReHitCTToString = boost::assign::map_list_of
        (gem::VFAT2ReHitCT::CYCLE0 , "6.4MICROSEC" ) //CYCLE0
        (gem::VFAT2ReHitCT::CYCLE1 , "1.6MILLISEC" ) //CYCLE1
        (gem::VFAT2ReHitCT::CYCLE2 , "0.4SEC"      ) //CYCLE2
        (gem::VFAT2ReHitCT::CYCLE3 , "107SEC"      );//CYCLE3

      const enumDigInSelToString DigInSelToString = boost::assign::map_list_of
        (gem::VFAT2DigInSel::ANALOG  , "ANIP"  ) //ANALOG
        (gem::VFAT2DigInSel::DIGITAL , "DIGIP" );//DIGITAL

      const enumTrimDACRangeToString TrimDACRangeToString = boost::assign::map_list_of
        (gem::VFAT2TrimDACRange::DEFAULT , "0" ) //DEFAULT
        (gem::VFAT2TrimDACRange::VALUE1  , "1"  ) //VALUE1
        (gem::VFAT2TrimDACRange::VALUE2  , "2"  ) //VALUE2
        (gem::VFAT2TrimDACRange::VALUE3  , "3"  ) //VALUE3
        (gem::VFAT2TrimDACRange::VALUE4  , "4"  ) //VALUE4
        (gem::VFAT2TrimDACRange::VALUE5  , "5"  ) //VALUE5
        (gem::VFAT2TrimDACRange::VALUE6  , "6"  ) //VALUE6
        (gem::VFAT2TrimDACRange::VALUE7  , "7"  );//VALUE7

      const enumPbBGToString PbBGToString = boost::assign::map_list_of
        (gem::VFAT2PbBG::UNCONNECTED , "OFF" ) //UNCONNECTED
        (gem::VFAT2PbBG::CONNECTED   , "ON"  );//CONNECTED

      const enumDFTestToString DFTestPatternToString = boost::assign::map_list_of
        (gem::VFAT2DFTestPattern::IDLE , "OFF" ) //IDLE
        (gem::VFAT2DFTestPattern::SEND , "ON"  );//SEND

    }  // namespace gem::hw::vfat
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_VFAT_VFAT2ENUMS2STRINGS_H
