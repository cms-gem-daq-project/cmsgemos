/** @file VFAT2Strings2Enums.h */

#ifndef GEM_HW_VFAT_VFAT2STRINGS2ENUMS_H
#define GEM_HW_VFAT_VFAT2STRINGS2ENUMS_H

#include "gem/hw/vfat/VFAT2SettingsEnums.h"

#include <unordered_map> // TODO: migrate to std::unordered_map if/when possible
#include <iostream>

#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>

namespace gem {
  namespace hw {
    namespace vfat {
      //VFAT2ChannelSettings
      typedef boost::unordered_map<std::string, gem::VFAT2ChannelBitMasksT > stringToChBitMask;
      typedef boost::unordered_map<std::string, gem::VFAT2ChannelBitShiftsT> stringToChBitShift;
      typedef boost::unordered_map<std::string, gem::VFAT2ContRegBitMasksT > stringToContRegBitMask;
      typedef boost::unordered_map<std::string, gem::VFAT2ContRegBitShiftsT> stringToContRegBitShift;
      typedef boost::unordered_map<std::string, gem::VFAT2RunModeT         > stringToRunMode;
      typedef boost::unordered_map<std::string, gem::VFAT2TrigModeT        > stringToTrigMode;
      typedef boost::unordered_map<std::string, gem::VFAT2CalModeT         > stringToCalibMode;
      typedef boost::unordered_map<std::string, gem::VFAT2MSPolT           > stringToMSPol;
      typedef boost::unordered_map<std::string, gem::VFAT2CalPolT          > stringToCalPol;
      typedef boost::unordered_map<std::string, gem::VFAT2ProbeModeT       > stringToProbeMode;
      typedef boost::unordered_map<std::string, gem::VFAT2LVDSModeT        > stringToLVDSMode;
      typedef boost::unordered_map<std::string, gem::VFAT2DACModeT         > stringToDACMode;
      typedef boost::unordered_map<std::string, gem::VFAT2HitCountModeT    > stringToHitCountMode;
      typedef boost::unordered_map<std::string, gem::VFAT2MSPulseLengthT   > stringToMSPulseLen;
      typedef boost::unordered_map<std::string, gem::VFAT2ReHitCTT         > stringToReHitCT;
      typedef boost::unordered_map<std::string, gem::VFAT2DigInSelT        > stringToDigInSel;
      typedef boost::unordered_map<std::string, gem::VFAT2TrimDACRangeT    > stringToTrimDACRange;
      typedef boost::unordered_map<std::string, gem::VFAT2PbBGT            > stringToPbBG;
      typedef boost::unordered_map<std::string, gem::VFAT2DFTestPatternT   > stringToDFTest;

      const stringToChBitMask StringToChBitMask = boost::assign::map_list_of
        ("TRIMDAC"  , gem::VFAT2ChannelBitMasks::TRIMDAC )
        ("ISMASKED" , gem::VFAT2ChannelBitMasks::ISMASKED)
        ("CHANCAL0" , gem::VFAT2ChannelBitMasks::CHANCAL0)
        ("CHANCAL"  , gem::VFAT2ChannelBitMasks::CHANCAL );

      const stringToChBitShift StringToChBitShift = boost::assign::map_list_of
        ("TRIMDAC"  , gem::VFAT2ChannelBitShifts::TRIMDAC )
        ("ISMASKED" , gem::VFAT2ChannelBitShifts::ISMASKED)
        ("CHANCAL0" , gem::VFAT2ChannelBitShifts::CHANCAL0)
        ("CHANCAL"  , gem::VFAT2ChannelBitShifts::CHANCAL );

      //VFAT2Settings
      const stringToContRegBitMask StringToContRegBitMask = boost::assign::map_list_of
        ("RUNMODE"      , gem::VFAT2ContRegBitMasks::RUNMODE       )
        ("TRIGMODE"     , gem::VFAT2ContRegBitMasks::TRIGMODE      )
        ("MSPOL"        , gem::VFAT2ContRegBitMasks::MSPOL         )
        ("CALPOL"       , gem::VFAT2ContRegBitMasks::CALPOL        )
        ("CALMODE"      , gem::VFAT2ContRegBitMasks::CALMODE       )

