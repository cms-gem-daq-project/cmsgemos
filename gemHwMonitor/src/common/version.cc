#include "toolbox/version.h"
#include "xdaq/version.h"
#include "xoap/version.h"
#include "gem/base/version.h"
#include "gem/utils/version.h"
#include "gem/hw/devices/version.h"
#include "gem/hw/utils/version.h"
#include "gem/hwMonitor/version.h"

GETPACKAGEINFO(gem::hwMonitor);

void gem::hwMonitor::checkPackageDependencies()
{
  CHECKDEPENDENCY(toolbox);
  CHECKDEPENDENCY(xdaq);
  CHECKDEPENDENCY(xoap);
  CHECKDEPENDENCY(gem::base);
  CHECKDEPENDENCY(gem::utils);
  CHECKDEPENDENCY(gem::hw::devices);
  CHECKDEPENDENCY(gem::hw::utils);
}

std::set<std::string, std::less<std::string> > gem::hwMonitor::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > deps;
  ADDDEPENDENCY(deps, toolbox);
  ADDDEPENDENCY(deps, xoap);
  ADDDEPENDENCY(deps, xdaq);
  ADDDEPENDENCY(deps, gem::base);
  ADDDEPENDENCY(deps, gem::utils);
  ADDDEPENDENCY(deps, gem::hw::devices);
  ADDDEPENDENCY(deps, gem::hw::utils);
  return deps;
}
