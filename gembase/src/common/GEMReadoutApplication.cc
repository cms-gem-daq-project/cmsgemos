/**
 * class: GEMReadoutApplication
 * description: Generic GEM application to handle readout
 *              structure borrowed from HCAL
 * author: J. Sturdy
 * date: 
 */

#include "gem/base/GEMReadoutApplication.h"
#include "gem/base/GEMWebApplication.h"

//XDAQ_INSTANTIATOR_IMPL(gem::supervisor::GEMSupervisor);

const int gem::base::GEMReadoutApplication::I2O_READOUT_NOTIFY=0x84;
const int gem::base::GEMReadoutApplication::I2O_READOUT_CONFIRM=0x85;

gem::base::GEMReadoutApplication::GEMReadoutSettings::GEMReadoutSettings() {
  runType        = "";
  fileName       = "";
  outputType     = "Bin";
  outputLocation = "";
}

void gem::base::GEMReadoutApplication::GEMReadoutSettings::registerFields(xdata::Bag<gem::base::GEMReadoutApplication::GEMReadoutSettings>* bag) {
  bag->addField("runType",        &runType);
  bag->addField("fileName",       &fileName);
  bag->addField("outputType",     &outputType);
  bag->addField("outputLocation", &outputLocation);

}


gem::base::GEMReadoutApplication::GEMReadoutApplication(xdaq::ApplicationStub* stub) :
  GEMFSMApplication(stub)
{
  //i2o::bind(this,&ReadoutApplication::onReadoutNotify,I2O_READOUT_NOTIFY,XDAQ_ORGANIZATION_ID);
  //xoap::bind(this,&ReadoutApplication::getReadoutCredits,"GetReadoutCredits","urn:GEMReadoutApplication-soap:1");
  p_appInfoSpace->fireItemAvailable("ReadoutSettings",&m_readoutSettings);
  p_appInfoSpace->fireItemAvailable("DeviceName",     &m_deviceName);
  p_appInfoSpace->fireItemAvailable("ConnectionFile", &m_connectionFile);

  p_appInfoSpace->addItemRetrieveListener("ReadoutSettings", this);
  p_appInfoSpace->addItemRetrieveListener("DeviceName",      this);
  p_appInfoSpace->addItemRetrieveListener("ConnectionFile",  this);
  p_appInfoSpace->addItemChangedListener( "ReadoutSettings", this);
  p_appInfoSpace->addItemChangedListener( "DeviceName",      this);
  p_appInfoSpace->addItemChangedListener( "ConnectionFile",  this);

  ////initialize the GLIB application objects
  //DEBUG("Connecting to the GLIBReadoutWeb interface");
  //p_gemWebInterface = new gem::hw::glib::GLIBManagerWeb(this);
  ////p_gemMonitor      = new gem::hw::glib::GLIBHwMonitor(this);
  //DEBUG("done");
  //
  ////set up the info hwCfgInfoSpace 
  //init();
}

gem::base::GEMReadoutApplication::~GEMReadoutApplication()
{
  
}

void gem::base::GEMReadoutApplication::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("GEMReadoutApplication::actionPerformed() setDefaultValues" << 
          "Default configuration values have been loaded from xml profile");
    importConfigurationParameters();
    importMonitoringParameters();
    //p_gemMonitor->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
  
}


void gem::base::GEMReadoutApplication::initializeAction()
  /*throw (gem::base::exception::Exception)*/
{
  DEBUG("gem::base::GEMReadoutApplication::initializeAction begin");
}

void gem::base::GEMReadoutApplication::configureAction()
  /*throw (gem::base::exception::Exception)*/
{
  DEBUG("gem::base::GEMReadoutApplication::configureAction begin");
}

void gem::base::GEMReadoutApplication::startAction()
  /*throw (gem::base::exception::Exception)*/
{
  DEBUG("gem::base::GEMReadoutApplication::startAction begin");
}

void gem::base::GEMReadoutApplication::pauseAction()
  /*throw (gem::base::exception::Exception)*/
{
  DEBUG("gem::base::GEMReadoutApplication::pauseAction begin");
}

void gem::base::GEMReadoutApplication::resumeAction()
  /*throw (gem::base::exception::Exception)*/
{
  DEBUG("gem::base::GEMReadoutApplication::resumeAction begin");
}

void gem::base::GEMReadoutApplication::stopAction()
  /*throw (gem::base::exception::Exception)*/
{
  DEBUG("gem::base::GEMReadoutApplication::stopAction begin");
}

void gem::base::GEMReadoutApplication::haltAction()
  /*throw (gem::base::exception::Exception)*/
{
  DEBUG("gem::base::GEMReadoutApplication::haltAction begin");
}

void gem::base::GEMReadoutApplication::resetAction()
  /*throw (gem::base::exception::Exception)*/
{
  DEBUG("gem::base::GEMReadoutApplication::resetAction begin");
}

void gem::base::GEMReadoutApplication::failAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  // close open file pointers
}

void gem::base::GEMReadoutApplication::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  // close open file pointers
}

