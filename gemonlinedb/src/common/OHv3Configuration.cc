#include "gem/onlinedb/OHv3Configuration.h"

#include <sstream>

#include "gem/onlinedb/GBTXConfiguration.h"
#include "gem/onlinedb/VFAT3ChipConfiguration.h"

namespace gem {
    namespace onlinedb {

        detail::RegisterData OHv3Configuration::getRegisterData() const
        {
            auto data = OHv3ConfigurationGen::getRegisterData();

            for (std::size_t vfat = 0; vfat < getTrigTapDelays().size(); ++vfat) {
                for (std::size_t i = 0; i < getTrigTapDelays(vfat).size(); ++i) {
                    std::stringstream ss;
                    ss << "TRIG_TAP_DELAY_VFAT" << vfat << "_BIT" << i;
                    data[ss.str()] = getTrigTapDelay(vfat, i);
                }
            }

            return data;
        }

        void OHv3Configuration::readRegisterData(const detail::RegisterData &data)
        {
            OHv3ConfigurationGen::readRegisterData(data);

            for (std::size_t vfat = 0; vfat < getTrigTapDelays().size(); ++vfat) {
                for (std::size_t i = 0; i < getTrigTapDelays(vfat).size(); ++i) {
                    std::stringstream ss;
                    ss << "TRIG_TAP_DELAY_VFAT" << vfat << "_BIT" << i;
                    setTrigTapDelay(vfat, i, data.at(ss.str()));
                }
            }
        }

        bool OHv3Configuration::operator== (const OHv3Configuration &other) const
        {
            return OHv3ConfigurationGen::operator== (other)
                && getTrigTapDelays() == other.getTrigTapDelays()
                && getVFATConfigs() == other.getVFATConfigs();
        }

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
