#include "gem/onlinedb/AMCConfiguration.h"

#include <sstream>

#include "gem/onlinedb/OHv3Configuration.h"

namespace gem {
    namespace onlinedb {

        detail::RegisterData AMCConfiguration::getRegisterData() const
        {
            auto data = AMCConfigurationGen::getRegisterData();

            for (std::size_t oh = 0; oh < getEOETimeouts().size(); ++oh) {
                std::stringstream ss;
                ss << "DAQ_OH" << oh << "_EOE_TIMEOUT";
                data[ss.str()] = getEOETimeout(oh);
            }

            for (std::size_t oh = 0; oh < getVFATMasks().size(); ++oh) {
                std::stringstream ss;
                ss << "DAQ_OH" << oh << "_VFAT_MASK";
                data[ss.str()] = getVFATMask(oh);
            }

            return data;
        }

        void AMCConfiguration::readRegisterData(const detail::RegisterData &data)
        {
            AMCConfigurationGen::readRegisterData(data);

            for (std::size_t oh = 0; oh < getEOETimeouts().size(); ++oh) {
                std::stringstream ss;
                ss << "DAQ_OH" << oh << "_EOE_TIMEOUT";
                setEOETimeout(oh, data.at(ss.str()));
            }

            for (std::size_t oh = 0; oh < getVFATMasks().size(); ++oh) {
                std::stringstream ss;
                ss << "DAQ_OH" << oh << "_VFAT_MASK";
                setVFATMask(oh, data.at(ss.str()));
            }
        }

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
