/**
 * class: AMC13Readout
 * description: Application to handle reading out from SDRAM on AMC13
 * author: M. Dalchenko
 * date: 31/03/2016
 */

#include "amc13/AMC13.hh"

#include <gem/hw/amc13/AMC13Readout.h>
#include <gem/utils/soap/GEMSOAPToolBox.h>
#include <gem/readout/exception/Exception.h>

XDAQ_INSTANTIATOR_IMPL(gem::hw::amc13::AMC13Readout);

gem::hw::amc13::AMC13Readout::AMC13Readout(xdaq::ApplicationStub* stub) :
  gem::readout::GEMReadoutApplication(stub)
{
  //p_appInfoSpace->fireItemAvailable("ConnectionFile", &m_connectionFile);
  p_appInfoSpace->fireItemAvailable("CardName",       &m_cardName      );
  p_appInfoSpace->fireItemAvailable("crateID",        &m_crateID       );
  p_appInfoSpace->fireItemAvailable("slot",           &m_slot          );
}

gem::hw::amc13::AMC13Readout::~AMC13Readout()
{
}

void gem::hw::amc13::AMC13Readout::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("AMC13Readout::actionPerformed() setDefaultValues" << 
          "Default configuration values have been loaded from xml profile");
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::amc13::AMC13Readout::initializeAction()
  throw (gem::hw::amc13::exception::Exception)
{
  INFO("AMC13Readout::initializeAction begin");
  //hcal has a pre-init, what is the reason to not do everything in initialize?
  std::string connection  = "${GEM_ADDRESS_TABLE_PATH}/"+m_connectionFile.toString();
  //std::string cardname    = toolbox::toString("gem.shelf%02d.amc13",m_crateID);
  std::string cardName    = m_cardName.toString();

  try {
    //gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_amc13Lock);
    DEBUG("Trying to create connection to " << cardName << " in " << connection);
    //p_amc13 = new ::amc13::AMC13(connection, cardName+".T1", cardName+".T2");
    p_amc13 = std::make_shared< ::amc13::AMC13>(connection, cardName+".T1", cardName+".T2");
  } catch (uhal::exception::exception & e) {
    ERROR("AMC13Readout::initializeAction failed, caught uhal::exception:" <<  e.what() );
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create class: ")+e.what());
  } catch (std::exception& e) {
    ERROR("AMC13Readout::initializeAction failed, caught std::exception:" << e.what() );
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create class: ")+e.what());
  } catch (...) {
    ERROR("AMC13Readout::initializeAction failed, caught ...");
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create AMC13 connection"));
  }
  DEBUG("AMC13Readout::initializeAction connected");
  
  gem::readout::GEMReadoutApplication::initializeAction();
}

void gem::hw::amc13::AMC13Readout::configureAction()
  throw (gem::hw::amc13::exception::Exception)
{
  INFO("AMC13Readout::configureAction begin");
  // grab these from the config, updated through SOAP too
  m_outFileName  = m_readoutSettings.bag.fileName.toString();
  gem::readout::GEMReadoutApplication::configureAction();
}

void gem::hw::amc13::AMC13Readout::startAction()
  throw (gem::hw::amc13::exception::Exception)
{
  INFO("AMC13Readout::startAction begin");
  gem::readout::GEMReadoutApplication::startAction();
}

void gem::hw::amc13::AMC13Readout::pauseAction()
  throw (gem::hw::amc13::exception::Exception)
{
  INFO("AMC13Readout::pauseAction begin");
  gem::readout::GEMReadoutApplication::pauseAction();
}

void gem::hw::amc13::AMC13Readout::resumeAction()
  throw (gem::hw::amc13::exception::Exception)
{
  INFO("AMC13Readout::resumeAction begin");
  gem::readout::GEMReadoutApplication::resumeAction();
}

void gem::hw::amc13::AMC13Readout::stopAction()
  throw (gem::hw::amc13::exception::Exception)
{
  INFO("AMC13Readout::stopAction begin");
  gem::readout::GEMReadoutApplication::stopAction();
}

void gem::hw::amc13::AMC13Readout::haltAction()
  throw (gem::hw::amc13::exception::Exception)
{
  INFO("AMC13Readout::haltAction begin");
  gem::readout::GEMReadoutApplication::haltAction();
}

void gem::hw::amc13::AMC13Readout::resetAction()
  throw (gem::hw::amc13::exception::Exception)
{
  INFO("AMC13Readout::resetAction begin");
  gem::readout::GEMReadoutApplication::resetAction();
}


int gem::hw::amc13::AMC13Readout::readout(unsigned int expected,
                                          unsigned int* eventNumbers,
                                          std::vector< ::toolbox::mem::Reference* >& data)
{
  return dumpData();
}


int gem::hw::amc13::AMC13Readout::dumpData()
{
  INFO("AMC13Readout::dumpData begin");
  size_t siz;
  int rc;
  uint64_t* pEvt;

  FILE *fp;
  fp = fopen( m_outFileName.c_str(), "w");
  int nwrote = 0;

  DEBUG("File for output open");
  while (1){
    DEBUG("Get number of events in the buffer");
    int nevt = p_amc13->read( ::amc13::AMC13Simple::T1, "STATUS.MONITOR_BUFFER.UNREAD_BLOCKS");
    DEBUG("Trying to read " << std::dec << nevt << " events\n");
    for( int i=0; i<nevt; i++) {
      if( (i % 100) == 0)
        DEBUG("calling readEvent " << std::dec << i << "...\n");
      pEvt = p_amc13->readEvent( siz, rc);

      if( rc == 0 && siz > 0 && pEvt != NULL) {
        fwrite( pEvt, sizeof(uint64_t), siz, fp);
        ++nwrote;
      } else {
        DEBUG("No more events\n");
        break;
      }
  
      if( pEvt)
        free( pEvt);
    }
    //if (nevt == 0) break;
  }
  fclose( fp);
  return nwrote;
}

