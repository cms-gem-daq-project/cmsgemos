#ifndef GEM_ONLINEDB_AMCCONFIGURATION_H
#define GEM_ONLINEDB_AMCCONFIGURATION_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>

#include "gem/onlinedb/detail/AMCConfigurationGen.h"

namespace gem {
    namespace onlinedb {

        // Forward declaration
        class OHv3Configuration;

        /**
         * @brief Describes the configuration of an AMC.
         *
         * @note Modifying objects of this class doesn't affect hardware.
         */
        class AMCConfiguration: public detail::AMCConfigurationGen
        {
        public:
            /**
             * @brief The number of optohybrid connections.
             */
            static const constexpr std::size_t OH_CONNECTION_COUNT = 12;

        private:
            std::array<
                std::shared_ptr<OHv3Configuration>,
                OH_CONNECTION_COUNT
            > m_ohConfigs;

        public:
            /**
             * @brief Compares two OHv3 configurations for equality.
             */
            bool operator== (const AMCConfiguration &other) const;

            ////////////////////////////////////////////////////////////////////

            /**
             * @name Child OH configuration
             * @{
             */
            /**
             * @brief Retrieves the configuration of the given OH, if set.
             */
            const std::shared_ptr<OHv3Configuration> getOHConfig(
                std::size_t oh) const {
                return m_ohConfigs.at(oh); };

            /**
             * @brief Retrieves the configuration of the given OH, if set.
             */
            std::shared_ptr<OHv3Configuration> getOHConfig(std::size_t oh) {
                return m_ohConfigs.at(oh); };

            /**
             * @brief Retrieves the configuration of all OHs.
             */
            auto getOHConfigs() const -> const decltype(m_ohConfigs) & {
                return m_ohConfigs; };

            /**
             * @brief Retrieves the configuration of all OHs.
             */
            auto getOHConfigs() -> decltype(m_ohConfigs) & { return m_ohConfigs; };

            /**
             * @brief Modifies the configuration of the given OH.
             */
            void setOHConfig(std::size_t oh,
                             const std::shared_ptr<OHv3Configuration> &config) {
                m_ohConfigs.at(oh) = config; };

            /**
             * @brief Modifies the configuration of all OHs.
             */
            void setOHConfigs(const decltype(m_ohConfigs) &configs) {
                m_ohConfigs = configs; };

            /**
             * @brief Unsets the configuration of the given OH.
             */
            void unsetOHConfig(std::size_t oh) { setOHConfig(oh, nullptr); };

            /**
             * @brief Unsets all OH configurations.
             */
            void unsetOHConfigs() {
                for (auto &config : getOHConfigs()) {
                    config = nullptr;
                }
            };

            /**
             * @brief Checks that all OH configurations are set.
             */
            bool hasAllOHConfigs() const {
                return 0 == std::count(getOHConfigs().begin(),
                                       getOHConfigs().end(),
                                       nullptr);
            };

            /**
             * @brief Checks that no OH configurations is set.
             */
            bool hasNoOHConfig() const {
                return OH_CONNECTION_COUNT == std::count(getOHConfigs().begin(),
                                                         getOHConfigs().end(),
                                                         nullptr);
            };

            /**
             * @brief Default-constructs all OH configurations.
             */
            void createAllOHConfigs();

            /**
             * @brief Initializes all OH configurations with copies of
             *        @c config.
             */
            void createAllOHConfigs(const OHv3Configuration &config);

            /**
             * @}
             */
        };

        template<>
        class ConfigurationTraits<AMCConfiguration> :
            public ConfigurationTraits<detail::AMCConfigurationGen>
        {};

    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_AMCCONFIGURATION_H
