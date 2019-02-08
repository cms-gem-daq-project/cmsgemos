#ifndef GEM_ONLINEDB_DBINTERFACE_H
#define GEM_ONLINEDB_DBINTERFACE_H

#include <xdata/Table.h>

#include "gem/onlinedb/detail/RegisterData.h"
#include "gem/onlinedb/exception/Exception.h"

namespace gem {
    namespace onlinedb {
        /**
         * @brief Used to interface with the database.
         */
        class DBInterface
        {
        public:
            /**
             * @brief Creates a configuration object from a database row.
             */
            template<class ConfigurationType>
            static ConfigurationType convertRow(const xdata::Table &table,
                                                xdata::Table::Row &row);
        };

        template<class ConfigurationType>
        ConfigurationType DBInterface::convertRow(const xdata::Table &table,
                                                  xdata::Table::Row &row)
        {
            auto columns = table.getColumns();
            detail::RegisterData data;
            for (const auto &c : columns) {
                auto value = row.getField(c);
                if (value != nullptr && value->type() == "string") {
                    auto str = value->toString();
                    try {
                        data[c] = std::stoi(str);
                    } catch (const std::invalid_argument &e) {
                        XCEPT_RAISE(exception::ParseError,
                                    "In database field \"" + c + "\": "
                                    "Cannot convert \"" + str + "\" to integer: " +
                                    e.what());
                    }
                } else {
                    XCEPT_RAISE(exception::ParseError,
                                "In database field \"" + c + "\": Null value");
                }
            }
            auto config = ConfigurationType();
            config.readRegisterData(data);
            return config;
        }
    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_DBINTERFACE_H