        ("DACMODE"      , gem::VFAT2ContRegBitMasks::DACMODE       )
        ("PROBEMODE"    , gem::VFAT2ContRegBitMasks::PROBEMODE     )
        ("LVDSMODE"     , gem::VFAT2ContRegBitMasks::LVDSMODE      )
        ("REHITCT"      , gem::VFAT2ContRegBitMasks::REHITCT       )

        ("HITCOUNTMODE" , gem::VFAT2ContRegBitMasks::HITCOUNTMODE  )
        ("MSPULSELENGTH", gem::VFAT2ContRegBitMasks::MSPULSELENGTH )
        ("DIGINSEL"     , gem::VFAT2ContRegBitMasks::DIGINSEL      )

        ("TRIMDACRANGE" , gem::VFAT2ContRegBitMasks::TRIMDACRANGE  )
        ("PADBANDGAP"   , gem::VFAT2ContRegBitMasks::PADBANDGAP    )
        ("DFTESTMODE"   , gem::VFAT2ContRegBitMasks::DFTESTMODE    )
        ("SPARE"        , gem::VFAT2ContRegBitMasks::SPARE         );

      const stringToContRegBitShift StringToContRegBitShift = boost::assign::map_list_of
        ("RUNMODE"       , gem::VFAT2ContRegBitShifts::RUNMODE       )
        ("TRIGMODE"      , gem::VFAT2ContRegBitShifts::TRIGMODE      )
        ("MSPOL"         , gem::VFAT2ContRegBitShifts::MSPOL         )
        ("CALPOL"        , gem::VFAT2ContRegBitShifts::CALPOL        )
        ("CALMODE"       , gem::VFAT2ContRegBitShifts::CALMODE       )

        ("DACMODE"       , gem::VFAT2ContRegBitShifts::DACMODE       )
        ("PROBEMODE"     , gem::VFAT2ContRegBitShifts::PROBEMODE     )
        ("LVDSMODE"      , gem::VFAT2ContRegBitShifts::LVDSMODE      )
        ("REHITCT"       , gem::VFAT2ContRegBitShifts::REHITCT       )

        ("HITCOUNTMODE"  , gem::VFAT2ContRegBitShifts::HITCOUNTMODE  )
        ("MSPULSELENGTH" , gem::VFAT2ContRegBitShifts::MSPULSELENGTH )
        ("DIGINSEL"      , gem::VFAT2ContRegBitShifts::DIGINSEL      )

        ("TRIMDACRANGE"  , gem::VFAT2ContRegBitShifts::TRIMDACRANGE  )
        ("PADBANDGAP"    , gem::VFAT2ContRegBitShifts::PADBANDGAP    )
        ("DFTESTMODE"    , gem::VFAT2ContRegBitShifts::DFTESTMODE    )
        ("SPARE"         , gem::VFAT2ContRegBitShifts::SPARE         );


      const stringToRunMode StringToRunMode = boost::assign::map_list_of
        ("SLEEP" , gem::VFAT2RunMode::SLEEP )
        ("RUN"   , gem::VFAT2RunMode::RUN   );

      const stringToTrigMode StringToTriggerMode = boost::assign::map_list_of
        ("NOTRIGGER" , gem::VFAT2TrigMode::NOTRIGGER )
        ("S1"        , gem::VFAT2TrigMode::ONEMODE   )//ONEMODE
        ("S1-S4"     , gem::VFAT2TrigMode::FOURMODE  )//FOURMODE
        ("S1-S8"     , gem::VFAT2TrigMode::EIGHTMODE )//EIGHTMODE
        ("GEMMODE"   , gem::VFAT2TrigMode::GEMMODE   );
      ////may not need these, as we will not set them explicitly
      //("SPARE0"   , gem::VFAT2TrigMode::SPARE0   )
      //("SPARE1"   , gem::VFAT2TrigMode::SPARE1   )
      //("SPARE2"   , gem::VFAT2TrigMode::SPARE2   );

