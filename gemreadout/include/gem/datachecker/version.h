#ifndef gem_datachecker_version_h
#define gem_datachecker_version_h

#ifndef DOXYGEN_IGNORE_THIS

#include "config/PackageInfo.h"

//namespace gem {
//  namespace datachecker {
namespace gemdatachecker {

#define GEMDATACHECKER_VERSION_MAJOR 0
#define GEMDATACHECKER_VERSION_MINOR 1
#define GEMDATACHECKER_VERSION_PATCH 0
  //#define GEMDATACHECKER_PREVIOUS_VERSIONS
#undef GEMDATACHECKER_PREVIOUS_VERSIONS

#define GEMDATACHECKER_VERSION_CODE PACKAGE_VERSION_CODE(GEMDATACHECKER_VERSION_MAJOR,GEMDATACHECKER_VERSION_MINOR, GEMDATACHECKER_VERSION_PATCH)

#ifndef GEMDATACHECKER_PREVIOUS_VERSIONS
#define GEMDATACHECKER_FULL_VERSION_LIST PACKAGE_VERSION_STRING(GEMDATACHECKER_VERSION_MAJOR,GEMDATACHECKER_VERSION_MINOR, GEMDATACHECKER_VERSION_PATCH)
#else
#define GEMDATACHECKER_FULL_VERSION_LIST GEMDATACHECKER_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEMDATACHECKER_VERSION_MAJOR, GEMDATACHECKER_VERSION_MINOR,GEMDATACHECKER_VERSION_PATCH)
#endif
    
  const std::string package = "gem/datachecker";
  const std::string versions = GEMDATACHECKER_FULL_VERSION_LIST;
  const std::string summary = "GEM datachecker";
  const std::string description = "";
  const std::string authors = "GEM Online Systems Group";
  const std::string link = "";
    
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  //  }
}

#endif // DOXYGEN_IGNORE_THIS

#endif // gem_datachecker_version_h
