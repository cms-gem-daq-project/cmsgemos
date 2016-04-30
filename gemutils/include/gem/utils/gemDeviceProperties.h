#ifndef GEM_UTILS_GEMDEVICEPROPERTIES_H
#define GEM_UTILS_GEMDEVICEPROPERTIES_H
/*
 * This class provides base interface for the device properties.
 */

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace gem {
  namespace utils {
    class gemDeviceProperties {
    public:
      gemDeviceProperties() {}

      virtual ~gemDeviceProperties() {}

      const std::string& getDeviceId() const {return m_deviceId;}

      void setDeviceId(const char* deviceId) {m_deviceId = deviceId;}

      const std::vector<std::string>& getSubDevicesIds() {return m_subDevicesIds;}

      void addSubDeviceId(const std::string& deviceId) {m_subDevicesIds.push_back(deviceId);}

      const std::map<std::string, std::string>& getDeviceProperties() {return m_deviceProperties;}

      void addDeviceProperty(const std::string& first, const std::string& second) {
        m_deviceProperties.insert(std::pair<std::string, std::string>(first,second));
      }
      
    private:
      std::string m_deviceId;
      std::vector <std::string> m_subDevicesIds;
      std::map <std::string, std::string> m_deviceProperties;
    };
  }  // end namespace gem::utils
}  // end namespace gem

#endif  // GEM_UTILS_GEMDEVICEPROPERTIES_H
