#ifndef gem_hw_glib_HwGLIB_h
#define gem_hw_glib_HwGLIB_h

#include "gem/hw/GEMHwDevice.h"
#include "toolbox/SyncQueue.h"
#include "i2o/i2o.h"
#include "toolbox/Task.h"

#include "gem/hw/glib/exception/Exception.h"
//#include "gem/hw/glib/GLIBMonitor.h"

namespace gem {
  namespace hw {
    namespace glib {
      
      //class GLIBMonitor;
      
      class HwGLIB: public gem::hw::GEMHwDevice
        {
        public:
          
          static const unsigned N_GTX = 2; /**maximum number of GTX links on the GLIB*/

          /**
           * @struct GLIBIPBusCounters
           * @brief This structure stores retrieved counters related to the GLIB IPBus transactions
           * @var GLIBIPBusCounters::OptoHybridStrobe
           * OptoHybridStrobe is a counter for the number of errors on the tracking data link
           * @var GLIBIPBusCounters::OptoHybridAck
           * OptoHybridAck is a counter for the number of errors on the trigger data link
           * @var GLIBIPBusCounters::TrackingStrobe
           * TrackingStrobe is a counter for the number of errors on the tracking data link
           * @var GLIBIPBusCounters::TrackingAck
           * TrackingAck is a counter for the number of errors on the trigger data link
           * @var GLIBIPBusCounters::CounterStrobe
           * CounterStrobe is a counter for the number of errors on the tracking data link
           * @var GLIBIPBusCounters::CounterAck
           * CounterAck is a counter for the number of errors on the trigger data link
           */
          typedef struct GLIBIPBusCounters {
            uint32_t OptoHybridStrobe; 
            uint32_t OptoHybridAck   ;
            uint32_t TrackingStrobe  ;
            uint32_t TrackingAck     ;
            uint32_t CounterStrobe   ;
            uint32_t CounterAck      ;
            
          GLIBIPBusCounters() : 
            OptoHybridStrobe(0),OptoHybridAck(0),
              TrackingStrobe(0),TrackingAck(0),
              CounterStrobe(0),CounterAck(0) {};
            void reset() {
              OptoHybridStrobe=0; OptoHybridAck=0;
              TrackingStrobe=0;   TrackingAck=0;
              CounterStrobe=0;    CounterAck=0;
              return; };
          } GLIBIPBusCounters;
          
          
          /**
           * Constructors, the preferred constructor is with a connection file and device name
           * as the IP address and address table can be managed there, rather than hard coded
           * Constrution from crateID and slotID uses this constructor as the back end
           **/
          HwGLIB();
          HwGLIB(std::string const& glibDevice, std::string const& connectionFile);
          HwGLIB(std::string const& glibDevice, std::string const& connectionURI,
                 std::string const& addressTable);
          HwGLIB(std::string const& glibDevice, uhal::HwInterface& uhalDevice);
          HwGLIB(int const& crate, int const& slot);
	
          virtual ~HwGLIB();

          //virtual void connectDevice();
          //virtual void releaseDevice();
          //virtual void initDevice();
          //virtual void enableDevice();
          //virtual void configureDevice();
          //virtual void configureDevice(std::string const& xmlSettings);
          //virtual void configureDevice(std::string const& dbConnectionString);
          //virtual void disableDevice();
          //virtual void pauseDevice();
          //virtual void startDevice();
          //virtual void stopDevice();
          //virtual void resumeDevice();
          //virtual void haltDevice();

          /**
           * Check if one can read/write to the registers on the GLIB
           * @returns true if the GLIB is accessible
           **/
          bool isHwConnected();

          //system core functionality
          /**
           * Read the board ID registers
           * @returns the GLIB board ID 
           **/
          std::string getBoardID()  ;

          /**
           * Read the board ID registers
           * @returns the GLIB board ID as 32 bit unsigned
           **/
          uint32_t getBoardIDRaw()  ;

          /**
           * Read the system information register
           * @returns a string corresponding to the system ID
           **/
          std::string getSystemID();

          /**
           * Read the system information register
           * @returns a string corresponding to the system ID as 32 bit unsigned
           **/
          uint32_t getSystemIDRaw();

