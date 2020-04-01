#include "toolbox/version.h"
#include "xdaq/version.h"
#include "xoap/version.h"
#include "gem/base/version.h"
#include "gem/utils/version.h"
#include "gem/calib/version.h"

GETPACKAGEINFO(gemcalibration);


void gemcalibration::checkPackageDependencies()
{
  CHECKDEPENDENCY(toolbox);
  CHECKDEPENDENCY(xdaq);
  CHECKDEPENDENCY(xoap);
  CHECKDEPENDENCY(gem::base);
  CHECKDEPENDENCY(gem::utils);
 
}

std::set<std::string, std::less<std::string> > gemcalibration::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > deps;
  ADDDEPENDENCY(deps, toolbox);
  ADDDEPENDENCY(deps, xoap);
  ADDDEPENDENCY(deps, xdaq);
  ADDDEPENDENCY(deps, gem::base);
  ADDDEPENDENCY(deps, gem::utils);
  return deps;
}
