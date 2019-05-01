#ifndef GEM_ONLINEDB_CONFIGURATIONPROVIDER_H
#define GEM_ONLINEDB_CONFIGURATIONPROVIDER_H

#include "gem/onlinedb/AMC13Configuration.h"
#include "gem/onlinedb/AMCConfiguration.h"
#include "gem/onlinedb/OHv3Configuration.h"
#include "gem/onlinedb/VFAT3ChipConfiguration.h"

#include "gem/onlinedb/ConfigurationTraits.h"
#include "gem/onlinedb/PartReference.h"

namespace gem {
    namespace onlinedb {

        /**
         * @brief Interface that provides configurations for parts.
         *
         * Objects implementing this interface can be used to retrieve the
         * configuration for parts of a GEM system. It is primarily intended for
         * use with @ref ConfigurationLinker.
         *
         * Retrieval functions throw @ref exception::ReferenceError when the
         * requested configuration is not found.
         */
        class ConfigurationProvider
        {
        public:
            /// @brief Constructor.
            explicit ConfigurationProvider() = default;

            /// @brief Destructor.
            virtual ~ConfigurationProvider() = default;

            /**
             * @brief Retrieves a configuration for the referenced AMC 13
             *
             * The corresponding AMC configurations doesn't need to be linked.
             */
            virtual std::shared_ptr<AMC13Configuration> getAMC13Configuration(
                const ConfigurationTraits<AMC13Configuration>::PartType &reference) const = 0;

            /**
             * @brief Retrieves a configuration for the referenced AMC
             *
             * The corresponding OH configurations doesn't need to be linked.
             */
            virtual std::shared_ptr<AMCConfiguration> getAMCConfiguration(
                const ConfigurationTraits<AMCConfiguration>::PartType &reference) const = 0;

            /**
             * @brief Retrieves a configuration for the referenced OH
             *
             * The corresponding VFAT configurations doesn't need to be linked.
             */
            virtual std::shared_ptr<OHv3Configuration> getOHv3Configuration(
                const ConfigurationTraits<OHv3Configuration>::PartType &reference) const = 0;

            /**
             * @brief Retrieves a chip configuration for the referenced VFAT.
             *
             * The corresponding channel configuration doesn't need to be linked.
             */
            virtual std::shared_ptr<VFAT3ChipConfiguration>
            getVFAT3ChipConfiguration(
                const ConfigurationTraits<VFAT3ChipConfiguration>::PartType &reference) const = 0;

            /// @brief Retrieves a channel configuration for the referenced VFAT
            virtual std::shared_ptr<VFAT3ChannelConfiguration>
            getVFAT3ChannelConfiguration(
                const ConfigurationTraits<VFAT3ChannelConfiguration>::PartType &reference) const = 0;

            /**
             * @brief Retrieves the list of sources from which the data is pulled.
             *
             * The list can be used for display purposes.
             */
            virtual std::vector<std::string> getObjectSources() const = 0;
        };

    } // namespace onlinedb
} // namespace gem

#endif // GEM_ONLINEDB_CONFIGURATIONPROVIDER_H
