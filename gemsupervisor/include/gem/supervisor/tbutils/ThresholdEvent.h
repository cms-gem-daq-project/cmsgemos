#ifndef gem_supervisor_tbutils_ThresholdEvent_h
#define gem_supervisor_tbutils_ThresholdEvent_h

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <bitset>

using namespace std;

namespace gem {
  namespace supervisor {
    namespace tbutils {

      typedef struct {
        uint64_t lsdata;  // ch1to64
        uint64_t msdata;  // ch65to128
      } ChannelData;
    
      typedef struct {
        uint16_t BC;      // 1010:4, BC:12 
        uint16_t EC;      // 1100:4, EC:8, Flags:4
        uint32_t bxExp;   // :28
        uint16_t bxNum;   // :6, Sbit:6
        uint16_t ChipID;  // 1110, :12
        ChannelData data;
        uint16_t crc;     // :16
      } VFATEvent;    
    
      typedef struct  {
        uint32_t header1;
        std::vector<VFATEvent> vfats;
        uint32_t trailer1;
      } GEMEvent;

      bool keepData(string file, int event, const ChannelData& ch){
        ofstream outf(file.c_str(), ios_base::app );
        if( event<0) return(false);
        if(!outf.is_open()) return(false);
          outf << hex << ch.lsdata << dec << endl;
          outf << hex << ch.msdata << dec << endl;
          outf.close();
        return(true);
      };	  

      bool keepEvent(string file, int event, const VFATEvent& ev, const ChannelData& ch){
        ofstream outf(file.c_str(), ios_base::app );
        if( event<0) return(false);
        if(!outf.is_open()) return(false);
          outf << hex << ev.BC << dec << endl;
          outf << hex << ev.EC << dec << endl;
          outf << hex << ev.bxExp << dec << endl;
          outf << hex << ev.bxNum << dec << endl;
          outf << hex << ev.ChipID << dec << endl;
          keepData (file, event, ch);
          outf << hex << ev.crc << dec << endl;
          outf.close();
        return(true);
      };	  

    } //end namespace gem::supervisor::tbutils
  } //end namespace gem::supervisor
} //end namespace gem
#endif
