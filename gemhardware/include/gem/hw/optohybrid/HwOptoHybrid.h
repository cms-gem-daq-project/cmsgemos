#ifndef gem_hw_optohybrid_HwOPTOHYBRID_h
#define gem_hw_optohybrid_HwOPTOHYBRID_h

#include "gem/hw/GEMHwDevice.h"
#include "gem/hw/glib/HwGLIB.h"

#include "gem/hw/optohybrid/exception/Exception.h"
//#include "gem/hw/optohybrid/OptoHybridMonitor.h"

#define MAX_VFATS 24

namespace gem {
  namespace hw {
    namespace optohybrid {
         
      //class OptoHybridMonitor;
      class HwOptoHybrid: public gem::hw::GEMHwDevice
        {
        public:

          /**
           * @struct OptoHybridWBMasterCounters
           * @brief This struct stores retrieved counters related to the OptoHybrid wishbone transactions
           * @var OptoHybridWBMasterCounters::GTX
           * GTX is a std::pair of uint32_t, containing counters for the number of strobe(first)
           * and acknowledge(second) requests on the GTX
           * @var OptoHybridWBMasterCounters::ExtI2C
           * ExtI2C is a std::pair of uint32_t, containing counters for the number of strobe(first)
           * and acknowledge(second) requests on the extended I2C module
           * @var OptoHybridWBMasterCounters::Scan
           * Scan is a std::pair of uint32_t, containing counters for the number of strobe(first)
           * and acknowledge(second) requests on the Scan module
           * @var OptoHybridWBMasterCounters::DAC
           * DAC is a std::pair of uint32_t, containing counters for the number of strobe(first)
           * and acknowldege(second) requests on the DAC module
           **/
          typedef struct OptoHybridWBMasterCounters {
            std::pair<uint32_t,uint32_t> GTX   ; 
            std::pair<uint32_t,uint32_t> ExtI2C; 
            std::pair<uint32_t,uint32_t> Scan  ; 
            std::pair<uint32_t,uint32_t> DAC   ; 
            
          OptoHybridWBMasterCounters() : 
            GTX(std::make_pair(0,0)),
              ExtI2C(std::make_pair(0,0)),
              Scan(std::make_pair(0,0)),
              DAC(std::make_pair(0,0)) {};

            void reset() {
              GTX=std::make_pair(0,0);
              ExtI2C=std::make_pair(0,0);
              Scan=std::make_pair(0,0);
              DAC=std::make_pair(0,0);
              return; };
          } OptoHybridWBMasterCounters;
          
          /**
           * @struct OptoHybridWBSlaveCounters
           * @brief This struct stores retrieved counters related to the OptoHybrid wishbone transactions
           * @var OptoHybridWBSlaveCounters::I2C
           * I2C is a std::vector<std::pair of uint32_t>, size of 6, containing counters for the number
           * of strobe(first) and acknowledge(second) requests on each of the 6 [0-5] I2C modules
           * @var OptoHybridWBSlaveCounters::ExtI2C
           * ExtI2C is a std::pair of uint32_t, containing counters for the number of strobe(first)
           * and acknowledge(second) requests on the extended I2C module
           * @var OptoHybridWBSlaveCounters::Scan
           * Scan is a std::pair of uint32_t, containing counters for the number of strobe(first)
           * and acknowledge(second) requests on the Scan module
           * @var OptoHybridWBSlaveCounters::T1
           * T1 is a std::pair of uint32_t, containing counters for the number of strobe(first)
           * and acknowledge(second) requests on the T1 module
           * @var OptoHybridWBSlaveCounters::DAC
           * DAC is a std::pair of uint32_t, containing counters for the number of strobe(first)
           * and acknowldege(second) requests on the DAC module
           * @var OptoHybridWBSlaveCounters::ADC
           * ADC is a std::pair of uint32_t, containing counters for the number of strobe(first)
           * and acknowldege(second) requests on the ADC module
           * @var OptoHybridWBSlaveCounters::Clocking
           * Clocking is a std::pair of uint32_t, containing counters for the number of strobe(first)
           * and acknowldege(second) requests on the Clocking module
           * @var OptoHybridWBSlaveCounters::Counters
           * Counters is a std::pair of uint32_t, containing counters for the number of strobe(first)
           * and acknowldege(second) requests on the Counters module
           * @var OptoHybridWBSlaveCounters::System
           * System is a std::pair of uint32_t, containing counters for the number of strobe(first)
           * and acknowldege(second) requests on the System module
           **/
          typedef struct OptoHybridWBSlaveCounters {
            std::vector<std::pair<uint32_t,uint32_t> > I2C; 
            std::pair<uint32_t,uint32_t> ExtI2C; 
            std::pair<uint32_t,uint32_t> Scan  ; 
            std::pair<uint32_t,uint32_t> T1    ; 
            std::pair<uint32_t,uint32_t> DAC   ; 
            std::pair<uint32_t,uint32_t> ADC   ; 
            std::pair<uint32_t,uint32_t> Clocking; 
            std::pair<uint32_t,uint32_t> Counters; 
            std::pair<uint32_t,uint32_t> System  ; 
            
          OptoHybridWBSlaveCounters() : 
            I2C(6, std::make_pair(0,0)),
              ExtI2C(std::make_pair(0,0)),
              Scan(std::make_pair(0,0)),
              T1(std::make_pair(0,0)),
              DAC(std::make_pair(0,0)),
              ADC(std::make_pair(0,0)),
              Clocking(std::make_pair(0,0)),
              Counters(std::make_pair(0,0)),
              System(std::make_pair(0,0)) {};

            void reset() {
              std::fill(I2C.begin(), I2C.end(), std::make_pair(0,0));
              ExtI2C=std::make_pair(0,0);
              Scan=std::make_pair(0,0);
              T1=std::make_pair(0,0);
              DAC=std::make_pair(0,0);
              ADC=std::make_pair(0,0);
              Clocking=std::make_pair(0,0);
              Counters=std::make_pair(0,0);
              System=std::make_pair(0,0);
              return; };
          } OptoHybridWBSlaveCounters;
          
