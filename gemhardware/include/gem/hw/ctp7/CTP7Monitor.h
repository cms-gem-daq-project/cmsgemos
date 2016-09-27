/** @file CTP7Monitor.h */

#ifndef GEM_HW_CTP7_CTP7MONITOR_H
#define GEM_HW_CTP7_CTP7MONITOR_H

#include "gem/base/GEMMonitor.h"
#include "gem/hw/ctp7/exception/Exception.h"
#include "gem/hw/ctp7/HwCTP7.h"

namespace gem {
  namespace hw {
    namespace ctp7 {

      class HwCTP7;
      class CTP7Manager;

      class CTP7Monitor : public gem::base::GEMMonitor
      {
      public:

        /**
         * Constructor from GEMFSMApplication derived classes
         * @param ctp7 the HwCTP7 uhal device which is to be monitored
         * @param ctp7Manager the manager application for the CTP7 to be monitored
         */
        CTP7Monitor(std::shared_ptr<HwCTP7> ctp7, CTP7Manager* ctp7Manager, int const& index);

        virtual ~CTP7Monitor();

        virtual void updateMonitorables();
        virtual void reset();
        void setupHwMonitoring();
        void buildMonitorPage(xgi::Output* out);
        std::string getDeviceID() { return p_ctp7->getDeviceID(); }

      private:
        std::shared_ptr<HwCTP7> p_ctp7;

        // system_monitorables
        //  "BOARD_ID"
        //  "SYSTEM_ID"
        //  "FIRMWARE_ID"
        //  "FIRMWARE_DATE"
        //  "IP_ADDRESS"
        //  "MAC_ADDRESS"
        //  "SFP1_STATUS"
        //  "SFP2_STATUS"
        //  "SFP3_STATUS"
        //  "SFP4_STATUS"
        //  "FMC1_STATUS"
        //  "FMC2_STATUS"
        //  "FPGA_RESET"
        //  "GBE_INT"
        //  "V6_CPLD"
        //  "CPLD_LOCK"

      };  // class CTP7Monitor
    }  // namespace gem::hw::ctp7
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_CTP7_CTP7MONITOR_H
