/**
 * class: GLIBReadout
 * description: Application to handle reading out from FIFO on GLIB
 *              structure borrowed from HCAL
 * author: J. Sturdy
 * date: 14/03/2016
 */

#include <gem/hw/glib/GLIBReadout.h>
#include <gem/hw/glib/HwGLIB.h>
#include <gem/utils/soap/GEMSOAPToolBox.h>
#include <gem/readout/exception/Exception.h>

#include <boost/utility/binary.hpp>
#include <bitset>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

XDAQ_INSTANTIATOR_IMPL(gem::hw::glib::GLIBReadout);

const uint32_t gem::hw::glib::GLIBReadout::kUPDATE  = 5000;
const uint32_t gem::hw::glib::GLIBReadout::kUPDATE7 = 7;

gem::hw::glib::GLIBReadout::GLIBReadout(xdaq::ApplicationStub* stub) :
  GEMReadoutApplication(stub),
  m_runType(0x0),
  m_runParams(0x0),
  m_ESexp(-1),
  m_isFirst(true),
  m_contvfats(0),
  m_queueLock(toolbox::BSem::FULL, true)
{
  xoap::bind(this,&GLIBReadout::updateScanParameters,"UpdateScanParameter","urn:GLIBReadout-soap:1");
  //xoap::bind(this,&GLIBReadout::queueDepth,          "QueueDepth",         "urn:GLIBReadout-soap:1");
  p_appInfoSpace->fireItemAvailable("QueueDepth", &m_queueDepth);
}

gem::hw::glib::GLIBReadout::~GLIBReadout()
{
  DEBUG("GLIBReadout::destructor called");
}

void gem::hw::glib::GLIBReadout::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("GLIBReadout::actionPerformed() setDefaultValues" << 
          "Default configuration values have been loaded from xml profile");
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::glib::GLIBReadout::initializeAction()
  throw (gem::hw::glib::exception::Exception)
{
  INFO("GLIBReadout::initializeAction begin");
  try {
    p_glib = glib_shared_ptr(new gem::hw::glib::HwGLIB(m_deviceName.toString(), m_connectionFile.toString()));
  } catch (gem::hw::glib::exception::Exception const& ex) {
    ERROR("GLIBReadout::initializeAction caught exception " << ex.what());
    XCEPT_RAISE(gem::hw::glib::exception::Exception, "initializeAction failed");
  } catch (toolbox::net::exception::MalformedURN const& ex) {
    ERROR("GLIBReadout::initializeAction caught exception " << ex.what());
    XCEPT_RAISE(gem::hw::glib::exception::Exception, "initializeAction failed");
  } catch (std::exception const& ex) {
    ERROR("GLIBReadout::initializeAction caught exception " << ex.what());
    XCEPT_RAISE(gem::hw::glib::exception::Exception, "initializeAction failed");
  }
  DEBUG("GLIBReadout::initializeAction connected");
  
}


void gem::hw::glib::GLIBReadout::configureAction()
  throw (gem::hw::glib::exception::Exception)
{
  INFO("GLIBReadout::configureAction begin");
  // grab these from the config, updated through SOAP too
  m_outFileName  = m_readoutSettings.bag.fileName.toString();
  m_errFileName  = m_outFileName + "_ERR";
  //m_slotFileName = slotFileName;
  m_outputType   = m_readoutSettings.bag.outputType.toString();
  m_counter = {0,0,0,0,0};
  m_vfat = 0;
  m_event = 0;
  m_sumVFAT = 0;
}

void gem::hw::glib::GLIBReadout::startAction()
  throw (gem::hw::glib::exception::Exception)
{
  INFO("GLIBReadout::startAction begin");
}

void gem::hw::glib::GLIBReadout::pauseAction()
  throw (gem::hw::glib::exception::Exception)
{
  INFO("GLIBReadout::pauseAction begin");
}

void gem::hw::glib::GLIBReadout::resumeAction()
  throw (gem::hw::glib::exception::Exception)
{
  INFO("GLIBReadout::resumeAction begin");
}