          /**
           * @struct OptoHybridT1Counters
           * @brief This struct stores retrieved counters related to the OptoHybrid T1 signals
           * @var OptoHybridT1Counters::AMC13
           * AMC13 contains the counters for T1 signals received from the GLIB TTC decder
           * @var OptoHybridT1Counters::Firmware
           * Firmware contains the counters for T1 signals generated in the firmware module
           * @var OptoHybridT1Counters::External
           * External contains the counters for T1 signals received an external sourcd
           * @var OptoHybridT1Counters::Loopback
           * Loopback contains the counters for T1 signals looped back onto the system
           * @var OptoHybridT1Counters::Sent
           * Sent contains the counters for T1 signals sent by the OptoHybrid to the GEB
           **/
          typedef struct OptoHybridT1Counters {
            std::vector<uint32_t> AMC13   ; 
            std::vector<uint32_t> Firmware;
            std::vector<uint32_t> External;
            std::vector<uint32_t> Loopback;
            std::vector<uint32_t> Sent    ;
            
          OptoHybridT1Counters() : 
            AMC13(4, 0),Firmware(4, 0),External(4, 0),Loopback(4, 0),Sent(4, 0) {};
            void reset() {
              std::fill(AMC13.begin(),    AMC13.end(),    0);
              std::fill(Firmware.begin(), Firmware.end(), 0);
              std::fill(External.begin(), External.end(), 0);
              std::fill(Loopback.begin(), Loopback.end(), 0);
              std::fill(Sent.begin(),     Sent.end(),     0);
              return; };
          } OptoHybridT1Counters;
          
           /**
            * @struct OptoHybridVFATCRCCounters
            *  @brief This struct stores retrieved counters related to the OptoHybrid CRC check on 
            *  the received VFAT data packets
            *  @var OptoHybridVFATCRCCounters::CRCCounters
            *  CRCCounters contains the counters for the CRC performed on the
            *  received VFAT packets from each chip
            *  - first is the number of Valid CRCs
            *  - second is the number of Incorrect CRCs
            **/
          typedef struct OptoHybridVFATCRCCounters {
            std::vector<std::pair<uint32_t,uint32_t> > CRCCounters;
            
          OptoHybridVFATCRCCounters() : 
            CRCCounters(24, std::make_pair(0,0)) {};
            void reset() {
              std::fill(CRCCounters.begin(), CRCCounters.end(), std::make_pair(0,0));
              return; };
          } OptoHybridVFATCRCCounters;
          
          HwOptoHybrid();
          HwOptoHybrid(std::string const& optohybridDevice, std::string const& connectionFile);
          HwOptoHybrid(std::string const& optohybridDevice, std::string const& connectionURI,
                       std::string const& addressTable);
          HwOptoHybrid(std::string const& optohybridDevice, uhal::HwInterface& uhalDevice);
          HwOptoHybrid(gem::hw::glib::HwGLIB const& glib, int const& slot);

          virtual ~HwOptoHybrid();
          
