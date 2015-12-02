/** @file GEMSupervisorMonitor.h */ 

#ifndef gem_supervisor_GEMSupervisorMonitor_h
#define gem_supervisor_GEMSupervisorMonitor_h


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
        void buildStateTable(xgi::Output* out);

      private:
          
      }; // end class GEMSupervisorMonitor
            
  }// end namespace gem::supervisor
}// end namespace gem

#endif
