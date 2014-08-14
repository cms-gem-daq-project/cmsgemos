#ifndef _gem_hw_vfat_VFAT2SettingsEnums_h
#define _gem_hw_vfat_VFAT2SettingsEnums_h

namespace gem {
  namespace hw {
    namespace vfat {
      class VFAT2ChannelSettings
      {
      public:
	struct ChannelBitMasks {
	  enum EChannelBitMasks {
	    TRIMDAC  = 0x1f,
	    ISMASKED = 0x20,
	    CHANCAL0 = 0x80,
	    CHANCAL  = 0x70
	  } ChannelBitMasks;
	};
      };

      class VFAT2Settings {
      public:
	struct ContRegBitMasks {
	  enum EContRegBitMasks {
	    //Control Register 0
	    RUNMODE       = 0x01,
	    TRIGMODE      = 0x0e,
	    MSPOL         = 0x10,
	    CALPOL        = 0x20,
	    CALMODE       = 0xc0,
	    
	    //Control Register 1
	    DACSEL        = 0x0f,
	    PROBEMODE     = 0x10,
	    LVDSMODE      = 0x20,
	    REHITCT       = 0xc0,
	    
	    //Control Register 2
	    HITCOUNTSEL   = 0x0f,
	    MSPULSELENGTH = 0x70,
	    DIGINSEL      = 0x80,
	    
	    //Control Register 3
	    TRIMDACRANGE  = 0x07,
	    PADBANDGAP    = 0x08,
	    DFTESTMODE    = 0x10,
	    SPARE         = 0xe0
	  } ContRegBitMasks;
	};

	struct ContRegBitShifts {
	  enum EContRegBitShifts {
	    //Control Register 0
	    RUNMODE       = 0,
	    TRIGMODE      = 1,
	    MSPOL         = 4,
	    CALPOL        = 5,
	    CALMODE       = 6,
	    
	    //Control Register 1
	    DACSEL        = 0,
	    PROBEMODE     = 4,
	    LVDSMODE      = 5,
	    REHITCT       = 6,
	    
	    //Control Register 2
	    HITCOUNTSEL   = 0,
	    MSPULSELENGTH = 4,
	    DIGINSEL      = 7,
	    
	    //Control Register 3
	    TRIMDACRANGE  = 0,
	    PADBANDGAP    = 3,
	    DFTESTMODE    = 4,
	    SPARE         = 5
	  } ContRegBitShifts;
	};

	struct RunMode { //defines the run mode
	  enum ERunMode { //defines the run mode
	    SLEEP = 0x0, //sleep (default)
	    RUN   = 0x1  //running
	  } RunMode;
	};

	struct TriggerMode { //Trigger mode settings
	  enum ETriggerMode { //Trigger mode settings
	    NOTRIGGER = 0x0, //No trigger (default)
	    ONEMODE   = 0x1, //One sector (S1)
	    FOURMODE  = 0x2, //Four sectors (S1-S4)
	    EIGHTMODE = 0x3,  //Eight sectors (S1-S8
	    GEMMODE   = 0x4 //GEM mode (S1 to S8 as defined in the HitCountMode)
	    //SPARE1    = 0x5, //GEM mode (S1 to S8 as defined in the HitCountMode)
	    //SPARE2    = 0x6, //GEM mode (S1 to S8 as defined in the HitCountMode)
	    //SPARE3    = 0x7  //GEM mode (S1 to S8 as defined in the HitCountMode)
	  } TriggerMode;
	};
	
	struct CalibrationMode { //Calibration settings
	  enum ECalibrationMode { //Calibration settings
	    NORMAL   = 0x0,      //Normal running (default)
	    VCAL     = 0x1,      //CalOut = VCal (programmable voltage level, possible to connect to DACo-V)
	    BASELINE = 0x2,      //CalOut = Baseline (fixed voltage level, possible to connect to DACo-V)
	    EXTERNAL = 0x3       //CalOut = External calibration pulse (not possible to connect to DACoV)
	  } CalibrationMode;
	};
	
	struct MSPolarity { //Monostable input polarity, tells the monostable polarity of the input signal
	  enum EMSPolarity { //Monostable input polarity, tells the monostable polarity of the input signal
	    POSITIVE = 0x0, //positive I/P signal (default)
	    NEGATIVE = 0x1  //negative I/P signal
	  } MSPolarity;
	};

	struct CalPolarity { //Calibration pulse polarity, to match with the polarity of the input signal
	  enum ECalPolarity { //Calibration pulse polarity, to match with the polarity of the input signal
	    POSITIVE = 0x0,  //positive I/P signal (default)
	    NEGATIVE = 0x1   //negative I/P signal
	  } CalPolarity;
	};
	
	struct ProbeMode { //Enables probe mode
	  enum EProbeMode { //Enables probe mode
	    OFF = 0x0,   //Probe mode disabled
	    ON  = 0x1    //Probe mode enabled
	  } ProbeMode;
	};

	struct LVDSPowerSave { //Enables power saving for unused LVDS sector drivers
	  enum ELVDSPowerSave { //Enables power saving for unused LVDS sector drivers
	    ENABLEALL = 0x0,   //Enable all sector LVDS drivers (default)
	    POWERSAVE = 0x1    //Enable only sector LVDS drivers needed (save power)
	  } LVDSPowerSave;
	};

