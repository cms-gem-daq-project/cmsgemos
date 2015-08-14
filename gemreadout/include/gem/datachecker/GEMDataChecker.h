#ifndef gem_datachecker_GEMDataChecker_h
#define gem_datachecker_GEMDataChecker_h

//#include "gem/utils/GEMLogging.h"
#include <stdint.h>

namespace gem {
//  namespace readout {
//    struct VFATData;
//    struct GEBData;
//    struct GEMData;
//  }
  namespace datachecker {
    class GEMDataChecker {
    public:

      GEMDataChecker(){}
      //GEMDataChecker(gem::readout::GEMData& gem, gem::readout::GEBData& geb, 
      //               gem::readout::VFATData& vfat);
      ~GEMDataChecker() {};

      //int  counterGEMdata  (gem::readout::GEMData& gem, gem::readout::GEBData& geb, 
      //                      gem::readout::VFATData& vfat);

      uint16_t checkCRC(uint16_t dataVFAT[11], bool OKprint);
      
    private:

      // log4cplus::Logger gemLogger_;
      int event_counter_;
      uint16_t crc_calc(uint16_t crc_in, uint16_t dato);
      
      };
   }
}
#endif