          /**
           * Read the IP address register
           * @returns a string corresponding to the dotted quad IP address of the board
           **/
          std::string getIPAddress();

          /**
           * Read the IP address register
           * @returns the IP address of the board as a 32 bit unsigned
           **/
          uint32_t getIPAddressRaw();

          /**
           * Read the MAC address register
           * @returns a string corresponding to the MAC address of the board
           **/
          std::string getMACAddress();

          /**
           * Read the MAC address register
           * @returns the MAC address of the board as a 64 bit unsigned
           **/
          uint64_t getMACAddressRaw();
          
          /**
           * Read the system firmware register
           * @returns a string corresponding to firmware version
           **/
          std::string getFirmwareVer();

          /**
           * Read the system firmware register
           * @returns the firmware version as a 32 bit unsigned
           **/
          uint32_t getFirmwareVerRaw();

          /**
           * Read the system firmware register
           * @returns a string corresponding to the build date
           **/
          std::string getFirmwareDate();
	  
          /**
           * Read the system firmware register
           * @returns the build date as a 32 bit unsigned
           **/
          uint32_t getFirmwareDateRaw();
	  
          //external clocking control functions
          /**
           * control the PCIe clock
           * @param factor 0 -> OUT = 2.5xIN, 1 -> OUT = 1.25xIN
           * @param reset 1 -> reset, 0 -> normal operation
           * @param enable 0 -> disabled, 1 -> enabled
           * void controlPCIe(uint8_t const& factor);
           **/
	  
          /**
           * select the PCIe clock multiplier
           * @param factor 0 -> OUT = 2.5xIN, 1 -> OUT = 1.25xIN
           **/
          void PCIeClkFSel(uint8_t const& factor) {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            writeReg(getDeviceBaseNode(),regName.str()+"PCIE_CLK_FSEL",(uint32_t)factor);
          };
	  
          /**
           * get the PCIe clock multiplier
           * @returns the clock multiplier 0 -> OUT = 2.5xIN, 1 -> OUT = 1.25xIN
           **/
          uint8_t PCIeClkFSel() {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            return (uint8_t)readReg(getDeviceBaseNode(),regName.str()+"PCIE_CLK_FSEL");
          };
	  
          /**
           * send master reset to the PCIe clock
           * @param reset 1 -> reset, 0 -> normal operation
           **/
          void PCIeClkMaster(bool reset) {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            writeReg(getDeviceBaseNode(),regName.str()+"PCIE_CLK_MR",(uint32_t)reset);
          };
	  
          /**
           * get the PCIe clock reset state
           * @returns the clock reset state 0 -> normal mode, 1 -> reset
           **/
          uint8_t PCIeClkMaster() {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            return (uint8_t)readReg(getDeviceBaseNode(),regName.str()+"PCIE_CLK_MR");
          };
	  
          /**
           * enable the PCIe clock output
           * @param enable 0 -> disabled, 1 -> enabled
           **/
          void PCIeClkOutput(bool enable) {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            writeReg(getDeviceBaseNode(),regName.str()+"PCIE_CLK_OE",(uint32_t)enable);
          };
	  
          /**
           * get the PCIe clock output status
           * @returns the clock output status 0 -> disabled, 1 -> enabled
           **/
          uint8_t PCIeClkOutput() {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            return (uint8_t)readReg(getDeviceBaseNode(),regName.str()+"PCIE_CLK_OE");
          };


          /**
           * enable the CDCE
           * @param powerup 0 -> power down, 1 -> power up
           **/
          void CDCEPower(bool powerup) {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            writeReg(getDeviceBaseNode(),regName.str()+"CDCE_POWERUP",(uint32_t)powerup);
          };
	  
          /**
           * get the CDCE clock output status
           * @returns the clock output status 0 -> disabled, 1 -> enabled
           **/
          uint8_t CDCEPower() {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            return (uint8_t)readReg(getDeviceBaseNode(),regName.str()+"CDCE_POWERUP");
          };

