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
    
    //class GEMWebApplication : virtual public gem::base::GEMApplication, public xdaq::WebApplication
    class GEMWebApplication : virtual public xdaq::WebApplication
      {
      public:
	//XDAQ_INSTANTIATOR();
	
	GEMWebApplication(xdaq::ApplicationStub *stub)
	  throw (xdaq::exception::Exception);
	
      protected:
	virtual void init();
	log4cplus::Logger gemWebLogger_;

	virtual void Default(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	
	virtual void Expert(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);

	virtual void webRedirect(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
      };
  } // namespace gem::base
} // namespace gem

#endif
