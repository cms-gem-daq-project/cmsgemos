#ifndef GEM_ONLINEDB_DBINTERFACE_H
#define GEM_ONLINEDB_DBINTERFACE_H

#include <xdata/Table.h>

#include "gem/onlinedb/detail/RegisterData.h"

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
                    if (!str.empty()) {
                        data[c] = std::stoi(value->toString());
                    } else {
                        // Empty value in database -> use 0
                        // This *does* actually happen...
                        data[c] = 0;
                    }
                } else {
                    // "null" in database -> use 0
                    // This *does* actually happen...
                    data[c] = 0;
                }
            }
            auto config = ConfigurationType();
            config.readRegisterData(data);
            return config;
        }
    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_DBINTERFACE_H
