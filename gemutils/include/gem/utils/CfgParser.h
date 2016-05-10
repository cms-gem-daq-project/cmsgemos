#ifndef GEM_UTILS_CFGPARSER
#define GEM_UTILS_CFGPARSER

#include <string>
#include <stdlib.h>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

namespace gem {
  namespace utils {
    class CfgParser {

    public:
      uint32_t processRegisterSetting(std::string regValue);

    };
  }  // namespace gem::utils
}  // namespace gem

#endif  // GEM_UTILS_CFGPARSER
