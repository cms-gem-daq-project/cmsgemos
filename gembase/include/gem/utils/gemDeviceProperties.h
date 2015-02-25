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
                virtual ~gemDeviceProperties();
                const std::string& getDeviceId() const {return deviceId_;}
                void setDeviceId(std::string deviceId) {deviceId_ = deviceId;}
                const std::vector<std::string>& getSubDevicesIds() {return subDevicesIds_;}
                const std::map<std::string, std::string>& getDeviceProperties() {return deviceProperties_;}

                private:
                std::string deviceId_;
                std::vector <std::string> subDevicesIds_;
                std::map <std::string, std::string> deviceProperties_;
            };
        }
    }
}

#endif
