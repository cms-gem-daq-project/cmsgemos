#include "toolbox/version.h"
#include "xdaq/version.h"
#include "xoap/version.h"
#include "gem/base/version.h"
#include "gem/utils/version.h"
#include "gem/hw/utils/version.h"
#include "gem/hw/managers/version.h"

GETPACKAGEINFO(gemhwmanagers);

void gemhwmanagers::checkPackageDependencies()
  throw (config::PackageInfo::VersionException)
{
  CHECKDEPENDENCY(toolbox);
  CHECKDEPENDENCY(xdaq);
  CHECKDEPENDENCY(xoap);
  CHECKDEPENDENCY(gembase);
  CHECKDEPENDENCY(gemutils);
  CHECKDEPENDENCY(gemhwutils);
}

std::set<std::string, std::less<std::string> > gemhwmanagers::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > deps;
  ADDDEPENDENCY(deps, toolbox);
  ADDDEPENDENCY(deps, xoap);
  ADDDEPENDENCY(deps, xdaq);
  ADDDEPENDENCY(deps, gembase);
  ADDDEPENDENCY(deps, gemutils);
  ADDDEPENDENCY(deps, gemhwutils);
  return deps;
}
