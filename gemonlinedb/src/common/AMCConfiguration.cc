#include "gem/onlinedb/AMCConfiguration.h"

#include <sstream>

#include "gem/onlinedb/OHv3Configuration.h"

namespace gem {
    namespace onlinedb {

        bool AMCConfiguration::operator== (const AMCConfiguration &other) const
        {
            return AMCConfigurationGen::operator== (other)
                && getEOETimeouts() == other.getEOETimeouts()
                && getVFATMasks() == other.getVFATMasks()
                && getOHConfigs() == other.getOHConfigs();
        }

        void AMCConfiguration::createAllOHConfigs()
        {
            for (auto &c : getOHConfigs()) {
                c = std::make_shared<OHv3Configuration>();
            }
        }

        void AMCConfiguration::createAllOHConfigs(
            const OHv3Configuration &config)
        {
            for (auto &c : getOHConfigs()) {
                c = std::make_shared<OHv3Configuration>(config);
            }
        }

    } /* namespace onlinedb */
} /* namespace gem */
