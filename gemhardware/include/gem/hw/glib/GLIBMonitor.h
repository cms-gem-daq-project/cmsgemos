/** @file GLIBMonitor.h */ 

#ifndef gem_hw_glib_GLIBMonitor_h
#define gem_hw_glib_GLIBMonitor_h


#include "gem/base/GEMMonitor.h"
#include "gem/hw/glib/exception/Exception.h"

namespace gem {
  namespace hw {
    namespace glib {
      
      class HwGLIB;
      class GLIBManager;
      
      class GLIBMonitor : public gem::base::GEMMonitor
      {
      public:

        /**
         * Constructor from GEMFSMApplication derived classes
         * @param glib the HwGLIB uhal device which is to be monitored
         * @param glibManager the manager application for the GLIB to be monitored
         */
        GLIBMonitor(std::shared_ptr<HwGLIB> glib, GLIBManager* glibManager);
        
        virtual ~GLIBMonitor();
        
        virtual void updateMonitorables();
      private:
        std::shared_ptr<HwGLIB> p_glib;
        
      }; // end class GLIBMonitor
            
    }// end namespace gem::hw::glib
  }// end namespace gem::hw
}// end namespace gem

#endif
