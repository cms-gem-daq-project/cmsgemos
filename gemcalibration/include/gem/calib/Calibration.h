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

    enum calType {NDEF, GBTPHASE, LATENCY, SCURVE, SBITARMDACSCAN, ARMDACSCAN, TRIMDAC, DACSCANV3, CALIBRATEARMDAC}; 
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
        
        void setCalType(xgi::Input *in, xgi::Output *out)
          throw (xgi::exception::Exception);

        std::vector<Calibration*> v_gemcal;

        calType_t m_calType;
	
        std::map<calType_t, std::map<std::string, uint32_t>> m_scanParams{
            {GBTPHASE  ,{{"nSamples",0},{"trigType", 0},}},
            {LATENCY,{
                {"nSamples"  , 100},
                {"trigType"  , 0},
                {"l1aTime"   , 250},
                {"calPhase"  , 0},
                {"mspl"      , 4},
                {"scanMin"   , 0},
                {"scanMax"   , 255},
                {"vfatChMin" , 0},
                {"vfatChMax" , 0},
                {"vt2"       , 0},
		{"trigThrottle"  ,100},
                {"signalSourceType"       , 0},
                }},
            {SCURVE,{
                {"nSamples"  , 100},
		{"trigType"  , 0}, // TODO: TTC local should be only possible one
                {"l1aTime"   , 250},
                {"pulseDelay", 40},
                {"latency"   , 33},
                {"vfatChMin" , 0},
                {"vfatChMax" , 127},
                {"calPhase"  , 0},
                }},
	    {SBITARMDACSCAN  ,{
		{"nSamples",0},
		{"comparatorType",0},
		{"perChannelType",0},
		{"vfatChMin" , 0},
                {"vfatChMax" , 0},
		}},
	    {ARMDACSCAN  ,{
		{"nSamples"  , 0},
		{"trigType"  , 0},
		{"vfatChMin" , 0},
		{"vfatChMax" , 0},
		{"vt2"       , 0},
		}},
            {TRIMDAC  , {
		{"nSamples"   , 0}, 
		{"trigType"   , 0}, // TODO: TTC local should be only possible one
		{"nSamples"   , 0},
		{"l1aTime"    , 250},
                {"pulseDelay" , 40},
                {"latency"    , 33},
		{"mspl"       , 4},
		{"trimValues" , 0},// TODO: need to be implemented properly as taking array of numbers 
                                   // TODO: need to implement interaction with DB to get proper configurations per ARM DAC
		}},
	    {DACSCANV3  ,{
	       {"nSamples",0},
	       {"adcType",0}
	      }},// TODO: drop down with DACs to select to scan on, and a select all button
	    {CALIBRATEARMDAC,{
	       {"nSamples"  , 100},
	       {"trigType"  , 0}, // TODO: TTC local should be only possible one
               {"l1aTime"   , 250},
               {"pulseDelay", 40},
               {"latency"   , 33},
               {"armDacList" , 0},// TODO: need to be implemented properly as taking array of numbers 
            
               {"calPhase"  , 0}, 
	       }}  
        };
	std::map< std::string,std::string > m_scanParamsLabels{
	  {"nSamples"  , "Number of samples"},
	  {"l1aTime"   , "L1A period (BX)"},
	  {"calPhase"  , "CalPulse phase ()"},
          {"mspl"      , "Pulse stretch (int)"},
	  {"scanMin"   , "Scan min ()"},
          {"scanMax"   , "Scan max ()"},
          {"vfatChMin" , "VFAT Ch min"},
          {"vfatChMax" , "VAT Ch max"},
          {"vt2"       , "CFG_THR_ARM_DAC"},
          {"trigThrottle"  , "Trigger throttle (int)"},
	  {"pulseDelay", "Pulse delay (BX)"},
	  {"latency"   , "Latency (BX)"},
	  {"timeInterval", "Interval bw measur. (s)"},
	  {"rates", "Rates (Hz)"} , // TODO: need to be implemented properly as taking array of numbers
	  {"armDacList", "List of ARM DAC settings to scan"}, // TODO: need to be implemented properly as taking array of numbers
	  {"trimValues", "Points in DAC range"}, // TODO: need to be implemented properly as taking array of numbers
	    };
        std::map<std::string, uint32_t> amc_optical_links;

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
        const std::map<std::string, calType_t> m_calTypeSelector{
	    {"GBT Phase Scan"                , GBTPHASE},
	    {"Latency Scan"                  , LATENCY},
	    {"S-curve Scan"                  , SCURVE},
            {"S-bit ARM DAC Scan"            , SBITARMDACSCAN},
            {"ARM DAC Scan"                  , ARMDACSCAN},
            {"Derive DAC Trim Registers"     , TRIMDAC},
            {"DAC Scan on VFAT3"             , DACSCANV3},
            {"Calibrate CFG_THR_ARM_DAC"     , CALIBRATEARMDAC},
	    
        };
      };
  }  // namespace gem::calib
}  // namespace gem

#endif  // GEM_GEMCAL_CALIBRATION_H
