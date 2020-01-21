#include "gem/onlinedb/FileConfigurationProvider.h"

#include <fstream>

#include "gem/onlinedb/SerializationData.h"
#include "gem/onlinedb/detail/FileUtils.h"
#include "gem/onlinedb/exception/Exception.h"

namespace gem {
    namespace onlinedb {
        namespace /* anonymous */ {
            /// @brief Turns a @c PartType into a string
            template<class PartType>
            std::string toString(const PartType &reference);

            template<>
            std::string toString<PartReferenceBarcode>(
                const PartReferenceBarcode &reference)
            {
                return reference.barcode;
            }

            template<>
            std::string toString<PartReferenceSN>(
                const PartReferenceSN &reference)
            {
                return reference.serialNumber;
            }
        } // anonymous namespace

        template<class ConfigurationType>
        void FileConfigurationProvider::loadInternal(
            const std::string &filename,
            std::map<std::string, std::shared_ptr<ConfigurationType>> &map)
        {
            // Find the file (throws if not found)
            auto filepath = detail::getFileInPath(filename, getSearchPath());
            m_sources.push_back(filepath);

            // Parse the JSON
            nlohmann::json json;
            std::ifstream in(filepath);
            in >> json;

            // Decode it
            const SerializationData<ConfigurationType> serData = json;

            // Add the parts
            auto dataSets = serData.getDataSets();
            for (const auto &set : dataSets) {
                auto part = set.getPart();
                if (map.count(toString(part)) > 0) {
                    // Corresponds to more than one configuration for the same
                    // part, which is not supported.
                    XCEPT_RAISE(
                        exception::ParseError,
                        "In " + filename + ": Duplicated part " + toString(part));
                }

                auto data = set.getData();
                if (data.size() > 1) {
                    // Corresponds to more than one configuration for the same
                    // part, which is not supported.
                    XCEPT_RAISE(
                        exception::ParseError,
                        "In " + filename +
                        ": DATA_SET section contains more than one DATA entry");
                } else if (data.size() == 1) {
                    map[toString(part)] =
                        std::make_shared<ConfigurationType>(data.front());
                }
            }
        }

        void FileConfigurationProvider::load(const DOMDocumentPtr &document)
        {
            auto systemEl = document->getDocumentElement();
            auto configEl = detail::findChildElement(systemEl, "gem:configuration");

            for (auto fileEl = configEl->getFirstElementChild();
                 fileEl != nullptr;
                 fileEl = fileEl->getNextElementSibling()) {
                auto tagName = detail::transcode(fileEl->getTagName());
                if (tagName == "gem:vfat3-channel-file") {
                    loadVFAT3Channel(detail::transcode(fileEl->getTextContent()));
                } else if (tagName == "gem:vfat3-chip-file") {
                    loadVFAT3Chip(detail::transcode(fileEl->getTextContent()));
                } else if (tagName == "gem:ohv3-file") {
                    loadOHv3(detail::transcode(fileEl->getTextContent()));
                } else if (tagName == "gem:gbtx-file") {
                    loadGBTX(detail::transcode(fileEl->getTextContent()));
                } else if (tagName == "gem:amc-file") {
                    loadAMC(detail::transcode(fileEl->getTextContent()));
                } else if (tagName == "gem:amc13-file") {
                    loadAMC13(detail::transcode(fileEl->getTextContent()));
                }
            }
        }

        void FileConfigurationProvider::loadAMC13(const std::string &filename)
        {
            loadInternal(filename, m_amc13Config);
        }

        void FileConfigurationProvider::loadAMC(const std::string &filename)
        {
            loadInternal(filename, m_amcConfig);
        }

        void FileConfigurationProvider::loadGBTX(const std::string &filename)
        {
            loadInternal(filename, m_gbtxConfig);
        }

        void FileConfigurationProvider::loadOHv3(const std::string &filename)
        {
            loadInternal(filename, m_ohv3Config);
        }

        void FileConfigurationProvider::loadVFAT3Chip(const std::string &filename)
        {
            loadInternal(filename, m_vfat3ChipConfig);
        }

        void FileConfigurationProvider::loadVFAT3Channel(const std::string &filename)
        {
            loadInternal(filename, m_vfat3ChannelConfig);
        }

        std::shared_ptr<AMC13Configuration>
        FileConfigurationProvider::getAMC13Configuration(
            const ConfigurationTraits<AMC13Configuration>::PartType &reference) const
        {
            try {
                return m_amc13Config.at(toString(reference));
            } catch (const std::out_of_range &e) {
                XCEPT_RAISE(exception::ReferenceError,
                            "AMC13 config not found: " + toString(reference));
            }
        }

        std::shared_ptr<AMCConfiguration>
        FileConfigurationProvider::getAMCConfiguration(
            const ConfigurationTraits<AMCConfiguration>::PartType &reference) const
        {
            try {
                return m_amcConfig.at(toString(reference));
            } catch (const std::out_of_range &e) {
                XCEPT_RAISE(exception::ReferenceError,
                            "AMC config not found: " + toString(reference));
            }
        }

        std::shared_ptr<GBTXConfiguration>
        FileConfigurationProvider::getGBTXConfiguration(
            const ConfigurationTraits<GBTXConfiguration>::PartType &reference) const
        {
            try {
                return m_gbtxConfig.at(toString(reference));
            } catch (const std::out_of_range &e) {
                XCEPT_RAISE(exception::ReferenceError,
                            "GBTX config not found: " + toString(reference));
            }
        }

        std::shared_ptr<OHv3Configuration>
        FileConfigurationProvider::getOHv3Configuration(
            const ConfigurationTraits<OHv3Configuration>::PartType &reference) const
        {
            try {
                return m_ohv3Config.at(toString(reference));
            } catch (const std::out_of_range &e) {
                XCEPT_RAISE(exception::ReferenceError,
                            "OHv3 config not found: " + toString(reference));
            }
        }

        std::shared_ptr<VFAT3ChipConfiguration>
        FileConfigurationProvider::getVFAT3ChipConfiguration(
            const ConfigurationTraits<VFAT3ChipConfiguration>::PartType &reference) const
        {
            try {
                return m_vfat3ChipConfig.at(toString(reference));
            } catch (const std::out_of_range &e) {
                XCEPT_RAISE(exception::ReferenceError,
                            "VFAT3 chip config not found: " + toString(reference));
            }
        }

        std::shared_ptr<VFAT3ChannelConfiguration>
        FileConfigurationProvider::getVFAT3ChannelConfiguration(
            const ConfigurationTraits<VFAT3ChannelConfiguration>::PartType &reference) const
        {
            try {
                return m_vfat3ChannelConfig.at(toString(reference));
            } catch (const std::out_of_range &e) {
                XCEPT_RAISE(exception::ReferenceError,
                            "VFAT3 channel config not found: " + toString(reference));
            }
        }

    } // namespace onlinedb
} // namespace gem

