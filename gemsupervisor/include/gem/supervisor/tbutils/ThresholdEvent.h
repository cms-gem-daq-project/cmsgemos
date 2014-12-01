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

      class ThresholdEventHeader {
	public:

         bool keepHeader(string file, int event){
           ofstream outf(file.c_str(), ios_base::app | ios::binary );
           if(!outf.is_open()) return(false);
           if( event<0) return(false);
           outf<<BC<< endl;
           outf<<EC<< endl;
           outf<<bxExp<< endl;
           outf<<bxNum<< endl;
           outf<<ChipID<< endl;
           outf<<crc<< endl;
           outf.close();
           return(true);
         };

	 uint16_t BC;     // 1010:4, BC:12 
         uint16_t EC;     // 1100:4, EC:8, Flags:4
         uint32_t bxExp;  // :28
         uint16_t bxNum;  // :6, Sbit:6
         uint16_t ChipID; // 1110, :12
         uint16_t crc;    // :16
	 
      }; //End of ThresholdEventHeader

       class ThresholdEvent {
	public:
  
         bool keepData(string file, int event){
           ofstream outf(file.c_str(), ios_base::app | ios::binary );
           if(!outf.is_open()) return(false);
           if( event<0) return(false);
           outf << lsdata << endl;
           outf << msdata << endl;
           outf.close();
           return(true);
         };	  
       
         uint64_t lsdata; // :64
         uint64_t msdata; // :64

      }; //End of ThresholdEvent

      struct ChannelData {
        uint64_t lsdata;  // ch1to64
        uint64_t msdata;  // ch65to128
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

    } //end namespace gem::supervisor::tbutils
  } //end namespace gem::supervisor
} //end namespace gem
#endif