          //updating interfaces////virtual void connectDevice();
          //updating interfaces////virtual void releaseDevice();
          //updating interfaces////virtual void initDevice();
          //updating interfaces////virtual void enableDevice();
          //updating interfaces//virtual void configureDevice();
          //updating interfaces//virtual void configureDevice(std::string const& xmlSettings);
          //updating interfaces////virtual void configureDevice(std::string const& dbConnectionString);
          //updating interfaces////virtual void disableDevice();
          //updating interfaces////virtual void pauseDevice();
          //updating interfaces////virtual void startDevice();
          //updating interfaces////virtual void stopDevice();
          //updating interfaces////virtual void resumeDevice();
          //updating interfaces////virtual void haltDevice();

          virtual bool isHwConnected();

          /** Read the board ID registers
           * @returns the OptoHybrid board ID 
           **/
          //std::string getBoardID()   const;

          /**
           * Read the firmware register
           * @returns a hex number corresponding to the build date
           **/

          uint32_t getFirmware() {
  std::cout << "oh device base node " << getDeviceBaseNode() << std::endl;
            uint32_t fwver = readReg(getDeviceBaseNode(),"STATUS.FW");
            DEBUG("OH has firmware version 0x" 
                  << std::hex << fwver << std::dec << std::endl);
            return fwver;
          };
	    
          /**
           * Read the firmware register
           * @returns a string corresponding to the build date
           **/
          std::string getFirmwareDate() {
            std::stringstream retval;
            retval << "0x" << std::hex << getFirmware() << std::dec << std::endl;
            return retval.str();
          };
					
        public:
          /**
           * Read the link status registers, store the information in a struct
           * @retval _status a struct containing the status bits of the optical link
           **/
          GEMHwDevice::OpticalLinkStatus LinkStatus();

          /**
           * Reset the link status registers
           * @param uint8_t resets control which bits to reset
           * bit 1 - TRK_ErrCnt         0x1
           * bit 2 - TRG_ErrCnt         0x2
           * bit 3 - Data_Rec           0x4
           **/
          void LinkReset(uint8_t const& resets);

          /**
           * Reset the all link status registers
           * @param uint8_t resets control which bits to reset
           * OBSOLETE in new V2 firmware
           **/
          void ResetLinks(uint8_t const& resets) {
            return;
            /*
            for (auto link = v_activeLinks.begin(); link != v_activeLinks.end(); ++link)
              LinkReset(link->first,resets);
            */
          };
	  
          /**
           * Read the trigger data
           * @retval uint32_t returns 32 bits 6 bits for s-bits and 26 for bunch countrr
           **/
          //uint32_t readTriggerData();

          //Clocking
          /**
           * Setup the OptoHybrid clock 
           * @param uint8_t source
           * 0x0 on board oscillator
           * 0x1 GTX recovered clock
           * 0x2 external clock
           **/
          void setReferenceClock(uint8_t const& source) {
            writeReg(getDeviceBaseNode(),"CONTROL.CLOCK.REF_CLK"  ,(uint32_t)source  );
          };

          /**
           * Setup the VFAT clock 
           * @returns uint32_t clock source
           * 0x0 on board oscillator
           * 0x1 GTX recovered clock
           * 0x2 external clock
           **/
          uint32_t getReferenceClock() {
            return readReg(getDeviceBaseNode(),"CONTROL.CLOCK.REF_CLK");
          };

          /**
           * Setup the VFAT clock 
           * @param bool source true uses the external clock, false uses the onboard clock
           * @param bool fallback uses the external clock, false uses the onboard clock
           * NOT YET AVAILABLE IN V2 FIRMWARE
           **/
          void setVFATClock(bool source, bool fallback) {
            //std::stringstream regName;
            //regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
            //writeReg(getDeviceBaseNode(),regName.str()+".CLOCKING.VFAT.SOURCE"  ,(uint32_t)source  );
            //writeReg(getDeviceBaseNode(),regName.str()+".CLOCKING.VFAT.FALLBACK",(uint32_t)fallback);
          };

          /**
           * VFAT clock status
           * @param bool source true uses the external clock, false uses the onboard clock
           * @param bool fallback uses the external clock, false uses the onboard clock
           * NOT YET AVAILABLE IN V2 FIRMWARE
           **/
          std::pair<bool,bool> StatusVFATClock() {
           // std::stringstream regName;
           // regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
           // uint32_t src = readReg(getDeviceBaseNode(),regName.str()+".CLOCKING.VFAT.SOURCE");
           // uint32_t flb = readReg(getDeviceBaseNode(),regName.str()+".CLOCKING.VFAT.FALLBACK");
           // //maybe do a check to ensure that the value has been read properly?
            return std::make_pair(0,0);
          };

