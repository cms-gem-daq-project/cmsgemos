#include <memory>

#include <tstore/OracleConnection.h>
#include <tstore/TStoreAPI.h>
#include <tstore/api/Connect.h>

#include <xdata/TableIterator.h>

#include "gem/onlinedb/AMC13Configuration.h"
#include "gem/onlinedb/AMCConfiguration.h"
#include "gem/onlinedb/DBInterface.h"
#include "gem/onlinedb/OHv3Configuration.h"
#include "gem/onlinedb/VFAT3ChannelConfiguration.h"
#include "gem/onlinedb/VFAT3ChipConfiguration.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TStoreSerialization
#include <boost/test/unit_test.hpp>

/* Needed to make the linker happy. */
#include <xdaq/version.h>
config::PackageInfo xdaq::getPackageInfo()
{
    return config::PackageInfo("", "", "", "", "", "", "", "", "");
}

/**
 * @brief Makes sure the DB remains open during the tests.
 */
struct DBFixture
{
    std::unique_ptr<tstore::Connection> connection;

    explicit DBFixture()
    {
        connection.reset(tstore::OracleConnection::create("INT2R",
                                                          "CMS_GEM_APPUSER_R",
                                                          "GEM_Reader_2015"));
        connection->openConnection();
    }

    // Shared between tests
    template<class ConfigurationType> void test();
};

template<class ConfigurationType>
void DBFixture::test()
{
    BOOST_REQUIRE(connection->isConnected());

    tstore::SQLQuery query("select * from CMS_GEM_MUON_COND." +
        gem::onlinedb::ConfigurationTraits<ConfigurationType>::extTableName());
    xdata::Table results;
    connection->execute(query, results);

    for (auto &row : results) {
        gem::onlinedb::DBInterface::convertRow<ConfigurationType>(results, row);
    }
}

BOOST_FIXTURE_TEST_SUITE(TStoreSerialization, DBFixture)

BOOST_AUTO_TEST_CASE(VFAT3ChannelConfiguration)
{
    test<gem::onlinedb::VFAT3ChannelConfiguration>();
}

BOOST_AUTO_TEST_CASE(VFAT3ChipConfiguration)
{
    test<gem::onlinedb::VFAT3ChipConfiguration>();
}

BOOST_AUTO_TEST_CASE(OHv3Configuration)
{
    test<gem::onlinedb::OHv3Configuration>();
}

BOOST_AUTO_TEST_CASE(AMCConfiguration)
{
    // There are some broken AMC configurations in the DB
    BOOST_CHECK_THROW(test<gem::onlinedb::AMCConfiguration>(),
                      gem::onlinedb::exception::ParseError);
}

BOOST_AUTO_TEST_CASE(AMC13Configuration)
{
    test<gem::onlinedb::AMC13Configuration>();
}

BOOST_AUTO_TEST_SUITE_END()
