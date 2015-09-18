#include "gem/readout/GEMDataParker.h"
#include "gem/hw/glib/HwGLIB.h"

#include <boost/utility/binary.hpp>
#include <bitset>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

typedef std::shared_ptr<int*> link_shared_ptr;
typedef gem::readout::GEMDataAMCformat::GEMData  AMCGEMData;
typedef gem::readout::GEMDataAMCformat::GEBData  AMCGEBData;
typedef gem::readout::GEMDataAMCformat::VFATData AMCVFATData;
std::vector<AMCVFATData> vfats;

uint16_t gem::readout::GEMslotContents::slot[24] = {
  0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,
  0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,
  0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,
};
bool gem::readout::GEMslotContents::isFileRead = false;

uint64_t bufferCount = 0;
uint32_t islotNegativeCount = 0;
uint64_t ZSFlag = 0;
bool dumpGEMevent_ = false;

std::map<uint16_t, bool> isFirst = {{0, true}};

int counterVFATs = 0;
std::map<uint16_t, int> counterVFAT = {{0,0}};

int event_ = 0;
int MaxEvent = 0;
std::map<uint16_t, int> numEC = {};

//  BX based "event conter"
uint16_t BX;
std::map<uint16_t, uint16_t> numBX = {};
std::map<uint16_t, uint16_t> BXexp = {{0,-1}};

// Main constructor
gem::readout::GEMDataParker::GEMDataParker(
                                          gem::hw::glib::HwGLIB& glibDevice,
                                          std::string const& outFileName, 
                                          std::string const& errFileName, 
                                          std::string const& outputType) 
                                          :
                                          gemLogger_(log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("gem:readout:GEMDataParker"))
){
  //gemLogger_   = log4cplus::Logger::getInstance("gem:readout:GEMDataParker");
  glibDevice_  = &glibDevice;
  outFileName_ = outFileName;
  errFileName_ = errFileName;
  outputType_  = outputType;
  counter_ = {0,0,0};
  vfat_ = 0;
  event_ = 0;
  sumVFAT_ = 0;

  gem::readout::GEMslotContents::initSlots();
}

uint64_t* gem::readout::GEMDataParker::dumpData(uint8_t const& readout_mask )
{
  // Get the size of GLIB data buffer
  uint64_t bufferDepth = 0;
  if (readout_mask&0x1)
    bufferDepth  = (uint64_t)glibDevice_->getFIFOOccupancy(0x0);
  if (readout_mask&0x2)
    bufferDepth += (uint64_t)glibDevice_->getFIFOOccupancy(0x1);
  if (readout_mask&0x4)
    bufferDepth += (uint64_t)glibDevice_->getFIFOOccupancy(0x2);
  bufferCount = bufferDepth;

  uint64_t *point = &counter_[0]; 

  INFO(" ABC::dumpData " << " counter VFATs " << counter_[0] << " , per event counter VFATs " << counter_[2] <<
        " bufferDepth " << bufferDepth );

  //if [0-7] in deviceNum
  if (readout_mask & 0x1) {
    gem::readout::GEMDataParker::dumpDataToDisk(0x0);
    DEBUG(" ABC::dumpData link0 " << " counter VFATs " << counter_[0] << " , per event counter VFATs " << counter_[2] );
  }
  //if [8-15] in deviceNum
  if (readout_mask & 0x2) {
    gem::readout::GEMDataParker::dumpDataToDisk(0x1);
    DEBUG(" ABC::dumpData link1 " << " counter VFATs " << counter_[0] << " , per event counter VFATs " << counter_[2] );
  }
  //if [16-23] in deviceNum
  if (readout_mask & 0x4) {
    gem::readout::GEMDataParker::dumpDataToDisk(0x2);
    DEBUG(" ABC::dumpData link2 " << " counter VFATs " << counter_[0] << " , per event counter VFATs " << counter_[2] );
  }
  return point;
}


