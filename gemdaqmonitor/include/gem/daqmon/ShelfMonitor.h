/** @file ShelfMonitor.h */

#ifndef GEM_DAQMON_SHELFMONITOR_H
#define GEM_DAQMON_SHELFMONITOR_H

#include <string>
#include <vector>

#include "gem/base/GEMApplication.h"
#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

#include "gem/daqmon/exception/Exception.h"

#include "gem/daqmon/DaqMonitor.h"

#define NAMC 12

namespace gem {
  namespace daqmon {

    class DaqMonitor;

    class ShelfMonitor : public gem::base::GEMApplication
      {

      public:
        XDAQ_INSTANTIATOR();

        ShelfMonitor(xdaq::ApplicationStub* s);

        virtual ~ShelfMonitor();

        virtual void init();

        virtual void actionPerformed(xdata::Event& event);

        void startMonitoring();

        void stopMonitoring();

        std::string monitoringState(){return m_state;}

        void stopAction(xgi::Input *in, xgi::Output *out);

        void resumeAction(xgi::Input *in, xgi::Output *out);

        void pauseAction(xgi::Input *in, xgi::Output *out);


        std::vector<DaqMonitor*> v_daqmon;
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
      };
  }  // namespace gem::daqmon
}  // namespace gem

#endif  // GEM_DAQMON_SHELFMONITOR_H
