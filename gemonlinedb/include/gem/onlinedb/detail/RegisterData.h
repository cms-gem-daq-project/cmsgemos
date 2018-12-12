#ifndef GEM_ONLINEDB_DETAIL_REGISTERDATA_H
#define GEM_ONLINEDB_DETAIL_REGISTERDATA_H

#include <map>

namespace gem {
    namespace onlinedb {
        namespace detail {
            /**
            * @brief Structure that holds register data stored in configuration
            *        objects.
            */
            using RegisterData = typename std::map<std::string, std::uint32_t>;
        } /* namespace detail */
    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_DETAIL_REGISTERDATA_H
