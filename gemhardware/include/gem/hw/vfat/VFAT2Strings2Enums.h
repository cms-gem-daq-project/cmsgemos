#ifndef GEM_HW_VFAT_VFAT2STRINGS2ENUMS_H
#define GEM_HW_VFAT_VFAT2STRINGS2ENUMS_H

#include "gem/hw/vfat/VFAT2SettingsEnums.h"
#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>
#include <iostream>

namespace gem {
  namespace hw {
    namespace vfat {
      //VFAT2ChannelSettings
      typedef boost::unordered_map<std::string, VFAT2ChannelSettings::ChannelBitMasks::EChannelBitMasks > stringToChBitMask;
      typedef boost::unordered_map<std::string, VFAT2Settings::ContRegBitMasks::EContRegBitMasks        > stringToContRegBitMask;
      typedef boost::unordered_map<std::string, VFAT2Settings::ContRegBitShifts::EContRegBitShifts      > stringToContRegBitShift;
      typedef boost::unordered_map<std::string, VFAT2Settings::RunMode::ERunMode                        > stringToRunMode;
      typedef boost::unordered_map<std::string, VFAT2Settings::TriggerMode::ETriggerMode                > stringToTrigMode;
      typedef boost::unordered_map<std::string, VFAT2Settings::CalibrationMode::ECalibrationMode        > stringToCalibMode;
      typedef boost::unordered_map<std::string, VFAT2Settings::MSPolarity::EMSPolarity                  > stringToMSPol;
      typedef boost::unordered_map<std::string, VFAT2Settings::CalPolarity::ECalPolarity                > stringToCalPol;
      typedef boost::unordered_map<std::string, VFAT2Settings::ProbeMode::EProbeMode                    > stringToProbeMode;
      typedef boost::unordered_map<std::string, VFAT2Settings::LVDSPowerSave::ELVDSPowerSave            > stringToLVDSPS;
      typedef boost::unordered_map<std::string, VFAT2Settings::DACMode::EDACMode                        > stringToDACMode;
      typedef boost::unordered_map<std::string, VFAT2Settings::HitCountMode::EHitCountMode              > stringToHitCountMode;
      typedef boost::unordered_map<std::string, VFAT2Settings::MSPulseLength::EMSPulseLength            > stringToMSPulseLen;
      typedef boost::unordered_map<std::string, VFAT2Settings::ReHitCT::EReHitCT                        > stringToReHitCT;
      typedef boost::unordered_map<std::string, VFAT2Settings::DigInSel::EDigInSel                      > stringToDigInSel;
      typedef boost::unordered_map<std::string, VFAT2Settings::TrimDACRange::ETrimDACRange              > stringToTrimDACRange;
      typedef boost::unordered_map<std::string, VFAT2Settings::PbBG::EPbBG                              > stringToPbBG;
      typedef boost::unordered_map<std::string, VFAT2Settings::DFTestPattern::EDFTestPattern            > stringToDFTest;

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

        ("DACMODE"      , VFAT2Settings::ContRegBitMasks::DACMODE       )
        ("PROBEMODE"    , VFAT2Settings::ContRegBitMasks::PROBEMODE     )
        ("LVDSMODE"     , VFAT2Settings::ContRegBitMasks::LVDSMODE      )
        ("REHITCT"      , VFAT2Settings::ContRegBitMasks::REHITCT       )

        ("HITCOUNTMODE" , VFAT2Settings::ContRegBitMasks::HITCOUNTMODE  )
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

        ("DACMODE"       , VFAT2Settings::ContRegBitShifts::DACMODE       )
        ("PROBEMODE"     , VFAT2Settings::ContRegBitShifts::PROBEMODE     )
        ("LVDSMODE"      , VFAT2Settings::ContRegBitShifts::LVDSMODE      )
        ("REHITCT"       , VFAT2Settings::ContRegBitShifts::REHITCT       )

