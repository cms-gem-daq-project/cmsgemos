
#include "gempython/cmsgemos_enums.h"

// gemhardware enum includes
#include "gem/hw/devices/amc/GenericAMCSettingsEnums.h"
#include "gem/hw/devices/optohybrid/OptoHybridSettingsEnums.h"
#include "gem/hw/devices/amc13/AMC13SettingsEnums.h"
#include "gem/hw/devices/glib/GLIBSettingsEnums.h"
// #include "gem/hw/devices/ctp7/CTP7SettingsEnums.h"
// #include "gem/hw/devices/vfat/VFAT2SettingsEnums.h"
// #include "gem/hw/devices/vfat/VFAT3SettingsEnums.h"

namespace bpy = boost::python;

void gempython::wrap_enums()
{
  //TODO: which enums to expose to python?

  // from GenericAMCSettingsEnums.h
  bpy::enum_<gem::AMCLinkBitMasks::ELinkBitMasks>("AMCLinkBitMasks")
    .value("MASK_ERROR",  gem::AMCLinkBitMasks::ERROR )
    .value("MASK_RECI2C", gem::AMCLinkBitMasks::RECI2C)
    .value("MASK_SNTI2C", gem::AMCLinkBitMasks::SNTI2C)
    .value("MASK_RECREG", gem::AMCLinkBitMasks::RECREG)
    .value("MASK_SNTREG", gem::AMCLinkBitMasks::SNTREG)
    ;
  bpy::enum_<gem::AMCLinkBitShifts::ELinkBitShifts>("AMCLinkBitShifts")
    .value("SHIFT_ERROR",  gem::AMCLinkBitShifts::ERROR )
    .value("SHIFT_RECI2C", gem::AMCLinkBitShifts::RECI2C)
    .value("SHIFT_SNTI2C", gem::AMCLinkBitShifts::SNTI2C)
    .value("SHIFT_RECREG", gem::AMCLinkBitShifts::RECREG)
    .value("SHIFT_SNTREG", gem::AMCLinkBitShifts::SNTREG)
    ;
  bpy::enum_<gem::AMCTTCEncoding::ETTCEncoding>("AMCTTCEncoding")
    .value("AMC13",  gem::AMCTTCEncoding::AMC13)
    .value("GEMCSC", gem::AMCTTCEncoding::GEMCSC)
    ;
  bpy::enum_<gem::AMCTTCCommand::ETTCCommand>("AMCTTCCommand")
    .value("TTC_L1A",        gem::AMCTTCCommand::TTC_L1A)
    .value("TTC_BC0",        gem::AMCTTCCommand::TTC_BC0)
    .value("TTC_EC0",        gem::AMCTTCCommand::TTC_EC0)
    .value("TTC_RESYNC",     gem::AMCTTCCommand::TTC_RESYNC)
    .value("TTC_OC0",        gem::AMCTTCCommand::TTC_OC0)
    .value("TTC_HARD_RESET", gem::AMCTTCCommand::TTC_HARD_RESET)
    .value("TTC_CALPULSE",   gem::AMCTTCCommand::TTC_CALPULSE)
    .value("TTC_START",      gem::AMCTTCCommand::TTC_START)
    .value("TTC_STOP",       gem::AMCTTCCommand::TTC_STOP)
    .value("TTC_TEST_SYNC",  gem::AMCTTCCommand::TTC_TEST_SYNC)
    ;
  bpy::enum_<gem::AMCOHLinkCount::EOHLinkCount>("AMCOHLinkCount")
    .value("LINK_NOT_VALID",    gem::AMCOHLinkCount::LINK_NOT_VALID)
    .value("LINK_MISSED_COMMA", gem::AMCOHLinkCount::LINK_MISSED_COMMA)
    .value("LINK_OVERFLOW",     gem::AMCOHLinkCount::LINK_OVERFLOW)
    .value("LINK_UNDERFLOW",    gem::AMCOHLinkCount::LINK_UNDERFLOW)
    .value("LINK_SYNC_WORD",    gem::AMCOHLinkCount::LINK_SYNC_WORD)
    ;

  // // from OptoHybridSettingsEnums.h
  // bpy::enum_<gem::optohybrid>("");

  // // from VFAT2SettingsEnums.h
  // bpy::enum_<gem::vfat>("");
  // // from VFAT3SettingsEnums.h
  // bpy::enum_<gem::vfat>("");

  // from AMC13SettingsEnums.h
  bpy::enum_<gem::AMC13BGOCmd::EBGOCmd>("AMC13BGOCmd")
    .value("BC0",       gem::AMC13BGOCmd::BC0)
    .value("EC0",       gem::AMC13BGOCmd::EC0)
    .value("OC0",       gem::AMC13BGOCmd::OC0)
    .value("CalPulse",  gem::AMC13BGOCmd::CalPulse)
    .value("Resync",    gem::AMC13BGOCmd::Resync)
    .value("HardReset", gem::AMC13BGOCmd::HardReset)
    ;
  bpy::enum_<gem::GEMBGOCmd::EBGOCmd>("GEMBGOCmd")
    .value("BC0",       gem::GEMBGOCmd::BC0)
    .value("EC0",       gem::GEMBGOCmd::EC0)
    .value("OC0",       gem::GEMBGOCmd::OC0)
    .value("CalPulse",  gem::GEMBGOCmd::CalPulse)
    .value("Resync",    gem::GEMBGOCmd::Resync)
    .value("HardReset", gem::GEMBGOCmd::HardReset)
    ;
}
