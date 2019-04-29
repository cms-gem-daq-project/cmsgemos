/** @file Calibration.h */

#ifndef GEM_CALIB_CALIBRATION_H
#define GEM_CALIB_CALIBRATION_H

#include <string>
#include <vector>

#include "gem/base/GEMApplication.h"
#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

#include "gem/utils/exception/Exception.h"
#include "gem/calib/GEMCalibEnums.h"


namespace gem {
    namespace calib {

        class Calibration : public gem::base::GEMApplication
        {

        public:
            XDAQ_INSTANTIATOR();

            Calibration(xdaq::ApplicationStub* s);

            virtual ~Calibration();

            virtual void init();

            virtual void actionPerformed(xdata::Event& event);

            void applyAction(xgi::Input *in, xgi::Output *out)
                throw (xgi::exception::Exception);

            /**
             * @brief Set the calibration routine to be perfomed from an initial dropdown menu and calls the appropriate selected interface
             * @throws
             */

            void setCalType(xgi::Input *in, xgi::Output *out)
                throw (xgi::exception::Exception);

            calType_t m_calType;

            /**
             *  map to link a particular calibration routine to the parameters needed for it.
             *  parameters can be filled with a form  or a radio selector
             */

            std::map<calType_t, std::map<std::string, std::string /* uint32_t */>> m_scanParams{
                {GBTPHASE  ,{{"nSamples","100"},{"phaseMin", "0"},{"phaseMax", "14"},{"stepSize", "1"},}},
                {LATENCY,{
                        {"nSamples"  , "100"},
                        {"trigType"  , "0"},
                        {"l1aTime"   , "250"},
                        {"mspl"      , "4"},
                        {"scanMin"   , "0"},
                        {"scanMax"   , "255"},
                        {"vfatChMin" , "0"},
                        {"vfatChMax" , "127"},
                            //{"vt2"       , 100},// TODO:need to be taken from DB
                        {"trigThrottle"  ,"0"},
                        {"signalSourceType"       , "0"},
                        {"pulseDelay" , "40"},
                }},
                {SCURVE,{
                        {"nSamples"  , "100"},
                        {"trigType"  , "0"}, // TODO: TTC local should be only possible one
                        {"l1aTime"   , "250"},
                        {"pulseDelay", "40"},
                        {"latency"   , "33"},
                        {"vfatChMin" , "0"},
                        {"vfatChMax" , "127"},
                        {"mspl"      , "4"},
                 }},
                 {SBITARMDACSCAN  ,{
                        {"comparatorType","0"},
                        {"perChannelType","0"},
                        {"vfatChMin" , "0"},
                        {"vfatChMax" , "127"},
                        {"stepSize", "1"},
                  }},
                  {ARMDACSCAN  ,{
                        {"nSamples"  , "1000"},
                        {"trigType"  , "0"},
                        {"vfatChMin" , "0"},
                        {"vfatChMax" , "127"},
                   }},
                   {TRIMDAC  , {
                        {"nSamples"   , "100"},
                        {"trigType"   , "0"}, // TODO: TTC local should be only possible one
                        {"nSamples"   , "100"},
                        {"l1aTime"    , "250"},
                        {"pulseDelay" , "40"},
                        {"latency"    , "33"},
                        {"mspl"       , "4"},
                        {"trimValues" , "-63,0,63"},// TODO: need to be implemented properly in the back end in order to get a given number of points {-63,0,63}
                        // TODO: need to implement interaction with DB to get proper configurations per ARM DAC
                   
                    }},
                    {DACSCANV3  ,{
                        {"adcType","0"},
                    }},
                    {CALIBRATEARMDAC,{
                        {"nSamples"  , "100"},
                        {"trigType"  , "0"}, // TODO: TTC local should be only possible one
                        {"l1aTime"   , "250"},
                        {"pulseDelay", "40"},
                        {"latency"   , "33"},
                        {"armDacPoins","17,20,23,25,30,40,50,60,70,80,100,125,150,175"},
                        // TODO: need to take the list of te ARM dac from a file
                        }}
            };

            struct scanParamsRadioSelector{
                std::string label;
                std::vector<std::string>  options;
            };

            /**
             *  map of the parameters for which radio selector are used with relative viable options
             */

            std::map<std::string, scanParamsRadioSelector> m_scanParamsRadioSelector{
                {"trigType",{"TriggerType", {"TTC input","Loopback","Lemo/T3"}}},
                {"signalSourceType", {"Signal Source", {"Calibration Pulse","Particle"}}},
                {"comparatorType", {"Coparator Type", {"CDF","Arming comparator"}}},
                {"adcType", {"VFAT ADC reference", {"Internal","External"}}},
                {"perChannelType", {"DAC scan per channel", {"False","True"}}},
            };
            /**
             *  set of parameters not filled with form
             */
            std::set <std::string> m_scanParamsNonForm  {"trigType", "signalSourceType", "signalSourceType", "comparatorType", "adcType", "perChannelType", "dacScanType" };

            /**
             *  map of the routine parameters and relative labels to appear in the interface
             */

