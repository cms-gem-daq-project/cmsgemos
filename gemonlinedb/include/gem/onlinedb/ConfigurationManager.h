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
         * @brief Main entry point to create and manage configuration objects.
         */
        class ConfigurationManager
        {
        public:
            /**
             * @brief Used to control the source of configuration data objects.
             */
            enum class Source
            {
                DB,  ///< Load configuration objects from the database.
                XML, ///< Load configuration objects from XML files.
            };

            /**
             * @brief Represent statistics about the configuration.
             */
            struct ObjectStats
            {
                std::size_t amcCount   = 0;
                std::size_t amc13Count = 0;
                std::size_t ohCount    = 0;
                std::size_t vfatCount  = 0;
            };

            /**
             * @brief Enables thread-safe read-only access to the shared
             *        configuration.
             */
            using ReadLock = typename boost::shared_lock<boost::shared_mutex>;

            /**
             * @brief Enables thread-safe read-write access to the shared
             *        configuration.
             */
            using EditLock = typename boost::unique_lock<boost::shared_mutex>;

        private:
            static std::unique_ptr<ConfigurationManager> s_instance;
            static boost::shared_mutex s_mutex;

            Source m_objectSource;
            Source m_topologySource;
            std::vector<std::string> m_objectSourceDetails;
            std::vector<std::string> m_topologySourceDetails;
            std::vector<std::unique_ptr<AMC13Configuration>> m_config;

        public:
            /**
             * @brief Constructor.
             */
            explicit ConfigurationManager(Source objectSource, Source topologySource);

            /**
             * @brief Retrieves the source of configuration objects.
             */
            Source getObjectSource() const { return m_objectSource; };

            /**
             * @brief Retrieves some details about the object source.
             */
            std::vector<std::string> getObjectSourceDetails() const
            { return m_objectSourceDetails; };

            /**
             * @brief Retrieves the source of the system topology.
             */
            Source getTopologySource() const { return m_topologySource; };

            /**
             * @brief Retrieves some details about the topology source.
             */
            std::vector<std::string> getTopologySourceDetails() const
            { return m_topologySourceDetails; };

            /**
             * @brief Retrieves the loaded configuration.
             */
            auto getConfiguration() const -> const decltype(m_config) &
            { return m_config; };

            /**
             * @brief Retrieves the loaded configuration.
             */
            auto getConfiguration() -> decltype(m_config) & { return m_config; };

            /**
             * @brief Computes statistics about the loaded configuration.
             */
            ObjectStats getStatistics() const;

            /**
             * @brief Creates a lock guarding the shared configuration for
             *        reading.
             *
             * The returned lock isn't locked.
             */
            static ReadLock makeReadLock() noexcept;

            /**
             * @brief Creates a lock guarding the shared configuration for
             *        writing.
             *
             * The returned lock is locked immediately.
             */
            static EditLock makeEditLock() noexcept;

            /**
             * @brief Obtains a reference to the system-wide manager object.
             *
             * Calls to this function and manipulation of the resulting data
             * must be guarded using a @c ReadLock. Typical usage is as follows:
             *
             *     auto lock = ConfigurationManager::makeReadLock();
             *     auto &manager = ConfigurationManager::getManager(lock);
             *     // Do something with the manager...
             *     // The lock is released automatically when going out of scope
             *
             * @throws xcept::Exception
             *         This function only throws Xcept exceptions
             */
            static const ConfigurationManager &getManager(ReadLock &lock);

            /**
             * @brief Gets a reference to the system-wide manager object.
             *
             * Calls to this function and manipulation of the resulting data
             * must be guarded using an @c EditLock. Typical usage is as
             * follows:
             *
             *     auto lock = ConfigurationManager::makeEditLock();
             *     auto &manager = ConfigurationManager::getConfiguration(lock);
             *     // Modify the manager...
             *     // The locks are released automatically when going out of scope
             *
             * @throws xcept::Exception
             *         This function only throws Xcept exceptions
             */
            static ConfigurationManager &getManager(EditLock &lock);

            /**
             * @brief Obtains a reference to the system-wide configuration
             *        object.
             *
             * Calls to this function and manipulation of the resulting data
             * must be guarded using a @c ReadLock. Typical usage is as follows:
             *
             *     auto lock = ConfigurationManager::makeReadLock();
             *     auto &config = ConfigurationManager::getConfiguration(lock);
             *     // Do something with config...
             *     // The lock is released automatically when going out of scope
             *
             * @throws xcept::Exception
             *         This function only throws Xcept exceptions
             */
            static const std::vector<std::unique_ptr<AMC13Configuration>> &
            getConfiguration(ReadLock &lock);

            /**
             * @brief Gets a reference to the system-wide configuration object.
             *
             * Calls to this function and manipulation of the resulting data
             * must be guarded using an @c EditLock. Typical usage is as
             * follows:
             *
             *     auto lock = ConfigurationManager::makeEditLock();
             *     auto &config = ConfigurationManager::getConfiguration(lock);
             *     // Modify the configuration...
             *     // The locks are released automatically when going out of scope
             *
             * @throws xcept::Exception
             *         This function only throws Xcept exceptions
             */
            static std::vector<std::unique_ptr<AMC13Configuration>> &
            getConfiguration(EditLock &lock);
        };

    } // namespace onlinedb
} // namespace gem

#endif // GEM_ONLINEDB_CONFIGURATIONMANAGER_H
