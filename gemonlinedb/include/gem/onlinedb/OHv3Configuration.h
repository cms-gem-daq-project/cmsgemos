#ifndef GEM_ONLINEDB_OHV3CONFIGURATION_H
#define GEM_ONLINEDB_OHV3CONFIGURATION_H

#include <array>
#include <cstdint>

#include "detail/OHv3ConfigurationGen.h"

namespace gem {
    namespace onlinedb {

        /**
         * @brief Describes the configuration of a version 3 optohybrid.
         *
         * @note Modifying objects of this class doesn't affect hardware.
         */
        class OHv3Configuration: public detail::OHv3ConfigurationGen
        {
        public:
            /**
             * @brief The number of wires in an HDMI cable.
             */
            static const constexpr std::size_t HDMI_WIRE_COUNT = 8;

            /**
             * @brief The number of VFATs connected to an OH.
             */
            static const constexpr std::size_t VFAT_COUNT = 24;

            /**
             * @brief The number of trigger channels going from a VFAT to the OH.
             */
            static const constexpr std::size_t TRIGGER_CHANNEL_COUNT = 8;

            /**
             * @brief Container holding tap delays for a VFAT.
             */
            using TrigTapDelayBits = std::array<std::uint32_t, TRIGGER_CHANNEL_COUNT>;

        private:
            std::array<std::uint32_t, HDMI_WIRE_COUNT> m_HDMISBitModes;
            std::array<std::uint32_t, HDMI_WIRE_COUNT> m_HDMISBitSel;
            std::array<std::uint32_t, VFAT_COUNT>      m_sotTapDelays;
            std::array<TrigTapDelayBits, VFAT_COUNT>   m_trigTapDelays;

        public:
            /**
             * @brief Compares two OHv3 configurations for equality.
             */
            bool operator== (const OHv3Configuration &other) const;

            ////////////////////////////////////////////////////////////////////

            /**
             * @brief Retrieves register data.
             * Implements the @c Configuration concept.
             */
            detail::RegisterData getRegisterData() const;

            /**
             * @brief Modifies register data.
             * Implements the @c Configuration concept.
             */
            void readRegisterData(const detail::RegisterData &data);

            ////////////////////////////////////////////////////////////////////

            /**
             * @brief Retrieves the S-bit mode for an HDMI wire.
             */
            auto getHDMISbitMode(std::size_t wire) const ->
                decltype(m_HDMISBitModes)::const_reference
            {
                return m_HDMISBitModes.at(wire);
            }

            /**
             * @brief Retrieves the S-bit mode for all HDMI wires.
             */
            auto getHDMISbitModes() const -> const decltype(m_HDMISBitModes) &
            {
                return m_HDMISBitModes;
            }

            /**
             * @brief Retrieves the S-bit mode for all HDMI wires.
             */
            auto getHDMISbitModes() -> decltype(m_HDMISBitModes) &
            {
                return m_HDMISBitModes;
            }

            /**
             * @brief Modifies the S-bit mode for an HDMI wire.
             */
            void setHDMISbitMode(std::size_t wire, std::uint32_t mode)
            {
                m_HDMISBitModes.at(wire) = mode;
            }

            /**
             * @brief Modifies the S-bit mode for all HDMI wires.
             */
            void setHDMISbitModes(const decltype(m_HDMISBitModes) &modes)
            {
                m_HDMISBitModes = modes;
            }

            ////////////////////////////////////////////////////////////////////

            /**
             * @brief Retrieves the value of the S-bit selection register for an
             *        HDMI wire.
             */
            std::uint32_t getHDMISbitSel(std::size_t wire) const
            {
                return m_HDMISBitSel.at(wire);
            }

            /**
             * @brief Retrieves the values of S-bit selection registers for all
             *        HDMI wires.
             */
            auto getHDMISbitSel() -> decltype(m_HDMISBitSel)
            {
                return m_HDMISBitSel;
            }

