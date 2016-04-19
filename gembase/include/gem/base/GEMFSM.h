#ifndef gem_base_GEMFSM_h
#define gem_base_GEMFSM_h

#include <map>
#include <string>

#include "log4cplus/logger.h"

#include "toolbox/Event.h"
#include "toolbox/fsm/FiniteStateMachine.h"
#include "toolbox/lang/Class.h"
#include "xcept/Exception.h"
//#include "xdaq2rc/RcmsStateNotifier.h"
#include "xoap/MessageReference.h"
#include "xcept/Exception.h"

#include "gem/base/GEMState.h"

//Basic implementation copied from TCDS code

namespace toolbox {
  namespace fsm {
    class AsynchronousFiniteStateMachine;
  }
}

namespace gem {
  namespace base {

    class GEMWebApplication;
    class GEMFSMApplication;
    
    class GEMFSM : virtual public toolbox::lang::Class
      {
        friend class GEMFSMApplication;
      public:
        
        GEMFSM(GEMFSMApplication* const gemAppP);//,
        //gem::base::utils::ApplicationStateInfoSpaceHandler* const infoSpaceHandlerP);
        virtual ~GEMFSM();
	
        void fireEvent(::toolbox::Event::Reference const &event);
	
        xoap::MessageReference changeState(xoap::MessageReference msg);
	
        std::string getCurrentState() const;
        std::string getStateName(toolbox::fsm::State const& state) const;
	
        /*may not need (all of) these, no need to just blindly copy TCDS :-)*/
        //void configureAndEnable();
        void gotoFailed(std::string const reason="No further information available");
        void gotoFailed(xcept::Exception& err);
        void gotoFailedAsynchronously(xcept::Exception& err);

      protected:
        /*void notifyRCMS(toolbox::fsm::FiniteStateMachine& fsm, std::string const msg)
        throw(toolbox::fsm::exception::Exception);*/
        void stateChanged(toolbox::fsm::FiniteStateMachine& fsm)
          throw(toolbox::fsm::exception::Exception);
        //void stateChangedWithNotification(toolbox::fsm::FiniteStateMachine& fsm);
        //void stateChangedToFailedWithNotification(toolbox::fsm::FiniteStateMachine& fsm);
	
        void invalidAction(toolbox::Event::Reference event);
	
      private:
        toolbox::fsm::AsynchronousFiniteStateMachine* p_gemfsm;
        xdata::InfoSpace *p_appInfoSpace;
        xdata::InfoSpace *p_appStateInfoSpace;

        xdata::String m_gemFSMState;
        xdata::String m_reasonForFailure;

        GEMFSMApplication* p_gemApp;
        log4cplus::Logger m_gemLogger;
        std::map<std::string, std::string> lookupMap_;
        //xdaq2rc::RcmsStateNotifier m_gemRCMSNotifier;
      };
    
  } // namespace gem::base
} // namespace gem

#endif
