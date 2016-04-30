#include "toolbox/version.h"
#include "xcept/version.h"
#include "xdaq/version.h"
#include "xoap/version.h"
#include "gem/base/version.h"
#include "gem/utils/version.h"

GETPACKAGEINFO(gembase);

void gembase::checkPackageDependencies()
  throw (config::PackageInfo::VersionException)
{
  CHECKDEPENDENCY(toolbox);
  CHECKDEPENDENCY(xcept);
  CHECKDEPENDENCY(xdaq);
  CHECKDEPENDENCY(xoap);
  CHECKDEPENDENCY(gemutils);
}

std::set<std::string, std::less<std::string> > gembase::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > deps;
  ADDDEPENDENCY(deps,toolbox);
  ADDDEPENDENCY(deps,xcept);
  ADDDEPENDENCY(deps,xdaq);
  ADDDEPENDENCY(deps,xoap);
  ADDDEPENDENCY(deps,gemutils);
  return deps;
}
