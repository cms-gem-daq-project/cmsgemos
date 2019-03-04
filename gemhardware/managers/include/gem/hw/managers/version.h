/** @file version.h */

#ifndef GEM_HW_MANAGERS_VERSION_H
#define GEM_HW_MANAGERS_VERSION_H

#ifndef DOXYGEN_IGNORE_THIS

#include "config/PackageInfo.h"

namespace gemhwmanagers {

#define GEMHWMANAGERS_VERSION_MAJOR 1
#define GEMHWMANAGERS_VERSION_MINOR 0
#define GEMHWMANAGERS_VERSION_PATCH 1
#define GEMHWMANAGERS_PREVIOUS_VERSIONS "0.0.0,0.1.0,0.2.0,0.2.1,0.2.2,0.2.3,0.3.0,0.3.1,0.3.2,0.4.0,0.99.0,0.99.1,1.0.0"

#define GEMHWMANAGERS_VERSION_CODE PACKAGE_VERSION_CODE(GEMHWMANAGERS_VERSION_MAJOR, GEMHWMANAGERS_VERSION_MINOR, GEMHWMANAGERS_VERSION_PATCH)
#ifndef GEMHWMANAGERS_PREVIOUS_VERSIONS
#define GEMHWMANAGERS_FULL_VERSION_LIST  PACKAGE_VERSION_STRING(GEMHWMANAGERS_VERSION_MAJOR, GEMHWMANAGERS_VERSION_MINOR, GEMHWMANAGERS_VERSION_PATCH)
#else
#define GEMHWMANAGERS_FULL_VERSION_LIST  GEMHWMANAGERS_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEMHWMANAGERS_VERSION_MAJOR, GEMHWMANAGERS_VERSION_MINOR, GEMHWMANAGERS_VERSION_PATCH)
#endif

  const std::string package     = "gemhwmanagers";
  const std::string versions    = GEMHWMANAGERS_FULL_VERSION_LIST;
  const std::string summary     = "HW interfaces for GEM managers";
  const std::string description = "";
  const std::string authors     = "GEM Online Systems Group";
  const std::string link        = "https://cms-gem-daq-project.github.io/cmsgemos/";

  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string,std::less<std::string> > getPackageDependencies();
}

#endif // DOXYGEN_IGNORE_THIS

#endif // GEM_HW_VERSION_H
