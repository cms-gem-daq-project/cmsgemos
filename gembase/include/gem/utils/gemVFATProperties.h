/*
 * This class provides interface for the VFAT properties.
 */
#ifndef gem_base_utils_gemVFATProperties_h
#define gem_base_utils_gemVFATProperties_h

#include <string>
#include <vector>
#include <map>

#include "gemDeviceProperties.h"

namespace gem {
    namespace base {
        namespace utils {
            class gemVFATProperties: public gemDeviceProperties {
                friend class gem::base::utils::gemXMLparser;
                gemVFATProperties();
                ~gemVFATProperties();
            };
        }
    }
}

#endif
