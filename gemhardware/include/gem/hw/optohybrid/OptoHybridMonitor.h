/** @file OptoHybridMonitor.h */

#ifndef GEM_HW_OPTOHYBRID_OPTOHYBRIDMONITOR_H
#define GEM_HW_OPTOHYBRID_OPTOHYBRIDMONITOR_H

#include "gem/base/GEMMonitor.h"
#include "gem/hw/optohybrid/exception/Exception.h"
#include "gem/hw/optohybrid/HwOptoHybrid.h"

namespace gem {
  namespace hw {
    namespace optohybrid {

      class HwOptoHybrid;
      class OptoHybridManager;

      class OptoHybridMonitor : public gem::base::GEMMonitor
      {
      public:

        /**
         * Constructor from GEMFSMApplication derived classes
         * @param optohybrid the HwOptoHybrid uhal device which is to be monitored
         * @param optohybridManager the manager application for the OptoHybrid to be monitored
         */
        OptoHybridMonitor(std::shared_ptr<HwOptoHybrid> optohybrid, OptoHybridManager* optohybridManager, int const& index);

        virtual ~OptoHybridMonitor();

        virtual void updateMonitorables();
        virtual void reset();
        void setupHwMonitoring();

        /**
         * @brief display the monitor items
         */
        void buildMonitorPage(xgi::Output* out);

        /**
         * @brief special layout for monitor items in 'TTC Counters' monitor set
         */
        void buildTTCCounterTable(xgi::Output* out);

        /**
         * @brief special layout for monitor items in 'Other Counters' monitor set
         */
        void buildOtherCounterTable(xgi::Output* out);

        std::string getDeviceID() { return p_optohybrid->getDeviceID(); }

      private:
        std::shared_ptr<HwOptoHybrid> p_optohybrid;

      };  // class OptoHybridMonitor

    }  // namespace gem::hw::optohybrid
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_OptoHybrid_OptoHybridMONITOR_H
