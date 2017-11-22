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

gem::base::GEMApplication::ScanInfo::ScanInfo()
{
  scanType  = 0;
  scanMin   = 0;
  scanMax   = 0;
  stepSize  = 0;
  nTriggers = 0;

}

void gem::base::GEMApplication::ScanInfo::registerFields(xdata::Bag<gem::base::GEMApplication::ScanInfo>* bag)
{
  bag->addField("ScanType",  &scanType );
  bag->addField("ScanMin",   &scanMin );
  bag->addField("ScanMax",   &scanMax );
  bag->addField("StepSize",  &stepSize );
  bag->addField("NTriggers", &nTriggers);
}

gem::base::GEMApplication::GEMApplication(xdaq::ApplicationStub *stub)
  throw (xdaq::exception::Exception) :
  xdaq::WebApplication(stub),
  m_gemLogger(this->getApplicationLogger()),
  p_gemWebInterface(NULL),
  p_gemMonitor(     NULL),
  p_appDescriptor(  NULL),
  p_appContext(     NULL),
  p_appGroup(       NULL),
  p_appZone(        NULL),
  m_xmlClass(       "N/A"),
  m_urn(            "N/A"),
  m_instance( 0),
  m_runNumber(0),
  m_runType("N/A"),

  m_scanType(0),
  m_scanMin(0),
  m_scanMax(0),
  m_stepSize(0),
  m_nScanTriggers(0)
{
  DEBUG("GEMApplication::called gem::base::GEMApplication constructor");
  INFO("GEMApplication GIT_VERSION:" << GIT_VERSION);
  INFO("GEMApplication developer:"   << GEMDEVELOPER);

  //p_gemWebInterface = new GEMWebApplication(this);

  try {
    p_appInfoSpace  = getApplicationInfoSpace();
    p_appDescriptor = const_cast<xdaq::ApplicationDescriptor*>(getApplicationDescriptor());  // temporary for slc6/cc7 compatibility
    p_appContext    = getApplicationContext();
    p_appZone       = const_cast<xdaq::Zone*>(p_appContext->getDefaultZone());  // temporary for slc6/cc7 compatibility
    p_appGroup      = const_cast<xdaq::ApplicationGroup*>(p_appZone->getApplicationGroup("default"));  // temporary for slc6/cc7 compatibility
    m_xmlClass      = p_appDescriptor->getClassName();
    m_instance      = p_appDescriptor->getInstance();
    m_urn           = p_appDescriptor->getURN();
  } catch(xcept::Exception e) {
    XCEPT_RETHROW(xdaq::exception::Exception, "Failed to get GEM application information", e);
  }

  p_appInfoSpaceToolBox = std::shared_ptr<utils::GEMInfoSpaceToolBox>(new utils::GEMInfoSpaceToolBox(this,
                                                                                                     p_appInfoSpace,
                                                                                                     // p_gemMonitor,
                                                                                                     false));
  DEBUG("GEMApplication::application infospace has name: " << p_appInfoSpace->name());
  DEBUG(m_urn);
  toolbox::net::URN monISURN(m_urn+toolbox::toString(":monitoring-infospace"));
  if (xdata::getInfoSpaceFactory()->hasItem(monISURN.toString())) {
    DEBUG("GEMApplication::GEMApplication::infospace " << monISURN.toString() << " already exists, getting");
    p_monitorInfoSpace = xdata::getInfoSpaceFactory()->get(monISURN.toString());
  } else {
    DEBUG("GEMApplication::GEMApplication::infospace " << monISURN.toString() << " does not exist, creating");
    p_monitorInfoSpace = xdata::getInfoSpaceFactory()->create(monISURN.toString());
  }
  p_monitorInfoSpaceToolBox = std::shared_ptr<utils::GEMInfoSpaceToolBox>(new utils::GEMInfoSpaceToolBox(this,
                                                                                                         p_monitorInfoSpace,
                                                                                                         // p_gemMonitor,
                                                                                                         false));
  toolbox::net::URN cfgISURN(m_urn+toolbox::toString(":config-infospace"));
  if (xdata::getInfoSpaceFactory()->hasItem(cfgISURN.toString())) {
    DEBUG("GEMApplication::GEMApplication::infospace " << cfgISURN.toString() << " already exists, getting");
    p_configInfoSpace = xdata::getInfoSpaceFactory()->get(cfgISURN.toString());
  } else {
    DEBUG("GEMApplication::GEMApplication::infospace " << cfgISURN.toString() << " does not exist, creating");
    p_configInfoSpace = xdata::getInfoSpaceFactory()->create(cfgISURN.toString());
  }
  p_configInfoSpaceToolBox = std::shared_ptr<utils::GEMInfoSpaceToolBox>(new utils::GEMInfoSpaceToolBox(this,
                                                                                                        p_configInfoSpace,
                                                                                                        // p_gemMonitor,
                                                                                                        false));

  DEBUG("GEMApplication::GEM application has infospace named " << p_appInfoSpace->name());
  xgi::framework::deferredbind(this, this, &GEMApplication::xgiDefault, "Default"    );
  xgi::framework::deferredbind(this, this, &GEMApplication::xgiMonitor, "monitorView");
  xgi::framework::deferredbind(this, this, &GEMApplication::xgiExpert,  "expertView" );
  // only used for passing data, does not need to bind to the in-framework model
  xgi::bind(this, &GEMApplication::jsonUpdate, "jsonUpdate" );

  p_appInfoSpace->addListener(this, "urn:xdaq-event:setDefaultValues");
  p_appInfoSpace->addListener(this, "urn:xdata-event:ItemGroupRetrieveEvent");
  p_appInfoSpace->addListener(this, "urn:xdata-event:ItemGroupChangedEvent");
  p_appInfoSpace->addListener(this, "urn:xdata-event:ItemRetrieveEvent");
  p_appInfoSpace->addListener(this, "urn:xdata-event:ItemChangedEvent");

  p_appInfoSpace->fireItemAvailable("configuration:parameters", p_configInfoSpace );
  p_appInfoSpace->fireItemAvailable("monitoring:parameters",    p_monitorInfoSpace);

  // all should come from initialize
  p_appInfoSpaceToolBox->createInteger64("RunNumber", m_runNumber.value_,   &m_runNumber, utils::GEMInfoSpaceToolBox::PROCESS);
  p_appInfoSpaceToolBox->createString(   "RunType",   m_runType.toString(), &m_runType,   utils::GEMInfoSpaceToolBox::PROCESS);
  p_appInfoSpaceToolBox->createString(   "CfgType",   m_cfgType.toString(), &m_cfgType,   utils::GEMInfoSpaceToolBox::PROCESS);
  // p_appInfoSpaceToolBox->createString("reasonForFailure", &reasonForFailure_,utils::GEMInfoSpaceToolBox::PROCESS);

  p_appInfoSpace->fireItemAvailable("ScanInfo",    &m_scanInfo);
  p_appInfoSpaceToolBox->createUInt32("ScanType", m_scanType.value_, &m_scanType, utils::GEMInfoSpaceToolBox::PROCESS);
  p_appInfoSpaceToolBox->createUInt32("ScanMin",  m_scanMin.value_,  &m_scanMin,  utils::GEMInfoSpaceToolBox::PROCESS);
  p_appInfoSpaceToolBox->createUInt32("ScanMax",  m_scanMax.value_,  &m_scanMax,  utils::GEMInfoSpaceToolBox::PROCESS);
  p_appInfoSpaceToolBox->createUInt32("StepSize", m_stepSize.value_, &m_stepSize, utils::GEMInfoSpaceToolBox::PROCESS);
  p_appInfoSpaceToolBox->createUInt64("nScanTriggers", m_nScanTriggers.value_, &m_nScanTriggers, utils::GEMInfoSpaceToolBox::PROCESS);

  // is this the correct syntax? what does it really do?
  p_appInfoSpace->addItemRetrieveListener("RunNumber", this);
  p_appInfoSpace->addItemRetrieveListener("RunType",   this);
  p_appInfoSpace->addItemRetrieveListener("CfgType",   this);

  p_appInfoSpace->addItemRetrieveListener("ScanInfo",      this);
  p_appInfoSpace->addItemRetrieveListener("ScanType",      this);
  p_appInfoSpace->addItemRetrieveListener("nScanTriggers", this);
  p_appInfoSpace->addItemRetrieveListener("ScanMin",       this);
  p_appInfoSpace->addItemRetrieveListener("ScanMax",       this);
  p_appInfoSpace->addItemRetrieveListener("StepSize",      this);

  p_appInfoSpace->addItemChangedListener( "RunNumber", this);
  p_appInfoSpace->addItemChangedListener( "RunType",   this);
  p_appInfoSpace->addItemChangedListener( "CfgType",   this);

  p_appInfoSpace->addItemChangedListener( "ScanInfo",      this);
  p_appInfoSpace->addItemChangedListener( "ScanType",      this);
  p_appInfoSpace->addItemChangedListener( "nScanTriggers", this);
  p_appInfoSpace->addItemChangedListener( "ScanMin",       this);
  p_appInfoSpace->addItemChangedListener( "ScanMax",       this);
  p_appInfoSpace->addItemChangedListener( "StepSize",      this);

  DEBUG("GEMApplication::gem::base::GEMApplication constructed");
}


