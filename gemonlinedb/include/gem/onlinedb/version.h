/** @file version.h */

#ifndef GEM_ONLINEDB_VERSION_H
#define GEM_ONLINEDB_VERSION_H

#ifndef DOXYGEN_IGNORE_THIS

#include "config/PackageInfo.h"

namespace gem {
  namespace onlinedb {

#define GEMONLINEDB_VERSION_MAJOR 0
#define GEMONLINEDB_VERSION_MINOR 1
#define GEMONLINEDB_VERSION_PATCH 0
#define GEMONLINEDB_PREVIOUS_VERSIONS ",0.0.0"

#define GEMONLINEDB_VERSION_CODE PACKAGE_VERSION_CODE(GEMONLINEDB_VERSION_MAJOR, GEMONLINEDB_VERSION_MINOR, GEMONLINEDB_VERSION_PATCH)

#ifndef GEMONLINEDB_PREVIOUS_VERSIONS
#define GEMONLINEDB_FULL_VERSION_LIST  PACKAGE_VERSION_STRING(GEMONLINEDB_VERSION_MAJOR, GEMONLINEDB_VERSION_MINOR, GEMONLINEDB_VERSION_PATCH)
#else
#define GEMONLINEDB_FULL_VERSION_LIST  GEMONLINEDB_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEMONLINEDB_VERSION_MAJOR, GEMONLINEDB_VERSION_MINOR, GEMONLINEDB_VERSION_PATCH)
#endif

    const std::string project     = "cmsgemos";
    const std::string package     = "onlinedb";
    const std::string versions    = GEMONLINEDB_FULL_VERSION_LIST;
    const std::string summary     = "<FILL ME>";
    const std::string description = "Utilities for connecting to DB resources";
    const std::string authors     = "GEM Online Systems Group";
    const std::string link        = "https://cms-gem-daq-project.github.io/cmsgemos/";

    config::PackageInfo getPackageInfo();
    void checkPackageDependencies();
    std::set<std::string,std::less<std::string> > getPackageDependencies();
  }
}

#endif // DOXYGEN_IGNORE_THIS

#endif // GEM_ONLINEDB_VERSION_H
