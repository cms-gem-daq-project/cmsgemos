#ifndef gem_base_utils_CfgParser
#define gem_base_utils_CfgParser

#include <stdlib.h>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

namespace gem {
  namespace base {
    namespace utils {
      class CfgParser {
	
        public:
	  uint32_t processRegisterSetting(std::string regValue);
	  
      };
    } //end namespace gem::base::utils
  } //end namespace gem::base
} //end namespace gem

#endif
