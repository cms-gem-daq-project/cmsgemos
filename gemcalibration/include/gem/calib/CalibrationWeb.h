/** @file CalibrationWeb.h */

#ifndef GEM_CALIB_CALIBRATIONWEB_H
#define GEM_CALIB_CALIBRATIONWEB_H

#include <memory>

#include "gem/base/GEMWebApplication.h"
#include "xdaq/WebApplication.h"


#include "cgicc/HTMLClasses.h"

#include "xdaq/WebApplication.h"
#include "xgi/framework/Method.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"
#include "gem/base/utils/GEMInfoSpaceToolBox.h"
#include "gem/calib/Calibration.h"
namespace gem {
    namespace calib {

        class Calibration;

        class CalibrationWeb: public gem::base::GEMWebApplication
        {
        
        public:
            CalibrationWeb(Calibration *CalibrationApp);
           

            virtual ~CalibrationWeb();
	
            /**
             * @brief Set the interface for the selected calibration routine
             * @param calibration routine to be perfomed
             * @param xgi output for the html 
             * @param number of shelves taken from XML configuration file for the amc optical links
             * @throws
             */

            void settingsInterface(calType_t m_calType, xgi::Output *out, xdata::Integer m_nShelves);

            /**
             * @brief Selector for the slot and OH mask 
             * @param xgi output for the html 
             * @param number of shelves taken from XML configuration file for the amc optical links
             * @throws
             */

            void slotsAndMasksSelector(xgi::Output *out, xdata::Integer m_nShelves);

            /**
             * @brief Generic selector for a calibration routine parameter 
             * @param label associated with the parameter
             * @param parameter name
             * @param parameter default value
             * @param xgi output for the html 
             * @throws
             */
	
            void genericParamSelector(std::string labelName, std::string paramName, std::string defaultValue, xgi::Output *out);
	
            /**
             * @brief Selector for the DAC scan calibration routine parameter with dropdown to select the parameter to scan on
             * @param xgi output for the html 
             * @throws
             */
	
            void dacScanV3Selector(xgi::Output *out);
	
            /**
             * @brief Form selector for the DAC scan calibration routine parameter
             * @param parameter name 
             * @param parameter default value 
             * @param xgi output for the html 
             * @throws
             */
	
            void genericParamSelector_dacScan( std::string paramName, int defaultValue, xgi::Output *out);

            /**
             * @brief Radio selector for calibration routine parameter
             * @param parameter name 
             * @param parameter options 
             * @param xgi output for the html 
             * @throws
             */
	
            void genericRadioSelector(std::string paramName, gem::calib::Calibration::scanParamsRadioSelector radio_param, xgi::Output *out);
            //
	

        protected:
            virtual void webDefault(  xgi::Input *in, xgi::Output *out );

            virtual void calibrationPage(  xgi::Input *in, xgi::Output *out );

            virtual void applicationPage(xgi::Input *in, xgi::Output *out);

        private:
            size_t level;
            const std::map<calType_t, std::string> alertMap {
                {GBTPHASE,"To run the routine select the cards, the optohybrids,the nuber of samples the phase maximum and minimu value and the step size for the scan."},
                {LATENCY,"To run the routine  indicate the number of events, the  pulse stretch configuration, the minimum and maximum scan values, the step size, and the desired VFAT channels."},
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
         
        };  // class gem::calib::CalibrationWeb
    }  // namespace gem::calib
}  // namespace gem

#endif  // GEM_CALIB_CALIBRATIONWEB_H
