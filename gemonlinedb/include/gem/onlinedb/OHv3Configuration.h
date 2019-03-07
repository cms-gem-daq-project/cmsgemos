#ifndef GEM_ONLINEDB_OHV3CONFIGURATION_H
#define GEM_ONLINEDB_OHV3CONFIGURATION_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>

#include "gem/onlinedb/detail/OHv3ConfigurationGen.h"

namespace gem {
    namespace onlinedb {

        // Forward declaration
        class VFAT3ChipConfiguration;

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
            std::array<std::uint32_t, HDMI_WIRE_COUNT> m_hdmiSBitModes;
            std::array<std::uint32_t, HDMI_WIRE_COUNT> m_hdmiSBitSel;
            std::array<std::uint32_t, VFAT_COUNT>      m_sotTapDelays;
            std::array<TrigTapDelayBits, VFAT_COUNT>   m_trigTapDelays;

            std::array<std::shared_ptr<VFAT3ChipConfiguration>, VFAT_COUNT> m_vfatConfigs;

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
             * @name HDMI pin control
             * @{
             */
            /**
             * @brief Retrieves the S-bit mode for an HDMI wire.
             */
            auto getHDMISBitMode(std::size_t wire) const ->
                decltype(m_hdmiSBitModes)::const_reference {
                return m_hdmiSBitModes.at(wire); };

            /**
             * @brief Retrieves the S-bit mode for all HDMI wires.
             */
            auto getHDMISBitModes() const -> const decltype(m_hdmiSBitModes) & {
                return m_hdmiSBitModes; };

            /**
             * @brief Retrieves the S-bit mode for all HDMI wires.
             */
            auto getHDMISBitModes() -> decltype(m_hdmiSBitModes) & {
                return m_hdmiSBitModes; };

            /**
             * @brief Modifies the S-bit mode for an HDMI wire.
             */
            void setHDMISBitMode(std::size_t wire, std::uint32_t mode) {
                m_hdmiSBitModes.at(wire) = mode; };

            /**
             * @brief Modifies the S-bit mode for all HDMI wires.
             */
            void setHDMISBitModes(const decltype(m_hdmiSBitModes) &modes) {
                m_hdmiSBitModes = modes; };

            ////////////////////////////////////////////////////////////////////

            /**
             * @brief Retrieves the value of the S-bit selection register for an
             *        HDMI wire.
             */
            std::uint32_t getHDMISBitSel(std::size_t wire) const {
                return m_hdmiSBitSel.at(wire); };

            /**
             * @brief Retrieves the values of S-bit selection registers for all
             *        HDMI wires.
             */
            auto getHDMISBitSel() -> decltype(m_hdmiSBitSel) {
                return m_hdmiSBitSel; };

            /**
             * @brief Retrieves the values of S-bit selection registers for all
             *        HDMI wires.
             */
            auto getHDMISBitSel() const -> const decltype(m_hdmiSBitSel) & {
                return m_hdmiSBitSel; };

            /**
             * @brief Modifies the value of the S-bit selection register for an
             *        HDMI wire.
             */
            void setHDMISBitSel(std::size_t wire, std::uint32_t value) {
                m_hdmiSBitSel.at(wire) = value; };

            /**
             * @brief Modifies the values of S-bit selection registers for all
             *        HDMI wires.
             */
            void setHDMISBitSel(const decltype(m_hdmiSBitSel) &values) {
                m_hdmiSBitSel = values; };

            /**
             * @}
             */

            ////////////////////////////////////////////////////////////////////

            /**
             * @name Trigger tap delays
             * @{
             */
            /**
             * @brief Retrieves the sot tap delay for the given VFAT.
             */
            std::uint32_t getSotTapDelay(std::size_t vfat) const {
                return m_sotTapDelays.at(vfat); };

            /**
             * @brief Retrieves all sot tap delays.
             */
            auto getSotTapDelays() const -> const decltype(m_sotTapDelays) & {
                return m_sotTapDelays; };

            /**
             * @brief Retrieves all sot tap delays.
             */
            auto getSotTapDelays() -> decltype(m_sotTapDelays) & {
                return m_sotTapDelays; };

            /**
             * @brief Modifies the sot tap delay for the given VFAT.
             */
            void setSotTapDelay(std::size_t vfat, std::uint32_t delay) {
                m_sotTapDelays.at(vfat) = delay; };

            /**
             * @brief Modifies all sot tap delays.
             */
            void setSotTapDelays(const decltype(m_sotTapDelays) &delays) {
                m_sotTapDelays = delays; };

            ////////////////////////////////////////////////////////////////////

            /**
             * @brief Retrieves the trigger tap delay at the given index for the
             *        given VFAT.
             */
            std::uint32_t getTrigTapDelay(std::size_t vfat, std::size_t index) const {
                return m_trigTapDelays.at(vfat).at(index); };

            /**
             * @brief Retrieves all trigger tap delays for the given VFAT.
             */
            const TrigTapDelayBits &getTrigTapDelays(std::size_t vfat) const {
                return m_trigTapDelays.at(vfat); };

            /**
             * @brief Retrieves all trigger tap delays for the given VFAT.
             */
            TrigTapDelayBits &getTrigTapDelays(std::size_t vfat) {
                return m_trigTapDelays.at(vfat); };

            /**
             * @brief Retrieves all trigger tap delays.
             */
            auto getTrigTapDelays() const -> const decltype(m_trigTapDelays) & {
                return m_trigTapDelays; };

            /**
             * @brief Retrieves all trigger tap delays.
             */
            auto getTrigTapDelays() -> decltype(m_trigTapDelays) & {
                return m_trigTapDelays; };

            /**
             * @brief Modifies the trigger tap delay at the given index for the
             *        given VFAT.
             */
            void setTrigTapDelay(std::size_t vfat,
                                 std::size_t index,
                                 std::uint32_t delay) {
                m_trigTapDelays.at(vfat).at(index) = delay; };

            /**
             * @brief Modifies the trigger tap delays for the given VFAT.
             */
            void setTrigTapDelay(std::size_t vfat,
                                 const TrigTapDelayBits &delays) {
                m_trigTapDelays.at(vfat) = delays; };

            /**
             * @brief Modifies all trigger tap delays.
             */
            void setTrigTapDelays(const decltype(m_trigTapDelays) &delays) {
                m_trigTapDelays = delays; };

            /**
             * @}
             */

            ////////////////////////////////////////////////////////////////////

            /**
             * @name Child VFAT configuration
             * @{
             */
            /**
             * @brief Retrieves the configuration of the given VFAT, if set.
             */
            const std::shared_ptr<VFAT3ChipConfiguration> getVFATConfig(
                std::size_t vfat) const {
                return m_vfatConfigs.at(vfat); };

            /**
             * @brief Retrieves the configuration of the given VFAT, if set.
             */
            std::shared_ptr<VFAT3ChipConfiguration> getVFATConfig(std::size_t vfat) {
                return m_vfatConfigs.at(vfat); };

            /**
             * @brief Retrieves the configuration of all VFATs.
             */
            auto getVFATConfigs() const -> const decltype(m_vfatConfigs) & {
                return m_vfatConfigs; };

            /**
             * @brief Retrieves the configuration of all VFATs.
             */
            auto getVFATConfigs() -> decltype(m_vfatConfigs) & { return m_vfatConfigs; };

            /**
             * @brief Modifies the configuration of the given VFAT.
             */
            void setVFATConfig(std::size_t vfat,
                               const std::shared_ptr<VFAT3ChipConfiguration> &config) {
                m_vfatConfigs.at(vfat) = config; };

            /**
             * @brief Modifies the configuration of all VFATs.
             */
            void setVFATConfigs(const decltype(m_vfatConfigs) &configs) {
                m_vfatConfigs = configs; };

            /**
             * @brief Unsets the configuration of the given VFAT.
             */
            void unsetVFATConfig(std::size_t vfat) { setVFATConfig(vfat, nullptr); };

            /**
             * @brief Unsets all VFAT configurations.
             */
            void unsetVFATConfigs() {
                for (auto &config : m_vfatConfigs) {
                    config = nullptr;
                }
            };

            /**
             * @brief Checks that all VFAT configurations are set.
             */
            bool hasAllVFATConfigs() const {
                return 0 == std::count(getVFATConfigs().begin(),
                                       getVFATConfigs().end(),
                                       nullptr);
            };

            /**
             * @brief Checks that no VFAT configurations is set.
             */
            bool hasNoVFATConfig() const {
                return VFAT_COUNT == std::count(getVFATConfigs().begin(),
                                                getVFATConfigs().end(),
                                                nullptr);
            };

            /**
             * @brief Default-constructs all VFAT configurations.
             */
            void createAllVFATConfigs();

            /**
             * @brief Initializes all VFAT configurations with copies of
             *        @c config.
             */
            void createAllVFATConfigs(const VFAT3ChipConfiguration &config);

            /**
             * @}
             */
        };

        template<>
        class ConfigurationTraits<OHv3Configuration> :
            public ConfigurationTraits<detail::OHv3ConfigurationGen>
        {};

    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_OHV3CONFIGURATION_H
