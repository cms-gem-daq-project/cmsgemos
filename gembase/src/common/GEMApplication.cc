/**
 * class: GEMApplication
 * description: Generic GEM application, all GEM applications should inherit
 * from this class and define and extend as necessary
 * structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date: 
 */

// GEMApplication.cc

#include "gem/base/GEMWebApplication.h"
#include "gem/base/GEMApplication.h"
#include "gem/base/GEMMonitor.h"

gem::base::GEMApplication::GEMApplication(xdaq::ApplicationStub *stub)
  throw (xdaq::exception::Exception) :
  xdaq::WebApplication(stub),
  m_gemLogger(this->getApplicationLogger()),
  p_gemWebInterface(NULL),
  p_gemMonitor(     NULL),
  m_runNumber(-1),
  m_runType("")

{
  INFO("called gem::base::GEMApplication constructor");
  
  p_gemWebInterface = new GEMWebApplication(this);

  try {
    p_appInfoSpace  = getApplicationInfoSpace();
    p_appDescriptor = getApplicationDescriptor();
    p_appContext    = getApplicationContext();
    p_appZone       = p_appContext->getDefaultZone();
    p_appGroup      = p_appZone->getApplicationGroup("default");
    m_xmlClass      = p_appDescriptor->getClassName();
    m_instance      = p_appDescriptor->getInstance();
    m_urn           = p_appDescriptor->getURN();
  } catch(xcept::Exception e) {
    XCEPT_RETHROW(xdaq::exception::Exception, "Failed to get GEM application information", e);
  }
  
  xgi::framework::deferredbind(this, this, &GEMApplication::xgiDefault, "Default"    );
  xgi::framework::deferredbind(this, this, &GEMApplication::xgiMonitor, "monitorView");
  xgi::framework::deferredbind(this, this, &GEMApplication::xgiExpert,  "expertView" );

  p_appInfoSpace->addListener(this, "urn:xdaq-event:setDefaultValues");
  //what other listeners are available through this interface?
  p_appInfoSpace->addListener(this, "urn:xdata-event:ItemGroupRetrieveEvent");
  p_appInfoSpace->addListener(this, "urn:xdata-event:ItemGroupChangedEvent");
  p_appInfoSpace->addListener(this, "urn:xdata-event:ItemRetrieveEvent");
  p_appInfoSpace->addListener(this, "urn:xdata-event:ItemChangedEvent");

  //how to have infospaces inside infospaces, or listeners on multiple infospaces
  //p_configInfoSpace->addListener( this, "urn:xdaq-event:setDefaultValues");
  //p_monitorInfoSpace->addListener(this, "urn:xdaq-event:setDefaultValues");
  p_appInfoSpace->fireItemAvailable("configuration:parameters", p_configInfoSpace );
  p_appInfoSpace->fireItemAvailable("monitoring:parameters",    p_monitorInfoSpace);
  //p_appInfoSpace->fireItemAvailable("reasonForFailure", &reasonForFailure_);
  
  p_appInfoSpace->fireItemAvailable("RunNumber",&m_runNumber);
  p_appInfoSpace->fireItemAvailable("RunType",  &m_runType  );
  p_appInfoSpace->fireItemAvailable("CfgType",  &m_cfgType  );

  //is this the correct syntax?
  p_appInfoSpace->addItemRetrieveListener("RunNumber", this);
  p_appInfoSpace->addItemRetrieveListener("RunType",   this);
  p_appInfoSpace->addItemRetrieveListener("CfgType",   this);
  p_appInfoSpace->addItemChangedListener( "RunNumber", this);
  p_appInfoSpace->addItemChangedListener( "RunType",   this);
  p_appInfoSpace->addItemChangedListener( "CfgType",   this);


  INFO("gem::base::GEMApplication constructed");
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

// This is the callback used for handling xdata:Event objects
void gem::base::GEMApplication::actionPerformed(xdata::Event& event)
{
  // This is called after all default configuration values have been
  // loaded (from the XDAQ configuration file).  This should be implemented in all derived classes
  // followed by a call to gem::base::GEMApplication::actionPerformed(event)
  /*
    if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("GEMApplication::actionPerformed() setDefaultValues" << 
    "Default configuration values have been loaded from xml profile");
    //DEBUG("GEMApplication::actionPerformed()   --> starting monitoring");
    //monitorP_->startMonitoring();
    }
  */
  // update monitoring variables
  if (event.type() == "ItemRetrieveEvent" ||
      event.type() == "urn:xdata-event:ItemRetrieveEvent") {
    DEBUG("GEMApplication::actionPerformed() ItemRetrieveEvent"
          << "");
  } else if (event.type() == "ItemGroupRetrieveEvent" || 
             event.type() == "urn:xdata-event:ItemGroupRetrieveEvent") {
    DEBUG("GEMApplication::actionPerformed() ItemGroupRetrieveEvent"
          << "");
  }
  //item is changed, update it
  if (event.type()=="ItemChangedEvent" ||
      event.type()=="urn:xdata-event:ItemChangedEvent") {
    DEBUG("GEMApplication::actionPerformed() ItemChangedEvent"
          << "");

    /* from HCAL runInfoServer
       std::list<std::string> names;
       names.push_back(str_RUNNUMBER);

       try {
       getMonitorInfospace()->fireItemGroupChanged(names, this);
       } catch (xcept::Exception& e) {
       ERROR(xcept::stdformat_exception_history(e));
       }
    */
  }
}

void gem::base::GEMApplication::importConfigurationParameters()
{
  //parse the xml configuration file or db configuration information
}


void gem::base::GEMApplication::fillConfigurationInfoSpace()
{
  //put the configuration parameters into the configuration infospace
}


void gem::base::GEMApplication::updateConfigurationInfoSpace()
{
  //update the configuration infospace object with new items
}


void gem::base::GEMApplication::importMonitoringParameters()
{
  //parse the xml monitoring file or db monitoring information
}


void gem::base::GEMApplication::fillMonitoringInfoSpace()
{
  //put the monitoring parameters into the monitoring infospace
}


void gem::base::GEMApplication::updateMonitoringInfoSpace()
{
  //update the monitoring infospace object with new items
}


void gem::base::GEMApplication::xgiDefault(xgi::Input* in, xgi::Output* out)
{
  p_gemWebInterface->webDefault(in,out);
}

void gem::base::GEMApplication::xgiMonitor(xgi::Input* in, xgi::Output* out)
{
  p_gemWebInterface->monitorPage(in,out);
}

void gem::base::GEMApplication::xgiExpert(xgi::Input* in, xgi::Output* out)
{
  p_gemWebInterface->expertPage(in,out);
}
// End of file
