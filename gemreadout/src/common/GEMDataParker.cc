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

#include "TStopwatch.h"

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

uint32_t kUPDATE = 5000, kUPDATE7 = 7;
int event_ = 0;
int rvent_ = 0;

int counterVFATs = 0;
std::map<uint32_t, int> counterVFAT = {{0,0}};
std::map<uint32_t, uint32_t> numES  = {{0,0}};
std::map<uint32_t, uint32_t> errES  = {{0,0}};

uint32_t ESexp = -1; 
bool isFirst = true;
uint64_t ZSFlag = 0;

// The main data flow
std::queue<uint32_t> dataque;

uint32_t contvfats_ = 0;

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
  counter_ = {0,0,0,0,0};
  vfat_ = 0;
  event_ = 0;
  rvent_ = 0;
  sumVFAT_ = 0;

  gem::readout::GEMslotContents::initSlots();
}

uint32_t* gem::readout::GEMDataParker::dumpData(uint8_t const& readout_mask )
{
  uint32_t *point = &counter_[0]; 
  contvfats_ = 0;

  //if [0-7] in deviceNum
  if (readout_mask & 0x1) {
    uint32_t* pDu = gem::readout::GEMDataParker::getGLIBData(0x0, counter_);
    counter_[0] = *(pDu+0);
    counter_[1] = *(pDu+1);
    counter_[2] = *(pDu+2);
    counter_[3] = *(pDu+3);
    counter_[4] = *(pDu+4);
    counter_[5] = *(pDu+5);
    DEBUG(" ::dumpData link0 " << " counter VFATs " << counter_[0] << " , per event counter VFATs " << counter_[2] << 
          " event [1] " << counter_[1] );
  }
  //if [8-15] in deviceNum
  if (readout_mask & 0x2) {
    uint32_t* pDu = gem::readout::GEMDataParker::getGLIBData(0x1, counter_);
    counter_[0] = *(pDu+0);
    counter_[1] = *(pDu+1);
    counter_[2] = *(pDu+2);
    counter_[3] = *(pDu+3);
    counter_[4] = *(pDu+4);
    counter_[5] = *(pDu+5);
  }
  //if [16-23] in deviceNum
  if (readout_mask & 0x4) {
    uint32_t* pDu = gem::readout::GEMDataParker::getGLIBData(0x2, counter_); 
    counter_[0] = *(pDu+0);
    counter_[1] = *(pDu+1);
    counter_[2] = *(pDu+2);
    counter_[3] = *(pDu+3);
    counter_[4] = *(pDu+4);
    counter_[5] = *(pDu+5);
  }

  DEBUG(" ::dumpData link0 " << " counter VFATs " << counter_[0] << " , per event counter VFATs " << counter_[2] << 
	" event [1] " << counter_[1] );

  return point;
}


uint32_t* gem::readout::GEMDataParker::getGLIBData(
						   uint8_t const& link,
						   uint32_t Counter[5]
						   ){
  uint32_t *point = &Counter[0]; 
  TStopwatch timer;

  //timer.Start();
  while ( glibDevice_->hasTrackingData(link) ) {
    std::vector<uint32_t> data;

    // timer.Start();
    data = glibDevice_->getTrackingData(link);
    /*
      timer.Stop(); Float_t RT = (Float_t)timer.RealTime();
      DEBUG(" ::getGLIBData The time for one call of getTrackingData(link) " << RT);
    */

    /*
      DEBUG(" ::getGLIBData numES " << numES.find(ES)->second << " errES " << errES.find(ES)->second << 
      " vfats.size " << vfats.size() << " erros.size " << erros.size() << " ES 0x" << std::hex << ES << std::dec << 
      " event " << Counter[1] );
    */

    uint32_t contqueue = 0;
    for (int iword=0; iword<7; iword++ ){
      contqueue++;
      dataque.push(data.at(iword));
      if (contqueue%kUPDATE7 == 0 &&  contqueue != 0) {
        contvfats_++;
        /*
	  INFO(" ::getGLIBData conter "  << contqueue << " contvfats " << contvfats_ << " dataque.size " << dataque.size() 
        */
      }
    }

    uint32_t* pDQ = gem::readout::GEMDataParker::GEMEventMaker(Counter);
    Counter[0] = *(pDQ+0); // VFAT Blocks counter
    Counter[1] = *(pDQ+1); // Events counter
    Counter[2] = *(pDQ+2); // VFATs per last event  
    Counter[3] = *(pDQ+3); // numES
    Counter[4] = *(pDQ+4); // errES

    DEBUG(" ::getGLIBData VFATs [0] " << Counter[0] << " VFATs per event [2] " << Counter[2] << 
	  " numES [3] " << Counter[3] << " errES [4] " << Counter[4] << " event [1] " << Counter[1] << " event_ " << event_ );

  }// while(glibDevice_->hasTrackingData(link))

  return point;
}


