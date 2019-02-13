#include "gem/onlinedb/DBInterface.h"

namespace gem {
    namespace onlinedb {
        std::uint32_t DBInterface::toInt(xdata::Table::Row &row,
                                         const std::string &column)
        {
            auto value = row.getField(column);
            if (value == nullptr) {
                XCEPT_RAISE(exception::ParseError,
                            "In database field \"" + column + "\": Null value");
            } else if (value->type() == "int") {
                return *dynamic_cast<xdata::SimpleType<int> *>(value);
            } else if (value->type() == "string") {
                /*
                 * Oracle databases don't have types like "int32" or "uint32",
                 * but instead rely on the number of decimal digits (because
                 * numbers are stored in plain text). The smallest database
                 * field that can contain any (u)int32 allows for all integer
                 * values < 10^11. Not all of them can be represented on 32
                 * bits.
                 *
                 * Unfortunately, TStore will report as "string" any numeric
                 * database field that can store values larger than INT_MAX.
                 * Since int is 32 bits on most platforms, database fields that
                 * can contain uint32 (and hence allow for 10 digits) are
                 * reported as strings.
                 *
                 * So below we convert the string returned by TStore to uint32,
                 * paying attention to overflow and conversion errors.
                 */
                auto str = value->toString();
                try {
                    auto value = std::stoul(str);
                    if (value > std::numeric_limits<std::uint32_t>().max()) {
                        XCEPT_RAISE(exception::ParseError,
                                    "In database field \"" + column + "\": "
                                    "Value " + std::to_string(value) + " too "
                                    "large");
                    }
                    return value;
                } catch (const std::invalid_argument &e) {
                    XCEPT_RAISE(exception::ParseError,
                                "In database field \"" + column + "\": "
                                "Cannot convert \"" + str + "\" to integer: " +
                                e.what());
                }
            } else {
                XCEPT_RAISE(exception::ParseError,
                            "In database field \"" + column +
                            "\": Unknown type " + value->type());
            }
        }
    } /* namespace onlinedb */
} /* namespace gem */
