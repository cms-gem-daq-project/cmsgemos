/**
 * class: GEMReadoutApplication
 * description: Generic GEM application to handle readout
 *              structure borrowed from HCAL
 * author: J. Sturdy
 * date: 
 */

#include "gem/base/GEMReadoutApplication.h"
#include "gem/base/GEMWebApplication.h"

gem::base::GEMReadoutApplication::GEMReadoutSettings::GEMReadoutSettings() {
  runType        = "";
  fileName       = "";
  outputLocation = "";
}

void gem::base::GEMReadoutApplication::GEMReadoutSettings::registerFields(xdata::Bag<gem::base::GEMReadoutApplication::GEMReadoutSettings>* bag) {
  bag->addField("runType",        &runType);
  bag->addField("fileName",       &fileName);
  bag->addField("outputLocation", &outputLocation);

}


gem::base::GEMReadoutApplication::GEMReadoutApplication(xdaq::ApplicationStub* stub) :
  GEMFSMApplication(stub)
{
  i2o::bind(this,&ReadoutApplication::onReadoutNotify,I2O_READOUT_NOTIFY,XDAQ_ORGANIZATION_ID);
  xoap::bind(this,&ReadoutApplication::getReadoutCredits,"GetReadoutCredits","urn:GEMReadoutApplication-soap:1");
  
}

gem::base::GEMReadoutApplication::~GEMReadoutApplication()
{
  
}
