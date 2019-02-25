/** @file GEMCrateUtils.h */

#ifndef GEM_HW_UTILS_GEMCRATEUTILS_H
#define GEM_HW_UTILS_GEMCRATEUTILS_H

#include <sstream>
#include <string>
#include <iomanip>

#include <gem/utils/exception/Exception.h>

#include "gem/utils/GEMLogging.h"
/* namespace log4cplus { */
/*   class Logger; */
/* } */

namespace gem {
  namespace hw {
    namespace utils {

      static log4cplus::Logger m_gemLogger;

      // FIXME, THIS SHOULD NOT BE HARDCODED, move out of class?
      static constexpr uint8_t  N_GTX                = 12;          ///< maximum number of GTX links on the GenericAMC
      static constexpr uint8_t  MAX_VFATS            = 24;          ///< maximum number of VFATs that can be connected
      static constexpr uint32_t ALL_VFATS_BCAST_MASK = 0xff000000;  ///< send broadcast I2C requests to all chips
      static constexpr uint32_t ALL_VFATS_DATA_MASK  = 0xffffffff;  ///< mask tracking data packets from all VFATs

      enum HWType {
        uTCA       = 0x0,
        GEB        = 0x1,
        AMC        = 0x2,
        CTP7       = 0x4,
        GLIB       = 0x8,
        OptoHybrid = 0x10
      };

      enum SCAType {
        ADC_V    = 0x0,
        ADC_T    = 0x1,
        PT100    = 0x2,
        SYSMON_V = 0x4,
        SYSMON_T = 0x8,
      };

      const double SYSMON_VCONV = 2.93; ///< 2.93mV per LSB 3/1024
      const double SYSMON_TCONV = 0.49; ///< 0.49C per LSB, 503.975/1024

      /**
       * parseAMCEnableList converts a string listing ranges of slot numbers to a bitmask
       * @param enableList is a , or - delineated list of active slots
       * @returns bitmask of active slots
       **/
      uint16_t parseAMCEnableList(std::string const&);

      /**
       * parseAMCEnableList converts a string listing ranges of slot numbers to a bitmask
       * @param enableList is a , or - delineated list of active slots
       * @param logger is the parent logger
       * @returns bitmask of active slots
       **/
      uint16_t parseAMCEnableList(std::string const&, log4cplus::Logger);
      // FIXME figure out how to pass the Logger objects as pointers, then we can get rid of the overloading FIXME
      /* uint16_t parseAMCEnableList(std::string const&, log4cplus::Logger*); */

      /**
       * parseVFATMaskList converts a string of listed VFATs to a bitmask
       * @param enableList is a , or - delineated list of VFATs to include in the mask
       * @returns Converted value
       **/
      uint32_t parseVFATMaskList(std::string const&);

      /**
       * parseVFATMaskList converts a string of listed VFATs to a bitmask
       * @param enableList is a , or - delineated list of VFATs to include in the mask
       * @param logger is the parent logger
       * @returns Converted value
       **/
      uint32_t parseVFATMaskList(std::string const&, log4cplus::Logger);

      /**
       * isValidSlotNumber takes a slot on a given type of hardware and returns true if it is valid
       * @param type is the type of hardware GEB or uTCA
       * @param slot is the slot
       * @returns true if slot is valid, false otherwise
       **/
      bool     isValidSlotNumber(HWType const&, std::string const&);

      /**
       * isValidSlotNumber takes a slot on a given type of hardware and returns true if it is valid
       * @param type is the type of hardware GEB or uTCA
       * @param slot is the slot
       * @param logger is the parent logger
       * @returns true if slot is valid, false otherwise
       **/
      bool     isValidSlotNumber(HWType const&, std::string const&, log4cplus::Logger);

      /**
       * scaConversion converts the ADC value to the corresponding temperature or voltage
       * @param type indicates the source of the ADC measurement
       * @param val is the raw value returned by the ADC
       * @returns Converted value
       **/
      double scaConversion(SCAType const& type, uint32_t const& val);

      /**
       * scaConversion converts the ADC value to the corresponding temperature or voltage
       * @param type indicates the source of the ADC measurement
       * @param val is the raw value returned by the ADC
       * @param logger is the parent logger
       * @returns Converted value
       **/
      /* double scaConversion(SCAType const& type, uint32_t const& val, log4cplus::Logger); */
      /* { */
      /*   switch(type) { */
      /*   case(gem::hw::utils::SCAType::ADC_V): */
      /*     return static_cast<double>(val*(0.244/1000)*3.0); */
      /*   case(gem::hw::utils::SCAType::ADC_T): */
      /*     return static_cast<double>(val*(-0.53)+381.2); */
      /*   case(gem::hw::utils::SCAType::PT100): */
      /*     return static_cast<double>(0.2597*(9891.8-2.44*val)); */
      /*   case(gem::hw::utils::SCAType::SYSMON_V): */
      /*     return static_cast<double>(val*0.49-273.15); */
      /*   case(gem::hw::utils::SCAType::SYSMON_T): */
      /*     return static_cast<double>(val*2.93/1000); */
      /*   default: */
      /*     return -1.0; */
      /*   } */
      /* }; */
    }  // namespace gem::hw::utils
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_UTILS_GEMCRATEUTILS_H
