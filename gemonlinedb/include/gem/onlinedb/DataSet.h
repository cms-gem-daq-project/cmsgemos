#ifndef _DataSet_h_
#define _DataSet_h_

#include <string>
#include <vector>

#include "gem/onlinedb/ConfigurationTraits.h"

namespace gem {
    namespace onlinedb {

        // Forward decl.
        template<class ConfigurationTypeT>
        class XMLBuilder;

        // Forward decl.
        template<class ConfigurationTypeT>
        class XMLReader;

        /**
         * @brief Represents a dataset to be stored in the XML file (a
         *        @c <DATA_SET> tag).
         */
        template<class ConfigurationTypeT>
        class DataSet
        {
        public:
            /**
             * @brief Convenience typedef.
             */
            using ConfigurationType = ConfigurationTypeT;

        private:
            friend class XMLBuilder<ConfigurationType>;
            friend class XMLReader<ConfigurationType>;

            std::string comment, version;
            typename ConfigurationTraits<ConfigurationType>::PartType part;
            std::vector<ConfigurationType> data;

        public:
            /**
             * @brief Gets the human-readable comment for this dataset.
             */
            std::string getComment() const { return comment; }

            /**
             * @brief Sets the human-readable comment for this dataset.
             */
            void setComment(const std::string &comment) { this->comment = comment; }

            /**
             * @brief Gets the version of the dataset.
             */
            std::string getVersion() const { return version; }

            /**
             * @brief Sets the version of the dataset.
             */
            void setVersion(const std::string &version) { this->version = version; }

            /**
             * @brief Gets the part configured by this dataset.
             */
            auto getPart() const -> decltype(part) { return part; }

            /**
             * @brief Sets the part configured by this dataset.
             */
            void setPart(
                const typename ConfigurationTraits<ConfigurationType>::PartType &part)
            {
                this->part = part;
            }

            /**
             * @brief Retrieves configuration data.
             */
            auto getData() -> decltype(data) const { return data; }

            /**
             * @brief Adds configuration data.
             */
            void addData(const ConfigurationType &configuration)
            {
                data.push_back(configuration);
            }
        };

    } /* namespace onlinedb */
} /* namespace gem */

#endif // _DataSet_h_
