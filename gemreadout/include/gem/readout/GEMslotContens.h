#ifndef gem_readout_GEMslotContens_h
#define gem_readout_GEMslotContens_h

namespace gem {
  namespace readout {

   /*
    *  ChipID GEB data, 21-Aug-2015, TAMU should update slot contens
    */
    uint16_t slot[24] = 
       { 0xa64, 0xe74, 0xac0, 0xe98, 0xe7b, 0xa9c, 0xe63, 0xe6b,
         0xe80, 0xeaf, 0xea3, 0xb44, 0xe5b, 0xb40, 0xeb4, 0xe5f,
         0xe97, 0xe9f, 0xea7, 0xa84, 0xa78, 0xe78, 0xeab, 0xe7f
       };

    /* TAMU 25-Aug-2015
    uint16_t slot[24] =
      { 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff,
        0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff,
        0xeb8, 0xeac, 0xea4, 0xea0, 0xe93, 0xe8f, 0xe87, 0xe83
	}; */

    /*
     *  Slot Index converter from Hex ChipID
     */
    int GEBslotIndex(const uint32_t& GEBChipID){

      int indxslot = -1;
      for (int islot = 0; islot < 24; islot++){
        if ( (GEBChipID & 0x0fff ) == gem::readout::slot[islot] ) indxslot = islot;
      }//end for slot

      return (indxslot);
    }

  } //end namespace gem::readout
} //end namespace gem
#endif
