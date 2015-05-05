#ifndef gem_supervisor_GEMSupervisor_h
#define gem_supervisor_GEMSupervisor_h

#include "xdaq/Application.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"

#include "uhal/uhal.hpp"

#include "gem/base/GEMFSMApplication.h"
#include "gem/supervisor/GEMSupervisorWeb.h"

#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

namespace gem {
  namespace supervisor {
    
    class GEMSupervisorWeb;
    
    class GEMSupervisor : public gem::base::GEMFSMApplication
    {
      friend class GEMSupervisorWeb;
    public:
      XDAQ_INSTANTIATOR();

      GEMSupervisor(xdaq::ApplicationStub* s);

      virtual ~GEMSupervisor();
      
      void init();

      void actionPerformed(xdata::Event& event);

    protected:
      //virtual void xgiDefault(   xgi::Input *in, xgi::Output *out );
      
      //state transitions
      virtual void initializeAction();
      virtual void enableAction(    );
      virtual void configureAction( );
      virtual void startAction(     );
      virtual void pauseAction(     );
      virtual void resumeAction(    );
      virtual void stopAction(      );
      virtual void haltAction(      );
      virtual void noAction(        ); 
      
      virtual void failAction(toolbox::Event::Reference e)
	throw (toolbox::fsm::exception::Exception); 
      
      virtual void resetAction(toolbox::Event::Reference e)
	throw (toolbox::fsm::exception::Exception);
      
    private:
      mutable gem::utils::Lock deviceLock_;
      
      
    };
  } //end namespace supervisor
} //end namespace gem
#endif
