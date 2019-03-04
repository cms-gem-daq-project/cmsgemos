#include "gem/hw/utils/version.h"

GETPACKAGEINFO(gemhwutils);

void gemhwutils::checkPackageDependencies()
  throw (config::PackageInfo::VersionException)
{
}

std::set<std::string, std::less<std::string> > gemhwutils::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > deps;
  return deps;
}
