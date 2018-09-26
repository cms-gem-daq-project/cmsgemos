/** @file GEMCrateUtils.h */

#ifndef GEM_HW_UTILS_GEMCRATEUTILS_H
#define GEM_HW_UTILS_GEMCRATEUTILS_H

#include <sstream>
#include <string>
#include <iomanip>

#include <gem/utils/exception/Exception.h>

namespace gem {
  namespace hw {
    namespace utils {

      enum HWType {
        uTCA = 0x0,
        GEB  = 0x1
      };

      uint16_t parseAMCEnableList(std::string const&);

      uint32_t parseVFATMaskList(std::string const&);

      bool     isValidSlotNumber(HWType const&, std::string const&);

    }  // namespace gem::hw::utils
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_UTILS_GEMCRATEUTILS_H
