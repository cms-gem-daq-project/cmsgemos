#ifndef gem_supervisor_GEMDataAMCformat_h
#define gem_supervisor_GEMDataAMCformat_h

#include <iostream>
#include <iomanip> 
#include <fstream>
#include <string>
#include <vector>

using namespace std;

namespace gem {
  namespace supervisor {

      struct ChannelData {
        uint64_t lsData;      // channels from 1to64
        uint64_t msData;      // channels from 65to128
      };
    
      struct VFATData {
        uint16_t BC;          // 1010:4,   BC:12 
        uint16_t EC;          // 1100:4,   EC:8,     Flags:4
        uint32_t bxExp;       // bxExp:28
        uint16_t bxNum;       // bxNum:8,  Sbit:8
        uint16_t ChipID;      // 1110,     ChipID:12
          ChannelData data;
        uint16_t crc;         // :16
      };    
    
      struct GEMData {
        uint64_t header;      // ZSFlag:24 ChamID:12 
        std::vector<VFATData> vfats;
        uint64_t trailer;     // OHcrc: 16 OHwCount:16  ChamStatus:16
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

      bool keepChannelDataBinary(string file, int event, const ChannelData& ch){
        ofstream outf(file.c_str(), ios_base::app | ios::binary );
        if(!outf.is_open()) return(false);
        if( event<0) return(false);
          outf.write( (char*)&ch.lsData, sizeof(ch.lsData));
	  outf.write( (char*)&ch.msData, sizeof(ch.msData));
          outf.close();
        return(true);
      };

      bool readChannelDataBinary(string file, int event, const ChannelData& ch){
        ifstream inpf(file.c_str(), ios::binary );
        if(!inpf.is_open()) return(false);
        if( event<0) return(false);
          inpf.read( (char*)&ch.lsData, sizeof(ch.lsData));
	  inpf.read( (char*)&ch.msData, sizeof(ch.msData));
          inpf.seekg (0, inpf.cur);
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

      bool keepVFATDataBinary(string file, int event, const VFATData& ev, const ChannelData& ch){
        ofstream outf(file.c_str(), ios_base::app | ios::binary );
        if( event<0) return(false);
        if(!outf.is_open()) return(false);
  	  outf.write( (char*)&ev.BC, sizeof(ev.BC));
  	  outf.write( (char*)&ev.EC, sizeof(ev.EC));
  	  outf.write( (char*)&ev.bxExp, sizeof(ev.bxExp));
  	  outf.write( (char*)&ev.bxNum, sizeof(ev.bxNum));
  	  outf.write( (char*)&ev.ChipID, sizeof(ev.ChipID));
            keepChannelDataBinary (file, event, ch);
  	  outf.write( (char*)&ev.crc, sizeof(ev.crc));
          outf.close();
        return(true);
      };	  

      bool readVFATDataBinary(string file, int event, const VFATData& ev, const ChannelData& ch){
        ifstream inpf(file.c_str(), ios_base::app | ios::binary );
        if( event<0) return(false);
        if(!inpf.is_open()) return(false);
  	  inpf.read( (char*)&ev.BC, sizeof(ev.BC));
  	  inpf.read( (char*)&ev.EC, sizeof(ev.EC));
  	  inpf.read( (char*)&ev.bxExp, sizeof(ev.bxExp));
  	  inpf.read( (char*)&ev.bxNum, sizeof(ev.bxNum));
  	  inpf.read( (char*)&ev.ChipID, sizeof(ev.ChipID));
            readChannelDataBinary (file, event, ch);
  	  inpf.read( (char*)&ev.crc, sizeof(ev.crc));
          inpf.seekg (0, inpf.cur);
        return(true);
      };	  

      //
      // Useful printouts 
      //
      void show4bits(uint8_t x) {
        int i;
        const unsigned long unit = 1;
        for(i=(sizeof(uint8_t)*4)-1; i>=0; i--)
          (x & ((unit)<<i))?putchar('1'):putchar('0');
     	//printf("\n");
      }

      void show16bits(uint16_t x) {
        int i;
        const unsigned long unit = 1;
        for(i=(sizeof(uint16_t)*8)-1; i>=0; i--)
          (x & ((unit)<<i))?putchar('1'):putchar('0');
     	printf("\n");
      }

      void show32bits(uint32_t x) {
        int i;
        const unsigned long unit = 1;
        for(i=(sizeof(uint32_t)*8)-1; i>=0; i--)
         (x & ((unit)<<i))?putchar('1'):putchar('0');
     	printf("\n");
      }

      void show64bits(uint64_t x) {
        int i;
        const unsigned long unit = 1;
        for(i=(sizeof(uint64_t)*8)-1; i>=0; i--)
          (x & ((unit)<<i))?putchar('1'):putchar('0');
     	printf("\n");
      }

      bool PrintVFATDataBits(int event, const VFATData& ev, const ChannelData& ch){
        if( event<0) return(false);
 	  cout << "\nReceived VFAT data word:" << endl;

          uint8_t   b1010 = (0xf000 & ev.BC) >> 12;
          show4bits(b1010); cout << " BC     0x" << hex << (0x0fff & ev.BC) << dec << endl;

          uint8_t   b1100 = (0xf000 & ev.EC) >> 12;
          uint16_t   EC   = (0x0ff0 & ev.EC) >> 4;
          uint8_t   Flag  = (0x000f & ev.EC);
          show4bits(b1100); cout << " EC     0x" << hex << EC << dec << endl; 
          show4bits(Flag);  cout << " Flags " << endl;

          uint8_t   b1110 = (0xf000 & ev.ChipID) >> 12;
          uint16_t ChipID = (0x0fff & ev.ChipID);
          show4bits(b1110); cout << " ChipID 0x" << hex << ChipID << dec << " " << endl;

          cout << "     bxExp  0x" << hex << ev.bxExp << dec << " " << endl;
	  cout << "     bxNum  0x" << hex << ((0xff00 & ev.bxNum) >> 8) << "        SBit " << (0x00ff & ev.bxNum) << endl;
          cout << " <127:64>:: 0x" << std::setfill('0') << std::setw(8) << hex << ch.msData << dec << endl;
          cout << " <63:0>  :: 0x" << std::setfill('0') << std::setw(8) << hex << ch.lsData << dec << endl;
          cout << "     crc    0x" << hex << ev.crc << dec << endl;

          //cout << " " << endl; show16bits(ev.EC);

        return(true);
      };

  } //end namespace gem::supervisor
} //end namespace gem
#endif
