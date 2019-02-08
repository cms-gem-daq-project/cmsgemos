#ifndef GEM_ONLINEDB_CONFIGURATIONLINKER_H
#define GEM_ONLINEDB_CONFIGURATIONLINKER_H

#include <memory>

#include "gem/onlinedb/AMC13Configuration.h"
#include "gem/onlinedb/ConfigurationProvider.h"
#include "gem/onlinedb/SystemTopology.h"

namespace gem {
    namespace onlinedb {

        /**
         * @brief "Links" configuration objects together.
         *
         * "Linking" is the operation that creates a configuration object tree
         * that corresponds to the @ref SystemTopology.
         */
        class ConfigurationLinker
        {
            std::shared_ptr<ConfigurationProvider> m_provider;

        public:
            /**
             * @brief Constructor.
             * @param provider The @ref ConfigurationProvider that will provide
             *                 the configurations during the linking step.
             */
            explicit ConfigurationLinker(
                const std::shared_ptr<ConfigurationProvider> &provider = nullptr);

            /// @brief Destructor.
            virtual ~ConfigurationLinker() = default;

            /// @brief Retrieves the configuration provider.
            std::shared_ptr<ConfigurationProvider> provider() const
            { return m_provider; };

            /// @brief Changes the configuration provider.
            void setProvider(
                const std::shared_ptr<ConfigurationProvider> &provider)
            { m_provider = provider; };

            /**
             * @brief Links configuration objects together according to the
             *        given @c topology.
             *
             * Configuration objects are pulled from the
             * @ref ConfigurationProvider associated with this linker.
             *
             * Returns a pointer to the top-level node of the configuration
             * object tree.
             */
            std::unique_ptr<AMC13Configuration> link(
                const std::shared_ptr<SystemTopology> &topology) const;
        };

    } // namespace onlinedb
} // namespace gem

#endif // GEM_ONLINEDB_CONFIGURATIONLINKER_H
