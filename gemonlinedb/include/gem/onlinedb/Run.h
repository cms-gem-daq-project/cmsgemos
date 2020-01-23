#ifndef GEM_ONLINEDB_RUN_H
#define GEM_ONLINEDB_RUN_H

#include <string>

#include <nlohmann/json.hpp>

namespace gem {
    namespace onlinedb {

        /**
         * Groups the data needed to fill the @c <RUN> tags for XML files. How
         * this data is created remains to be understood.
         */
        struct Run
        {
            std::string type;
            std::uint64_t number;
            std::string begin;
            std::string end;
            std::string description;
            std::string location;
            std::string initiatingUser;

            /**
             * @brief Checks two Run objects for equality.
             *
             * Two Run objects are equal if all their fields are equal. In
             * particular, this means that two Run objects with the same run
             * number can be different.
             */
            bool operator== (const Run &other) const;
        };

        /**
         * @brief Converts @ref Run to JSON
         *
         * @see https://github.com/nlohmann/json#arbitrary-types-conversions
         * @see https://github.com/valdasraps/cmsdbldr/blob/master/src/main/java/org/cern/cms/dbloader/model/condition/Run.java
         * @related Run
         */
        void to_json(nlohmann::json &json, const Run &data);

        /**
         * @brief Converts JSON to @ref Run
         *
         * @see https://github.com/nlohmann/json#arbitrary-types-conversions
         * @see https://github.com/valdasraps/cmsdbldr/blob/master/src/main/java/org/cern/cms/dbloader/model/condition/Run.java
         * @related Run
         */
        void from_json(const nlohmann::json &json, Run &data);

    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_RUN_H
