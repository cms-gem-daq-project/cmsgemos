#include "amc13/AMC13.hh"
#include "gem/hw/amc13/AMC13Manager.h"

XDAQ_INSTANTIATOR_IMPL(gem::hw::amc13::AMC13Manager);
//copied closely from the hcal DTC manager

gem::hw::vfat::AMC13Manager::AMC13Manager(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception) :
  xdaq::WebApplication(s),
  amc13Device_(0)
  //
{
  m_crateID = -1;
  m_slot = 13;
  
  getApplicationInfoSpace()->fireItemAvailable("crateId", &m_crateId);
  getApplicationInfoSpace()->fireItemAvailable("slot",    &m_slot);
}

void gem::hw::amc13::AMC13Manager::init()                throw (gem::base::exception::Exception) {
  gem::base::GEMFSMApplication::init();

  LOG4CPLUS_DEBUG(getApplicationLogger(),"Entering gem::hw::amc13::AMC13Manager::init()");

  if (amc13Device_==0) return;

  DTCMutex::getInstance().lock(m_crateId.value_);

  // Enable DAQ Link if config doc says so
  amc13Device_->daqLinkEnable(m_daqLinkEnable);

  // Enable Fake Data Generator if config doc says so
  amc13Device_->fakeDataEnable(m_fakeDataEnable);

  // Use local TTC signal if config doc says so
  amc13Device_->localTtcSignalEnable(m_localTtcSignalEnable);

  // Enable Monitor Buffer Backpressure if config doc says so
  amc13Device_->monBufBackPressEnable(m_monBufBackPressEnable);

  // Enable the Mega Monitor Scale if config doc says so
  if(m_megaMonitorScale)
    amc13Device_->configurePrescale(1,m_preScaleFactNumOfZeros);
  else amc13Device_->configurePrescale(0,1);

  // generate internal L1A? Debugging/parasitic running only
  if(m_internalPeriodicEnable) {
    amc13Device_->configureLocalL1A(true,0,1,m_internalPeriodicPeriod,0);
  } else amc13Device_->configureLocalL1A(false,0,1,1,0);

  // set the FED id
  amc13Device_->setFEDid(m_fedId);

  // reset the T1
  amc13Device_->reset(amc13::AMC13::T1);

  // setup the monitoring helper
  m_monitoringHelper.setup(amc13Device_,m_crateId);
  std::vector<gem::base::monitor::Monitorable*> mons=m_monitoringHelper.getMonitorables();
  for (std::vector<gem::base::monitor::Monitorable*>::iterator qq=mons.begin(); qq!=mons.end(); qq++)
    exportMonitorable(*qq);

  DTCMutex::getInstance().unlock(m_crateId.value_);

}

void gem::hw::amc13::AMC13Manager::init()
  throw (gem::base::exception::Exception) {
}

void gem::hw::amc13::AMC13Manager::enable()
  throw (gem::base::exception::Exception) {
  LOG4CPLUS_DEBUG(getApplicationLogger(),"Entering gem::hw::amc13::AMC13Manager::enable()");
  gem::base::GEMFSMApplication::enable();
  DTCMutex::getInstance().lock(m_crateId.value_);
  amc13Device_->startRun();
  DTCMutex::getInstance().unlock(m_crateId.value_);
}

void gem::hw::amc13::AMC13Manager::disable()
  throw (gem::base::exception::Exception) {
  LOG4CPLUS_DEBUG(getApplicationLogger(),"Entering gem::hw::amc13::AMC13Manager::disable()");
  gem::base::GEMFSMApplication::disable();
  DTCMutex::getInstance().lock(m_crateId.value_);
  amc13Device_->endRun();
  DTCMutex::getInstance().unlock(m_crateId.value_);
}