void gem::hw::glib::GLIBReadout::stopAction()
  throw (gem::hw::glib::exception::Exception)
{
  INFO("GLIBReadout::stopAction begin");
}

void gem::hw::glib::GLIBReadout::haltAction()
  throw (gem::hw::glib::exception::Exception)
{
  INFO("GLIBReadout::haltAction begin");
}

void gem::hw::glib::GLIBReadout::resetAction()
  throw (gem::hw::glib::exception::Exception)
{
  INFO("GLIBReadout::resetAction begin");
}

uint32_t* gem::hw::glib::GLIBReadout::dumpData(uint8_t const& readout_mask)
{

  INFO("info dump data parker");
  DEBUG("Reading out dumpData(" << (int)readout_mask << ")");
  uint32_t *point = &m_counter[0]; 
  m_contvfats = 0;
  uint32_t* pDu = getGLIBData(readout_mask, m_counter);
  DEBUG("point 0x" << std::hex << point << " pDu 0x" << pDu << std::dec);
  if (pDu)
    for (unsigned count = 0; count < 5; ++count) m_counter[count] = *(pDu+count);
  
  return point;
}

uint32_t* gem::hw::glib::GLIBReadout::getGLIBData(uint8_t const& gtx, uint32_t counter[5])
{
  uint32_t *point = &counter[0]; 
  
  DEBUG("GLIBReadout::getGLIBData Starting while loop readout "
        << std::endl << "FIFO VFAT block depth 0x" << std::hex
        << p_glib->getFIFOVFATBlockOccupancy(gtx)
        << std::endl << "FIFO depth 0x" << std::hex
        << p_glib->getFIFOOccupancy(gtx)
        );
  while ( p_glib->getFIFOVFATBlockOccupancy(gtx) ) {
    DEBUG("GLIBReadout::getGLIBData initiating call to getTrackingData(gtx,"
          << p_glib->getFIFOVFATBlockOccupancy(gtx) << ")");
    std::vector<uint32_t> data = p_glib->getTrackingData(gtx, p_glib->getFIFOVFATBlockOccupancy(gtx));
    DEBUG("GLIBReadout::getGLIBData"
          << std::endl << "FIFO VFAT block depth 0x" << std::hex
          << p_glib->getFIFOVFATBlockOccupancy(gtx)
          << std::endl << "FIFO depth 0x" << std::hex
          << p_glib->getFIFOOccupancy(gtx)
          );

    uint32_t contqueue = 0;
    for (auto iword = data.begin(); iword != data.end(); ++iword) {
      contqueue++;
      //gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_queueLock);
      DEBUG(" ::getGLIBData pushing into queue 0x"
            << std::setfill('0') << std::setw(8) << std::hex << *iword << std::dec );
      m_dataque.push(*iword);
      if (contqueue%kUPDATE7 == 0 &&  contqueue != 0) {
        m_contvfats++;
        DEBUG(" ::getGLIBData counter " << contqueue << " contvfats " << m_contvfats
              << " m_dataque.size " << m_dataque.size());
      }
    }
    DEBUG(" ::getGLIBData end of while loop do we go again?" << std::endl
          << " FIFO VFAT block occupancy  0x" << std::hex << p_glib->getFIFOVFATBlockOccupancy(gtx)
          << std::endl
          << " FIFO occupancy             0x" << std::hex << p_glib->getFIFOOccupancy(gtx) << std::endl
          << " hasTrackingData            0x" << std::hex << p_glib->hasTrackingData(gtx)  << std::endl
          );
  }// while(p_glib->getFIFOVFATBlockOccupancy(gtx))
  DEBUG("GLIBReadout::getGLIBData"
        << std::endl
        << " FIFO VFAT block occupancy  0x" << std::hex << p_glib->getFIFOVFATBlockOccupancy(gtx)
        << std::endl
        << " FIFO occupancy             0x" << std::hex << p_glib->getFIFOOccupancy(gtx) << std::endl
        << " hasTrackingData            0x" << std::hex << p_glib->hasTrackingData(gtx)  << std::endl
        );
  return point;
}

