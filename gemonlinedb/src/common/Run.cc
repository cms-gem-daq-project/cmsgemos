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

    } /* namespace onlinedb */
} /* namespace gem */