          /**
           * Setup the CDCE clock 
           * @param bool source true uses the external clock, false uses the onboard clock
           * @param bool fallback uses the external clock, false uses the onboard clock
           * NOT YET AVAILABLE IN V2 FIRMWARE
           **/
          void setCDCEClock(bool source, bool fallback) {
            //std::stringstream regName;
            //regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
            //writeReg(getDeviceBaseNode(),regName.str()+".CLOCKING.CDCE.SOURCE"  ,(uint32_t)source  );
            //writeReg(getDeviceBaseNode(),regName.str()+".CLOCKING.CDCE.FALLBACK",(uint32_t)fallback);
          };
      
          /**
           * CDCE clock status
           * @param bool source true uses the external clock, false uses the onboard clock
           * @param bool fallback uses the external clock, false uses the onboard clock
           * NOT YET AVAILABLE IN V2 FIRMWARE
           **/
          std::pair<bool,bool> StatusCDCEClock() {
            //std::stringstream regName;
            //regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
            //uint32_t src = readReg(getDeviceBaseNode(),regName.str()+".CLOCKING.CDCE.SOURCE");
            //uint32_t flb = readReg(getDeviceBaseNode(),regName.str()+".CLOCKING.CDCE.FALLBACK");
            ////maybe do a check to ensure that the value has been read properly?
            return std::make_pair(0,0);
          };

          ///** Read the VFAT clock source
          // * @retval bool true uses the external clock, false uses the onboard clock
          // **/
          //bool getVFATClockSource();
          //
          //
          ///** Set the VFAT onboard clock fallback
          // * Allows the clock to fall back to 
          // * @param bool true allows the optohybrid to switch back to the onboard clock
          // * if it fails to lock
          // **/
          //void setVFATFallback(bool);
          //
          ///** Read the VFAT clock fallback status
          // * @retval returns status of VFAT clock fallback (false does not allow automatic
          // * fallback to onboard clock
          // **/
          //bool getVFATFallback();
          //
          //
          ///** Set the CDCE clock source
          // * @param bool true uses the external clock, false uses the onboard clock
          // **/
          //void setCDCEClockSource(bool);
          //
          ///** Read the CDCE clock source
          // * @retval bool true uses the external clock, false uses the onboard clock
          // **/
          //bool getCDCEClockSource();
          //
          ///** Set the CDCE clock fallback mode
          // * @param bool true allows the optohybrid to switch back to the onboard clock if the
          // * CDCE doesn't lock
          // **/
          //void setCDCEFallback(bool);
          //
          ///** Read the CDCE clock fallback mode
          // * @retval bool false does not allow the optohybrid to switch the clock back
          // **/
          //bool getCDCEFallback();


          /**
           * Set the Trigger source
           * @param uint8_t mode
           * 0 from GLIB TTC decoder
           * 1 from OptoHybrid firmware (T1 module)
           * 2 from external source (LEMO)
           * 3 Internal loopback of s-bits
           * 4 from all
           **/
          void setTrigSource(uint8_t const& mode) {
            switch (mode) {
            case(0):
              writeReg(getDeviceBaseNode(),"CONTROL.TRIGGER.SOURCE",mode);
              return;
            case(1):
              writeReg(getDeviceBaseNode(),"CONTROL.TRIGGER.SOURCE",mode);
              return;
            case(2):
              writeReg(getDeviceBaseNode(),"CONTROL.TRIGGER.SOURCE",mode);
              return;
            case(3):
              writeReg(getDeviceBaseNode(),"CONTROL.TRIGGER.SOURCE",mode);
              return;
            case(4):
              writeReg(getDeviceBaseNode(),"CONTROL.TRIGGER.SOURCE",mode);
              return;
            default:
              writeReg(getDeviceBaseNode(),"CONTROL.TRIGGER.SOURCE",0x0);
              return;
            }
          };

          /**
           * Read the Trigger source
           * @retval uint8_t
           * 0 from GLIB TTC decoder
           * 1 from OptoHybrid firmware (T1 module)
           * 2 from external source (LEMO)
           * 3 Internal loopback of s-bits
           * 4 from all
           **/
          uint8_t getTrigSource() { 
            return readReg(getDeviceBaseNode(),"CONTROL.TRIGGER.SOURCE"); };


          /**
           * Set the S-bit source
           * @param uint32_t mask which s-bits to forward (maximum 6)
           **/
          void setSBitSource(uint32_t const& mask) {
            writeReg(getDeviceBaseNode(),"CONTROL.OUTPUT.SBits",mask); };

