#ifndef gem_utils_GEMREGISTERUTILS_h
#define gem_utils_GEMREGISTERUTILS_h

#include <sstream>
#include <string>
#include <iomanip>

#include "gem/utils/exception/Exception.h"

namespace gem {
  namespace utils {
    
    std::string uint32ToString(uint32_t const& val);
    
    std::string uint32ToDottedQuad(uint32_t const& val);
    
    std::string uint32ToGroupedHex(uint32_t const& val1, uint32_t const& val2);
    
  } // end namespace gem::utils
} // end namespace gem

#endif
