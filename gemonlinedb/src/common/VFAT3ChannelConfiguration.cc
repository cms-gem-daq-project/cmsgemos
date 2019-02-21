#include "gem/onlinedb/VFAT3ChannelConfiguration.h"

#include <sstream>

namespace gem {
    namespace onlinedb {
        detail::RegisterData VFAT3ChannelConfiguration::getRegisterData() const
        {
            auto data = detail::RegisterData();
            for (std::size_t i = 0; i < registers.size(); ++i) {
                std::stringstream ss;
                ss << "CFG_CHANNEL_" << i;
                data[ss.str()] = registers[i].registerValue;
            }
            return data;
        }

        void VFAT3ChannelConfiguration::readRegisterData(
            const detail::RegisterData &data)
        {
            for (std::size_t i = 0; i < registers.size(); ++i) {
                std::stringstream ss;
                ss << "CFG_CHANNEL_" << i;
                registers[i].registerValue = data.at(ss.str());
            }
        }

        bool VFAT3ChannelConfiguration::operator== (
            const VFAT3ChannelConfiguration &other) const
        {
            return registers == other.registers;
        }
    } /* namespace onlinedb */
} /* namespace gem */
