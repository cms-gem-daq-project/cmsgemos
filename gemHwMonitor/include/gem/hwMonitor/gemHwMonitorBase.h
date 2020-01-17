/** @file gemHwMonitorBase.h */

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
          {
            p_gemDevice = new T();
            m_isConfigured = false;
          }

        virtual ~gemHwMonitorBase()
          {
            delete p_gemDevice;
          }

        bool isConfigured() {return m_isConfigured;}

        void setIsConfigured(bool state) {m_isConfigured=state;}

        const std::string getDeviceId();

        /**
         *   Get subdevice status
         *   0 - device is working well, 1 - device has errors, 2 - device status unknown
         */
        unsigned int getSubDeviceStatus (unsigned int i) { return m_subDeviceStatus.at(i); }

        /**
         *   Set subdevice status
         *   0 - device is working well, 1 - device has errors, 2 - device status unknown
         */
        void setSubDeviceStatus (const unsigned int deviceStatus, const unsigned int i) { m_subDeviceStatus.at(i) = deviceStatus; }

        /**
         *   Add subdevice status
         *   0 - device is working well, 1 - device has errors, 2 - device status unknown
         */
        void addSubDeviceStatus (unsigned int deviceStatus) { m_subDeviceStatus.push_back(deviceStatus); }

        /**
         *   Get device status
         *   0 - device is working well, 1 - device has errors, 2 - device status unknown
         */
        unsigned int getDeviceStatus () { return m_deviceStatus; }

        /**
         *   Set device status
         *   0 - device is working well, 1 - device has errors, 2 - device status unknown
         */
        void setDeviceStatus (const unsigned int deviceStatus) { m_deviceStatus = deviceStatus; }
        /**
         *   Set device configuration
         */
        void setDeviceConfiguration(T& device);
        /**
         *   Get device reference
         */
        T*  getDevice() { return p_gemDevice; }

        int getNumberOfSubDevices();

        const std::string getCurrentSubDeviceId(unsigned int subDeviceNumber);

        /**
         *   Access to board utils
         virtual void boardUtils ();
        */
      protected:
      private:
        bool m_isConfigured;
        unsigned int m_deviceStatus;                  ///< 0 - device is working well, 1 - device has errors, 2 - device status unknown
        std::vector<unsigned int> m_subDeviceStatus;  ///< 0 - device is working well, 1 - device has errors, 2 - device status unknown
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
