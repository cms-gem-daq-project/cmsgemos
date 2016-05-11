#ifndef GEM_HW_AMC13_AMC13SETTINGSENUMS_H
#define GEM_HW_AMC13_AMC13SETTINGSENUMS_H

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

        struct CSCGEMBGOCmd { //defines the CSCGEM format BGO commands
          enum CSCGEMEBGOCmd { //defines the CSCGEM format BGO commands
            BC0 = 0x1, //Bunch count reset 
            EC0 = 0x2, //Event count reset
            OC0 = 0x1, //Orbit count reset
            CalPulse  = 0x14, //CalPulse
            Resync    = 0x15, //Resync
            HardReset = 0x16, //HardReset
          } BGOCmd;
        };
      };  // class AMC13Settings
    }  // namespace gem::hw::amc13
  }  // namespace gem::hw
  
  //typedef the enum for casting and access
  typedef gem::hw::amc13::AMC13Settings::BGOCmd::EBGOCmd             AMC13BGOCmd;
  typedef gem::hw::amc13::AMC13Settings::CSCGEMBGOCmd::CSCGEMEBGOCmd GEMBGOCmd;
}  // namespace gem

#endif  // GEM_HW_AMC13_AMC13SETTINGSENUMS_H
