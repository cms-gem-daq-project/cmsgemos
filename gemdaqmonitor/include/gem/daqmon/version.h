/** @file version.h */

#ifndef GEM_DAQMON_VERSION_H
#define GEM_DAQMON_VERSION_H

#ifndef DOXYGEN_IGNORE_THIS

#include "config/PackageInfo.h"

namespace gem {
  namespace daqmon {

#define GEMDAQMONITOR_VERSION_MAJOR 1
#define GEMDAQMONITOR_VERSION_MINOR 0
#define GEMDAQMONITOR_VERSION_PATCH 0
#define GEMDAQMONITOR_PREVIOUS_VERSIONS "0.0.0,0.1.0,0.2.0"

#define GEMDAQMONITOR_VERSION_CODE PACKAGE_VERSION_CODE(GEMDAQMONITOR_VERSION_MAJOR, GEMDAQMONITOR_VERSION_MINOR, GEMDAQMONITOR_VERSION_PATCH)

#ifndef GEMDAQMONITOR_PREVIOUS_VERSIONS
#define GEMDAQMONITOR_FULL_VERSION_LIST  PACKAGE_VERSION_STRING(GEMDAQMONITOR_VERSION_MAJOR, GEMDAQMONITOR_VERSION_MINOR, GEMDAQMONITOR_VERSION_PATCH)
#else
#define GEMDAQMONITOR_FULL_VERSION_LIST  GEMDAQMONITOR_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEMDAQMONITOR_VERSION_MAJOR, GEMDAQMONITOR_VERSION_MINOR, GEMDAQMONITOR_VERSION_PATCH)
#endif

    const std::string project     = "cmsgemos";
    const std::string package     = "daqmon";
    const std::string versions    = GEMDAQMONITOR_FULL_VERSION_LIST;
    const std::string summary     = "<FILL ME>";
    const std::string description = "";
    const std::string authors     = "GEM Online Systems Group";
    const std::string link        = "https://cms-gem-daq-project.github.io/cmsgemos/";

    config::PackageInfo getPackageInfo();
    void checkPackageDependencies();
    std::set<std::string,std::less<std::string> > getPackageDependencies();
  }
}

#endif // DOXYGEN_IGNORE_THIS

#endif // GEM_DAQMON_VERSION_H
