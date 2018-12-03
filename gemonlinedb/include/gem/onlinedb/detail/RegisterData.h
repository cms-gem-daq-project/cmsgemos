#ifndef _RegisterData_h_
#define _RegisterData_h_

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

#endif // _RegisterData_h_
