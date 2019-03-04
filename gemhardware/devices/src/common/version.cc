#include "toolbox/version.h"
#include "xdaq/version.h"
#include "gem/utils/version.h"
#include "gem/hw/utils/version.h"
#include "gem/hw/devices/version.h"

GETPACKAGEINFO(gemhwdevices);

void gemhwdevices::checkPackageDependencies()
  throw (config::PackageInfo::VersionException)
{
  CHECKDEPENDENCY(toolbox);
  CHECKDEPENDENCY(xdaq);
  CHECKDEPENDENCY(gemutils);
  CHECKDEPENDENCY(gemhwutils);
}

std::set<std::string, std::less<std::string> > gemhwdevices::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > deps;
  ADDDEPENDENCY(deps, toolbox);
  ADDDEPENDENCY(deps, xdaq);
  ADDDEPENDENCY(deps, gemutils);
  ADDDEPENDENCY(deps, gemhwutils);
  return deps;
}