        ("HITCOUNTMODE"  , VFAT2Settings::ContRegBitShifts::HITCOUNTMODE  )
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
        ("S1"        , VFAT2Settings::TriggerMode::ONEMODE   )//ONEMODE
        ("S1-S4"     , VFAT2Settings::TriggerMode::FOURMODE  )//FOURMODE
        ("S1-S8"     , VFAT2Settings::TriggerMode::EIGHTMODE )//EIGHTMODE
        ("GEMMODE"   , VFAT2Settings::TriggerMode::GEMMODE   );
      ////may not need these, as we will not set them explicitly
      //("SPARE0"   , VFAT2Settings::TriggerMode::SPARE0   )
      //("SPARE1"   , VFAT2Settings::TriggerMode::SPARE1   )
      //("SPARE2"   , VFAT2Settings::TriggerMode::SPARE2   );

      const stringToCalibMode StringToCalibrationMode = boost::assign::map_list_of
        ("NORMAL"   , VFAT2Settings::CalibrationMode::NORMAL   )
        ("VHI"      , VFAT2Settings::CalibrationMode::VCAL     )//VCAL
        ("VLOW"     , VFAT2Settings::CalibrationMode::BASELINE )//BASELINE
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
        ("OFF" , VFAT2Settings::LVDSPowerSave::ENABLEALL ) //ENABLEALL
        ("ON"  , VFAT2Settings::LVDSPowerSave::POWERSAVE );//POWERSAVE

      const stringToDACMode StringToDACMode = boost::assign::map_list_of
        ("OFF"         , VFAT2Settings::DACMode::NORMAL      )//NORMAL
        ("IPREAMPIN"   , VFAT2Settings::DACMode::IPREAMPIN   )
        ("IPREAMPFEED" , VFAT2Settings::DACMode::IPREAMPFEED )
        ("IPREAMPOUT"  , VFAT2Settings::DACMode::IPREAMPOUT  )
        ("ISHAPER"     , VFAT2Settings::DACMode::ISHAPER     )
        ("ISHAPERFEED" , VFAT2Settings::DACMode::ISHAPERFEED )
        ("ICOMP"       , VFAT2Settings::DACMode::ICOMP       )
        ("VTHRESHOLD1" , VFAT2Settings::DACMode::ITHRESHOLD1 )//ITHRESHOLD1
        ("VTHRESHOLD2" , VFAT2Settings::DACMode::ITHRESHOLD2 )//ITHRESHOLD2
        ("VCAL"        , VFAT2Settings::DACMode::VCAL        )
        ("CALOUT"      , VFAT2Settings::DACMode::CALOUT      )
        ////may not need these, as we will never set them explicitly
        ("SPARE0"   , VFAT2Settings::DACMode::SPARE0   )
        ("SPARE1"   , VFAT2Settings::DACMode::SPARE1   )
        ("SPARE2"   , VFAT2Settings::DACMode::SPARE2   )
        ("SPARE3"   , VFAT2Settings::DACMode::SPARE3   )
        ("SPARE4"   , VFAT2Settings::DACMode::SPARE4   );

      const stringToHitCountMode StringToHitCountMode = boost::assign::map_list_of
        ("FASTOR" , VFAT2Settings::HitCountMode::FASTOR128 ) //FASTOR128
        ("S1"     , VFAT2Settings::HitCountMode::COUNTS1   ) //COUNTS1
        ("S2"     , VFAT2Settings::HitCountMode::COUNTS2   ) //COUNTS2
        ("S3"     , VFAT2Settings::HitCountMode::COUNTS3   ) //COUNTS3
        ("S4"     , VFAT2Settings::HitCountMode::COUNTS4   ) //COUNTS4
        ("S5"     , VFAT2Settings::HitCountMode::COUNTS5   ) //COUNTS5
        ("S6"     , VFAT2Settings::HitCountMode::COUNTS6   ) //COUNTS6
        ("S7"     , VFAT2Settings::HitCountMode::COUNTS7   ) //COUNTS7
        ("S8"     , VFAT2Settings::HitCountMode::COUNTS8   );//COUNTS8
      ////may not need these, as we will never set them explicitly
      //("REDUNDANT0"   , VFAT2Settings::HitCountMode::REDUNDANT0   )
      //("REDUNDANT1"   , VFAT2Settings::HitCountMode::REDUNDANT1   )
      //("REDUNDANT2"   , VFAT2Settings::HitCountMode::REDUNDANT2   )
      //("REDUNDANT3"   , VFAT2Settings::HitCountMode::REDUNDANT3   )
      //("REDUNDANT4"   , VFAT2Settings::HitCountMode::REDUNDANT4   )
      //("REDUNDANT5"   , VFAT2Settings::HitCountMode::REDUNDANT5   )
      //("REDUNDANT6"   , VFAT2Settings::HitCountMode::REDUNDANT6   );

