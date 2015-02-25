/*
 * This class provides base interface for the device properties.
 */
#ifndef gem_base_utils_gemSystemProperties_h
#define gem_base_utils_gemSystemProperties_h

#include <string>
#include <vector>
#include <map>

#include "gemDeviceProperties.h"
#include "gemCrateProperties.h"

namespace gem {
    namespace base {
        namespace utils {
            class gemSystemProperties: public gemDeviceProperties {
                friend class gemXMLparser;
                public:
                gemSystemProperties();
                ~gemSystemProperties();
                const std::vector<gem::base::utils::gemCrateProperties*>& getCrateRefs() {return subDevicesRefs_;}

                private:
                std::vector <gem::base::utils::gemCrateProperties*> subDevicesRefs_;
            };
        }
    }
}

#endif
