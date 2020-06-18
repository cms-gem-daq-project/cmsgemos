/** @file version.h */

#ifndef GEM_HW_VERSION_H
#define GEM_HW_VERSION_H

#ifndef DOXYGEN_IGNORE_THIS

#include "config/PackageInfo.h"

namespace gemhardware {

#define GEMHARDWARE_VERSION_MAJOR 1
#define GEMHARDWARE_VERSION_MINOR 0
#define GEMHARDWARE_VERSION_PATCH 0
#define GEMHARDWARE_PREVIOUS_VERSIONS "0.0.0,0.1.0,0.2.0,0.2.1,0.2.2,0.2.3,0.3.0,0.3.1,0.3.2,0.4.0,0.99.0,0.99.1"

#define GEMHARDWARE_VERSION_CODE PACKAGE_VERSION_CODE(GEMHARDWARE_VERSION_MAJOR, GEMHARDWARE_VERSION_MINOR, GEMHARDWARE_VERSION_PATCH)
#ifndef GEMHARDWARE_PREVIOUS_VERSIONS
#define GEMHARDWARE_FULL_VERSION_LIST  PACKAGE_VERSION_STRING(GEMHARDWARE_VERSION_MAJOR, GEMHARDWARE_VERSION_MINOR, GEMHARDWARE_VERSION_PATCH)
#else
#define GEMHARDWARE_FULL_VERSION_LIST  GEMHARDWARE_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEMHARDWARE_VERSION_MAJOR, GEMHARDWARE_VERSION_MINOR, GEMHARDWARE_VERSION_PATCH)
#endif

  const std::string package     = "gemhardware";
  const std::string versions    = GEMHARDWARE_FULL_VERSION_LIST;
  const std::string summary     = "HW interfaces for GEM devices";
  const std::string description = "";
  const std::string authors     = "GEM Online Systems Group";
  const std::string link        = "https://cmsgemdaq.web.cern.ch/cmsgemdaq//";

  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string,std::less<std::string> > getPackageDependencies();
}

#endif // DOXYGEN_IGNORE_THIS

#endif // GEM_HW_VERSION_H