          /**
           * Read the S-bit source
           * @retval uint32_t which VFAT chips are sending S-bits
           **/
          uint32_t getSBitSource() {
            return readReg(getDeviceBaseNode(),"CONTROL.OUTPUT.SBits"); };


          /**
           * @brief the T1 module is very different between V1/1.5 and V2
           * One must select the mode 
           * One must select the signal
          **/
          typedef struct T1Sequence {
            uint64_t l1a_seq;
            uint64_t cal_seq;
            uint64_t rsy_seq;
            uint64_t bc0_seq;
          } T1Sequence;
          
          /**
           * @brief the T1 module is very different between V1/1.5 and V2
           * @param uint8_t mode can be any of
           *  - 0 Single command
           *  - 1 CalPulse followed by L1A
           *  - 2 Follow the sequence specified
           * @param uint8_t type is the type of command to send for mode 0
           *  - 0 L1A
           *  - 1 CalPulse
           *  - 2 Resync
           *  - 3 BC0
           * @param T1Sequence sequence is a sequence of T1 signals to generate
           * @param bool reset says whether to reset the module or not
          **/
          void configureT1Generator(uint8_t const& mode, uint8_t const& type,
                                    T1Sequence sequence,
                                    bool reset) {
            if (reset)
              writeReg(getDeviceBaseNode(),"T1Controller.RESET",0x1);

            writeReg(getDeviceBaseNode(),"T1Controller.MODE",mode);
            if (mode == 0x0) {
              writeReg(getDeviceBaseNode(),"T1Controller.TYPE",type);
            } else if (mode == 0x2) {
              writeReg(getDeviceBaseNode(),"T1Controller.Sequence.L1A.MSB",     sequence.l1a_seq>>32);
              writeReg(getDeviceBaseNode(),"T1Controller.Sequence.L1A.LSB",     sequence.l1a_seq&0xffffffff);
              writeReg(getDeviceBaseNode(),"T1Controller.Sequence.CalPulse.MSB",sequence.cal_seq>>32);
              writeReg(getDeviceBaseNode(),"T1Controller.Sequence.CalPulse.LSB",sequence.cal_seq&0xffffffff);
              writeReg(getDeviceBaseNode(),"T1Controller.Sequence.Resync.MSB",  sequence.rsy_seq>>32);
              writeReg(getDeviceBaseNode(),"T1Controller.Sequence.Resync.LSB",  sequence.rsy_seq&0xffffffff);
              writeReg(getDeviceBaseNode(),"T1Controller.Sequence.BCO.MSB",     sequence.bc0_seq>>32);
              writeReg(getDeviceBaseNode(),"T1Controller.Sequence.BC0.LSB",     sequence.bc0_seq&0xffffffff);
            }
          };
          
          /**
           * @brief Start the T1 generator (must be configured first or have a configuration already loaded)
           * @param uint32_t ntrigs number of signals to send before stopping (0 will send continuously)
           * @param uint32_t rate rate at which to repeat the sending
           * @param uint32_t delay delay between CalPulse and L1A (only for T1 mode 1)
           **/
          void startT1Generator(uint32_t const& ntrigs, uint32_t const& rate, uint32_t const& delay) {
            uint32_t interval = 1/(rate*0.000000025);
            
            writeReg(getDeviceBaseNode(),"T1Controller.NUMBER"  ,ntrigs  );
            writeReg(getDeviceBaseNode(),"T1Controller.INTERVAL",interval);
            writeReg(getDeviceBaseNode(),"T1Controller.DELAY"   ,delay   );
            
            //don't toggle off if the generator is currently running
            if (!statusT1Generator())
              writeReg(getDeviceBaseNode(),"T1Controller.TOGGLE",0x1);
          };
          
          /**
           * @brief Stop the T1 generator
           * @param bool reset tells whether to reset the state of the module
           **/
          void stopT1Generator(bool reset) {
            //don't toggle on if the generator is currently not running
            if (statusT1Generator())
              writeReg(getDeviceBaseNode(),"T1Controller.TOGGLE",0x1);
            if (reset)
              writeReg(getDeviceBaseNode(),"T1Controller.RESET",0x1);
          };
          
          /**
           * @brief Status of the T1 generator
           * @returns uint8_t the status of the T1 generator, telling which mode is running
           * (0 is nothing running)
           **/
          uint8_t statusT1Generator() {
            return readReg(getDeviceBaseNode(),"T1Controller.MONITOR");
          };

