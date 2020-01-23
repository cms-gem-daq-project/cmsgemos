#include "gem/onlinedb/OHv3Configuration.h"

#include <sstream>

#include "gem/onlinedb/GBTXConfiguration.h"
#include "gem/onlinedb/VFAT3ChipConfiguration.h"

namespace gem {
    namespace onlinedb {

        void OHv3Configuration::createAllVFATConfigs()
        {
            for (auto &c : getVFATConfigs()) {
                c = std::make_shared<VFAT3ChipConfiguration>();
            }
        }

        void OHv3Configuration::createAllVFATConfigs(
                const VFAT3ChipConfiguration &config)
        {
            for (auto &c : getVFATConfigs()) {
                c = std::make_shared<VFAT3ChipConfiguration>(config);
            }
        }

        void OHv3Configuration::createAllGBTXConfigs()
        {
            for (auto &c : getGBTXConfigs()) {
                c = std::make_shared<GBTXConfiguration>();
            }
        }

        void OHv3Configuration::createAllGBTXConfigs(const GBTXConfiguration &config)
        {
            for (auto &c : getGBTXConfigs()) {
                c = std::make_shared<GBTXConfiguration>(config);
            }
        }

    } /* namespace onlinedb */
} /* namespace gem */
