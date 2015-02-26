#ifndef gem_hwMonitor_gemHwMonitorBase_h
#define gem_hwMonitor_gemHwMonitorBase_h

#include <string>
#include <vector>
#include <cstdlib>

#include "xdaq/Application.h"
#include "xgi/framework/Method.h"

#include "gem/utils/gemXMLparser.h"
#include "gem/utils/gemComplexDeviceProperties.h"
#include "gem/utils/gemDeviceProperties.h"

namespace gem {
    namespace hwMonitor {
        template <class T>
        class gemHwMonitorBase
        {
            public:
                gemHwMonitorBase()
                    throw (xdaq::exception::Exception)
                {}

                virtual ~gemHwMonitorBase(){}

                bool isConfigured()
                    throw (xgi::exception::Exception)
                {return isConfigured_;}

                void setIsConfigured(bool state)
                    throw (xgi::exception::Exception)
                {isConfigured_=state;}
               
                const std::string& getDeviceId()
                    throw (xgi::exception::Exception);
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
                void setDeviceStatus (const unsigned int deviceStatus)
                    throw (xgi::exception::Exception)
		            {deviceStatus_ = deviceStatus;}
                /**
                 *   Get device configuration
                 */
                void getDeviceConfiguration()
                    throw (xgi::exception::Exception);

                int getNumberOfSubDevices()
                    throw (xgi::exception::Exception);

                std::string getCurrentSubDeviceId(unsigned int subDeviceNumber)
                                    throw (xgi::exception::Exception);

                /**
                 *   Access to board utils
                virtual void boardUtils ()
                    throw (xgi::exception::Exception);
                 */
            protected:
            private:
                bool isConfigured_;
                unsigned int deviceStatus_; // 0 - device is working well, 1 - device has errors, 2 - device status unknown
                std::string xmlConfigFileName_;
                gem::base::utils::gemXMLparser *gemXMLparser_;
                T* gemDevice_;
        };

        typedef gemHwMonitorBase<gem::base::utils::gemSystemProperties> gemHwMonitorSystem;
        typedef gemHwMonitorBase<gem::base::utils::gemCrateProperties> gemHwMonitorCrate;
    } // end namespace hwMon
} // end namespace gem
//#include "../../../src/common/gemHwMonitorBase.cc"
#endif