          /**
           * Send an internal L1A
           * @param uint32_t ntrigs, how many L1As to send
           * @param uint32_t rate, rate at which signals will be generated
           **/
          void sendL1A(uint32_t const& ntrigs, uint32_t const& rate=1) {
            T1Sequence sequence;
            configureT1Generator(0x0, 0x0, sequence, true);
            startT1Generator(ntrigs,rate, 0);
          };

          /**
           * Send an internal CalPulse
           * @param uint32_t npulse, how many CalPulses to send
           * @param uint32_t rate, rate at which signals will be generated
           **/
          void sendCalPulse(uint32_t const& npulse, uint32_t const& rate=1) {
            T1Sequence sequence;
            configureT1Generator(0x0, 0x1, sequence, true);
            startT1Generator(npulse, rate, 0);
          };
          
          /**
           * Send a CalPulse followed by an L1A
           * @param uint32_t npulse, how many pairs to send
           * @param uint32_t delay, how long between L1A and CalPulse
           * @param uint32_t rate, rate at which signals will be generated
           **/
          void sendL1ACal(uint32_t const& npulse, uint32_t const& delay, uint32_t const& rate=1) {
            T1Sequence sequence;
            configureT1Generator(0x1, 0x0, sequence, true);
            startT1Generator(npulse,rate, delay);
          };

          /**
           * Send an internal Resync
           * @param uint32_t nresync, total number of resync signals to send
           * @param uint32_t rate, rate at which signals will be generated
           **/
          void sendResync(uint32_t const& nresync=1,uint32_t const& rate=1) {
            writeReg(getDeviceBaseNode(), "CONTROL.TRIGGER.SOURCE",0x0);
            writeReg(getDeviceBaseNode(), "CONTROL.CLOCK.REF_CLK",0x1);
            T1Sequence sequence;
            configureT1Generator(0x0, 0x2, sequence, true);
            startT1Generator(nresync, rate, 0); };


          /**
           * Send an internal BC0
           * @param uint32_t nbc0, total number of BC0 signals to send
           * @param uint32_t rate, rate at which signals will be generated
           **/
          void sendBC0(uint32_t const& nbc0=1, uint32_t const& rate=1) {
            T1Sequence sequence;
            configureT1Generator(0x0, 0x3, sequence, true);
            startT1Generator(nbc0, rate, 0); };

          ///Counters

          /**
           * Get the recorded number of signals sent/received by the OptoHybrid wishbone master
           * @returns OptoHybridWBMasterCounters struct, with updated values
           **/
          OptoHybridWBMasterCounters getWBMasterCounters() { return m_wbMasterCounters; };
          void updateWBMasterCounters();
          void resetWBMasterCounters();

          /**
           * Get the recorded number of signals sent/received by the OptoHybrid wishbone slave
           * @returns OptoHybridWBSlaveCounters struct, with updated values
           **/
          OptoHybridWBSlaveCounters getWBSlaveCounters() { return m_wbSlaveCounters; };
          void updateWBSlaveCounters();
          void resetWBSlaveCounters();

          /**
           * Get the recorded number of signals sent/received by the OptoHybrid wishbone slave
           * @returns OptoHybridT1Counters struct, with updated values
           **/
          OptoHybridT1Counters getT1Counters() { return m_t1Counters; };
          void updateT1Counters();
          void resetT1Counters();
	  
          /**
           * Get the recorded number of valid/incorrect CRCs performed by the OptoHybrid 
           *  on the data packets received from the VFATs
           * @returns OptoHybridT1Counters struct, with updated values
           **/
          OptoHybridVFATCRCCounters getVFATCRCCounters() { return m_vfatCRCCounters; };
          void updateVFATCRCCounters();
          void resetVFATCRCCounters();
	  
          /**
           * Get the recorded number of T1 signals
           * @param signal specifies which T1 signal counter to read
           *  - 0 L1A
           *  - 1 CalPulse
           *  - 2 Resync
           *  - 3 BC0
           * @param mode specifies which T1 counter to read
           *  - 0 from the TTC decoder on the GLIB
           *  - 1 from the T1 generator in the firmware
           *  - 2 from an external source
           *  - 3 from looping back the sbits
           *  - 4 sent along the GEB
           **/
          uint32_t getT1Count(uint8_t const& signal, uint8_t const& mode) {
            std::stringstream t1Signal;
            if (signal == 0x0)
              t1Signal << "L1A";
            if (signal == 0x1)
              t1Signal << "CalPulse";
            if (signal == 0x2)
              t1Signal << "Resync";
            if (signal == 0x3)
              t1Signal << "BC0";
            
            switch(mode) {
            case 0:
              return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.TTC.%s",     (t1Signal.str()).c_str()));
            case 1:
              return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.INTERNAL.%s",(t1Signal.str()).c_str()));
            case 2:
              return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.EXTERNAL.%s",(t1Signal.str()).c_str()));
            case 3:
              return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.LOOPBACK.%s",(t1Signal.str()).c_str()));
            case 4:
              return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.SENT.%s",    (t1Signal.str()).c_str()));
            default:
              return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.SENT.%s",    (t1Signal.str()).c_str()));
            }
          };
	  
