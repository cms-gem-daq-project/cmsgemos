/** @file ShelfMonitorWeb.h */

#ifndef GEM_DAQMON_SHELFMONITORWEB_H
#define GEM_DAQMON_SHELFMONITORWEB_H

#include <memory>

#include "gem/base/GEMWebApplication.h"

namespace gem {
  namespace daqmon {

    class ShelfMonitor;

    class ShelfMonitorWeb: public gem::base::GEMWebApplication
      {
        // friend class GEMMonitor;
        // friend class GEMSupervisor;
        // friend class gem::base::GEMFSMApplication;

      public:
        ShelfMonitorWeb(ShelfMonitor *shelfMonitorApp);

        virtual ~ShelfMonitorWeb();

      protected:
        virtual void webDefault(  xgi::Input *in, xgi::Output *out )
          throw (xgi::exception::Exception);

//        virtual void controlPanel(  xgi::Input *in, xgi::Output *out )
//          throw (xgi::exception::Exception);
//
        virtual void monitorPage(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);
//
        virtual void expertPage(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);
//
        virtual void applicationPage(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);
//
        virtual void jsonUpdate(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);

//
//        void displayManagedStateTable(xgi::Input *in, xgi::Output *out)
//          throw (xgi::exception::Exception);

      private:
        size_t level;
        // GEMSupervisor *gemSupervisorP__;
        // GEMSupervisorWeb(GEMSupervisorWeb const&);
      };  // class gem::daqmon::ShelfMonitorWeb
  }  // namespace gem::daqmon
}  // namespace gem

#endif  // GEM_DAQMON_SHELFMONITORWEB_H