            /**
             * @brief Retrieves the values of S-bit selection registers for all
             *        HDMI wires.
             */
            auto getHDMISbitSel() const -> const decltype(m_HDMISBitSel) &
            {
                return m_HDMISBitSel;
            }

            /**
             * @brief Modifies the value of the S-bit selection register for an
             *        HDMI wire.
             */
            void setHDMISbitSel(std::size_t wire, std::uint32_t value)
            {
                m_HDMISBitSel.at(wire) = value;
            }

            /**
             * @brief Modifies the values of S-bit selection registers for all
             *        HDMI wires.
             */
            void setHDMISbitSel(const decltype(m_HDMISBitSel) &values)
            {
                m_HDMISBitSel = values;
            }

            ////////////////////////////////////////////////////////////////////

            /**
             * @brief Retrieves the sot tap delay for the given VFAT.
             */
            std::uint32_t getSotTapDelay(std::size_t vfat) const
            {
                return m_sotTapDelays.at(vfat);
            }

            /**
             * @brief Retrieves all sot tap delays.
             */
            auto getSotTapDelays() const -> const decltype(m_sotTapDelays) &
            {
                return m_sotTapDelays;
            }

            /**
             * @brief Retrieves all sot tap delays.
             */
            auto getSotTapDelays() -> decltype(m_sotTapDelays) &
            {
                return m_sotTapDelays;
            }

            /**
             * @brief Modifies the sot tap delay for the given VFAT.
             */
            void setSotTapDelay(std::size_t vfat, std::uint32_t delay)
            {
                m_sotTapDelays.at(vfat) = delay;
            }

            /**
             * @brief Modifies all sot tap delays.
             */
            void setSotTapDelays(const decltype(m_sotTapDelays) &delays)
            {
                m_sotTapDelays = delays;
            }

            ////////////////////////////////////////////////////////////////////

            /**
             * @brief Retrieves the trigger tap delay at the given index for the
             *        given VFAT.
             */
            std::uint32_t getTrigTapDelay(std::size_t vfat, std::size_t index) const
            {
                return m_trigTapDelays.at(vfat).at(index);
            }

            /**
             * @brief Retrieves all trigger tap delays for the given VFAT.
             */
            const TrigTapDelayBits &getTrigTapDelays(std::size_t vfat) const
            {
                return m_trigTapDelays.at(vfat);
            }

            /**
             * @brief Retrieves all trigger tap delays for the given VFAT.
             */
            TrigTapDelayBits &getTrigTapDelays(std::size_t vfat)
            {
                return m_trigTapDelays.at(vfat);
            }

            /**
             * @brief Retrieves all trigger tap delays.
             */
            auto getTrigTapDelays() const -> const decltype(m_trigTapDelays) &
            {
                return m_trigTapDelays;
            }

            /**
             * @brief Retrieves all trigger tap delays.
             */
            auto getTrigTapDelays() -> decltype(m_trigTapDelays) &
            {
                return m_trigTapDelays;
            }

            /**
             * @brief Modifies the trigger tap delay at the given index for the
             *        given VFAT.
             */
            void setTrigTapDelay(std::size_t vfat,
                                 std::size_t index,
                                 std::uint32_t delay)
            {
                m_trigTapDelays.at(vfat).at(index) = delay;
            }

            /**
             * @brief Modifies the trigger tap delays for the given VFAT.
             */
            void setTrigTapDelay(std::size_t vfat,
                                 const TrigTapDelayBits &delays)
            {
                m_trigTapDelays.at(vfat) = delays;
            }

            /**
             * @brief Modifies all trigger tap delays.
             */
            void setTrigTapDelays(const decltype(m_trigTapDelays) &delays)
            {
                m_trigTapDelays = delays;
            }
        };

        template<>
        class ConfigurationTraits<OHv3Configuration> :
            public ConfigurationTraits<detail::OHv3ConfigurationGen>
        {};

    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_OHV3CONFIGURATION_H
