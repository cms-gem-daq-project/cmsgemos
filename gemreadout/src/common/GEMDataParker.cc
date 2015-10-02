#include "gem/readout/GEMDataParker.h"
#include "gem/hw/glib/HwGLIB.h"

#include <boost/utility/binary.hpp>
#include <bitset>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <queue>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

typedef std::shared_ptr<int*> link_shared_ptr;
typedef gem::readout::GEMDataAMCformat::GEMData  AMCGEMData;
typedef gem::readout::GEMDataAMCformat::GEBData  AMCGEBData;
typedef gem::readout::GEMDataAMCformat::VFATData AMCVFATData;
std::vector<AMCVFATData> vfats;
std::vector<AMCVFATData> erros;

uint16_t gem::readout::GEMslotContents::slot[24] = {
  0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,
  0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,
  0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,
};
bool gem::readout::GEMslotContents::isFileRead = false;

uint32_t islotNegativeCount = 0;
uint64_t ZSFlag = 0;
bool dumpGEMevent_ = false;

std::map<uint32_t, bool> isFirst = {{0, true}};

int counterVFATs = 0;
std::map<uint32_t, int> counterVFAT = {{0,0}};

int event_   = 0;
int eRvent_  = 0;
int MaxEvent = 0;
int MaxErr   = 0;

//  BX based "event conter"
uint32_t BX;
std::map<uint32_t, uint32_t> numBX = {};
std::map<uint32_t, uint32_t> errBX = {};
std::map<uint32_t, uint32_t> BXexp = {{0,-1}};

// The main data flow
std::queue<uint32_t> dataque;

// Main constructor
gem::readout::GEMDataParker::GEMDataParker(
                                          gem::hw::glib::HwGLIB& glibDevice,
                                          std::string const& outFileName, 
                                          std::string const& errFileName, 
                                          std::string const& outputType) 
                                          :
                                          m_gemLogger(log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("gem:readout:GEMDataParker"))
){
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
  uint64_t *point = &counter_[0]; 
  uint32_t bufferCount[4] = {0,0,0,0};
  uint32_t Counter[4] = {0,0,0,0};

  //if [0-7] in deviceNum
  if (readout_mask & 0x1) {
    uint32_t* pDu = gem::readout::GEMDataParker::dumpDataToDisk(0x0, bufferCount);
    Counter[0] = *(pDu+0);
    Counter[1] = *(pDu+1);
    Counter[2] = *(pDu+2);
    Counter[3] = *(pDu+3);
    DEBUG(" ::dumpData link0 " << " counter VFATs " << counter_[0] << " , per event counter VFATs " << counter_[2] );
  }
  //if [8-15] in deviceNum
  if (readout_mask & 0x2) {
    uint32_t* pDu = gem::readout::GEMDataParker::dumpDataToDisk(0x1, bufferCount);
    Counter[0] = *(pDu+0);
    Counter[1] = *(pDu+1);
    Counter[2] = *(pDu+2);
    Counter[3] = *(pDu+3);
    DEBUG(" ::dumpData link1 " << " counter VFATs " << counter_[0] << " , per event counter VFATs " << counter_[2] );
  }
  //if [16-23] in deviceNum
  if (readout_mask & 0x4) {
    uint32_t* pDu = gem::readout::GEMDataParker::dumpDataToDisk(0x2, bufferCount); 
    Counter[0] = *(pDu+0);
    Counter[1] = *(pDu+1);
    Counter[2] = *(pDu+2);
    Counter[3] = *(pDu+3);
 
    DEBUG(" ::dumpData link2 " << " counter VFATs " << counter_[0] << " , per event counter VFATs " << counter_[2] );
  }

  DEBUG(" ::dumpData " << " counter VFATs " << counter_[0] << " , per event counter VFATs " << counter_[2] <<
       " Combined bufferDepth[3] " << bufferCount[3] );

  INFO(" ::dumpDataToDisk  bufferCount[0] " << Counter[0] << " bufferCount[1] " << Counter[1] << " bufferCount[2] " << Counter[2] << 
       " Combined Buffer " << Counter[3] );

  return point;
}


