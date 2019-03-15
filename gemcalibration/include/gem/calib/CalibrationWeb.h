/** @file CalibrationWeb.h */

#ifndef GEM_GEMCAL_CALIBRATIONWEB_H
#define GEM_GEMCAL_CALIBRATIONWEB_H

#include <memory>

#include "gem/base/GEMWebApplication.h"

#include "gem/calib/Calibration.h"

namespace gem {
  namespace calib {

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
//
        void phaseInterface(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void latencyInterface(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void scurveInterface(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void sbitRateInterface(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void triggerSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void nSamplesSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void slotsAndMasksSelector(xgi::Output *out)
          throw (xgi::exception::Exception);


      protected:
        virtual void webDefault(  xgi::Input *in, xgi::Output *out )
          throw (xgi::exception::Exception);

        virtual void calibrationPage(  xgi::Input *in, xgi::Output *out )
          throw (xgi::exception::Exception);

       virtual void monitorPage(xgi::Input *in, xgi::Output *out)
         throw (xgi::exception::Exception);
//
        virtual void expertPage(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);
	
//        virtual void LatencyScanPage(xgi::Input *in, xgi::Output *out)
        //  throw (xgi::exception::Exception);
//
        virtual void applicationPage(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);
//
        virtual void jsonUpdate(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);

      private:
        size_t level;
        // GEMSupervisor *gemSupervisorP__;
        // GEMSupervisorWeb(GEMSupervisorWeb const&);
      };  // class gem::calib::CalibrationWeb
  }  // namespace gem::calib
}  // namespace gem

#endif  // GEM_GEMCAL_CALIBRATIONWEB_H