          /**
           * select the CDCE reference clock
           * @param refsrc 0 -> CLK1, 1 -> CLK2
           **/
          void CDCEReference(bool refsrc) {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            writeReg(getDeviceBaseNode(),regName.str()+"CDCE_REFSEL",(uint32_t)refsrc);
          };
	  
          /**
           * get the CDCE reference clock
           * @returns the reference clock status 0 -> CLK1, 1 -> CLK2
           **/
          uint8_t CDCEReference() {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            return (uint8_t)readReg(getDeviceBaseNode(),regName.str()+"CDCE_REFSEL");
          };

          /**
           * resync the CDCE requires a transition from 0 to 1
           * @param powerup 0 -> power down, 1 -> power up
           **/
          void CDCESync(bool sync) {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            if (sync) {
              writeReg(getDeviceBaseNode(),regName.str()+"CDCE_SYNC",0x0);
              writeReg(getDeviceBaseNode(),regName.str()+"CDCE_SYNC",0x1);
            }
          };
	  
          /**
           * get the CDCE syncronization status
           * @returns the cdce sync status 0 -> disabled, 1 -> enabled
           **/
          uint8_t CDCESync() {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            return (uint8_t)readReg(getDeviceBaseNode(),regName.str()+"CDCE_SYNC");
          };

          /**
           * choose who controls the CDCE
           * @param source 0 -> system firmware, 1 -> user firmware
           **/
          void CDCEControl(bool source) {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            writeReg(getDeviceBaseNode(),regName.str()+"CDCE_CTRLSEL",(uint32_t)source);
          };
	  
          /**
           * get the CDCE clock output status
           * @returns the cdce control status 0 -> system firmware, 1 -> user firmware
           **/
          uint8_t CDCEControl() {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            return (uint8_t)readReg(getDeviceBaseNode(),regName.str()+"CDCE_CTRLSEL");
          };


          /**
           * enable TClkB output to the backplane
           * @param enable 0 -> disabled, 1 -> enabled
           **/
          void TClkBOutput(bool enable) {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            writeReg(getDeviceBaseNode(),regName.str()+"TCLKB_DR_EN",(uint32_t)enable);
          };
	  
          /**
           * get the TClkB output to the backplane status
           * @returns the clock output status 0 -> disabled, 1 -> enabled
           **/
          uint8_t TClkBOutput() {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            return (uint8_t)readReg(getDeviceBaseNode(),regName.str()+"TCLKB_DR_EN");
          };
	  
          /**
           * control the xpoint switch
           * @param xpoint2 0 -> xpoint1, 1 -> xpoint2
           * @param input selects the XPoint switch clock input to route
           * @param output selects the XPoint switch clock output to route
           * \brief Fucntion to control the routing of the two XPoint switches
           *
           * XPoint1 inputs 1-4 can be routed to outputs 1-4
           * XPoint1 input 1 carries XPoint2 output 1
           * XPoint1 input 2 carries SMA_CLK or the onboard 40MhZ oscillator
           * XPoint1 input 3 carries TCLKA
           * XPoint1 input 4 carries FCLKA
           *
           * XPoint1 output 1 routes to IO_L0_GC_34
           * XPoint1 output 2 routes to TP
           * XPoint1 output 3 routes to IO_L1_GC_34
           * XPoint1 output 4 routes to CDCE PRI_CLK
           *
           * XPoint2 can only have output 1 controlled (routing options are inputs 1-3 to output 1
           *
           * XPoint2 inputs 1-4 can be routed to outputs 1-4
           * XPoint2 input 1 carries FMC1_CLK0_M2C
           * XPoint2 input 2 carries FMC2_CLK0_M2C
           * XPoint2 input 3 carries TCLKC
           * XPoint2 input 4 carries nothing
           *
           * XPoint2 output 1 routes to XPoint1 input 1
           * XPoint2 output 2 routes to IO_L18_GC_35
           * XPoint2 output 3 routes to MGT113REFCLK1
           * XPoint2 output 4 routes nowhere
           **/
          void XPointControl(bool xpoint2, uint8_t const& input, uint8_t const& output);
	  
          /**
           * get the routing of the XPoint switch
           * @returns the input that is currently routed to a specified output
           **/
          uint8_t XPointControl(bool xpoint2, uint8_t const& output);
	  
