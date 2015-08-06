#ifndef gem_readout_GEMDataParker_h
#define gem_readout_GEMDataParker_h

#include "xdata/String.h"
#include <string>

#include "gem/utils/GEMLogging.h"

namespace gem {
  namespace hw {
    namespace glib {
      class HwGLIB;
    }   
  }
  namespace readout {
    struct VFATData;
    struct GEBData;
    struct GEMData;
  }
  namespace readout {
    class GEMDataParker
    {
    public:
      GEMDataParker(gem::hw::glib::HwGLIB& glibDevice, std::string const& outFileName, std::string const& outputType);
      ~GEMDataParker() {};

      int *dumpDataToDisk(uint8_t const& link);

      int  getGLIBData  (uint8_t const& link,
                         gem::readout::GEMData& gem,
                         gem::readout::GEBData& geb, 
                         gem::readout::VFATData& vfat);
      void fillGEMevent (gem::readout::GEMData& gem,
                         gem::readout::GEBData& geb, 
                         gem::readout::VFATData& vfat);
      void writeGEMevent(gem::readout::GEMData& gem,
                         gem::readout::GEBData& geb,
                         gem::readout::VFATData& vfat);

    private:

      log4cplus::Logger gemLogger_;
      gem::hw::glib::HwGLIB* glibDevice_;
      std::string outFileName_;
      std::string outputType_;

      // Counter
      int counter_[3];

      // Events Counter     
      int vfat_;

      // Events Counter     
      int event_;
         
      // VFATs counter per event
      int sumVFAT_;

    };
  }
}
#endif
