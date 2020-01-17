#include "toolbox/version.h"
#include "xdaq/version.h"
#include "xoap/version.h"
#include "gem/base/version.h"
#include "gem/utils/version.h"
#include "gem/readout/version.h"
#include "gem/hw/devices/version.h"
#include "gem/hw/managers/version.h"
#include "gem/supervisor/version.h"

GETPACKAGEINFO(gem::supervisor);

void gem::supervisor::checkPackageDependencies()
{
  CHECKDEPENDENCY(toolbox);
  CHECKDEPENDENCY(xdaq);
  CHECKDEPENDENCY(xoap);
  CHECKDEPENDENCY(gem::base);
  CHECKDEPENDENCY(gem::utils);
  CHECKDEPENDENCY(gem::readout);
  CHECKDEPENDENCY(gem::hw::devices);
  CHECKDEPENDENCY(gem::hw::managers);
}

std::set<std::string, std::less<std::string> > gem::supervisor::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > deps;
  ADDDEPENDENCY(deps, toolbox);
  ADDDEPENDENCY(deps, xoap);
  ADDDEPENDENCY(deps, xdaq);
  ADDDEPENDENCY(deps, gem::base);
  ADDDEPENDENCY(deps, gem::utils);
  ADDDEPENDENCY(deps, gem::readout);
  ADDDEPENDENCY(deps, gem::hw::devices);
  ADDDEPENDENCY(deps, gem::hw::managers);
  return deps;
}
