#ifndef GEM_HWMONITOR_GEMHWMONITORBASE_H
#define GEM_HWMONITOR_GEMHWMONITORBASE_H

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
          {
            p_gemDevice = new T();
            m_isConfigured = false;
          }

        virtual ~gemHwMonitorBase()
          {
            delete p_gemDevice;
          }

        bool isConfigured()
          throw (xgi::exception::Exception)
        {return m_isConfigured;}

        void setIsConfigured(bool state)
          throw (xgi::exception::Exception)
        { m_isConfigured=state; }

        const std::string getDeviceId()
          throw (xgi::exception::Exception);

        /**
         *   Get subdevice status
         *   0 - device is working well, 1 - device has errors, 2 - device status unknown
         */
        unsigned int getSubDeviceStatus (unsigned int i)
          throw (xgi::exception::Exception)
        { return m_subDeviceStatus.at(i); }

        /**
         *   Set subdevice status
         *   0 - device is working well, 1 - device has errors, 2 - device status unknown
         */
        void setSubDeviceStatus (const unsigned int deviceStatus, const unsigned int i)
          throw (xgi::exception::Exception)
        { m_subDeviceStatus.at(i) = deviceStatus; }

        /**
         *   Add subdevice status
         *   0 - device is working well, 1 - device has errors, 2 - device status unknown
         */
        void addSubDeviceStatus (unsigned int deviceStatus)
          throw (xgi::exception::Exception)
        { m_subDeviceStatus.push_back(deviceStatus); }

        /**
         *   Get device status
         *   0 - device is working well, 1 - device has errors, 2 - device status unknown
         */
        unsigned int getDeviceStatus ()
          throw (xgi::exception::Exception)
        { return m_deviceStatus; }

        /**
         *   Set device status
         *   0 - device is working well, 1 - device has errors, 2 - device status unknown
         */
        void setDeviceStatus (const unsigned int deviceStatus)
          throw (xgi::exception::Exception)
        { m_deviceStatus = deviceStatus; }
        /**
         *   Set device configuration
         */
        void setDeviceConfiguration(T& device)
          throw (xgi::exception::Exception);
        /**
         *   Get device reference
         */
        T*  getDevice()
          throw (xgi::exception::Exception)
          { return p_gemDevice; }

        int getNumberOfSubDevices()
          throw (xgi::exception::Exception);

        const std::string getCurrentSubDeviceId(unsigned int subDeviceNumber)
          throw (xgi::exception::Exception);

        /**
         *   Access to board utils
         virtual void boardUtils ()
         throw (xgi::exception::Exception);
        */
      protected:
      private:
        bool m_isConfigured;
        unsigned int m_deviceStatus;                  ///!< 0 - device is working well, 1 - device has errors, 2 - device status unknown
        std::vector<unsigned int> m_subDeviceStatus;  ///!< 0 - device is working well, 1 - device has errors, 2 - device status unknown
        std::string m_xmlConfigFileName;
        T* p_gemDevice;
      };

    typedef gemHwMonitorBase<gem::utils::gemSystemProperties> gemHwMonitorSystem;
    typedef gemHwMonitorBase<gem::utils::gemCrateProperties>  gemHwMonitorCrate;
    typedef gemHwMonitorBase<gem::utils::gemGLIBProperties>   gemHwMonitorGLIB;
    typedef gemHwMonitorBase<gem::utils::gemOHProperties>     gemHwMonitorOH;
    typedef gemHwMonitorBase<gem::utils::gemVFATProperties>   gemHwMonitorVFAT;
  }  // end namespace gem::hwMonitor
}  // end namespace gem

#endif  // GEM_HWMONITOR_GEMHWMONITORBASE_H
