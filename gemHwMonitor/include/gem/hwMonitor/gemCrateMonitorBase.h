#ifndef GEM_HWMONITOR_GEMCRATEMONITORBASE_H
#define GEM_HWMONITOR_GEMCRATEMONITORBASE_H

#include <string>
#include <vector>

#include "xdaq/Application.h"
#include "xgi/framework/Method.h"

#include "gem/utils/gemXMLparser.h"
#include "gem/utils/gemCrateProperties.h"
#include "gem/utils/gemDeviceProperties.h"

namespace gem {
  namespace base {
    namespace utils {
      class gemXMLparser;
      class gemCrateProperties;
      class gemDeviceProperties;
    }
  }
  namespace hwMonitor {
    class gemCrateMonitorBase: public gemHwMonitorBase
    {
    public:
      gemCrateMonitorBase(const gem::utils::gemCrateProperties & crateRef)
        throw (xdaq::exception::Exception);

      ~gemCrateMonitorBase() {}
      /*
       *   Get system configuration
       */
      void getDeviceConfiguration()
        throw (xgi::exception::Exception);
      /**
       *   Access to board utils
       virtual void boardUtils ()
       throw (xgi::exception::Exception);
      */

    protected:
    private:
      gem::utils::gemCrateProperties* p_crateRef;
    };
  }  // namespace gem::hwMonitor
}  // namespace gem

#endif  // GEM_HWMONITOR_GEMCRATEMONITORBASE_H
