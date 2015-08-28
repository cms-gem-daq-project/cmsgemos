#ifndef gem_readout_GEMslotContens_h
#define gem_readout_GEMslotContens_h
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace gem {
  namespace readout {

   /*
    *  ChipID GEB data, 21-Aug-2015, TAMU should update slot contens
    */
    /*uint16_t slot[24] = 
       { 0xa64, 0xe74, 0xac0, 0xe98, 0xe7b, 0xa9c, 0xe63, 0xe6b,
         0xe80, 0xeaf, 0xea3, 0xb44, 0xe5b, 0xb40, 0xeb4, 0xe5f,
         0xe97, 0xe9f, 0xea7, 0xa84, 0xa78, 0xe78, 0xeab, 0xe7f
       };*/

    // Intialize slot array
    uint16_t slot[24] =
      { 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff,
        0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff,
        0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff
	}; 

    bool isFileRead = false;

    void getSlotCfg(){
      std::ifstream ifile;
      std::string path = std::getenv("BUILD_HOME");
      path +="/gemdaq-testing/gemreadout/data/slot_table.csv";
      ifile.open(path);
      for (int row = 0; row < 3; row++)
      {
        std::string line;
        std::getline(ifile, line);
        std::istringstream iss(line);
        if ( !ifile.good() ) break;
        for (int col = 0; col < 8; col++)
        {
          std::string val;
          std::getline(iss,val,',');
          std::stringstream convertor(val);
          convertor >> std::hex >> gem::readout::slot[8*row+col];
        }
      }
      ifile.close();
      gem::readout::isFileRead = true;
    }

    /*
     *  Slot Index converter from Hex ChipID
     */
    int GEBslotIndex(const uint32_t& GEBChipID){
      if (!isFileRead) getSlotCfg();
      int indxslot = -1;
      for (int islot = 0; islot < 24; islot++){
        if ( (GEBChipID & 0x0fff ) == gem::readout::slot[islot] ) indxslot = islot;
      }//end for slot

      return (indxslot);
    }

  } //end namespace gem::readout
} //end namespace gem
#endif
