#include "gem/onlinedb/AMC13Configuration.h"

#include <sstream>

#include <xercesc/dom/DOMXPathResult.hpp>

#include "gem/onlinedb/SerializationData.h"
#include "gem/onlinedb/detail/XMLUtils.h"
#include "gem/onlinedb/exception/Exception.h"

XERCES_CPP_NAMESPACE_USE

namespace gem {
    namespace onlinedb {
        template<>
        AMC13Configuration DBInterface::convertRow<AMC13Configuration>(
            const xdata::Table &table, xdata::Table::Row &row)
        {
            AMC13Configuration config;

            config.setFEDId(toInt(row, "FED_ID"));
            config.setEnableLocalTTC(toInt(row, "ENABLE_LOCALTTC"));

            auto value = row.getField("HOSTNAME");
            if (value == nullptr) {
                XCEPT_RAISE(exception::ParseError,
                            "In database field \"HOSTNAME\": Null value");
            } else if (value->type() == "string") {
                config.setHostname(value->toString());
            } else {
                XCEPT_RAISE(exception::ParseError,
                            "In database field \"HOSTNAME\": Unknown type " +
                            value->type());
            }

            return config;
        }
    } /* namespace onlinedb */
} /* namespace gem */