      const stringToCalibMode StringToCalibrationMode = boost::assign::map_list_of
        ("NORMAL"   , gem::VFAT2CalMode::NORMAL   )
        ("VHI"      , gem::VFAT2CalMode::VCAL     )//VCAL
        ("VLOW"     , gem::VFAT2CalMode::BASELINE )//BASELINE
        ("EXTERNAL" , gem::VFAT2CalMode::EXTERNAL );

      const stringToMSPol StringToMSPolarity = boost::assign::map_list_of
        ("POSITIVE" , gem::VFAT2MSPol::POSITIVE )
        ("NEGATIVE" , gem::VFAT2MSPol::NEGATIVE );

      const stringToCalPol StringToCalPolarity = boost::assign::map_list_of
        ("POSITIVE" , gem::VFAT2CalPol::POSITIVE )
        ("NEGATIVE" , gem::VFAT2CalPol::NEGATIVE );

      const stringToProbeMode StringToProbeMode = boost::assign::map_list_of
        ("OFF" , gem::VFAT2ProbeMode::OFF )
        ("ON"  , gem::VFAT2ProbeMode::ON  );

      const stringToLVDSMode StringToLVDSMode = boost::assign::map_list_of
        ("OFF" , gem::VFAT2LVDSMode::ENABLEALL ) //ENABLEALL
        ("ON"  , gem::VFAT2LVDSMode::POWERSAVE );//POWERSAVE

      const stringToDACMode StringToDACMode = boost::assign::map_list_of
        ("OFF"         , gem::VFAT2DACMode::NORMAL      )//NORMAL
        ("IPREAMPIN"   , gem::VFAT2DACMode::IPREAMPIN   )
        ("IPREAMPFEED" , gem::VFAT2DACMode::IPREAMPFEED )
        ("IPREAMPOUT"  , gem::VFAT2DACMode::IPREAMPOUT  )
        ("ISHAPER"     , gem::VFAT2DACMode::ISHAPER     )
        ("ISHAPERFEED" , gem::VFAT2DACMode::ISHAPERFEED )
        ("ICOMP"       , gem::VFAT2DACMode::ICOMP       )
        ("VTHRESHOLD1" , gem::VFAT2DACMode::ITHRESHOLD1 )//ITHRESHOLD1
        ("VTHRESHOLD2" , gem::VFAT2DACMode::ITHRESHOLD2 )//ITHRESHOLD2
        ("VCAL"        , gem::VFAT2DACMode::VCAL        )
        ("CALOUT"      , gem::VFAT2DACMode::CALOUT      )
        ////may not need these, as we will never set them explicitly
        ("SPARE0"   , gem::VFAT2DACMode::SPARE0   )
        ("SPARE1"   , gem::VFAT2DACMode::SPARE1   )
        ("SPARE2"   , gem::VFAT2DACMode::SPARE2   )
        ("SPARE3"   , gem::VFAT2DACMode::SPARE3   )
        ("SPARE4"   , gem::VFAT2DACMode::SPARE4   );

