#include "gem/onlinedb/ConfigurationManager.h"

#include <cstdlib>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include "gem/onlinedb/ConfigurationLinker.h"
#include "gem/onlinedb/SystemTopology.h"
#include "gem/onlinedb/XMLConfigurationProvider.h"
#include "gem/onlinedb/detail/FileUtils.h"

namespace gem {
    namespace onlinedb {

        namespace /* anonymous */ {

            /**
             * \brief Loads a system-topology.xml file
             */
            DOMDocumentPtr loadXMLTopologyFile(const std::string &path)
            {
                using namespace detail::literals;
                XERCES_CPP_NAMESPACE_USE

                detail::XercesGuard guard; // Make sure that Xerces is loaded

                // Find the schema (throws if not found)
                auto schemaPath = detail::getFileInPath(
                    "system-topology.xsd",
                    "xml/schema:/opt/cmsgemos/share/gemonlinedb/xml/schema");

                // Find the file (throws if not found)
                auto filePath = detail::getFileInPath("system-topology.xml", path);

                // Get an implementation
                auto impl = DOMImplementationRegistry::getDOMImplementation("LS"_xml);
                if (impl == nullptr) {
                    throw std::logic_error("Cannot find a DOM implementation");
                }

                auto errorHandler =
                    std::make_shared<detail::XercesAlwaysThrowErrorHandler>();

                XercesDOMParser parser;
                parser.setErrorHandler(errorHandler.get());
                parser.setExternalSchemaLocation(
                    ("https://github.com/cms-gem-daq-project/cmsgemos/tree/master/gemonlinedb/xml/schema/system-topology.xsd "
                    + schemaPath).c_str());
                parser.setValidationScheme(XercesDOMParser::Val_Always);
                parser.setDoNamespaces(true);
                parser.setDoSchema(true);

                // Parse and validate
                parser.parse(detail::XercesString(filePath));

                // Load contents
                DOMDocumentPtr document;
                document.reset(parser.adoptDocument());
                return document;
            }

        } // anonymous namespace

        std::unique_ptr<ConfigurationManager>
        ConfigurationManager::s_instance = nullptr;

        ConfigurationManager::ConfigurationManager(Source objectSource,
                                                   Source topologySource)
        {
            // Deduce the search path for XML files
            std::string path = "/etc/opt/cmsgemos"; // FHS 3.0 standard location
            if (auto envPath = std::getenv("CMSGEMOS_CONFIG_PATH")) {
                path = envPath;
            }

            ConfigurationLinker linker;

            // Configuration objects
            switch (objectSource) {
            case Source::DB:
                // TODO Not supported yet
                break;

            case Source::XML:
                auto provider = std::make_shared<XMLConfigurationProvider>();
                provider->setSearchPath(path);
                provider->load(loadXMLTopologyFile(path));
                linker.setProvider(provider);
                break;
            }

            // Topology
            auto topology = std::make_shared<SystemTopology>();

            switch (topologySource) {
            case Source::DB:
                // TODO Not supported yet
                break;

            case Source::XML:
                topology->populate(loadXMLTopologyFile(path));
                break;
            }

            m_config = std::move(linker.link(topology));
        }

        const std::vector<std::unique_ptr<AMC13Configuration>> &
        ConfigurationManager::getConfiguration()
        {
            // Just making it const
            return getEditableConfiguration();
        }

        std::vector<std::unique_ptr<AMC13Configuration>> &
        ConfigurationManager::getEditableConfiguration()
        {
            if (s_instance == nullptr) {
                // TODO Only XML is supported for now
                s_instance.reset(
                    new ConfigurationManager(Source::XML, Source::XML));
            }
            return s_instance->m_config;
        }

    } // namespace onlinedb
} // namespace gem
