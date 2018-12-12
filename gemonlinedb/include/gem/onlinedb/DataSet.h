#ifndef GEM_ONLINEDB_DATASET_H
#define GEM_ONLINEDB_DATASET_H

#include <string>
#include <vector>

#include "gem/onlinedb/ConfigurationTraits.h"

namespace gem {
    namespace onlinedb {

        // Forward decl.
        template<class ConfigurationTypeT>
        class XMLSerializationData;

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
            friend class XMLSerializationData<ConfigurationType>;

            std::string comment, version;
            typename ConfigurationTraits<ConfigurationType>::PartType part;
            std::vector<ConfigurationType> data;

        public:
            /**
             * @brief Checks two data sets for equality.
             */
            bool operator== (const DataSet<ConfigurationType> &other) const;

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

        template<class ConfigurationType>
        bool DataSet<ConfigurationType>::operator== (
            const DataSet<ConfigurationType> &other) const
        {
            return comment == other.comment
                && version == other.version
                && part == other.part
                && data == other.data;
        }

    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_DATASET_H