uint32_t* gem::hw::glib::GLIBReadout::selectData(uint32_t counter[5])
{
  for(int j = 0; j < 5; j++) {
    INFO("GLIBReadout::selectData counter " << j <<  " "<< counter[j] );
  }
  uint32_t *point = &counter[0]; 
  DEBUG("GLIBReadout::selectData point  " << std::hex << point );
  uint32_t* pDQ = GEMEventMaker(counter);
  for (unsigned count = 0; count < 5; ++count) counter[count] = *(pDQ+count);
  return point;
}

uint32_t* gem::hw::glib::GLIBReadout::GEMEventMaker(uint32_t counter[5])
{
  uint32_t *point = &counter[0];

  AMCGEMData  gem; 
  AMCGEBData  geb;
  AMCVFATData vfat;

  //int islot = -1;

  // Booking FIFO variables
  uint64_t msVFAT, lsVFAT;
  uint32_t ES;

  DEBUG("GLIBReadout::GEMEventMaker  " << std::hex << point );
  if (m_dataque.empty()) return point;
  DEBUG(" ::GEMEventMaker m_dataque.size " << m_dataque.size() );

  this->readVFATblock(m_dataque);
  
  uint64_t data1  = dat10 | dat11;
  uint64_t data2  = dat20 | dat21;
  uint64_t data3  = dat30 | dat31;
  uint64_t data4  = dat40 | dat41;

  m_vfat++;

  //islot = slotInfo->GEBslotIndex( (uint32_t)chipid);

  // GEM Event selector
  ES = ( evn << 12 ) | bcn;
  DEBUG(" ::GEMEventMaker ES 0x" << std::hex << ES << " evn 0x"<< evn
        << " bcn 0x" << std::hex << bcn << std::dec << " vftas.size "
        << m_vfats.size() << " m_erros.size " << m_erros.size()
        << " chip ID 0x" << std::hex << (int)chipid << std::dec << 
        //" slot number " << islot << 
        " m_isFirst " << m_isFirst << " event " << m_event);
  
  lsVFAT = (data3 << 32) | (data4);
  msVFAT = (data1 << 32) | (data2);

  vfat.BC     = ( b1010 << 12 ) | (bcn);                // 1010     | bcn:12
  vfat.EC     = ( b1100 << 12 ) | (evn << 4) | (flags); // 1100     | EC:8      | Flag:4
  vfat.ChipID = ( b1110 << 12 ) | (chipid);             // 1110     | ChipID:12
  vfat.lsData = lsVFAT;                                 // lsData:64
  vfat.msData = msVFAT;                                 // msData:64
  vfat.BXfrOH = BX;                                     // BXfrOH:32
  vfat.crc    = vfatcrc;                                // crc:16

  if ( ES == m_ESexp ) { 
    m_isFirst = false;
  } else { 
    m_isFirst = true;
    
    if ( m_vfats.size() != 0 || m_erros.size() != 0 ) {
      DEBUG(" ::GEMEventMaker m_isFirst GEMevSelector ");
      GEMevSelector(m_ESexp);
    } 
    
    m_event++;
    // VFATS dimensions have limits
    m_vfats.reserve(MaxVFATS);
    m_erros.reserve(MaxERRS);
    m_ESexp = ES;
  }
  DEBUG(" ::GEMEventMaker ES 0x" << std::hex << ES << std::dec << " bool " << m_isFirst );
  //if (islot < 0 || islot > 23) {
  //  if ( int(m_erros.size()) <MaxERRS ) m_erros.push_back(vfat);
  //  DEBUG(" ::GEMEventMaker warning !!! islot is undefined " << islot << " m_erros.size " << int(m_erros.size()) );
  //} else {
  // VFATs Pay Load
  if ( int(m_vfats.size()) <= MaxVFATS )
    m_vfats.push_back(vfat);
  DEBUG(" ::GEMEventMaker m_event " << m_event << " m_vfats.size " << m_vfats.size() << std::hex << " ES 0x" << ES << std::dec );
  //}//end of event selection 

  m_queueDepth = m_dataque.size();
  p_appInfoSpace->fireItemValueRetrieve("QueueDepth");
  p_appInfoSpace->fireItemValueChanged("QueueDepth");

  counter[0] = m_vfat;
  counter[1] = m_event;
  counter[2] = m_vfats.size() + m_erros.size();
  counter[3] = m_vfats.size();
  counter[4] = m_erros.size();
  
  return point;
}

