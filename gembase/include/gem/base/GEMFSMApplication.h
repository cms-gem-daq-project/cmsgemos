#ifndef GEM_BASE_GEMFSMAPPLICATION_H
#define GEM_BASE_GEMFSMAPPLICATION_H

#include "toolbox/task/exception/Exception.h"

#include "gem/base/GEMApplication.h"
#include "gem/base/GEMFSM.h"

#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

namespace toolbox {
  namespace task{
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
      static const unsigned MAX_AMCS_PER_CRATE      = 12;  ///<
      static const unsigned MAX_OPTOHYBRIDS_PER_AMC = 2;   ///<
      static const unsigned MAX_VFATS_PER_GEB       = 24;  ///<

      friend class GEMFSM;
      friend class GEMWebApplication;

      GEMFSMApplication(xdaq::ApplicationStub *stub)
        throw (xdaq::exception::Exception);

      virtual ~GEMFSMApplication();

    protected:
      /*
        xgi interfaces, should be treated the same way as a command recieved via SOAP,
        or should maybe create a SOAP message and send it to the application
        basically, should be no difference when receiving a command through the web
        interface or from the function manager or via SOAP
        make non virtual so as to not re-implement in derived classes
      */
      /**
       * @brief
       */
      void xgiInitialize(xgi::Input *in, xgi::Output *out);

      /**
       * @brief
       */
      void xgiConfigure(xgi::Input *in, xgi::Output *out);

      /**
       * @brief
       */
      void xgiStart(xgi::Input *in, xgi::Output *out);

      /**
       * @brief
       */
      void xgiPause(xgi::Input *in, xgi::Output *out);

      /**
       * @brief
       */
      void xgiResume(xgi::Input *in, xgi::Output *out);

      /**
       * @brief
       */
      void xgiStop(xgi::Input *in, xgi::Output *out);

      /**
       * @brief
       */
      void xgiHalt(xgi::Input *in, xgi::Output *out);

      /**
       * @brief
       */
      void xgiReset(xgi::Input *in, xgi::Output *out);

      /**
       * @brief
       */
      void jsonStateUpdate(xgi::Input *in, xgi::Output *out);

      // directs commands to the proper workloop
      /**
       * @brief
       */
      void workloopDriver(std::string const& command)
        throw (toolbox::task::exception::Exception);

      // work loop call-back functions
      /**
       * @brief
       */
      bool initialize(toolbox::task::WorkLoop *wl);

      /**
       * @brief
       */
      bool configure(toolbox::task::WorkLoop *wl);

      /**
       * @brief
       */
      bool start(toolbox::task::WorkLoop *wl);

      /**
       * @brief
       */
      bool pause(toolbox::task::WorkLoop *wl);

      /**
       * @brief
       */
      bool resume(toolbox::task::WorkLoop *wl);

      /**
       * @brief
       */
      bool stop(toolbox::task::WorkLoop *wl);

      /**
       * @brief
       */
      bool halt(toolbox::task::WorkLoop *wl);

      /**
       * @brief
       */
      bool reset(toolbox::task::WorkLoop *wl);
      // bool noAction(toolbox::task::WorkLoop *wl) { return false; };
      // bool fail(toolbox::task::WorkLoop *wl) { return false; };

      /**
       * @brief
       */
      bool calibrationAction(toolbox::task::WorkLoop *wl) { return false; };

      /**
       * @brief
       */
      bool calibrationSequencer(toolbox::task::WorkLoop *wl) { return false; };

      /* state transitions
       * defines the behaviour of the application for each state transition
       * most will be pure virtual to enforce derived application specific
       * implementations, common implementations will be merged into the base
       * application
       */
      /**
       * @brief
       */
      virtual void initializeAction() /*throw (gem::base::exception::Exception)*/ = 0;

      /**
       * @brief
       */
      virtual void configureAction()  /*throw (gem::base::exception::Exception)*/ = 0;

      /**
       * @brief
       */
      virtual void startAction()      /*throw (gem::base::exception::Exception)*/ = 0;

      /**
       * @brief
       */
      virtual void pauseAction()      /*throw (gem::base::exception::Exception)*/ = 0;

      /**
       * @brief
       */
      virtual void resumeAction()     /*throw (gem::base::exception::Exception)*/ = 0;

