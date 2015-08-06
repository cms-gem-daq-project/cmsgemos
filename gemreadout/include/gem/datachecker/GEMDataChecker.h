#ifndef gem_datachecker_GEMDataChecker_h
#define gem_datachecker_GEMDataChecker_h

//#include "gem/utils/GEMLogging.h"

namespace gem {
  namespace readout {
    struct VFATData;
    struct GEBData;
    struct GEMData;
  }
  namespace datachecker {
    class GEMDataChecker {
    public:

      GEMDataChecker(gem::readout::GEMData& gem, gem::readout::GEBData& geb, 
                     gem::readout::VFATData& vfat);
      ~GEMDataChecker() {};

      int  counterGEMdata  (gem::readout::GEMData& gem, gem::readout::GEBData& geb, 
                            gem::readout::VFATData& vfat);

    private:

      // log4cplus::Logger gemLogger_;
      int event_counter_;

    };
  }
}
#endif