void gem::hw::glib::GLIBReadout::GEMevSelector(const  uint32_t& ES)
{
  //  GEM Event Data Format definition
  AMCGEMData  gem; 
  AMCGEBData  geb;
  AMCVFATData vfat;

  DEBUG(" ::GEMEventMaker m_vfats.size " << int(m_vfats.size()) << " m_rvent " << m_rvent << " event " << m_event);
 
  uint32_t locEvent = 0;
  uint32_t locError = 0;
  std::string TypeDataFlag = "PayLoad";
 
  // contents all local events (one buffer, all links):
  locEvent++;
  uint32_t nChip = 0;
  for (auto iVFAT=m_vfats.begin(); iVFAT != m_vfats.end(); ++iVFAT) {//try auto
 
    uint8_t ECff = ( (0x0ff0 & (*iVFAT).EC ) >> 4);
    uint32_t localEvent = ( ECff << 12 ) | ( 0x0fff & (*iVFAT).BC );

    DEBUG(" ::GEMEventMaker vfats ES 0x" << std::hex << ( 0x00ffffff & ES) << " and from vfat 0x" << 
          ( 0x00ffffff & localEvent ) << " EC 0x" << (int)ECff << " BC 0x" << ( 0x0fff & (*iVFAT).BC ) << std::dec );
 
    if ( ES == localEvent ) {
      nChip++;
      // VFATs Pay Load
      geb.vfats.push_back(*iVFAT);
      //int islot = slotInfo->GEBslotIndex((uint32_t)(*iVFAT).ChipID);
      //DEBUG(" ::GEMevSelector slot number " << islot );
 
      if (VFATfillData(/*islot, */geb) ) {
        if ( m_vfats.size() == nChip ) {
 
          GEMfillHeaders(m_event, nChip, gem, geb);
          GEMfillTrailers(gem, geb);
          // GEM Event Writing
          DEBUG(" ::GEMEventMaker writing...  geb.vfats.size " << int(geb.vfats.size()) );
          TypeDataFlag = "PayLoad";
          if(int(geb.vfats.size()) != 0) writeGEMevent(m_outFileName, false, TypeDataFlag,
                                                       gem, geb, vfat);
          // update online histograms
	  //          p_gemOnlineDQM->Update(geb);
          geb.vfats.clear();
        }// end of writing event
      }// if slot correct
    }// if localEvent
  }// end of GEB PayLoad Data
 
  geb.vfats.clear();
  TypeDataFlag = "Errors";

  // contents all local events (one buffer, all links):
  DEBUG(" ::GEMEventMaker END ES 0x" << std::hex << ES << std::dec << " errES " <<  m_erros.size() << " m_rvent " << m_rvent );
  
  uint32_t nErro = 0;
  for (auto iErr=m_erros.begin(); iErr != m_erros.end(); ++iErr) {
 
    uint8_t ECff = ( (0x0ff0 & (*iErr).EC ) >> 4);
    uint32_t localErr = ( ECff << 12 ) | ( 0x0fff & (*iErr).BC );
    DEBUG(" ::GEMEventMaker ERROR vfats ES 0x" << ES << " EC " << localErr );
  
    if( ES == localErr ) {
      nErro++;
      DEBUG(" ::GEMEventMaker " << " nErro " << nErro << " ES 0x" << std::hex << ES << std::dec );
      // VFATs Errors
      geb.vfats.push_back(*iErr);
      if ( m_erros.size() == nErro ) {
        // GEMDataAMCformat::printVFATdataBits(nErro, vfat);
        //int islot = -1;
        VFATfillData(/*islot, */geb);
        GEMfillHeaders(m_rvent, nErro, gem, geb);
        GEMfillTrailers(gem, geb);
        // GEM ERRORS Event Writing
        TypeDataFlag = "Errors";
        if(int(geb.vfats.size()) != 0) writeGEMevent(m_errFileName, false, TypeDataFlag,
                                                     gem, geb, vfat);
        geb.vfats.clear();
      }// if localErr
    }// if localErr
  }// end of GEB PayLoad Data
 
  geb.vfats.clear();
    
  if (m_event%kUPDATE == 0 &&  m_event != 0) {
    DEBUG(" ::GEMEventMaker m_vfats.size " << std::setfill(' ') << std::setw(7) << int(m_vfats.size()) <<
          " m_erros.size " << std::setfill(' ') << std::setw(3) << int(m_erros.size()) << 
          " locEvent   " << std::setfill(' ') << std::setw(6) << locEvent << 
          " locError   " << std::setfill(' ') << std::setw(3) << locError << " event " << m_event
          );
  }

  locEvent = 0;
 
  m_vfats.clear();
  m_erros.clear();
  // reset event logic
  m_isFirst = true;
}

