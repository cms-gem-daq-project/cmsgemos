/** @file GenericAMCSettingsEnums.h */

#ifndef GEM_HW_AMCSETTINGSENUMS_H
#define GEM_HW_AMCSETTINGSENUMS_H

namespace gem {
  namespace hw {

    // TODO: FIXME: replace these constructs with enum class/struct objects (only possible with c++11 or greater)
    class AMCLinkSettings
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

    class AMCSettings {
    public:

      struct TTCEncoding { //TTCEncoding settings
        enum ETTCEncoding { //TTCEncoding settings
          AMC13  = 0x0, //Use the AMC13 style TTC encoding
          GEMCSC = 0x1, //Use the CSC/GEM style TTC encoding
        } TTCEncoding;
      };

      struct TTCCommand {  ///< TTCCommand settings
        enum ETTCCommand {  ///< TTCCommand settings
          TTC_L1A,
          TTC_BC0,
          TTC_EC0,
          TTC_RESYNC,
          TTC_OC0,
          TTC_HARD_RESET,
          TTC_CALPULSE,
          TTC_START,
          TTC_STOP,
          TTC_TEST_SYNC,
        } TTCCommand;
      };

      struct OHLinkCount {  ///< OHLinkCount settings
        enum EOHLinkCount {  ///< OHLinkCount settings
          LINK_NOT_VALID,
          LINK_MISSED_COMMA,
          LINK_OVERFLOW,
          LINK_UNDERFLOW,
          LINK_SYNC_WORD
        } OHLinkCount;
      };
    };  // class AMCSettings
  }  // namespace gem::hw

  // <name>  is the enum scoped namespace for scope::VALUE access
  // <name>T is the enum type
  // typedef the struct for access to the members via struct::VALUE
  typedef gem::hw::AMCLinkSettings::LinkBitMasks  AMCLinkBitMasks;
  typedef gem::hw::AMCLinkSettings::LinkBitShifts AMCLinkBitShifts;

  typedef gem::hw::AMCSettings::TTCEncoding AMCTTCEncoding;
  typedef gem::hw::AMCSettings::TTCCommand  AMCTTCCommand;
  typedef gem::hw::AMCSettings::OHLinkCount AMCOHLinkCount;

  // typedef the enum for casting and access
  typedef gem::hw::AMCLinkSettings::LinkBitMasks::ELinkBitMasks   AMCLinkBitMasksT;
  typedef gem::hw::AMCLinkSettings::LinkBitShifts::ELinkBitShifts AMCLinkBitShiftsT;

  typedef gem::hw::AMCSettings::TTCEncoding::ETTCEncoding AMCTTCEncodingT;
  typedef gem::hw::AMCSettings::TTCCommand::ETTCCommand   AMCTTCCommandT;
  typedef gem::hw::AMCSettings::OHLinkCount::EOHLinkCount AMCOHLinkCountT;
}  // namespace gem

#endif  // GEM_HW_AMCSETTINGSENUMS_H
