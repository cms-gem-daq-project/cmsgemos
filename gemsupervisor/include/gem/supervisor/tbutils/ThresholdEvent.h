#ifndef GEM_SUPERVISOR_TBUTILS_THRESHOLDEVENT_H
#define GEM_SUPERVISOR_TBUTILS_THRESHOLDEVENT_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <bitset>

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

      bool keepData(std::string file, int event, const ChannelData& ch){
        std::ofstream outf(file.c_str(), std::ios_base::app );
        if( event<0) return(false);
        if(!outf.is_open()) return(false);
        outf << std::hex << ch.lsData << std::dec << std::endl;
        outf << std::hex << ch.msData << std::dec << std::endl;
        outf << ch.delVT << std::endl;
        outf.close();
        return(true);
      };

      bool keepEvent(std::string file, int event, const VFATEvent& ev, const ChannelData& ch){
        std::ofstream outf(file.c_str(), std::ios_base::app );
        if( event<0) return(false);
        if(!outf.is_open()) return(false);
        outf << std::hex << ev.BC << std::dec << std::endl;
        outf << std::hex << ev.EC << std::dec << std::endl;
        outf << std::hex << ev.bxExp << std::dec << std::endl;
        outf << std::hex << ev.bxNum << std::dec << std::endl;
        outf << std::hex << ev.ChipID << std::dec << std::endl;
        keepData (file, event, ch);
        outf << std::hex << ev.crc << std::dec << std::endl;
        outf.close();
        return(true);
      };

      bool keepAppHeader(std::string file, const AppHeader& ah){
        std::ofstream outf(file.c_str(), std::ios_base::app );
        if(!outf.is_open()) return(false);
        outf << ah.minTh << std::endl;
        outf << ah.maxTh << std::endl;
        outf << ah.stepSize << std::endl;
        outf.close();
        return(true);
      };

    }  // namespace gem::supervisor::tbutils
  }  // namespace gem::supervisor
}  // namespace gem

#endif  // GEM_SUPERVISOR_TBUTILS_THRESHOLDEVENT_H