          /**
           * Get the recorded number of L1A signals
           * @param mode specifies which L1A counter to read
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           **/
          uint32_t getL1ACount(uint8_t const& mode) {
            return getT1Count(0x0, mode); };
	  
          /**
           * Get the recorded number of CalPulse signals
           * @param mode specifies which CalPulse counter to read
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           **/
          uint32_t getCalPulseCount(uint8_t const& mode) {
            return getT1Count(0x1, mode); };
	  
          /**
           * Get the recorded number of Resync signals
           * @param mode specifies which L1A counter to read
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           **/
          uint32_t getResyncCount(uint8_t const& mode=0x0) {
            return getT1Count(0x2, mode); };

          /**
           * Get the recorded number of BC0 signals
           * @param mode specifies which L1A counter to read
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           **/
          uint32_t getBC0Count(uint8_t const& mode=0x0) {
            return getT1Count(0x3, mode); };
          
          /**
           * Get the recorded number of BXCount signals
           * OBSOLETE in V2 firmware
           **/
          uint32_t getBXCountCount() {
            return 0x0;
            /*
            std::stringstream regName;
            regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
            return readReg(getDeviceBaseNode(),regName.str()+".COUNTERS.BXCount");
            */
          };
	  
          ///Resets
          /**
           * Get the recorded number of T1 signals
           * @param mode specifies which T1 counter to read
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           * 5 all
           **/
          void resetT1Count(uint8_t const& signal, uint8_t const& mode) {
            std::stringstream t1Signal;
            if (signal == 0x0)
              t1Signal << "L1A";
            if (signal == 0x1)
              t1Signal << "CalPulse";
            if (signal == 0x2)
              t1Signal << "Resync";
            if (signal == 0x3)
              t1Signal << "BC0";
            
            switch(mode) {
            case 0:
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.TTC.%s.Reset",     (t1Signal.str()).c_str()),0x1);
              return;
            case 1:
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.INTERNAL.%s.Reset",(t1Signal.str()).c_str()),0x1);
              return;
            case 2:
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.EXTERNAL.%s.Reset",(t1Signal.str()).c_str()),0x1);
              return;
            case 3:
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.LOOPBACK.%s.Reset",(t1Signal.str()).c_str()),0x1);
              return;
            case 4:
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.SENT.%s.Reset",    (t1Signal.str()).c_str()),0x1);
              return;
            case 5:
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.TTC.%s.Reset",     (t1Signal.str()).c_str()),0x1);
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.INTERNAL.%s.Reset",(t1Signal.str()).c_str()),0x1);
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.EXTERNAL.%s.Reset",(t1Signal.str()).c_str()),0x1);
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.LOOPBACK.%s.Reset",(t1Signal.str()).c_str()),0x1);
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.SENT.%s.Reset",    (t1Signal.str()).c_str()),0x1);
              return;
            default:
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.TTC.%s.Reset",     (t1Signal.str()).c_str()),0x1);
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.INTERNAL.%s.Reset",(t1Signal.str()).c_str()),0x1);
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.EXTERNAL.%s.Reset",(t1Signal.str()).c_str()),0x1);
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.LOOPBACK.%s.Reset",(t1Signal.str()).c_str()),0x1);
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.T1.SENT.%s.Reset",    (t1Signal.str()).c_str()),0x1);
              return;
            }
          };
	  
          /**
           * Reset recorded number of L1A signals
           * @param mode specifies which L1A counter to reset
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           * 5 all
           **/
          void resetL1ACount(uint8_t const& mode) {
            resetT1Count(0x0,mode);
          };
	  
          /**
           * Reset recorded number of CalPulse signals
           * @param mode specifies which CalPulse counter to reset
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           * 5 all
           **/
          void resetCalPulseCount(uint8_t const& mode) {
            return resetT1Count(0x1, mode); };
          
          /**
           * Reset recorded number of Resync signals
           * @param mode specifies which CalPulse counter to reset
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           * 5 all
           **/
          void resetResyncCount(uint8_t const& mode=0x0) {
            return resetT1Count(0x2, mode); };

          /**
           * Reset recorded number of BC0 signals
           * @param mode specifies which CalPulse counter to reset
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           * 5 all
           **/
          void resetBC0Count(uint8_t const& mode=0x0) {
            return resetT1Count(0x3, mode); };

