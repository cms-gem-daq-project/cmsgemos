#include "gem/hwMonitor/gemGLIBMonitorBase.h"

gem::hwMonitor::gemGLIBMonitorBase::gemGLIBMonitorBase(gem::base::utils::gemGLIBProperties& glibRef)
  throw (xgi::exception::Exception)
{
  m_glibRef = glibRef;
}

gem::hwMonitor::gemGLIBMonitorBase::~gemGLIBMonitorBase()
  throw (xgi::exception::Exception)
{
}

void gem::hwMonitor::gemGLIBMonitorBase::setGLIBRef()
  throw (xgi::exception::Exception)
{
  glibRefs_ = ;
}

gem::hwMonitor::gemGLIBMonitorBase::getGLIBRef()
throw (xgi::exception::Exception)
{
}
