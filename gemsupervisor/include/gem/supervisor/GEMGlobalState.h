#ifndef gem_supervisor_GEMGlobalState_h
#define gem_supervisor_GEMGlobalState_h

#include "toolbox/task/TimerListener.h"
#include <string>

namespace toolbox {
  namespace task {
    class Timer;
  }
}

namespace gem {
  namespace supervisor {

    class GEMSupervisor;

    /**
     * @class GEMGlobalState
     * @brief Computes the global GEM state depending on the state of all managed applications
     *        Idea and major implementation borrowed from hcal StateVector and hcalSupervisor
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

      //void ttcPausedIsReady(bool setit=true) { m_ttcPausedIsReady=setit; }

      /**
       * @brief
       * @returns a list of the failed applications
       */
      std::string getFailedApplicationList() const;
      

    protected:


    private:
      
    }
  } //end namespace supervisor
} //end namespace gem
#endif
