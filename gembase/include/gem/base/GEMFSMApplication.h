/** @file GEMFSMApplication.h */

#ifndef GEM_BASE_GEMFSMAPPLICATION_H
#define GEM_BASE_GEMFSMAPPLICATION_H

#include "toolbox/task/exception/Exception.h"

#include "gem/base/GEMApplication.h"
#include "gem/base/GEMFSM.h"

#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

namespace toolbox {
  namespace task {
    class WorkLoop;
    class ActionSignature;
  }
}

namespace gem {
  namespace base {

    class GEMFSM;
    class GEMApplication;
    class GEMWebApplication;

    class GEMFSMApplication : public GEMApplication
    {
    public:

      friend class GEMFSM;
      friend class GEMWebApplication;

      GEMFSMApplication(xdaq::ApplicationStub *stub)
        throw (xdaq::exception::Exception);

      virtual ~GEMFSMApplication();

    protected:
      /**
       * @brief Forwards a web received Initialize command to the FSM via a SOAP message
       */
      void xgiInitialize(xgi::Input *in, xgi::Output *out);

      /**
       * @brief Forwards a web received Configure command to the FSM via a SOAP message
       */
      void xgiConfigure(xgi::Input *in, xgi::Output *out);

      /**
       * @brief Forwards a web received Start command to the FSM via a SOAP message
       */
      void xgiStart(xgi::Input *in, xgi::Output *out);

      /**
       * @brief Forwards a web received Pause command to the FSM via a SOAP message
       */
      void xgiPause(xgi::Input *in, xgi::Output *out);

      /**
       * @brief Forwards a web received Resume command to the FSM via a SOAP message
       */
      void xgiResume(xgi::Input *in, xgi::Output *out);

      /**
       * @brief Forwards a web received Stop command to the FSM via a SOAP message
       */
      void xgiStop(xgi::Input *in, xgi::Output *out);

      /**
       * @brief Forwards a web received Halt command to the FSM via a SOAP message
       */
      void xgiHalt(xgi::Input *in, xgi::Output *out);

      /**
       * @brief Forwards a web received Reset command to the FSM via a SOAP message
       */
      void xgiReset(xgi::Input *in, xgi::Output *out);

      /**
       * @brief
       */
      void jsonStateUpdate(xgi::Input *in, xgi::Output *out);

      /**
       * @brief Handles the state transitions forwarding to the appropriate workloop task
       * @param command Event type to execute
       */
      void workloopDriver(std::string const& command);

      /**
       * @brief Workloop task bound to the initialize action
       * @details On reciept of an initialize command, the workloop driver calls this task
       *              This task then calls initializeAction
       * @param wl pointer to a toolbox::task::WorkLoop executing the action
       * @returns false (do not resubmit workloop)
       */
      bool initialize(toolbox::task::WorkLoop *wl);

      /**
       * @brief Workloop task bound to the configure action
       * @details On reciept of a configure command, the workloop driver calls this task
       *              This task then calls configureAction
       * @param wl pointer to a toolbox::task::WorkLoop executing the action
       * @returns false (do not resubmit workloop)
       */
      bool configure(toolbox::task::WorkLoop *wl);

      /**
       * @brief Workloop task bound to the start action
       * @details On reciept of a start command, the workloop driver calls this task
       *              This task then calls startAction
       * @param wl pointer to a toolbox::task::WorkLoop executing the action
       * @returns false (do not resubmit workloop)
       */
      bool start(toolbox::task::WorkLoop *wl);

      /**
       * @brief Workloop task bound to the pause action
       * @details On reciept of a pause command, the workloop driver calls this task
       *              This task then calls pauseAction
       * @param wl pointer to a toolbox::task::WorkLoop executing the action
       * @returns false (do not resubmit workloop)
       */
      bool pause(toolbox::task::WorkLoop *wl);

      /**
       * @brief Workloop task bound to the resme action
       * @details On reciept of a resume command, the workloop driver calls this task
       *              This task then calls resmeAction
       * @param wl pointer to a toolbox::task::WorkLoop executing the action
       * @returns false (do not resubmit workloop)
       */
      bool resume(toolbox::task::WorkLoop *wl);

