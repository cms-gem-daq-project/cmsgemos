#include "gem/onlinedb/ConfigurationLinker.h"
#include "gem/onlinedb/SystemTopology.h"
#include "gem/onlinedb/FileConfigurationProvider.h"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLString.hpp>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestConfigurationLinker
#include <boost/test/unit_test.hpp>

#include <stdexcept>

/* Needed to make the linker happy. */
#include <xdaq/version.h>
config::PackageInfo xdaq::getPackageInfo()
{
    return config::PackageInfo("", "", "", "", "", "", "", "", "");
}

using namespace gem::onlinedb;
using namespace gem::onlinedb::detail::literals;

BOOST_FIXTURE_TEST_SUITE(TestConfigurationLinker, detail::XercesGuard)

BOOST_AUTO_TEST_CASE(LoadFromSysConfig)
{
    XERCES_CPP_NAMESPACE_USE

    detail::xercesExceptionsToStd([]{
        // Get an implementation
        auto impl = DOMImplementationRegistry::getDOMImplementation("LS"_xml);
        BOOST_REQUIRE(impl != nullptr);

        auto errorHandler = std::make_shared<detail::XercesAlwaysThrowErrorHandler>();

        XercesDOMParser parser;
        parser.setErrorHandler(errorHandler.get());
        parser.setExternalSchemaLocation(
            "https://github.com/cms-gem-daq-project/cmsgemos/tree/master/gemonlinedb/xml/schema/system-topology.xsd "
            "../schema/system-topology.xsd"); // Need to figure out a better way
        parser.setValidationScheme(XercesDOMParser::Val_Always);
        parser.setDoNamespaces(true);
        parser.setDoSchema(true);

        // Parse and validate
        parser.parse("xml/examples/system-topology.xml"_xml);
        DOMDocumentPtr document;
        document.reset(parser.adoptDocument());

        // Load configurations
        auto provider = std::make_shared<FileConfigurationProvider>();
        provider->setSearchPath("xml/examples");
        provider->load(document);

        // Load topology
        auto topology = std::make_shared<SystemTopology>();
        topology->populate(document);

        // Link
        auto linker = ConfigurationLinker(provider);
        auto config = linker.link(topology);
    });
}

BOOST_AUTO_TEST_SUITE_END()
