#include "gem/readout/GEMDataParker.h"

#include "gem/readout/exception/Exception.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"

#include "toolbox/mem/MemoryPoolFactory.h"
#include "toolbox/mem/CommittedHeapAllocator.h"
#include "i2o/Method.h"
#include "i2o/utils/AddressMap.h"

#include "toolbox/string.h"
#include "xercesc/dom/DOMNode.hpp"
#include "xercesc/dom/DOMNodeList.hpp"
#include "xercesc/util/XercesDefs.hpp"
#include "xcept/tools.h"

#include <boost/utility/binary.hpp>
#include <bitset>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "TStopwatch.h"

typedef std::shared_ptr<int*> link_shared_ptr;

const uint32_t gem::readout::GEMDataParker::kUPDATE = 5000;
const uint32_t gem::readout::GEMDataParker::kUPDATE7 = 7;

const int gem::readout::GEMDataParker::I2O_READOUT_NOTIFY=0x84;
const int gem::readout::GEMDataParker::I2O_READOUT_CONFIRM=0x85;

// Main constructor
gem::readout::GEMDataParker::GEMDataParker(amc13::AMC13& amc13,
                                           std::string const& outFileName) :
  m_gemLogger(log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("gem:readout:GEMDataParker"))),
  m_queueLock(toolbox::BSem::FULL, true)

{
  //  these bindings necessitate that the GEMDataParker inherit from some xdaq application stuff
  //  i2o::bind(this,&GEMDataParker::onReadoutNotify,I2O_READOUT_NOTIFY,XDAQ_ORGANIZATION_ID);
  //  xoap::bind(this,&GEMDataParker::updateScanParameters,"UpdateScanParameter","urn:GEMDataParker-soap:1");
  INFO("Data Parker");

  p_amc13   = &amc13;

  uint32_t serno = p_amc13->read( amc13::AMC13Simple::T1, "STATUS.SERIAL_NO");
  printf("Connected to AMC13 serial number %d\n", serno);
  m_outFileName  = outFileName;
  //m_counter = {0,0,0,0,0};
}

void gem::readout::GEMDataParker::dumpData()
{

  std::cout << "Dump Data!" << std::endl;

  size_t siz;
  int rc;
  uint64_t* pEvt;
  uint64_t head[2];

  FILE *fp;
  fp = fopen( m_outFileName.c_str(), "w");
  int nwrote = 0;

  std::cout << "File for output open" << std::endl;
  while (1){
    std::cout << "Get number of events in the buffer" << std::endl;
    int nevt = p_amc13->read( amc13::AMC13Simple::T1, "STATUS.MONITOR_BUFFER.UNREAD_EVENTS");
    printf("Trying to read %d events\n", nevt);
    for( int i=0; i<nevt; i++) {
      if( (i % 100) == 0)
	      printf("calling readEvent (%d)...\n", i);
      pEvt = p_amc13->readEvent( siz, rc);

      if( rc == 0 && siz > 0 && pEvt != NULL) {
	      head[0] = 0xbadc0ffeebadcafe;
	      head[1] = siz;
	      fwrite( head, sizeof(uint64_t), 2, fp);
	      fwrite( pEvt, sizeof(uint64_t), siz, fp);
	      ++nwrote;
      } else {
	      printf("No more events\n");
	      break;
      }
  
      if( pEvt)
	      free( pEvt);
    }
    //if (nevt == 0) break;
  }
  fclose( fp);

}

/*
xoap::MessageReference gem::readout::GEMDataParker::updateScanParameters(xoap::MessageReference msg)
  throw (xoap::exception::Exception)
{
  INFO("GEMDataParker::updateScanParameters()");
  if (msg.isNull()) {
    XCEPT_RAISE(xoap::exception::Exception,"Null message received!");
  }
  
  std::string commandName    = "undefined";
  std::string parameterValue = "-1";
  try {
    std::pair<std::string, std::string> command
      = gem::utils::soap::GEMSOAPToolBox::extractCommandWithParameter(msg);
    commandName = command.first;
    parameterValue = command.second;
    INFO("GEMDataParker received command " << commandName);
  } catch(xoap::exception::Exception& err) {
    std::string msgBase = toolbox::toString("Unable to extract command from CommandWithParameter SOAP message");
    ERROR(toolbox::toString("%s: %s.", msgBase.c_str(), xcept::stdformat_exception_history(err).c_str()));
    XCEPT_DECLARE_NESTED(gem::readout::exception::SOAPCommandParameterProblem, top,
                         toolbox::toString("%s.", msgBase.c_str()), err);
    //p_gemApp->notifyQualified("error", top);
    std::string faultString = toolbox::toString("%s failed", commandName.c_str());
    std::string faultCode   = "Client";
    std::string detail      = toolbox::toString("%s: %s.",
                                                msgBase.c_str(),
                                                err.message().c_str());
    //this has to change to something real, but will come when data parker becomes the gem readout application
    std::string faultActor = "";
    xoap::MessageReference reply =
      gem::utils::soap::GEMSOAPToolBox::makeSOAPFaultReply(faultString, faultCode, detail, faultActor);
    return reply;
  }
  //this has to be injected into the GEM header
  m_scanParam = std::stoi(parameterValue);
  DEBUG(toolbox::toString("GEMDataParker::updateScanParameters() received command '%s' with value. %s",
                          commandName.c_str(), parameterValue.c_str()));
  return gem::utils::soap::GEMSOAPToolBox::makeFSMSOAPReply(commandName, "ParametersUpdated");
}

void gem::readout::GEMDataParker::ScanRoutines(uint8_t latency, uint8_t VT1, uint8_t VT2)
{

  m_latency = latency;
  m_VT1 = VT1;
  m_VT2 = VT2;

  INFO( " Dataparker scan routines Latency = " << (int)m_latency  << " VT1 = " << (int)m_VT1 << " VT2 = " << (int)m_VT2);

}
*/
