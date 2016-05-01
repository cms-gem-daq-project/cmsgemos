#ifndef GEM_HWMONITOR_GEMHWMONITORWEB_H
#define GEM_HWMONITOR_GEMHWMONITORWEB_H

#include <string>
#include <vector>
#include <sys/stat.h>

#include "cgicc/HTMLClasses.h"

#include "xdaq/WebApplication.h"
#include "xgi/framework/Method.h"

#include "gemHwMonitorBase.h"
#include "gemHwMonitorHelper.h"

#include "gem/hw/GEMHwDevice.h"
#include "gem/hw/glib/HwGLIB.h"
#include "gem/hw/optohybrid/HwOptoHybrid.h"
#include "gem/hw/vfat/HwVFAT2.h"
#include "gem/hw/vfat/VFAT2Settings.h"
#include "gem/hw/vfat/VFAT2SettingsEnums.h"
#include "gem/hw/vfat/VFAT2Enums2Strings.h"
#include "gem/hw/vfat/VFAT2Strings2Enums.h"

namespace cgicc {
  BOOLEAN_ELEMENT(section, "section");
}
namespace gem {
  namespace hw {
    class GEMHwDevice;
    namespace glib {
      class HwGLIB;
    }
    namespace optohybrid {
      class HwOptoHybrid;
    }
    namespace vfat {
      class HwVFAT2;
      class VFAT2Settings;
    }
  }

  typedef std::shared_ptr<hw::vfat::HwVFAT2 > vfat_shared_ptr;
  typedef std::shared_ptr<hw::glib::HwGLIB >  glib_shared_ptr;
  typedef std::shared_ptr<hw::optohybrid::HwOptoHybrid > optohybrid_shared_ptr;

  namespace hwMonitor {
    class gemHwMonitorWeb: public xdaq::WebApplication
      {
      public:
        XDAQ_INSTANTIATOR();
        gemHwMonitorWeb(xdaq::ApplicationStub* s)
          throw (xdaq::exception::Exception);
        ~gemHwMonitorWeb();
        void Default(xgi::Input* in, xgi::Output* out )
          throw (xgi::exception::Exception);
        void Dummy(xgi::Input* in, xgi::Output* out )
          throw (xgi::exception::Exception);
        void controlPanel(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        void showCratesAvailability(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        void pingCrate(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        void showCrateUtilities(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        void setConfFile(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        void uploadConfFile(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        void displayConfFile(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        void getCratesConfiguration(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        void expandCrate(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        void selectCrate(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        void cratePanel(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        void expandGLIB(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        void glibPanel(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        void expandOH(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        void ohPanel(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        void expandVFAT(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        void vfatPanel(xgi::Input* in, xgi::Output* out)
          throw (xgi::exception::Exception);
        /*
          static void createVFATInfoLayout(       xgi::Output* out,
          const gem::hw::vfat::VFAT2ControlParams params) {}
          static void createControlRegisterLayout(xgi::Output* out,
          const gem::hw::vfat::VFAT2ControlParams params) {}
          static void createSettingsLayout(       xgi::Output* out,
          const gem::hw::vfat::VFAT2ControlParams params) {}
          static void createCounterLayout(        xgi::Output* out,
          const gem::hw::vfat::VFAT2ControlParams params) {}
          static void createChannelRegisterLayout(xgi::Output* out,
          const gem::hw::vfat::VFAT2ControlParams params) {}
          static void createCommandLayout(        xgi::Output* out,
          const gem::hw::vfat::VFAT2ControlParams params) {}

          static void getCurrentParametersAsXML() {}
          static void saveCurrentParametersAsXML() {}
          static void setParametersByXML() {}
        */
      private:
        gemHwMonitorSystem* m_gemHwMonitorSystem;
        std::vector<gemHwMonitorCrate*> m_gemHwMonitorCrate;
        std::vector<gemHwMonitorGLIB*>  m_gemHwMonitorGLIB;
        std::vector<gemHwMonitorOH*>    m_gemHwMonitorOH;
        std::vector<gemHwMonitorVFAT*>  m_gemHwMonitorVFAT;
        gemHwMonitorHelper* p_gemSystemHelper;
        bool m_crateCfgAvailable;
        int m_nCrates;
        int m_indexCrate;
        int m_indexGLIB;
        int m_indexOH;
        int m_indexVFAT;
        std::string m_crateToShow;
        std::string m_glibToShow;
        std::string m_ohToShow;
        std::string m_vfatToShow;
        std::string m_glibIP;
        glib_shared_ptr       p_glibDevice;
        optohybrid_shared_ptr p_ohDevice;
        vfat_shared_ptr       p_vfatDevice;
        std::vector<std::string> m_checkedCrates;

        void printVFAThwParameters(const char* key, const char* value1, const char* value2, xgi::Output* out)
          throw (xgi::exception::Exception);
        void printVFAThwParameters(const char* key, const char* value,  xgi::Output* out)
          throw (xgi::exception::Exception);
        void printVFAThwParameters(const char* key, uint8_t value,  xgi::Output* out)
          throw (xgi::exception::Exception);
        void printVFAThwParameters(const char* key, const char* value1, uint8_t value2, xgi::Output* out)
          throw (xgi::exception::Exception);
      };  // class gemHwMonitorWeb
  }  // namespace gem::hwMonitor
}  // namespace gem

#endif  // GEM_HWMONITOR_GEMHWMONITORWEB_H
