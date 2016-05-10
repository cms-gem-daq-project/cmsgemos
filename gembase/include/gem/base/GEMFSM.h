#ifndef GEM_BASE_GEMFSM_H
#define GEM_BASE_GEMFSM_H

#include <map>
#include <string>

#include "log4cplus/logger.h"

#include "toolbox/Event.h"
#include "toolbox/fsm/FiniteStateMachine.h"
#include "toolbox/lang/Class.h"

#include "xcept/Exception.h"

#include "xdata/String.h"

#include "xoap/MessageReference.h"

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

        GEMFSM(GEMFSMApplication* const gemAppP);

        virtual ~GEMFSM();

        /**
         * @brief
         */
        void fireEvent(::toolbox::Event::Reference const &event);

        /**
         * @brief
         */
        xoap::MessageReference changeState(xoap::MessageReference msg);

        /**
         * @brief
         */
        std::string getCurrentState() const;

        /**
         * @brief
         */
        std::string getStateName(toolbox::fsm::State const& state) const;

        /**
         * @brief
         */
        void gotoFailed(std::string const reason="No further information available");

        /**
         * @brief
         */
        void gotoFailed(xcept::Exception& err);

        /**
         * @brief
         */
        void gotoFailedAsynchronously(xcept::Exception& err);

      protected:

        /**
         * @brief
         */
        void stateChanged(toolbox::fsm::FiniteStateMachine& fsm)
          throw(toolbox::fsm::exception::Exception);

        //void stateChangedWithNotification(toolbox::fsm::FiniteStateMachine& fsm);

        //void stateChangedToFailedWithNotification(toolbox::fsm::FiniteStateMachine& fsm);

        /**
         * @brief
         */
        void invalidAction(toolbox::Event::Reference event);

      private:
        toolbox::fsm::AsynchronousFiniteStateMachine* p_gemfsm;
        xdata::InfoSpace *p_appInfoSpace;
        xdata::InfoSpace *p_appStateInfoSpace;

        xdata::String m_gemFSMState;
        xdata::String m_reasonForFailure;

        GEMFSMApplication* p_gemApp;
        log4cplus::Logger m_gemLogger;
        std::map<std::string, std::string> m_lookupMap;
      };
  }  // namespace gem::base
}  // namespace gem

#endif  // GEM_BASE_GEMFSM_H
