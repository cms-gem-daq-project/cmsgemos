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

gem::hw::amc13::AMC13Readout::AMC13Readout(xdaq::ApplicationStub* stub)
  throw (xdaq::exception::Exception) :
  gem::readout::GEMReadoutApplication(stub),
  m_cardName("CardName"),
  m_crateID(0),
  m_slot(0)
{
  DEBUG("AMC13Readout ctor begin");
  p_appInfoSpace->fireItemAvailable("CardName",       &m_cardName);
  p_appInfoSpace->fireItemAvailable("crateID",        &m_crateID );
  p_appInfoSpace->fireItemAvailable("slot",           &m_slot    );

  p_appInfoSpace->addItemRetrieveListener("CardName", this);
  p_appInfoSpace->addItemRetrieveListener("crateID",  this);
  p_appInfoSpace->addItemRetrieveListener("slot",     this);

  p_appInfoSpace->addItemChangedListener( "CardName", this);
  p_appInfoSpace->addItemChangedListener( "crateID",  this);
  p_appInfoSpace->addItemChangedListener( "slot",     this);

  DEBUG("AMC13Readout::AMC13Readout() "                        << std::endl
        << " m_cardName:"       << m_cardName.toString()       << std::endl
        << " m_deviceName:"     << m_deviceName.toString()     << std::endl
        << " m_connectionFile:" << m_connectionFile.toString() << std::endl
        << " m_eventsReadout:"  << m_eventsReadout.toString()  << std::endl
        << " m_crateID:"        << m_crateID.toString()        << std::endl
        << " m_slot:"           << m_slot.toString()           << std::endl
        );
  DEBUG("AMC13Readout ctor end");
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

  DEBUG("AMC13Readout::actionPerformed() " << event.type()     << std::endl
        << " m_cardName:"       << m_cardName.toString()       << std::endl
        << " m_deviceName:"     << m_deviceName.toString()     << std::endl
        << " m_connectionFile:" << m_connectionFile.toString() << std::endl
        << " m_eventsReadout:"  << m_eventsReadout.toString()  << std::endl
        << " m_crateID:"        << m_crateID.toString()        << std::endl
        << " m_slot:"           << m_slot.toString()           << std::endl
        );
  // update monitoring variables
  gem::readout::GEMReadoutApplication::actionPerformed(event);
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
  DEBUG("AMC13Readout::configureAction begin");
  // grab these from the config, updated through SOAP too
  //m_outFileName  = m_readoutSettings.bag.fileName.toString();
  gem::readout::GEMReadoutApplication::configureAction();
}

void gem::hw::amc13::AMC13Readout::startAction()
  throw (gem::hw::amc13::exception::Exception)
{
  DEBUG("AMC13Readout::startAction begin");
  gem::readout::GEMReadoutApplication::startAction();
}

void gem::hw::amc13::AMC13Readout::pauseAction()
  throw (gem::hw::amc13::exception::Exception)
{
  DEBUG("AMC13Readout::pauseAction begin");
  gem::readout::GEMReadoutApplication::pauseAction();
}

void gem::hw::amc13::AMC13Readout::resumeAction()
  throw (gem::hw::amc13::exception::Exception)
{
  DEBUG("AMC13Readout::resumeAction begin");
  gem::readout::GEMReadoutApplication::resumeAction();
}

void gem::hw::amc13::AMC13Readout::stopAction()
  throw (gem::hw::amc13::exception::Exception)
{
  DEBUG("AMC13Readout::stopAction begin");
  gem::readout::GEMReadoutApplication::stopAction();
}

void gem::hw::amc13::AMC13Readout::haltAction()
  throw (gem::hw::amc13::exception::Exception)
{
  DEBUG("AMC13Readout::haltAction begin");
  gem::readout::GEMReadoutApplication::haltAction();
}

void gem::hw::amc13::AMC13Readout::resetAction()
  throw (gem::hw::amc13::exception::Exception)
{
  DEBUG("AMC13Readout::resetAction begin");
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
  DEBUG("AMC13Readout::dumpData begin");
  size_t siz;
  int rc;
  uint64_t* pEvt;

  //FILE *fp;
  //fp = fopen(m_outFileName.c_str(), "a");
  std::ofstream outf(m_outFileName.c_str(), std::ios_base::app | std::ios::binary );
  int nwrote = 0;

  DEBUG("File for output open");
  while (true) {
    DEBUG("Get number of events in the buffer");
    int nevt = p_amc13->read( ::amc13::AMC13Simple::T1, "STATUS.MONITOR_BUFFER.UNREAD_BLOCKS");
    DEBUG("Trying to read " << std::dec << nevt << " events" << std::endl);
    for (int i = 0; i < nevt; i++) {
      if ( (i % 100) == 0)
        DEBUG("calling readEvent " << std::dec << i << "..." << std::endl);
      pEvt = p_amc13->readEvent(siz, rc);
      
      if (rc == 0 && siz > 0 && pEvt != NULL) {
        //fwrite(pEvt, sizeof(uint64_t), siz, fp);
        outf.write((char*)pEvt, siz*sizeof(uint64_t));
        ++nwrote;
      } else {
        DEBUG("No more events" << std::endl);
        break;
      }
      if (pEvt)
        free(pEvt);
    }
    if (nevt == 0) {
      DEBUG("Monitor buffer empty" << std::endl);
      break;
    }
  }
  DEBUG("Closing file" << std::endl);
  //fclose(fp);
  outf.close();
  return nwrote;
}

