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
             * @brief Finds the path to the XML topology file
             */
            std::string findXMLTopologyFile(const std::string &path)
            {
                // Find the file (throws if not found)
                return detail::getFileInPath("system-topology.xml", path);
            }

            /**
             * @brief Loads a system-topology.xml file
             */
            DOMDocumentPtr loadXMLTopologyFile(const std::string &filePath)
            {
                using namespace detail::literals;
                XERCES_CPP_NAMESPACE_USE

                detail::XercesGuard guard; // Make sure that Xerces is loaded

                // Find the schema (throws if not found)
                auto schemaPath = detail::getFileInPath(
                    "system-topology.xsd",
                    "xml/schema:/opt/cmsgemos/share/gemonlinedb/xml/schema");

                // Get an implementation
                auto impl = DOMImplementationRegistry::getDOMImplementation("LS"_xml);
                if (impl == nullptr) {
                    throw std::logic_error("Cannot find a DOM implementation");
                }

                auto errorHandler =
                    std::make_shared<detail::XercesAlwaysThrowErrorHandler>();

                XercesDOMParser parser;
                parser.setErrorHandler(errorHandler.get());
                // FIXME Change the ns URL to the documentation website
                // (don't forget to change the schema and example)
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

        boost::shared_mutex ConfigurationManager::s_mutex;

        ConfigurationManager::ConfigurationManager(Source objectSource,
                                                   Source topologySource) :
            m_objectSource(objectSource),
            m_topologySource(topologySource)
        {
            try {
                detail::xercesExceptionsToXcept([&]{
                    // Deduce the search path for XML files
                    std::string path = "/opt/cmsgemos/etc"; // Following xDAQ's "standards"
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
                        provider->load(loadXMLTopologyFile(findXMLTopologyFile(path)));
                        m_objectSourceDetails = provider->getObjectSources();
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
                        auto file = findXMLTopologyFile(path);
                        topology->populate(loadXMLTopologyFile(file));
                        m_topologySourceDetails = { file };
                        break;
                    }

                    m_config = std::move(linker.link(topology));
                });
            } catch (std::exception &e) {
                XCEPT_RAISE(exception::SoftwareProblem, e.what());
            }
        }

        ConfigurationManager::ObjectStats ConfigurationManager::getStatistics() const
        {
            ObjectStats stats;

            auto &config = getConfiguration();

            for (auto &amc13 : config) {
                if (amc13 != nullptr) {
                    stats.amc13Count++;

                    auto amcConfigs = amc13->getAMCConfigs();
                    for (auto &amc : amcConfigs) {
                        if (amc != nullptr) {
                            stats.amcCount++;

                            auto ohConfigs = amc->getOHConfigs();
                            for (auto &oh : ohConfigs) {
                                if (oh != nullptr) {
                                    stats.ohCount++;

                                    auto vfatConfigs = oh->getVFATConfigs();
                                    for (auto &vfat : vfatConfigs) {
                                        if (vfat != nullptr) {
                                            stats.vfatCount++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            return stats;
        }

        ConfigurationManager::ReadLock ConfigurationManager::makeReadLock() noexcept
        {
            return ReadLock(s_mutex, boost::defer_lock);
        }

        ConfigurationManager::EditLock ConfigurationManager::makeEditLock() noexcept
        {
            return EditLock(s_mutex, boost::defer_lock);
        }

        const ConfigurationManager &
        ConfigurationManager::getManager(ConfigurationManager::ReadLock &lock)
        {
            // Get read-only access if needed
            if (!lock.owns_lock()) {
                lock.lock();
            }
            if (s_instance == nullptr) {
                // Revoke read access
                lock.unlock();
                {
                    // Need write access to create the config
                    auto uniqueLock = makeEditLock();
                    getManager(uniqueLock);
                }
                // Get back read access once the unique lock is released

                // Note that another thread may have modified the config, so we
                // cannot rely on the result of getConfiguration(uniqueLock)
                lock.lock();
            }
            return *s_instance;
        }

        ConfigurationManager &
        ConfigurationManager::getManager(ConfigurationManager::EditLock &lock)
        {
            // Get read-write access if needed
            if (!lock.owns_lock()) {
                lock.lock();
            }
            if (s_instance == nullptr) {
                // TODO Only XML is supported for now
                s_instance.reset(
                    new ConfigurationManager(Source::XML, Source::XML));
            }
            return *s_instance;
        }

        const std::vector<std::unique_ptr<AMC13Configuration>> &
        ConfigurationManager::getConfiguration(ConfigurationManager::ReadLock &lock)
        {
            return getManager(lock).getConfiguration();
        }

        std::vector<std::unique_ptr<AMC13Configuration>> &
        ConfigurationManager::getConfiguration(ConfigurationManager::EditLock &lock)
        {
            return getManager(lock).getConfiguration();
        }

    } // namespace onlinedb
} // namespace gem
