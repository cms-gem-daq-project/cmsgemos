/*
 * This class provides interface for the Crate properties.
 */
#ifndef gem_base_utils_gemCrateProperties_h
#define gem_base_utils_gemCrateProperties_h

#include <string>
#include <vector>
#include <map>

#include "gemDeviceProperties.h"
#include "gemGLIBProperties.h"

namespace gem {
    namespace base {
        namespace utils {
            class gemCrateProperties public gemDeviceProperties {
                friend class gem::base::utils::gemXMLparser;
                gemCrateProperties(){}
                ~gemCrateProperties(){}
                const std::vector<gem::base::utils::gemGLIBProperties>& getGLIBRefs(return subDevicesIDs_;)

                private:
                std::vector <gem::base::utils::gemGLIBProperties> subDevicesRefs_;
            }
        }
    }
}

#endif
