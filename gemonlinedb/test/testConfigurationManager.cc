#include <stdlib.h> // setenv
#include <memory>

#include "gem/onlinedb/ConfigurationManager.h"
#include "gem/onlinedb/detail/XMLUtils.h"

#include <xercesc/dom/DOM.hpp>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SystemTopologyTest
#include <boost/test/unit_test.hpp>

/* Needed to make the linker happy. */
#include <xdaq/version.h>
config::PackageInfo xdaq::getPackageInfo()
{
    return config::PackageInfo("", "", "", "", "", "", "", "");
}

using namespace gem::onlinedb;

BOOST_FIXTURE_TEST_SUITE(TestConfigurationManager, detail::XercesGuard)

BOOST_AUTO_TEST_CASE(GetConfiguration)
{
    XERCES_CPP_NAMESPACE_USE

    try {
        setenv("CMSGEMOS_CONFIG_PATH", "xml/examples", 1);

        auto &config = ConfigurationManager::getEditableConfiguration();

        // The example system topology has 2 AMC 13's
        BOOST_CHECK(config.size() == 2);

    } catch (DOMException &e) {
        throw std::runtime_error(detail::transcode(e.getMessage()));
    } catch (XMLException &e) {
        throw std::runtime_error(detail::transcode(e.getMessage()));
    } catch (SAXParseException &e) {
        auto column = e.getColumnNumber();
        auto line = e.getLineNumber();
        throw std::runtime_error(
            std::to_string(line) + ":" +
            std::to_string(column) + ": " +
            detail::transcode(e.getMessage()));
    } catch (SAXException &e) {
        throw std::runtime_error(detail::transcode(e.getMessage()));
    }
}

BOOST_AUTO_TEST_SUITE_END()
