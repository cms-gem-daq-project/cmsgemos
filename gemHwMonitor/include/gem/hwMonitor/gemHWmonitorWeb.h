#ifndef gem_hwMon_gemHWmonitor_h
#define gem_hwMon_gemHWmonitor_h

#include "xdaq/WebApplication.h"
#include "xgi/framework/Method.h"
#include "cgicc/HTMLClasses.h"

namespace cgicc {
    BOOLEAN_ELEMENT(section,"section");
}
namespace gem {
    namespace hw {
        class gemHWmonitorWeb public xdata::WebApplication
        {
            public:
                XDAQ_INSTANTIATOR();
                gemHWmonitorWeb(xdaq::ApplicationStub *s)
                    throw (xdaq::exception::Exception);
                ~gemHWmonitorWeb();
                static void createHeader(xgi::Output *out );

                static void createVFATInfoLayout(       xgi::Output *out,
                        const gem::hw::vfat::VFAT2ControlParams params);
                static void createControlRegisterLayout(xgi::Output *out,
                        const gem::hw::vfat::VFAT2ControlParams params);
                static void createSettingsLayout(       xgi::Output *out,
                        const gem::hw::vfat::VFAT2ControlParams params);
                static void createCounterLayout(        xgi::Output *out,
                        const gem::hw::vfat::VFAT2ControlParams params);
                static void createChannelRegisterLayout(xgi::Output *out,
                        const gem::hw::vfat::VFAT2ControlParams params);
                static void createCommandLayout(        xgi::Output *out,
                        const gem::hw::vfat::VFAT2ControlParams params);

                static void getCurrentParametersAsXML();
                static void saveCurrentParametersAsXML();
                static void setParametersByXML();

        };// end class GEMControllerPanelWeb
    }// end namespace hwMon
}// end namespace gem
#endif