uint32_t* gem::readout::GEMDataParker::dumpDataToDisk(
						 uint8_t const& link,
                                                 uint32_t bufferCount[4]
){
  uint32_t Counter[4] = {0,0,0,0};
  uint32_t *point = &bufferCount[0]; 
  /*
   * get GLIB data from one VFAT chip, as it's (update that part for MP7 when it'll be)
   */
  uint32_t* pDupm = gem::readout::GEMDataParker::getGLIBData(link,bufferCount);

  Counter[0] = *(pDupm+0);
  Counter[1] = *(pDupm+1);
  Counter[2] = *(pDupm+2);
  Counter[3] = *(pDupm+3);
  
  INFO(" ::dumpDataToDisk" << 
       " [0] "  << std::setfill(' ') << std::setw(6) << Counter[0] << 
       " [1] "  << std::setfill(' ') << std::setw(6) << Counter[1] << 
       " [2] "  << std::setfill(' ') << std::setw(6) << Counter[2] << 
       " Com "  << std::setfill(' ') << std::setw(7) << Counter[3] );

  DEBUG(" ::dumpDataToDisk " << "counter VFATs " << std::setfill(' ') << std::setw(5) << counter_[0] << " event " << counter_[1] << 
        " , per event counter VFATs " << counter_[2]);

  return point;
}


