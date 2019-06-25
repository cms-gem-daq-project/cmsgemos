/** @file GEMRegisterUtils.h */

#ifndef GEM_UTILS_GEMREGISTERUTILS_H
#define GEM_UTILS_GEMREGISTERUTILS_H

#include <sstream>
#include <string>
#include <iomanip>

#include <gem/utils/exception/Exception.h>

namespace gem {
  namespace utils {

    /**
     * @brief Interprets a 32-bit word as 4 ASCII characters
     * @param val is a 32-bit integer word
     * @returns string corresponding to the ASCII representation of the 4 bytes "b3b2b1b0"
     */
    std::string uint32ToString(uint32_t const& val);

    /**
     * @brief Interprets a 32-bit word as a dotted quad (e.g., IP address)
     * @param val is a 32-bit integer word
     * @returns string corresponding to the dotted quad representation of the 4 bytes "b3.b2.b1.b0"
     */
    std::string uint32ToDottedQuad(uint32_t const& val);

    /**
     * @brief Interprets a pair of 32-bit words as 6 hex characters (e.g., MAC address)
     * @param val1 is a 32-bit integer word corresponding to 32 MSBs
     * @param val2 is a 32-bit integer word corresponding to 32 LSBs
     * @returns string corresponding to the hex representation of the 4 bytes "b5:b4:b3:b2:b1:b0"
     */
    std::string uint32ToGroupedHex(uint32_t const& val1, uint32_t const& val2);

    /**
     * @brief Interprets a 64-bit word as 6 hex characters (e.g., MAC address)
     * @param val is a 64-bit integer word
     * @returns string corresponding to the hex representation of the 4 bytes "b5:b4:b3:b2:b1:b0"
     */
    std::string uint64ToGroupedHex(uint64_t const& val);

  }  // namespace gem::utils
}  // namespace gem

#endif  // GEM_UTILS_GEMREGISTERUTILS_H
