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

      void show4bits(uint8_t x) {
        int i; 
        for(i=(sizeof(uint8_t)*4)-1; i>=0; i--)
          (x&(1<<i))?putchar('1'):putchar('0');
     	printf("\n");
      }

      void show16bits(uint16_t x) {
        int i; 
        for(i=(sizeof(uint16_t)*8)-1; i>=0; i--)
          (x&(1<<i))?putchar('1'):putchar('0');
     	printf("\n");
      }

      struct ChannelData {
        uint64_t lsData;  // ch1to64
        uint64_t msData;  // ch65to128
      };
    
      struct VFATData {
        uint16_t BC;      // 1010:4,   BC:12 
        uint16_t EC;      // 1100:4,   EC:8,     Flags:4
        uint32_t bxExp;   // bxExp:28
        uint16_t bxNum;   // bxNum:6,  Sbit:6
        uint16_t ChipID;  // 1110,     ChipID:12
          ChannelData data;
        uint16_t crc;     // :16
      };    
    
      struct GEMData {
        uint32_t header1;
        std::vector<VFATData> vfats;
        uint32_t trailer1;
      };

      bool keepChannelData(string file, int event, const ChannelData& ch){
        ofstream outf(file.c_str(), ios_base::app | ios::binary );
        if( event<0) return(false);
        if(!outf.is_open()) return(false);
          outf << ch.lsData << endl;
          outf << ch.msData << endl;
          outf.close();
        return(true);
      };	  

      bool PrintChannelData(int event, const ChannelData& ch){
        if( event<0) return(false);
 	  cout << "data words:" << endl;
	  cout << "<127:64>:: 0x" << std::setfill('0') << std::setw(8) << hex << ch.msData << dec << endl;
	  cout << "<63:0>  :: 0x" << std::setfill('0') << std::setw(8) << hex << ch.lsData << dec << endl;
        return(true);
      };

      bool keepVFATData(string file, int event, const VFATData& ev, const ChannelData& ch){
        ofstream outf(file.c_str(), ios_base::app | ios::binary );
        if( event<0) return(false);
        if(!outf.is_open()) return(false);
          outf << ev.BC << endl;
          outf << ev.EC << endl;
          outf << ev.bxExp << endl;
          outf << ev.bxNum << endl;
          outf << ev.ChipID << endl;
            keepChannelData (file, event, ch);
          outf << ev.crc << endl;
          outf.close();
        return(true);
      };	  

      bool PrintVFATData(int event, const VFATData& ev, const ChannelData& ch){
        if( event<0) return(false);
 	  cout << "Received tracking data word:" << endl;
	  cout << "BC      :: 0x" << std::setfill('0') << std::setw(4) << hex << ev.BC     << dec << endl;
	  cout << "EC      :: 0x" << std::setfill('0') << std::setw(4) << hex << ev.EC     << dec << endl;
	  cout << "ChipID  :: 0x" << std::setfill('0') << std::setw(4) << hex << ev.ChipID << dec << endl;
	  cout << "bxExp   :: 0x" << std::setfill('0') << std::setw(6) << hex << ev.bxExp  << dec << endl;
	  cout << "bxNum   :: 0x" << std::setfill('0') << std::setw(4) << hex << ev.bxNum  << dec << endl;
          cout << "<127:64>:: 0x" << std::setfill('0') << std::setw(8) << hex << ch.msData << dec << endl;
          cout << "<63:0>  :: 0x" << std::setfill('0') << std::setw(8) << hex << ch.lsData << dec << endl;
	  cout << "crc     :: 0x" << std::setfill('0') << std::setw(4) << hex << ev.crc    << dec << endl;
        return(true);
      };

      bool PrintVFATDataBits(int event, const VFATData& ev, const ChannelData& ch){
        if( event<0) return(false);
 	  cout << " print:  EC" << endl;

 	  uint8_t b1100 = (0xf000 & ev.EC) >> 12;
          uint8_t EC    = (0x0ff0 & ev.EC) >> 4;
          uint8_t Flags = (0x000f & ev.EC);
          show16bits(ev.EC);

	  /*
          ev.BC     = ( ((data.at(5) & 0xF0000000)>>28) << 12 ) | (bcn);                // 1010     | bcn:12
          ev.EC     = ( ((data.at(5) & 0x0000F000)>>12) << 12 ) | (evn << 4) | (flags); // 1100     | EC:8      | Flag:4 (zero?)
          ev.ChipID = ( ((data.at(4) & 0xF0000000)>>28) << 12 ) | (chipid);             // 1110     | ChipID:12
          ev.bxExp  = bxExp;                                                            // bxExp:28
          ev.bxNum  = (bxNum << 6 ) | (SBit);                                           // bxNum:6  | SBit:6
          ch.lsData = lsData;                                                           // lsData:64
          ch.msData = msData;                                                           // msData:64
          ev.crc    = 0x0000ffff & data.at(0);                                          // crc:16
	  */
        return(true);
      };

  } //end namespace gem::supervisor
} //end namespace gem
#endif
