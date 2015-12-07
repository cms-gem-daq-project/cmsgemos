#ifndef gem_supervisor_GEMSupervisorWeb_h
#define gem_supervisor_GEMSupervisorWeb_h

#include <memory>

#include "gem/base/GEMWebApplication.h"

namespace gem {
  namespace supervisor {

    class GEMSupervisor;

    class GEMSupervisorWeb: public gem::base::GEMWebApplication
      {
        //friend class GEMMonitor;
        //friend class GEMSupervisor;
        //friend class gem::base::GEMFSMApplication;

      public:
        GEMSupervisorWeb(GEMSupervisor *gemSupervisorApp);
	
        virtual ~GEMSupervisorWeb();
	
      protected:

        virtual void webDefault(  xgi::Input *in, xgi::Output *out )
          throw (xgi::exception::Exception);

        virtual void monitorPage(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);
	
        virtual void expertPage(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);
	
        virtual void jsonUpdate(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);
	
        void displayManagedStateTable(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);
	
      private:
        size_t level;
        //GEMSupervisor *gemSupervisorP__;
        //GEMSupervisorWeb(GEMSupervisorWeb const&);
	
      }; //class gem::supervisor::GEMSupervisorWeb
  } // namespace gem::supervisor
} // namespace gem
#endif