      /**
       * @brief
       */
      virtual void stopAction()       /*throw (gem::base::exception::Exception)*/ = 0;

      /**
       * @brief
       */
      virtual void haltAction()       /*throw (gem::base::exception::Exception)*/ = 0;

      /**
       * @brief
       */
      virtual void resetAction()      /*throw (gem::base::exception::Exception)*/ = 0;
      // virtual void noAction()         /*throw (gem::base::exception::Exception)*/ = 0;

      /**
       * @brief Responses to xdata::Event
       */
      void transitionDriver(toolbox::Event::Reference e)
        throw (toolbox::fsm::exception::Exception);

      /**
       * @brief resetAction
       * takes the GEMFSM from a state to the uninitialzed state
       * recovery from a failed transition, or just a reset
       */
      virtual void resetAction(toolbox::Event::Reference e)
        throw (toolbox::fsm::exception::Exception);

      /**
       * failAction
       * determines how to handle a failed transition
       *
       virtual void failAction(toolbox::Event::Reference e)
       throw (toolbox::fsm::exception::Exception);
      */

      /**
       * @brief stateChanged
       *
       *
       */
      virtual void stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
        throw (toolbox::fsm::exception::Exception);

      /**
       * @brief transitionFailed
       */
      virtual void transitionFailed(toolbox::Event::Reference event)
        throw (toolbox::fsm::exception::Exception);

      /**
       * @brief fireEvent
       * Forwards a state change to the GEMFSM object
       * @param std::string event name of the event to pass to the GEMFSM
       */
      virtual void fireEvent(std::string event)
        throw (toolbox::fsm::exception::Exception);

      /**
       * @brief changeState
       * Forwards a state change to the GEMFSM object
       * @param xoap::MessageReference msg message containing the state transition
       * @returns xoap::MessageReference response of the SOAP transaction
       */
      virtual xoap::MessageReference changeState(xoap::MessageReference msg);

      std::string workLoopName;

      toolbox::task::ActionSignature* initSig_  ;  ///<
      toolbox::task::ActionSignature* confSig_  ;  ///<
      toolbox::task::ActionSignature* startSig_ ;  ///<
      toolbox::task::ActionSignature* stopSig_  ;  ///<
      toolbox::task::ActionSignature* pauseSig_ ;  ///<
      toolbox::task::ActionSignature* resumeSig_;  ///<
      toolbox::task::ActionSignature* haltSig_  ;  ///<
      toolbox::task::ActionSignature* resetSig_ ;  ///<

    public:
      //is it a problem to make this public?
      /* getCurrentState
       * @returns std::string name of the current state of the GEMFSM object
       *
       virtual std::string getCurrentState() const {
       return m_gemfsm.getCurrentState();
       };*/
      /**
       * @brief
       */
      virtual std::string getCurrentState() {
        return m_stateName.toString();
      };

      /**
       * @brief
       */
      std::shared_ptr<utils::GEMInfoSpaceToolBox> getAppStateISToolBox() { return p_appStateInfoSpaceToolBox; };

    private:
      GEMFSM m_gemfsm;

      xdata::Double m_progress;    // just to see the progress of the various transitions
      bool b_accept_web_commands;  // should we allow state transition commands from the web interface

      toolbox::BSem m_wl_semaphore;      // do we need a semaphore for the workloop?
      toolbox::BSem m_db_semaphore;      // do we need a semaphore for the database?
      toolbox::BSem m_cfg_semaphore;     // do we need a semaphore for the config file?
      toolbox::BSem m_web_semaphore;     // do we need a semaphore for the web access?
      toolbox::BSem m_infspc_semaphore;  // do we need a semaphore for the infospace?

      toolbox::task::WorkLoop *p_wl;

    protected:
      /**
       * @brief updateState
       */
      virtual void updateState() { m_stateName = m_gemfsm.getCurrentState(); };

      std::shared_ptr<utils::GEMInfoSpaceToolBox> p_appStateInfoSpaceToolBox;

      xdata::InfoSpace* p_appStateInfoSpace;

      xdata::String  m_stateName;
      xdata::String  m_stateMessage;
    };
  }  // namespace gem::base
}  // namespace gem

#endif  // GEM_BASE_GEMFSMAPPLICATION_H
