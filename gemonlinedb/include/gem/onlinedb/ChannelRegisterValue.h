#ifndef GEM_ONLINEDB_CHANNELREGISTERVALUE_H
#define GEM_ONLINEDB_CHANNELREGISTERVALUE_H

#include <nlohmann/json.hpp>

namespace gem {
    namespace onlinedb {
        /**
         * @brief Represents the value of a channel register.
         */
        union ChannelRegisterValue
        {
            /**
             * @brief The raw value.
             */
            std::uint16_t registerValue;

            /**
             * @brief Individual settings making up the value.
             */
            struct {
                /**
                 * @brief Arming comparator threshold trimming: amplitude.
                 */
                std::uint16_t armDACCorrectionAmplitude : 6;

                /**
                 * @brief Arming comparator threshold trimming: polarity.
                 */
                bool armDACCorrectionPolarity : 1;

                /**
                 * @brief Zero crossing comparator timing optimization:
                 *        amplitude.
                 */
                std::uint16_t zccDACCorrectionAmplitude : 6;

                /**
                 * @brief Zero crossing comparator timing optimization:
                 *        polarity.
                 */
                bool zccDACCorrectionPolarity : 1;

                /**
                 * @brief Mask the channel output for trigger and readout.
                 */
                bool mask : 1;

                /**
                 * @brief Enable the calibration pulse to the channel input.
                 */
                bool cal : 1;
            } fields;

            /**
             * @brief Compares two register unions for equality.
             */
            inline bool operator== (const ChannelRegisterValue other) const
            {
                return registerValue == other.registerValue;
            }
        };

        // Check
        static_assert(sizeof(ChannelRegisterValue) == sizeof(std::uint16_t),
                      "ChannelRegisterValue not packed in the expected way");

        /**
         * @brief Converts @ref ChannelRegisterValue to JSON
         *
         * @see https://github.com/nlohmann/json#arbitrary-types-conversions
         * @related ChannelRegisterValue
         */
        inline void to_json(nlohmann::json &json, const ChannelRegisterValue &data)
        {
            json = data.registerValue;
        }

        /**
         * @brief Converts JSON to @ref ChannelRegisterValue
         *
         * @see https://github.com/nlohmann/json#arbitrary-types-conversions
         * @related ChannelRegisterValue
         */
        inline void from_json(const nlohmann::json &json, ChannelRegisterValue &data)
        {
            data.registerValue = json.get<std::uint16_t>();
        }

    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_CHANNELREGISTERVALUE_H
