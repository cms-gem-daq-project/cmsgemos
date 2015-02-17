/*
 * This class provides interface for the OH properties.
 */
#ifndef gem_base_utils_gemOHProperties_h
#define gem_base_utils_gemOHProperties_h

#include <string>
#include <vector>
#include <map>

#include "gemDeviceProperties.h"
#include "gemVFATProperties.h"

namespace gem {
    namespace base {
        namespace utils {
            class gemOHProperties: public gemDeviceProperties {
                friend class gem::base::utils::gemXMLparser;
                gemOHProperties();
                ~gemOHProperties();
                const std::vector<gem::base::utils::gemVFATProperties*>& getVFATRefs() {return subDevicesRefs_;}

                private:
                std::vector <gem::base::utils::gemVFATProperties*> subDevicesRefs_;
            };
        }
    }
}

#endif
