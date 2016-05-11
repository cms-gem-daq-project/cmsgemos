#ifndef GEM_UTILS_GEMREGISTERUTILS_H
#define GEM_UTILS_GEMREGISTERUTILS_H

#include <sstream>
#include <string>
#include <iomanip>

#include <gem/utils/exception/Exception.h>

namespace gem {
  namespace utils {
    
    std::string uint32ToString(uint32_t const& val);
    
    std::string uint32ToDottedQuad(uint32_t const& val);
    
    std::string uint32ToGroupedHex(uint32_t const& val1, uint32_t const& val2);
    
  }  // namespace gem::utils
}  // namespace gem

#endif  // GEM_UTILS_GEMREGISTERUTILS_H
