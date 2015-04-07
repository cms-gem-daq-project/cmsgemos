#ifndef gem_base_GEMApplication_h
#define gem_base_GEMApplication_h

#include <cstdlib>
#include "string.h"
#include "limits.h"
#include <string>
#include <string>
#include <deque>
#include <map>

#include "log4cplus/logger.h"

namespace xdaq {
  class ApplicationStub;
}

namespace xgi {
  class Input;
  class Output;
}

namespace gem {
  namespace base {
    
    class GEMWebApplication;
    class GEMMonitor;
    //class ConfigurationInfoSpaceHandler;
    
    class GEMApplication : public xdaq::WebApplication, public xdata::ActionListener
      {
      public:
	GEMApplication(xdaq::ApplicationStub *stub)
	  throw (xdaq::exception::Exception);

	virtual ~GEMApplication();

	std::string getFullURL();
	
	/**
	 * The init method is pure virtual in the base class, to ensure
	 * that it is fully implemented in every derived application,
	 * with a specific implementation
	 */
	virtual void init() = 0;
	
	virtual void actionPerformed(xdata::Event& event);
	
      protected:
	log4cplus::Logger gemLogger_;

	//virtual ConfigurationInfoSpaceHandler* getCfgInfoSpace() const;
	virtual GEMWebApplication*  getWebApp()  const;
	virtual GEMMonitor*         getMonitor() const;

      private:
	
	/**
	 * various application properties
	 *
	 */
	xdata::InfoSpace *appInfoSpaceP_;             /* */
						    
	xdaq::ApplicationDescriptor *appDescriptorP_; /* */
	xdaq::ApplicationContext    *appContextP_;    /* */
	xdaq::ApplicationGroup      *appGroupP_;      /* */
	xdaq::Zone                  *appZoneP_;       /* */

	std::string xmlClass_;
	unsigned long instance_;
	std::string urn_;	
	
	gem::base::GEMWebApplication* gemWebInterfaceP_;
	gem::base::GEMMonitor*        gemMonitorP_;

        //xdaq2rc::RcmsStateNotifier rcmsStateNotifier_;

	/**
	 *
	 */
	void monitorView(xgi::Input* in, xgi::Output* out);
	void expertView( xgi::Input* in, xgi::Output* out);
      };
    
  } // namespace gem::base
} // namespace gem

#endif
