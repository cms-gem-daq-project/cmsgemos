#ifndef gem_base_GEMWebApplication_h
#define gem_base_GEMWebApplication_h

#include "xdaq/WebApplication.h"

#include "xgi/framework/Method.h"
#include "xgi/framework/UIManager.h"

#include "gem/base/GEMApplication.h"

#include "cgicc/HTMLClasses.h"

namespace xdaq {
  class ApplicationStub;
}

namespace xgi {
  class Input;
  class Output;
}

namespace cgicc {
  BOOLEAN_ELEMENT(section,"section");
}

namespace gem {
  namespace base {

    class GEMMonitor;

    class GEMWebApplication
      {
      public:
	GEMWebApplication(xdaq::Application *gemApp, GEMMonitor* gemMonitor, bool hasFSM=false)
	  throw (xdaq::exception::Exception);
	
      protected:
	virtual void monitorPage(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	
	virtual void expertPage(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);

	//fsm specific functions
	virtual void webEnable(   xgi::Input *in, xgi::Output *out );
	virtual void webConfigure(xgi::Input *in, xgi::Output *out );
	virtual void webStart(    xgi::Input *in, xgi::Output *out );
	virtual void webPause(    xgi::Input *in, xgi::Output *out );
	virtual void webResume(   xgi::Input *in, xgi::Output *out );
	virtual void webStop(     xgi::Input *in, xgi::Output *out );
	virtual void webHalt(     xgi::Input *in, xgi::Output *out );
	virtual void webReset(    xgi::Input *in, xgi::Output *out );
	
	
      private:
	log4cplus::Logger gemWebLogger_;
	GEMMonitor* gemMonitorP_;
	xdaq::Application* gemAppP_;

	GEMWebApplication(GEMWebApplication const&);

      };
  } // namespace gem::base
} // namespace gem

#endif
