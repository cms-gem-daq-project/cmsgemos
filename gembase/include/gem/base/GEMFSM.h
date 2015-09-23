#ifndef gem_base_GEMFSM_h
#define gem_base_GEMFSM_h

#include <map>
#include <string>

#include "log4cplus/logger.h"

#include "toolbox/Event.h"
#include "toolbox/fsm/FiniteStateMachine.h"
#include "toolbox/lang/Class.h"
#include "xcept/Exception.h"
#include "xdaq2rc/RcmsStateNotifier.h"
#include "xoap/MessageReference.h"
#include "xcept/Exception.h"

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
    
    static const toolbox::fsm::State STATE_INITIAL     = 'I'; ///< Initial state
    static const toolbox::fsm::State STATE_HALTED      = 'H'; ///< Halted state
    static const toolbox::fsm::State STATE_CONFIGURED  = 'C'; ///< Configured state
    static const toolbox::fsm::State STATE_RUNNING     = 'E'; ///< Running (enabled, active) state
    static const toolbox::fsm::State STATE_PAUSED      = 'P'; ///< Paused state
    static const toolbox::fsm::State STATE_FAILED      = 'F'; ///< Failed state
    
    //transitional states, TCDS way seems more elegant than HCAL, but both use a similar idea
    static const toolbox::fsm::State STATE_INITIALIZING = 'i'; ///< Initializing transitional state
    static const toolbox::fsm::State STATE_CONFIGURING  = 'c'; ///< Configuring transitional state
    static const toolbox::fsm::State STATE_HALTING      = 'h'; ///< Halting transitional state
    static const toolbox::fsm::State STATE_PAUSING      = 'p'; ///< Pausing transitional state
    static const toolbox::fsm::State STATE_STOPPING     = 's'; ///< Stopping transitional state
    static const toolbox::fsm::State STATE_STARTING     = 'e'; ///< Starting transitional state
    static const toolbox::fsm::State STATE_RESUMING     = 'r'; ///< Resuming transitional state
    static const toolbox::fsm::State STATE_RESETTING    = 't'; ///< Resetting transitional state
    static const toolbox::fsm::State STATE_FIXING       = 'X'; ///< Fixing transitional state
        
    class GEMFSM : virtual public toolbox::lang::Class
      {
        friend class GEMFSMApplication;
      public:
        //HCAL states, are they useful for GEM?
        /*
          static const toolbox::fsm::State STATE_UNINIT  ='U'; ///< Uninitialized state (power on, reset, and recovery state)
          static const toolbox::fsm::State STATE_COLD    ='C'; ///< Cold initialization state (firmware reload)
          static const toolbox::fsm::State STATE_INIT    ='I'; ///< Initialization state
          static const toolbox::fsm::State STATE_PREINIT ='N'; ///< Pre-initialized state used by cards which may require two stages of configuration, where the first is fast
          static const toolbox::fsm::State STATE_WARM    ='W'; ///< WARM Initialization state
        */
        
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
        void notifyRCMS(toolbox::fsm::FiniteStateMachine& fsm, std::string const msg)
          throw(toolbox::fsm::exception::Exception);
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
        xdaq2rc::RcmsStateNotifier m_gemRCMSNotifier;
      };
    
  } // namespace gem::base
} // namespace gem

#endif
