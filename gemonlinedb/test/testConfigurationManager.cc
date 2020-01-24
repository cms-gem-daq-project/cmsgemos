#include <atomic>
#include <chrono>
#include <stdlib.h> // setenv
#include <thread>
#include <memory>
#include <vector>

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
    return config::PackageInfo("", "", "", "", "", "", "", "", "");
}

using namespace gem::onlinedb;

BOOST_FIXTURE_TEST_SUITE(TestConfigurationManager, detail::XercesGuard)

BOOST_AUTO_TEST_CASE(GetConfiguration)
{
    XERCES_CPP_NAMESPACE_USE

    detail::xercesExceptionsToStd([]{
        setenv("CMSGEMOS_CONFIG_PATH", "xml/examples", 1);

        auto lock = ConfigurationManager::makeReadLock();
        auto &config = ConfigurationManager::getConfiguration(lock);

        // The example system topology has 2 AMC 13's
        BOOST_CHECK(config.size() == 2);
    });
}

BOOST_AUTO_TEST_CASE(SharedRead)
{
    // Tests that reading can be done concurrently: create 10 threads locking
    // for 100ms each. The total time must be well under 1s.

    XERCES_CPP_NAMESPACE_USE

    detail::xercesExceptionsToStd([]{
        setenv("CMSGEMOS_CONFIG_PATH", "xml/examples", 1);

        auto start = std::chrono::system_clock::now();
        std::atomic<std::exception_ptr> eptr;

        std::vector<std::thread> threads;
        for (int i = 0; i < 10; ++i) {
            threads.emplace_back([&eptr] {
                try {
                    auto lock = ConfigurationManager::makeReadLock();
                    ConfigurationManager::getConfiguration(lock); // Locks
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                } catch (...) {
                    // Exceptions cannot pass thread boundaries, so save the
                    // current one to rethrow it later.
                    eptr = std::current_exception();
                }
            });
        }
        for (auto &t : threads) {
            t.join();
        }

        // If there was an exception, rethrow it (if there were several, only
        // throw the last one.
        if (eptr != nullptr) {
            std::rethrow_exception(eptr);
        }

        auto duration = std::chrono::system_clock::now() - start;
        BOOST_CHECK(duration < std::chrono::milliseconds(500));
    });
}

BOOST_AUTO_TEST_CASE(WriteBlockRead)
{
    // Tests that writing blocks reading: create a thread that locks for writing
    // for 100ms and another locking for reading for 100ms. The total time must
    // be over 200ms.

    XERCES_CPP_NAMESPACE_USE

    detail::xercesExceptionsToStd([]{
        setenv("CMSGEMOS_CONFIG_PATH", "xml/examples", 1);

        auto start = std::chrono::system_clock::now();
        std::atomic<std::exception_ptr> eptr;

        std::thread writer([&eptr] {
            try {
                auto lock = ConfigurationManager::makeEditLock();
                ConfigurationManager::getConfiguration(lock); // Locks
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } catch (...) {
                // Exceptions cannot pass thread boundaries, so save the
                // current one to rethrow it later.
                eptr = std::current_exception();
            }
        });
        std::thread reader([&eptr] {
            try {
                auto lock = ConfigurationManager::makeReadLock();
                ConfigurationManager::getConfiguration(lock); // Locks
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } catch (...) {
                // Exceptions cannot pass thread boundaries, so save the
                // current one to rethrow it later.
                eptr = std::current_exception();
            }
        });
        writer.join();
        reader.join();

        // If there was an exception, rethrow it (if there were several, only
        // throw the last one.
        if (eptr != nullptr) {
            std::rethrow_exception(eptr);
        }

        auto duration = std::chrono::system_clock::now() - start;
        BOOST_CHECK(duration > std::chrono::milliseconds(200));
    });
}

BOOST_AUTO_TEST_SUITE_END()
