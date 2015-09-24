#include "gem/supervisor/GEMTestBeamSupervisor.h"
#include "gem/supervisor/GEMTestBeamSupervisorWeb.h"
#include "gem/hw/vfat/VFAT2Manager.h"
#include "gem/supervisor/tbutils/LatencyScan.h"
#include "gem/supervisor/tbutils/ThresholdScan.h"
#include "gem/supervisor/tbutils/DAQWeb.h"
#include "xdaq/ApplicationRegistry.h"

gem::supervisor::GEMTestBeamSupervisor::GEMTestBeamSupervisor(xdaq::ApplicationStub* stub) :
  xdaq::Application(stub),
  webInterface_(0),
  m_gemLogger(Logger::getInstance("gem::supervisor::GEMTestBeamSupervisor")),
  isInitialized_(false),
  semaphore_(toolbox::BSem::FULL),
  wl_semaphore_(toolbox::BSem::EMPTY)
{
  LOG4CPLUS_DEBUG(getApplicationLogger(), "gem::supervisor::GEMTestBeamSupervisor being constructed");
  
  try {
    //i2oAddressMap_ = i2o::utils::getAddressMap();
    //poolFactory_   = toolbox::mem::getMemoryPoolFactory();
    appInfoSpace_  = getApplicationInfoSpace();
    appDescriptor_ = getApplicationDescriptor();
    appContext_    = getApplicationContext();
    appGroup_      = appContext_->getDefaultZone()->getApplicationGroup("default");
    xmlClass_      = appDescriptor_->getClassName();
    instance_      = appDescriptor_->getInstance();
    appURN_        = appDescriptor_->getURN();
  }
  catch(xcept::Exception e) {
    XCEPT_RETHROW(xdaq::exception::Exception, "Failed to get GEM TestBeam Supervisor application information", e);
  }
  
  xgi::bind(webInterface_, &gem::supervisor::GEMTestBeamSupervisorWeb::Default, "Default");
  
  appInfoSpace_->addListener(this, "urn:xdaq-event:setDefaultValues");
  
  appInfoSpace_->fireItemAvailable("isInitialized", &isInitialized_);
  appInfoSpace_->addItemChangedListener("isInitialized", this);
  
  semaphore_.take();
  gem::supervisor::GEMTestBeamSupervisor::initialize();
  semaphore_.give();
  
  //how to use this?
  std::list<xdaq::Application*> apps = appContext_->getApplicationRegistry()->getApplications();
  for (std::list<xdaq::Application*>::iterator i = apps.begin(); i != apps.end(); ++i) {
    xdaq::Application* app = *i;
    xdaq::ApplicationDescriptor* ad = app->getApplicationDescriptor();
    if (ad->getClassName() == "VFAT2Manager") {
      xdata::UnsignedShort* chipID  = dynamic_cast<xdata::UnsignedShort*>(app->getApplicationInfoSpace()->find("chipID"));
      xdata::UnsignedShort* gebSlot = dynamic_cast<xdata::UnsignedShort*>(app->getApplicationInfoSpace()->find("gebSlot"));
      managerApps_.push_back((gem::hw::vfat::VFAT2Manager)(app->getApplicationStub()));
      connectedChips_.push_back(new std::pair(static_cast<uint16_t>(chipID->getValue()),
                                              static_cast<uint8_t>(gebSlot->getValue())));
    }
    else if (ad->getClassName() == "GEMTestBeamSupervisorWeb") {
      webInterface_ = new GEMTestBeamSupervisorWeb(app->getApplicationStub());
    }
  }
  
  LOG4CPLUS_DEBUG(getApplicationLogger(), "loaded " << managerApps_.size() << " VFAT2Manager applications into the supervisor");

  LOG4CPLUS_DEBUG(getApplicationLogger(), "gem::supervisor::GEMTestBeamSupervisor constructed");
}

gem::supervisor::GEMTestBeamSupervisor::~GEMTestBeamSupervisor() 
{
  LOG4CPLUS_DEBUG(getApplicationLogger(), "gem::supervisor::GEMTestBeamSupervisor being destructed");
  semaphore_.take();
  if (webInterface_ != 0)
    delete webInterface_;
  webInterface_ = 0;
  
  /*
    if (gemTBfsmP_ != 0)
    delete gemTBfsmP_;
  */
  gemTBfsmP_ = 0;
  
  /*
    if (poolFactory_ != 0)
    delete poolFactory_;
  */
  poolFactory_ = 0;

  /*
    if (i2oAddressMap_ != 0)
    delete i2oAddressMap_;
  */
  i2oAddressMap_ = 0;

  if (appInfoSpace_ != 0)
    delete appInfoSpace_;
  appInfoSpace_ = 0;

  semaphore_.give();
  
  LOG4CPLUS_DEBUG(getApplicationLogger(), "gem::supervisor::GEMTestBeamSupervisor destructed");
}


void gem::supervisor::GEMTestBeamSupervisor::actionPerformed (xdata::Event& event)
{
  if ( event.type() == "urn:xdaq-event:setDefaultValues" )
    {
      std::stringstream ss;
      LOG4CPLUS_DEBUG(this->getApplicationLogger(), ss.str());
    }
}


void gem::supervisor::GEMTestBeamSupervisor::initialize() 
{
  //check GLIB firmware with version supplied in xml?
  
  //check OptoHybrid firmware with version supplied in xml?
  
  //find connected VFAT chips (based on those listed in xml?), and connect to their VFAT2Manager apps
  
  xdata::Serializable* ser = appInfoSpace_->find("isInitialized");
  xdata::Boolean* ini = dynamic_cast<xdata::Boolean*>(ser);
  *ini = true;
    
  //isInitialized_ = true;
  appInfoSpace_->fireItemChanged("isInitialized",0);
  return;
}



