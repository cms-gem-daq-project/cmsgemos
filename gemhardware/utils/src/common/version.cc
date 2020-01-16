#include "gem/hw/utils/version.h"

GETPACKAGEINFO(gem::hw::utils);

void gem::hw::utils::checkPackageDependencies()
  throw (config::PackageInfo::VersionException)
{
}

std::set<std::string, std::less<std::string> > gem::hw::utils::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > deps;
  return deps;
}
