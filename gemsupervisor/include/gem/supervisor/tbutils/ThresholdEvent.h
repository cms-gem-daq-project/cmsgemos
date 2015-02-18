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

      struct ChannelData {
        uint64_t lsData;  // ch1to64
        uint64_t msData;  // ch65to128
        double    delVT;  // deviceVT2-deviceVT1, Threshold Scan needs
      };
    
      struct VFATEvent {
        uint16_t BC;      // 1010:4, BC:12 
        uint16_t EC;      // 1100:4, EC:8, Flags:4
        uint32_t bxExp;   // :28
        uint16_t bxNum;   // :6, Sbit:6
        uint16_t ChipID;  // 1110, :12
        ChannelData data;
        uint16_t crc;     // :16
      };    
    
      struct GEMEvent {
        uint32_t header1;
        std::vector<VFATEvent> vfats;
        uint32_t trailer1;
      };

      struct AppHeader {  
        int minTh;
        int maxTh;
        int stepSize;
      };

      bool keepData(string file, int event, const ChannelData& ch){
        ofstream outf(file.c_str(), ios_base::app );
        if( event<0) return(false);
        if(!outf.is_open()) return(false);
          outf << hex << ch.lsData << dec << endl;
          outf << hex << ch.msData << dec << endl;
          outf << ch.delVT << endl;
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

      bool keepAppHeader(string file, const AppHeader& ah){
        ofstream outf(file.c_str(), ios_base::app );
        if(!outf.is_open()) return(false);
          outf << ah.minTh << endl;
          outf << ah.maxTh << endl;
          outf << ah.stepSize << endl;
          outf.close();
        return(true);
      };	  

    } //end namespace gem::supervisor::tbutils
  } //end namespace gem::supervisor
} //end namespace gem
#endif