bool gem::hw::glib::GLIBReadout::VFATfillData(/*int const& islot, */AMCGEBData&  geb)
{
  // Chamber Header, Zero Suppression flags, Chamber ID
  uint64_t ZSFlag  = 0x0;                    // :24
  uint64_t ChamID  = 0xffffffffffffffff & (0b00011111);                  // :5
  uint64_t sumVFAT = int(3*int(geb.vfats.size()));  // :11
  geb.header  = (ZSFlag << 40)|(ChamID << 35)|(sumVFAT << 23);
  ZSFlag =  (0xffffff0000000000 & geb.header) >> 40; 
  ChamID =  (0x000000fff0000000 & geb.header) >> 28; 
  sumVFAT=  (0x000000000fffffff & geb.header);    

  DEBUG(" ::VFATfillData ChamID 0x" << ChamID << std::dec
        //<< " islot " << islot
        << " sumVFAT " << sumVFAT);
  
  //if (islot == -1) {
  //  return (false);
  //} else {
  //  ZSFlag           = (ZSFlag | (1 << (23-islot))); // :24
  //  return (true);
  //}// end else
  return true;
}// end VFATfillData


void gem::hw::glib::GLIBReadout::writeGEMevent(std::string  outFile, bool const&  OKprint,
                                               std::string const& TypeDataFlag,
                                               AMCGEMData&  gem, AMCGEBData&  geb, AMCVFATData& vfat)
{
  if(OKprint) {
    DEBUG(" ::writeGEMevent m_vfat " << m_vfat << " event " << m_event << " sumVFAT " << (0x000000000fffffff & geb.header) <<
          " geb.vfats.size " << int(geb.vfats.size()) );
  }
  // GEM Chamber's Data
  if (m_outputType == "Hex") {
    gem::readout::GEMDataAMCformat::writeGEMhd1 (outFile, m_event, gem);
    gem::readout::GEMDataAMCformat::writeGEMhd2 (outFile, m_event, gem);
    gem::readout::GEMDataAMCformat::writeGEMhd3 (outFile, m_event, gem);
  } else {
    gem::readout::GEMDataAMCformat::writeGEMhd1Binary (outFile, m_event, gem);
    gem::readout::GEMDataAMCformat::writeGEMhd2Binary (outFile, m_event, gem);
    gem::readout::GEMDataAMCformat::writeGEMhd3Binary (outFile, m_event, gem);
  } 
  //  GEB Headers Data
  if (m_outputType == "Hex") {
    gem::readout::GEMDataAMCformat::writeGEBheader (outFile, m_event, geb);
    gem::readout::GEMDataAMCformat::writeGEBrunhed (outFile, m_event, geb);
  } else {
    gem::readout::GEMDataAMCformat::writeGEBheaderBinary (outFile, m_event, geb);
    //gem::readout::GEMDataAMCformat::writeGEBrunhedBinary (outFile, m_event, geb);
  } // gem::readout::GEMDataAMCformat::printGEBheader (m_event, geb);
  //  GEB PayLoad Data
  int nChip=0;
  for (auto iVFAT=geb.vfats.begin(); iVFAT != geb.vfats.end(); ++iVFAT) {
    nChip++;
     
    if (m_outputType == "Hex") {
      gem::readout::GEMDataAMCformat::writeVFATdata (outFile, nChip, *iVFAT); 
    } else {
      gem::readout::GEMDataAMCformat::writeVFATdataBinary (outFile, nChip, *iVFAT);
    };
  }//end of GEB PayLoad Data
  //  GEB Trailers Data
  if (m_outputType == "Hex") {
    gem::readout::GEMDataAMCformat::writeGEBtrailer (outFile, m_event, geb);
  } else {
    gem::readout::GEMDataAMCformat::writeGEBtrailerBinary (outFile, m_event, geb);
  } 
  //  GEM Trailers Data
  if (m_outputType == "Hex") {
    gem::readout::GEMDataAMCformat::writeGEMtr2 (outFile, m_event, gem);
    gem::readout::GEMDataAMCformat::writeGEMtr1 (outFile, m_event, gem);
  } else {
    gem::readout::GEMDataAMCformat::writeGEMtr2Binary (outFile, m_event, gem);
    gem::readout::GEMDataAMCformat::writeGEMtr1Binary (outFile, m_event, gem);
  } 
}

