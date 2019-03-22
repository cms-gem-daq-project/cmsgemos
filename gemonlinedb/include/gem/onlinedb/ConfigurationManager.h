#ifndef GEM_ONLINEDB_CONFIGURATIONMANAGER_H
#define GEM_ONLINEDB_CONFIGURATIONMANAGER_H

#include <memory>
#include <vector>

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

        private:
            static std::unique_ptr<ConfigurationManager> s_instance;

            std::vector<std::unique_ptr<AMC13Configuration>> m_config;

        public:
            /**
             * \brief Constructor.
             */
            explicit ConfigurationManager(Source objectSource, Source topologySource);

            /**
             * \brief Obtains a reference to the system-wide configuration
             *        object.
             *
             * The reference should not be stored. If storage of the returned
             * objects outside of the scope of the calling function is needed,
             * they should be copied.
             *
             * @warning This function is not thread-safe..
             */
            static const std::vector<std::unique_ptr<AMC13Configuration>> &
            getConfiguration();

            /**
             * \brief Gets a reference to the system-wide configuration object.
             *
             * @warning This function is not thread-safe.
             */
            static std::vector<std::unique_ptr<AMC13Configuration>> &
            getEditableConfiguration();
        };

    } // namespace onlinedb
} // namespace gem

#endif // GEM_ONLINEDB_CONFIGURATIONMANAGER_H
