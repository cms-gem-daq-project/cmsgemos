#include "gem/onlinedb/VFAT3ChannelConfiguration.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestVFAT3ChannelConfiguration
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <iomanip>

/* Needed to make the linker happy. */
#include <xdaq/version.h>
config::PackageInfo xdaq::getPackageInfo()
{
    return config::PackageInfo("", "", "", "", "", "", "", "", "");
}

using namespace gem::onlinedb;

BOOST_AUTO_TEST_SUITE(TestVFAT3ChannelConfiguration)

BOOST_AUTO_TEST_CASE(ChannelRegisterValue)
{
    VFAT3ChannelConfiguration::ChannelRegisterValue value;

    value.registerValue = 0xFFFF;
    BOOST_CHECK(value.fields.cal == true);
    BOOST_CHECK(value.fields.mask == true);

    value.registerValue = 0;
    value.fields.cal = true;
    BOOST_CHECK(value.registerValue == (1 << 15));

    value.registerValue = 0;
    value.fields.armDACCorrectionPolarity = 1;
    BOOST_CHECK(value.registerValue == (1 << 6));

    value.registerValue = 0;
    value.fields.zccDACCorrectionPolarity = 1;
    BOOST_CHECK(value.registerValue == (1 << 13));

    value.registerValue = 0;
    value.fields.zccDACCorrectionAmplitude = 0b101010;
    BOOST_CHECK(value.registerValue == (0b101010 << 7));
}

BOOST_AUTO_TEST_CASE(SerializationData)
{
    VFAT3ChannelConfiguration config;
    nlohmann::json json = config;
    BOOST_CHECK(json.count("CFG_CHANNEL_15") == 1);
    config = json;
}

BOOST_AUTO_TEST_SUITE_END()
