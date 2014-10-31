#ifndef gem_supervisor_version_h
#define gem_supervisor_version_h

#include "config/PackageInfo.h"

#define GEM_SUPERVISOR_VERSION_MAJOR 0
#define GEM_SUPERVISOR_VERSION_MINOR 1
#define GEM_SUPERVISOR_VERSION_PATCH 0

#undef GEM_SUPERVISOR_PREVIOUS_VERSIONS

#define GEM_SUPERVISOR_VERSION_CODE PACKAGE_VERSION_CODE(GEM_SUPERVISOR_VERSION_MAJOR,GEM_SUPERVISOR_VERSION_MINOR, GEM_SUPERVISOR_VERSION_PATCH)

#ifndef GEM_SUPERVISOR_PREVIOUS_VERSIONS
#define GEM_SUPERVISOR_FULL_VERSION_LIST PACKAGE_VERSION_STRING(GEM_SUPERVISOR_VERSION_MAJOR,GEM_SUPERVISOR_VERSION_MINOR, GEM_SUPERVISOR_VERSION_PATCH)

#else
#define GEM_SUPERVISOR_FULL_VERSION_LIST GEM_SUPERVISOR_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEM_SUPERVISOR_VERSION_MAJOR, GEM_SUPERVISOR_VERSION_MINOR,GEM_SUPERVISOR_VERSION_PATCH)
#endif

namespace gem {
  namespace supervisor {
    const std::string package = "supervisor";
    const std::string versions = GEM_SUPERVISOR_FULL_VERSION_LIST;
    const std::string summary = "GEM Supervisor";
    const std::string description = "GEM supervisor application";
    const std::string authors = "Jared Sturdy";
    const std::string link = "http://xdaq.web.cern.ch";
    config::PackageInfo getPackageInfo();
    void checkPackageDependencies() throw (config::PackageInfo::VersionException);
    std::set<std::string, std::less<std::string> > getPackageDependencies();
  }
}
#endif
