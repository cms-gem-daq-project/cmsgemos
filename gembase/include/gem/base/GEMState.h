#ifndef gem_base_GEMState_h
#define gem_base_GEMState_h

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

//Basic implementation copied from HCAL code

namespace toolbox {
  namespace fsm {
    class AsynchronousFiniteStateMachine;
  }
}

namespace gem {
  namespace base {

    class GEMState : virtual public toolbox::lang::Class, virtual public ::toolbox::task::TimerListener
      {
      public:
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


        /** \brief Global state from all the internal states of the GEM applications
            Idea taken from hcalSupervisor and StateVector
         Rules:
         - if any application is in STATE_RESETTING, global state is STATE_RESETTING
         - else if any application is in STATE_FAILED, global state is STATE_FAILED
         - else if any application is in STATE_INITIALIZING, global state is STATE_INITIALIZING
         - else if any application is in STATE_CONFIGURING, global state is STATE_CONFIGURING
         - else if any application is in STATE_HALTING, global state is STATE_HALTING
         - else if any application is in STATE_PAUSING, global state is STATE_PAUSING
         - else if any application is in STATE_STOPPING, global state is STATE_STOPPING
         - else if any application is in STATE_STARTING, global state is STATE_STARTING
         - else if any application is in STATE_RESUMING, global state is STATE_RESUMING
         - else if any application is in STATE_INITIAL, global state is STATE_INITIAL
         - else if any application is in STATE_HALTED, global state is STATE_HALTED
         - else if any application is in STATE_CONFIGURED, global state is STATE_CONFIGURED
         - else if any application is in STATE_PAUSED, global state is STATE_PAUSED
         - else if all applications are in STATERUNNING, global state is STATE_RUNNING
        */

        
        GEMState(xdaq::ApplicationContext* context, GEMFSMApplication* supervisor);
        virtual ~GEMState();
	
        xoap::MessageReference changeState(xoap::MessageReference msg);
	
        std::string getCurrentState() const;
        std::string getStateName(toolbox::fsm::State const& state) const;
	

      protected:
	
      private:

        xdata::String m_gemGlobalState;
        xdata::String m_reasonForFailure;

        log4cplus::Logger m_gemLogger;
        std::map<xdaq::ApplicationDescriptor*, GEMApplicationState> m_appStateMap;
      };
    
  } // namespace gem::base
} // namespace gem

#endif
