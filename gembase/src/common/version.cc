#include "config/version.h"
#include "xcept/version.h"
#include "xdaq/version.h"
#include "gem/base/version.h"

GETPACKAGEINFO(gem::base)

void gem::base::checkPackageDependencies() throw (config::PackageInfo::VersionException)
{
  CHECKDEPENDENCY(config);
  CHECKDEPENDENCY(xcept);
  CHECKDEPENDENCY(xdaq);
}

std::set<std::string, std::less<std::string> > gem::base::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > dependencies;
  ADDDEPENDENCY(dependencies,config);
  ADDDEPENDENCY(dependencies,xcept);
  ADDDEPENDENCY(dependencies,xdaq);
  return dependencies;
}
