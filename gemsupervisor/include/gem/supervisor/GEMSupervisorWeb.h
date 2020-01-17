/** @file GEMSupervisorWeb.h */

#ifndef GEM_SUPERVISOR_GEMSUPERVISORWEB_H
#define GEM_SUPERVISOR_GEMSUPERVISORWEB_H

#include <memory>

#include "gem/base/GEMWebApplication.h"

namespace gem {
  namespace supervisor {

    class GEMSupervisor;

    class GEMSupervisorWeb: public gem::base::GEMWebApplication
      {
        // friend class GEMMonitor;
        // friend class GEMSupervisor;
        // friend class gem::base::GEMFSMApplication;

      public:
        GEMSupervisorWeb(GEMSupervisor *gemSupervisorApp);

        virtual ~GEMSupervisorWeb();

      protected:
        virtual void webDefault(  xgi::Input *in, xgi::Output *out);

        virtual void controlPanel(  xgi::Input *in, xgi::Output *out);

        virtual void monitorPage(xgi::Input *in, xgi::Output *out);

        virtual void expertPage(xgi::Input *in, xgi::Output *out);

        virtual void applicationPage(xgi::Input *in, xgi::Output *out);

        virtual void jsonUpdate(xgi::Input *in, xgi::Output *out);

        void displayManagedStateTable(xgi::Input *in, xgi::Output *out);

      private:
        size_t level;
        // GEMSupervisor *gemSupervisorP__;
        // GEMSupervisorWeb(GEMSupervisorWeb const&);
      };  // class gem::supervisor::GEMSupervisorWeb
  }  // namespace gem::supervisor
}  // namespace gem

#endif  // GEM_SUPERVISOR_GEMSUPERVISORWEB_H
