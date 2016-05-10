#ifndef GEM_HWMONITOR_GEMGLIBMONITOR_H
#define GEM_HWMONITOR_GEMGLIBMONITOR_H

#include "gemHwMonitorBase.h"

namespace gem {
  namespace base {
    namespace utils {
      class gemXMLparser;
      class gemCrateProperties;
      class gemGLIBProperties;
    }
  }
  namespace hwMonitor {
    class gemGLIBMonitor: public gemHwMonitorBase
    {
    public:
      gemGLIBMonitor(gem::utils::gemGLIBProperties& glibRef)
        throw (xgi::exception::Exception);
      ~gemGLIBMonitor()
        throw (xgi::exception::Exception);

      void setGLIBRef()
        throw (xgi::exception::Exception);
      void getGLIBRef()
        throw (xgi::exception::Exception);

    private:
      gem::utils::gemGLIBProperties m_glibRef;
    };
  }  // namespace gem::hwMonitor
}  // namespace gem

#endif  // GEM_HWMONITOR_GEMGLIBMONITOR_H
