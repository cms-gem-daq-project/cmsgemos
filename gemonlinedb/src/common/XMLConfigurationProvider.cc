#include "gem/onlinedb/XMLConfigurationProvider.h"

#include <boost/filesystem.hpp>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include "gem/onlinedb/XMLSerializationData.h"
#include "gem/onlinedb/detail/FileUtils.h"
#include "gem/onlinedb/detail/XMLUtils.h"
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

            /// @brief Loads and validates a document.
            DOMDocumentPtr loadDOM(const std::string &path,
                                   const std::string &schema)
            {
                using namespace detail::literals;

                // Find the schema (throws if not found)
                auto schemaPath = detail::getFileInPath(
                    schema, "xml/schema:/opt/cmsgemos/share/gemonlinedb/xml/schema");

                // Make the paths absolute
                auto absPath = boost::filesystem::canonical(path).string();
                schemaPath = boost::filesystem::canonical(schemaPath).string();

                // Get a DOM implementation
                auto impl = DOMImplementationRegistry::getDOMImplementation("LS"_xml);
                if (impl == nullptr) {
                    throw std::logic_error("Cannot find a DOM implementation");
                }

                auto errorHandler =
                    std::make_shared<detail::XercesAlwaysThrowErrorHandler>();

                XercesDOMParser parser;
                parser.setErrorHandler(errorHandler.get());
                parser.setExternalNoNamespaceSchemaLocation(
                    detail::XercesString(schemaPath));
                parser.setValidationScheme(XercesDOMParser::Val_Always);
                parser.setDoNamespaces(true);
                parser.setDoSchema(true);
                parser.setValidationSchemaFullChecking(true);

                // Parse and validate
                parser.parse(detail::XercesString(absPath));

                DOMDocumentPtr document;
                document.reset(parser.adoptDocument());
                return document;
            }

            /// @brief Common part to all loadXXX methods
            template<class ConfigurationType>
            void loadInternal(
                const std::string &filename,
                const DOMDocumentPtr &document,
                std::map<std::string, std::shared_ptr<ConfigurationType>> &map)
            {
                XMLSerializationData<ConfigurationType> xmlData;
                xmlData.readDOM(document);

                auto dataSets = xmlData.getDataSets();
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

        void XMLConfigurationProvider::load(const DOMDocumentPtr &document)
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
                } else if (tagName == "gem:amc-file") {
                    loadAMC(detail::transcode(fileEl->getTextContent()));
                } else if (tagName == "gem:amc13-file") {
                    loadAMC13(detail::transcode(fileEl->getTextContent()));
                }
            }
        }

        void XMLConfigurationProvider::loadAMC13(const std::string &filename)
        {
            // Find the file (throws if not found)
            auto path = detail::getFileInPath(filename, getSearchPath());
            m_sources.push_back(path);

            detail::XercesGuard guard; // Make sure that Xerces is loaded
            auto document = loadDOM(path, "AMC13Configuration.xsd");

            loadInternal(path, document, m_amc13Config);
        }

        void XMLConfigurationProvider::loadAMC(const std::string &filename)
        {
            // Find the file (throws if not found)
            auto path = detail::getFileInPath(filename, getSearchPath());
            m_sources.push_back(path);

            detail::XercesGuard guard; // Make sure that Xerces is loaded
            auto document = loadDOM(path, "AMCConfiguration.xsd");

            loadInternal(path, document, m_amcConfig);
        }

        void XMLConfigurationProvider::loadOHv3(const std::string &filename)
        {
            // Find the file (throws if not found)
            auto path = detail::getFileInPath(filename, getSearchPath());
            m_sources.push_back(path);

            detail::XercesGuard guard; // Make sure that Xerces is loaded
            auto document = loadDOM(path, "OHv3Configuration.xsd");

            loadInternal(path, document, m_ohv3Config);
        }

        void XMLConfigurationProvider::loadVFAT3Chip(const std::string &filename)
        {
            // Find the file (throws if not found)
            auto path = detail::getFileInPath(filename, getSearchPath());
            m_sources.push_back(path);

            detail::XercesGuard guard; // Make sure that Xerces is loaded
            auto document = loadDOM(path, "VFAT3ChipConfiguration.xsd");

            loadInternal(path, document, m_vfat3ChipConfig);
        }

        void XMLConfigurationProvider::loadVFAT3Channel(const std::string &filename)
        {
            // Find the file (throws if not found)
            auto path = detail::getFileInPath(filename, getSearchPath());
            m_sources.push_back(path);

            detail::XercesGuard guard; // Make sure that Xerces is loaded
            auto document = loadDOM(path, "VFAT3ChannelConfiguration.xsd");

            loadInternal(path, document, m_vfat3ChannelConfig);
        }

        std::shared_ptr<AMC13Configuration>
        XMLConfigurationProvider::getAMC13Configuration(
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
        XMLConfigurationProvider::getAMCConfiguration(
            const ConfigurationTraits<AMCConfiguration>::PartType &reference) const
        {
            try {
                return m_amcConfig.at(toString(reference));
            } catch (const std::out_of_range &e) {
                XCEPT_RAISE(exception::ReferenceError,
                            "AMC config not found: " + toString(reference));
            }
        }

        std::shared_ptr<OHv3Configuration>
        XMLConfigurationProvider::getOHv3Configuration(
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
        XMLConfigurationProvider::getVFAT3ChipConfiguration(
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
        XMLConfigurationProvider::getVFAT3ChannelConfiguration(
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

