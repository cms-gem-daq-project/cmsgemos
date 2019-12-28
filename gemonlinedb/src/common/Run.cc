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

    } /* namespace onlinedb */
} /* namespace gem */
