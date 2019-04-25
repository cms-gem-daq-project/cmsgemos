#ifndef GEM_ONLINEDB_XMLCONFIGURATIONPROVIDER_H
#define GEM_ONLINEDB_XMLCONFIGURATIONPROVIDER_H

#include <map>
#include <string>

#include "gem/onlinedb/ConfigurationProvider.h"
#include "gem/onlinedb/detail/XMLUtils.h"

namespace gem {
    namespace onlinedb {

        /**
         * @brief Configuration provider that takes XML files as input.
         */
        class XMLConfigurationProvider : public ConfigurationProvider
        {
            std::map<std::string, std::shared_ptr<AMC13Configuration>> m_amc13Config;
            std::map<std::string, std::shared_ptr<AMCConfiguration>> m_amcConfig;
            std::map<std::string, std::shared_ptr<OHv3Configuration>> m_ohv3Config;
            std::map<std::string, std::shared_ptr<VFAT3ChipConfiguration>> m_vfat3ChipConfig;
            std::map<std::string, std::shared_ptr<VFAT3ChannelConfiguration>> m_vfat3ChannelConfig;

            std::string m_searchPath = ".";
            std::vector<std::string> m_sources;

        public:
            /// @brief Constructor.
            explicit XMLConfigurationProvider() = default;

            /// @brief Destructor.
            virtual ~XMLConfigurationProvider() = default;

            ////////////////////////////////////////////////////////////////////

            /**
             * @brief Constructs an @c XMLConfigurationProvider by reading the
             *        contents of a @c system-topology file.
             * @param document The document to load data from.
             * @warning Behavior is undefined if the provided document doesn't
             *          conform to the @c system-topology schema.
             */
            void load(const DOMDocumentPtr &document);

            /**
             * @brief Sets the base path used to locate XML files
             *
             * A colon-separated list of directories can be used. By default,
             * files are searched for in the current directory.
             */
            void setSearchPath(const std::string &basePath) { m_searchPath = basePath; };

            /// @brief Gets the base path used to locate XML files
            std::string getSearchPath() const { return m_searchPath; };

            /**
             * @brief Loads AMC 13 configurations from an XML file.
             *
             * The configurations found in the file are made available using
             * retrieval functions.
             *
             * The file is searched for in the base path (see @ref setSearchPath).
             */
            void loadAMC13(const std::string &filename);

            /**
             * @brief Loads AMC configurations from an XML file.
             *
             * The configurations found in the file are made available using
             * retrieval functions.
             *
             * The file is searched for in the base path (see @ref setSearchPath).
             */
            void loadAMC(const std::string &filename);

            /**
             * @brief Loads OHv3 configurations from an XML file.
             *
             * The configurations found in the file are made available using
             * retrieval functions.
             *
             * The file is searched for in the base path (see @ref setSearchPath).
             */
            void loadOHv3(const std::string &filename);

            /**
             * @brief Loads VFAT3 chip configurations from an XML file.
             *
             * The configurations found in the file are made available using
             * retrieval functions.
             *
             * The file is searched for in the base path (see @ref setSearchPath).
             */
            void loadVFAT3Chip(const std::string &filename);

            /**
             * @brief Loads VFAT3 channel configurations from an XML file.
             *
             * The configurations found in the file are made available using
             * retrieval functions.
             *
             * The file is searched for in the base path (see @ref setSearchPath).
             */
            void loadVFAT3Channel(const std::string &filename);

            ////////////////////////////////////////////////////////////////////

            std::shared_ptr<AMC13Configuration> getAMC13Configuration(
                const ConfigurationTraits<AMC13Configuration>::PartType &reference) const override;

            std::shared_ptr<AMCConfiguration> getAMCConfiguration(
                const ConfigurationTraits<AMCConfiguration>::PartType &reference) const override;

            std::shared_ptr<OHv3Configuration> getOHv3Configuration(
                const ConfigurationTraits<OHv3Configuration>::PartType &reference) const override;

            std::shared_ptr<VFAT3ChipConfiguration> getVFAT3ChipConfiguration(
                const ConfigurationTraits<VFAT3ChipConfiguration>::PartType &reference) const override;

            std::shared_ptr<VFAT3ChannelConfiguration> getVFAT3ChannelConfiguration(
                const ConfigurationTraits<VFAT3ChannelConfiguration>::PartType &reference) const override;

            std::vector<std::string> getObjectSources() const override
            { return m_sources; };
        };

    } // namespace onlinedb
} // namespace gem

#endif // GEM_ONLINEDB_XMLCONFIGURATIONPROVIDER_H
