#ifndef gem_base_version_h
#define gem_base_version_h

#include "config/PackageInfo.h"

#define GEM_BASE_VERSION_MAJOR 0
#define GEM_BASE_VERSION_MINOR 1
#define GEM_BASE_VERSION_PATCH 0

#undef GEM_BASE_PREVIOUS_VERSIONS

#define GEM_BASE_VERSION_CODE PACKAGE_VERSION_CODE(GEM_BASE_VERSION_MAJOR,GEM_BASE_VERSION_MINOR, GEM_BASE_VERSION_PATCH)

#ifndef GEM_BASE_PREVIOUS_VERSIONS
#define GEM_BASE_FULL_VERSION_LIST PACKAGE_VERSION_STRING(GEM_BASE_VERSION_MAJOR,GEM_BASE_VERSION_MINOR, GEM_BASE_VERSION_PATCH)

#else
#define GEM_BASE_FULL_VERSION_LIST GEM_BASE_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEM_BASE_VERSION_MAJOR, GEM_BASE_VERSION_MINOR,GEM_BASE_VERSION_PATCH)
#endif

namespace gem {
  namespace base {
    const std::string package = "base";
    const std::string versions = GEM_BASE_FULL_VERSION_LIST;
    const std::string summary = "Base classes for GEM DAQ applications";
    const std::string description = "Base classes for GEM DAQ applications";
    const std::string authors = "Jared Sturdy";
    const std::string link = "http://xdaq.web.cern.ch";
    config::PackageInfo getPackageInfo();
    void checkPackageDependencies() throw (config::PackageInfo::VersionException);
    std::set<std::string, std::less<std::string> > getPackageDependencies();
  }
}
#endif
