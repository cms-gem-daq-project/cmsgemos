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
                    defaulXMLcfgFile +="/gemdaq-testing/gembase/xml/gem_conf.xml";
                    this->setXMLconfigFile(defaulXMLcfgFile.c_str());
                    gemSystem_ = gemSystem;
                    gemSystem_->setDeviceStatus(2);
                }

                virtual ~gemHwMonitorHelper()
                {
                    delete gemXMLparser_;
                }

                void setXMLconfigFile (const char* inputXMLfilename)
                    throw (xgi::exception::Exception)
                {
                    xmlConfigFileName_ = inputXMLfilename;
                    gemSystem_->setIsConfigured(false);
                }
                const std::string& getXMLconfigFile ()
                    throw (xgi::exception::Exception)
                {
                    return xmlConfigFileName_;
                }
                void setDBSconfigFile (std::string inputDBSfilename)
                    throw (xgi::exception::Exception){}
                const std::string& getDBSconfigFile (std::string inputDBSfilename)
                    throw (xgi::exception::Exception){return "Not implemented yet";}
                void configure()
                    throw (xgi::exception::Exception)
                {
                    gemXMLparser_ = new gem::base::utils::gemXMLparser(xmlConfigFileName_);
                    gemXMLparser_->parseXMLFile();
                    gemSystem_->getDeviceConfiguration(gemXMLparser_->getGEMDevice());
                }

            protected:
            private:
                gemHwMonitorSystem* gemSystem_;
                gem::base::utils::gemXMLparser* gemXMLparser_;
                std::string xmlConfigFileName_;

        };
    }
}
#endif
