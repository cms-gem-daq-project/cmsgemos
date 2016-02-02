#ifndef gem_hwMonitor_gemHwMonitorHelper_h
#define gem_hwMonitor_gemHwMonitorHelper_h

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
          ptr_gemSystem_ = gemSystem;
          ptr_gemSystem_->setDeviceStatus(2);
        }

      virtual ~gemHwMonitorHelper()
        {
          delete gemXMLparser_;
        }

      //Make sure XML filename contains full path (adds BUILD_HOME/gemdaq-testing/gembase/xml/ if not)
      const std::string fixXMLconfigFile (const char* XMLfilename)
	throw (xgi::exception::Exception)
      {
	std::string file = XMLfilename;
	std::string temp_filename;
	//std::cout << "Contains BUILD_HOME?: "<< file.find(std::getenv("BUILD_HOME")) << std::endl;

	if( file.empty() )
	  return "";
        if( file.find(std::getenv("BUILD_HOME")) == 0)
	  {
	    temp_filename = XMLfilename;
	  }
	else
	  {
	    temp_filename = std::getenv("BUILD_HOME");
	    temp_filename += "/gemdaq-testing/gembase/xml/";
	    temp_filename += XMLfilename;
	  }
	return temp_filename;
      }

      void setXMLconfigFile (const char* inputXMLfilename)
        throw (xgi::exception::Exception)
      {

	xmlConfigFileName_ = inputXMLfilename;

        if (ptr_gemSystem_) 
          ptr_gemSystem_->setIsConfigured(false);
        else 
          std::cout << "[WARNING] New XML file set, but device state hasn't changed (NULL-pointer)" << std::endl;
      }
      const std::string getXMLconfigFile ()
        throw (xgi::exception::Exception)
      {
        return xmlConfigFileName_;
      }
      void setDBSconfigFile (std::string inputDBSfilename)
        throw (xgi::exception::Exception) {}
      const std::string getDBSconfigFile ()
        throw (xgi::exception::Exception) { return "Not implemented yet"; }
     void configure()
        throw (xgi::exception::Exception)
      {
        gemXMLparser_ = new gem::utils::gemXMLparser(xmlConfigFileName_);
        gemXMLparser_->parseXMLFile();
        ptr_gemSystem_->setDeviceConfiguration(*(gemXMLparser_->getGEMDevice()));
      }

    protected:
    private:
      gemHwMonitorSystem* ptr_gemSystem_;
      gem::utils::gemXMLparser* gemXMLparser_;
      std::string xmlConfigFileName_;

    };
  }
}
#endif
