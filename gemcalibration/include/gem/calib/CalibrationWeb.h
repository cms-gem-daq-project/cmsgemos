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
        void settingsInterface(calType_t m_calType, xgi::Output *out)
          throw (xgi::exception::Exception);
//	
        void triggerSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void comparatorSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void signalSourceSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void perChannelTypeSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void adcTypeSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void slotsAndMasksSelector(xgi::Output *out)
          throw (xgi::exception::Exception);
//
        void genericParamSelector(std::string labelName, std::string paramName, int defaultValue, xgi::Output *out)
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
        virtual void applicationPage(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);
//
        virtual void jsonUpdate(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);

      private:
        size_t level;
        const std::map<calType_t, std::string> alertMap {
            {GBTPHASE,"HELLO WORLD! I'm not yet implemented..."},
            {LATENCY,"To run the routine select the cards, the optohybrids, the VFATs and links, indicate the number of events \
               for each position, the throttle, the  pulse stretch configuration, the minimum and maximum scan values, and the CFG_THR_ARM_DAC."},
            {SCURVE,"To run the routine select the cards, the optohybrids, the VFATs and links. \
                Indicate the number of events for each position and the latency and pulse stretch configuration."},
            {SBITARMDACSCAN,"To run the routine select the cards, the optohybrids, the VFATs and links."},
            {ARMDACSCAN,"To run the routine select the cards, the optohybrids, the VFATs and links, indicate the number of events \
                for each position, the minimum and maximum scan values, and the CFG_THR_ARM_DAC."},
            {TRIMDAC,"To run the routine select the cards, the optohybrids."},
            {DACSCANV3,"To run the routine select the cards, and the optohybrids. Really?? //TODO: clarify!!"},
	    {CALIBRATEARMDAC,"To run the routine select the cards, the optohybrids. \
                Indicate the number of events for each position and the latency and pulse stretch configuration."},
        
        };
        // GEMSupervisor *gemSupervisorP__;
        // GEMSupervisorWeb(GEMSupervisorWeb const&);
      };  // class gem::calib::CalibrationWeb
  }  // namespace gem::calib
}  // namespace gem

#endif  // GEM_GEMCAL_CALIBRATIONWEB_H