	struct DACMode {     //Controls DAC selecton for DCU monitoring
	  enum EDACMode {     //Controls DAC selecton for DCU monitoring
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
	    CALOUT    = 0xa  //CalOut to DACo-V,      DACo-I "Hi Z"
	    //SPARE0 = 0xb, //Spare
	    //SPARE1 = 0xc, //Spare
	    //SPARE2 = 0xd, //Spare
	    //SPARE3 = 0xe, //Spare
	    //SPARE4 = 0xf  //Spare
	  } DACMode;
	};

	struct HitCountMode {  //Defines the input for the Hitounter
	  enum EHitCountMode {  //Defines the input for the Hitounter
	    FASTOR128   = 0x0, //HitCounter counts a FastOR of all 128 channels
	    COUNTS1     = 0x1, //HitCounter counts S1 (4, 5, 6, 28, 29, 30, 52, 53, 54, 76, 77, 78, 100, 101, 102   )
	    COUNTS2     = 0x2, //HitCounter counts S2 (7, 8, 9, 31, 32, 33, 55, 56, 57, 79, 80, 81, 103, 104, 105   )
	    COUNTS3     = 0x3, //HitCounter counts S3 (10, 11, 12, 34, 35, 36, 58, 59, 60, 82, 83, 84, 106, 107, 108)
	    COUNTS4     = 0x4, //HitCounter counts S4 (13, 14, 15, 37, 38, 39, 61, 62, 63, 85, 86, 87, 109, 110, 111)
	    COUNTS5     = 0x5, //HitCounter counts S5 (16, 17, 18, 40, 41, 42, 64, 65, 66, 88, 89, 90, 112, 113, 114)
	    COUNTS6     = 0x6, //HitCounter counts S6 (19, 20, 21, 43, 44, 45, 67, 68, 69, 91, 92, 93, 115, 116, 117)
	    COUNTS7     = 0x7, //HitCounter counts S7 (22, 23, 24, 46, 47, 48, 70, 71, 72, 94, 95, 96, 118, 119, 120)
	    COUNTS8     = 0x8  //HitCounter counts S8 (25, 26, 27, 49, 50, 51, 73, 74, 75, 97, 98, 99, 121, 122, 123)
	    //REDUNDANT0  = 0x9, //HitCounter counts S8
	    //REDUNDANT1  = 0xa, //HitCounter counts S8
	    //REDUNDANT2  = 0xb, //HitCounter counts S8
	    //REDUNDANT3  = 0xc, //HitCounter counts S8
	    //REDUNDANT4  = 0xd, //HitCounter counts S8
	    //REDUNDANT5  = 0xe, //HitCounter counts S8
	    //REDUNDANT6  = 0xf  //HitCounter counts S8
	  } HitCountMode;
	};
	
	struct MSPulseLength { //Defines the length of the monostable pulse
	  enum EMSPulseLength { //Defines the length of the monostable pulse
	    CLOCK1 = 0x0,      //pulse length is 1 clock period (default)
	    CLOCK2 = 0x1,      //pulse length is 2 clock period
	    CLOCK3 = 0x2,      //pulse length is 3 clock period
	    CLOCK4 = 0x3,      //pulse length is 4 clock period
	    CLOCK5 = 0x4,      //pulse length is 5 clock period
	    CLOCK6 = 0x5,      //pulse length is 6 clock period
	    CLOCK7 = 0x6,      //pulse length is 7 clock period
	    CLOCK8 = 0x7       //pulse length is 8 clock period
	  } MSPulseLength;
	};

	struct ReHitCT {  //The cycle time (or number of bits) for the Hit counter
	  enum EReHitCT {  //The cycle time (or number of bits) for the Hit counter
	    CYCLE0 = 0x0, //
	    CYCLE1 = 0x1, //
	    CYCLE2 = 0x2, //
	    CYCLE3 = 0x3  //
	  } ReHitCT;
	};
	
	struct DigInSel {  //Selects the input via the digital input pads (bypass of the analog frontend)
	  enum EDigInSel {  //Selects the input via the digital input pads (bypass of the analog frontend)
	    ANALOG  = 0x0, //default is analog pads
	    DIGITAL = 0x1  //bypass the analog pads
	  } DigInSel;
	};

	struct TrimDACRange {  //Adjusts the range of the TrimDACs CR3<2:0>
	  enum ETrimDACRange {
	    DEFAULT = 0x0,     //default value is 0
	    VALUE1  = 0x1,     //
	    VALUE2  = 0x2,     //
	    VALUE3  = 0x3,     //
	    VALUE4  = 0x4,     //
	    VALUE5  = 0x5,     //
	    VALUE6  = 0x6,     //
	    VALUE7  = 0x7      //
	  } TrimDACRange;
	};

	struct PbBG {          
	  enum EPbBG {         //Enables pad access to the bandgap output
	    UNCONNECTED = 0x0, //default is disconnected
	    CONNECTED   = 0x1  //pad connected
	  } PbBG;
	};

	struct DFTestPattern { 
	  enum EDFTestPattern { //Predefined data packet is sent to the DataOut without need for an input signal or trigger
	    IDLE = 0x0,         //Normal mode, sit idle
	    SEND = 0x1          //Send the packet
	  } DFTestPattern;
	};
      }; //end class VFAT2Settings
      
    }//end namespace gem::hw::vfat
    
  }//end namespace gem::hw
  
}//end namespace gem

#endif
