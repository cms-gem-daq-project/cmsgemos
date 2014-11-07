// GEMApplication.cc

#include "gem/base/GEMApplication.h"
#include "gem/base/utils/GEMLogging.h"

#include "xdaq/NamespaceURI.h"  // XDAQ_NS_URI
#include "xoap/Method.h"
#include "xoap/MessageFactory.h"  // createMessage()
#include "xoap/SOAPPart.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/SOAPSerializer.h"
#include "xoap/domutils.h"  // XMLCh2String()

#include "toolbox/fsm/FailedEvent.h"
#include "toolbox/task/WorkLoopFactory.h" // getWorkLoopFactory()
#include "xcept/tools.h"

#include "xoap/DOMParser.h"
#include "xoap/DOMParserFactory.h"
#include "xoap/domutils.h"
#include "xdata/soap/Serializer.h"

#include "xcept/tools.h"
#include "xdaq2rc/RcmsStateNotifier.h"
#include "toolbox/fsm/FailedEvent.h"

#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationDescriptorImpl.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/exception/Exception.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/XceptSerializer.h"
#include "xdata/Float.h" 
#include "xdata/Double.h" 
#include "xdata/Boolean.h"

//XDAQ_INSTANTIATOR_IMPL(gem::base::GEMApplication)

gem::base::GEMApplication::GEMApplication(xdaq::ApplicationStub *stub)
  throw (xdaq::exception::Exception) :
  xdaq::Application(stub),
  gemLogger_(Logger::getInstance("gem::base::GEMApplication")),
  run_type_("Monitor"), run_number_(1), runSequenceNumber_(0),
  nevents_(-1),
  rcmsStateNotifier_(getApplicationLogger(), getApplicationDescriptor(), getApplicationContext()),
  wl_semaphore_(toolbox::BSem::EMPTY)
{

  try {
    i2oAddressMap_ = i2o::utils::getAddressMap();
    poolFactory_   = toolbox::mem::getMemoryPoolFactory();
    appInfoSpace_  = getApplicationInfoSpace();
    appDescriptor_ = getApplicationDescriptor();
    appContext_    = getApplicationContext();
    appGroup_      = appContext_->getDefaultZone()->getApplicationGroup("default");
    xmlClass_      = appDescriptor_->getClassName();
    instance_      = appDescriptor_->getInstance();
    urn_           = appDescriptor_->getURN();
    }
  catch(xcept::Exception e) {
    XCEPT_RETHROW(xdaq::exception::Exception, "Failed to get GEM application information", e);
  }

  //getApplicationInfoSpace()->fireItemAvailable("reasonForFailure", &reasonForFailure_);
  
  //Get the RCMS state listener
  rcmsStateNotifier_.findRcmsStateListener();
  getApplicationInfoSpace()->fireItemAvailable("rcmsStateListener",      rcmsStateNotifier_.getRcmsStateListenerParameter());
  getApplicationInfoSpace()->fireItemAvailable("foundRcmsStateListener", rcmsStateNotifier_.getFoundRcmsStateListenerParameter());
    
  LOG4CPLUS_INFO(getApplicationLogger(), "gem::base::GEMApplication constructed");
}


gem::base::GEMApplication::~GEMApplication() 
{
  
}

std::string gem::base::GEMApplication::getFullURL()
{
  std::string url = getApplicationDescriptor()->getContextDescriptor()->getURL();
  std::string urn = getApplicationDescriptor()->getURN();
  std::string fullURL = toolbox::toString("%s/%s", url.c_str(), urn.c_str());
  return fullURL;
}

// This is the callback used for setting parameters.  
/******
 **no need to add this for the base GEMApplication class**
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
******/
// End of file
