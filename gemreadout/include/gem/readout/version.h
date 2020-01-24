/** @file version.h */

#ifndef GEM_READOUT_VERSION_H
#define GEM_READOUT_VERSION_H

#ifndef DOXYGEN_IGNORE_THIS

#include "config/PackageInfo.h"

namespace gem {
  namespace readout {

#define GEMREADOUT_VERSION_MAJOR 1
#define GEMREADOUT_VERSION_MINOR 0
#define GEMREADOUT_VERSION_PATCH 0
#define GEMREADOUT_PREVIOUS_VERSIONS "0.0.0,0.1.0,0.99.0"

#define GEMREADOUT_VERSION_CODE PACKAGE_VERSION_CODE(GEMREADOUT_VERSION_MAJOR, GEMREADOUT_VERSION_MINOR, GEMREADOUT_VERSION_PATCH)

#ifndef GEMREADOUT_PREVIOUS_VERSIONS
#define GEMREADOUT_FULL_VERSION_LIST PACKAGE_VERSION_STRING(GEMREADOUT_VERSION_MAJOR, GEMREADOUT_VERSION_MINOR, GEMREADOUT_VERSION_PATCH)
#else
#define GEMREADOUT_FULL_VERSION_LIST GEMREADOUT_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEMREADOUT_VERSION_MAJOR, GEMREADOUT_VERSION_MINOR, GEMREADOUT_VERSION_PATCH)
#endif

    const std::string project     = "cmsgemos";
    const std::string package     = "readout";
    const std::string versions    = GEMREADOUT_FULL_VERSION_LIST;
    const std::string summary     = "GEM readout";
    const std::string description = "";
    const std::string authors     = "GEM Online Systems Group";
    const std::string link        = "https://cms-gem-daq-project.github.io/cmsgemos/";

    config::PackageInfo getPackageInfo();
    void checkPackageDependencies();
    std::set<std::string, std::less<std::string> > getPackageDependencies();
  }
}

#endif // DOXYGEN_IGNORE_THIS

#endif // GEM_READOUT_VERSION_H
