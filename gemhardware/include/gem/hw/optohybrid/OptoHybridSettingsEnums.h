/** @file OptoHybridSettingsEnums.h */

#ifndef GEM_HW_OPTOHYBRID_OPTOHYBRIDSETTINGSENUMS_H
#define GEM_HW_OPTOHYBRID_OPTOHYBRIDSETTINGSENUMS_H

namespace gem {
  namespace hw {
    namespace optohybrid {

      // TODO: FIXME: replace these constructs with enum class/struct objects (only possible with c++11 or greater)
      class OptoHybridLinkSettings
      {
      public:
        struct LinkBitMasks {
          enum ELinkBitMasks {
            ERROR  = 0x01,
            RECI2C = 0x02,
            SNTI2C = 0x04,
            RECREG = 0x08,
            SNTREG = 0x10,
          } LinkBitMasks;
        };

        struct LinkBitShifts {
          enum ELinkBitShifts {
            ERROR  = 0,
            RECI2C = 1,
            SNTI2C = 2,
            RECREG = 3,
            SNTREG = 4,
          } LinkBitShifts;
        };
      };

      class OptoHybridSettings {
      public:

        struct TTCMode { //TTC mode count settings
          enum ETTCMode { //TTC mode count settings
            GBT_TTC   = 0x0, // TTC signals coming from the GBT link
            LOCAL_TTC = 0x1, // TTC signals generated at the OH
            RESET     = 0x2, // RESET
          } TTCMode;
        };

        struct TriggerSource { // Trigger source settings
          enum ETriggerSource { // Trigger source settings
            GBT_TTC   = 0x0, // L1As coming from the GBT link
            LOCAL_TTC = 0x1, // L1As generated in the firmware
            ALL       = 0x2, // L1As from any source
          } TriggerSource;
        };

      };  // class OptoHybridSettings
    }  // namespace gem::hw::optohybrid
  }  // namespace gem::hw

  // <name>  is the enum scoped namespace for scope::VALUE access
  // <name>T is the enum type
  // typedef the struct for access to the members via struct::VALUE
  typedef gem::hw::optohybrid::OptoHybridLinkSettings::LinkBitMasks  OptoHybridLinkBitMasks;
  typedef gem::hw::optohybrid::OptoHybridLinkSettings::LinkBitShifts OptoHybridLinkBitShifts;

  typedef gem::hw::optohybrid::OptoHybridSettings::TTCMode       OptoHybridTTCMode;
  typedef gem::hw::optohybrid::OptoHybridSettings::TriggerSource OptoHybridTrigSrc;

  // typedef the enum for casting and access
  typedef gem::hw::optohybrid::OptoHybridLinkSettings::LinkBitMasks::ELinkBitMasks   OptoHybridLinkBitMasksT;
  typedef gem::hw::optohybrid::OptoHybridLinkSettings::LinkBitShifts::ELinkBitShifts OptoHybridLinkBitShiftsT;

  typedef gem::hw::optohybrid::OptoHybridSettings::TTCMode::ETTCMode             OptoHybridTTCModeT;
  typedef gem::hw::optohybrid::OptoHybridSettings::TriggerSource::ETriggerSource OptoHybridTrigSrcT;

}  // namespace gem

#endif  // GEM_HW_OPTOHYBRID_OPTOHYBRIDSETTINGSENUMS_H
