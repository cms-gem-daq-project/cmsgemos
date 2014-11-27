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
           outf<<che1<< endl;
           outf<<bcn<< endl;
           outf<<che2<< endl;
           outf<<EC<< endl;
           outf<<che3<< endl;
           outf<<bxNum<< endl;
           outf<<SBit<< endl;
           outf<<che4<< endl;
           outf.close();
           return(true);
         };

          uint8_t che1:4;
          uint32_t bcn:28;
          uint8_t che2:4;
          uint16_t EC:8;
  	  uint8_t che3:4;
          uint16_t bxNum:6;
          uint16_t SBit:6;
          uint8_t che4:4;
          
      }; //End of ThresholdEventHeader

       class ThresholdEvent {
	public:
  
         bool keepData(string file, int event){
           ofstream outf(file.c_str(), ios_base::app | ios::binary );
           if(!outf.is_open()) return(false);
           if( event<0) return(false);
           outf << BC << endl;
           outf << EC << endl;
           outf << Flag << endl;
           outf << ChipID << endl;
           outf << lsdata << endl;
           outf << msdata << endl;
           outf << checkSum << endl;
           outf.close();
           return(true);
         };	  
       
         uint16_t BC:12;
         uint16_t EC:8;
         uint8_t  Flag:4;
         uint16_t ChipID:12;
         uint64_t lsdata:64;
         uint64_t msdata:64;
         uint16_t checkSum:16;

      }; //End of ThresholdEvent

    } //end namespace gem::supervisor::tbutils
  } //end namespace gem::supervisor
} //end namespace gem
#endif