gem::base::GEMApplication::~GEMApplication()
{
  DEBUG("GEMApplication::gem::base::GEMApplication destructor called");
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
    // DEBUG("GEMApplication::actionPerformed()   --> starting monitoring");
    // monitorP_->startMonitoring();
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
  // item is changed, update it
  if (event.type() == "ItemChangedEvent" ||
      event.type() == "urn:xdata-event:ItemChangedEvent") {
    DEBUG("GEMApplication::actionPerformed() ItemChangedEvent"
          << "m_runNumber:" << m_runNumber
          << " getInteger64(\"RunNumber\"):" << p_appInfoSpaceToolBox->getInteger64("RunNumber"));

    /*
    INFO("GEMApplication::actionPerformed() ItemChangedEvent"
          << "m_RUNTYPE:" << m_RUNTYPE
          << " getInteger(\"ScanType\"):" << p_appInfoSpaceToolBox->getInteger("ScanType"));

    INFO("GEMApplication::actionPerformed() ItemChangedEvent"
          << "m_Min:" << m_Min
          << " getInteger(\"Min\"):" << p_appInfoSpaceToolBox->getInteger("Min"));

    INFO("GEMApplication::actionPerformed() ItemChangedEvent"
          << "m_Max:" << m_Max
          << " getInteger(\"Max\"):" << p_appInfoSpaceToolBox->getInteger("Max"));

    INFO("GEMApplication::actionPerformed() ItemChangedEvent"
          << "m_StepSize:" << m_StepSize
          << " getInteger(\"StepSize\"):" << p_appInfoSpaceToolBox->getInteger("StepSize"));
    */

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
  m_scanType      = m_scanInfo.bag.scanType;
  m_scanMin       = m_scanInfo.bag.scanMin;
  m_scanMax       = m_scanInfo.bag.scanMax;
  m_stepSize      = m_scanInfo.bag.stepSize;
  m_nScanTriggers = m_scanInfo.bag.nTriggers;
}

void gem::base::GEMApplication::importConfigurationParameters()
{
  // parse the xml configuration file or db configuration information
}


void gem::base::GEMApplication::fillConfigurationInfoSpace()
{
  // put the configuration parameters into the configuration infospace
}


void gem::base::GEMApplication::updateConfigurationInfoSpace()
{
  // update the configuration infospace object with new items
}


void gem::base::GEMApplication::importMonitoringParameters()
{
  // parse the xml monitoring file or db monitoring information
}


void gem::base::GEMApplication::fillMonitoringInfoSpace()
{
  // put the monitoring parameters into the monitoring infospace
}


void gem::base::GEMApplication::updateMonitoringInfoSpace()
{
  // update the monitoring infospace object with new items
}


void gem::base::GEMApplication::xgiDefault(xgi::Input* in, xgi::Output* out)
{
  p_gemWebInterface->webDefault(in, out);
}

void gem::base::GEMApplication::xgiMonitor(xgi::Input* in, xgi::Output* out)
{
  p_gemWebInterface->monitorPage(in, out);
}

void gem::base::GEMApplication::xgiExpert(xgi::Input* in, xgi::Output* out)
{
  p_gemWebInterface->expertPage(in, out);
}

void gem::base::GEMApplication::jsonUpdate(xgi::Input* in, xgi::Output* out)
{
  p_gemWebInterface->jsonUpdate(in, out);
}
