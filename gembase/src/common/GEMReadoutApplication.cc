/**
 * class: GEMReadoutApplication
 * description: Generic GEM application to handle readout
 *              structure borrowed from HCAL
 * author: J. Sturdy
 * date: 
 */

#include "gem/base/GEMReadoutApplication.h"
#include "gem/base/GEMWebApplication.h"

gem::base::GEMReadoutApplication::GEMReadoutApplication(xdaq::ApplicationStub* stub)
  :
  GEMApplication(stub)
{
  i2o::bind(this,&ReadoutApplication::onReadoutNotify,I2O_READOUT_NOTIFY,XDAQ_ORGANIZATION_ID);
  xoap::bind(this,&ReadoutApplication::getReadoutCredits,"GetReadoutCredits","urn:hcalReadoutApplication-soap:1");
  
}

gem::base::GEMReadoutApplication::~GEMReadoutApplication()
{
  
}
