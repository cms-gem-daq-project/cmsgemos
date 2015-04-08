#ifndef gem_supervisor_GEMDataAMCformat_h
#define gem_supervisor_GEMDataAMCformat_h

#include <iostream>
#include <iomanip> 

#include <fstream>
#include <string>
#include <vector>
#include <bitset>

using namespace std;

namespace gem {
  namespace supervisor {

      struct ChannelData {
        uint64_t lsData;  // ch1to64
        uint64_t msData;  // ch65to128
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

      bool keepData(string file, int event, const ChannelData& ch){
        ofstream outf(file.c_str(), ios_base::app | ios::binary );
        if( event<0) return(false);
        if(!outf.is_open()) return(false);
          outf << hex << ch.lsData << dec << endl;
          outf << hex << ch.msData << dec << endl;
          outf.close();
        return(true);
      };	  

      bool PrintVFATData(int event, const ChannelData& ch){
        if( event<0) return(false);
 	  cout << "data words:" << endl;
	  cout << "<127:64>:: 0x" << std::setfill('0') << std::setw(8) << hex << ch.msData << dec << endl;
	  cout << "<63:0>  :: 0x" << std::setfill('0') << std::setw(8) << hex << ch.lsData << dec << endl;
        return(true);
      };

      bool keepEvent(string file, int event, const VFATEvent& ev, const ChannelData& ch){
        ofstream outf(file.c_str(), ios_base::app | ios::binary );
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

      bool PrintGEMData(int event, const VFATEvent& ev, const ChannelData& ch){
        if( event<0) return(false);
 	  cout << "Received tracking data word:" << endl;
	  cout << "bxn     :: 0x" << std::setfill('0') << std::setw(4) << hex << ev.bxNum << dec << endl;
	  cout << "bcn     :: 0x" << std::setfill('0') << std::setw(4) << hex << ev.BC    << dec << endl;
	  cout << "evn     :: 0x" << std::setfill('0') << std::setw(4) << hex << ev.EC    << dec << endl;
	  //cout << "flags   :: 0x" << std::setfill('0') << std::setw(2) << hex << (unsigned)flags  << dec << endl;
	  cout << "chipid  :: 0x" << std::setfill('0') << std::setw(4) << hex << ev.ChipID << dec << endl;
          //cout << "<127:0> :: 0x" << std::setfill('0') << std::setw(8) << hex << ch.msData << dec 
          //                        << std::setfill('0') << std::setw(8) << hex << ch.lsData << dec << endl;
          cout << "<127:64>:: 0x" << std::setfill('0') << std::setw(8) << hex << ch.msData << dec << endl;
          cout << "<63:0>  :: 0x" << std::setfill('0') << std::setw(8) << hex << ch.lsData << dec << endl;
	  cout << "crc     :: 0x" << std::setfill('0') << std::setw(4) << hex << ev.crc    << dec << endl;
        return(true);
      };

  } //end namespace gem::supervisor
} //end namespace gem
#endif
