#ifndef gem_base_GEMWebApplication_h
#define gem_base_GEMWebApplication_h

#include "xdaq/WebApplication.h"

#include "xgi/framework/Method.h"
#include "xgi/framework/UIManager.h"

#include "gem/base/GEMApplication.h"

#include "cgicc/HTMLClasses.h"

namespace cgicc {
  BOOLEAN_ELEMENT(section,"section");
}

namespace gem {
  namespace base {
    
    class GEMWebApplication : public gem::base::GEMApplication, public xgi::framework::UIManager
      {
      public:
	XDAQ_INSTANTIATOR();
	
	GEMWebApplication(xdaq::ApplicationStub *stub)
	  throw (xdaq::exception::Exception);
	
      protected:
	void Default(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);

	// hyperdaq action callbacks
	void webEnable(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	void webConfigure(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	void webStart(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	void webPause(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	void webResume(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	void webStop(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	void webHalt(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	void webReset(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);

	void webRedirect(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
      };
  } // namespace gem::base
} // namespace gem

#endif
