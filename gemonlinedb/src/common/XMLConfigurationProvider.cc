#include "gem/onlinedb/XMLConfigurationProvider.h"

#include <boost/filesystem.hpp>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include "gem/onlinedb/XMLSerializationData.h"
#include "gem/onlinedb/detail/FileUtils.h"
#include "gem/onlinedb/detail/XMLUtils.h"

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
            DOMDocumentPtr loadDOM(const std::string &filename,
                                   const std::string &searchPath,
                                   const std::string &schema)
            {
                using namespace detail::literals;

                // Find the file (throws if not found)
                auto path = detail::getFileInPath(filename, searchPath);

                // Find the schema (throws if not found)
                auto schemaPath = detail::getFileInPath(
                    schema, "xml/schema:/opt/cmsgemos/share/gemonlinedb/xml/schema");

                // Make the paths absolute
                path = boost::filesystem::canonical(path).string();
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
                parser.parse(detail::XercesString(path));

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
                        throw std::runtime_error(
                            "In " + filename + ": Duplicated part " + toString(part));
                    }

                    auto data = set.getData();
                    if (data.size() > 1) {
                        std::cout << data.size() << std::endl;
                        // Corresponds to more than one configuration for the same
                        // part, which is not supported.
                        throw std::runtime_error(
                            "In " + filename +
                            ": DATA_SET section contains more than one DATA entry");
                    }

                    map[toString(part)] =
                        std::make_shared<ConfigurationType>(data.front());
                }
            }

        } // anonymous namespace

        void XMLConfigurationProvider::loadAMC13(const std::string &filename)
        {
            detail::XercesGuard guard; // Make sure that Xerces is loaded
            auto document = loadDOM(filename,
                                    getSearchPath(),
                                    "AMC13Configuration.xsd");

            loadInternal(filename, document, m_amc13Config);
        }

        void XMLConfigurationProvider::loadAMC(const std::string &filename)
        {
            detail::XercesGuard guard; // Make sure that Xerces is loaded
            auto document = loadDOM(filename,
                                    getSearchPath(),
                                    "AMCConfiguration.xsd");

            loadInternal(filename, document, m_amcConfig);
        }

        void XMLConfigurationProvider::loadOHv3(const std::string &filename)
        {
            detail::XercesGuard guard; // Make sure that Xerces is loaded
            auto document = loadDOM(filename,
                                    getSearchPath(),
                                    "OHv3Configuration.xsd");

            loadInternal(filename, document, m_ohv3Config);
        }

        void XMLConfigurationProvider::loadVFAT3Chip(const std::string &filename)
        {
            detail::XercesGuard guard; // Make sure that Xerces is loaded
            auto document = loadDOM(filename,
                                    getSearchPath(),
                                    "VFAT3ChipConfiguration.xsd");

            loadInternal(filename, document, m_vfat3ChipConfig);
        }

        void XMLConfigurationProvider::loadVFAT3Channel(const std::string &filename)
        {
            detail::XercesGuard guard; // Make sure that Xerces is loaded
            auto document = loadDOM(filename,
                                    getSearchPath(),
                                    "VFAT3ChannelConfiguration.xsd");

            loadInternal(filename, document, m_vfat3ChannelConfig);
        }

        std::shared_ptr<AMC13Configuration>
        XMLConfigurationProvider::getAMC13Configuration(
            const ConfigurationTraits<AMC13Configuration>::PartType &reference) const
        {
            return m_amc13Config.at(toString(reference));
        }

        std::shared_ptr<AMCConfiguration>
        XMLConfigurationProvider::getAMCConfiguration(
            const ConfigurationTraits<AMCConfiguration>::PartType &reference) const
        {
            return m_amcConfig.at(toString(reference));
        }

        std::shared_ptr<OHv3Configuration>
        XMLConfigurationProvider::getOHv3Configuration(
            const ConfigurationTraits<OHv3Configuration>::PartType &reference) const
        {
            return m_ohv3Config.at(toString(reference));
        }

        std::shared_ptr<VFAT3ChipConfiguration>
        XMLConfigurationProvider::getVFAT3ChipConfiguration(
            const ConfigurationTraits<VFAT3ChipConfiguration>::PartType &reference) const
        {
            return m_vfat3ChipConfig.at(toString(reference));
        }

        std::shared_ptr<VFAT3ChannelConfiguration>
        XMLConfigurationProvider::getVFAT3ChannelConfiguration(
            const ConfigurationTraits<VFAT3ChannelConfiguration>::PartType &reference) const
        {
            return m_vfat3ChannelConfig.at(toString(reference));
        }

    } // namespace onlinedb
} // namespace gem

