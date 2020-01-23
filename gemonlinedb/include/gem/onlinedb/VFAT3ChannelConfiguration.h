#ifndef GEM_ONLINEDB_VFAT3CHANNELCONFIGURATION_H
#define GEM_ONLINEDB_VFAT3CHANNELCONFIGURATION_H

#include <array>
#include <cstdint>

#include "gem/onlinedb/ChannelRegisterValue.h"
#include "gem/onlinedb/ConfigurationTraits.h"
#include "gem/onlinedb/PartReference.h"
#include "gem/onlinedb/detail/VFAT3ChannelConfigurationGen.h"
#include "gem/onlinedb/detail/RegisterData.h"

namespace gem {
    namespace onlinedb {
        /**
         * @brief Describes the configuration of channels in a VFAT 3
         */
        class VFAT3ChannelConfiguration: public detail::VFAT3ChannelConfigurationGen
        {
        public:
            /**
             * @brief Represents the value of a channel register.
             * @deprecated Use @ref gem::onlinedb::ChannelRegisterValue instead.
             */
            using ChannelRegisterValue = gem::onlinedb::ChannelRegisterValue;
        };

        template<>
        class ConfigurationTraits<VFAT3ChannelConfiguration> :
            public ConfigurationTraits<detail::VFAT3ChannelConfigurationGen>
        {};

    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_VFAT3CHANNELCONFIGURATION_H
