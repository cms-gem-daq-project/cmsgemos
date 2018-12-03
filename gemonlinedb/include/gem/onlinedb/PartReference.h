#ifndef _PartReference_h_
#define _PartReference_h_

#include <string>

namespace gem {
    namespace onlinedb {

        /**
         * @brief References a part by its barcode.
         */
        struct PartReferenceBarcode
        {
            std::string barcode;
        };

        /**
         * @brief References a part by its serial number.
         */
        struct PartReferenceSN
        {
            std::string serialNumber;
        };

    } /* namespace onlinedb */
} /* namespace gem */

#endif // _PartReference_h_
