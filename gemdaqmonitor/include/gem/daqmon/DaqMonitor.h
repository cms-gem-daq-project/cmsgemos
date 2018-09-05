/** @file DAQMonitor.h */

#ifndef GEM_DAQMON_DAQMONITOR_H
#define GEM_DAQMON_DAQMONITOR_H

#include "gem/base/GEMMonitor.h"
#include "xhal/rpc/utils.h"
#include "xhal/XHALInterface.h"

namespace gem {
  namespace daqmon {

    class DaqMonitor : public gem::base::GEMMonitor public xhal::XHALInterface
    {
    public:
    
      /**
       * Constructor from GEMFSMApplication derived classes
       * @param optohybrid the HwOptoHybrid uhal device which is to be monitored
       * @param optohybridManager the manager application for the OptoHybrid to be monitored
       */
      DaqMonitor(const std::string& board_domain_name);
    
      virtual ~DaqMonitor();
    
      virtual void updateMonitorables();
      virtual void reset();
      void setupDaqMonitoring();
    
      /**
       * @brief display the monitor items
       */
      void buildMonitorPage(xgi::Output* out);
    
    protected:
      void init();
    
    };  // class DaqMonitor

  }  // namespace gem::daqmon
}  // namespace gem

#endif  // GEM_DAQMON_MONITOR_H