uint32_t* gem::readout::GEMDataParker::selectData(
                                                  uint32_t Counter[5]
						  ){
  uint32_t *point = &Counter[0]; 

  uint32_t* pDQ = gem::readout::GEMDataParker::GEMEventMaker(Counter);
  Counter[0] = *(pDQ+0);
  Counter[1] = *(pDQ+1);
  Counter[2] = *(pDQ+2);
  Counter[3] = *(pDQ+3);
  Counter[4] = *(pDQ+4);
  Counter[5] = *(pDQ+5);

  return point;
}


uint32_t* gem::readout::GEMDataParker::GEMEventMaker(
						     uint32_t Counter[5]
						     ){
  uint32_t *point = &Counter[0];

  int MaxVFATS = 32;
  int MaxERRS  = 4095;
  int MaxEvent = 0;
  int MaxErr   = 0;

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

  DEBUG(" ::GEMEventMaker dataque.size " << dataque.size() );

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
  }// end queue
  DEBUG(" ::GEMEventMaker after pop dataque.size " << dataque.size() );

  uint64_t data1  = dat10 | dat11;
  uint64_t data2  = dat20 | dat21;
  uint64_t data3  = dat30 | dat31;
  uint64_t data4  = dat40 | dat41;

  /*
    if (!(((b1010 == 0xa) && (b1100==0xc) && (b1110==0xe)))) {
    * do not ignore incorrect data
    WARN("VFAT headers do not match expectation");
    continue;
    *
    }*/

  vfat_++;

  ECff = evn;
  islot = gem::readout::GEMslotContents::GEBslotIndex( (uint32_t)chipid );

  // GEM Event selector
  ES = ( ECff << 12 ) | bcn;
  DEBUG(" ::GEMEventMaker ES 0x" << std::hex << ES << std::dec <<
        " vftas.size " << vfats.size() << " erros.size " << erros.size() << " numES " << numES.find(ES)->second << 
        " errES " << errES.find(ES)->second << " isFirst " << isFirst << " event " << event_);

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
   * dump VFAT data 
   GEMDataAMCformat::printVFATdataBits(vfat_, vfat);
   INFO(" ::GEMEventMaker slot " << islot <<"\n");
  */

  if ( ES == ESexp /* ESexp.find(ES)->second */ ) { 
    isFirst = false;
    DEBUG(" ::GEMEventMaker ESexp numES " << numES.find(ES)->second << " errES " << errES.find(ES)->second << 
          " isFirst " << isFirst << " event " << event_);
  } else { 
    isFirst = true;

    if ( vfats.size() != 0 || erros.size() != 0 ){
      numES.erase(ES);
      numES.insert(std::pair<uint32_t, uint32_t>(ES,vfats.size()));
      errES.erase(ES);
      errES.insert(std::pair<uint32_t, uint32_t>(ES,erros.size()));

      DEBUG(" ::GEMEventMaker isFirst vftas.size " << vfats.size() << " erros.size " << erros.size() <<
            " numES " << numES.find(ES)->second << " errES " << errES.find(ES)->second << " isFirst " << isFirst << " event " << event_);

      DEBUG(" ::GEMEventMaker isFirst GEMevSelector ");
      gem::readout::GEMDataParker::GEMevSelector(ES, MaxEvent, MaxErr);
    } 

    event_++;

    // VFATS dimensions have limits
    vfats.reserve(MaxVFATS);
    erros.reserve(MaxERRS);

    ESexp = ES;

    counterVFATs = 0;
    counterVFAT.erase(ES);
    counterVFAT.insert(std::pair<uint32_t, int>(ES,0));

    numES.erase(ES);
    numES.insert(std::pair<uint32_t, uint32_t>(ES,0));
    errES.erase(ES);
    errES.insert(std::pair<uint32_t, uint32_t>(ES,0));

    ZSFlag = 0;
  }

  DEBUG(" ::GEMEventMaker ES " << std::hex << ES << std::dec << " bool " << isFirst );
  counterVFATs++;
  counterVFAT.erase(ES);
  counterVFAT.insert(std::pair<uint32_t, int>(ES,counterVFATs));

  std::map<uint32_t, uint32_t>::iterator it;// pay load events 
  std::map<uint32_t, uint32_t>::iterator ir;// errors events
  if (islot<0 || islot > 23) {
    ir=errES.find(ES);
    if (ir != errES.end()){
      // local event calculator inside one buffer, ES based 
      MaxErr = errES.find(ES)->second;
      MaxErr++;
      errES.erase(ES);
      errES.insert(std::pair<uint32_t, uint32_t>(ES,MaxErr));
      DEBUG(" ::GEMEventMaker Err ES 0x" << std::hex << ES << std::dec << " errES " <<  errES.find(ES)->second << 
            " rvent_ " << rvent_ );
    }
    // islot out of [0-23]
    if ( int(erros.size()) <MaxERRS ) erros.push_back(vfat);
    DEBUG(" ::GEMEventMaker warning !!! islot is undefined " << islot << " erros.size " << int(erros.size()) );
    /*
     * dump VFAT data 
     GEMDataAMCformat::printVFATdataBits(vfat_, vfat);
     INFO(" ::GEMEventMaker wrong slot " << islot <<"\n");
    */

  } else {
    it=numES.find(ES);
    if (it != numES.end()){
      // local event calculator inside one buffer, ES based 
      MaxEvent = numES.find(ES)->second;
      MaxEvent++;
      numES.erase(ES);
      numES.insert(std::pair<uint32_t, uint32_t>(ES,MaxEvent));
      DEBUG(" ::GEMEventMaker ES 0x" << std::hex << ES << std::dec << " numES " <<  numES.find(ES)->second << 
            " MaxEvent " << MaxEvent <<" event_ " << event_ );
      /*
       * dump VFAT data 
       GEMDataAMCformat::printVFATdataBits(vfat_, vfat);
       DEBUG(" ::GEMEventMaker payload slot " << islot <<"\n");
      */

    }
    /*
     * VFATs Pay Load
     */
    if ( int(vfats.size()) <= MaxVFATS ) vfats.push_back(vfat);
    DEBUG(" ::GEMEventMaker event_ " << event_ << " vfats.size " << vfats.size() << std::hex << " ES 0x" << ES << std::dec );

  }//end of event selection 

  DEBUG(" ::GEMEventMaker END numES " << numES.find(ES)->second << " errES " << errES.find(ES)->second << 
	" vfats.size " << vfats.size() << " erros.size " << erros.size() << " event_ " << event_ );

  Counter[0] = vfat_;
  Counter[1] = event_;
  Counter[2] = numES.find(ES)->second + errES.find(ES)->second;
  Counter[3] = numES.find(ES)->second;
  Counter[4] = errES.find(ES)->second;

  return point;
}

