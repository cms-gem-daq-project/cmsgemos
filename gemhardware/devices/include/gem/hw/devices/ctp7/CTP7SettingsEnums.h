/** @file CTP7SettingsEnums.h */

#ifndef GEM_HW_CTP7_CTP7SETTINGSENUMS_H
#define GEM_HW_CTP7_CTP7SETTINGSENUMS_H

namespace gem {
  namespace hw {
    namespace ctp7 {
      class CTP7LinkSettings
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

      class CTP7Settings {
      public:

        //what are these for the CTP7??
        struct ContRegBitMasks {
          enum EContRegBitMasks {
            //Control Register 0
            RUNMODE       = 0x01,
            TRIGMODE      = 0x0e,
            MSPOL         = 0x10,
            CALPOL        = 0x20,
            CALMODE       = 0xc0,
          } ContRegBitMasks;
        };

        struct ContRegBitShifts {
          enum EContRegBitShifts {
            //Control Register 0
            RUNMODE       = 0,
            TRIGMODE      = 1,
            MSPOL         = 4,
            CALPOL        = 5,
            CALMODE       = 6,
          } ContRegBitShifts;
        };

        struct RunMode { //defines the run mode
          enum ERunMode { //defines the run mode
            SLEEP = 0x0, //sleep (default)
            RUN   = 0x1  //running
          } RunMode;
        };

        struct TriggerMode { //Trigger mode settings
          enum ETriggerMode { //Trigger mode settings
            CTP7 = 0x0, //Triggers are generated internally
            TTC  = 0x1, //Triggers come from the TTC link on the backplane
            BOTH = 0x2, //Use all sources of triggers
          } TriggerMode;
        };

        struct TTCEncoding { //TTCEncoding settings
          enum ETTCEncoding { //TTCEncoding settings
            AMC13  = 0x0, //Use the AMC13 style TTC encoding
            GEMCSC = 0x1, //Use the CSC/GEM style TTC encoding
          } TTCEncoding;
        };
      };  // class CTP7Settings
    }  // namespace gem::hw::ctp7
  }  // namespace gem::hw

  //typedef the struct for access to the members via struct::VALUE
  typedef gem::hw::ctp7::CTP7LinkSettings::LinkBitMasks   CTP7LinkBitMasks;
  typedef gem::hw::ctp7::CTP7LinkSettings::LinkBitShifts  CTP7LinkBitShifts;

  //typedef the enum for casting and access
  typedef gem::hw::ctp7::CTP7Settings::RunMode::ERunMode          CTP7RunMode;
  typedef gem::hw::ctp7::CTP7Settings::TriggerMode::ETriggerMode  CTP7TrigMode;
  typedef gem::hw::ctp7::CTP7Settings::TTCEncoding::ETTCEncoding  CTP7TTCEncoding;
}  // namespace gem

#endif  // GEM_HW_CTP7_CTP7SETTINGSENUMS_H
