#include "toolbox/version.h"
#include "xdaq/version.h"
#include "xoap/version.h"
#include "gem/base/version.h"
#include "gem/utils/version.h"
#include "gem/readout/version.h"

GETPACKAGEINFO(gem::readout);

void gem::readout::checkPackageDependencies()
  throw (config::PackageInfo::VersionException)
{
  CHECKDEPENDENCY(toolbox);
  CHECKDEPENDENCY(xdaq);
  CHECKDEPENDENCY(xoap);
  CHECKDEPENDENCY(gem::base);
  CHECKDEPENDENCY(gem::utils);
}

std::set<std::string, std::less<std::string> > gem::readout::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > deps;
  ADDDEPENDENCY(deps, toolbox);
  ADDDEPENDENCY(deps, xoap);
  ADDDEPENDENCY(deps, xdaq);
  ADDDEPENDENCY(deps, gem::base);
  ADDDEPENDENCY(deps, gem::utils);
  return deps;
}