void gem::readout::GEMDataParker::dumpDataToDisk(
						uint8_t const& link
){
  /*
   * get GLIB data from one VFAT chip, as it's (update that part for MP7 when it'll be)
   */
  vfat_ = gem::readout::GEMDataParker::getGLIBData(link);

  counter_[0] = vfat_;
  counter_[1] = event_;
  counter_[2] = counterVFATs;

  DEBUG(" ABC::dumpDataToDisk " << "counter VFATs " << counter_[0] << " event " << counter_[1] << " , per event counter VFATs " << counter_[2]);

}


int gem::readout::GEMDataParker::getGLIBData(
                                            uint8_t const& link
){
 /*  
  *  GEM Event Data Format definition
  */
  AMCGEMData  gem; 
  AMCGEBData  geb;
  AMCVFATData vfat;

  int islot = -1;

  // Booking FIFO variables
  uint8_t  SBit, flags;
  uint16_t bcn, evn, chipid, vfatcrc;
  uint32_t TrigReg, BXOHTrig;
  uint64_t msVFAT, lsVFAT;

  /** the FIFO depth is not reliable */
  uint64_t bufferDepth = 0;

  bufferDepth = (uint64_t)glibDevice_->getFIFOOccupancy(link);
  INFO(" ABN::bufferDepth = " << std::hex << bufferDepth << std::dec);

  // For each event in GLIB data buffer should probably switch this while with the next if, 
  // to ensure that there is actually a value in the vector

  while (bufferDepth) {
    std::vector<uint32_t> data;

    if (glibDevice_->hasTrackingData(link)) {
      data = glibDevice_->getTrackingData(link);
    }

    // read trigger data
    TrigReg = glibDevice_->readTriggerFIFO(link);
    BXOHTrig = TrigReg >> 6;
    SBit = TrigReg & 0x0000003F;

    uint16_t b1010, b1100, b1110;
    b1010 = ((data.at(5) & 0xF0000000)>>28);
    b1100 = ((data.at(5) & 0x0000F000)>>12);
    b1110 = ((data.at(4) & 0xF0000000)>>28);
	
    if (!(((b1010 == 0xa) && (b1100==0xc) && (b1110==0xe)))) {
      WARN("VFAT headers do not match expectation");
      /* do not ignore incorrect data
         bufferDepth = glibDevice_->getFIFOOccupancy(link);
         continue;
      */
    }

    BX = (uint16_t)data.at(6);
    vfat_++;

    bcn     = (0x0fff0000 & data.at(5)) >> 16;
    evn     = (0x00000ff0 & data.at(5)) >> 4;
    chipid  = (0x0fff0000 & data.at(4)) >> 16;
    flags   = (0x0000000f & data.at(5));
    vfatcrc = (0x0000ffff & data.at(0));

    islot = gem::readout::GEMslotContents::GEBslotIndex( (uint32_t)chipid );
    if (islot<0 && islot > 23) {
      // islot out of [0-23]
      islotNegativeCount++;
      DEBUG(" ABC::getGLIBData warning !!! islot is negative " << islot << " islotNegativeCount " << islotNegativeCount );
      continue;
    }

    if ( BX == BXexp.find(BX)->second ) { 
      isFirst.erase(BX);
      isFirst.insert(std::pair<uint16_t, bool>(BX,false));
    } else { 
      isFirst.erase(BX);
      isFirst.insert(std::pair<uint16_t, bool>(BX,true));
    }
  
    DEBUG(" ABC::getGLIBData BX " << std::hex << BX << std::dec << " bool " << isFirst.find(BX)->second );
    if ( isFirst.find(BX)->second ) {

      isFirst.erase(BX);
      isFirst.insert(std::pair<uint16_t, bool>(BX,false));

      BXexp.erase(BX);
      BXexp.insert(std::pair<uint16_t, uint16_t>(BX,BX));

      counterVFATs = 0;
      counterVFAT.erase(BX);
      counterVFAT.insert(std::pair<uint16_t, int>(BX,0));

      numBX.erase(BX);
      numBX.insert(std::pair<uint16_t, uint16_t>(BX,0));
      INFO(" ABC::getGLIBData isFirst  BXexp 0x" << std::hex << BXexp.find(evn)->second << " BX 0x" << BX << std::dec << 
           " vfat_ " << vfat_ << " event_ " << event_ );

      ZSFlag = 0;

    }
    counterVFATs++;
    counterVFAT.erase(BX);
    counterVFAT.insert(std::pair<uint16_t, int>(BX,counterVFATs));
  
    bufferCount--;

    std::map<uint16_t, uint16_t>::iterator it;

    it=numBX.find(BX);
    if (it != numBX.end()){
      // local event calculator inside one buffer, BX based 
      MaxEvent = numBX.find(BX)->second;
      MaxEvent++;
      numBX.erase(BX);
      numBX.insert(std::pair<uint16_t, uint16_t>(BX,MaxEvent));
      DEBUG(" ABC::getGLIBData BX 0x" << std::hex << BX << std::dec << " numBX " <<  numBX.find(evn)->second );
    }

    uint64_t data1  = ((0x0000ffff & data.at(4)) << 16) | ((0xffff0000 & data.at(3)) >> 16);
    uint64_t data2  = ((0x0000ffff & data.at(3)) << 16) | ((0xffff0000 & data.at(2)) >> 16);
    uint64_t data3  = ((0x0000ffff & data.at(2)) << 16) | ((0xffff0000 & data.at(1)) >> 16);
    uint64_t data4  = ((0x0000ffff & data.at(1)) << 16) | ((0xffff0000 & data.at(0)) >> 16);
  
    lsVFAT = (data3 << 32) | (data4);
    msVFAT = (data1 << 32) | (data2);

    vfat.BC     = ( b1010 << 12 ) | (bcn);                // 1010     | bcn:12
    vfat.EC     = ( b1100 << 12 ) | (evn << 4) | (flags); // 1100     | EC:8      | Flag:4
    vfat.ChipID = ( b1110 << 12 ) | (chipid);             // 1110     | ChipID:12
    vfat.lsData = lsVFAT;                                 // lsData:64
    vfat.msData = msVFAT;                                 // msData:64
    vfat.BXfrOH = BX;                                     // BXfrOH:16
    vfat.crc    = vfatcrc;                                // crc:16

    bufferDepth = (uint64_t)glibDevice_->getFIFOOccupancy(link);
  
    INFO(" ABV::getGLIBData bufferDepth " << bufferDepth << " event_ " << event_ 
         /* << " vfat_ " << vfat_ << " counterVFATs " << counterVFAT.find(evn)->second */ <<
	 " vfats.size " << int(vfats.size()) << std::hex << " BX 0x" << BX << std::dec );

   /*
    * dump VFAT data
    GEMDataAMCformat::printVFATdataBits(vfat_, vfat);
    INFO(" ABC::getGLIBData slot " << islot );
   */

   /*
    * VFATs Pay Load
    */
    vfats.push_back(vfat);
  
    if ( bufferCount == 0 ){
      
      DEBUG(" CDE::getGLIBData vfats.size " << int(vfats.size()) << " bufferCount " << bufferCount << " event " << event_);

      int IlocalEvent = 0;
      // contents all local events (one buffer, all links):
      for (std::map<uint16_t, uint16_t>::iterator it=numBX.begin(); it!=numBX.end(); ++it){
         event_++;
         IlocalEvent++;
         DEBUG(" ABC::getGLIBData END BX 0x" << std::hex << it->first << std::dec << " numBX " <<  it->second << " event_ " << event_);

         int nChip = 0;
         for (std::vector<GEMDataAMCformat::VFATData>::iterator iVFAT=vfats.begin(); iVFAT != vfats.end(); ++iVFAT) {

           uint16_t localEvent = (*iVFAT).BXfrOH;
           DEBUG(" vfats evn 0x" << it->first << " EC " << localEvent );

           if ( it->first == localEvent ) {
             nChip++;
             vfat.BC     = (*iVFAT).BC;
             vfat.EC     = (*iVFAT).EC;
             vfat.ChipID = (*iVFAT).ChipID;
             vfat.lsData = (*iVFAT).lsData;
             vfat.msData = (*iVFAT).msData;
             vfat.BXfrOH = (*iVFAT).BXfrOH;
             vfat.crc    = (*iVFAT).crc;
             
            /*
             * VFATs Pay Load
             */
             geb.vfats.push_back(vfat);
         
             /*
             GEMDataAMCformat::printVFATdataBits(nChip, vfat);
             */
             int islot = gem::readout::GEMslotContents::GEBslotIndex((uint32_t)vfat.ChipID );
             DEBUG(" ABC::getGLIBData  writeGEMevent slot " << islot);

             if ( gem::readout::GEMDataParker::VFATfillData( islot, geb) ){
                 if ( it->second == nChip ){

                    /*    
                     * GEM data filling
                     */
                     DEBUG(" writeGEMevent Complete and could be finally filled and keeped " );

                    /*
                     * GEM headers and trealers filling
                     */
                     gem::readout::GEMDataParker::GEMfillHeaders(evn, gem, geb);
                     gem::readout::GEMDataParker::GEMfillTrailers(gem, geb);
        
                    /*
                     * GEM Event Writing
                     */
                     DEBUG(" ABC::getGLIBData writing...  geb.vfats.size " << int(geb.vfats.size()) );
                     if(int(geb.vfats.size()) != 0) gem::readout::GEMDataParker::writeGEMevent(gem, geb, vfat);
        
                     geb.vfats.clear();
        
    	         }//end of writing event

             } else {
               DEBUG(" ABC::getGLIBData warning slot wrong !!!" << islot);
             }// if slot correct

  	   }// if localEvent
         }//end of GEB PayLoad Data
       }//end of all local events

       INFO(" CDE::getGLIBData  vfats.size " << int(vfats.size()) << " IlocalEvent " << IlocalEvent << " event " << event_ );

       vfats.clear();
       IlocalEvent = 0;

       // local event cleaning 
       numEC.clear();
       numBX.clear();
       MaxEvent = 0;

       // reset event logic
       isFirst.erase(BX);
       isFirst.insert(std::pair<uint16_t, bool>(BX,true));

       counterVFAT.clear();
       ZSFlag = 0;

    }//end if, event writing
  }// while(!bufferDepth)

  return vfat_;
}


