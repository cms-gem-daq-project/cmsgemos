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

          /** @struct OptoHybridWBCounters
           *  @brief This struct stores retrieved counters related to the OptoHybrid wishbone transactions
           *  @var OptoHybridWBCounters::GTXStrobg
           *  GTXStrobe is a counter for the number of sent requests on the GTX
           *  @var OptoHybridWBCounters::GTXAck
           *  GTXAck is a counter for the number of received requests on the GTX
           *  @var OptoHybridWBCounters::ExtI2CStrobe
           *  ExtI2CStrobe is a counter for the number of sent requests on the extended I2C
           *  @var OptoHybridWBCounters::ExtI2CAck
           *  ExtI2CAck is a counter for the number of received requests on the extended I2C
           *  @var OptoHybridWBScans::ScanStrobe
           *  ScanStrobe is a counter for the number of sent scan requests
           *  @var OptoHybridWBScans::ScanAck
           *  ScanAck is a counter for the number of received scan requests
           *  @var OptoHybridWBDACs::DACStrobe
           *  DACStrobe is a counter for the number of sent DAC requests
           *  @var OptoHybridWBDACs::DACAck
           *  DACAck is a counter for the number of received DAC requests
           */
          typedef struct OptoHybridWBCounters {
            uint32_t GTXStrobe   ; 
            uint32_t GTXAck      ;
            uint32_t ExtI2CStrobe;
            uint32_t ExtI2CAck   ;
            uint32_t ScanStrobe  ;
            uint32_t ScanAck     ;
            uint32_t DACStrobe   ;
            uint32_t DACAck      ;
            
          OptoHybridWBCounters() : 
            GTXStrobe(0),GTXAck(0),
              ExtI2CStrobe(0),ExtI2CAck(0),
              ScanStrobe(0),ScanAck(0),
              DACStrobe(0),DACAck(0) {};
            void reset() {
              GTXStrobe=0;    GTXAck=0;
              ExtI2CStrobe=0; ExtI2CAck=0;
              ScanStrobe=0;   ScanAck=0;
              DACStrobe=0;    DACAck=0;
              return; };
          } OptoHybridWBCounters;
          
          /** @struct OptoHybridT1Counters
           *  @brief This struct stores retrieved counters related to the OptoHybrid T1 signals
           *  @var OptoHybridT1Counters::AMC13
           *  AMC13 contains the counters for T1 signals received from the GLIB TTC decder
           *  @var OptoHybridT1Counters::Firmware
           *  Firmware contains the counters for T1 signals generated in the firmware module
           *  @var OptoHybridT1Counters::External
           *  External contains the counters for T1 signals received an external sourcd
           *  @var OptoHybridT1Counters::Loopback
           *  Loopback contains the counters for T1 signals looped back onto the system
           *  @var OptoHybridT1Counters::Sent
           *  Sent contains the counters for T1 signals sent by the OptoHybrid to the GEB
           */
          typedef struct OptoHybridT1Counters {
            uint32_t AMC13[4]   ; 
            uint32_t Firmware[4];
            uint32_t External[4];
            uint32_t Loopback[4];
            uint32_t Sent[4]    ;
            
          OptoHybridT1Counters() : 
            AMC13({0,0,0,0}),Firmware({0,0,0,0}),
              External({0,0,0,0}),Loopback({0,0,0,0}),
              Sent({0,0,0,0}) {};
            void reset() {
              AMC13={0,0,0,0};Firmware={0,0,0,0};
              External={0,0,0,0};Loopback={0,0,0,0};
              Sent={0,0,0,0};
              return; };
          } OptoHybridT1Counters;
          
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

          /** Read the firmware register
           * @returns a hex number corresponding to the build date
           **/

          uint32_t getFirmware() {
            uint32_t fwver = readReg(getDeviceBaseNode(),"STATUS.FW");
            DEBUG("OH has firmware version 0x" 
                  << std::hex << fwver << std::dec << std::endl);
            return fwver;
          };
	    
          /** Read the firmware register
           * @returns a string corresponding to the build date
           **/
          std::string getFirmwareDate() {
            std::stringstream retval;
            retval << "0x" << std::hex << getFirmware() << std::dec << std::endl;
            return retval.str();
          };
					
        public:
          /** Read the link status registers, store the information in a struct
           * @retval _status a struct containing the status bits of the optical link
           **/
          GEMHwDevice::OpticalLinkStatus LinkStatus();

          /** Reset the link status registers
           * @param uint8_t resets control which bits to reset
           * bit 1 - TRK_ErrCnt         0x1
           * bit 2 - TRG_ErrCnt         0x2
           * bit 3 - Data_Rec           0x4
           **/
          void LinkReset(uint8_t const& resets);

          /** Reset the all link status registers
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
	  
          /** Read the trigger data
           * @retval uint32_t returns 32 bits 6 bits for s-bits and 26 for bunch countrr
           **/
          //uint32_t readTriggerData();

          //Clocking
          /** Setup the VFAT clock 
           * @param uint8_t source
           * 0x0 on board oscillator
           * 0x1 GTX recovered clock
           * 0x2 external clock
           **/
          void setRefClock(uint8_t const& source) {
            writeReg(getDeviceBaseNode(),"CONTROL.CLOCK.REF_CLK"  ,(uint32_t)source  );
          };

          /** Setup the VFAT clock 
           * @param bool source true uses the external clock, false uses the onboard clock
           * @param bool fallback uses the external clock, false uses the onboard clock
           * NOT YET AVAILABLE IN V2 FIRMWARE
           **/
          void setVFATClock(bool source, bool fallback) {
            std::stringstream regName;
            regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
            writeReg(getDeviceBaseNode(),regName.str()+".CLOCKING.VFAT.SOURCE"  ,(uint32_t)source  );
            writeReg(getDeviceBaseNode(),regName.str()+".CLOCKING.VFAT.FALLBACK",(uint32_t)fallback);
          };

          /** VFAT clock status
           * @param bool source true uses the external clock, false uses the onboard clock
           * @param bool fallback uses the external clock, false uses the onboard clock
           * NOT YET AVAILABLE IN V2 FIRMWARE
           **/
          std::pair<bool,bool> StatusVFATClock() {
            std::stringstream regName;
            regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
            uint32_t src = readReg(getDeviceBaseNode(),regName.str()+".CLOCKING.VFAT.SOURCE");
            uint32_t flb = readReg(getDeviceBaseNode(),regName.str()+".CLOCKING.VFAT.FALLBACK");
            //maybe do a check to ensure that the value has been read properly?
            return std::make_pair(src,flb);
          };

          /** Setup the CDCE clock 
           * @param bool source true uses the external clock, false uses the onboard clock
           * @param bool fallback uses the external clock, false uses the onboard clock
           * NOT YET AVAILABLE IN V2 FIRMWARE
           **/
          void setCDCEClock(bool source, bool fallback) {
            std::stringstream regName;
            regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
            writeReg(getDeviceBaseNode(),regName.str()+".CLOCKING.CDCE.SOURCE"  ,(uint32_t)source  );
            writeReg(getDeviceBaseNode(),regName.str()+".CLOCKING.CDCE.FALLBACK",(uint32_t)fallback);
          };
      
          /** CDCE clock status
           * @param bool source true uses the external clock, false uses the onboard clock
           * @param bool fallback uses the external clock, false uses the onboard clock
           * NOT YET AVAILABLE IN V2 FIRMWARE
           **/
          std::pair<bool,bool> StatusCDCEClock() {
            std::stringstream regName;
            regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
            uint32_t src = readReg(getDeviceBaseNode(),regName.str()+".CLOCKING.CDCE.SOURCE");
            uint32_t flb = readReg(getDeviceBaseNode(),regName.str()+".CLOCKING.CDCE.FALLBACK");
            //maybe do a check to ensure that the value has been read properly?
            return std::make_pair(src,flb);
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


          /** Set the Trigger source
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

          /** Read the Trigger source
           * @retval uint8_t
           * 0 from GLIB TTC decoder
           * 1 from OptoHybrid firmware (T1 module)
           * 2 from external source (LEMO)
           * 3 Internal loopback of s-bits
           * 4 from all
           **/
          uint8_t getTrigSource() { 
            return readReg(getDeviceBaseNode(),"CONTROL.TRIGGER.SOURCE"); };


          /** Set the S-bit source
           * @param uint32_t mask which s-bits to forward (maximum 6)
           **/
          void setSBitSource(uint32_t const& mask) {
            writeReg(getDeviceBaseNode(),"CONTROL.OUTPUT.SBits",mask); };

          /** Read the S-bit source
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
          
          void startT1Generator(uint32_t const& ntrigs, uint32_t const& rate, uint32_t const& delay) {
            //passing ntrigs=0 will send the specified T1 signals continuously continuously
            uint32_t interval = 1/(rate*0.000000025);
            
            writeReg(getDeviceBaseNode(),"T1Controller.NUMBER"  ,ntrigs  );
            writeReg(getDeviceBaseNode(),"T1Controller.INTERVAL",interval);
            writeReg(getDeviceBaseNode(),"T1Controller.DELAY"   ,delay   );
            
            //don't toggle off if the generator is currently running
            if (!statusT1Generator())
              writeReg(getDeviceBaseNode(),"T1Controller.TOGGLE",0x1);
          };
          
          void stopT1Generator(bool reset) {
            //don't toggle on if the generator is currently not running
            if (statusT1Generator())
              writeReg(getDeviceBaseNode(),"T1Controller.TOGGLE",0x1);
            if (reset)
              writeReg(getDeviceBaseNode(),"T1Controller.RESET",0x1);
          };
          
          uint8_t statusT1Generator() {
            return readReg(getDeviceBaseNode(),"T1Controller.MONITOR");
          };
          /* Generate and send specific T1 commands on the OptoHybrid */
          /** Send an internal L1A
           * @param uint32_t ntrigs, how many L1As to send
           * @param uint32_t rate, rate at which signals will be generated
           **/
          void SendL1A(uint32_t const& ntrigs, uint32_t const& rate=1) {
            T1Sequence sequence;
            configureT1Generator(0x0, 0x0, sequence, true);
            startT1Generator(ntrigs,rate, 0);
          };

          /** Send an internal CalPulse
           * @param uint32_t npulse, how many CalPulses to send
           * @param uint32_t rate, rate at which signals will be generated
           **/
          void SendCalPulse(uint32_t const& npulse, uint32_t const& rate=1) {
            T1Sequence sequence;
            configureT1Generator(0x0, 0x1, sequence, true);
            startT1Generator(npulse,rate, 0);
          };
          
          /** Send a CalPulse followed by an L1A
           * @param uint32_t npulse, how many pairs to send
           * @param uint32_t delay, how long between L1A and CalPulse
           * @param uint32_t rate, rate at which signals will be generated
           **/
          void SendL1ACal(uint32_t const& npulse, uint32_t const& delay, uint32_t const& rate=1) {
            T1Sequence sequence;
            configureT1Generator(0x1, 0x0, sequence, true);
            startT1Generator(npulse,rate, delay);
          };

          /** Send an internal Resync
           * @param uint32_t nresync, total number of resync signals to send
           * @param uint32_t rate, rate at which signals will be generated
           * 
           **/
          void SendResync(uint32_t const& nresync=1,uint32_t const& rate=1) {
            T1Sequence sequence;
            configureT1Generator(0x0, 0x2, sequence, true);
            startT1Generator(nresync, rate, 0); };


          /** Send an internal BC0
           * @param uint32_t nbc0, total number of BC0 signals to send
           * @param uint32_t rate, rate at which signals will be generated
           * 
           **/
          void SendBC0(uint32_t const& nbc0=1, uint32_t const& rate=1) {
            T1Sequence sequence;
            configureT1Generator(0x0, 0x3, sequence, true);
            startT1Generator(nbc0, rate, 0); };

          ///Counters
          /** Get the recorded number of T1 signals
           * @param signal specifies which T1 signal counter to read
           * 0 L1A
           * 1 CalPulse
           * 2 Resync
           * 3 BC0
           * @param mode specifies which T1 counter to read
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
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
              return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.TTC.%s",     (t1Signal.str()).c_str()));
            case 1:
              return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.INTERNAL.%s",(t1Signal.str()).c_str()));
            case 2:
              return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.EXTERNAL.%s",(t1Signal.str()).c_str()));
            case 3:
              return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.LOOPBACK.%s",(t1Signal.str()).c_str()));
            case 4:
              return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.SENT.%s",    (t1Signal.str()).c_str()));
            default:
              return readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.SENT.%s",    (t1Signal.str()).c_str()));
            }
          };
	  
          /** Get the recorded number of L1A signals
           * @param mode specifies which L1A counter to read
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           **/
          uint32_t GetL1ACount(uint8_t const& mode) {
            return getT1Count(0x0, mode); };
	  
          /** Get the recorded number of CalPulse signals
           * @param mode specifies which CalPulse counter to read
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           **/
          uint32_t GetCalPulseCount(uint8_t const& mode) {
            return getT1Count(0x1, mode); };
	  
          /** Get the recorded number of Resync signals
           * @param mode specifies which L1A counter to read
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           **/
          uint32_t GetResyncCount(uint8_t const& mode=0x0) {
            return getT1Count(0x2, mode); };

          /** Get the recorded number of BC0 signals
           * @param mode specifies which L1A counter to read
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           **/
          uint32_t GetBC0Count(uint8_t const& mode=0x0) {
            return getT1Count(0x3, mode); };
          
          /** Get the recorded number of BXCount signals
           * OBSOLETE in V2 firmware
           **/
          uint32_t GetBXCountCount() {
            return 0x0;
            /*
            std::stringstream regName;
            regName << "OptoHybrid_LINKS.LINK" << (int)m_controlLink;
            return readReg(getDeviceBaseNode(),regName.str()+".COUNTERS.BXCount");
            */
          };
	  
          ///Resets
          /** Get the recorded number of T1 signals
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
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.TTC.%s.Reset",     (t1Signal.str()).c_str()),0x1);
              return;
            case 1:
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.INTERNAL.%s.Reset",(t1Signal.str()).c_str()),0x1);
              return;
            case 2:
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.EXTERNAL.%s.Reset",(t1Signal.str()).c_str()),0x1);
              return;
            case 3:
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.LOOPBACK.%s.Reset",(t1Signal.str()).c_str()),0x1);
              return;
            case 4:
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.SENT.%s.Reset",    (t1Signal.str()).c_str()),0x1);
              return;
            case 5:
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.TTC.%s.Reset",     (t1Signal.str()).c_str()),0x1);
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.INTERNAL.%s.Reset",(t1Signal.str()).c_str()),0x1);
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.EXTERNAL.%s.Reset",(t1Signal.str()).c_str()),0x1);
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.LOOPBACK.%s.Reset",(t1Signal.str()).c_str()),0x1);
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.SENT.%s.Reset",    (t1Signal.str()).c_str()),0x1);
              return;
            default:
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.TTC.%s.Reset",     (t1Signal.str()).c_str()),0x1);
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.INTERNAL.%s.Reset",(t1Signal.str()).c_str()),0x1);
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.EXTERNAL.%s.Reset",(t1Signal.str()).c_str()),0x1);
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.LOOPBACK.%s.Reset",(t1Signal.str()).c_str()),0x1);
              writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.SENT.%s.Reset",    (t1Signal.str()).c_str()),0x1);
              return;
            }
          };
	  
          /** Reset recorded number of L1A signals
           * @param mode specifies which L1A counter to reset
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           * 5 all
           **/
          void ResetL1ACount(uint8_t const& mode) {
            resetT1Count(0x0,mode);
          };
	  
          /** Get the recorded number of CalPulse signals
           * @param mode specifies which CalPulse counter to reset
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           * 5 all
           **/
          void ResetCalPulseCount(uint8_t const& mode) {
            return resetT1Count(0x1, mode); };
          
          /** Get the recorded number of Resync signals
           * @param mode specifies which CalPulse counter to reset
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           * 5 all
           **/
          void ResetResyncCount(uint8_t const& mode=0x0) {
            return resetT1Count(0x2, mode); };

          /** Get the recorded number of BC0 signals
           * @param mode specifies which CalPulse counter to reset
           * 0 from the TTC decoder on the GLIB
           * 1 from the T1 generator in the firmware
           * 2 from an external source
           * 3 from looping back the sbits
           * 4 sent along the GEB
           * 5 all
           **/
          void ResetBC0Count(uint8_t const& mode=0x0) {
            return resetT1Count(0x3, mode); };

          /** Get the recorded number of BXCount signals
           * OBSOLETE in V2 firmawre
           **/
          void ResetBXCount() { return; };
	  
          uhal::HwInterface& getOptoHybridHwInterface() const {
            return getGEMHwInterface(); };

          std::vector<linkStatus> getActiveLinks() { return v_activeLinks; }
          bool isLinkActive(int i) { return b_links[i]; }

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
