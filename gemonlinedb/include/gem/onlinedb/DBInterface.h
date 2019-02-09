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
        private:
            /// @brief Converts a database field to an integer.
            static int toInt(xdata::Table::Row &row, const std::string &column);

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
                data[c] = toInt(row, c);
            }
            auto config = ConfigurationType();
            config.readRegisterData(data);
            return config;
        }
    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_DBINTERFACE_H
