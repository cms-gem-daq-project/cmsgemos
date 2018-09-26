/** @file OptoHybridSettingsEnums.h */

#ifndef GEM_HW_OPTOHYBRID_OPTOHYBRIDSETTINGSENUMS_H
#define GEM_HW_OPTOHYBRID_OPTOHYBRIDSETTINGSENUMS_H

namespace gem {
  namespace hw {
    namespace optohybrid {
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
            GTX_TTC  = 0x0, // TTC signals coming from the 8b10b link
            INTERNAL = 0x1, // TTC signals generated in the firmware
            EXTERNAL = 0x2, // TTC signals coming external input (HDMI board/LEMO)
            LOOPBACK = 0x3, // TTC signals coming from the VFATs (only s-bits)
            SENT     = 0x4, // TTC signals sent along the GEB
            GBT_TTC  = 0x5, // TTC signals coming from the GBT link
            RESET    = 0x6, // RESET
          } TTCMode;
        };

        struct TriggerSource { // Trigger source settings
          enum ETriggerSource { // Trigger source settings
            GTX_TTC  = 0x0, // L1As coming from the 8b10b link
            INTERNAL = 0x1, // L1As generated in the firmware
            EXTERNAL = 0x2, // L1As coming external input (HDMI board/LEMO)
            LOOPBACK = 0x3, // L1As coming from the VFATs (only s-bits)
            ALL      = 0x4, // OR of all sources of L1As
            GBT_TTC  = 0x5, // L1As coming from the GBT link
          } TriggerSource;
        };

      };  // class OptoHybridSettings
    }  // namespace gem::hw::optohybrid
  }  // namespace gem::hw

  // typedef the struct for access to the members via struct::VALUE
  typedef gem::hw::optohybrid::OptoHybridLinkSettings::LinkBitMasks   OptoHybridLinkBitMasks;
  typedef gem::hw::optohybrid::OptoHybridLinkSettings::LinkBitShifts  OptoHybridLinkBitShifts;

  // typedef the enum for casting and access
  typedef gem::hw::optohybrid::OptoHybridSettings::TTCMode::ETTCMode             OptoHybridTTCMode;
  typedef gem::hw::optohybrid::OptoHybridSettings::TriggerSource::ETriggerSource OptoHybridTrigSrc;

}  // namespace gem

#endif  // GEM_HW_OPTOHYBRID_OPTOHYBRIDSETTINGSENUMS_H