void gem::readout::GEMDataParker::GEMevSelector(const  uint32_t& ES,
                                                int MaxEvent,
                                                int MaxErr
                                                ){

  /*  
   *  GEM Event Data Format definition
   */
  AMCGEMData  gem; 
  AMCGEBData  geb;
  AMCVFATData vfat;

  if ( numES.find(ES)->second != 0 || errES.find(ES)->second != 0 ){
    DEBUG(" ::GEMEventMaker vfats.size " << int(vfats.size()) << " rvent_ " << rvent_ << " event " << event_);
 
    uint32_t locEvent = 0;
    uint32_t locError = 0;
    std::string TypeDataFlag = "PayLoad";
 
    // contents all local events (one buffer, all links):
    for (std::map<uint32_t, uint32_t>::iterator itES=numES.begin(); itES!=numES.end(); ++itES){
      //event_++;
      locEvent++;
      DEBUG(" ::GEMEventMaker END ES 0x" << std::hex << itES->first << std::dec << " numES " <<  itES->second << 
            " locEvent " << locEvent << " event_ " << event_ );
 
      uint32_t nChip = 0;
      for (std::vector<GEMDataAMCformat::VFATData>::iterator iVFAT=vfats.begin(); iVFAT != vfats.end(); ++iVFAT) {
 
        uint8_t ECff = ( (0x0ff0 & (*iVFAT).EC ) >> 4);
        uint32_t localEvent = ( ECff << 12 ) | ( 0x0fff & (*iVFAT).BC );

        DEBUG(" ::GEMEventMaker vfats ES 0x" << std::hex << ( 0x00ffffff & itES->first ) << " and from vfat 0x" << 
              ( 0x00ffffff & localEvent ) << " EC 0x" << ECff << " BC 0x" << ( 0x0fff & (*iVFAT).BC ) << std::dec );
 
        if ( itES->first == localEvent ) {
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
          if (islot<0 || islot > 23) { 
            INFO(" ::GEMEventMaker  coutld be error&warning slot " << islot);
          }
 
          if ( gem::readout::GEMDataParker::VFATfillData( islot, geb) ){
            if ( itES->second == nChip ){
 
              gem::readout::GEMDataParker::GEMfillHeaders(event_, itES->second, gem, geb);
              gem::readout::GEMDataParker::GEMfillTrailers(gem, geb);
         
              /*
               * GEM Event Writing
               */
              DEBUG(" ::GEMEventMaker writing...  geb.vfats.size " << int(geb.vfats.size()) );
              TypeDataFlag = "PayLoad";
              if(int(geb.vfats.size()) != 0) gem::readout::GEMDataParker::writeGEMevent(outFileName_, true, TypeDataFlag,
                                                                                        gem, geb, vfat);
              geb.vfats.clear();
         
            }//end of writing event
          }// if slot correct
        }// if localEvent
      }//end of GEB PayLoad Data
    }// end itES
 
    geb.vfats.clear();
    TypeDataFlag = "Errors";

    // contents all local events (one buffer, all links):
    for (std::map<uint32_t, uint32_t>::iterator irES=errES.begin(); irES!=errES.end(); ++irES){
      //event_++;
      DEBUG(" ::GEMEventMaker END ES 0x" << std::hex << irES->first << std::dec << " errES " <<  irES->second << 
            " rvent_ " << rvent_ );
 
      uint32_t nErro = 0;
      for (std::vector<GEMDataAMCformat::VFATData>::iterator iErr=erros.begin(); iErr != erros.end(); ++iErr) {
 
        uint8_t ECff = ( (0x0ff0 & (*iErr).EC ) >> 4);
        uint32_t localErr = ( ECff << 12 ) | ( 0x0fff & (*iErr).BC );
        DEBUG(" ::GEMEventMaker ERROR vfats ES 0x" << irES->first << " EC " << localErr );
  
        if( irES->first == localErr ) {
          nErro++;
          locError = nErro;
          vfat.BC     = (*iErr).BC;
          vfat.EC     = (*iErr).EC;
          vfat.ChipID = (*iErr).ChipID;
          vfat.lsData = (*iErr).lsData;
          vfat.msData = (*iErr).msData;
          vfat.BXfrOH = (*iErr).BXfrOH;
          vfat.crc    = (*iErr).crc;
                
          DEBUG(" ::GEMEventMaker " << " nErro " << nErro << " ES 0x" << std::hex << irES->first << std::dec );
   
          /*
           * VFATs Errors
           */
          geb.vfats.push_back(vfat);
          if ( irES->second == nErro ){
            /*
              GEMDataAMCformat::printVFATdataBits(nErro, vfat);
            */
            int islot = -1;
            gem::readout::GEMDataParker::VFATfillData( islot, geb);
            gem::readout::GEMDataParker::GEMfillHeaders(rvent_, irES->second, gem, geb);
            gem::readout::GEMDataParker::GEMfillTrailers(gem, geb);
              
            /*
             * GEM ERRORS Event Writing
             */
            TypeDataFlag = "Errors";
            if(int(geb.vfats.size()) != 0) gem::readout::GEMDataParker::writeGEMevent(errFileName_, false, TypeDataFlag,
                                                                                      gem, geb, vfat);
            geb.vfats.clear();
          }// if localErr
        }// if localErr
      }//end of GEB PayLoad Data
    }//end irES
 
    geb.vfats.clear();
      
    if (event_%kUPDATE == 0 &&  event_ != 0) {
      INFO(" ::GEMEventMaker vfats.size " << std::setfill(' ') << std::setw(7) << int(vfats.size()) <<
           " erros.size " << std::setfill(' ') << std::setw(3) << int(erros.size()) << 
           " locEvent   " << std::setfill(' ') << std::setw(6) << locEvent << 
           " locError   " << std::setfill(' ') << std::setw(3) << locError << " event " << event_
           );
    }

    locEvent = 0;
 
    vfats.clear();
    erros.clear();

    // local event cleaning 
    numES.clear();
    errES.clear();
    MaxEvent = 0;
    MaxErr   = 0;
 
    // reset event logic
    isFirst = true;
    counterVFAT.clear();
    ZSFlag = 0;
  }// end of writing all events for slected ES
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
    INFO(" ::writeGEMevent " << TypeDataFlag << " geb.vfats.size " << int(geb.vfats.size()) << 
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
