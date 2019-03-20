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
        void thresholdInterface(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void scurveInterface(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void sbitRateInterface(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void sbitReadOutInterface(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void sbitMapAndRateInterface(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void trimDACInterface(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void scanDACInterface(xgi::Output *out)
          throw (xgi::exception::Exception);
//	
        void temperatureInterface(xgi::Output *out)
          throw (xgi::exception::Exception);
//	
        void triggerSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void nSamplesSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void l1aTimeSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void latencySelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void pulseDelaySelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void calPhaseSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void vfatChMinSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void vfatChMaxSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void scanMinSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void scanMaxSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void throttleSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void vt2ThresholdSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void msplSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void slotsAndMasksSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void comparatorSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void timeIntervalSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void timeAcquisitionSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void timemsSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void rateArraySelector(xgi::Output *out)
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
	
//
	/* virtual void LatencyScanPage(xgi::Input *in, xgi::Output *out) */
	/*   throw (xgi::exception::Exception); */
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
