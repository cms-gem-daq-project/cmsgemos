/** @file Calibration.h */

#ifndef GEM_GEMCAL_CALIBRATION_H
#define GEM_GEMCAL_CALIBRATION_H

#include <string>
#include <vector>

#include "gem/base/GEMApplication.h"
#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

//#include "gem/daqmon/exception/Exception.h"
#include "gem/utils/exception/Exception.h"
//#include "gem/daqmon/DaqMonitor.h"

#define NAMC 12
#define NSHELF 2

namespace gem {
  namespace calib {

    enum calType {NDEF, PHASE, LATENCY, SCURVE, SBITRATE}; 
    typedef enum calType calType_t;

    class Calibration : public gem::base::GEMApplication
      {

      public:
        XDAQ_INSTANTIATOR();

        Calibration(xdaq::ApplicationStub* s);

        virtual ~Calibration();

        virtual void init();

        virtual void actionPerformed(xdata::Event& event);

	// void startMonitoring();

        //void stopMonitoring();

        //std::string monitoringState(){return m_state;}

        void stopAction(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);

        void resumeAction(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);

        void pauseAction(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);
	
        void applyAction(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);

//
        void setCalType(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);

        std::vector<Calibration*> v_gemcal;

        int PulseDelay;
	
      protected:
        /* virtual bool calibrationAction(toolbox::task::WorkLoop *wl); */
        /* virtual bool calibrationSequencer(toolbox::task::WorkLoop *wl); */

      private:
        /**
         * @param classname is the class to check to see whether it is a GEMApplication inherited application
         * @throws
         */
        bool isGEMApplication(const std::string& classname) const;
        xdata::Integer m_shelfID;
        log4cplus::Logger m_logger; //FIXME should be removed!
        std::string m_state;
        calType_t m_calType;
        const std::map<std::string, calType_t> m_calTypeSelector{
            {"Phase Scan"       , PHASE},
            {"Latency Scan"     , LATENCY},
            {"S-curve Scan"     , SCURVE},
            {"S-bit Rate Scan"  , SBITRATE},
        };
        std::map<calType_t, std::map<std::string, uint32_t>> m_scanParams{
        {PHASE  ,{{"n_samples",0},{"trig_type", 0},}},
        {LATENCY,{
            {"nSamples"  , 0},
            {"trigType"  , 0},
            {"l1aTime"   , 0},
            {"calPhase"  , 0},
            {"mspl"      , 0},
            {"scanMin"   , 0},
            {"scanMax"   , 0},
            {"vfatChMin" , 0},
            {"vfatChMax" , 0},
            {"vt2"       , 0},
            }},
        {SCURVE,{
            {"nSamples"  , 0},
            {"trigType"  , 0},
            {"l1aTime"   , 0},
            {"pulseDelay", 0},
            {"latency"   , 0},
            {"vfatChMin" , 0},
            {"vfatChMax" , 0},
            {"calPhase"  , 0},
            }},
         };
        //int nSamples, trigType, l1aTime, latency, pulseDelay, calPhase, vfatChMin, vfatChMax; 
	    //int scanMin, scanMax, throttle, vt2, mspl;
	  
      };
  }  // namespace gem::calib
}  // namespace gem

#endif  // GEM_GEMCAL_CALIBRATION_H
