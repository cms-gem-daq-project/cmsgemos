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
           outf<<Che1<< endl;
           outf<<BXH<< endl;
           outf<<Che2<< endl;
           outf<<EC<< endl;
           outf<<Che3<< endl;
           outf<<NumData<< endl;
           outf<<SBit<< endl;
           outf<<Che4<< endl;
           outf.close();
           return(true);
         };

         bool PrintHeader(int event){
           if( event<0 ) return(false);
           cout << " " << endl; cout << "VFAT Event Heder " << event << endl;
           cout << Che1<< endl;
           cout << BXH<< endl;
           cout << Che2<< endl;
           cout << EC<< endl;
           cout << Che3<< endl;
           cout << NumData<< endl;
           cout << SBit<< endl;
           cout << Che4<< endl;
           cout << "END of Heder" << endl;
           return(true);
         };

          uint32_t Che1:4;
          uint32_t BXH:28;
          uint32_t Che2:4;
          uint32_t EC:8;
  	  uint32_t Che3:4;
          uint32_t NumData:6;
          uint32_t SBit:6;
          uint32_t Che4:4;
          
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
           outf << data0 << endl;
           outf << data1 << endl;
           outf << data2 << endl;
           outf << data3 << endl;
           outf << data4 << endl;
           outf << data5 << endl;
           outf << data6 << endl;
           outf << data7 << endl;
           outf << checkSum << endl;
           outf.close();
           return(true);
         };	  
       
         bool PrintData(int event){
           if( event<0 ) return(false);
           cout << " " << endl; cout << "VFAT Block " << event << endl;
           cout << " BC " << BC << endl;
           cout << " EC " << EC << endl;
           cout << " Flag " << Flag << endl;
           cout << " ChipID " << ChipID << endl;
           cout << " data0 " << data0 << endl;
           cout << " data1 " << data1 << endl;
           cout << " data2 " << data2 << endl;
           cout << " data3 " << data3 << endl;
           cout << " data4 " << data4 << endl;
           cout << " data5 " << data5 << endl;
           cout << " data6 " << data6 << endl;
           cout << " data7 " << data7 << endl;
           cout << "checkSum  " << checkSum << endl;
           cout << "END of Block" << endl;
           return(true);
         };

         uint32_t BC:12;
         uint32_t EC:8;
         uint32_t Flag:4;
         uint32_t ChipID:12;

         uint32_t data0:16;
         uint32_t data1:16;
         uint32_t data2:16;
         uint32_t data3:16;
         uint32_t data4:16;
         uint32_t data5:16;
         uint32_t data6:16;
         uint32_t data7:16;
       
         uint32_t checkSum:16;

      }; //End of ThresholdEvent

    } //end namespace gem::supervisor::tbutils
  } //end namespace gem::supervisor
} //end namespace gem
#endif
