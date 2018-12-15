#include "gem/onlinedb/OHv3Configuration.h"

#include <sstream>

namespace gem {
    namespace onlinedb {

        detail::RegisterData OHv3Configuration::getRegisterData() const
        {
            auto data = OHv3ConfigurationGen::getRegisterData();

            for (std::size_t wire = 0; wire < m_HDMISBitModes.size(); ++wire) {
                std::stringstream ss;
                ss << "HDMI_SBIT_MODE" << wire;
                data[ss.str()] = m_HDMISBitModes[wire];
            }

            for (std::size_t wire = 0; wire < m_HDMISBitSel.size(); ++wire) {
                std::stringstream ss;
                ss << "HDMI_SBIT_SEL" << wire;
                data[ss.str()] = m_HDMISBitSel[wire];
            }

            for (std::size_t vfat = 0; vfat < m_sotTapDelays.size(); ++vfat) {
                std::stringstream ss;
                ss << "SOT_TAP_DELAY_VFAT" << vfat;
                data[ss.str()] = m_sotTapDelays[vfat];
            }

            for (std::size_t vfat = 0; vfat < m_trigTapDelays.size(); ++vfat) {
                for (std::size_t i = 0; i < m_trigTapDelays[vfat].size(); ++i) {
                    std::stringstream ss;
                    ss << "TRIG_TAP_DELAY_VFAT" << vfat << "_BIT" << i;
                    data[ss.str()] = m_trigTapDelays[vfat][i];
                }
            }

            return data;
        }

        void OHv3Configuration::readRegisterData(const detail::RegisterData &data)
        {
            OHv3ConfigurationGen::readRegisterData(data);

            for (std::size_t wire = 0; wire < m_HDMISBitModes.size(); ++wire) {
                std::stringstream ss;
                ss << "HDMI_SBIT_MODE" << wire;
                m_HDMISBitModes[wire] = data.at(ss.str());
            }

            for (std::size_t wire = 0; wire < m_HDMISBitSel.size(); ++wire) {
                std::stringstream ss;
                ss << "HDMI_SBIT_SEL" << wire;
                m_HDMISBitSel[wire] = data.at(ss.str());
            }

            for (std::size_t vfat = 0; vfat < m_sotTapDelays.size(); ++vfat) {
                std::stringstream ss;
                ss << "SOT_TAP_DELAY_VFAT" << vfat;
                m_sotTapDelays[vfat] = data.at(ss.str());
            }

            for (std::size_t vfat = 0; vfat < m_trigTapDelays.size(); ++vfat) {
                for (std::size_t i = 0; i < m_trigTapDelays[vfat].size(); ++i) {
                    std::stringstream ss;
                    ss << "TRIG_TAP_DELAY_VFAT" << vfat << "_BIT" << i;
                    m_trigTapDelays[vfat][i] = data.at(ss.str());
                }
            }
        }

        bool OHv3Configuration::operator== (const OHv3Configuration &other) const
        {
            return OHv3ConfigurationGen::operator== (other)
                && getHDMISbitModes() == other.getHDMISbitModes()
                && getHDMISbitSel() == other.getHDMISbitSel()
                && getSotTapDelays() == other.getSotTapDelays()
                && getTrigTapDelays() == other.getTrigTapDelays();
        }

    } /* namespace onlinedb */
} /* namespace gem */
