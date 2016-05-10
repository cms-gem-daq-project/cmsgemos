#ifndef GEM_SUPERVISOR_GEMSUPERVISOR_H
#define GEM_SUPERVISOR_GEMSUPERVISOR_H

#include <string>
#include <vector>

#include "uhal/uhal.hpp"

#include "xdaq2rc/RcmsStateNotifier.h"

#include "gem/base/GEMFSMApplication.h"
#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

#include "gem/supervisor/GEMSupervisorWeb.h"
#include "gem/supervisor/GEMGlobalState.h"
#include "gem/supervisor/exception/Exception.h"

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

        // state transitions
        virtual void initializeAction() throw (gem::supervisor::exception::Exception);
        virtual void configureAction()  throw (gem::supervisor::exception::Exception);
        virtual void startAction()      throw (gem::supervisor::exception::Exception);
        virtual void pauseAction()      throw (gem::supervisor::exception::Exception);
        virtual void resumeAction()     throw (gem::supervisor::exception::Exception);
        virtual void stopAction()       throw (gem::supervisor::exception::Exception);
        virtual void haltAction()       throw (gem::supervisor::exception::Exception);
        virtual void resetAction()      throw (gem::supervisor::exception::Exception);
        // virtual void noAction()         throw (gem::supervisor::exception::Exception);

        virtual void failAction(toolbox::Event::Reference e)
          throw (toolbox::fsm::exception::Exception);

        virtual void resetAction(toolbox::Event::Reference e)
          throw (toolbox::fsm::exception::Exception);

        std::vector<xdaq::ApplicationDescriptor*> getSupervisedAppDescriptors() {
          return v_supervisedApps; };

        friend class gem::supervisor::GEMGlobalState;

        /* getCurrentState
         * @returns std::string name of the current global state
         *
        virtual std::string getCurrentState() {
          return m_stateName.toString();
          };*/
      private:
        /**
         * @param classname is the class to check to see whether it is a GEMApplication inherited application
         * @throws
         */
        bool isGEMApplication(const std::string& classname) const;

        /**
         * @param classname, checks whether or not GEMSupervisor should manage (send transitions) to this application
         * @throws
         */
        bool manageApplication(const std::string& classname) const;

        /**
         * @brief Sets global supervisor state and optionally sends to RCMS
         * @param before state before the update
         * @param after state after the update
         * @throws
         */
        void globalStateChanged(toolbox::fsm::State before, toolbox::fsm::State after);

        /**
         * @brief gets new run number in the case of a run transistion stop/start
         */
        void updateRunNumber();

        /**
         * @param cfgType tells the application which type of configuration to use, XML or DB
         * @param ad is the application descriptor to send the SOAP message to
         * @throws
         */
        void sendCfgType(std::string const& cfgType, xdaq::ApplicationDescriptor* ad)
          throw (gem::supervisor::exception::Exception);

        /**
         * @param runType tells the application which type of run to take
         * @param ad is the application descriptor to send the SOAP message to
         * @throws
         */
        void sendRunType(std::string const& runType, xdaq::ApplicationDescriptor* ad)
          throw (gem::supervisor::exception::Exception);

        /**
         * @param runNumber is the run number
         * @param ad is the application descriptor to send the SOAP message to
         * @throws
         */
        void sendRunNumber(int64_t const& runNumber, xdaq::ApplicationDescriptor* ad)
          throw (gem::supervisor::exception::Exception);

        std::shared_ptr<GEMSupervisorMonitor> m_supervisorMonitor;

        mutable gem::utils::Lock m_deviceLock;
        std::vector<xdaq::ApplicationDescriptor*> v_supervisedApps;
        xdaq::ApplicationDescriptor* readoutApp;

        GEMGlobalState m_globalState;

        xdata::Boolean             m_reportToRCMS;
        xdata::String              m_rcmsStateListenerUrl;
        xdaq2rc::RcmsStateNotifier m_gemRCMSNotifier;
      };
  }  // namespace gem::supervisor
}  // namespace gem

#endif  // GEM_SUPERVISOR_GEMSUPERVISOR_H