          /**
           * get the status of the GLIB SFP
           * @param sfpcage
           * @returns the 3 status bits of the specified SFP
           **/
          uint8_t SFPStatus(uint8_t const& sfpcage);
	  
          /**
           * get the presence of the FMC in slot 1 or 2
           * @param fmc2 true for FMC2 false for FMC1
           * @returns true if an FMC is in the selected slot
           **/
          bool FMCPresence(bool fmc2);
	  
          /**
           * is there a GbE interrupt on the PHY
           * @returns true if there is an interrupt
           **/
          bool GbEInterrupt();
	  
          /**
           * returns the state of the FPGA reset line (driven by the CPLD)
           * @returns true if there is a reset
           **/
          bool FPGAResetStatus();
	  
          /**
           * returns the status of the 6-bit bus between the FPGA and the CPLD
           * @returns 
           **/
          uint8_t V6CPLDStatus();
	  
          /**
           * is the CDCE locked
           * @returns true if the CDCE is locked
           **/
          bool CDCELockStatus();
	  
	  
          //user core functionality
          /**
           * Read the user firmware register using m_controlLink
           * @returns a hex number corresponding to the build date
           * OBSOLETE in V2 firmware
           **/
          uint32_t getUserFirmware();
	  
          /**
           * Read the user firmware register using m_controlLink
           * @returns a std::string corresponding to the build date
           * OBSOLETE in V2 firmware
           **/
          std::string getUserFirmwareDate();

        private:
          /**
           * Read the user firmware register for a given gtx
           * @returns a hex number corresponding to the build date
           * is private to ensure that it is only used internally
           * gtx agnostic versions should be used outside of HwGLIB
           * OBSOLETE in V2 firmware
           **/
          uint32_t getUserFirmware(uint8_t const& gtx);
	  
          /**
           * Read the user firmware register for a given gtx
           * @returns a string corresponding to the build date
           * is private to ensure that it is only used internally
           * gtx agnostic versions should be used outside of HwGLIB
           * OBSOLETE in V2 firmware
           **/
          std::string getUserFirmwareDate(uint8_t const& gtx);
	  
          /**
           * Check if the gtx requested is known to be operational
           * @param uint8_t gtx GTX gtx to be queried
           * @param std::string opMsg Operation message to append to the log message
           * @returns true if the gtx is in range and active, false otherwise
           **/
          bool linkCheck(uint8_t const& gtx, std::string const& opMsg);

        public:
          /**
           * Read the gtx status registers, store the information in a struct
           * @param uint8_t gtx is the number of the gtx to query
           * @retval _status a struct containing the status bits of the optical link
           * @throws gem::hw::glib::exception::InvalidLink if the gtx number is outside of 0-1
           **/
          GEMHwDevice::OpticalLinkStatus LinkStatus(uint8_t const& gtx);
	  
          /**
           * Reset the gtx status registers
           * @param uint8_t gtx is the number of the gtx to query
           * @param uint8_t resets control which bits to reset
           * bit 1 - TRK_ErrCnt         0x1
           * bit 2 - TRG_ErrCnt         0x2
           * bit 3 - Data_Rec           0x4
           * @throws gem::hw::glib::exception::InvalidLink if the gtx number is outside of 0-1
           **/
          void LinkReset(uint8_t const& gtx, uint8_t const& resets);

          /**
           * Reset the all gtx status registers
           * @param uint8_t resets control which bits to reset
           **/
          void ResetLinks(uint8_t const& resets) {
            for (auto gtx = v_activeLinks.begin(); gtx != v_activeLinks.end(); ++gtx)
              LinkReset(gtx->first,resets);
          };
	  
