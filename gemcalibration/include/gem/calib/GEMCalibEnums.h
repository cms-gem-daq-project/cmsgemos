/** @file GEMCalibEnums.h */

#ifndef GEM_CALIB_CALIBENUMS_H
#define GEM_CALIB_CALIBENUMS_H

namespace gem {
    namespace calib {

        enum calType {NDEF, GBTPHASE, LATENCY, SCURVE, SBITARMDACSCAN, ARMDACSCAN, TRIMDAC, DACSCANV3, CALIBRATEARMDAC};
        
        typedef enum calType calType_t;
       
        enum dacScanType {CFG_CAL_DAC, CFG_BIAS_PRE_I_BIT, CFG_BIAS_PRE_I_BLCC, CFG_BIAS_PRE_I_BSF, CFG_BIAS_SH_I_BFCAS, CFG_BIAS_SH_I_BDIFF, CFG_BIAS_SD_I_BDIFF, CFG_BIAS_SD_I_BFCAS, CFG_BIAS_SD_I_BSF, CFG_BIAS_CFD_DAC_1, CFG_BIAS_CFD_DAC_2, CFG_HYST, CFG_THR_ARM_DAC, CFG_THR_ZCC_DAC, CFG_BIAS_PRE_VREF, CFG_VREF_ADC};
        
        typedef enum dacScanType dacScanType_t;
    }
}


#endif // GEM_CALIB_CALIBENUMS_H
