// GEMApplication.cc

#include "gem/base/GEMWebApplication.h"
#include "gem/base/GEMMonitor.h"
#include "gem/base/GEMApplication.h"
#include "gem/base/utils/GEMLogging.h"

#include "toolbox/string.h"

#include "xgi/Input.h"
#include "xgi/Method.h"
#include "xgi/Output.h"
#include "xoap/Method.h"

#include "xcept/Exception.h"
#include "xcept/tools.h"

#include "xdaq/NamespaceURI.h"
#include "xdaq/Application.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationDescriptorImpl.h"
#include "xdaq/exception/Exception.h"

#include "xdaq/XceptSerializer.h"
#include "xdata/Float.h" 
#include "xdata/Double.h" 
#include "xdata/Boolean.h"

gem::base::GEMApplication::GEMApplication(xdaq::ApplicationStub *stub)
  throw (xdaq::exception::Exception) :
  xdaq::WebApplication(stub),
  gemLogger_(this->getApplicationLogger()),
  gemWebInterfaceP_(0),
  gemMonitorP_(0)
{
  DEBUG("called gem::base::GEMApplication constructor");

  try {
    appInfoSpaceP_  = getApplicationInfoSpace();
    appDescriptorP_ = getApplicationDescriptor();
    appContextP_    = getApplicationContext();
    appZoneP_       = appContextP_->getDefaultZone();
    appGroupP_      = appZoneP_->getApplicationGroup("default");
    xmlClass_      = appDescriptorP_->getClassName();
    instance_      = appDescriptorP_->getInstance();
    urn_           = appDescriptorP_->getURN();
  }
  catch(xcept::Exception e) {
    XCEPT_RETHROW(xdaq::exception::Exception, "Failed to get GEM application information", e);
  }
  
  xgi::framework::deferredbind(this, this, &GEMApplication::monitorView, "Default"    );
  xgi::framework::deferredbind(this, this, &GEMApplication::monitorView, "monitorView");
  xgi::framework::deferredbind(this, this, &GEMApplication::expertView,  "expertView" );

  getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");
  //getApplicationInfoSpace()->fireItemAvailable("reasonForFailure", &reasonForFailure_);
  
  DEBUG("gem::base::GEMApplication constructed");
}


gem::base::GEMApplication::~GEMApplication() 
{
  INFO("gem::base::GEMApplication destructor called");
  
}

std::string gem::base::GEMApplication::getFullURL()
{
  std::string url = getApplicationDescriptor()->getContextDescriptor()->getURL();
  std::string urn = getApplicationDescriptor()->getURN();
  std::string fullURL = toolbox::toString("%s/%s", url.c_str(), urn.c_str());
  return fullURL;
}

// This is the callback used for setting parameters.  
void gem::base::GEMApplication::actionPerformed(xdata::Event& event)
{
  // This is called after all default configuration values have been
  // loaded (from the XDAQ configuration file).
  if (event.type() == "urn:xdaq-event:setDefaultValues")
    {
      LOG4CPLUS_DEBUG(getApplicationLogger(), "GEMApplication::actionPerformed() Default configuration values have been loaded");
      //LOG4CPLUS_DEBUG(getApplicationLogger(), "GEMApplication::actionPerformed()   --> starting monitoring");
      //monitorP_->startMonitoring();
    }
}
// End of file