bool gem::readout::GEMDataParker::VFATfillData(
                                               int const& islot,
                                               AMCGEBData&  geb
){
  /*
   * One GEM bord loop, 24 VFAT chips maximum
   * VFAT position definition on the board, in the future from OH
   */
  
   if (islot == -1) {
     INFO(" ::VFATfillData warning : wrong slot Index !!!" );
     return (false);

   } else {
     // Chamber Header, Zero Suppression flags, Chamber ID
     ZSFlag           = (ZSFlag | (1 << (23-islot))); // :24
     uint64_t ChamID  = 0xdea;                        // :12
     uint64_t sumVFAT = int(geb.vfats.size());        // :28
    
     geb.header  = (ZSFlag << 40)|(ChamID << 28)|(sumVFAT);
    
     ZSFlag =  (0xffffff0000000000 & geb.header) >> 40; 
     ChamID =  (0x000000fff0000000 & geb.header) >> 28; 
     sumVFAT=  (0x000000000fffffff & geb.header);    

     DEBUG(" ::VFATfillData ChamID 0x" << ChamID << std::dec << " islot " << islot << " sumVFAT " << sumVFAT);
     //GEMDataAMCformat::show24bits(ZSFlag); 

     return (true);

   }//end while
}


void gem::readout::GEMDataParker::GEMfillHeaders(
                                                 uint16_t const& BC,
                                                 AMCGEMData& gem,
                                                 AMCGEBData& geb
){
  /*
   *  GEM, All Chamber Data
   */

  // GEM Event Headers [1]
  uint64_t AmcNo       = BOOST_BINARY( 1 );    // :4 
  uint64_t ZeroFlag    = BOOST_BINARY( 0000 ); // :4
  uint64_t LV1ID       = BC;                   // :24
  uint64_t BXID        = BOOST_BINARY( 1 );    // :12
  uint64_t DataLgth    = BOOST_BINARY( 1 );    // :20

  gem.header1 = (AmcNo <<60)|(ZeroFlag << 56)|(LV1ID <<32)|(BXID << 20)|(DataLgth);

  AmcNo    =  (0xf000000000000000 & gem.header1) >> 60;
  ZeroFlag =  (0x0f00000000000000 & gem.header1) >> 56; 
  LV1ID    =  (0x00ffffff00000000 & gem.header1) >> 32; 
  BXID     =  (0x00000000fff00000 & gem.header1) >> 20;
  DataLgth =  (0x00000000000fffff & gem.header1);

  DEBUG(" ABC::GEMfillHeaders event_" << event_ << " LV1ID " << LV1ID << " BC " << BC);

  // GEM Event Headers [2]
  uint64_t User        = BOOST_BINARY( 1 );    // :32
  uint64_t OrN         = BOOST_BINARY( 1 );    // :16
  uint64_t BoardID     = BOOST_BINARY( 1 );    // :16

  gem.header2 = (User << 32)|(OrN << 16)|(BoardID);

  User     =  (0xffffffff00000000 & gem.header2) >> 32; 
  OrN      =  (0x00000000ffff0000 & gem.header2) >> 16;
  BoardID  =  (0x000000000000ffff & gem.header2);

  // GEM Event Headers [3]
  uint64_t DAVList     = BOOST_BINARY( 1 );    // :24
  uint64_t BufStat     = BOOST_BINARY( 1 );    // :24
  uint64_t DAVCount    = BOOST_BINARY( 1 );    // :5
  uint64_t FormatVer   = BOOST_BINARY( 1 );    // :3
  uint64_t MP7BordStat = BOOST_BINARY( 1 );    // :8

  gem.header3 = (BufStat << 40)|(DAVCount << 16)|(DAVCount << 11)|(FormatVer << 8)|(MP7BordStat);

  DAVList     = (0xffffff0000000000 & gem.header3) >> 40; 
  BufStat     = (0x000000ffffff0000 & gem.header3) >> 16;
  DAVCount    = (0x000000000000ff00 & gem.header3) >> 11;
  FormatVer   = (0x0000000000000f00 & gem.header3) >> 8;
  MP7BordStat = (0x00000000000000ff & gem.header3);

  // RunType:4, all other depends from RunType
  uint64_t RunType = BOOST_BINARY( 1 ); // :4

  geb.runhed  = (RunType << 60);

}


