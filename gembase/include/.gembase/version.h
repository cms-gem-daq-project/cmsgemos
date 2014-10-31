#ifndef _gembase_version_h_
#define _gembase_version_h_

#include "config/PackageInfo.h"

#define GEMBASE_VERSION_MAJOR 0
#define GEMBASE_VERSION_MINOR 1
#define GEMBASE_VERSION_PATCH 0

#undef GEMBASE_PREVIOUS_VERSIONS

#define GEMBASE_VERSION_CODE PACKAGE_VERSION_CODE(GEMBASE_VERSION_MAJOR,GEMBASE_VERSION_MINOR, GEMBASE_VERSION_PATCH)

#ifndef GEMBASE_PREVIOUS_VERSIONS
#define GEMBASE_FULL_VERSION_LIST PACKAGE_VERSION_STRING(GEMBASE_VERSION_MAJOR,GEMBASE_VERSION_MINOR, GEMBASE_VERSION_PATCH)

#else
#define GEMBASE_FULL_VERSION_LIST GEMBASE_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(GEMBASE_VERSION_MAJOR, GEMBASE_VERSION_MINOR,GEMBASE_VERSION_PATCH)
#endif

namespace gembase
{
  const std::string package = "gembase";
  const std::string versions = GEMBASE_FULL_VERSION_LIST;
  const std::string summary = "Base classes for GEM DAQ applications";
  const std::string description = "Base classes for GEM DAQ applications";
  const std::string authors = "Jared Sturdy";
  const std::string link = "http://xdaq.web.cern.ch";
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
}
#endif
