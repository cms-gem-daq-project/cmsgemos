#ifndef gem_supervisor_GEMSupervisor_h
#define gem_supervisor_GEMSupervisor_h

#include "uhal/uhal.hpp"

#include "gem/base/GEMFSMApplication.h"
#include "gem/supervisor/GEMSupervisorWeb.h"
#include "gem/supervisor/exception/Exception.h"

#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

namespace gem {
  namespace supervisor {
    
    class GEMSupervisorWeb;
    class GEMSupervisorMonitor;
    
    class GEMSupervisor : public gem::base::GEMFSMApplication
      {

        friend class GEMSupervisorWeb;
        friend class GEMSupervisorMonitor;

      public:
        XDAQ_INSTANTIATOR();

        GEMSupervisor(xdaq::ApplicationStub* s);

        virtual ~GEMSupervisor();
      
        virtual void init();

        virtual void actionPerformed(xdata::Event& event);

      protected:
        /* virtual bool calibrationAction(toolbox::task::WorkLoop *wl); */
        /* virtual bool calibrationSequencer(toolbox::task::WorkLoop *wl); */

        //state transitions
        virtual void initializeAction() throw (gem::supervisor::exception::Exception);
        virtual void configureAction()  throw (gem::supervisor::exception::Exception);
        virtual void startAction()      throw (gem::supervisor::exception::Exception);
        virtual void pauseAction()      throw (gem::supervisor::exception::Exception);
        virtual void resumeAction()     throw (gem::supervisor::exception::Exception);
        virtual void stopAction()       throw (gem::supervisor::exception::Exception);
        virtual void haltAction()       throw (gem::supervisor::exception::Exception);
        virtual void resetAction()      throw (gem::supervisor::exception::Exception);
        //virtual void noAction()         throw (gem::supervisor::exception::Exception); 
      
        virtual void failAction(toolbox::Event::Reference e)
          throw (toolbox::fsm::exception::Exception); 
      
        virtual void resetAction(toolbox::Event::Reference e)
          throw (toolbox::fsm::exception::Exception);
      
        std::vector<xdaq::ApplicationDescriptor*> getSupervisedAppDescriptors() {
          return v_supervisedApps; };
        
      private:
        bool isGEMApplication(const std::string& classname) const;
        bool manageApplication(const std::string& classname) const;

        std::shared_ptr<GEMSupervisorMonitor> m_supervisorMonitor;
        
        mutable gem::utils::Lock deviceLock_;
        std::vector<xdaq::ApplicationDescriptor*> v_supervisedApps;
        xdaq::ApplicationDescriptor* readoutApp;
      
      };
  } //end namespace supervisor
} //end namespace gem
#endif