void gem::hw::glib::GLIBReadout::GEMfillHeaders(uint32_t const& event, uint32_t const& DAVCount_,
                                                AMCGEMData& gem, AMCGEBData& geb)
{

  // GEM, All Chamber Data
  // GEM Event Headers [1]
  uint64_t AmcNo       = BOOST_BINARY( 1 );            // :4 
  uint64_t ZeroFlag    = BOOST_BINARY( 0000 );         // :4
  uint64_t LV1ID       = (0x0000000000ffffff & event); // :24
  uint64_t BXID        = 0;                            // :12  ! why we have only 12 Bits for BX !
  uint64_t DataLgth    = BOOST_BINARY( 1 );            // :20

  gem.header1 = (AmcNo <<60)|(ZeroFlag << 56)|(LV1ID << 32)|(BXID << 20)|(DataLgth);

  AmcNo    =  (0xf000000000000000 & gem.header1) >> 60;
  ZeroFlag =  (0x0f00000000000000 & gem.header1) >> 56; 
  LV1ID    =  (0x00ffffff00000000 & gem.header1) >> 32; 
  BXID     =  (0x00000000ffffffff & gem.header1) >> 20;
  DataLgth =  (0x00000000000fffff & gem.header1);

  DEBUG(" ::GEMfillHeaders event " << event << " LV1ID " << LV1ID << " BXID " << BXID);

  // GEM Event Headers [2]
  uint64_t User        = BOOST_BINARY( 1 );    // :32
  uint64_t OrN         = BOOST_BINARY( 1 );    // :16
  uint64_t BoardID     = BOOST_BINARY( 1 );    // :16
  uint64_t FormatVersion = 0x0;
  uint64_t runType = 0x1;
  uint64_t temp = 0x00000000000000ff;

  //gem.header2 = (User << 32)|(OrN << 16)|(BoardID);
  gem.header2 = (FormatVersion << 60) | (runType << 56) | ((temp & m_latency) << 48) | ((temp & m_VT1) << 40) | ((temp & m_VT2) << 32) |(OrN << 16)|(BoardID);

  User     =  (0xffffffff00000000 & gem.header2) >> 32; 
  OrN      =  (0x00000000ffff0000 & gem.header2) >> 16;
  BoardID  =  (0x000000000000ffff & gem.header2);

  // GEM Event Headers [3]
  uint64_t DAVList     = BOOST_BINARY( 1 );    // :24
  uint64_t BufStat     = BOOST_BINARY( 1 );    // :24
  //uint64_t DAVCount    = BOOST_BINARY( 1 );    // :5
  uint64_t DAVCount    = 0x00000000ffffffff & DAVCount_;
  uint64_t FormatVer   = BOOST_BINARY( 1 );    // :3
  uint64_t MP7BordStat = BOOST_BINARY( 1 );    // :8

  gem.header3 = (BufStat << 40)|(DAVList << 16)|(DAVCount << 11)|(FormatVer << 8)|(MP7BordStat);
  DEBUG("GEM HEADER 3 " << std::hex << gem.header3 << "\n");

  DAVList     = (0xffffff0000000000 & gem.header3) >> 40; 
  BufStat     = (0x000000ffffff0000 & gem.header3) >> 16;
  uint16_t DAVCount_check;
  DAVCount_check= 0b0000000000011111 & (gem.header3 >> 11);
  FormatVer   = (0x0000000000000f00 & gem.header3) >> 8;
  MP7BordStat = (0x00000000000000ff & gem.header3);

  DEBUG("DAVCount " << std::hex << DAVCount_check << "\n");

  // last geb header:
  uint64_t runhed;
  //geb.runhed = (m_runType << 24)|(m_latency << 16)|(m_VT1 << 8)|(m_VT2);
  geb.runhed = (m_runType << 24)|(m_runParams);
  INFO("GEMfillHeadres" << geb.runhed);
}// end GEMfillHeaders

