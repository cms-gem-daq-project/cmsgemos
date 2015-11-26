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
         * @param glib the HwGLIB uhal device which is to be monitored
         * @param glibManager the manager application for the GLIB to be monitored
         */
        GEMSupervisorMonitor(GEMSupervisor* glibSupervisor);
        
        virtual ~GEMSupervisorMonitor();
        
        virtual void updateMonitorables();
      private:
          
      }; // end class GEMSupervisorMonitor
            
  }// end namespace gem::supervisor
}// end namespace gem

#endif
