#ifndef gem_supervisor_GEMDataParker_h
#define gem_supervisor_GEMDataParker_h

#include "xdata/String.h"
#include <string>

namespace gem {
    namespace hw {
        namespace vfat {
            class HwVFAT2;
        }   
    }
    namespace supervisor {
            struct VFATData;
            struct GEBData;
            struct GEMData;
        }
    namespace supervisor {
        class GEMDataParker
        {
            public:
	        GEMDataParker(gem::hw::vfat::HwVFAT2& vfatDevice, std::string& outFileName, std::string& outputType);
                ~GEMDataParker() {};

                int dumpDataToDisk();

		int  getGLIBData  (gem::supervisor::GEMData& gem, gem::supervisor::GEBData& geb, 
                     gem::supervisor::VFATData& vfat);
                void fillGEMevent (gem::supervisor::GEMData& gem, gem::supervisor::GEBData& geb, 
                     gem::supervisor::VFATData& vfat);
                void writeGEMevent(gem::supervisor::GEMData& gem, gem::supervisor::GEBData& geb,
                     gem::supervisor::VFATData& vfat);

            private:

                gem::hw::vfat::HwVFAT2* vfatDevice_;
                std::string outFileName_;
                std::string outputType_;
                int counter_;
        };
    }
}
#endif
