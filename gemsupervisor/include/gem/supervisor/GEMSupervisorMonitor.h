#ifndef GEM_SUPERVISOR_GEMSUPERVISORMONITOR_H
#define GEM_SUPERVISOR_GEMSUPERVISORMONITOR_H

/** @file GEMSupervisorMonitor.h */


#include "gem/base/GEMMonitor.h"
#include "gem/supervisor/exception/Exception.h"

namespace gem {
  namespace supervisor {

    class GEMSupervisor;

    class GEMSupervisorMonitor : public gem::base::GEMMonitor
      {
      public:

        /**
         * Constructor from GEMFSMApplication derived classes
         * @param gemSupervisor the supervisor application to be monitored
         */
        GEMSupervisorMonitor(GEMSupervisor* gemSupervisor);

        virtual ~GEMSupervisorMonitor();

        virtual void updateMonitorables();
        void setupAppStateMonitoring();
        void updateApplicationStates();
        void buildStateTable(xgi::Output* out);

      private:
      };  // class GEMSupervisorMonitor

  }  // namespace gem::supervisor
}  // namespace gem

#endif  // GEM_SUPERVISOR_GEMSUPERVISORMONITOR_H
