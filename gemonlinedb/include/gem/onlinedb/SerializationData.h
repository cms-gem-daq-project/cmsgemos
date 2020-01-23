#ifndef GEM_ONLINEDB_SERIALIZATIONDATA_H
#define GEM_ONLINEDB_SERIALIZATIONDATA_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "gem/onlinedb/ConfigurationTraits.h"
#include "gem/onlinedb/DataSet.h"
#include "gem/onlinedb/PartReference.h"
#include "gem/onlinedb/Run.h"
#include "gem/onlinedb/detail/RegisterData.h"
#include "gem/onlinedb/exception/Exception.h"

namespace gem {
    namespace onlinedb {

        /**
         * @brief Represents the contents of XML files.
         *
         * This class is used to represent the contents of XML files as C++
         * objects before they are serialized and after they are read. It is
         * used as an indermediate step when (un)serializing configuration data.
         *
         * @tparam ConfigurationTypeT The configuration class serialized in the
         *                            XML file.
         */
        template<class ConfigurationTypeT>
        class SerializationData
        {
        public:
            /**
             * @brief Convenience typedef.
             */
            using ConfigurationType = ConfigurationTypeT;

        private:
            Run m_run;
            std::vector<DataSet<ConfigurationType>> m_dataSets;

        public:
            /**
             * @brief Returns the run information from the file.
             */
            Run getRun() const { return m_run; }

            /**
             * @brief Sets the run for which to generate XML data.
             */
            void setRun(const Run &run) { m_run = run; }

            /**
             * @brief Gets the list of parsed datasets.
             */
            std::vector<DataSet<ConfigurationType>> getDataSets() const
            {
                return m_dataSets;
            }

            /**
             * @brief Sets the list of datasets.
             */
            void setDataSets(const std::vector<DataSet<ConfigurationType>> &dataSets)
            {
                m_dataSets = dataSets;
            }

            /**
             * @brief Adds a dataset to be serialized
             */
            void addDataSet(const DataSet<ConfigurationType> &dataSet)
            {
                m_dataSets.push_back(dataSet);
            }

        };

        template<class ConfigurationTypeT>
        bool operator== (const SerializationData<ConfigurationTypeT> &lhs,
                         const SerializationData<ConfigurationTypeT> &rhs)
        {
            return lhs.getRun() == rhs.getRun() && lhs.getDataSets() == rhs.getDataSets();
        }

        /**
         * @brief Converts @ref SerializationData to JSON
         *
         * @see https://github.com/nlohmann/json#arbitrary-types-conversions
         * @see https://github.com/valdasraps/cmsdbldr/blob/master/src/main/java/org/cern/cms/dbloader/model/serial/Root.java
         * @related SerializationData
         */
        template<class ConfigurationTypeT>
        void to_json(nlohmann::json &json, const SerializationData<ConfigurationTypeT> &data)
        {
            using Info = ConfigurationTraits<ConfigurationTypeT>;
            json = {
                { "Root", nlohmann::json({
                    { "Header", nlohmann::json({
                        { "Type", nlohmann::json({
                            { "ExtensionTableName", Info::extTableName() },
                            { "Name", Info::typeName() },
                        })},
                        { "Run", data.getRun() },
                    })},
                    { "Datasets", data.getDataSets() },
                })},
            };
        }

        /**
         * @brief Converts JSON to @ref SerializationData
         *
         * @throws exception::ParseError if the JSON data isn't of the expected type.
         * @throws nlohmann::json::exception if the JSON data doesn't match the expected format.
         * @see https://github.com/nlohmann/json#arbitrary-types-conversions
         * @see https://github.com/valdasraps/cmsdbldr/blob/master/src/main/java/org/cern/cms/dbloader/model/serial/Root.java
         * @related SerializationData
         */
        template<class ConfigurationTypeT>
        void from_json(const nlohmann::json &json, SerializationData<ConfigurationTypeT> &data)
        {
            using Info = ConfigurationTraits<ConfigurationTypeT>;
            auto header = json.at("Root").at("Header");
            auto type = header.at("Type");
            if (type.at("ExtensionTableName") != Info::extTableName()
                || type.at("Name") != Info::typeName()) {
                XCEPT_RAISE(exception::ParseError, "JSON data is not of the expected type");
            }
            data.setRun(header.at("Run").get<Run>());
            data.setDataSets(json.at("Root")
                                 .at("Datasets")
                                 .get<std::vector<DataSet<ConfigurationTypeT>>>());
        }

    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_SERIALIZATIONDATA_H
