#include "gem/onlinedb/FileConfigurationProvider.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestFileConfigurationProvider
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <stdexcept>

/* Needed to make the linker happy. */
#include <xdaq/version.h>
config::PackageInfo xdaq::getPackageInfo()
{
    return config::PackageInfo("", "", "", "", "", "", "", "");
}

using namespace gem::onlinedb;

BOOST_AUTO_TEST_SUITE(TestFileConfigurationProvider)

BOOST_AUTO_TEST_CASE(LoadAMC13)
{
    FileConfigurationProvider provider;
    provider.loadAMC13("xml/examples/AMC13_Configuration.xml");
}

BOOST_AUTO_TEST_CASE(LoadAMC)
{
    FileConfigurationProvider provider;
    provider.loadAMC("xml/examples/AMC_Configuration.xml");

    // SN "unknown" isn't present in the xml file
    BOOST_CHECK_THROW(provider.getAMCConfiguration({ "unknown" }),
                      exception::ReferenceError);

    // SN "AMC-VI-VERSION-0001" is present in the xml file
    provider.getAMCConfiguration({ "AMC-VI-VERSION-0001" });

    // SN "AMC-VI-VERSION-6464" is present in the xml file
    provider.getAMCConfiguration({ "AMC-VI-VERSION-6464" });
}

BOOST_AUTO_TEST_CASE(LoadOHv3)
{
    FileConfigurationProvider provider;
    provider.loadOHv3("xml/examples/OHv3_Configuration.xml");
}

BOOST_AUTO_TEST_CASE(LoadVFAT3Chip)
{
    FileConfigurationProvider provider;
    provider.loadVFAT3Chip("xml/examples/VFAT3_Chip_Configuration.xml");

    // Barcode "unknown" isn't present in the xml file
    BOOST_CHECK_THROW(provider.getVFAT3ChipConfiguration({ "unknown" }),
                      exception::ReferenceError);

    // Barcode "2940" is present in the xml file
    provider.getVFAT3ChipConfiguration({ "2940" });

    // Barcode "2941" is present in the xml file
    provider.getVFAT3ChipConfiguration({ "2941" });
}

BOOST_AUTO_TEST_CASE(LoadVFAT3Channel)
{
    FileConfigurationProvider provider;
    provider.loadVFAT3Channel("xml/examples/VFAT3_Channel_Configuration.xml");

    // Barcode "unknown" isn't present in the xml file
    BOOST_CHECK_THROW(provider.getVFAT3ChannelConfiguration({ "unknown" }),
                      exception::ReferenceError);

    // Barcode "2940" is present in the xml file
    provider.getVFAT3ChannelConfiguration({ "2940" });
}

BOOST_AUTO_TEST_CASE(LoadNonCompliant)
{
    FileConfigurationProvider provider;
    // xml/tests/NonCompliantFile.xml doesn't comply with the schema
    BOOST_CHECK_THROW(
        provider.loadAMC13("xml/tests/NonCompliantFile.xml"),
        exception::ParseError);
}

BOOST_AUTO_TEST_SUITE_END()
