#ifndef gem_hwMonitor_gemHwMonitorBase_h
#define gem_hwMonitor_gemHwMonitorBase_h

#include <string>
#include <vector>
#include <cstdlib>

#include "xdaq/Application.h"
#include "xgi/framework/Method.h"

#include "gem/utils/gemXMLparser.h"
#include "gem/utils/gemHwProperties.h"

namespace gem {
    namespace base {
        namespace utils {
            class gemXMLparcer;
            class gemDeviceProperties;
        }
    }
    namespace hwMonitor {
        class gemHwMonitorBase
        {
            public:
                gemHwMonitorBase()
                    throw (xdaq::exception::Exception);

                ~gemHwMonitorBase(){delete gemXMLparser_;}
               /**
                 *   Set XML configuration file
                 */
                void setXMLconfigFile (std::string inputXMLfilename)
                    throw (xgi::exception::Exception)
		            {xmlConfigFileName_ = inputXMLfilename;}
                /**
                 *   Get XML configuration file
                 */
                std::string getXMLconfigFile ()
                    throw (xgi::exception::Exception)
		            {return xmlConfigFileName_;}
                /**
                 *   Select database configuration file
                 */
                void setDBSconfigFile (std::string inputDBSfilename)
                    throw (xgi::exception::Exception){}
                /**
                 *   Select one of available devices
                 */
                void setDeviceID(std::string deviceID)
                    throw (xgi::exception::Exception)
		            {deviceID_ = deviceID;}
                /**
                 *   Get selected device
                 */
                std::string getDeviceID()
                    throw (xgi::exception::Exception)
		            {return deviceID_;}
                /**
                 *   Get device status
                 *   0 - device is working well, 1 - device has errors, 2 - device status unknown
                 */
                unsigned int getDeviceStatus ()
                    throw (xgi::exception::Exception)
		            {return deviceStatus_;}

                /**
                 *   Set device status
                 *   0 - device is working well, 1 - device has errors, 2 - device status unknown
                 */
                void setDeviceStatus (unsigned int deviceStatus)
                    throw (xgi::exception::Exception)
		            {deviceStatus_ = deviceStatus;}
                /**
                 *   Initialize XML parser
                 */
                void initParser()
                    throw (xgi::exception::Exception);
                /**
                 *   Get device configuration
                 */
                void getDeviceConfiguration()
                    throw (xgi::exception::Exception);

                int getNumberOfSubDevices()
                    throw (xgi::exception::Exception)
                    {return subDevicesRefs_.size();}

                /**
                 *   Access to board utils
                virtual void boardUtils ()
                    throw (xgi::exception::Exception);
                 */
            protected:
            private:
                std::string currentDeviceID_;
                unsigned int deviceStatus_; // 0 - device is working well, 1 - device has errors, 2 - device status unknown
                std::string deviceID_;
                std::string xmlConfigFileName_;
                gem::base::utils::gemXMLparser *gemXMLparser_;
                std::vector<gem::base::utils::gemDeviceProperties*> subDevicesRefs_;
        }; // end namespace hwMon
    }
} // end namespace gem
#endif