      /**
       * @brief Workloop task bound to the stop action
       * @details On reciept of a stop command, the workloop driver calls this task
       *              This task then calls stopAction
       * @param wl pointer to a toolbox::task::WorkLoop executing the action
       * @returns false (do not resubmit workloop)
       */
      bool stop(toolbox::task::WorkLoop *wl);

      /**
       * @brief Workloop task bound to the halt action
       * @details On reciept of a halt command, the workloop driver calls this task
       *              This task then calls haltAction
       * @param wl pointer to a toolbox::task::WorkLoop executing the action
       * @returns false (do not resubmit workloop)
       */
      bool halt(toolbox::task::WorkLoop *wl);

      /**
       * @brief Workloop task bound to the reset action
       * @details On reciept of a reset command, the workloop driver calls this task
       *              This task then calls resetAction
       * @param wl pointer to a toolbox::task::WorkLoop executing the action
       * @returns false (do not resubmit workloop)
       */
      bool reset(toolbox::task::WorkLoop *wl);
      // bool noAction(toolbox::task::WorkLoop *wl) { return false; };
      // bool fail(toolbox::task::WorkLoop *wl) { return false; };

      /**
       * @brief Workloop task bound to the calibration action
       * @details On reciept of a calibration command, the workloop driver calls this task
       *              This task then calls calibrationAction
       * @param wl pointer to a toolbox::task::WorkLoop executing the action
       * @returns false (do not resubmit workloop)
       */
      bool calibrationAction(toolbox::task::WorkLoop *wl) { return false; };

      /**
       * @brief Workloop task bound to the calibrationSequence action
       * @details On reciept of a calibrationSequence command, the workloop driver calls this task
       *              This task then calls calibrationSequenceAction
       * @param wl pointer to a toolbox::task::WorkLoop executing the action
       * @returns false (do not resubmit workloop)
       */
      bool calibrationSequencer(toolbox::task::WorkLoop *wl) { return false; };

      /**
       * @defgroup StateTransitions GEMFSM State Transitions
       * @details Defines the behaviour of the application for each state transition
       *              most will be pure virtual to enforce derived application specific
       *              implementations, common implementations will be merged into the base
       *              application
       */
      /**
       * @brief Initialize state transition
       * @ingroup StateTransitions
       * @details Defines the behaviour of the application to go from Initial to Halted
       */
      virtual void initializeAction()=0;

      /**
       * @brief Configure state transition
       * @ingroup StateTransitions
       * @details Defines the behaviour of the application to go from Halted to Configured
       */
      virtual void configureAction()=0;

      /**
       * @brief Start state transition
       * @ingroup StateTransitions
       * @details Defines the behaviour of the application to go from Configured to Running
       */
      virtual void startAction()=0;

      /**
       * @brief Pause state transition
       * @ingroup StateTransitions
       * @details Defines the behaviour of the application to go from Running to Paused
       */
      virtual void pauseAction()=0;

      /**
       * @brief Resume state transition
       * @ingroup StateTransitions
       * @details Defines the behaviour of the application to go from Paused to Running
       */
      virtual void resumeAction()=0;

      /**
       * @brief Stop state transition
       * @ingroup StateTransitions
       * @details Defines the behaviour of the application to go from Running to Configured
       */
      virtual void stopAction()=0;

      /**
       * @brief Halt state transition
       * @ingroup StateTransitions
       * @details Defines the behaviour of the application to go to Halted
       */
      virtual void haltAction()=0;

      /**
       * @brief Reset state transition
       * @ingroup StateTransitions
       * @details Defines the behaviour of the application to go to Initial
       */
      virtual void resetAction()=0;
      // virtual void noAction()=0;

      /**
       * @brief Responses to xdata::Event and selects the appropriate transtion
       * @details The drive calls the workloop driver with the approprate transition
       * @param event is a toolbox::Event::Reference of the event to pass to the GEMFSM
       */
      void transitionDriver(toolbox::Event::Reference event);

