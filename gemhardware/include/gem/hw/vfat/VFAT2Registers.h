#ifndef GEM_HW_VFAT_VFAT2REGISTERS_H
#define GEM_HW_VFAT_VFAT2REGISTERS_H

namespace gem {
  namespace hw {
    namespace vfat {
      class VFAT2ChannelData;

      class VFAT2Registers
      {
      public:

        enum RunMode { //defines the run mode
          SLEEP = 0x0, //sleep (default)
          RUN   = 0x1  //running
        }

        enum TriggerMode { //Trigger mode settings
          NOTRIGGER = 0x0, //No trigger (default)
          ONEMODE   = 0x1, //One sector (S1)
          FOURMODE  = 0x2, //Four sectors (S1-S4)
          GEMMODE   = 0x3, //Eight sectors (S1-S8
          SPARE0    = 0x4, //GEM mode (S1 to S8 as defined in the HitCountMode)
          SPARE1    = 0x5, //GEM mode (S1 to S8 as defined in the HitCountMode)
          SPARE2    = 0x6, //GEM mode (S1 to S8 as defined in the HitCountMode)
          SPARE3    = 0x7, //GEM mode (S1 to S8 as defined in the HitCountMode)
        }

        enum CalibrationMode { //Calibration settings
          NORMAL   = 0x0,      //Normal running (default)
          VCAL     = 0x1,      //CalOut = VCal (programmable voltage level, possible to connect to DACo-V)
          BASELINE = 0x2,      //CalOut = Baseline (fixed voltage level, possible to connect to DACo-V)
          EXTERNAL = 0x3,      //CalOut = External calibration pulse (not possible to connect to DACoV)
        }

        enum MSPolarity { //Monostable input polarity, tells the monostable polarity of the input signal
          POSITIVE = 0x0, //positive I/P signal (default)
          NEGATIVE = 0x1  //negative I/P signal
        }

        enum CalPolarity { //Calibration pulse polarity, to match with the polarity of the input signal
          POSITIVE = 0x0,  //positive I/P signal (default)
          NEGATIVE = 0x1   //negative I/P signal
        }

        enum LVDSPowerSave { //Enables power saving for unused LVDA sector drivers
          ENABLEALL = 0x0,   //Enable all sector LVDS drivers (default)
          POWERSAVE = 0x1    //Enable only sector LVDS drivers needed (save power)
        }

        enum DACMode {     //Controls DAC selecton for DCU monitoring
          NORMAL    = 0x0, //DACo-V "low",          DACo-I "Hi Z", normal running (default)
          IPREIN    = 0x1, //IPreampIn to DACo-I,   DACo-V "low"
          IPREFEED  = 0x2, //IPreampFeed to DACo-I, DACo-V "low"
          IPREOUT   = 0x3, //IPreampOut to DACo-I,  DACo-V "low"
          ISHAPER   = 0x4, //IShaper to DACo-I,     DACo-V "low"
          ISHAPFEED = 0x5, //IShaperFeed to DACo-I, DACo-V "low"
          ICOMP     = 0x6, //IComp to DACo-I,       DACo-V "low"
          ITHRESH1  = 0x7, //IThreshold1 to DACo-I, DACo-V "low"
          ITHRESH2  = 0x8, //IThreshold2 to DACo-I, DACo-V "low"
          VCAL      = 0x9, //VCal to DACo-V,        DACo-I "Hi Z"
          CALOUT    = 0xa, //CalOut to DACo-V,      DACo-I "Hi Z"
          SPARE0 = 0xb, //Spare
          SPARE1 = 0xc, //Spare
          SPARE2 = 0xd, //Spare
          SPARE3 = 0xe, //Spare
          SPARE4 = 0xf, //Spare
        }

        enum HitCountMode {  //Defines the input for the Hitounter
          FASTOR128   = 0x0, //HitCounter counts a FastOR of all 128 channels
          COUNTS1     = 0x1, //HitCounter counts S1 (4, 5, 6, 28, 29, 30, 52, 53, 54, 76, 77, 78, 100, 101, 102   )
          COUNTS2     = 0x2, //HitCounter counts S2 (7, 8, 9, 31, 32, 33, 55, 56, 57, 79, 80, 81, 103, 104, 105   )
          COUNTS3     = 0x3, //HitCounter counts S3 (10, 11, 12, 34, 35, 36, 58, 59, 60, 82, 83, 84, 106, 107, 108)
          COUNTS4     = 0x4, //HitCounter counts S4 (13, 14, 15, 37, 38, 39, 61, 62, 63, 85, 86, 87, 109, 110, 111)
          COUNTS5     = 0x5, //HitCounter counts S5 (16, 17, 18, 40, 41, 42, 64, 65, 66, 88, 89, 90, 112, 113, 114)
          COUNTS6     = 0x6, //HitCounter counts S6 (19, 20, 21, 43, 44, 45, 67, 68, 69, 91, 92, 93, 115, 116, 117)
          COUNTS7     = 0x7, //HitCounter counts S7 (22, 23, 24, 46, 47, 48, 70, 71, 72, 94, 95, 96, 118, 119, 120)
          COUNTS8     = 0x8, //HitCounter counts S8 (25, 26, 27, 49, 50, 51, 73, 74, 75, 97, 98, 99, 121, 122, 123)
          REDUNDANT0  = 0x9, //HitCounter counts S8
          REDUNDANT1  = 0xa, //HitCounter counts S8
          REDUNDANT2  = 0xb, //HitCounter counts S8
          REDUNDANT3  = 0xc, //HitCounter counts S8
          REDUNDANT4  = 0xd, //HitCounter counts S8
          REDUNDANT5  = 0xe, //HitCounter counts S8
          REDUNDANT6  = 0xf, //HitCounter counts S8
        }

        enum MSPulseLength { //Defines the length of the monostable pulse
          CLOCK1 = 0x0,      //pulse length is 1 clock period (default)
          CLOCK2 = 0x1,      //pulse length is 2 clock period
          CLOCK3 = 0x2,      //pulse length is 3 clock period
          CLOCK4 = 0x3,      //pulse length is 4 clock period
          CLOCK5 = 0x4,      //pulse length is 5 clock period
          CLOCK6 = 0x5,      //pulse length is 6 clock period
          CLOCK7 = 0x6,      //pulse length is 7 clock period
          CLOCK8 = 0x7,      //pulse length is 8 clock period
        }

        enum ReHitCT {  //The cycle time (or number of bits) for the Hit counter
          CYCLE0 = 0x0, //
          CYCLE1 = 0x1, //
          CYCLE2 = 0x2, //
          CYCLE3 = 0x3  //
        }

        enum DigInSel {  //Selects the input via the digital input pads (bypass of the analog frontend)
          ANALOG  = 0x0, //default is analog pads
          DIGITAL = 0x1  //bypass the analog pads
        }

        enum TrimDACRange { //Adjusts the range of the TrimDACs
          DEFAULT = 0x0,    //default value is 0
        }

        enum PbBG {          //Enables pad access to the bandgap output
          UNCONNECTED = 0x0, //default is disconnected
          CONNECTED   = 0x1  //pad connected
        }

        enum DFTestPattern { //Predefined data packet is sent to the DataOut without need for an input signal or trigger
          IDLE = 0x0,        //Normal mode, sit idle
          SEND = 0x1         //Send the packet
        }
      };  // class VFAT2Registers
    }  // namespace gem::hw::vfat
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_VFAT_VFAT2REGISTERS_H
