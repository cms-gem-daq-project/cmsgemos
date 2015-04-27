#ifndef gem_hw_glib_GLIBManagerWeb_h
#define gem_hw_glib_GLIBManagerWeb_h

#include "gem/base/GEMWebApplication.h"

namespace gem {
  namespace hw {
    namespace glib {
      
      class GLIBManager;

      class GLIBManagerWeb : public gem::base::GEMWebApplication
	{
	  //friend class GLIBMonitor;
	  //friend class GLIBManager;

	public:
	  GLIBManagerWeb(GLIBManager *glibApp);
	  
	  ~GLIBManagerWeb();

	protected:
	  void monitorPage(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	
	  void expertPage(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);

	  //FSM web callbacks
	  void webInitialize(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void webEnable(    xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void webConfigure( xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void webStart(     xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void webPause(     xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void webResume(    xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void webStop(      xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void webHalt(      xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void webReset(     xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  
	  void webRedirect(  xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  
	
	private:
	  
	  //GLIBManagerWeb(GLIBManagerWeb const&);
	  
	};
    } // namespace gem::glib
  } // namespace gem::hw
} // namespace gem

#endif