      /**
       * @brief Action in response to a Resetevent
       * @details Takes the GEMFSM from a state to the uninitialzed state
       *              recovery from a failed transition, or just a reset
       * @param event is a toolbox::Event::Reference of the event to pass to the GEMFSM
       */
      virtual void resetAction(toolbox::Event::Reference event);

      /**
       * failAction
       * determines how to handle a failed transition
       *
       virtual void failAction(toolbox::Event::Reference e)
      */

      /**
       * @brief Defines behaviour of a change in the FSM
       * @param fsm reference to the toolbox::fsm::FiniteStateMachine
       */
      virtual void stateChanged(toolbox::fsm::FiniteStateMachine &fsm);

      /**
       * @brief Handles a failed transition
       * @param event is a toolbox::Event::Reference of the event to pass to the GEMFSM
       */
      virtual void transitionFailed(toolbox::Event::Reference event);

      /**
       * @brief Forwards a state change to the GEMFSM object
       * @param event name of the event to pass to the GEMFSM
       */
      virtual void fireEvent(std::string event);

      /**
       * @brief changeState
       * Forwards a state change to the GEMFSM object
       * @param xoap::MessageReference msg message containing the state transition
       * @returns xoap::MessageReference response of the SOAP transaction
       */
      virtual xoap::MessageReference changeState(xoap::MessageReference msg);

      std::string workLoopName;

      toolbox::task::ActionSignature* m_initSig  ;  ///< Command driver signature to trigger the initialize action
      toolbox::task::ActionSignature* m_confSig  ;  ///< Command driver signature to trigger the configure action
      toolbox::task::ActionSignature* m_startSig ;  ///< Command driver signature to trigger the start action
      toolbox::task::ActionSignature* m_stopSig  ;  ///< Command driver signature to trigger the stop action
      toolbox::task::ActionSignature* m_pauseSig ;  ///< Command driver signature to trigger the pause action
      toolbox::task::ActionSignature* m_resumeSig;  ///< Command driver signature to trigger the resume action
      toolbox::task::ActionSignature* m_haltSig  ;  ///< Command driver signature to trigger the halt action
      toolbox::task::ActionSignature* m_resetSig ;  ///< Command driver signature to trigger the reset action

    public:
      // FIXME is it a problem to make this public?
      /**
       * getCurrentState
       * @returns std::string name of the current state of the GEMFSM object
       *
       virtual std::string getCurrentState() const {return m_gemfsm.getCurrentState();};
      */

      /**
       * @brief
       */
      virtual std::string getCurrentState() {
        updateState();
        return m_stateName.toString();
      };

      virtual toolbox::fsm::State getCurrentFSMState() {
        return m_gemfsm.getCurrentFSMState();
      };

      /**
       * @brief Returns a pointer to the application state infospace tool box
       * @returns
       */
      std::shared_ptr<utils::GEMInfoSpaceToolBox> getAppStateISToolBox() { return p_appStateInfoSpaceToolBox; };

    private:
      GEMFSM m_gemfsm;  ///< GEMFSM state machine object

      bool b_accept_web_commands;  ///< should we allow state transition commands from the web interface

      toolbox::BSem m_wl_semaphore;      ///< do we need a semaphore for the workloop?
      toolbox::BSem m_db_semaphore;      ///< do we need a semaphore for the database?
      toolbox::BSem m_cfg_semaphore;     ///< do we need a semaphore for the config file?
      toolbox::BSem m_web_semaphore;     ///< do we need a semaphore for the web access?
      toolbox::BSem m_infspc_semaphore;  ///< do we need a semaphore for the infospace?

      /* toolbox::task::WorkLoop *p_wl;  ///< */

    protected:
      /**
       * @brief updateState
       */
      virtual void updateState() { m_stateName = m_gemfsm.getCurrentState(); };

      std::shared_ptr<utils::GEMInfoSpaceToolBox> p_appStateInfoSpaceToolBox;  ///<

      xdata::InfoSpace* p_appStateInfoSpace;  ///<

      xdata::Double  m_progress;      ///<
      xdata::String  m_stateName;     ///<
      xdata::String  m_stateMessage;  ///<
    };
  }  // namespace gem::base
}  // namespace gem

#endif  // GEM_BASE_GEMFSMAPPLICATION_H