      const stringToMSPulseLen StringToMSPulseLength = boost::assign::map_list_of
        ("1" , VFAT2Settings::MSPulseLength::CLOCK1 ) //CLOCK1
        ("2" , VFAT2Settings::MSPulseLength::CLOCK2 ) //CLOCK2
        ("3" , VFAT2Settings::MSPulseLength::CLOCK3 ) //CLOCK3
        ("4" , VFAT2Settings::MSPulseLength::CLOCK4 ) //CLOCK4
        ("5" , VFAT2Settings::MSPulseLength::CLOCK5 ) //CLOCK5
        ("6" , VFAT2Settings::MSPulseLength::CLOCK6 ) //CLOCK6
        ("7" , VFAT2Settings::MSPulseLength::CLOCK7 ) //CLOCK7
        ("8" , VFAT2Settings::MSPulseLength::CLOCK8 );//CLOCK8

      const stringToReHitCT StringToReHitCT = boost::assign::map_list_of
        ("6.4MICROSEC" , VFAT2Settings::ReHitCT::CYCLE0 ) //CYCLE0
        ("1.6MILLISEC" , VFAT2Settings::ReHitCT::CYCLE1 ) //CYCLE1
        ("0.4SEC"      , VFAT2Settings::ReHitCT::CYCLE2 ) //CYCLE2
        ("107SEC"      , VFAT2Settings::ReHitCT::CYCLE3 );//CYCLE3

      const stringToDigInSel StringToDigInSel = boost::assign::map_list_of
        ("ANIP"  , VFAT2Settings::DigInSel::ANALOG  ) //ANALOG
        ("DIGIP" , VFAT2Settings::DigInSel::DIGITAL );//DIGITAL

      const stringToTrimDACRange StringToTrimDACRange = boost::assign::map_list_of
        ("0" , VFAT2Settings::TrimDACRange::DEFAULT ) //DEFAULT
        ("1" , VFAT2Settings::TrimDACRange::VALUE1  ) //VALUE1
        ("2" , VFAT2Settings::TrimDACRange::VALUE2  ) //VALUE2
        ("3" , VFAT2Settings::TrimDACRange::VALUE3  ) //VALUE3
        ("4" , VFAT2Settings::TrimDACRange::VALUE4  ) //VALUE4
        ("5" , VFAT2Settings::TrimDACRange::VALUE5  ) //VALUE5
        ("6" , VFAT2Settings::TrimDACRange::VALUE6  ) //VALUE6
        ("7" , VFAT2Settings::TrimDACRange::VALUE7  );//VALUE7

      const stringToPbBG StringToPbBG = boost::assign::map_list_of
        ("OFF" , VFAT2Settings::PbBG::UNCONNECTED ) //UNCONNECTED
        ("ON"  , VFAT2Settings::PbBG::CONNECTED   );//CONNECTED

      const stringToDFTest StringToDFTestPattern = boost::assign::map_list_of
        ("OFF" , VFAT2Settings::DFTestPattern::IDLE ) //IDLE
        ("ON"  , VFAT2Settings::DFTestPattern::SEND );//SEND

    }  // namespace gem::hw::vfat
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_VFAT_VFAT2STRINGS2ENUMS_H
