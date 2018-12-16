#ifndef GEM_ONLINEDB_ONLINEDB_H
#define GEM_ONLINEDB_ONLINEDB_H

/**
 * @file
 * @brief Includes all of the online database support library.
 */

#include "gem/onlinedb/AMC13Configuration.h"
#include "gem/onlinedb/AMCConfiguration.h"
#include "gem/onlinedb/OHv3Configuration.h"
#include "gem/onlinedb/VFAT3ChipConfiguration.h"

#include "gem/onlinedb/ConfigurationTraits.h"
#include "gem/onlinedb/DataSet.h"
#include "gem/onlinedb/PartReference.h"
#include "gem/onlinedb/Run.h"
#include "gem/onlinedb/XMLSerializationData.h"

/**
 * @namespace gem::onlinedb
 * @brief Online database support library.
 *
 * The online database support library provides objects representing the
 * configuration of readout hardware, and utilities to serialize them. Much of
 * the functionality is achieved through template metaprogramming using the
 * @c Configuration concept (see below).
 *
 * The following classes represent the configuration of an element in the
 * readout chain:
 *
 * - @ref VFAT3ChannelConfiguration
 * - @ref VFAT3ChipConfiguration
 * - @ref OHv3Configuration
 * - @ref AMCConfiguration
 * - @ref AMC13Configuration
 *
 * ## XML serialization
 *
 * Configuration objects can be stored to (and read back from) a standardized
 * XML format. The contents of XML documents is represented by
 * @ref XMLSerializationData.
 *
 * ## The Configuration concept
 *
 * The @c Configuration concept requires that configuration classes implement
 * the following:
 *
 * 1. The class must have two methods called @c getRegisterData and
 *    @c readRegisterData that serialize the object state into
 *    @ref detail::RegisterData.
 * 2. The traits template @ref ConfigurationTraits must be specialized for the
 *    configuration type and expose the following members:
 *
 *        static std::string extTableName();
 *        static std::string typeName();
 *        static std::string kindOfPart();
 *        typename PartType;
 *
 *    @c PartType must be one of @ref PartReferenceBarcode or
 *    @ref PartReferenceSN.
 *
 * All configuration classes realize the @c Configuration concept, except that
 * @c AMC13Configuration doesn't have @c getRegisterData and
 * @c readRegisterData. Functions that require them are instead specialized to
 * provide the same functionality.
 */

/**
 * @namespace gem::onlinedb::detail
 * @brief Implementation details for @c gem::onlinedb.
 *
 * The contents this namespace are not part of the public API and may change
 * without notice.
 */

#endif // GEM_ONLINEDB_ONLINEDB_H
