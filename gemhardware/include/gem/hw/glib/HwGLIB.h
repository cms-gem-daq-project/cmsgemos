/** @file HwGLIB.h */

#ifndef GEM_HW_GLIB_HWGLIB_H
#define GEM_HW_GLIB_HWGLIB_H

#include "gem/hw/HwGenericAMC.h"

#include "gem/hw/glib/exception/Exception.h"
#include "gem/hw/glib/GLIBSettingsEnums.h"

namespace gem {
  namespace hw {
    namespace glib {

      //class GLIBMonitor;

      class HwGLIB: public gem::hw::HwGenericAMC
        {
        public:

          /**
           * Constructors, the preferred constructor is with a connection file and device name
           * as the IP address and address table can be managed there, rather than hard coded
           * Constrution from crateID and slotID uses this constructor as the back end
           */
          /* HwGLIB(); */
          HwGLIB(std::string const& glibDevice, std::string const& connectionFile);
          HwGLIB(std::string const& glibDevice, std::string const& connectionURI,
                 std::string const& addressTable);
          HwGLIB(std::string const& glibDevice, uhal::HwInterface& uhalDevice);
          /* HwGLIB(int const& crate, int const& slot); */

          virtual ~HwGLIB();

          /**
           * Connect to te RPC manager and load necessary modules
           * @param reconnect determine if the conection should be reestablished and the modules reloaded
           void connectRPC(bool reconnect=false) override;
           */

          /**
           * Check if one can read/write to the registers on the GLIB
           * @returns true if the GLIB is accessible
          bool isHwConnected();
           */

          //system core functionality
          /**
           * Read the board ID registers
           * @returns the GLIB board ID
           */
          std::string getBoardIDString();

          /**
           * Read the board ID registers
           * @returns the GLIB board ID as 32 bit unsigned
           */
          uint32_t getBoardID();

          /**
           * Read the system information register
           * @returns a string corresponding to the system ID
           */
          std::string getSystemIDString();

          /**
           * Read the system information register
           * @returns a string corresponding to the system ID as 32 bit unsigned
           */
          uint32_t getSystemID();

          /**
           * Read the IP address register
           * @returns a string corresponding to the dotted quad IP address of the board
           */
          std::string getIPAddressString();

          /**
           * Read the IP address register
           * @returns the IP address of the board as a 32 bit unsigned
           */
          uint32_t getIPAddress();

          /**
           * Read the MAC address register
           * @returns a string corresponding to the MAC address of the board
           */
          std::string getMACAddressString();

          /**
           * Read the MAC address register
           * @returns the MAC address of the board as a 64 bit unsigned
           */
          uint64_t getMACAddress();

          //external clocking control functions
          /**
           * control the PCIe clock
           * @param factor 0 -> OUT = 2.5xIN, 1 -> OUT = 1.25xIN
           * @param reset 1 -> reset, 0 -> normal operation
           * @param enable 0 -> disabled, 1 -> enabled
           * void controlPCIe(uint8_t const& factor);
           */

          /**
           * select the PCIe clock multiplier
           * @param factor 0 -> OUT = 2.5xIN, 1 -> OUT = 1.25xIN
           */
          void PCIeClkFSel(uint8_t const& factor) {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            writeReg(getDeviceBaseNode(),regName.str()+"PCIE_CLK_FSEL",(uint32_t)factor);
          };

          /**
           * get the PCIe clock multiplier
           * @returns the clock multiplier 0 -> OUT = 2.5xIN, 1 -> OUT = 1.25xIN
           */
          uint8_t PCIeClkFSel() {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            return (uint8_t)readReg(getDeviceBaseNode(),regName.str()+"PCIE_CLK_FSEL");
          };

          /**
           * send master reset to the PCIe clock
           * @param reset 1 -> reset, 0 -> normal operation
           */
          void PCIeClkMaster(bool reset) {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            writeReg(getDeviceBaseNode(),regName.str()+"PCIE_CLK_MR",(uint32_t)reset);
          };

          /**
           * get the PCIe clock reset state
           * @returns the clock reset state 0 -> normal mode, 1 -> reset
           */
          uint8_t PCIeClkMaster() {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            return (uint8_t)readReg(getDeviceBaseNode(),regName.str()+"PCIE_CLK_MR");
          };

          /**
           * enable the PCIe clock output
           * @param enable 0 -> disabled, 1 -> enabled
           */
          void PCIeClkOutput(bool enable) {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            writeReg(getDeviceBaseNode(),regName.str()+"PCIE_CLK_OE",(uint32_t)enable);
          };

          /**
           * get the PCIe clock output status
           * @returns the clock output status 0 -> disabled, 1 -> enabled
           */
          uint8_t PCIeClkOutput() {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            return (uint8_t)readReg(getDeviceBaseNode(),regName.str()+"PCIE_CLK_OE");
          };


          /**
           * enable the CDCE
           * @param powerup 0 -> power down, 1 -> power up
           */
          void CDCEPower(bool powerup) {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            writeReg(getDeviceBaseNode(),regName.str()+"CDCE_POWERUP",(uint32_t)powerup);
          };

