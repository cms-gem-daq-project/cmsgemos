#ifndef gem_supervisor_GEMSupervisor_h
#define gem_supervisor_GEMSupervisor_h

#include "xdaq/Application.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"

#include "uhal/uhal.hpp"

#include "gem/base/GEMFSMApplication.h"

#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

namespace gem {
  namespace supervisor {
    
    class GEMSupervisor : public gem::base::GEMFSMApplication
    {
    public:
      XDAQ_INSTANTIATOR();

      GEMSupervisor(xdaq::ApplicationStub* s);

      virtual ~GEMSupervisor();
      
      void init();

      void actionPerformed(xdata::Event& event);

    protected:
      //state transitions
      virtual void initializeAction(toolbox::Event::Reference e);
      virtual void enableAction(    toolbox::Event::Reference e);
      virtual void configureAction( toolbox::Event::Reference e);
      virtual void startAction(     toolbox::Event::Reference e);
      virtual void pauseAction(     toolbox::Event::Reference e);
      virtual void resumeAction(    toolbox::Event::Reference e);
      virtual void stopAction(      toolbox::Event::Reference e);
      virtual void haltAction(      toolbox::Event::Reference e);
      virtual void noAction(        toolbox::Event::Reference e); 
      virtual void failAction(      toolbox::Event::Reference e); 
      
      virtual void resetAction()//toolbox::Event::Reference e)
        throw (toolbox::fsm::exception::Exception);
      
      virtual void stateChanged(    toolbox::fsm::FiniteStateMachine &fsm)
        throw (toolbox::fsm::exception::Exception);
      virtual void transitionFailed(toolbox::Event::Reference event)
        throw (toolbox::fsm::exception::Exception);
      
      virtual void fireEvent(std::string event)
        throw (toolbox::fsm::exception::Exception);
      
      virtual xoap::MessageReference changeState(xoap::MessageReference msg);
      
    private:
      mutable gem::utils::Lock deviceLock_;
      
      
    };
  } //end namespace supervisor
} //end namespace gem
#endif