void gem::hw::glib::GLIBReadout::GEMfillTrailers(AMCGEMData&  gem,AMCGEBData&  geb)
{
  // GEM, All Chamber Data
  // GEM Event Treailer [2]
  uint64_t EventStat  = BOOST_BINARY( 1 );    // :32
  uint64_t GEBerrFlag = BOOST_BINARY( 1 );    // :24

  gem.trailer2 = ( EventStat << 40)|(GEBerrFlag);

  // GEM Event Treailer [1]
  uint64_t crc      = BOOST_BINARY( 1 );    // :32
  uint64_t LV1IDT   = BOOST_BINARY( 1 );    // :8
  uint64_t ZeroFlag = BOOST_BINARY( 0000 ); // :4
  uint64_t DataLgth = BOOST_BINARY( 1 );    // :20

  gem.trailer1 = (crc<<32)|(LV1IDT << 24)|(ZeroFlag <<20)|(DataLgth);

  crc      = (0xffffffff00000000 & gem.trailer1) >> 32;
  LV1IDT   = (0x00000000ff000000 & gem.trailer1) >> 24;
  ZeroFlag = (0x0000000000f00000 & gem.trailer1) >> 20;
  DataLgth = (0x00000000000fffff & gem.trailer1);

  // Chamber Trailer, OptoHybrid: crc, wordcount, Chamber status
  uint64_t OHcrc       = BOOST_BINARY( 1 ); // :16
  uint64_t OHwCount    = BOOST_BINARY( 1 ); // :16
  uint64_t ChamStatus  = BOOST_BINARY( 1 ); // :16
  geb.trailer = ((OHcrc << 48)|(OHwCount << 32 )|(ChamStatus << 16));

  OHcrc      = (0xffff000000000000 & geb.trailer) >> 48; 
  OHwCount   = (0x0000ffff00000000 & geb.trailer) >> 32; 
  ChamStatus = (0x00000000ffff0000 & geb.trailer) >> 16;

  DEBUG(" OHcrc 0x" << std::hex << OHcrc << " OHwCount " << OHwCount << " ChamStatus " << ChamStatus << std::dec);
}

