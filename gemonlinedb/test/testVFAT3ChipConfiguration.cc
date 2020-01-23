#include "gem/onlinedb/VFAT3ChipConfiguration.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestVFAT3ChipConfiguration
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <iomanip>

/* Needed to make the linker happy. */
#include <xdaq/version.h>
config::PackageInfo xdaq::getPackageInfo()
{
    return config::PackageInfo("", "", "", "", "", "", "", "");
}

using namespace gem::onlinedb;

BOOST_AUTO_TEST_SUITE(TestVFAT3ChipConfiguration)

BOOST_AUTO_TEST_CASE(JSONRoundtrip)
{
    /*
     * Tests that the configuration can be deserialized from JSON.
     */
    using namespace nlohmann;

    // Create a configuration
    VFAT3ChipConfiguration config;
    config.setGlbCal0CfgCtrl(1);
    config.setGlbCal1CfgCtrl(2);
    config.setGlbCfdBiasCtrl(3);
    config.setGlbCfdCtrl(4);
    config.setGlbDataCtrl(5);
    config.setGlbFeCtrl(6);
    config.setGlbHysterisisCtrl(7);
    config.setGlbIrefCtrl(8);
    config.setGlbLatencyCtrl(9);
    config.setGlbLogicCtrl(10);
    config.setGlbMonCtrl(11);
    config.setGlbPreampBiasCtrl0(12);
    config.setGlbPreampBiasCtrl1(13);
    config.setGlbSdBiasCtrl(14);
    config.setGlbShaperBiasCtrl0(15);
    config.setGlbShaperBiasCtrl1(16);
    config.setGlbThresholdCtrl(17);
    config.setHwRwReg(18);

    // Check that we can turn it into the correct JSON
    nlohmann::json json = config;
    BOOST_CHECK(json == R"({
        "GLB_CAL0_CFG_CTRL":      1,
        "GLB_CAL1_CFG_CTRL":      2,
        "GLB_CFD_BIAS_CTRL":      3,
        "GLB_CFD_CTRL":           4,
        "GLB_DATA_CTRL":          5,
        "GLB_FE_CTRL":            6,
        "GLB_HYSTERISIS_CTRL":    7,
        "GLB_IREF_CTRL":          8,
        "GLB_LATENCY_CTRL":       9,
        "GLB_LOGIC_CTRL":        10,
        "GLB_MON_CTRL":          11,
        "GLB_PREAMP_BIAS_CTRL0": 12,
        "GLB_PREAMP_BIAS_CTRL1": 13,
        "GLB_SD_BIAS_CTRL":      14,
        "GLB_SHAPER_BIAS_CTRL0": 15,
        "GLB_SHAPER_BIAS_CTRL1": 16,
        "GLB_THRESHOLD_CTRL":    17,
        "HW_RW_REG":             18
    })"_json);

    // Check that we can turn it back into the configuration
    config = json.get<VFAT3ChipConfiguration>();
    BOOST_CHECK( 1 == config.getGlbCal0CfgCtrl());
    BOOST_CHECK( 2 == config.getGlbCal1CfgCtrl());
    BOOST_CHECK( 3 == config.getGlbCfdBiasCtrl());
    BOOST_CHECK( 4 == config.getGlbCfdCtrl());
    BOOST_CHECK( 5 == config.getGlbDataCtrl());
    BOOST_CHECK( 6 == config.getGlbFeCtrl());
    BOOST_CHECK( 7 == config.getGlbHysterisisCtrl());
    BOOST_CHECK( 8 == config.getGlbIrefCtrl());
    BOOST_CHECK( 9 == config.getGlbLatencyCtrl());
    BOOST_CHECK(10 == config.getGlbLogicCtrl());
    BOOST_CHECK(11 == config.getGlbMonCtrl());
    BOOST_CHECK(12 == config.getGlbPreampBiasCtrl0());
    BOOST_CHECK(13 == config.getGlbPreampBiasCtrl1());
    BOOST_CHECK(14 == config.getGlbSdBiasCtrl());
    BOOST_CHECK(15 == config.getGlbShaperBiasCtrl0());
    BOOST_CHECK(16 == config.getGlbShaperBiasCtrl1());
    BOOST_CHECK(17 == config.getGlbThresholdCtrl());
    BOOST_CHECK(18 == config.getHwRwReg());
}

BOOST_AUTO_TEST_SUITE_END()
