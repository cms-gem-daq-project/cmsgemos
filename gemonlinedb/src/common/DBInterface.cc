#include "gem/onlinedb/DBInterface.h"

namespace gem {
    namespace onlinedb {
        int DBInterface::toInt(xdata::Table::Row &row,
                               const std::string &column)
        {
            auto value = row.getField(column);
            if (value == nullptr) {
                XCEPT_RAISE(exception::ParseError,
                            "In database field \"" + column + "\": Null value");
            } else if (value->type() == "int") {
                return *dynamic_cast<xdata::SimpleType<int> *>(value);
            } else if (value->type() == "string") {
                auto str = value->toString();
                try {
                    return std::stoi(str);
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
