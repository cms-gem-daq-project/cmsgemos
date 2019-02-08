#include "gem/onlinedb/XMLConfigurationProvider.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE XMLSerializationData
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

BOOST_AUTO_TEST_SUITE(TestXMLConfigurationProvider)

BOOST_AUTO_TEST_CASE(LoadAMC13)
{
    XMLConfigurationProvider provider;
    provider.loadAMC13("xml/AMC13_Configuration.xml");
}

BOOST_AUTO_TEST_CASE(LoadAMC)
{
    XMLConfigurationProvider provider;
    provider.loadAMC("xml/AMC_Configuration.xml");

    // SN "unknown" isn't present in the xml file
    BOOST_CHECK_THROW(provider.getAMCConfiguration({ "unknown" }),
                      std::out_of_range);

    // SN "AMC-VI-VERSION-0001" is present in the xml file
    provider.getAMCConfiguration({ "AMC-VI-VERSION-0001" });

    // SN "AMC-VI-VERSION-6464" is present in the xml file
    provider.getAMCConfiguration({ "AMC-VI-VERSION-6464" });
}

BOOST_AUTO_TEST_CASE(LoadOHv3)
{
    XMLConfigurationProvider provider;
    provider.loadOHv3("xml/OHv3_Configuration.xml");
}

BOOST_AUTO_TEST_CASE(LoadVFAT3Chip)
{
    XMLConfigurationProvider provider;
    provider.loadVFAT3Chip("xml/VFAT3_Chip_Configuration.xml");

    // Barcode "unknown" isn't present in the xml file
    BOOST_CHECK_THROW(provider.getVFAT3ChipConfiguration({ "unknown" }),
                      std::out_of_range);

    // Barcode "2940" is present in the xml file
    provider.getVFAT3ChipConfiguration({ "2940" });

    // Barcode "2941" is present in the xml file
    provider.getVFAT3ChipConfiguration({ "2941" });
}

BOOST_AUTO_TEST_CASE(LoadVFAT3Channel)
{
    XMLConfigurationProvider provider;
    provider.loadVFAT3Channel("xml/VFAT3_Channel_Configuration.xml");

    // Barcode "unknown" isn't present in the xml file
    BOOST_CHECK_THROW(provider.getVFAT3ChannelConfiguration({ "unknown" }),
                      std::out_of_range);

    // Barcode "2940" is present in the xml file
    provider.getVFAT3ChannelConfiguration({ "2940" });
}

BOOST_AUTO_TEST_SUITE_END()
