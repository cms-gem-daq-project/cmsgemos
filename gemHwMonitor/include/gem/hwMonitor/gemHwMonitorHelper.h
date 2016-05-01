#ifndef GEM_HWMONITOR_GEMHWMONITORHELPER_H
#define GEM_HWMONITOR_GEMHWMONITORHELPER_H

#include <string>
#include <vector>
#include <cstdlib>

#include "gem/utils/gemXMLparser.h"
#include "gem/utils/gemComplexDeviceProperties.h"
#include "gem/utils/gemDeviceProperties.h"

#include "gemHwMonitorBase.h"

namespace gem {
  namespace hwMonitor {
    class gemHwMonitorHelper
    {
    public:
      gemHwMonitorHelper(gemHwMonitorSystem* gemSystem)
        throw (xdaq::exception::Exception)
        {
          std::string defaulXMLcfgFile = std::getenv("BUILD_HOME");
          defaulXMLcfgFile +="/gemdaq-testing/gembase/xml/gem_conf_tamu_test.xml";
          this->setXMLconfigFile(defaulXMLcfgFile.c_str());
          p_gemSystem = gemSystem;
          p_gemSystem->setDeviceStatus(2);
        }

      virtual ~gemHwMonitorHelper()
        {
          delete p_gemXMLparser;
        }

      // Make sure XML filename contains full path (adds BUILD_HOME/gemdaq-testing/gembase/xml/ if not)
      const std::string fixXMLconfigFile (const char* XMLfilename)
        throw (xgi::exception::Exception)
      {
        std::string file = XMLfilename;
        std::string temp_filename;
        // std::cout << "Contains BUILD_HOME?: "<< file.find(std::getenv("BUILD_HOME")) << std::endl;

        if (file.empty())
          return "";
        if (file.find(std::getenv("BUILD_HOME")) == 0) {
          temp_filename = XMLfilename;
        } else {
          temp_filename = std::getenv("BUILD_HOME");
          temp_filename += "/gemdaq-testing/gembase/xml/";
          temp_filename += XMLfilename;
        }
        return temp_filename;
      }

      void setXMLconfigFile (const char* inputXMLfilename)
        throw (xgi::exception::Exception)
      {
        m_xmlConfigFileName = inputXMLfilename;

        if (p_gemSystem)
          p_gemSystem->setIsConfigured(false);
        else
          std::cout << "[WARNING] New XML file set, but device state hasn't changed (NULL-pointer)" << std::endl;
      }
      const std::string getXMLconfigFile ()
        throw (xgi::exception::Exception)
      {
        return m_xmlConfigFileName;
      }
      void setDBSconfigFile (std::string inputDBSfilename)
        throw (xgi::exception::Exception) {}
      const std::string getDBSconfigFile ()
        throw (xgi::exception::Exception) { return "Not implemented yet"; }
     void configure()
        throw (xgi::exception::Exception)
      {
        p_gemXMLparser = new gem::utils::gemXMLparser(m_xmlConfigFileName);
        p_gemXMLparser->parseXMLFile();
        p_gemSystem->setDeviceConfiguration(*(p_gemXMLparser->getGEMDevice()));
      }

    protected:
    private:
      gemHwMonitorSystem* p_gemSystem;
      gem::utils::gemXMLparser* p_gemXMLparser;
      std::string m_xmlConfigFileName;
    };
  }  // namespace gem::hwMonitor
}  // namespace gem

#endif  // GEM_HWMONITOR_GEMHWMONITORHELPER_H
