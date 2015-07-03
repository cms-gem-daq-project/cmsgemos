#ifndef gem_hwMonitor_gemHwMonitorHelper_h
#define gem_hwMonitor_gemHwMonitorHelper_h

#include <string>
#include <vector>
#include <cstdlib>

#include "gem/utils/gemXMLparser.h"
#include "gem/utils/gemComplexDeviceProperties.h"
#include "gem/utils/gemDeviceProperties.h"

#include "gemHwMonitorBase.h"
#include "gem/utils/GEMLogging.h"

namespace gem {
  namespace hwMonitor {
    class gemHwMonitorHelper
    {
    public:
      gemHwMonitorHelper(log4cplus::Logger const& gemLogger, gemHwMonitorSystem* gemSystem) 
	throw (xdaq::exception::Exception) :
      ptr_gemSystem_(gemSystem),
	gemLogger_(gemLogger)
	{
	  //gemLogger_.setLogLevel(DEBUG_LOG_LEVEL);
	  WARN("gemHwMonitorHelper::getting env vars");
	  std::string defaulXMLcfgFile = std::getenv("BUILD_HOME");
	  WARN("gemHwMonitorHelper::setting default file");
	  defaulXMLcfgFile +="/gemdaq-testing/gembase/xml/gem_conf_proposal.xml";
	  WARN("gemHwMonitorHelper::setting file");
	  this->setXMLconfigFile(defaulXMLcfgFile.c_str());
	  WARN("gemHwMonitorHelper::setting gemsystem pointer");
	  //ptr_gemSystem_ = gemSystem;
	  WARN("gemHwMonitorHelper::setting gemsystem pointer device status");
	  ptr_gemSystem_->setDeviceStatus(2);
	  WARN("gemHwMonitorHelper::constructor finished");
	}

      virtual ~gemHwMonitorHelper()
	{
	  delete gemXMLparser_;
	}

      void setXMLconfigFile (const char* inputXMLfilename)
      //throw (xgi::exception::Exception)
      {
	WARN("gemHwMonitorHelper::setXMLconfigFile");
	xmlConfigFileName_ = inputXMLfilename;
	if (ptr_gemSystem_) {
	  WARN("gemHwMonitorHelper::device state changed");
	  //something is happening here, but what???
	  ptr_gemSystem_->setIsConfigured(false);
	  return;
	} else {
	  WARN("gemHwMonitorHelper::New XML file set, but device state hasn't changed (NULL-pointer)");
	  return;
	}
      }
      const std::string getXMLconfigFile ()
	throw (xgi::exception::Exception)
      {
	return xmlConfigFileName_;
      }
      void setDBSconfigFile (std::string inputDBSfilename)
	throw (xgi::exception::Exception){}
      const std::string getDBSconfigFile ()
	throw (xgi::exception::Exception){return "Not implemented yet";}
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
      log4cplus::Logger gemLogger_;

    };
  }
}
#endif
