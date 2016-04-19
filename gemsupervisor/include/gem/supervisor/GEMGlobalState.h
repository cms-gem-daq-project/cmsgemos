#ifndef gem_supervisor_GEMGlobalState_h
#define gem_supervisor_GEMGlobalState_h

#include <string>
#include <memory>

#include "toolbox/task/TimerListener.h"
#include "toolbox/fsm/FiniteStateMachine.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"
#include "gem/utils/Lock.h"

namespace toolbox {
  namespace task {
    class TimerFactory;
    class Timer;
  }
}

namespace gem {
  namespace supervisor {

    class GEMSupervisor;
    class GEMGlobalState;
    
    struct GEMApplicationState {
      GEMApplicationState();
      toolbox::fsm::State state;
      double progress;
      double progressWeight;
      std::string stateMessage;
      bool isGEMNative;
      
    private:
      friend class GEMGlobalState;
      xoap::MessageReference updateMsg;
    };

    /**
     * @class GEMGlobalState
     * @brief Computes the global GEM state depending on the state of all managed applications
     *        Idea and major implementation borrowed from hcal StateVector and hcalSupervisor
     *   Rules:
     *   - if any application is in STATE_RESETTING, global state is STATE_RESETTING
     *   - else if any application is in STATE_FAILED, global state is STATE_FAILED
     *   - else if any application is in STATE_INITIALIZING, global state is STATE_INITIALIZING
     *   - else if any application is in STATE_CONFIGURING, global state is STATE_CONFIGURING
     *   - else if any application is in STATE_HALTING, global state is STATE_HALTING
     *   - else if any application is in STATE_PAUSING, global state is STATE_PAUSING
     *   - else if any application is in STATE_STOPPING, global state is STATE_STOPPING
     *   - else if any application is in STATE_STARTING, global state is STATE_STARTING
     *   - else if any application is in STATE_RESUMING, global state is STATE_RESUMING
     *   - else if any application is in STATE_INITIAL, global state is STATE_INITIAL
     *   - else if any application is in STATE_HALTED, global state is STATE_HALTED
     *   - else if any application is in STATE_CONFIGURED, global state is STATE_CONFIGURED
     *   - else if any application is in STATE_PAUSED, global state is STATE_PAUSED
     *   - else if all applications are in STATE_RUNNING, global state is STATE_RUNNING
     */
    class GEMGlobalState : public toolbox::task::TimerListener
      {
      public:
        static const toolbox::fsm::State STATE_NULL = 0;
      
        GEMGlobalState(xdaq::ApplicationContext* context, GEMSupervisor* gemSupervisor);

        virtual ~GEMGlobalState();

        /**
         * @brief add an application to the global state calculator
         * @param the xdaq::ApplicationDescriptor pointer to the application to be added
         */
        void addApplication(xdaq::ApplicationDescriptor* app);

        /**
         * @brief clears the list of applications from the global state calculator
         */
        void clear();

        /**
         * @brief updates the global state based on the individual states of the managed applications
         */
        void update();

        /**
         * @brief starts the update timer
         */
        void startTimer();

        /**
         * @brief enables the global state object to automatically update the state
         */
        void enableAuto();

        /**
         * @brief disable the automatic update of the global state
         */
        void disableAuto();

        /**
         * @brief
         * @returns a list of the failed applications
         */
        std::string getFailedApplicationList() const;
      
        /**
         * Inherited from TimerListener, must be implemented
         * @param event
         */
        virtual void timeExpired(toolbox::task::TimerEvent& event);

        static std::string getStateName(toolbox::fsm::State state);

      protected:
        typedef std::map<xdaq::ApplicationDescriptor*,GEMApplicationState> ApplicationMap;
        typedef ApplicationMap::const_iterator app_state_const_iterator;
        
        app_state_const_iterator begin() const { return m_states.begin(); }
        app_state_const_iterator end()   const { return m_states.end();   }
      
      private:
        /**
         * @brief updates the stored application state
         * @param app is the application descriptor
         */
        void updateApplication(xdaq::ApplicationDescriptor* app);

        /**
         * @brief updates the global state based on the individual states of the managed applications
         */
        void calculateGlobals();
      
        //std::shared_ptr<toolbox::task::Timer> p_timer;
        //std::shared_ptr<GEMSupervisor>        p_gemSupervisor;
        toolbox::task::Timer* p_timer;
        GEMSupervisor*        p_gemSupervisor;

        ApplicationMap m_states;
        xdaq::ApplicationContext*    p_appContext;
        xdaq::ApplicationDescriptor* p_srcApp;

        toolbox::fsm::State m_globalState, m_forceGlobal;
        log4cplus::Logger m_gemLogger;
        mutable gem::utils::Lock m_mutex;
      };
  } //end namespace supervisor
} //end namespace gem
#endif
