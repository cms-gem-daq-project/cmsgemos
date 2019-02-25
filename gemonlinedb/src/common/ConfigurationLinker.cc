#include "gem/onlinedb/ConfigurationLinker.h"

namespace gem {
    namespace onlinedb {
        namespace /* anonymous */ {
            /// @brief Links a VFAT.
            std::shared_ptr<VFAT3ChipConfiguration> linkVFAT3(
                const std::shared_ptr<ConfigurationProvider> &provider,
                const SystemTopology::VFAT3Node &node)
            {
                // Fetch and copy the configuration from the provider
                auto result = std::make_shared<VFAT3ChipConfiguration>(
                    *provider->getVFAT3ChipConfiguration(node.reference));

                // Handle channel configuration
                result->setChannelConfiguration(
                    *provider->getVFAT3ChannelConfiguration(node.reference));

                return result;
            }

            /// @brief Links a GBTX.
            std::shared_ptr<GBTXConfiguration> linkGBTX(
                const std::shared_ptr<ConfigurationProvider> &provider,
                const SystemTopology::GBTXNode &node)
            {
                // Fetch and copy the configuration from the provider
                return std::make_shared<GBTXConfiguration>(
                    *provider->getGBTXConfiguration(node.reference));
            }

            /// @brief Links an OH and all child objects.
            std::shared_ptr<OHv3Configuration> linkOHv3(
                const std::shared_ptr<ConfigurationProvider> &provider,
                const SystemTopology::OHv3Node &node)
            {
                // Fetch and copy the configuration from the provider
                auto result = std::make_shared<OHv3Configuration>(
                    *provider->getOHv3Configuration(node.reference));
                result->unsetVFATConfigs();

                // Handle child GBTXs
                for (std::size_t i = 0; i < node.gbtx.size(); ++i) {
                    if (node.gbtx[i] == nullptr) {
                        // GBTX not present
                        continue;
                    }
                    result->setGBTXConfig(i, linkGBTX(provider, *node.gbtx[i]));
                }

                // Handle child VFATs
                for (std::size_t i = 0; i < node.vfat.size(); ++i) {
                    if (node.vfat[i] == nullptr) {
                        // VFAT not present
                        continue;
                    }
                    result->setVFATConfig(i, linkVFAT3(provider, *node.vfat[i]));
                }

                return result;
            }

            /// @brief Links an AMC and all child objects.
            std::shared_ptr<AMCConfiguration> linkAMC(
                const std::shared_ptr<ConfigurationProvider> &provider,
                const SystemTopology::AMCNode &node)
            {
                // Fetch and copy the configuration from the provider
                auto result = std::make_shared<AMCConfiguration>(
                    *provider->getAMCConfiguration(node.reference));
                result->unsetOHConfigs();

                // Handle child OHs
                for (std::size_t i = 0; i < node.oh.size(); ++i) {
                    if (node.oh[i] == nullptr) {
                        // OH not connected
                        continue;
                    }
                    result->setOHConfig(i, linkOHv3(provider, *node.oh[i]));
                }

                return result;
            }

            /// @brief Links an AMC 13 and all child objects.
            std::unique_ptr<AMC13Configuration> linkAMC13(
                const std::shared_ptr<ConfigurationProvider> &provider,
                const SystemTopology::AMC13Node &node)
            {
                // Fetch and copy the configuration from the provider
                auto result = std::unique_ptr<AMC13Configuration>();
                result.reset(new AMC13Configuration(
                    *provider->getAMC13Configuration(node.reference)));

                // Handle child AMCs
                std::vector<std::shared_ptr<AMCConfiguration>> children;
                for (const auto &amcNode : node.amc) {
                    children.push_back(linkAMC(provider, amcNode));
                }
                result->setAMCConfigs(children);

                return result;
            }
        } // anonymous namespace

        ConfigurationLinker::ConfigurationLinker(
            const std::shared_ptr<ConfigurationProvider> &provider) :
            m_provider(provider)
        {}

        std::vector<std::unique_ptr<AMC13Configuration>>
        ConfigurationLinker::link(
            const std::shared_ptr<SystemTopology> &topology) const
        {
            std::vector<std::unique_ptr<AMC13Configuration>> result;
            for (const auto &root : topology->roots()) {
                result.emplace_back(linkAMC13(m_provider, root));
            }
            return result;
        }

    } // namespace onlinedb
} // namespace gem
