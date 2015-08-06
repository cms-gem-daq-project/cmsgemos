#ifndef gem_readout_GEMDataAMCformat_h
#define gem_readout_GEMDataAMCformat_h

#include <iostream>
#include <iomanip> 
#include <fstream>
#include <string>
#include <vector>

namespace gem {
  namespace readout {

    struct VFATData {
      uint16_t BC;          // 1010:4,   BC:12 
      uint16_t EC;          // 1100:4,   EC:8,      Flags:4
      uint16_t ChipID;      // 1110,     ChipID:12
      uint64_t lsData;      // channels from 1to64
      uint64_t msData;      // channels from 65to128
      uint16_t BXfrOH;      // :16       BX from OH  
      uint16_t crc;         // :16       CRC
    };    
    
    struct GEBData {
      uint64_t header;      // ZSFlag:24 ChamID:12 
      std::vector<VFATData> vfats;
      uint64_t trailer;     // OHcrc: 16 OHwCount:16  ChamStatus:16
    };

    struct GEMData {
      uint64_t header1;      // AmcNo:4      0000:4     LV1ID:24   BXID:12     DataLgth:20 
      uint64_t header2;      // User:32      OrN:16     BoardID:16
      uint64_t header3;      // DAVList:24   BufStat:24 DAVCount:5 FormatVer:3 MP7BordStat:8 
      std::vector<GEBData> gebs;
      uint64_t trailer2;     // EventStat:32 GEBerrFlag:24  
      uint64_t trailer1;     // crc:32       LV1IDT:8   0000:4     DataLgth:20 
    };

    /*
     *  GEB Data Format
     *    geb.header
     *    vfat vector
     *    geb.trailer
     *
     */

    bool writeGEBheader(std::string file, int event, const GEBData& geb) {
      std::ofstream outf(file.c_str(), std::ios_base::app );
      if ( event<0) return(false);
      if (!outf.is_open()) return(false);
      outf << std::hex << geb.header << std::dec << std::endl;
      outf.close();
      return(true);
    };	  

    bool readGEBheader(std::ifstream& inpf, GEBData& geb) {
      inpf >> std::hex >> geb.header;
      return(true);
    };	  

    bool printGEBheader(int event, const GEBData& geb) {
      if ( event<0) return(false);
      std::cout << "Received tracking data word: event " << event << std::endl;
      std::cout << " 0x" << std::setw(8) << std::hex << geb.header << " ChamID " << ((0x000000fff0000000 & geb.header) >> 28) 
                << std::dec << " sumVFAT " << (0x000000000fffffff & geb.header) << std::endl;
      return(true);
    };	  

    bool writeZEROline(std::string file) {
      std::ofstream outf(file.c_str(), std::ios_base::app );
      if (!outf.is_open()) return(false);
      outf << "\n" << std::endl;
      outf.close();
      return(true);
    };	  

    bool writeGEBtrailer(std::string file, int event, const GEBData& geb) {
      std::ofstream outf(file.c_str(), std::ios_base::app );
      if ( event<0) return(false);
      if (!outf.is_open()) return(false);
      outf << std::hex << geb.trailer << std::dec << std::endl;
      outf.close();
      return(true);
    };	  

    bool readGEBtrailer(std::ifstream& inpf, GEBData& geb) {
      inpf >> std::hex >> geb.trailer;
      return(true);
    };	  

    bool printGEBtrailer(int event, const GEBData& geb) {
      if ( event<0) return(false);
      uint64_t OHcrc      = (0xffff000000000000 & geb.trailer) >> 48; 
      uint64_t OHwCount   = (0x0000ffff00000000 & geb.trailer) >> 32; 
      uint64_t ChamStatus = (0x00000000ffff0000 & geb.trailer) >> 16;
      std::cout << "GEM Camber Treiler: OHcrc " << std::hex << OHcrc << " OHwCount " << OHwCount << " ChamStatus " << ChamStatus << std::dec 
                << std::endl;
      return(true);
    };	  

    bool writeVFATdata(std::string file, int event, const VFATData& vfat) {
      std::ofstream outf(file.c_str(), std::ios_base::app );
      if ( event<0) return(false);
      if (!outf.is_open()) return(false);
      outf << std::hex << vfat.BC << std::dec << std::endl;
      outf << std::hex << vfat.EC << std::dec << std::endl;
      outf << std::hex << vfat.ChipID << std::dec << std::endl;
      outf << std::hex << vfat.lsData << std::dec << std::endl;
      outf << std::hex << vfat.msData << std::dec << std::endl;
      outf << std::hex << vfat.BXfrOH << std::dec << std::endl;
      outf << std::hex << vfat.crc << std::dec << std::endl;
      //writeZEROline(file);
      outf.close();
      return(true);
    };	  

    bool printVFATdata(int event, const VFATData& vfat) {
      if ( event<0) return(false);
      std::cout << "Received tracking data word:" << std::endl;
      std::cout << "BC      :: 0x" << std::setfill('0') << std::setw(4) << std::hex << vfat.BC     << std::dec << std::endl;
      std::cout << "EC      :: 0x" << std::setfill('0') << std::setw(4) << std::hex << vfat.EC     << std::dec << std::endl;
      std::cout << "ChipID  :: 0x" << std::setfill('0') << std::setw(4) << std::hex << vfat.ChipID << std::dec << std::endl;
      std::cout << "<127:64>:: 0x" << std::setfill('0') << std::setw(8) << std::hex << vfat.msData << std::dec << std::endl;
      std::cout << "<63:0>  :: 0x" << std::setfill('0') << std::setw(8) << std::hex << vfat.lsData << std::dec << std::endl;
      std::cout << "BXfrOH  :: 0x" << std::setfill('0') << std::setw(4) << std::hex << vfat.BXfrOH << std::dec << std::endl;
      std::cout << "crc     :: 0x" << std::setfill('0') << std::setw(4) << std::hex << vfat.crc    << std::dec << std::endl;
      return(true);
    };

