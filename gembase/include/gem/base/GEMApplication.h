#ifndef gem_base_GEMApplication_h
#define gem_base_GEMApplication_h

#include <cstdlib>
#include "string.h"
#include "limits.h"
#include <string>
#include <string>
#include <deque>
#include <map>

#include "xdaq/Application.h"

#include "log4cplus/logger.h"

#include "toolbox/BSem.h"
#include "toolbox/task/WorkLoop.h"
#include "toolbox/fsm/FiniteStateMachine.h"
#include "toolbox/mem/MemoryPoolFactory.h"

#include "xdata/Bag.h"
#include "xdata/Vector.h"
#include "xdata/InfoSpace.h"
#include "xdata/ActionListener.h"

#include "xdata/Float.h"
#include "xdata/String.h"
#include "xdata/Boolean.h"
#include "xdata/Integer.h"
#include "xdata/Integer64.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger.h"

#include "xoap/MessageReference.h"

#include "i2o/utils/AddressMap.h"

#include "xdaq2rc/RcmsStateNotifier.h"

namespace gem {
  namespace hw {
    class GEMHwDevice;
  }
}

namespace gem {
  namespace base {
    
    class GEMWebApplication;
    //class ConfigurationInfoSpaceHandler;
    //class Monitor;
    //class WebServer;
    
    class GEMApplication : public xdaq::Application, public xdata::ActionListener
      {
      public:
	GEMApplication(xdaq::ApplicationStub *stub);
	
	virtual void init() = 0;
	
	virtual void actionPerformed(xdata::Event& event);
	
      protected:
	gem::hw::GEMHwDevice* gemHWP_;
	log4cplus::Logger gemlogger_;

	//virtual ConfigurationInfoSpaceHandler* getCfgInfoSpace() const;
	virtual GEMHwDevice* getHw() const;
	//virtual Monitor* getMonitor() const;

      private:

	/**** application properties ****/
	i2o::utils::AddressMap *i2oAddressMap_;
	toolbox::mem::MemoryPoolFactory *poolFactory_;
	
	xdata::InfoSpace *appInfoSpace_;

        xdaq::ApplicationDescriptor *appDescriptor_;
	xdaq::ApplicationContext    *appContext_;
	xdaq::ApplicationGroup      *appGroup_;

	std::string xmlClass_;
	unsigned long instance_;
	std::string urn_;	
	

	xdata::String          run_type_;
	xdata::UnsignedInteger run_number_;
	xdata::UnsignedInteger runSequenceNumber_;

	xdata::Integer64 nevents_;

        xdaq2rc::RcmsStateNotifier rcmsStateNotifier_;

	toolbox::BSem wl_semaphore_;

	toolbox::task::WorkLoop *wl_;

	toolbox::task::ActionSignature *enable_sig_, *configure_sig_, *initialize_sig_;
	toolbox::task::ActionSignature *start_sig_,  *pause_sig_, *resume_sig_;
	toolbox::task::ActionSignature *stop_sig_,   *halt_sig_;

      };
    
  } // namespace gem::base
} // namespace gem

#endif
