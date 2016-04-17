#ifndef gem_hw_amc13_AMC13SettingsEnums_h
#define gem_hw_amc13_AMC13SettingsEnums_h

namespace gem {
  namespace hw {
    namespace amc13 {
      class AMC13Settings {
      public:
        struct BGOCmd { //defines the BGO commands
          enum EBGOCmd { //defines the BGO commands
            BC0 = 0x1, //Bunch count reset 
            EC0 = 0x2, //Event count reset
            OC0 = 0x1, //Orbit count reset
            CalPulse  = 0x14, //CalPulse
            Resync    = 0x15, //Resync
            HardReset = 0x16, //HardReset
          } BGOCmd;
        };

      }; //end class AMC13Settings
      
    }//end namespace gem::hw::amc13
    
  }//end namespace gem::hw
  
  //typedef the enum for casting and access
  typedef gem::hw::amc13::AMC13Settings::BGOCmd::EBGOCmd AMC13BGOCmd;
}//end namespace gem

#endif