          /**
           * Set the Trigger source
           * @param uint8_t mode 0 from software, 1 from TTC decoder (AMC13), 2 from both
           * OBSOLETE in V2 firmware, taken care of in the OptoHybrid
           **/
          void setTrigSource(uint8_t const& mode, uint8_t const& gtx=0x0) {
            return;
            /*
            std::stringstream regName;
            regName << "GLIB_LINKS.LINK" << (int)m_controlLink;
            switch (mode) {
            case(0):
              writeReg(getDeviceBaseNode(),regName.str()+".TRIGGER.SOURCE",mode);
              return;
            case(1):
              writeReg(getDeviceBaseNode(),regName.str()+".TRIGGER.SOURCE",mode);
              return;
            case(2):
              writeReg(getDeviceBaseNode(),regName.str()+".TRIGGER.SOURCE",mode);
              return;
            default:
              writeReg(getDeviceBaseNode(),regName.str()+".TRIGGER.SOURCE",0x2);
              return;
              }*/
          };

          /**
           * Read the Trigger source
           * @retval uint8_t 0 from GLIB, 1 from AMC13, 2 from both
           * OBSOLETE in V2 firmware, taken care of in the OptoHybrid
           **/
          uint8_t getTrigSource(uint8_t const& gtx=0x0) {
            return 0;
            /*
            std::stringstream regName;
            regName << "GLIB_LINKS.LINK" << (int)m_controlLink;
            return readReg(getDeviceBaseNode(),regName.str()+".TRIGGER.SOURCE");
            */
          };

          /**
           * Set the S-bit source
           * @param uint8_t chip
           * OBSOLETE in V2 firmware
           **/
          void setSBitSource(uint8_t const& mode, uint8_t const& gtx=0x0) {
            return;
            /*
            std::stringstream regName;
            regName << "GLIB_LINKS.LINK" << (int)m_controlLink;
            writeReg(getDeviceBaseNode(),regName.str()+".TRIGGER.TDC_SBits",mode);
            */
          };

          /**
           * Read the S-bit source
           * @retval uint8_t which VFAT chip is sending the S-bits
           * OBSOLETE in V2 firmware
           **/
          uint8_t getSBitSource(uint8_t const& gtx=0x0) {
            return 0;
            /*
            std::stringstream regName;
            regName << "GLIB_LINKS.LINK" << (int)m_controlLink;
            return readReg(getDeviceBaseNode(),regName.str()+".TRIGGER.TDC_SBits");
            */
          };

          ///Counters
          /**
           * Get the recorded number of IPBus signals sent/received by the GLIB
           * @param uint8_t gtx which GTX
           * @param uint8_t mode which counter
           * bit 1 OptoHybridStrobe
           * bit 2 OptoHybridAck
           * bit 3 TrackingStrobe
           * bit 4 TrackingAck
           * bit 5 CounterStrobe
           * bit 6 CounterAck
           * @returns GLIBIPBusCounters struct, with updated values for the ones specified in the mask
           **/
          GLIBIPBusCounters getIPBusCounters(uint8_t const& gtx, uint8_t const& mode);
	  
          /**
           * Get the recorded number of L1A signals received from the TTC decoder
           **/
          uint32_t getL1ACount() {
            return readReg(getDeviceBaseNode(),"COUNTERS.T1.L1A"); };
	  
          /**
           * Get the recorded number of CalPulse signals received from the TTC decoder
           **/
          uint32_t getCalPulseCount() {
            return readReg(getDeviceBaseNode(),"COUNTERS.T1.CalPulse"); };
	  
          /**
           * Get the recorded number of Resync signals received from the TTC decoder
           **/
          uint32_t getResyncCount() {
            return readReg(getDeviceBaseNode(),"COUNTERS.T1.Resync"); };
          
          /**
           * Get the recorded number of BC0 signals
           **/
          uint32_t getBC0Count() {
            return readReg(getDeviceBaseNode(),"COUNTERS.T1.BC0"); };
          
          ///Counter resets
          /**
           * Get the recorded number of IPBus signals sent/received by the GLIB
           * @param uint8_t gtx which GTX
           * @param uint8_t mode which counter
           * bit 1 OptoHybridStrobe
           * bit 2 OptoHybridAck
           * bit 3 TrackingStrobe
           * bit 4 TrackingAck
           * bit 5 CounterStrobe
           * bit 6 CounterAck
           **/
          void resetIPBusCounters(uint8_t const& gtx, uint8_t const& mode);
	  
          /**
           * Reset the recorded number of L1A signals received from the TTC decoder
           **/
          void resetL1ACount() {
            return writeReg(getDeviceBaseNode(),"COUNTERS.T1.L1A.Reset", 0x1); };
	  
