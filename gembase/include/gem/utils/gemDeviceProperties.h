/*
 * This class provides base interface for the device properties.
 */
#ifndef gem_base_utils_gemDeviceProperties_h
#define gem_base_utils_gemDeviceProperties_h

#include <string>
#include <vector>
#include <map>

namespace gem {
    namespace base {
        namespace utils {
            class gemDeviceProperties {
                friend class gemXMLparser;
                public:
                gemDeviceProperties();
                ~gemDeviceProperties();
                const std::string& getDeviceId() const {return deviceId_;}
                void setDeviceId(std::string deviceId) {deviceId_ = deviceId;}
                const std::vector<std::string>& getSubDevicesIDs() {return subDevicesIDs_;}
                const std::map<std::string, std::string>& getDeviceProperties() {return deviceProperties_;}

                private:
                std::string deviceId_;
                std::vector <std::string> subDevicesIDs_;
                std::map <std::string, std::string> deviceProperties_;
            };
        }
    }
}

#endif
