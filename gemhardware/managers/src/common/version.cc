#include "toolbox/version.h"
#include "xdaq/version.h"
#include "xoap/version.h"
#include "gem/base/version.h"
#include "gem/utils/version.h"
#include "gem/hw/utils/version.h"
#include "gem/hw/managers/version.h"

GETPACKAGEINFO(gem::hw::managers);

void gem::hw::managers::checkPackageDependencies()
  throw (config::PackageInfo::VersionException)
{
  CHECKDEPENDENCY(toolbox);
  CHECKDEPENDENCY(xdaq);
  CHECKDEPENDENCY(xoap);
  CHECKDEPENDENCY(gem::base);
  CHECKDEPENDENCY(gem::utils);
  CHECKDEPENDENCY(gem::hw::utils);
}

std::set<std::string, std::less<std::string> > gem::hw::managers::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > deps;
  ADDDEPENDENCY(deps, toolbox);
  ADDDEPENDENCY(deps, xoap);
  ADDDEPENDENCY(deps, xdaq);
  ADDDEPENDENCY(deps, gem::base);
  ADDDEPENDENCY(deps, gem::utils);
  ADDDEPENDENCY(deps, gem::hw::utils);
  return deps;
}