void gem::readout::GEMDataParker::GEMfillTrailers(
                                                  AMCGEMData&  gem,
                                                  AMCGEBData&  geb
){
  /*
   *  GEM, All Chamber Data
   */

  // GEM Event Treailer [2]
  uint64_t EventStat  = BOOST_BINARY( 1 );    // :32
  uint64_t GEBerrFlag = BOOST_BINARY( 1 );    // :24

  gem.trailer2 = ( EventStat << 40)|(GEBerrFlag);

  /* SB
  uint64_t FormatVer   = (0xffffffffff000000 & gem.trailer2) >> 40;
  uint64_t MP7BordStat = (0x0000000000ffffff & gem.trailer2);
  */

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

  DEBUG(" OHcrc " << std::hex << OHcrc << " OHwCount " << OHwCount << " ChamStatus " << ChamStatus << std::dec);

}


void gem::readout::GEMDataParker::writeGEMevent(
                                                AMCGEMData&  gem,
                                                AMCGEBData&  geb,
                                                AMCVFATData& vfat
){
  INFO(" ABC::writeGEMevent vfat_ " << vfat_ << " event " << event_ << " sumVFAT " << (0x000000000fffffff & geb.header) <<
       " geb.vfats.size " << int(geb.vfats.size()) );

  /*
    int nGEB=0;
    for (vector<GEBData>::iterator iGEB=gem.gebs.begin(); iGEB != gem.gebs.end(); ++iGEB) {
    nGEB++; uint64_t ZSFlag =  (0xffffff0000000000 & geb.header) >> 40; show24bits(ZSFlag);
  */

 /*
  *  GEM Chamber's Data
  */

  if (outputType_ == "Hex") {
    GEMDataAMCformat::writeGEMhd1 (outFileName_, event_, gem);
    GEMDataAMCformat::writeGEMhd2 (outFileName_, event_, gem);
    GEMDataAMCformat::writeGEMhd3 (outFileName_, event_, gem);
  } else {
    GEMDataAMCformat::writeGEMhd1Binary (outFileName_, event_, gem);
    GEMDataAMCformat::writeGEMhd2Binary (outFileName_, event_, gem);
    GEMDataAMCformat::writeGEMhd3Binary (outFileName_, event_, gem);
  } 

 /*
  *  GEB Headers Data
  */

  if (outputType_ == "Hex") {
    GEMDataAMCformat::writeGEBheader (outFileName_, event_, geb);
    GEMDataAMCformat::writeGEBrunhed (outFileName_, event_, geb);
  } else {
    GEMDataAMCformat::writeGEBheaderBinary (outFileName_, event_, geb);
    GEMDataAMCformat::writeGEBrunhedBinary (outFileName_, event_, geb);
  } // GEMDataAMCformat::printGEBheader (event_, geb);
    
 /*
  *  GEB PayLoad Data
  */

  int nChip=0;
  for (std::vector<GEMDataAMCformat::VFATData>::iterator iVFAT=geb.vfats.begin(); iVFAT != geb.vfats.end(); ++iVFAT) {
    nChip++;
    vfat.BC     = (*iVFAT).BC;
    vfat.EC     = (*iVFAT).EC;
    vfat.ChipID = (*iVFAT).ChipID;
    vfat.lsData = (*iVFAT).lsData;
    vfat.msData = (*iVFAT).msData;
    vfat.crc    = (*iVFAT).crc;
      
    if (outputType_ == "Hex") {
      GEMDataAMCformat::writeVFATdata (outFileName_, nChip, vfat); 
    } else {
      GEMDataAMCformat::writeVFATdataBinary (outFileName_, nChip, vfat);
    };
    //GEMDataAMCformat::printVFATdataBits(nChip, vfat);
    //INFO(" writeGEMevent slot " << gem::readout::GEMslotContents::GEBslotIndex( (uint32_t)vfat.ChipID ) );

  }//end of GEB PayLoad Data

 /*
  *  GEB Trailers Data
  */

  if (outputType_ == "Hex") {
    GEMDataAMCformat::writeGEBtrailer (outFileName_, event_, geb);
  } else {
    GEMDataAMCformat::writeGEBtrailerBinary (outFileName_, event_, geb);
  } 

 /*
  *  GEM Trailers Data
  */

  if (outputType_ == "Hex") {
    GEMDataAMCformat::writeGEMtr2 (outFileName_, event_, gem);
    GEMDataAMCformat::writeGEMtr1 (outFileName_, event_, gem);
  } else {
    GEMDataAMCformat::writeGEMtr2Binary (outFileName_, event_, gem);
    GEMDataAMCformat::writeGEMtr1Binary (outFileName_, event_, gem);
  } 

  uint64_t ZSFlag =  (0xffffff0000000000 & geb.header) >> 40; GEMDataAMCformat::show24bits(ZSFlag);
  INFO(" writeGEMevent:: end of event " << event_ << "\n");
  /* } // end of GEB */
}
