#ifndef GEM_ONLINEDB_CONFIGURATIONTRAITS_H
#define GEM_ONLINEDB_CONFIGURATIONTRAITS_H

namespace gem {
    namespace onlinedb {

        /**
         * @brief Template used to describe how a part is referenced by a
         *        configuration object.
         *
         * Specializations of this template must implement the following public
         * API:
         *
         *  * Three static functions called @c extTableName, @c typeName and
         *    @c kindOfPart returning respectively the name of the extension
         *    table, the type of extension and the kind of part configured, in a
         *    form convertible to @c std::string.
         *  * A @c PartType typedef pointing to one of the following classes:
         *
         *      * @ref PartReferenceBarcode
         *      * @ref PartReferenceSN
         */
        template<class ConfigurationType>
        class ConfigurationTraits;

    } /* namespace onlinedb */
} /* namespace gem */

#endif // GEM_ONLINEDB_CONFIGURATIONTRAITS_H