uint32_t* gem::readout::GEMDataParker::getGLIBData(
					          uint8_t const& link,
                                                  uint32_t bufferCount[4]
){
  uint32_t *point = &bufferCount[0]; 
  uint32_t Counter[4] = {0,0,0,0};

  /*
  AMCGEMData  gem; 
  AMCGEBData  geb;
  AMCVFATData vftmp;

  int islot = -1;

  // Booking FIFO variables
  uint8_t  SBit, flags;
  uint16_t bcn, evn, chipid, vfatcrc;
  uint32_t TrigReg, BXOHTrig;
  uint64_t msVFAT, lsVFAT;

  /** the FIFO depth is not reliable 
  DEBUG(" ::getGLIBData bufferCount[" << (int)link << "] " << bufferCount[link] << std::dec);
  */

  while ( glibDevice_->hasTrackingData(link) ) {
    std::vector<uint32_t> data;

    data = glibDevice_->getTrackingData(link);

    for (int iword=0; iword<7; iword++ ){
      dataque.push(data.at(iword));
    }

    bufferCount[(int)link]++; 

    uint32_t* pDQ = gem::readout::GEMDataParker::GEMEventMaker(link,bufferCount);
    Counter[0] = *(pDQ+0);
    Counter[1] = *(pDQ+1);
    Counter[2] = *(pDQ+2);
    Counter[3] = *(pDQ+3);
  
    /*
    uint16_t b1010, b1100, b1110;
    b1010 = ((data.at(5) & 0xF0000000)>>28);
    b1100 = ((data.at(5) & 0x0000F000)>>12);
    b1110 = ((data.at(4) & 0xF0000000)>>28);
	
    if (!(((b1010 == 0xa) && (b1100==0xc) && (b1110==0xe)))) {
      /* do not ignore incorrect data
         WARN("VFAT headers do not match expectation");
         continue;
      *
    }

    BX = data.at(6);
    vfat_++;

    bcn     = (0x0fff0000 & data.at(5)) >> 16;
    evn     = (0x00000ff0 & data.at(5)) >> 4;
    chipid  = (0x0fff0000 & data.at(4)) >> 16;
    flags   = (0x0000000f & data.at(5));
    vfatcrc = (0x0000ffff & data.at(0));

    islot = gem::readout::GEMslotContents::GEBslotIndex( (uint32_t)chipid );

    /*
    if ( BX == BXexp.find(BX)->second ) { 
      isFirst.erase(BX);
      isFirst.insert(std::pair<uint32_t, bool>(BX,false));
    } else { 
      isFirst.erase(BX);
      isFirst.insert(std::pair<uint32_t, bool>(BX,true));
    }
    */

    /*
    DEBUG(" ::getGLIBData BX " << std::hex << BX << std::dec << " bool " << isFirst.find(BX)->second );
    if ( isFirst.find(BX)->second ) {

      isFirst.erase(BX);
      isFirst.insert(std::pair<uint32_t, bool>(BX,false));

      BXexp.erase(BX);
      BXexp.insert(std::pair<uint32_t, uint32_t>(BX,BX));

      counterVFATs = 0;
      counterVFAT.erase(BX);
      counterVFAT.insert(std::pair<uint32_t, int>(BX,0));

      numBX.erase(BX);
      numBX.insert(std::pair<uint32_t, uint32_t>(BX,0));
      DEBUG(" ::getGLIBData isFirst  BXexp 0x" << std::hex << BXexp.find(BX)->second << " BX 0x" << BX << std::dec << 
           " vfat_ " << vfat_ << " eRvent_ " << eRvent_ << " event_ " << event_ );

      errBX.erase(BX);
      errBX.insert(std::pair<uint32_t, uint32_t>(BX,0));

      ZSFlag = 0;

    }
    counterVFATs++;
    counterVFAT.erase(BX);
    counterVFAT.insert(std::pair<uint32_t, int>(BX,counterVFATs));

    bufferCount[]--;

    uint64_t data1  = ((0x0000ffff & data.at(4)) << 16) | ((0xffff0000 & data.at(3)) >> 16);
    uint64_t data2  = ((0x0000ffff & data.at(3)) << 16) | ((0xffff0000 & data.at(2)) >> 16);
    uint64_t data3  = ((0x0000ffff & data.at(2)) << 16) | ((0xffff0000 & data.at(1)) >> 16);
    uint64_t data4  = ((0x0000ffff & data.at(1)) << 16) | ((0xffff0000 & data.at(0)) >> 16);
  
    lsVFAT = (data3 << 32) | (data4);
    msVFAT = (data1 << 32) | (data2);

    vftmp.BC     = ( b1010 << 12 ) | (bcn);                // 1010     | bcn:12
    vftmp.EC     = ( b1100 << 12 ) | (evn << 4) | (flags); // 1100     | EC:8      | Flag:4
    vftmp.ChipID = ( b1110 << 12 ) | (chipid);             // 1110     | ChipID:12
    vftmp.lsData = lsVFAT;                                 // lsData:64
    vftmp.msData = msVFAT;                                 // msData:64
    vftmp.BXfrOH = BX;                                     // BXfrOH:32
    vftmp.crc    = vfatcrc;                                // crc:16

    * dump VFAT data 
    GEMDataAMCformat::printVFATdataBits(vfat_, vftmp);
    INFO(" ::getGLIBData slot " << islot );
   
    /*
    std::map<uint32_t, uint32_t>::iterator it;
    std::map<uint32_t, uint32_t>::iterator ir;

    if (islot<0 || islot > 23) {
      ir=errBX.find(BX);
      if (ir != errBX.end()){
        // local event calculator inside one buffer, BX based 
        MaxErr = errBX.find(BX)->second;
        MaxErr++;
        errBX.erase(BX);
        errBX.insert(std::pair<uint32_t, uint32_t>(BX,MaxErr));
        DEBUG(" ::getGLIBData Err BX 0x" << std::hex << BX << std::dec << " errBX " <<  errBX.find(BX)->second );
      }
  
      // islot out of [0-23]
      islotNegativeCount++;
      if ( int(erros.size()) < 10000 ) erros.push_back(vfat);
      DEBUG(" ::getGLIBData warning !!! islot is undefined " << islot << " NegativeCount " << islotNegativeCount << 
           " erros.size " << int(erros.size()) );
     /*
      * dump VFAT data
      GEMDataAMCformat::printVFATdataBits(vfat_, vfat);
      INFO(" ::getGLIBData wrong slot " << islot );
      *

    } else {

      it=numBX.find(BX);
      if (it != numBX.end()){
        // local event calculator inside one buffer, BX based 
        MaxEvent = numBX.find(BX)->second;
        MaxEvent++;
        numBX.erase(BX);
        numBX.insert(std::pair<uint32_t, uint32_t>(BX,MaxEvent));
        DEBUG(" ::getGLIBData BX 0x" << std::hex << BX << std::dec << " numBX " <<  numBX.find(BX)->second << " eRvent_ " << eRvent_ );
      }

     /*
      * VFATs Pay Load
      *
      if ( int(vfats.size()) < 90000 ) vfats.push_back(vfat);
      
      DEBUG(" ::getGLIBData event_ " << event_ <<
	   " vfats.size " << int(vfats.size()) << std::hex << " BX 0x" << BX << std::dec );
    }
    */

    /*
    if ( bufferCount == 0 ){
      
       DEBUG(" ::getGLIBData vfats.size " << int(vfats.size()) << " bufferCount " << bufferCount << 
             " eRvent_ " << eRvent_ << " event " << event_);
 
       uint32_t locEvent = 0;
       uint32_t locError = 0;
       std::string TypeDataFlag = "PayLoad";
 
       // contents all local events (one buffer, all links):
       for (std::map<uint32_t, uint32_t>::iterator itBX=numBX.begin(); itBX!=numBX.end(); ++itBX){
	  event_++;
          locEvent++;
          DEBUG(" ::getGLIBData END BX 0x" << std::hex << itBX->first << std::dec << " numBX " <<  itBX->second << 
	       " locEvent " << locEvent << " event_ " << event_ );
 
          uint32_t nChip = 0;
          for (std::vector<GEMDataAMCformat::VFATData>::iterator iVFAT=vfats.begin(); iVFAT != vfats.end(); ++iVFAT) {
 
            uint32_t localEvent = (*iVFAT).BXfrOH;
            DEBUG(" ::getGLIBData vfats BX 0x" << std::hex << itBX->first << " EC 0x" << localEvent << std::dec << "\n");
 
            if ( itBX->first == localEvent ) {
              nChip++;
              vfat.BC     = (*iVFAT).BC;
              vfat.EC     = (*iVFAT).EC;
              vfat.ChipID = (*iVFAT).ChipID;
              vfat.lsData = (*iVFAT).lsData;
              vfat.msData = (*iVFAT).msData;
              vfat.BXfrOH = (*iVFAT).BXfrOH;
              vfat.crc    = (*iVFAT).crc;
              
              geb.vfats.push_back(vfat);
              //GEMDataAMCformat::printVFATdataBits(nChip, vfat);
 
              if ( gem::readout::GEMDataParker::VFATfillData( islot, geb) ){
 	       if ( itBX->second == nChip ){
 
                      gem::readout::GEMDataParker::GEMfillHeaders(event_, itBX->second, gem, geb);
                      gem::readout::GEMDataParker::GEMfillTrailers(gem, geb);
         
                      DEBUG(" ::getGLIBData writing...  geb.vfats.size " << int(geb.vfats.size()) );
                      TypeDataFlag = "PayLoad";
                      if(int(geb.vfats.size()) != 0) gem::readout::GEMDataParker::writeGEMevent(outFileName_, false, TypeDataFlag,
                                                                                                gem, geb, vfat);
                      geb.vfats.clear();
         
 	       }//end of writing event
              }// if slot correct
   	   }// if localEvent
          }//end of GEB PayLoad Data
       }// end itBX
 
       geb.vfats.clear();
       TypeDataFlag = "Errors";

       // contents all local events (one buffer, all links):
       for (std::map<uint32_t, uint32_t>::iterator irBX=errBX.begin(); irBX!=errBX.end(); ++irBX){
	  //eRvent_++;
          DEBUG(" ::getGLIBData END BX 0x" << std::hex << irBX->first << std::dec << " errBX " <<  irBX->second << 
                " eRvent_ " << eRvent_ );
 
          uint32_t nErro = 0;
          for (std::vector<GEMDataAMCformat::VFATData>::iterator iErr=erros.begin(); iErr != erros.end(); ++iErr) {
 
             uint32_t localErr = (*iErr).BXfrOH;
             DEBUG(" ::getGLIBData ERROR vfats BX 0x" << irBX->first << " EC " << localErr );
  
             if( irBX->first == localErr ) {
                nErro++;
                locError = nErro;
                vfat.BC     = (*iErr).BC;
                vfat.EC     = (*iErr).EC;
                vfat.ChipID = (*iErr).ChipID;
                vfat.lsData = (*iErr).lsData;
                vfat.msData = (*iErr).msData;
                vfat.BXfrOH = (*iErr).BXfrOH;
                vfat.crc    = (*iErr).crc;
                
                DEBUG(" ::getGLIBData " << " nErro " << nErro << " BX 0x" << std::hex << irBX->first << std::dec );
   
                geb.vfats.push_back(vfat);
   
                if ( irBX->second == nErro ){
                   //GEMDataAMCformat::printVFATdataBits(nErro, vfat);

                   int islot = -1;
                   gem::readout::GEMDataParker::VFATfillData( islot, geb);
                   gem::readout::GEMDataParker::GEMfillHeaders(eRvent_, irBX->second, gem, geb);
                   gem::readout::GEMDataParker::GEMfillTrailers(gem, geb);
              
                   TypeDataFlag = "Errors";
                   if(int(geb.vfats.size()) != 0) gem::readout::GEMDataParker::writeGEMevent(errFileName_, false, TypeDataFlag,
                                                                                             gem, geb, vfat);
                   geb.vfats.clear();
     	      }// if localErr
    	   }// if localErr
         }//end of GEB PayLoad Data
      }//end irBX
 
      geb.vfats.clear();
      INFO(" ::getGLIBData vfats.size " << std::setfill(' ') << std::setw(7) << int(vfats.size()) <<
 	                     " erros.size " << std::setfill(' ') << std::setw(3) << int(erros.size()) << 
           " locEvent   " << std::setfill(' ') << std::setw(6) << locEvent << 
 	   " locError   " << std::setfill(' ') << std::setw(3) << locError << " event " << event_
      );

      locEvent = 0;
 
      vfats.clear();
      erros.clear();
 
      // local event cleaning 
      numBX.clear();
      errBX.clear();
      MaxEvent = 0;
      MaxErr   = 0;
 
      // reset event logic
      isFirst.erase(BX);
      isFirst.insert(std::pair<uint32_t, bool>(BX,true));
 
      counterVFAT.clear();
      ZSFlag = 0;
 
    }//end if, event writing
*/

  }// while(glibDevice_->hasTrackingData(link))

  bufferCount[3] += bufferCount[(int)link];
  return point;
}


