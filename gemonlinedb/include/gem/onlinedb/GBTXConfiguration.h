#ifndef GEM_ONLINEDB_GBTXCONFIGURATION_H
#define GEM_ONLINEDB_GBTXCONFIGURATION_H

#include <string>

#include "gem/onlinedb/detail/GBTXConfigurationGen.h"

namespace gem {
    namespace onlinedb {

        /**
         * @brief Describes the configuration of a GBTX.
         *
         * @note Modifying objects of this class doesn't affect hardware.
         */
        class GBTXConfiguration: public detail::GBTXConfigurationGen
        {
        public:
            /**
             * @brief Compares two OHv3 configurations for equality.
             */
            bool operator== (const GBTXConfiguration &other) const
            { return detail::GBTXConfigurationGen::operator== (other); };
        };

        template<>
        class ConfigurationTraits<GBTXConfiguration> :
            public ConfigurationTraits<detail::GBTXConfigurationGen>
        {};

    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_GBTXCONFIGURATION_H
