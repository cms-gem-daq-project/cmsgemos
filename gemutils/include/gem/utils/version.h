#ifndef GEM_UTILS_VERSION_H
#define GEM_UTILS_VERSION_H

#ifndef DOXYGEN_IGNORE_THIS

#include "config/PackageInfo.h"

namespace gemutils {

#define GEMUTILS_VERSION_MAJOR 0
#define GEMUTILS_VERSION_MINOR 2
#define GEMUTILS_VERSION_PATCH 0
#define GEMUTILS_PREVIOUS_VERSIONS "0.0.0,0.1.0"

#define GEMUTILS_VERSION_CODE PACKAGE_VERSION_CODE(GEMUTILS_VERSION_MAJOR, GEMUTILS_VERSION_MINOR, GEMUTILS_VERSION_PATCH)

#ifndef GEMUTILS_PREVIOUS_VERSIONS
#define GEMUTILS_FULL_VERSION_LIST  PACKAGE_VERSION_STRING(GEMUTILS_VERSION_MAJOR, GEMUTILS_VERSION_MINOR, GEMUTILS_VERSION_PATCH)
#else
#define GEMUTILS_FULL_VERSION_LIST  GEMUTILS_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEMUTILS_VERSION_MAJOR, GEMUTILS_VERSION_MINOR, GEMUTILS_VERSION_PATCH)
#endif
    
  const std::string package     = "gemutils";
  const std::string versions    = GEMUTILS_FULL_VERSION_LIST;
  const std::string summary     = "Utilities for GEM online software";
  const std::string description = "";
  const std::string authors     = "GEM Online Systems Group";
  const std::string link        = "";
    
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string,std::less<std::string> > getPackageDependencies();
}

#endif // DOXYGEN_IGNORE_THIS

#endif // GEM_UTILS_VERSION_H