            std::map< std::string,std::string > m_scanParamsLabels{
                {"nSamples"  , "Number of samples"},
                {"l1aTime"   , "L1A period (BX)"},
                {"mspl"      , "Pulse stretch (int)"},
                {"scanMin"   , "Scan min"},
                {"scanMax"   , "Scan max"},
                {"vfatChMin" , "VFAT Ch min"},
                {"vfatChMax" , "VFAT Ch max"},
                    //{"vt2"       , "CFG_THR_ARM_DAC"},// TODO:need to be taken from DB
                {"trigThrottle"  , "Trigger throttle (int)"},
                {"pulseDelay", "Pulse delay (BX)"},
                {"latency"   , "Latency (BX)"},
                {"trimValues", "Points in dac range"}, // TODO:need to be implemented properly in the back end in order to get a given number of points {-63,0,63}
                {"phaseMin"  , "Phase min (int)"  },  
                {"phaseMax"  , "Phase max (int)"  },
                {"stepSize", "Step size (int)"},
                {"armDacPoins", "ARM DAC points"}, 
            };

            std::map<std::string, uint32_t> m_amcOpticalLinks;

            dacScanType_t m_dacScanType;

            struct dacScanTypeFeature{
                std::string label;
                std::map<std::string, uint32_t> range;
            };
            
             /**
             *  map of selectable DAC scan for the VFAT3 parameters and relative labels and range limits
             */

            
            std::map<dacScanType_t, dacScanTypeFeature> m_dacScanTypeParams{
                {CFG_CAL_DAC,{"CFG_CAL_DAC", {{"CFG_CAL_DAC_Min"  , 0},{"CFG_CAL_DAC_Max", 255},}}},
                {CFG_BIAS_PRE_I_BIT, {"CFG_BIAS_PRE_I_BIT",{{"CFG_BIAS_PRE_I_BIT_Min",0},{"CFG_BIAS_PRE_I_BIT_Max", 255},}}}, 
                {CFG_BIAS_PRE_I_BLCC,{"CFG_BIAS_PRE_I_BLCC",{{"CFG_BIAS_PRE_I_BLCC_Min", 0},{"CFG_BIAS_PRE_I_BLCC_Max", 63},}}},
                {CFG_BIAS_PRE_I_BSF,{"CFG_BIAS_PRE_I_BSF",{{"CFG_BIAS_PRE_I_BSF_Min",0}, {"CFG_BIAS_PRE_I_BSF_Max", 63},}}},
                {CFG_BIAS_SH_I_BFCAS,{"CFG_BIAS_SH_I_BFCAS",{{"CFG_BIAS_SH_I_BFCAS_Min",0},{"CFG_BIAS_SH_I_BFCAS_Max", 255},}}},
                {CFG_BIAS_SH_I_BDIFF,{"CFG_BIAS_SH_I_BDIFF",{{"CFG_BIAS_SH_I_BDIFF_Min", 0},{"CFG_BIAS_SH_I_BDIFF_Max", 255},}}},
                {CFG_BIAS_SD_I_BDIFF,{"CFG_BIAS_SD_I_BDIFF",{{"CFG_BIAS_SD_I_BDIFF_Min",0},{"CFG_BIAS_SD_I_BDIFF_Max",255},}}},
                {CFG_BIAS_SD_I_BFCAS,{"CFG_BIAS_SD_I_BFCAS",{{"CFG_BIAS_SD_I_BFCAS_Min",0},{"CFG_BIAS_SD_I_BFCAS_Max", 255},}}},
                {CFG_BIAS_SD_I_BSF,{"CFG_BIAS_SD_I_BSF",{{"CFG_BIAS_SD_I_BSF_Min",0}, {"CFG_BIAS_SD_I_BSF_Max", 63},}}},
                {CFG_BIAS_CFD_DAC_1,{"CFG_BIAS_CFD_DAC",{{"CFG_BIAS_CFD_DAC_1_Min",0},{"CFG_BIAS_CFD_DAC_1_Max", 63},}}},
                {CFG_BIAS_CFD_DAC_2,{"CFG_BIAS_CFD_DAC",{{"CFG_BIAS_CFD_DAC_2_Min", 0},{"CFG_BIAS_CFD_DAC_2_Max", 63},}}},
                {CFG_HYST,{"CFG_HYST",{{"CFG_HYST_Min",0},{"CFG_HYST_Max", 63},}}},
                {CFG_THR_ARM_DAC,{"CFG_THR_ARM_DAC",{{"CFG_THR_ARM_DAC_Min",0},{"CFG_THR_ARM_DAC_Max", 255},}}},
                {CFG_THR_ZCC_DAC,{"CFG_THR_ZCC_DAC",{{"CFG_THR_ZCC_DAC_Min",0}, {"CFG_THR_ZCC_DAC_Max", 255},}}},
                {CFG_BIAS_PRE_VREF,{"CFG_BIAS_PRE_VREF",{{"CFG_BIAS_PRE_VREF_Min",0},{"CFG_BIAS_PRE_VREF_Max", 255},}}},
                {CFG_VREF_ADC,{"CFG_VREF_ADC", {{"CFG_VREF_ADC_Min",0},{"CFG_VREF_ADC_Max", 3},}}}
            };

  


        protected:

        private:
            /**
             * @param classname is the class to check to see whether it is a GEMApplication inherited application
             * @throws
             */
            bool isGEMApplication(const std::string& classname) const;

            xdata::Integer m_nShelves;

            log4cplus::Logger m_logger;

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

#endif  // GEM_CALIB_CALIBRATION_H
