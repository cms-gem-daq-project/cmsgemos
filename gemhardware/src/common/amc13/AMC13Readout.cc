/**
 * class: AMC13Readout
 * description: Application to handle reading out from SDRAM on AMC13
 * author: M. Dalchenko
 * date: 31/03/2016
 */

#include <gem/hw/amc13/AMC13Readout.h>
#include <gem/hw/amc13/HwAMC13.h>
#include <gem/utils/soap/GEMSOAPToolBox.h>
#include <gem/readout/exception/Exception.h>

XDAQ_INSTANTIATOR_IMPL(gem::hw::glib::AMC13Readout);

const uint32_t gem::hw::glib::AMC13Readout::kUPDATE  = 5000;
const uint32_t gem::hw::glib::AMC13Readout::kUPDATE7 = 7;

void gem::hw::amc13::AMC13Readout::AMC13Readout(xdaq::ApplicationStub* stub) :
  GEMReadoutApplication(stub),
  m_runType(0x0)
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
  throw (gem::hw::glib::exception::Exception)
{
  INFO("AMC13Readout::initializeAction begin");
  try {
    //p_glib = glib_shared_ptr(new gem::hw::glib::HwGLIB(m_deviceName.toString(), m_connectionFile.toString()));
  } catch (gem::hw::glib::exception::Exception const& ex) {
    ERROR("AMC13Readout::initializeAction caught exception " << ex.what());
    XCEPT_RAISE(gem::hw::glib::exception::Exception, "initializeAction failed");
  } catch (toolbox::net::exception::MalformedURN const& ex) {
    ERROR("AMC13Readout::initializeAction caught exception " << ex.what());
    XCEPT_RAISE(gem::hw::glib::exception::Exception, "initializeAction failed");
  } catch (std::exception const& ex) {
    ERROR("AMC13Readout::initializeAction caught exception " << ex.what());
    XCEPT_RAISE(gem::hw::glib::exception::Exception, "initializeAction failed");
  }
  DEBUG("AMC13Readout::initializeAction connected");
  
}

void gem::hw::amc13::AMC13Readout::configureAction()
  throw (gem::hw::amc13::exception::Exception)
{
  INFO("AMC13Readout::configureAction begin");
  // grab these from the config, updated through SOAP too
  m_outFileName  = m_readoutSettings.bag.fileName.toString();
}

void gem::hw::amc13::AMC13Readout::startAction()
  throw (gem::hw::amc13::exception::Exception)
{
  INFO("AMC13Readout::startAction begin");
}

void gem::hw::amc13::AMC13Readout::pauseAction()
  throw (gem::hw::amc13::exception::Exception)
{
  INFO("AMC13Readout::pauseAction begin");
}

void gem::hw::amc13::AMC13Readout::resumeAction()
  throw (gem::hw::amc13::exception::Exception)
{
  INFO("AMC13Readout::resumeAction begin");
}

void gem::hw::amc13::AMC13Readout::stopAction()
  throw (gem::hw::amc13::exception::Exception)
{
  INFO("AMC13Readout::stopAction begin");
}

void gem::hw::amc13::AMC13Readout::haltAction()
  throw (gem::hw::amc13::exception::Exception)
{
  INFO("AMC13Readout::haltAction begin");
}

void gem::hw::amc13::AMC13Readout::resetAction()
  throw (gem::hw::amc13::exception::Exception)
{
  INFO("AMC13Readout::resetAction begin");
}

void gem::hw::amc13::AMC13Readout::dumpData()
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
    int nevt = p_amc13->read( amc13::AMC13Simple::T1, "STATUS.MONITOR_BUFFER.UNREAD_EVENTS");
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
}