    bool readVFATdata(std::ifstream& inpf, int event, VFATData& vfat) {
      if (event<0) return(false);
      inpf >> std::hex >> vfat.BC;
      inpf >> std::hex >> vfat.EC;
      inpf >> std::hex >> vfat.ChipID;
      inpf >> std::hex >> vfat.lsData;
      inpf >> std::hex >> vfat.msData;
      inpf >> std::hex >> vfat.BXfrOH;
      inpf >> std::hex >> vfat.crc;
      return(true);
    };	  

    bool writeGEBheaderBinary(std::string file, int event, const GEBData& geb) {
      std::ofstream outf(file.c_str(), std::ios_base::app | std::ios::binary );
      if ( event<0) return(false);
      if (!outf.is_open()) return(false);
      outf.write( (char*)&geb.header, sizeof(geb.header));
      outf.close();
      return(true);
    };
	  
    bool writeGEBtrailerBinary(std::string file, int event, const GEBData& geb) {
      std::ofstream outf(file.c_str(), std::ios_base::app | std::ios::binary );
      if ( event<0) return(false);
      if (!outf.is_open()) return(false);
      outf.write( (char*)&geb.trailer, sizeof(geb.trailer));
      outf.close();
      return(true);
    };

    bool writeVFATdataBinary(std::string file, int event, const VFATData& vfat) {
      std::ofstream outf(file.c_str(), std::ios_base::app | std::ios::binary );
      if ( event<0) return(false);
      if (!outf.is_open()) return(false);
      outf.write( (char*)&vfat.BC, sizeof(vfat.BC));
      outf.write( (char*)&vfat.EC, sizeof(vfat.EC));
      outf.write( (char*)&vfat.ChipID, sizeof(vfat.ChipID));
      outf.write( (char*)&vfat.lsData, sizeof(vfat.lsData));  
      outf.write( (char*)&vfat.msData, sizeof(vfat.msData));
      outf.write( (char*)&vfat.BXfrOH, sizeof(vfat.msData));
      outf.write( (char*)&vfat.crc, sizeof(vfat.crc));
      outf.close();
      return(true);
    };	  

    bool readVFATDataBinary(std::string file, int event, const VFATData& vfat) {
      std::ifstream inpf(file.c_str(), std::ios_base::app | std::ios::binary );
      if ( event<0) return(false);
      if (!inpf.is_open()) return(false);
      inpf.read( (char*)&vfat.BC, sizeof(vfat.BC));
      inpf.read( (char*)&vfat.EC, sizeof(vfat.EC));
      inpf.read( (char*)&vfat.ChipID, sizeof(vfat.ChipID));
      inpf.read( (char*)&vfat.lsData, sizeof(vfat.lsData));
      inpf.read( (char*)&vfat.msData, sizeof(vfat.msData));
      inpf.read( (char*)&vfat.BXfrOH, sizeof(vfat.msData));
      inpf.read( (char*)&vfat.crc, sizeof(vfat.crc));
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

    void show24bits(uint32_t x) {
      int i;
      const unsigned long unit = 1;
      for(i=(sizeof(uint32_t)*8)-8-1; i>=0; i--)
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

    bool printVFATdataBits(int event, const VFATData& vfat) {
      if ( event<0) return(false);
      std::cout << "\nReceived VFAT data word: ichip " << event << std::endl;

      uint8_t   b1010 = (0xf000 & vfat.BC) >> 12;
      show4bits(b1010); std::cout << " BC     0x" << std::hex << (0x0fff & vfat.BC) 
                                  << std::setfill('0') << std::setw(4) << "      BX 0x" << vfat.BXfrOH << std::dec << std::endl;

      uint8_t   b1100 = (0xf000 & vfat.EC) >> 12;
      uint16_t   EC   = (0x0ff0 & vfat.EC) >> 4;
      uint8_t   Flag  = (0x000f & vfat.EC);
      show4bits(b1100); std::cout << " EC     0x" << std::hex << EC << std::dec << std::endl; 
      show4bits(Flag);  std::cout << " Flags " << std::endl;

      uint8_t   b1110 = (0xf000 & vfat.ChipID) >> 12;
      uint16_t ChipID = (0x0fff & vfat.ChipID);
      show4bits(b1110); std::cout << " ChipID 0x" << std::hex << ChipID << std::dec << " " << std::endl;

      /* std::cout << "     bxExp  0x" << std::hex << vfat.bxExp << std::dec << " " << std::endl;
         std::cout << "     bxNum  0x" << std::hex << ((0xff00 & vfat.bxNum) >> 8) << "        SBit " << (0x00ff & vfat.bxNum) << std::endl;
      */
      std::cout << " <127:64>:: 0x" << std::setfill('0') << std::setw(8) << std::hex << vfat.msData << std::dec << std::endl;
      std::cout << " <63:0>  :: 0x" << std::setfill('0') << std::setw(8) << std::hex << vfat.lsData << std::dec << std::endl;
      std::cout << "      BX    0x" << std::hex << vfat.BXfrOH << std::dec << std::endl;
      std::cout << "     crc    0x" << std::hex << vfat.crc << std::dec << std::endl;

      //std::cout << " " << std::endl; show16bits(vfat.EC);

      return(true);
    };

  } //end namespace gem::readout
} //end namespace gem
#endif
