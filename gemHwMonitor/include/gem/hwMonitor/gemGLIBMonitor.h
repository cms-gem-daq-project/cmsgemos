#ifndef gem_hwMonitor_gemGLIBMonitor_h
#define gem_hwMonitor_gemGLIBMonitor_h

#include "gemHwMonitorBase.h"

namespace gem { 
    namespace base {
       namespace utils {
           class gemXMLparcer;
           class gemGLIBProperties;
        }
    }
    namespace hwMonitor {
        class gemGLIBMonitor: public gemHwMonitorBase
            {
                public:
                gemGLIBMonitor()
                    throw (xgi::exception::Exception);
                ~gemGLIBMonitor()
                    throw (xgi::exception::Exception);

                void setGLIBRefs()
                    throw (xgi::exception::Exception);
                void getGLIBRefs()
                    throw (xgi::exception::Exception);
                
                private:
                std::vector<gem::base::utils::gemGLIBProperties*> glibRefs_;
            }
    }
}
#endif
