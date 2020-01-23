#include "gem/onlinedb/Run.h"

namespace gem {
    namespace onlinedb {

        bool Run::operator== (const Run &other) const
        {
            return type == other.type
                && number == other.number
                && begin == other.begin
                && end == other.end
                && description == other.description
                && location == other.location
                && initiatingUser == other.initiatingUser;
        }

        void to_json(nlohmann::json &json, const Run &data)
        {
            json = {
                { "RunNumber", data.number },
                { "RunBeginTimestamp", data.begin },
                { "RunEndTimestamp", data.end },
                // { "Description", data.description }, FIXME
                { "Location", data.location },
                { "InitiatedByUser", data.initiatingUser },
            };
        }

        void from_json(const nlohmann::json &json, Run &data)
        {
            if (json.find("RunNumber") != json.end()) {
                data.number = json["RunNumber"];
            }
            if (json.find("RunBeginTimestamp") != json.end()) {
                data.begin = json["RunBeginTimestamp"];
            }
            if (json.find("RunEndTimestamp") != json.end()) {
                data.end = json["RunEndTimestamp"];
            }
            if (json.find("Location") != json.end()) {
                data.location = json["Location"];
            }
            if (json.find("InitiatedByUser") != json.end()) {
                data.initiatingUser = json["InitiatedByUser"];
            }
            data.description = ""; // FIXME
        }

    } /* namespace onlinedb */
} /* namespace gem */
