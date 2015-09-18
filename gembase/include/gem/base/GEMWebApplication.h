#ifndef gem_base_GEMWebApplication_h
#define gem_base_GEMWebApplication_h

#include "xdaq/WebApplication.h"
#include "xgi/framework/Method.h"
#include "xgi/framework/UIManager.h"

#include "cgicc/HTMLClasses.h"

#include "gem/utils/GEMLogging.h"

namespace xgi {
  class Input;
  class Output;
}

namespace cgicc {
  BOOLEAN_ELEMENT(section,"section");
}

namespace gem {
  namespace base {

    class GEMFSM;
    class GEMMonitor;
    class GEMApplication;
    class GEMFSMApplication;
    
    class GEMWebApplication
    {
      friend class GEMFSM;
      friend class GEMMonitor;
      friend class GEMApplication;
      friend class GEMFSMApplication;

    public:
      GEMWebApplication(GEMApplication *gemApp)
        throw (xdaq::exception::Exception);
	
      GEMWebApplication(GEMFSMApplication *gemFSMApp)
        throw (xdaq::exception::Exception);

      ~GEMWebApplication();

    protected:
      virtual void monitorPage(xgi::Input *in, xgi::Output *out)
        throw (xgi::exception::Exception);
	
      virtual void expertPage(xgi::Input *in, xgi::Output *out)
        throw (xgi::exception::Exception);

      virtual void webRedirect(  xgi::Input *in, xgi::Output *out )
        throw (xgi::exception::Exception);

      virtual void webDefault(  xgi::Input *in, xgi::Output *out )
        throw (xgi::exception::Exception);

      // fsm specific functions, only called when the constructing app is derived from a
      // GEMFSMApplication
      virtual void webInitialize(xgi::Input *in, xgi::Output *out )
        throw (xgi::exception::Exception);
      virtual void webEnable(    xgi::Input *in, xgi::Output *out )
        throw (xgi::exception::Exception);
      virtual void webConfigure( xgi::Input *in, xgi::Output *out )
        throw (xgi::exception::Exception);
      virtual void webStart(     xgi::Input *in, xgi::Output *out )
        throw (xgi::exception::Exception);
      virtual void webPause(     xgi::Input *in, xgi::Output *out )
        throw (xgi::exception::Exception);
      virtual void webResume(    xgi::Input *in, xgi::Output *out )
        throw (xgi::exception::Exception);
      virtual void webStop(      xgi::Input *in, xgi::Output *out )
        throw (xgi::exception::Exception);
      virtual void webHalt(      xgi::Input *in, xgi::Output *out )
        throw (xgi::exception::Exception);
      virtual void webReset(     xgi::Input *in, xgi::Output *out )
        throw (xgi::exception::Exception);
	
      log4cplus::Logger m_gemLogger;
	
      GEMMonitor*        p_gemMonitor;
      GEMFSMApplication* p_gemFSMApp;
      GEMApplication*    p_gemApp;
      //xdaq::Application* p_gemApp;

      bool b_is_working, b_is_initialized, b_is_configured, b_is_running, b_is_paused;

    private:

      GEMWebApplication(GEMWebApplication const&);
      virtual void buildCfgWebpage();

    };
  } // namespace gem::base
} // namespace gem

#endif
