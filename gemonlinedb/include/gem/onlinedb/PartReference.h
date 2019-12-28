#ifndef GEM_ONLINEDB_PARTREFERENCE_H
#define GEM_ONLINEDB_PARTREFERENCE_H

#include <string>

#include <nlohmann/json.hpp>

namespace gem {
    namespace onlinedb {

        /**
         * @brief References a part by its barcode.
         */
        struct PartReferenceBarcode
        {
            std::string barcode;

            /**
             * @brief Checks two barcode part references for equality.
             */
            bool operator== (const PartReferenceBarcode &other) const
            {
                return barcode == other.barcode;
            }
        };

        /**
         * @brief Converts @ref PartReferenceBarcode to JSON
         *
         * @see https://github.com/nlohmann/json#arbitrary-types-conversions
         * @see https://github.com/valdasraps/cmsdbldr/blob/master/src/main/java/org/cern/cms/dbloader/model/construct/Part.java
         * @related PartReferenceBarcode
         */
        inline void to_json(nlohmann::json &json, const PartReferenceBarcode &data)
        {
            json = {
                { "Part", nlohmann::json({
                    { "Barcode", data.barcode },
                })}
            };
        }

        /**
         * @brief Converts JSON to @ref PartReferenceBarcode
         *
         * @see https://github.com/nlohmann/json#arbitrary-types-conversions
         * @see https://github.com/valdasraps/cmsdbldr/blob/master/src/main/java/org/cern/cms/dbloader/model/construct/Part.java
         * @related PartReferenceBarcode
         */
        inline void from_json(const nlohmann::json &json, PartReferenceBarcode &data)
        {
            data.barcode = json.at("Part").at("Barcode");
        }

        /**
         * @brief References a part by its serial number.
         */
        struct PartReferenceSN
        {
            std::string serialNumber;

            /**
             * @brief Checks two S/N part references for equality.
             */
            bool operator== (const PartReferenceSN &other) const
            {
                return serialNumber == other.serialNumber;
            }
        };

        /**
         * @brief Converts @ref PartReferenceSN to JSON
         *
         * @see https://github.com/nlohmann/json#arbitrary-types-conversions
         * @see https://github.com/valdasraps/cmsdbldr/blob/master/src/main/java/org/cern/cms/dbloader/model/construct/Part.java
         * @related PartReferenceSN
         */
        inline void to_json(nlohmann::json &json, const PartReferenceSN &data)
        {
            json = {
                { "Part", nlohmann::json({
                    { "SerialNumber", data.serialNumber },
                })}
            };
        }

        /**
         * @brief Converts JSON to @ref PartReferenceSN
         *
         * @see https://github.com/nlohmann/json#arbitrary-types-conversions
         * @see https://github.com/valdasraps/cmsdbldr/blob/master/src/main/java/org/cern/cms/dbloader/model/construct/Part.java
         * @related PartReferenceSN
         */
        inline void from_json(const nlohmann::json &json, PartReferenceSN &data)
        {
            data.serialNumber = json.at("Part").at("SerialNumber");
        }

    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_PARTREFERENCE_H
