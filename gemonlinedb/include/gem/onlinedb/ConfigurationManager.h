#ifndef GEM_ONLINEDB_CONFIGURATIONMANAGER_H
#define GEM_ONLINEDB_CONFIGURATIONMANAGER_H

#include <memory>
#include <vector>

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace gem {
    namespace onlinedb {

        class AMC13Configuration;

        /**
         * \brief Main entry point to create and manage configuration objects.
         */
        class ConfigurationManager
        {
        public:
            /**
             * \brief Used to control the source of configuration data objects.
             */
            enum class Source
            {
                DB,  ///< \brief Load configuration objects from the database.
                XML, ///< \brief Load configuration objects from XML files.
            };

            /**
             * \brief Enables thread-safe read-only access to the shared
             *        configuration.
             */
            using ReadLock = typename boost::shared_lock<boost::shared_mutex>;

            /**
             * \brief Enables thread-safe read-write access to the shared
             *        configuration.
             */
            using EditLock = typename boost::unique_lock<boost::shared_mutex>;

        private:
            static std::unique_ptr<ConfigurationManager> s_instance;
            static boost::shared_mutex s_mutex;

            std::vector<std::unique_ptr<AMC13Configuration>> m_config;

        public:
            /**
             * \brief Constructor.
             */
            explicit ConfigurationManager(Source objectSource, Source topologySource);

            /**
             * \brief Creates a lock guarding the shared configuration for
             *        reading.
             *
             * The returned lock isn't locked.
             */
            static ReadLock makeReadLock();

            /**
             * \brief Creates a lock guarding the shared configuration for
             *        writing.
             *
             * The returned lock is locked immediately.
             */
            static EditLock makeEditLock();

            /**
             * \brief Obtains a reference to the system-wide configuration
             *        object.
             *
             * Calls to this function and manipulation of the resulting data
             * must be guarded using a \c ReadLock. Typical usage is as follows:
             *
             *     auto lock = ConfigurationManager::makeReadLock();
             *     auto &config = ConfigurationManager::getConfiguration(lock);
             *     // Do something with config...
             *     // The lock is released automatically when going out of scope
             */
            static const std::vector<std::unique_ptr<AMC13Configuration>> &
            getConfiguration(ReadLock &lock);

            /**
             * \brief Gets a reference to the system-wide configuration object.
             *
             * Calls to this function and manipulation of the resulting data
             * must be guarded using an \c EditLock. Typical usage is as
             * follows:
             *
             *     auto readLock = ConfigurationManager::makeReadLock();
             *     auto editLock = ConfigurationManager::makeEditLock(readLock);
             *     auto &config = ConfigurationManager::getConfiguration(editLock);
             *     // Modify the configuration...
             *     // The locks are released automatically when going out of scope
             */
            static std::vector<std::unique_ptr<AMC13Configuration>> &
            getConfiguration(EditLock &lock);
        };

    } // namespace onlinedb
} // namespace gem

#endif // GEM_ONLINEDB_CONFIGURATIONMANAGER_H