          /**
           * Reset the recorded number of CalPulse signals received from the TTC decoder
           **/
          void resetCalPulseCount() {
            return writeReg(getDeviceBaseNode(),"COUNTERS.T1.CalPulse.Reset", 0x1); };
	  
          /**
           * Reset the recorded number of Resync signals received from the TTC decoder
           **/
          void resetResyncCount() {
            return writeReg(getDeviceBaseNode(),"COUNTERS.T1.Resync.Reset", 0x1); };
          
          /**
           * Reset the recorded number of BC0 signals
           **/
          void resetBC0Count() {
            return writeReg(getDeviceBaseNode(),"COUNTERS.T1.BC0.Reset", 0x1); };
          
          /**
           * Read the trigger data
           * @retval uint32_t returns 32 bits 6 bits for s-bits and 26 for bunch countrr
           **/
          uint32_t readTriggerFIFO(uint8_t const& gtx);

          /**
           * Empty the trigger data FIFO
           **/
          void flushTriggerFIFO(uint8_t const& gtx);

          /**
           * Read the tracking data FIFO occupancy in terms of raw 32bit words
           * @param uint8_t gtx is the number of the gtx to query
           * @retval uint32_t returns the number of words in the tracking data FIFO
           **/
          uint32_t getFIFOOccupancy(uint8_t const& gtx);

          /**
           * Read the tracking data FIFO occupancy in terms of the number of 7x32bit words
           * composing a single VFAT block
           * @param uint8_t gtx is the number of the gtx to query
           * @retval uint32_t returns the number of VFAT blocks in the tracking data FIFO
           **/
          uint32_t getFIFOVFATBlockOccupancy(uint8_t const& gtx);

          /**
           * see if there is tracking data available
           * @param uint8_t gtx is the number of the column of the tracking data to read
           * @retval bool returns true if there is tracking data in the FIFO
           TRK_DATA.COLX.DATA_RDY
          */
          bool hasTrackingData(uint8_t const& gtx);

          /**
           * get the tracking data, have to do this intelligently, as IPBus transactions are expensive
           * and need to pack all events together
           * @param uint8_t gtx is the number of the GTX tracking data to read
           * @param size_t nBlocks is the number of VFAT data blocks (7*32bit words) to read
           * @retval std::vector<uint32_t> returns the 7*nBlocks data words in the buffer
          **/
          std::vector<uint32_t> getTrackingData(uint8_t const& gtx, size_t const& nBlocks=1);
          //which of these will be better and do what we want
          uint32_t getTrackingData(uint8_t const& gtx, uint32_t* data, size_t const& nBlocks=1);
          //which of these will be better and do what we want
          uint32_t getTrackingData(uint8_t const& gtx, std::vector<toolbox::mem::Reference*>& data,
                                   size_t const& nBlocks=1);
          
          /**
           * Empty the tracking data FIFO
           * @param uint8_t gtx is the number of the gtx to query
           * 
           **/
          void flushFIFO(uint8_t const& gtx);

          // DAQ LINK functionality
          void enableDAQLink();
          uint32_t getDAQLinkControl();
          uint32_t getDAQLinkStatus();
          uint32_t getDAQLinkFlags();
          uint32_t getDAQLinkCorruptCount();
          uint32_t getDAQLinkEventsBuilt();
          uint32_t getDAQLinkEventsSent();
          uint32_t getDAQLinkL1AID();
          uint32_t getDAQLinkDebug(uint8_t const& mode);
          uint32_t getDAQLinkDisperErrors();
          uint32_t getDAQLinkNonidentifiableErrors();

          std::vector<GLIBIPBusCounters> m_ipBusCounters; /** for each gtx, IPBus counters */
          
        protected:
          //GLIBMonitor *monGLIB_;
	
          bool b_links[N_GTX];
          
          std::vector<linkStatus> v_activeLinks;

        private:
          uint8_t m_controlLink;
          int m_crate, m_slot;
	
        }; //end class HwGLIB
    } //end namespace gem::hw::glib
  } //end namespace gem::hw
} //end namespace gem
#endif