          /**
           * get the CDCE clock output status
           * @returns the clock output status 0 -> disabled, 1 -> enabled
           */
          uint8_t CDCEPower() {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            return (uint8_t)readReg(getDeviceBaseNode(),regName.str()+"CDCE_POWERUP");
          };

          /**
           * select the CDCE reference clock
           * @param refsrc 0 -> CLK1, 1 -> CLK2
           */
          void CDCEReference(bool refsrc) {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            writeReg(getDeviceBaseNode(),regName.str()+"CDCE_REFSEL",(uint32_t)refsrc);
          };

          /**
           * get the CDCE reference clock
           * @returns the reference clock status 0 -> CLK1, 1 -> CLK2
           */
          uint8_t CDCEReference() {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            return (uint8_t)readReg(getDeviceBaseNode(),regName.str()+"CDCE_REFSEL");
          };

          /**
           * resync the CDCE requires a transition from 0 to 1
           * @param powerup 0 -> power down, 1 -> power up
           */
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
           */
          uint8_t CDCESync() {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            return (uint8_t)readReg(getDeviceBaseNode(),regName.str()+"CDCE_SYNC");
          };

          /**
           * choose who controls the CDCE
           * @param source 0 -> system firmware, 1 -> user firmware
           */
          void CDCEControl(bool source) {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            writeReg(getDeviceBaseNode(),regName.str()+"CDCE_CTRLSEL",(uint32_t)source);
          };

          /**
           * get the CDCE clock output status
           * @returns the cdce control status 0 -> system firmware, 1 -> user firmware
           */
          uint8_t CDCEControl() {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            return (uint8_t)readReg(getDeviceBaseNode(),regName.str()+"CDCE_CTRLSEL");
          };


          /**
           * enable TClkB output to the backplane
           * @param enable 0 -> disabled, 1 -> enabled
           */
          void TClkBOutput(bool enable) {
            std::stringstream regName;
            regName << "SYSTEM.CLK_CTRL.";
            writeReg(getDeviceBaseNode(),regName.str()+"TCLKB_DR_EN",(uint32_t)enable);
          };

          /**
           * get the TClkB output to the backplane status
           * @returns the clock output status 0 -> disabled, 1 -> enabled
           */
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
           * XPoint1 input 2 carries SMA_CLK or the onboard 40MHz oscillator
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
           */
          void XPointControl(bool xpoint2, uint8_t const& input, uint8_t const& output);

          /**
           * get the routing of the XPoint switch
           * @returns the input that is currently routed to a specified output
           */
          uint8_t XPointControl(bool xpoint2, uint8_t const& output);

          /**
           * get the status of the GLIB SFP
           * @param sfpcage
           * @returns the 3 status bits of the specified SFP
           */
          uint8_t SFPStatus(uint8_t const& sfpcage);

          /**
           * get the presence of the FMC in slot 1 or 2
           * @param fmc2 true for FMC2 false for FMC1
           * @returns true if an FMC is in the selected slot
           */
          bool FMCPresence(bool fmc2);

          /**
           * is there a GbE interrupt on the PHY
           * @returns true if there is an interrupt
           */
          bool GbEInterrupt();

          /**
           * returns the state of the FPGA reset line (driven by the CPLD)
           * @returns true if there is a reset
           */
          bool FPGAResetStatus();

          /**
           * returns the status of the 6-bit bus between the FPGA and the CPLD
           * @returns
           */
          uint8_t V6CPLDStatus();

          /**
           * is the CDCE locked
           * @returns true if the CDCE is locked
           */
          bool CDCELockStatus();

          /** Legacy GLIB firmware functionality **/
          /**
           * Read the tracking data FIFO occupancy in terms of raw 32bit words
           * @param uint8_t gtx is the number of the gtx to query
           * @retval uint32_t returns the number of words in the tracking data FIFO
           */
          uint32_t getFIFOOccupancy(uint8_t const& gtx);

          /**
           * Read the tracking data FIFO occupancy in terms of the number of 7x32bit words
           * composing a single VFAT block
           * @param uint8_t gtx is the number of the gtx to query
           * @retval uint32_t returns the number of VFAT blocks in the tracking data FIFO
           */
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
          */
          std::vector<uint32_t> getTrackingData(uint8_t const& gtx, size_t const& nBlocks=1);
          //which of these will be better and do what we want
          uint32_t getTrackingData(uint8_t const& gtx, uint32_t* data, size_t const& nBlocks=1);
          //which of these will be better and do what we want
          /* uint32_t getTrackingData(uint8_t const& gtx, std::vector<toolbox::mem::Reference*>& data, */
          /*                          size_t const& nBlocks=1); */

          /**
           * Empty the tracking data FIFO
           * @param uint8_t gtx is the number of the gtx to query
           *
           */
          void flushFIFO(uint8_t const& gtx);

        protected:
          //GLIBMonitor *monGLIB_;

        private:

        };  // class HwGLIB
    }  // namespace gem::hw::glib
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_GLIB_HWGLIB_H