uint32_t* gem::readout::GEMDataParker::GEMEventMaker(
					          uint8_t const& link,
                                                  uint32_t bufferCount[4]
){
  uint32_t *point = &bufferCount[0];
 /*  
  *  GEM Event Data Format definition
  */
  AMCGEMData  gem; 
  AMCGEBData  geb;
  AMCVFATData vfat;

  int islot = -1;

  // Booking FIFO variables
  uint8_t  flags, ECff;
  uint16_t bcn, evn, chipid, vfatcrc;
  uint16_t b1010, b1100, b1110;
  uint64_t msVFAT, lsVFAT;
  uint32_t dat10,dat11, dat20,dat21, dat30,dat31, dat40,dat41;
  uint32_t BX, ES;

  INFO(" ::GEMEventMaker dataque.size " << dataque.size() );

  int iQue = -1;
  uint32_t datafront = 0;
  while (!dataque.empty()){
    iQue++;
    datafront = dataque.front();
    DEBUG(" ::GEMEventMaker iQue " << iQue << " " << std::hex << datafront << std::dec );

    if (iQue == 0 ){
        dat41   = ((0xffff0000 & datafront) >> 16 );
        vfatcrc = (0x0000ffff & datafront);
    } else if ( iQue == 1 ){
	dat40   = ((0x0000ffff & datafront) << 16 );
	dat31   = ((0xffff0000 & datafront) >> 16 );
    } else if ( iQue == 2 ){
	dat21   = ((0xffff0000 & datafront) >> 16 );
	dat30   = ((0x0000ffff & datafront) << 16 );
    } else if ( iQue == 3 ){
        dat11   = ((0xffff0000 & datafront) >> 16 );
        dat20   = ((0x0000ffff & datafront) << 16 );
    } else if ( iQue == 4 ){
        b1110   = ((0xf0000000 & datafront) >> 28 );
        chipid  = ((0x0fff0000 & datafront) >> 16 );
        dat10   = ((0x0000ffff & datafront) << 16 );
    } else if ( iQue == 5 ){
        b1010   = ((0xf0000000 & datafront) >> 28 );
        b1100   = ((0x0000f000 & datafront) >> 12 );
        bcn     = ((0x0fff0000 & datafront) >> 16 );
        evn     = ((0x00000ff0 & datafront) >>  4 );
        flags   = (0x0000000f & datafront);
    } else if ( iQue == 6 ){
        BX      = datafront;
    }
    dataque.pop();
  }
  INFO(" ::GEMEventMaker after pop dataque.size " << dataque.size() );

  uint64_t data1  = dat10 | dat11;
  uint64_t data2  = dat20 | dat21;
  uint64_t data3  = dat30 | dat31;
  uint64_t data4  = dat40 | dat41;

  if (!(((b1010 == 0xa) && (b1100==0xc) && (b1110==0xe)))) {
    /* do not ignore incorrect data
       WARN("VFAT headers do not match expectation");
       continue;
    */
  }

  vfat_++;

  ECff = evn;
  islot = gem::readout::GEMslotContents::GEBslotIndex( (uint32_t)chipid );

  // GEM Event selector
  ES = ( ECff << 12 ) | bcn;
  DEBUG(" ::getGLIBData vfats ES 0x" << std::hex << ( 0x00ffffff & ES ) << " EC 0x" << ECff << " BC 0x" << bcn << std::dec );

  lsVFAT = (data3 << 32) | (data4);
  msVFAT = (data1 << 32) | (data2);

  vfat.BC     = ( b1010 << 12 ) | (bcn);                // 1010     | bcn:12
  vfat.EC     = ( b1100 << 12 ) | (evn << 4) | (flags); // 1100     | EC:8      | Flag:4
  vfat.ChipID = ( b1110 << 12 ) | (chipid);             // 1110     | ChipID:12
  vfat.lsData = lsVFAT;                                 // lsData:64
  vfat.msData = msVFAT;                                 // msData:64
  vfat.BXfrOH = BX;                                     // BXfrOH:32
  vfat.crc    = vfatcrc;                                // crc:16

  /*
   * dump VFAT data */
   GEMDataAMCformat::printVFATdataBits(vfat_, vfat);
   INFO(" ::getGLIBData slot " << islot );
    
  return point;
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

     // Chamber Header, Zero Suppression flags, Chamber ID
     ZSFlag           = 0x0;                    // :24
     uint64_t ChamID  = 0xdea;                  // :12
     uint64_t sumVFAT = int(geb.vfats.size());  // :28
    
     geb.header  = (ZSFlag << 40)|(ChamID << 28)|(sumVFAT);
    
     ZSFlag =  (0xffffff0000000000 & geb.header) >> 40; 
     ChamID =  (0x000000fff0000000 & geb.header) >> 28; 
     sumVFAT=  (0x000000000fffffff & geb.header);    

     DEBUG(" ::VFATfillData ChamID 0x" << ChamID << std::dec << " islot " << islot << " sumVFAT " << sumVFAT);
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


void gem::readout::GEMDataParker::writeGEMevent(
                                                std::string  outFile,
                                                bool const&  OKprint,
                                                std::string const& TypeDataFlag,
                                                AMCGEMData&  gem,
                                                AMCGEBData&  geb,
                                                AMCVFATData& vfat
){
  if(OKprint){
    DEBUG(" ::writeGEMevent vfat_ " << vfat_ << " event " << event_ << " sumVFAT " << (0x000000000fffffff & geb.header) <<
         " geb.vfats.size " << int(geb.vfats.size()) );
  }

  /*
    int nGEB=0;
    for (vector<GEBData>::iterator iGEB=gem.gebs.begin(); iGEB != gem.gebs.end(); ++iGEB) {
    nGEB++; uint64_t ZSFlag =  (0xffffff0000000000 & geb.header) >> 40; show24bits(ZSFlag);
  */

 /*
  *  GEM Chamber's Data
  */

  if (outputType_ == "Hex") {
    GEMDataAMCformat::writeGEMhd1 (outFile, event_, gem);
    GEMDataAMCformat::writeGEMhd2 (outFile, event_, gem);
    GEMDataAMCformat::writeGEMhd3 (outFile, event_, gem);
  } else {
    GEMDataAMCformat::writeGEMhd1Binary (outFile, event_, gem);
    GEMDataAMCformat::writeGEMhd2Binary (outFile, event_, gem);
    GEMDataAMCformat::writeGEMhd3Binary (outFile, event_, gem);
  } 

 /*
  *  GEB Headers Data
  */

  if (outputType_ == "Hex") {
    GEMDataAMCformat::writeGEBheader (outFile, event_, geb);
    GEMDataAMCformat::writeGEBrunhed (outFile, event_, geb);
  } else {
    GEMDataAMCformat::writeGEBheaderBinary (outFile, event_, geb);
    GEMDataAMCformat::writeGEBrunhedBinary (outFile, event_, geb);
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
      GEMDataAMCformat::writeVFATdata (outFile, nChip, vfat); 
    } else {
      GEMDataAMCformat::writeVFATdataBinary (outFile, nChip, vfat);
    };
    if( OKprint ){
      //GEMDataAMCformat::printVFATdataBits(nChip, vfat);
      DEBUG(" writeGEMevent slot " << gem::readout::GEMslotContents::GEBslotIndex( (uint32_t)vfat.ChipID ) );
    }
  }//end of GEB PayLoad Data

 /*
  *  GEB Trailers Data
  */

  if (outputType_ == "Hex") {
    GEMDataAMCformat::writeGEBtrailer (outFile, event_, geb);
  } else {
    GEMDataAMCformat::writeGEBtrailerBinary (outFile, event_, geb);
  } 

 /*
  *  GEM Trailers Data
  */

  if (outputType_ == "Hex") {
    GEMDataAMCformat::writeGEMtr2 (outFile, event_, gem);
    GEMDataAMCformat::writeGEMtr1 (outFile, event_, gem);
  } else {
    GEMDataAMCformat::writeGEMtr2Binary (outFile, event_, gem);
    GEMDataAMCformat::writeGEMtr1Binary (outFile, event_, gem);
  } 

  uint64_t ZSFlag =  (0xffffff0000000000 & geb.header) >> 40;
  if( OKprint ){
    GEMDataAMCformat::show24bits(ZSFlag); 
    INFO(" ::writeGEMevent:: " << TypeDataFlag << " geb.vfats.size " << int(geb.vfats.size()) << 
         " end of event " << event_ << "\n");
  }
  /* } // end of GEB */
}


