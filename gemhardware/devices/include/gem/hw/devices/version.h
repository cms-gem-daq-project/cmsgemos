/** @file version.h */

#ifndef GEM_HW_DEVICES_VERSION_H
#define GEM_HW_DEVICES_VERSION_H

#ifndef DOXYGEN_IGNORE_THIS

#include "config/PackageInfo.h"

namespace gemhwdevices {

#define GEMHWDEVICES_VERSION_MAJOR 1
#define GEMHWDEVICES_VERSION_MINOR 0
#define GEMHWDEVICES_VERSION_PATCH 1
#define GEMHWDEVICES_PREVIOUS_VERSIONS "0.0.0,0.1.0,0.2.0,0.2.1,0.2.2,0.2.3,0.3.0,0.3.1,0.3.2,0.4.0,0.99.0,0.99.1,1.0.0"

#define GEMHWDEVICES_VERSION_CODE PACKAGE_VERSION_CODE(GEMHWDEVICES_VERSION_MAJOR, GEMHWDEVICES_VERSION_MINOR, GEMHWDEVICES_VERSION_PATCH)
#ifndef GEMHWDEVICES_PREVIOUS_VERSIONS
#define GEMHWDEVICES_FULL_VERSION_LIST  PACKAGE_VERSION_STRING(GEMHWDEVICES_VERSION_MAJOR, GEMHWDEVICES_VERSION_MINOR, GEMHWDEVICES_VERSION_PATCH)
#else
#define GEMHWDEVICES_FULL_VERSION_LIST  GEMHWDEVICES_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEMHWDEVICES_VERSION_MAJOR, GEMHWDEVICES_VERSION_MINOR, GEMHWDEVICES_VERSION_PATCH)
#endif

  const std::string package     = "gemhwdevices";
  const std::string versions    = GEMHWDEVICES_FULL_VERSION_LIST;
  const std::string summary     = "HW interfaces for GEM devices";
  const std::string description = "";
  const std::string authors     = "GEM Online Systems Group";
  const std::string link        = "https://cms-gem-daq-project.github.io/cmsgemos/";

  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string,std::less<std::string> > getPackageDependencies();
}

#endif // DOXYGEN_IGNORE_THIS

#endif // GEM_HW_VERSION_H
