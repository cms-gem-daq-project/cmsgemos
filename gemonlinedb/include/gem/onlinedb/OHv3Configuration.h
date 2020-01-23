#ifndef GEM_ONLINEDB_OHV3CONFIGURATION_H
#define GEM_ONLINEDB_OHV3CONFIGURATION_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>

#include "gem/onlinedb/detail/OHv3ConfigurationGen.h"

namespace gem {
    namespace onlinedb {

        // Forward declarations
        class GBTXConfiguration;
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
             * @brief The number of GBTX on an OH.
             */
            static const constexpr std::size_t GBTX_COUNT = 3;

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
            std::array<std::shared_ptr<VFAT3ChipConfiguration>, VFAT_COUNT> m_vfatConfigs;
            std::array<std::shared_ptr<GBTXConfiguration>, GBTX_COUNT> m_gbtxConfigs;

        public:
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

            ////////////////////////////////////////////////////////////////////

            /**
             * @name Child GBTX configuration
             * @{
             */
            /**
             * @brief Retrieves the configuration of the given GBTX, if set.
             */
            const std::shared_ptr<GBTXConfiguration> getGBTXConfig(
                std::size_t vfat) const {
                return m_gbtxConfigs.at(vfat); };

            /**
             * @brief Retrieves the configuration of the given GBTX, if set.
             */
            std::shared_ptr<GBTXConfiguration> getGBTXConfig(std::size_t vfat) {
                return m_gbtxConfigs.at(vfat); };

            /**
             * @brief Retrieves the configuration of all GBTXs.
             */
            auto getGBTXConfigs() const -> const decltype(m_gbtxConfigs) & {
                return m_gbtxConfigs; };

            /**
             * @brief Retrieves the configuration of all GBTXs.
             */
            auto getGBTXConfigs() -> decltype(m_gbtxConfigs) & { return m_gbtxConfigs; };

            /**
             * @brief Modifies the configuration of the given GBTX.
             */
            void setGBTXConfig(std::size_t vfat,
                               const std::shared_ptr<GBTXConfiguration> &config) {
                m_gbtxConfigs.at(vfat) = config; };

            /**
             * @brief Modifies the configuration of all GBTXs.
             */
            void setGBTXConfigs(const decltype(m_gbtxConfigs) &configs) {
                m_gbtxConfigs = configs; };

            /**
             * @brief Unsets the configuration of the given GBTX.
             */
            void unsetGBTXConfig(std::size_t vfat) { setGBTXConfig(vfat, nullptr); };

            /**
             * @brief Unsets all GBTX configurations.
             */
            void unsetGBTXConfigs() {
                for (auto &config : m_gbtxConfigs) {
                    config = nullptr;
                }
            };

            /**
             * @brief Checks that all GBTX configurations are set.
             */
            bool hasAllGBTXConfigs() const {
                return 0 == std::count(getGBTXConfigs().begin(),
                                       getGBTXConfigs().end(),
                                       nullptr);
            };

            /**
             * @brief Checks that no GBTX configurations is set.
             */
            bool hasNoGBTXConfig() const {
                return GBTX_COUNT == std::count(getGBTXConfigs().begin(),
                                                getGBTXConfigs().end(),
                                                nullptr);
            };

            /**
             * @brief Default-constructs all GBTX configurations.
             */
            void createAllGBTXConfigs();

            /**
             * @brief Initializes all GBTX configurations with copies of
             *        @c config.
             */
            void createAllGBTXConfigs(const GBTXConfiguration &config);

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
