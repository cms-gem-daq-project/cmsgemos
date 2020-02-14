/** @file DAQMonitor.h */

#ifndef GEM_DAQMON_DAQMONITOR_H
#define GEM_DAQMON_DAQMONITOR_H

#include "gem/base/GEMMonitor.h"
#include "gem/base/GEMApplication.h"
//#include "xhal/rpc/utils.h"
#include "xhal/client/XHALInterface.h"

#include <iostream>
#include <fstream>
#include <time.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

namespace bo = boost::iostreams;


namespace gem {
  namespace base {
    class GEMApplication;
  }
  namespace daqmon {

    class DaqMonitor : public gem::base::GEMMonitor, public xhal::client::XHALInterface
    {
    public:

      /**
       * Constructor from GEMMonitor derived class
       * @param board_domain_name The domain name of the AMC card
       * @param optohybridManager logger Log4cplus logger
       * @param gemApp Calling GEMApplication instance
       * @param index Index
       */

      struct LabelData
      {
        std::string labelId;
        std::string labelClass;
        std::string labelValue;
      };

      DaqMonitor(const std::string& board_domain_name,log4cplus::Logger& logger, base::GEMApplication* gemApp, int const& index);

      virtual ~DaqMonitor();

      virtual void reconnect();

      virtual void updateMonitorables();
      void updateDAQmain();
      void updateDAQOHmain();
      void updateTTCmain();
      void updateTRIGGERmain();
      void updateTRIGGEROHmain();
      void updateOHmain();
      void updateOHSCA();
      void updateOHSysmon();
      //all voltages are divided by 3 on the OH board by a voltage divider (temperature sensor voltage is not divided).
      //All values are in units of ADC counts, and this is a 12bit ADC with a range of 0.0 - 1.0V, so each count is 1V / 4095 = 0.244mV.
      double scaVconv(uint32_t val);
      //There's not much information about the SCA chip temperature units, but some information can be found in the SCA manual page 52, which shows linear dependence of temperature from -37.5 deg C to 79 deg C corresponding to 790 counts for -37.5degC and going down to about 570 counts for 79degC.
      double scaTconv(uint32_t val);
      //For PT100 measurements, a more complex conversion is necessary to covert the ADC counts to temperature. The way it works is that the ADC supplies 100uA current that goes through the PT100 sensor to ground, and the voltage drop over the sensor then corresponds to the temperature. The exact part number of the PT100 sensor we are using is P0K1.1206.2P.B, which has a resistance of 100 Ohms at 0 deg C, and changes by 0.385% per each degree C cumulatively (more info can be found here: http://www.farnell.com/datasheets/2207165.pdf?_ga=2.247382788.1064362929.1535377707-998824428.1535377707 and here: https://www.intech.co.nz/products/temperature/typert/RTD-Pt100-Conversion.pdf). This means that at 0 deg C the voltage drop over the PT100 will be 100uA * 100 Ohms = 10mV, so the ADC will read 1.0V - 10mV = 0.99V, and if the temperature is 30 degC, the PT100 will have a resistance of 111.67 Ohms, and so the voltage drop will be 100uA * 111.67 Ohms = 11.167mV, so the ADC will read 1.0V - 11.167mV = 0.988833V.
      double scaPTconv(uint32_t val);
      double sysmonTconv(uint32_t val);
      double sysmonVconv(uint32_t val);
      virtual void reset();
      void setupDaqMonitoring();
      void addDaqMonitorable(const std::string& m_name, const std::string& m_monset, const std::string& m_spacename);

      /**
       * @brief display the monitor items
       */
      void buildMonitorPage(xgi::Output* out);

      void buildTable(const std::string& table_name, xgi::Output* out);

      typedef std::shared_ptr<gem::base::utils::GEMInfoSpaceToolBox> is_toolbox_ptr;

      void updateDAQmainTableContent();

      void updateTTCmainTableContent();

      void updateOHmainTableContent();

      void jsonContentUpdate(xgi::Output* out);

      bool is_connected(){return isConnected;}

      std::string boardName(){return m_board_domain_name;}

    protected:
      void init();
      static const int NOH=12;
    private:
      is_toolbox_ptr is_daqmon;
      std::string is_name;
      std::unordered_map<std::string,LabelData*> m_LabelData;
      std::vector<std::string> v_daq_main;
      std::vector<std::string> v_daq_oh_main;
      std::vector<std::string> v_daq_ttc_main;
      std::vector<std::string> v_daq_trigger_oh_main;
      std::vector<std::string> v_oh_main;
      std::vector<std::string> v_oh_sca;
      std::vector<std::string> v_oh_sysmon;

      int logCnt;

    };  // class DaqMonitor

  }  // namespace gem::daqmon
}  // namespace gem

#endif  // GEM_DAQMON_MONITOR_H
