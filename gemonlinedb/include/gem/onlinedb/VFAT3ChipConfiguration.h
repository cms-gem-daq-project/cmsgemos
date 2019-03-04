#ifndef GEM_ONLINEDB_VFAT3CHIPCONFIGURATION_H
#define GEM_ONLINEDB_VFAT3CHIPCONFIGURATION_H

#include <string>

#include "gem/onlinedb/VFAT3ChannelConfiguration.h"
#include "gem/onlinedb/detail/VFAT3ChipConfigurationGen.h"

namespace gem {
    namespace onlinedb {

        /**
         * @brief Describes the configuration of a VFAT 3 chip.
         *
         * The configuration is split in two parts: general registers are
         * handled by this class, while channel registers are handled by
         * @ref VFAT3ChannelConfiguration. This class always owns a member of
         * the latter, accessible using getChannelConfiguration().
         *
         * When retrieving information from the database or XML files, the
         * channel configuration has to be loaded separately.
         *
         * @note Modifying objects of this class doesn't affect hardware.
         */
        class VFAT3ChipConfiguration: public detail::VFAT3ChipConfigurationGen
        {
            VFAT3ChannelConfiguration m_channelConfig;

        public:
            /**
             * @brief Modifies the channel configuration.
             */
            void setChannelConfiguration(
                const VFAT3ChannelConfiguration &channelConfig)
            {
                m_channelConfig = channelConfig;
            }

            /**
             * @brief Retrieves a modifiable view of the channel configuration.
             */
            VFAT3ChannelConfiguration &getChannelConfiguration()
            {
                return m_channelConfig;
            }

            /**
             * @brief Retrieves the channel configuration.
             */
            const VFAT3ChannelConfiguration &getChannelConfiguration() const
            {
                return m_channelConfig;
            }
        };

        template<>
        class ConfigurationTraits<VFAT3ChipConfiguration> :
            public ConfigurationTraits<detail::VFAT3ChipConfigurationGen>
        {};

    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_VFAT3CHIPCONFIGURATION_H
