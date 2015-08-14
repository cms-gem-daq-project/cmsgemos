#include "gem/datachecker/GEMDataChecker.h"
//#include "gem/readout/GEMDataAMCformat.h"

#include <boost/utility/binary.hpp>
#include <bitset>

#include <sstream>
#include <vector>

int counter_vfats_ = 0;
int event_counter_ = 0;

// Main constructor
//gem::datachecker::GEMDataChecker::GEMDataChecker(gem::readout::GEMData& gem, gem::readout::GEBData& geb, 
//                                                 gem::readout::VFATData& vfat)
//{
//  event_counter_ = 0;
//}
//
//int gem::datachecker::GEMDataChecker::counterGEMdata(gem::readout::GEMData& gem, gem::readout::GEBData& geb, 
//                                                     gem::readout::VFATData& vfat)
//{
//  return event_counter_;
//}

uint16_t gem::datachecker::GEMDataChecker::checkCRC(uint16_t dataVFAT[11], bool OKprint)
{
  uint16_t crc_fin = 0xffff;
  for (int i = 11; i >= 1; i--)
  {
    crc_fin = this->crc_calc(crc_fin, dataVFAT[i]);
    /*
    if(OKprint){
          cout << " dataVFAT[" << std::setfill('0') 
          << std::setw(2) << i << "] " << hex << std::setfill('0') << std::setw(4) << dataVFAT[i]
               << " crc_temp " << std::setfill('0') << std::setw(4) << crc_fin << dec << endl;
    }
    */
  }
  return(crc_fin);
}

uint16_t gem::datachecker::GEMDataChecker::crc_calc(uint16_t crc_in, uint16_t dato)
{
  uint16_t v = 0x0001;
  uint16_t mask = 0x0001;    
  bool d=0;
  uint16_t crc_temp = crc_in;
  unsigned char datalen = 16;
   
  for (int i=0; i<datalen; i++){
    if (dato & v) d = 1;
    else d = 0;
    if ((crc_temp & mask)^d) crc_temp = crc_temp>>1 ^ 0x8408;
    else crc_temp = crc_temp>>1;
    v<<=1;
  }
  return(crc_temp);
}
