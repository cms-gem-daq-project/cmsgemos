#ifndef GEM_ONLINEDB_DATASET_H
#define GEM_ONLINEDB_DATASET_H

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "gem/onlinedb/ConfigurationTraits.h"

namespace gem {
    namespace onlinedb {

        // Forward decl.
        template<class ConfigurationTypeT>
        class SerializationData;

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
            std::string m_comment, m_version;
            typename ConfigurationTraits<ConfigurationType>::PartType m_part;
            std::vector<ConfigurationType> m_data;

        public:
            /**
             * @brief Checks two data sets for equality.
             */
            bool operator== (const DataSet<ConfigurationType> &other) const;

            /**
             * @brief Gets the human-readable comment for this dataset.
             */
            std::string getComment() const { return m_comment; }

            /**
             * @brief Sets the human-readable comment for this dataset.
             */
            void setComment(const std::string &comment) { m_comment = comment; }

            /**
             * @brief Gets the version of the dataset.
             */
            std::string getVersion() const { return m_version; }

            /**
             * @brief Sets the version of the dataset.
             */
            void setVersion(const std::string &version) { m_version = version; }

            /**
             * @brief Gets the part configured by this dataset.
             */
            auto getPart() const -> decltype(m_part) { return m_part; }

            /**
             * @brief Sets the part configured by this dataset.
             */
            void setPart(
                const typename ConfigurationTraits<ConfigurationType>::PartType &part)
            {
                m_part = part;
            }

            /**
             * @brief Retrieves configuration data.
             */
            auto getData() const -> decltype(m_data) { return m_data; }

            /**
             * @brief Adds configuration data.
             */
            void addData(const ConfigurationType &configuration)
            {
                m_data.push_back(configuration);
            }
        };

        template<class ConfigurationType>
        bool DataSet<ConfigurationType>::operator== (
            const DataSet<ConfigurationType> &other) const
        {
            return m_comment == other.m_comment
                && m_version == other.m_version
                && m_part == other.m_part
                && m_data == other.m_data;
        }

        /**
         * @brief Converts @ref DataSet to JSON
         *
         * @see https://github.com/nlohmann/json#arbitrary-types-conversions
         * @see https://github.com/valdasraps/cmsdbldr/blob/master/src/main/java/org/cern/cms/dbloader/model/condition/Dataset.java
         * @related DataSet
         */
        template<class ConfigurationTypeT>
        void to_json(nlohmann::json &json, const DataSet<ConfigurationTypeT> &data)
        {
            json = {
                { "Dataset", nlohmann::json({
                    { "Version", data.getVersion() },
                    { "part", data.getPart() },
                    { "Data", data.getData() },
                })},
            };
            if (!data.getComment().empty()) {
                json["Dataset"]["CommentDescription"] = data.getComment();
            };
        }

    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_DATASET_H
