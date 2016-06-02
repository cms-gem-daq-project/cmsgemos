#ifndef GEM_DATACHECKER_VERSION_H
#define GEM_DATACHECKER_VERSION_H

#ifndef DOXYGEN_IGNORE_THIS

#include "config/PackageInfo.h"

namespace gemdatachecker {

#define GEMDATACHECKER_VERSION_MAJOR 0
#define GEMDATACHECKER_VERSION_MINOR 2
#define GEMDATACHECKER_VERSION_PATCH 0
#define GEMDATACHECKER_PREVIOUS_VERSIONS = "0.0.0,0.1.0"

#define GEMDATACHECKER_VERSION_CODE PACKAGE_VERSION_CODE(GEMDATACHECKER_VERSION_MAJOR, GEMDATACHECKER_VERSION_MINOR, GEMDATACHECKER_VERSION_PATCH)

#ifndef GEMDATACHECKER_PREVIOUS_VERSIONS
#define GEMDATACHECKER_FULL_VERSION_LIST PACKAGE_VERSION_STRING(GEMDATACHECKER_VERSION_MAJOR, GEMDATACHECKER_VERSION_MINOR, GEMDATACHECKER_VERSION_PATCH)
#else
#define GEMDATACHECKER_FULL_VERSION_LIST GEMDATACHECKER_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEMDATACHECKER_VERSION_MAJOR, GEMDATACHECKER_VERSION_MINOR, GEMDATACHECKER_VERSION_PATCH)
#endif
    
  const std::string package     = "gemdatachecker";
  const std::string versions    = GEMDATACHECKER_FULL_VERSION_LIST;
  const std::string summary     = "GEM datachecker";
  const std::string description = "";
  const std::string authors     = "GEM Online Systems Group";
  const std::string link        = "";
    
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
}

#endif // DOXYGEN_IGNORE_THIS

#endif // GEM_DATACHECKER_VERSION_H
