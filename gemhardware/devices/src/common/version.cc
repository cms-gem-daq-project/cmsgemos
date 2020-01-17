#include "toolbox/version.h"
#include "xdaq/version.h"
#include "gem/utils/version.h"
#include "gem/hw/utils/version.h"
#include "gem/hw/devices/version.h"

GETPACKAGEINFO(gem::hw::devices);

void gem::hw::devices::checkPackageDependencies()
{
  CHECKDEPENDENCY(toolbox);
  CHECKDEPENDENCY(xdaq);
  CHECKDEPENDENCY(gem::utils);
  CHECKDEPENDENCY(gem::hw::utils);
}

std::set<std::string, std::less<std::string> > gem::hw::devices::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > deps;
  ADDDEPENDENCY(deps, toolbox);
  ADDDEPENDENCY(deps, xdaq);
  ADDDEPENDENCY(deps, gem::utils);
  ADDDEPENDENCY(deps, gem::hw::utils);
  return deps;
}
