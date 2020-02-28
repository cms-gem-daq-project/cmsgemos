/** @file AMCMonitor.h */

#ifndef GEM_HW_AMC_AMCMONITOR_H
#define GEM_HW_AMC_AMCMONITOR_H

#include "gem/base/GEMMonitor.h"
#include "gem/hw/devices/exception/Exception.h"
#include "gem/hw/devices/glib/HwGLIB.h"

namespace gem {
  namespace hw {
    namespace glib {
      class HwGLIB;
    }

    namespace amc {

      class AMCManager;

      class AMCMonitor : public gem::base::GEMMonitor
      {
      public:

        /**
         * Constructor from GEMFSMApplication derived classes
         * @param amc the HwGLIB uhal device which is to be monitored
         * @param amcManager the manager application for the AMC to be monitored
         */
        AMCMonitor(std::shared_ptr<gem::hw::glib::HwGLIB> amc, AMCManager* amcManager, int const& index);

        virtual ~AMCMonitor();

        virtual void updateMonitorables();
        virtual void reset();
        void setupHwMonitoring();
        void buildMonitorPage(xgi::Output* out);
        void buildDAQStatusTable(xgi::Output* out);
        void buildTriggerStatusTable(xgi::Output* out);
        std::string getDeviceID() { return p_amc->getDeviceID(); }

      private:
        std::shared_ptr<gem::hw::glib::HwGLIB> p_amc;

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

      };  // class AMCMonitor

    }  // namespace gem::hw::amc
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_AMC_AMCMONITOR_H
