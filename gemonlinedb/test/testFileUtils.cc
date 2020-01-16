#include "gem/onlinedb/detail/FileUtils.h"

#include <cstdlib>

#include <boost/filesystem.hpp>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE FileUtils
#include <boost/test/unit_test.hpp>

/* Needed to make the linker happy. */
#include <xdaq/version.h>
config::PackageInfo xdaq::getPackageInfo()
{
    return config::PackageInfo("", "", "", "", "", "", "", "", "");
}

using namespace gem::onlinedb::detail;

BOOST_AUTO_TEST_SUITE(FileUtils)

BOOST_AUTO_TEST_CASE(FileInPath)
{
    std::string path = std::getenv("PATH");

    auto ls = getFileInPath("ls", path);
    BOOST_CHECK(!ls.empty());

    BOOST_CHECK_THROW(getFileInPath("this-program-doesnt-exist", path),
                      boost::filesystem::filesystem_error);
}

BOOST_AUTO_TEST_SUITE_END()