          /**
           * Reset recorded number of BXCount signals
           * OBSOLETE in V2 firmawre
           **/
          void resetBXCount() { return; };

          //pertaining to VFATs
          /**
           * Returns VFATs to the 0 run mode
           * 
           */
          void resetVFATs() {
            return writeReg(getDeviceBaseNode(),toolbox::toString("CONTROL.VFAT.RESET"),0x1); };
          
          /**
           * Returns the VFAT tracking data mask that the OptoHybrid uses to determine which data
           * packets to send to the GLIB
           * 
           */
          uint32_t getVFATMask() {
            return readReg(getDeviceBaseNode(),toolbox::toString("CONTROL.VFAT.MASK")); };
          
          /**
           * Sets the VFAT tracking data mask that the OptoHybrid uses to determine which data
           *  packets to send to the GLIB
           *  a 0 means the VFAT will NOT be masked, and it's data packets will go to the GLIB
           *  a 1 means the VFAT WILL be masked, and it's data packets will NOT go to the GLIB
           */
          void setVFATMask(uint32_t const& mask) {
            return writeReg(getDeviceBaseNode(),toolbox::toString("CONTROL.VFAT.MASK"),mask); };
          
          /**
           * Sends a read request to all (un-masked) VFATs on the same register
           * @param std::string name name of the register to broadcast the request to
           * @returns a std::vector of uint32_t words, one response for each VFAT
           */
          std::vector<uint32_t> broadcastRead(std::string const& name, uint32_t const& mask, bool reset=false);
          
          /**
           * Sends a write request to all (un-masked) VFATs on the same register
           * @param std::string name name of the register to broadcast the request to
           * @param uint32_t value value to be written to all VFATs receiving the broadcast
           * @returns a std::vector of uint32_t words, one response for each VFAT
           */
          void broadcastWrite(std::string const& name, uint32_t const& mask, uint32_t const& value,
                              bool reset=false);
          
          /**
           * Get the number of valid/incorrect CRCs performed by the OptoHybrid
           * on the received data packets from a given VFAT
           * @param slot specifies which VFAT counters to read
           * 0-23
           **/
          std::pair<uint32_t,uint32_t> getVFATCRCCount(uint8_t const& chip) {
            std::stringstream vfatCRC;
            vfatCRC << "COUNTERS.CRC.";
            uint32_t valid     = readReg(getDeviceBaseNode(),
                                         toolbox::toString("COUNTERS.CRC.VALID.VFAT%d",    chip));
            uint32_t incorrect = readReg(getDeviceBaseNode(),
                                         toolbox::toString("COUNTERS.CRC.INCORRECT.VFAT%d",chip));
            return std::make_pair<uint32_t, uint32_t>(valid,incorrect);
          };
	  
          /**
           * Reset the number of valid/incorrect CRCs performed by the OptoHybrid
           * on the received data packets from a given VFAT
           * @param slot specifies which VFAT counters to read
           * 0-23
           **/
          void resetVFATCRCCount(uint8_t const& chip) {
            std::stringstream vfatCRC;
            vfatCRC << "COUNTERS.CRC.";
            writeReg(getDeviceBaseNode(),
                     toolbox::toString("COUNTERS.CRC.VALID.VFAT%d.Reset",    chip),0x1);
            writeReg(getDeviceBaseNode(),
                     toolbox::toString("COUNTERS.CRC.INCORRECT.VFAT%d.Reset",chip),0x1);
            return;
          };
	  
          uhal::HwInterface& getOptoHybridHwInterface() const {
            return getGEMHwInterface(); };

          std::vector<linkStatus> getActiveLinks() { return v_activeLinks; }
          bool isLinkActive(int i) { return b_links[i]; }

          OptoHybridWBMasterCounters m_wbMasterCounters; /** Wishbone master transaction counters */
          OptoHybridWBSlaveCounters  m_wbSlaveCounters;  /** Wishbone slave transaction counters */
          OptoHybridT1Counters       m_t1Counters;       /** T1 command counters */
          OptoHybridVFATCRCCounters  m_vfatCRCCounters;  /** VFAT CRC counters */

        protected:
          //OptoHybridMonitor *monOptoHybrid_;

          bool b_links[3];
	    
          std::vector<linkStatus> v_activeLinks;

        private:
          uint8_t m_controlLink;
          int m_slot;
	  
        }; //end class HwOptoHybrid
    } //end namespace gem::hw::glib
  } //end namespace gem::hw
} //end namespace gem
#endif
