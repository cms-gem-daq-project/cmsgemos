#ifndef gem_hw_glib_GLIBManager_h
#define gem_hw_glib_GLIBManager_h

#include <string>

#include "xdaq/WebApplication.h"

#include "xdata/String.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"

#include "uhal/uhal.hpp"

#include "xgi/framework/Method.h"
#include "cgicc/HTMLClasses.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "gem/hw/glib/GLIBSettings.h"

//typedef uhal::exception::exception uhalException;

#define MAX_GLIBS_PER_CRATE 12

namespace cgicc {
  BOOLEAN_ELEMENT(section,"section");
}

namespace gem {
  namespace base {
    class GEMApplication;
    class GEMFSM;
    class GEMFSMApplication;
    class GEMWebApplication;
  }
  
  namespace hw {
    namespace glib {
      class HwGLIB;
      
      //class GLIBManager: public gem::base::GEMWebApplication, public gem::base::GEMFSMApplication
      class GLIBManager: public xdaq::WebApplication, public xdata::ActionListener
	{
	  
	public:
	  XDAQ_INSTANTIATOR();
	  
	  GLIBManager(xdaq::ApplicationStub * s)
	    throw (xdaq::exception::Exception);

	  ~GLIBManager();
	  
	  gem::hw::glib::HwGLIB* m_GLIBs[MAX_GLIBS_PER_CRATE];
	  
	}; //end class GLIBManager

    }//end namespace gem::hw::glib
    
  }//end namespace gem::hw
  
}//end namespace gem

#endif
