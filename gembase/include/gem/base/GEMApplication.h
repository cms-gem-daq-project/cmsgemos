#ifndef gem_base_GEMApplication_h
#define gem_base_GEMApplication_h

#include <cstdlib>
//#include "limits.h"
#include <limits>
#include <string>
#include <memory>
#include <deque>
#include <map>

#include "xdaq/WebApplication.h"
#include "xgi/framework/UIManager.h"

#include "xdata/Boolean.h"
#include "xdata/Integer.h"
#include "xdata/Integer32.h"
#include "xdata/Integer64.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"
#include "xdata/UnsignedInteger64.h"
#include "xdata/String.h"
#include "xdata/Float.h" 
#include "xdata/Double.h" 
#include "xdata/Boolean.h"
#include "xdata/Vector.h"
#include "xdaq/XceptSerializer.h"

#include "toolbox/string.h"

#include "xgi/Input.h"
#include "xgi/Method.h"
#include "xgi/Output.h"
#include "xoap/Method.h"

#include "xcept/Exception.h"
#include "xcept/tools.h"

#include "xdaq/NamespaceURI.h"
#include "xdaq/Application.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationDescriptorImpl.h"
#include "xdaq/exception/Exception.h"

#include "toolbox/TimeVal.h"

#include "log4cplus/logger.h"
#include "gem/utils/GEMLogging.h"
#include "gem/base/exception/Exception.h"
#include "gem/base/utils/exception/Exception.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

namespace xdaq {
  class ApplicationStub;
}

namespace xgi {
  class Input;
  class Output;
}

namespace gem {
  namespace base {
    
    class GEMMonitor;
    class GEMFSMApplication;
    class GEMWebApplication;

    namespace utils {
      class GEMInfoSpaceToolBox;
    }

    class GEMApplication : public xdaq::WebApplication, public xdata::ActionListener
      {
        friend class GEMMonitor;
        //friend class GEMFSMApplication;
        friend class GEMWebApplication;

      public:
        GEMApplication(xdaq::ApplicationStub *stub)
          throw (xdaq::exception::Exception);

        virtual ~GEMApplication();

        std::string getFullURL();
	
        ///**
        // * The init method is pure virtual in the base class, to ensure
        // * that it is fully implemented in every derived application,
        // * with a specific implementation
        // */
        //virtual void init() = 0;
	
        /**
         * The actionPerformed method will have a default implementation here
         * and can be further specified in derived applications, that will 
         * subsequently call gem::base::GEMApplication::actionPerformed(event)
         * to replicate the default behaviour
         **/
        virtual void actionPerformed(xdata::Event& event);
	
        void xgiDefault(xgi::Input* in, xgi::Output* out);
        void xgiMonitor(xgi::Input* in, xgi::Output* out);
        void xgiExpert( xgi::Input* in, xgi::Output* out);
        void jsonUpdate(xgi::Input* in, xgi::Output* out);

        // std::shared_ptr<utils::GEMInfoSpaceToolBox> getGEMISToolBox() { return p_infoSpaceToolBox;       };
        std::shared_ptr<utils::GEMInfoSpaceToolBox> getAppISToolBox() { return p_appInfoSpaceToolBox;     };
        std::shared_ptr<utils::GEMInfoSpaceToolBox> getMonISToolBox() { return p_monitorInfoSpaceToolBox; };
        std::shared_ptr<utils::GEMInfoSpaceToolBox> getCfgISToolBox() { return p_configInfoSpaceToolBox;  };

      protected:
        log4cplus::Logger m_gemLogger;
        
        // std::shared_ptr<utils::GEMInfoSpaceToolBox> p_infoSpaceToolBox;
        std::shared_ptr<utils::GEMInfoSpaceToolBox> p_appInfoSpaceToolBox;
        std::shared_ptr<utils::GEMInfoSpaceToolBox> p_monitorInfoSpaceToolBox;
        std::shared_ptr<utils::GEMInfoSpaceToolBox> p_configInfoSpaceToolBox;

        xdata::InfoSpace *p_appInfoSpace;       /* generic application parameters */
        // maybe instead of multiple info spaces, use sets inside the infospace toolbox?
        xdata::InfoSpace *p_monitorInfoSpace;   /* monitoring parameters, stored in the appInfoSpace */
        xdata::InfoSpace *p_configInfoSpace;    /* configuration parameters, stored in the appInfoSpace */

        // some of these, namely update, move to monitor?
        virtual void importConfigurationParameters();
        virtual void fillConfigurationInfoSpace();
        virtual void updateConfigurationInfoSpace();

        virtual void importMonitoringParameters();
        virtual void fillMonitoringInfoSpace();
        virtual void updateMonitoringInfoSpace();

        virtual GEMWebApplication *getWebApp()  const { return p_gemWebInterface; };
        virtual GEMMonitor        *getMonitor() const { return p_gemMonitor;      };

        GEMWebApplication *p_gemWebInterface; /* */
        GEMMonitor        *p_gemMonitor;      /* */

      public:
        // should these be protected?
        /**
         * various application properties
         */
        xdaq::ApplicationDescriptor *p_appDescriptor; /* */
        xdaq::ApplicationContext    *p_appContext;    /* */
        xdaq::ApplicationGroup      *p_appGroup;      /* */
        xdaq::Zone                  *p_appZone;       /* */

        std::string m_xmlClass;
        std::string m_urn;
	
        uint32_t m_instance;

      protected:
        xdata::Integer64 m_runNumber;
        
        xdata::String  m_runType;
        xdata::String  m_cfgType;
                
        //xdaq2rc::RcmsStateNotifier rcmsStateNotifier_;

      };
    
  } // namespace gem::base
} // namespace gem

#endif
