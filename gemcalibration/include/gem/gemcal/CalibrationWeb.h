/** @file CalibrationWeb.h */

#ifndef GEM_GEMCAL_CALIBRATIONWEB_H
#define GEM_GEMCAL_CALIBRATIONWEB_H

#include <memory>

#include "gem/base/GEMWebApplication.h"

namespace gem {
  namespace gemcal {

    class Calibration;

    class CalibrationWeb: public gem::base::GEMWebApplication
      {
        // friend class GEMMonitor;
        // friend class GEMSupervisor;
        // friend class gem::base::GEMFSMApplication;

      public:
	CalibrationWeb(Calibration *CalibrationApp);
	//CalibrationWeb();

        virtual ~CalibrationWeb();

      protected:
        virtual void webDefault(  xgi::Input *in, xgi::Output *out )
          throw (xgi::exception::Exception);

//        virtual void controlPanel(  xgi::Input *in, xgi::Output *out )
//          throw (xgi::exception::Exception);
//
//        virtual void monitorPage(xgi::Input *in, xgi::Output *out)
//          throw (xgi::exception::Exception);
//
        virtual void expertPage(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);
//
        virtual void applicationPage(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);
//
//        virtual void jsonUpdate(xgi::Input *in, xgi::Output *out)
//          throw (xgi::exception::Exception);

//
//        void displayManagedStateTable(xgi::Input *in, xgi::Output *out)
//          throw (xgi::exception::Exception);

      private:
        size_t level;
        // GEMSupervisor *gemSupervisorP__;
        // GEMSupervisorWeb(GEMSupervisorWeb const&);
      };  // class gem::gemcal::CalibrationWeb
  }  // namespace gem::gemcal
}  // namespace gem

#endif  // GEM_GEMCAL_CALIBRATIONWEB_H
