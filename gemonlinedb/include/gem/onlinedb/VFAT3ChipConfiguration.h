#include <string>

#include "detail/VFAT3ChipConfigurationGen.h"

namespace gem {
    namespace onlinedb {
        class VFAT3ChipConfiguration: public detail::VFAT3ChipConfigurationGen
        {
        public:
        };

        template<>
        class ConfigurationTraits<VFAT3ChipConfiguration> :
            public ConfigurationTraits<detail::VFAT3ChipConfigurationGen>
        {};
    } /* namespace onlinedb */
} /* namespace gem */