void gem::readout::GEMDataParker::GEMfillHeaders(
                                                 uint32_t const& event,
                                                 uint32_t const& BX,
                                                 AMCGEMData& gem,
                                                 AMCGEBData& geb
){
  /*
   *  GEM, All Chamber Data
   */

  // GEM Event Headers [1]
  uint64_t AmcNo       = BOOST_BINARY( 1 );            // :4 
  uint64_t ZeroFlag    = BOOST_BINARY( 0000 );         // :4
  uint64_t LV1ID       = (0x0000000000ffffff & event); // :24
  uint64_t BXID        = (0x00000000ffffffff & BX);    // :12  ! why we have only 12 Bits for BX !
  //uint64_t DataLgth    = BOOST_BINARY( 1 );          // :20

  gem.header1 = (AmcNo <<60)|(ZeroFlag << 56)|(LV1ID << 32)|(BXID); // (BXID << 20)|(DataLgth);

  AmcNo    =  (0xf000000000000000 & gem.header1) >> 60;
  ZeroFlag =  (0x0f00000000000000 & gem.header1) >> 56; 
  LV1ID    =  (0x00ffffff00000000 & gem.header1) >> 32; 
  BXID     =  (0x00000000ffffffff & gem.header1); // >> 20 
  //DataLgth =  (0x00000000000fffff & gem.header1);

  DEBUG(" ::GEMfillHeaders event " << event << " LV1ID " << LV1ID << " BXID " << BXID);

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
