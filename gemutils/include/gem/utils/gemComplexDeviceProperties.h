#ifndef GEM_UTILS_GEMCOMPLEXDEVICEPROPERTIES_H
#define GEM_UTILS_GEMCOMPLEXDEVICEPROPERTIES_H
/*
 * This class provides base interface for the device properties.
 */

#include <algorithm>
#include <cstdlib>

#include "gemDeviceProperties.h"

namespace gem {
  namespace utils {
    template <class T>
      class gemComplexDeviceProperties : public gemDeviceProperties {

    public:
      gemComplexDeviceProperties() {}

      ~gemComplexDeviceProperties() {for_each(m_subDevicesRefs.begin(), m_subDevicesRefs.end(), free);}  // Unsafe code. Deletes objects created in different place.

      const std::vector<T*>& getSubDevicesRefs() {return m_subDevicesRefs;}

      void addSubDeviceRef(T* &&subDeviceRef) {m_subDevicesRefs.push_back(subDeviceRef);}
      
    private:
      std::vector <T*> m_subDevicesRefs;
    };
    
    typedef gemDeviceProperties gemVFATProperties;
    typedef gemComplexDeviceProperties<gemVFATProperties>  gemOHProperties;
    typedef gemComplexDeviceProperties<gemOHProperties>    gemGLIBProperties;
    typedef gemComplexDeviceProperties<gemGLIBProperties>  gemCrateProperties;
    typedef gemComplexDeviceProperties<gemCrateProperties> gemSystemProperties;
  }  // end namespace gem::utils
}  // end namespace gem

#endif  // GEM_UTILS_GEMCOMPLEXDEVICEPROPERTIES_H
