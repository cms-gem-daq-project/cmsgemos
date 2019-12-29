#include "gem/onlinedb/FileConfigurationProvider.h"

#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "gem/onlinedb/SerializationData.h"
#include "gem/onlinedb/detail/FileUtils.h"
#include "gem/onlinedb/exception/Exception.h"

XERCES_CPP_NAMESPACE_USE

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

            /// @brief Common part to all loadXXX methods
            template<class ConfigurationType>
            void loadInternal(
                const std::string &filename,
                const SerializationData<ConfigurationType> &serData,
                std::map<std::string, std::shared_ptr<ConfigurationType>> &map)
            {
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

        } // anonymous namespace

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
            // Find the file (throws if not found)
            auto path = detail::getFileInPath(filename, getSearchPath());
            m_sources.push_back(path);

            if (boost::algorithm::ends_with(path, ".json")) {
                nlohmann::json json;
                std::ifstream in(path);
                in >> json;
                loadInternal(path,
                             json.get<SerializationData<AMC13Configuration>>(),
                             m_amc13Config);
            } else {
                XCEPT_RAISE(exception::ParseError, "Unknown file type");
            }
        }

        void FileConfigurationProvider::loadAMC(const std::string &filename)
        {
            // Find the file (throws if not found)
            auto path = detail::getFileInPath(filename, getSearchPath());
            m_sources.push_back(path);

            if (boost::algorithm::ends_with(path, ".json")) {
                nlohmann::json json;
                std::ifstream in(path);
                in >> json;
                loadInternal(path,
                             json.get<SerializationData<AMCConfiguration>>(),
                             m_amcConfig);
            } else {
                XCEPT_RAISE(exception::ParseError, "Unknown file type");
            }
        }

        void FileConfigurationProvider::loadGBTX(const std::string &filename)
        {
            // Find the file (throws if not found)
            auto path = detail::getFileInPath(filename, getSearchPath());
            m_sources.push_back(path);

            if (boost::algorithm::ends_with(path, ".json")) {
                nlohmann::json json;
                std::ifstream in(path);
                in >> json;
                loadInternal(path,
                             json.get<SerializationData<GBTXConfiguration>>(),
                             m_gbtxConfig);
            } else {
                XCEPT_RAISE(exception::ParseError, "Unknown file type");
            }
        }

        void FileConfigurationProvider::loadOHv3(const std::string &filename)
        {
            // Find the file (throws if not found)
            auto path = detail::getFileInPath(filename, getSearchPath());
            m_sources.push_back(path);

            if (boost::algorithm::ends_with(path, ".json")) {
                nlohmann::json json;
                std::ifstream in(path);
                in >> json;
                loadInternal(path,
                             json.get<SerializationData<OHv3Configuration>>(),
                             m_ohv3Config);
            } else {
                XCEPT_RAISE(exception::ParseError, "Unknown file type");
            }
        }

        void FileConfigurationProvider::loadVFAT3Chip(const std::string &filename)
        {
            // Find the file (throws if not found)
            auto path = detail::getFileInPath(filename, getSearchPath());
            m_sources.push_back(path);

            if (boost::algorithm::ends_with(path, ".json")) {
                nlohmann::json json;
                std::ifstream in(path);
                in >> json;
                loadInternal(path,
                             json.get<SerializationData<VFAT3ChipConfiguration>>(),
                             m_vfat3ChipConfig);
            } else {
                XCEPT_RAISE(exception::ParseError, "Unknown file type");
            }
        }

        void FileConfigurationProvider::loadVFAT3Channel(const std::string &filename)
        {
            // Find the file (throws if not found)
            auto path = detail::getFileInPath(filename, getSearchPath());
            m_sources.push_back(path);

            if (boost::algorithm::ends_with(path, ".json")) {
                nlohmann::json json;
                std::ifstream in(path);
                in >> json;
                loadInternal(path,
                             json.get<SerializationData<VFAT3ChannelConfiguration>>(),
                             m_vfat3ChannelConfig);
            } else {
                XCEPT_RAISE(exception::ParseError, "Unknown file type");
            }
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

