#ifndef gem_readout_GEMslotContents_h
#define gem_readout_GEMslotContents_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace gem {
  namespace readout {

    struct GEMslotContents {
      //struct is a class with all members public by default

      static uint16_t slot[24];
      static bool isFileRead;

      static void initSlots(const std::string& slotFile="slot_table.csv") {
        for (int i = 0; i < 24; ++i)
          slot[i] = 0xfff;
        isFileRead = false;
        return;
      };

      static void getSlotCfg(const std::string& slotFile="slot_table.csv") {
        std::ifstream ifile;
        std::string path = std::getenv("BUILD_HOME");
        path +="/gemdaq-testing/gemreadout/data/";
        path += slotFile;
        ifile.open(path);
        
        if(!ifile.is_open()) {
          std::cout << "\nThe file: " << ifile << " is missing.\n" << std::endl;
          isFileRead = false;
          return;
        };        
        
        for (int row = 0; row < 3; row++) {
          std::string line;
          std::getline(ifile, line);
          std::istringstream iss(line);
          if ( !ifile.good() ) break;
          for (int col = 0; col < 8; col++) {
            std::string val;
            std::getline(iss,val,',');
            std::stringstream convertor(val);
            convertor >> std::hex >> slot[8*row+col];
          }
        }
        ifile.close();
        isFileRead = true;
      };
      
      /*
       *  Slot Index converter from Hex ChipID
       */
      static int GEBslotIndex(const uint32_t& GEBChipID,
                              const std::string& slotFile) {
        if (!isFileRead) getSlotCfg(slotFile);
        int indxslot = -1;
        for (int islot = 0; islot < 24; islot++) {
          if ( (GEBChipID & 0x0fff ) == slot[islot] ) indxslot = islot;
        }//end for slot
        
        return (indxslot);
      };
    }; // end class GEMslotContents
  } //end namespace gem::readout
} //end namespace gem
#endif
