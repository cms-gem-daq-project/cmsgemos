/*
 * This class provides base interface for the device properties.
 */
#ifndef gem_base_utils_gemComplexDeviceProperties_h
#define gem_base_utils_gemComplexDeviceProperties_h

#include <string>
#include <vector>
#include <map>

#include "gemDeviceProperties.h"

namespace gem {
    namespace base {
        namespace utils {
            template <class T>
            class gemComplexDeviceProperties: public gemDeviceProperties {
                friend class gemXMLparser;
                public:
                gemComplexDeviceProperties();
                ~gemComplexDeviceProperties();
                const std::vector<T*>& getSubDevicesRefs() {return subDevicesRefs_;}
    
                private:
                std::vector <T*> subDevicesRefs_;
            };
            typedef gemComplexDeviceProperties<gem::base::utils::gemCrateProperties> gemSystemProperties;
            typedef gemComplexDeviceProperties<gem::base::utils::gemGLIBProperties> gemCrateProperties;
            typedef gemComplexDeviceProperties<gem::base::utils::gemOHProperties> gemGLIBProperties;
            typedef gemComplexDeviceProperties<gem::base::utils::gemVFATProperties> gemOHProperties;
        }
    }
}

#endif
