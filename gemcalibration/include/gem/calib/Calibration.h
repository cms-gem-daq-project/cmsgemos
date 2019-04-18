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


    enum dacScanType {CFG_CAL_DAC, CFG_BIAS_PRE_I_BIT, CFG_BIAS_PRE_I_BLCC, CFG_BIAS_PRE_I_BSF, CFG_BIAS_SH_I_BFCAS, CFG_BIAS_SH_I_BDIFF, CFG_BIAS_SD_I_BDIFF, CFG_BIAS_SD_I_BFCAS, CFG_BIAS_SD_I_BSF, CFG_BIAS_CFD_DAC_1, CFG_BIAS_CFD_DAC_2, CFG_HYST, CFG_THR_ARM_DAC, CFG_THR_ZCC_DAC, CFG_BIAS_PRE_VREF, CFG_VREF_ADC}; 
    typedef enum dacScanType dacScanType_t;

    
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
            {GBTPHASE  ,{{"nSamples",100},{"trigType", 0},}},
            {LATENCY,{
                {"nSamples"  , 100},
                {"trigType"  , 0},
                {"l1aTime"   , 250},
                {"calPhase"  , 0},
                {"mspl"      , 4},
                {"scanMin"   , 0},
                {"scanMax"   , 255},
                {"vfatChMin" , 0},
                {"vfatChMax" , 127},
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
		{"nSamples", 100},
		{"comparatorType",0},
		{"perChannelType",0},
		{"vfatChMin" , 0},
		{"vfatChMax" , 127},
		}},
	    {ARMDACSCAN  ,{
		{"nSamples"  , 100},
		{"trigType"  , 0},
		{"vfatChMin" , 0},
		{"vfatChMax" , 127},
		}},
            {TRIMDAC  , {
		{"nSamples"   , 100}, 
		{"trigType"   , 0}, // TODO: TTC local should be only possible one
		{"nSamples"   , 100},
		{"l1aTime"    , 250},
                {"pulseDelay" , 40},
                {"latency"    , 33},
		{"mspl"       , 4},
		{"trimValues" , 3},// TODO: need to be implemented properly in the back end in order to get a given number of points {-63,0,63}
                                   // TODO: need to implement interaction with DB to get proper configurations per ARM DAC
		}},
	    {DACSCANV3  ,{
	       {"nSamples",100},
	       {"adcType",0},
	      }},
	    {CALIBRATEARMDAC,{
	       {"nSamples"  , 100},
	       {"trigType"  , 0}, // TODO: TTC local should be only possible one
               {"l1aTime"   , 250},
               {"pulseDelay", 40},
               {"latency"   , 33},
               {"calPhase"  , 0},
		 // TODO: need to take the list of te ARM dac from a file
	       }}  
        };

	struct  scanParamsRadioSelector{
	  //std::string name;
	  
	  std::string label;
	  std::vector<std::string>  radio_options;
	  
	};
	
    
	std::map<std::string, scanParamsRadioSelector> m_scanParamsRadioSelector{
	  {"trigType",
	      {"TriggerType", {"TTC input","Loopback","Lemo/T3"}}},
	      {"signalSourceType", {"Signal Source", {"Calibration Pulse","Particle"}}},
	      {"comparatorType", {"Coparator Type", {"CDF","Arming comparator"}}},
	      {"adcType", {"VFAT ADC reference", {"Internal","External"}}},
	      {"perChannelType", {"DAC scan per channel", {"False","True"}}},
	    };
	std::set <std::string> m_scanParamsNonForm  {"trigType", "signalSourceType", "signalSourceType", "comparatorType", "adcType", "perChannelType", "dacScanType" };
      
	
	std::map< std::string,std::string > m_scanParamsLabels{
	  {"nSamples"  , "Number of samples"},
	  {"l1aTime"   , "L1A period (BX)"},
	  {"calPhase"  , "CalPulse phase"},
          {"mspl"      , "Pulse stretch (int)"},
	  {"scanMin"   , "Scan min"},
          {"scanMax"   , "Scan max"},
          {"vfatChMin" , "VFAT Ch min"},
          {"vfatChMax" , "VAT Ch max"},
          {"vt2"       , "CFG_THR_ARM_DAC"},
          {"trigThrottle"  , "Trigger throttle (int)"},
	  {"pulseDelay", "Pulse delay (BX)"},
	  {"latency"   , "Latency (BX)"},
	  {"trimValues", "Points in dac range (odd)"}, // TODO:need to be implemented properly in the back end in order to get a given number of points {-63,0,63}
	    };
        std::map<std::string, uint32_t> amc_optical_links;

	dacScanType_t m_dacScanType;
	std::map<dacScanType_t,  std::map<std::string, uint32_t>> m_dacScanTypeParams{
	  {CFG_CAL_DAC,{{"CFG_CAL_DAC_Min"  , 0},{"CFG_CAL_DAC_Max", 255},}},
	  {CFG_BIAS_PRE_I_BIT, {{"CFG_BIAS_PRE_I_BIT_Min",0},{"CFG_BIAS_PRE_I_BIT_Max", 255},}},
	  {CFG_BIAS_PRE_I_BLCC,{{"CFG_BIAS_PRE_I_BLCC_Min", 0},{"CFG_BIAS_PRE_I_BLCC_Max", 63},}},
	  {CFG_BIAS_PRE_I_BSF,{{"CFG_BIAS_PRE_I_BSF_Min",0}, {"CFG_BIAS_PRE_I_BSF_Max", 63},}},
	  {CFG_BIAS_SH_I_BFCAS,{{"CFG_BIAS_SH_I_BFCAS_Min",0},{"CFG_BIAS_SH_I_BFCAS_Max", 255},}},
	  {CFG_BIAS_SH_I_BDIFF,{{"CFG_BIAS_SH_I_BDIFF_Min", 0},{"CFG_BIAS_SH_I_BDIFF_Max", 255},}},
	  {CFG_BIAS_SD_I_BDIFF,{{"CFG_BIAS_SD_I_BDIFF_Min",0},{"CFG_BIAS_SD_I_BDIFF_Max",255},}},
	  {CFG_BIAS_SD_I_BFCAS,{{"CFG_BIAS_SD_I_BFCAS_Min",0},{"CFG_BIAS_SD_I_BFCAS_Max", 255}, }},
	  {CFG_BIAS_SD_I_BSF,{{"CFG_BIAS_SD_I_BSF_Min",0}, {"CFG_BIAS_SD_I_BSF_Max", 63},}},
	  {CFG_BIAS_CFD_DAC_1,{{"CFG_BIAS_CFD_DAC_1_Min",0},{"CFG_BIAS_CFD_DAC_1_Max", 63},}},
	  {CFG_BIAS_CFD_DAC_2,{{"CFG_BIAS_CFD_DAC_2_Min", 0},{"CFG_BIAS_CFD_DAC_2_Max", 63},}},
	  {CFG_HYST,{{"CFG_HYST_Min",0},{"CFG_HYST_Max", 63},}},
	  {CFG_THR_ARM_DAC,{{"CFG_THR_ARM_DAC_Min",0},{"CFG_THR_ARM_DAC_Max", 255},}},
	  {CFG_THR_ZCC_DAC,{{"CFG_THR_ZCC_DAC_Min",0}, {"CFG_THR_ZCC_DAC_Max", 255},}},
	  {CFG_BIAS_PRE_VREF,{{"CFG_BIAS_PRE_VREF_Min",0},{"CFG_BIAS_PRE_VREF_Max", 255},}},  
	  {CFG_VREF_ADC, {{"CFG_VREF_ADC_Min",0},{"CFG_VREF_ADC_Max", 3},}}
	    };
	std::map<dacScanType_t,  std::string > m_dacScanTypeParams_label{
	  {CFG_CAL_DAC, "CFG_CAL_DAC"},
	  {CFG_BIAS_PRE_I_BIT, "CFG_BIAS_PRE_I_BIT"},
	  {CFG_BIAS_PRE_I_BLCC, "CFG_BIAS_PRE_I_BLCC"},
	  {CFG_BIAS_PRE_I_BSF, "CFG_BIAS_PRE_I_BSF"},
	  {CFG_BIAS_SH_I_BFCAS,"CFG_BIAS_SH_I_BFCAS"},
	  {CFG_BIAS_SH_I_BDIFF, "CFG_BIAS_SH_I_BDIFF"},
	  {CFG_BIAS_SD_I_BDIFF, "CFG_BIAS_SD_I_BDIFF"},
	  {CFG_BIAS_SD_I_BFCAS, "CFG_BIAS_SD_I_BFCAS"},
	  {CFG_BIAS_SD_I_BSF, "CFG_BIAS_SD_I_BSF"},
	  {CFG_BIAS_CFD_DAC_1,"CFG_BIAS_CFD_DAC_1"},
	  {CFG_BIAS_CFD_DAC_2,"CFG_BIAS_CFD_DAC_2"},
	  {CFG_HYST,"CFG_HYST"},
	  {CFG_THR_ARM_DAC,"CFG_THR_ARM_DAC"},
	  {CFG_THR_ZCC_DAC, "CFG_THR_ZCC_DAC"},
	  {CFG_BIAS_PRE_VREF,"CFG_BIAS_PRE_VREF"},
	  {CFG_VREF_ADC, "CFG_VREF_ADC"},
	};
	

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
