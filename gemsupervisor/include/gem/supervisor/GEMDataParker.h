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
            struct ChannelData;
            struct VFATData;
            struct GEMData;
        }
    namespace supervisor {
        class GEMDataParker
        {
            public:
                GEMDataParker(gem::hw::vfat::HwVFAT2& vfatDevice, std::string& outFileName);
                ~GEMDataParker() {};

                int dumpDataToDisk();
		int getGLIBData(gem::supervisor::ChannelData& ch, gem::supervisor::VFATData& vfat);

            private:

                gem::hw::vfat::HwVFAT2* vfatDevice_;
                std::string outFileName_;
                int counter_;
        };
    }
}
#endif