      const stringToHitCountMode StringToHitCountMode = boost::assign::map_list_of
        ("FASTOR" , gem::VFAT2HitCountMode::FASTOR128 ) //FASTOR128
        ("S1"     , gem::VFAT2HitCountMode::COUNTS1   ) //COUNTS1
        ("S2"     , gem::VFAT2HitCountMode::COUNTS2   ) //COUNTS2
        ("S3"     , gem::VFAT2HitCountMode::COUNTS3   ) //COUNTS3
        ("S4"     , gem::VFAT2HitCountMode::COUNTS4   ) //COUNTS4
        ("S5"     , gem::VFAT2HitCountMode::COUNTS5   ) //COUNTS5
        ("S6"     , gem::VFAT2HitCountMode::COUNTS6   ) //COUNTS6
        ("S7"     , gem::VFAT2HitCountMode::COUNTS7   ) //COUNTS7
        ("S8"     , gem::VFAT2HitCountMode::COUNTS8   );//COUNTS8
      ////may not need these, as we will never set them explicitly
      //("REDUNDANT0"   , gem::VFAT2HitCountMode::REDUNDANT0   )
      //("REDUNDANT1"   , gem::VFAT2HitCountMode::REDUNDANT1   )
      //("REDUNDANT2"   , gem::VFAT2HitCountMode::REDUNDANT2   )
      //("REDUNDANT3"   , gem::VFAT2HitCountMode::REDUNDANT3   )
      //("REDUNDANT4"   , gem::VFAT2HitCountMode::REDUNDANT4   )
      //("REDUNDANT5"   , gem::VFAT2HitCountMode::REDUNDANT5   )
      //("REDUNDANT6"   , gem::VFAT2HitCountMode::REDUNDANT6   );

      const stringToMSPulseLen StringToMSPulseLength = boost::assign::map_list_of
        ("1" , gem::VFAT2MSPulseLength::CLOCK1 ) //CLOCK1
        ("2" , gem::VFAT2MSPulseLength::CLOCK2 ) //CLOCK2
        ("3" , gem::VFAT2MSPulseLength::CLOCK3 ) //CLOCK3
        ("4" , gem::VFAT2MSPulseLength::CLOCK4 ) //CLOCK4
        ("5" , gem::VFAT2MSPulseLength::CLOCK5 ) //CLOCK5
        ("6" , gem::VFAT2MSPulseLength::CLOCK6 ) //CLOCK6
        ("7" , gem::VFAT2MSPulseLength::CLOCK7 ) //CLOCK7
        ("8" , gem::VFAT2MSPulseLength::CLOCK8 );//CLOCK8

      const stringToReHitCT StringToReHitCT = boost::assign::map_list_of
        ("6.4MICROSEC" , gem::VFAT2ReHitCT::CYCLE0 ) //CYCLE0
        ("1.6MILLISEC" , gem::VFAT2ReHitCT::CYCLE1 ) //CYCLE1
        ("0.4SEC"      , gem::VFAT2ReHitCT::CYCLE2 ) //CYCLE2
        ("107SEC"      , gem::VFAT2ReHitCT::CYCLE3 );//CYCLE3

      const stringToDigInSel StringToDigInSel = boost::assign::map_list_of
        ("ANIP"  , gem::VFAT2DigInSel::ANALOG  ) //ANALOG
        ("DIGIP" , gem::VFAT2DigInSel::DIGITAL );//DIGITAL

      const stringToTrimDACRange StringToTrimDACRange = boost::assign::map_list_of
        ("0" , gem::VFAT2TrimDACRange::DEFAULT ) //DEFAULT
        ("1" , gem::VFAT2TrimDACRange::VALUE1  ) //VALUE1
        ("2" , gem::VFAT2TrimDACRange::VALUE2  ) //VALUE2
        ("3" , gem::VFAT2TrimDACRange::VALUE3  ) //VALUE3
        ("4" , gem::VFAT2TrimDACRange::VALUE4  ) //VALUE4
        ("5" , gem::VFAT2TrimDACRange::VALUE5  ) //VALUE5
        ("6" , gem::VFAT2TrimDACRange::VALUE6  ) //VALUE6
        ("7" , gem::VFAT2TrimDACRange::VALUE7  );//VALUE7

      const stringToPbBG StringToPbBG = boost::assign::map_list_of
        ("OFF" , gem::VFAT2PbBG::UNCONNECTED ) //UNCONNECTED
        ("ON"  , gem::VFAT2PbBG::CONNECTED   );//CONNECTED

      const stringToDFTest StringToDFTestPattern = boost::assign::map_list_of
        ("OFF" , gem::VFAT2DFTestPattern::IDLE ) //IDLE
        ("ON"  , gem::VFAT2DFTestPattern::SEND );//SEND

    }  // namespace gem::hw::vfat
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_VFAT_VFAT2STRINGS2ENUMS_H