void gem::hw::glib::GLIBReadout::readVFATblock(std::queue<uint32_t>& dataque)
{
  uint32_t datafront = 0;
  for (int iQue = 0; iQue < 7; iQue++){
    datafront = dataque.front();
    DEBUG(" ::GEMEventMaker iQue " << iQue << " 0x"
          << std::setfill('0') << std::setw(8) << std::hex << datafront << std::dec );
    //this never seems to get reset? maybe iQue%7 to read the words after the first block?
    if ((iQue%7) == 5 ) {
      dat41   = ((0xffff0000 & datafront) >> 16 );
      vfatcrc = (0x0000ffff & datafront);
    } else if ( (iQue%7) == 4 ) {
      dat40   = ((0x0000ffff & datafront) << 16 );
      dat31   = ((0xffff0000 & datafront) >> 16 );
    } else if ( (iQue%7) == 3 ) {
      dat21   = ((0xffff0000 & datafront) >> 16 );
      dat30   = ((0x0000ffff & datafront) << 16 );
    } else if ( (iQue%7) == 2 ) {
      dat11   = ((0xffff0000 & datafront) >> 16 );
      dat20   = ((0x0000ffff & datafront) << 16 );
    } else if ( (iQue%7) == 1 ) {
      b1110   = ((0xf0000000 & datafront) >> 28 );
      chipid  = ((0x0fff0000 & datafront) >> 16 );
      dat10   = ((0x0000ffff & datafront) << 16 );
    } else if ( (iQue%7) == 0 ) {
      b1010   = ((0xf0000000 & datafront) >> 28 );
      b1100   = ((0x0000f000 & datafront) >> 12 );
      bcn     = ((0x0fff0000 & datafront) >> 16 );
      evn     = ((0x00000ff0 & datafront) >>  4 );
      flags   = (0x0000000f & datafront);
      
      if (!(b1010 == 0xa && b1100 == 0xc)) {
        bool misAligned_ = true;
        while ((misAligned_) && (dataque.size()>7)){
          /* we have a misaligned word, increment misalignment counter, pop queue,
             push bad value into some form of storage for later analysis?
             then continue with the loop, but without incrementing iQue so we hopefully
             eventually align again
             
             Do not go through all the loop with condition statements 
             since iQue stays the same and we removed 7 blocks
             -MD
          */
          INFO(" ::GEMEventMaker found misaligned word 0x"
               << std::setfill('0') << std::hex << datafront << std::dec
               << " queue m_dataque.size " << dataque.size() );
          dataque.pop();
          datafront = dataque.front();
          b1010   = ((0xf0000000 & datafront) >> 28 );
          b1100   = ((0x0000f000 & datafront) >> 12 );
          bcn     = ((0x0fff0000 & datafront) >> 16 );
          evn     = ((0x00000ff0 & datafront) >>  4 );
          flags   = (0x0000000f & datafront);
          if ((b1010 == 0xa && b1100 == 0xc)) { misAligned_ = false;}
        }// end of while misaligned
      }
    } else if ( (iQue%7) == 6 ) {
      BX      = datafront;
    }
    DEBUG(" ::GEMEventMaker (pre pop) m_dataque.size " << dataque.size() );
    dataque.pop();
    DEBUG(" ::GEMEventMaker (post pop)  m_dataque.size " << dataque.size() );
  }// end queue
}



void gem::hw::glib::GLIBReadout::ScanRoutines(uint8_t latency, uint8_t VT1, uint8_t VT2)
{
  m_latency = latency;
  m_VT1 = VT1;
  m_VT2 = VT2;
  DEBUG("GLIBReadout::ScanRoutines Latency = " << (int)m_latency  << " VT1 = " << (int)m_VT1 << " VT2 = " << (int)m_VT2);
}

xoap::MessageReference gem::hw::glib::GLIBReadout::updateScanParameters(xoap::MessageReference msg)
  throw (xoap::exception::Exception)
{
  INFO("GLIBReadout::updateScanParameters()");
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
    INFO("GLIBReadout received command " << commandName);
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
  m_runParams = std::stoi(parameterValue);
  DEBUG(toolbox::toString("GLIBReadout::updateScanParameters() received command '%s' with value. %s",
                          commandName.c_str(), parameterValue.c_str()));
  return gem::utils::soap::GEMSOAPToolBox::makeFSMSOAPReply(commandName, "ParametersUpdated");
}

/*
xoap::MessageReference gem::hw::glib::GLIBReadout::queueDepth(xoap::MessageReference msg)
  throw (xoap::exception::Exception)
{
  INFO("GLIBReadout::queueDepth()");
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
    INFO("GLIBReadout received command " << commandName);
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
  DEBUG(toolbox::toString("GLIBReadout::queueDepth() received command '%s' with value. %s",
                          commandName.c_str(), parameterValue.c_str()));
  return gem::utils::soap::GEMSOAPToolBox::makeFSMSOAPReply(commandName, "ParametersUpdated");
}
*/
