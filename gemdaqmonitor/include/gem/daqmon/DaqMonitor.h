/** @file DAQMonitor.h */

#ifndef GEM_DAQMON_DAQMONITOR_H
#define GEM_DAQMON_DAQMONITOR_H

#include "gem/base/GEMMonitor.h"
#include "gem/base/GEMApplication.h"
#include "xhal/rpc/utils.h"
#include "xhal/XHALInterface.h"

namespace gem {
  namespace base {
    class GEMApplication;
  }
  namespace daqmon {

    class DaqMonitor : public gem::base::GEMMonitor, public xhal::XHALInterface
    {
    public:
    
      /**
       * Constructor from GEMMonitor derived class
       * @param board_domain_name The domain name of the AMC card
       * @param optohybridManager logger Log4cplus logger
       * @param gemApp Calling GEMApplication instance
       * @param index Index
       */
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
      virtual void reset();
      void setupDaqMonitoring();
    
      /**
       * @brief display the monitor items
       */
      void buildMonitorPage(xgi::Output* out);

      void buildDAQmainTable(xgi::Output* out);

      void buildTTCmainTable(xgi::Output* out);

      typedef std::shared_ptr<gem::base::utils::GEMInfoSpaceToolBox> is_toolbox_ptr;
    
    protected:
      void init();
      static const int NOH=12;
    private:
      is_toolbox_ptr is_daqmon;
      std::string is_name;
    
    };  // class DaqMonitor

  }  // namespace gem::daqmon
}  // namespace gem

#endif  // GEM_DAQMON_MONITOR_H
