#include "toolbox/version.h"
#include "xcept/version.h"
#include "xdaq/version.h"
#include "xoap/version.h"
#include "gem/base/version.h"
#include "gem/utils/version.h"

GETPACKAGEINFO(gem::base);

void gem::base::checkPackageDependencies()
{
  CHECKDEPENDENCY(toolbox);
  CHECKDEPENDENCY(xcept);
  CHECKDEPENDENCY(xdaq);
  CHECKDEPENDENCY(xoap);
  CHECKDEPENDENCY(gem::utils);
}

std::set<std::string, std::less<std::string> > gem::base::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > deps;
  ADDDEPENDENCY(deps,toolbox);
  ADDDEPENDENCY(deps,xcept);
  ADDDEPENDENCY(deps,xdaq);
  ADDDEPENDENCY(deps,xoap);
  ADDDEPENDENCY(deps,gem::utils);
  return deps;
}
