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
#include "gem/onlinedb/SerializationData.h"

#include "gem/onlinedb/ConfigurationLinker.h"
#include "gem/onlinedb/ConfigurationManager.h"
#include "gem/onlinedb/ConfigurationProvider.h"

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
 * The main entry point to get your hands on configuration objects is
 * @ref ConfigurationManager. This class maintains the settings that should be
 * used when configuring detectors. Read-only access is provided through
 * @ref ConfigurationManager::getConfiguration, which is typically used as
 * follows:
 *
 *    auto lock = ConfigurationManager::makeReadLock();
 *    auto &config = ConfigurationManager::getConfiguration(lock);
 *    // Do something with the config...
 *    // The lock is released automatically when going out of scope
 *
 * The lock prevents any modification of the data while it is being used, so you
 * should restrict its lifetime to the strict minimum.
 *
 * ## Exceptions
 *
 * Unless otherwise stated, all functions in this module can throw exceptions
 * that inherit @c std::exception. In addition, functions that manipulate XML
 * data may throw a set of Xerces-specific exceptions, but you shouldn't need to
 * use them directly.
 *
 * ## Thread safety
 *
 * All object methods in this module are reentrant (can be called on different
 * objects from different threads). However, most method are <b>not</b>
 * thread-safe (can be called on the same object from different threads).
 *
 * ## XML serialization
 *
 * Configuration objects can be stored to (and read back from) a standardized
 * XML format. The contents of XML documents is represented by
 * @ref SerializationData.
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
