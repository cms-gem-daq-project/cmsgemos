/** @file HwOptoHybrid.h */

#ifndef GEM_HW_OPTOHYBRID_HWOPTOHYBRID_H
#define GEM_HW_OPTOHYBRID_HWOPTOHYBRID_H

#include "gem/hw/GEMHwDevice.h"
#include "gem/hw/HwGenericAMC.h"
/* #include "gem/hw/vfat/HwVFAT2.h" */

#include "gem/hw/optohybrid/exception/Exception.h"
#include "gem/hw/optohybrid/OptoHybridSettingsEnums.h"

namespace gem {
  namespace hw {
    /* namespace vfat { */
    /*   class HwVFAT2; */
    /* } */

    namespace optohybrid {

      // class OptoHybridMonitor;
      class HwOptoHybrid: public gem::hw::GEMHwDevice
        {
        public:
          HwOptoHybrid(std::string const& optohybridDevice, std::string const& connectionFile);
          HwOptoHybrid(std::string const& optohybridDevice, std::string const& connectionURI,
                       std::string const& addressTable);
          HwOptoHybrid(std::string const& optohybridDevice, uhal::HwInterface& uhalDevice);
          HwOptoHybrid(gem::hw::HwGenericAMC const& amc,
                       uint8_t               const& slot);

          /*
          // constructors from existing GEM hardware devices
          HwOptoHybrid(uhal::HwInterface& device, uint8_t const& slot, uint8_t const& olink, uint8_t const& );
          HwOptoHybrid(GEMHwDevice  const& gemDevice,  uint8_t const& slot, uint8_t const& olink);
          HwOptoHybrid(HwGLIB       const& glibDevice, uint8_t const& olink);
          */

          /*
          // constructors from existing GEM hardware devices
          HwOptoHybrid(uhal::HwInterface& device, uint8_t const& slot, uint8_t const& olink, uint8_t const& );
          HwOptoHybrid(GEMHwDevice  const& gemDevice,  uint8_t const& slot, uint8_t const& olink);
          HwOptoHybrid(HwGLIB       const& glibDevice, uint8_t const& olink);
          */

          /*
          // constructors from existing GEM hardware devices
          HwOptoHybrid(uhal::HwInterface& device, uint8_t const& slot, uint8_t const& olink, uint8_t const& );
          HwOptoHybrid(GEMHwDevice  const& gemDevice,  uint8_t const& slot, uint8_t const& olink);
          HwOptoHybrid(HwGLIB       const& glibDevice, uint8_t const& olink);
          */

          virtual ~HwOptoHybrid();

          /**
           * Connect to te RPC manager and load necessary modules
           * @param reconnect determine if the conection should be reestablished and the modules reloaded
           */
          virtual void connectRPC(bool reconnect=false) override;

          virtual bool isHwConnected() override;

          /** Read the board ID registers
           * @returns the OptoHybrid board ID
           */
          //std::string getBoardID()   const;

          /**
           * Read the firmware register
           * @returns a hex number corresponding to the release version
           */

          uint32_t getFirmwareVersion() {
            uint32_t fwver = readReg(getDeviceBaseNode(),"FPGA.CONTROL.RELEASE.VERSION");
            CMSGEMOS_TRACE("OH has firmware version 0x" << std::hex << fwver << std::dec << std::endl);
            return fwver;
          };

          /**
           * Read the firmware register
           * @returns a string corresponding to the firmware maj.min.ver.generation
           */
          std::string getFirmwareVersionString() {
            std::stringstream retval;
            uint32_t fwver = getFirmwareVersion();
            retval << std::hex
                   << ((fwver)     & 0xff) << "."
                   << ((fwver>>8)  & 0xff) << "."
                   << ((fwver>>16) & 0xff) << "."
                   << ((fwver>>24) & 0xff)
                   << std::dec;
            return retval.str();
          };

          /**
           * Read the firmware register
           * @returns a hex number corresponding to the build date
           */
          uint32_t getFirmwareDate() {
            uint32_t fwver = readReg(getDeviceBaseNode(),"FPGA.CONTROL.RELEASE.DATE");
            CMSGEMOS_TRACE("OH has firmware date 0x" << std::hex << fwver << std::dec << std::endl);
            return fwver;
          };

          /**
           * Read the firmware register
           * @returns a string corresponding to the build date in the format dd/mm/yyyy
           */
          std::string getFirmwareDateString() {
            std::stringstream retval;
            uint32_t rawDate = getFirmwareDate();
            retval << std::hex << std::setw(2) << std::setfill('0') << ((rawDate)&0xff)       << std::dec << "/"
                   << std::hex << std::setw(2) << std::setfill('0') << ((rawDate>>8)&0xff)    << std::dec << "/"
                   << std::hex << std::setw(4) << std::setfill('0') << ((rawDate>>16)&0xffff) << std::dec;
            return retval.str();
          };

        public:

          ///// Inherited from GEMHwDevice
          /**
           * @brief performs a general reset of the AMC
           */
          virtual void generalReset() override;

          /**
           * @brief performs a reset of the AMC counters
           */
          virtual void counterReset() override;

          /**
           * @brief performs a reset of the AMC link
           * @param link is the link to perform the reset on
           */
          virtual void linkReset(uint8_t const& link) override;


          /////////  Specific to the OptoHybrid board  \\\\\\\\\\\*

          /**
           * OptoHybrid CONTROL module
           * Functions realted to the registers in the CONTROL block
           * in the OptoHybrid firmware
           **/

          //// VFAT \\\\*
          /**
           * @brief Returns the VFAT tracking data/slow control mask
           *        A 1 means that neither slow control nor tracking data will be possible with the specified VFAT
           *
           */
          uint32_t getVFATMask() {
            return readReg(getDeviceBaseNode(),"VFAT_MASK"); };

          /**
           * @brief Sets the VFAT tracking data/slow control mask
           * @param mask is the mask to apply
           *        - 0 means the VFAT will *not* be masked
           *        - 1 means the VFAT *will* be masked
           */
          void setVFATMask(uint32_t const mask) {
            CMSGEMOS_DEBUG("HwOptoHybrid::setVFATMask setting VFAT mask to "
                  << std::hex << std::setw(8) << std::setfill('0') << mask << std::dec);
            return writeReg(getDeviceBaseNode(),"VFAT_MASK", mask&0x00ffffff); };

          /**
           * @brief Returns VFATs to the 0 run mode
           *
           */
          void resetVFATs() {
            return writeReg(getDeviceBaseNode(),"FPGA.CONTROL.VFAT.RESET",0x1); };

          /**
           * Set the S-bit mask
           * @param uint32_t mask s-bits coming from specific GEB slots
           
           */
          void setSBitMask(uint32_t const mask) {
            writeReg(getDeviceBaseNode(),"FPGA.TRIG.CTRL.VFAT_MASK", mask&0x00ffffff); };

          /**
           * Read the S-bit mask
           * @retval uint32_t which slots s-bits are processed
           */
          uint32_t getSBitMask() {
            return readReg(getDeviceBaseNode(),"FPGA.TRIG.CTRL.VFAT_MASK"); };

          //// TRIGGER \\\\*
          // FIXME OBSOLETE?
          /* void setTrigSource(uint8_t const& mode); */
          /* uint8_t getTrigSource(); */
          /* void setLoopbackSBitSource(uint8_t const& sbit); */
          /* uint8_t getLoopbackSBitSource(); */
          /* void setTriggerThrottle(uint32_t const& throttle); */
          /* uint8_t getTriggerThrottle(); */

          //// HDMI_OUTPUT \\\\*
          /**
           * FIXME NEEDS REVISION
           * Set the HDMI output s-bit source
           * @param uint32_t mask which s-bits to forward (maximum 6)
           */
          void setHDMISBitSource(uint8_t const& bit, uint8_t const& mode) {
            writeReg(getDeviceBaseNode(),toolbox::toString("FPGA.CONTROL.HDMI.SBIT_SEL%i",static_cast<uint32_t>(bit)),mode); };

          /**
           * FIXME NEEDS REVISION
           * Set the HDMI output s-bit source
           * @param std::array<uint8_t, 8> which s-bits to forward (maximum 8)
           */
          void setHDMISBitSource(std::array<uint8_t, 8> const sources) {
            for (int i = 0; i < 8; ++i)
              setHDMISBitSource(i, sources[i]);
          };

          /**
           * FIXME NEEDS REVISION
           * Read the HDMI output s-bit source
           * @retval uint32_t which VFAT chips are sending s-bits
           */
          uint32_t getHDMISBitSource(uint8_t const& bit) {
            return readReg(getDeviceBaseNode(),toolbox::toString("FPGA.CONTROL.HDMI.SBIT_SEL%i",static_cast<uint32_t>(bit))); };

          /**
           * FIXME NEEDS REVISION
           * Set the HDMI output s-bit mode
           * @param uint32_t mode of sending s-bits out the HDMI connector
           *        if mode is 0 - the SBITS parameter will correspond to a given VFAT [0-23]
           *        if mode is 1 - the SBITS parameter will correspond to a given iEta sector [0-7]
           *        if mode is 2 - the output sbits will correspond to the I2C sector
           *        if mode is 3 - the output sbits will be constant 0's
           */
          void setHDMISBitMode(uint8_t const& bit, uint8_t const& mode) {
            writeReg(getDeviceBaseNode(),toolbox::toString("FPGA.CONTROL.HDMI.SBIT_MODE%i",static_cast<uint32_t>(bit)), mode); };

          /**
           * FIXME NEEDS REVISION
           * Read the HDMI output s-bit mode
           * @retval uint32_t which mode the OptoHybrid is sending s-bits to the HDMI connector
           */
          uint32_t getHDMISBitMode(uint8_t const& bit) {
            return readReg(getDeviceBaseNode(),toolbox::toString("FPGA.CONTROL.HDMI.SBIT_MODE%i",static_cast<uint32_t>(bit))); };

          //// Clocking \\\\*
          /**
           * @brief Get the clocking status information
           */
          uint32_t getClockStatus() {
            return readReg(getDeviceBaseNode(),"FPGA.CLOCKING");
          };

          /**
           * OptoHybrid STATUS module
           * Functions realted to the registers in the STATUS block
           * in the OptoHybrid firmware
           **/
          // FW


          /**
           * @returns whether there is a critical error detected due to SEU
           */
          bool hasSEUError() { return readReg(getDeviceBaseNode(),"STATUS.SEU"); };


          /** @defgroup ohlocalttc Optohybrid local TTC Command Generator
           *  Functions related to the OptoHybrid TTC controller module
           *  @{
           */
          /**
           * Send an internal L1A
           * @param uint32_t ntrigs, how many L1As to send
           * @param uint32_t rate, rate at which signals will be generated
           */
          void sendL1A(uint32_t const& ntrigs, uint32_t const& rate=1);

          /**
           * Send an internal Resync
           * @param uint32_t ncmd, total number of resync signals to send
           * @param uint32_t rate, rate at which signals will be generated
           */
          void sendResync(uint32_t const& ncmd=1, uint32_t const& rate=1);

          /**
           * Send an internal BC0
           * @param uint32_t ncmd, total number of BC0 signals to send
           * @param uint32_t rate, rate at which signals will be generated
           */
          void sendBC0(uint32_t const& ncmd=1, uint32_t const& rate=1);

          /**
           * Send an internal BXN
           * @param uint32_t ncmd, total number of BXN signals to send
           * @param uint32_t rate, rate at which signals will be generated
           */
          void sendBXN(uint32_t const& ncmd=1, uint32_t const& rate=1);
          /** @} */ // end of ohlocalttc


          /** @defgroup adcinterface Optohybrid ADC Interface
           *  Functions related to the OptoHybrid ADC interface
           *  @{
           */

          /**
           * Read the FPGA temperature from the ADC
           * @returns uint32_t raw ADC temperature value from FPGA
           */
          uint32_t getFPGATemp() {
            return readReg(getDeviceBaseNode(),"ADC.TEMP"); };

          /**
           * Read the maximum FPGA temperature from the ADC
           * @returns uint32_t raw ADC maximum temperature value from FPGA
           */
          uint32_t getFPGAMaxTemp() {
            return readReg(getDeviceBaseNode(),"ADC.TEMP.MAX"); };

          /**
           * Read the minimum FPGA temperature from the ADC
           * @returns uint32_t raw ADC minimum temperature value from FPGA
           */
          uint32_t getFPGAMinTemp() {
            return readReg(getDeviceBaseNode(),"ADC.TEMP.MIN"); };

          /**
           * Read the FPGA Vcc int from the ADC
           * @returns uint32_t raw ADC internal Vcc value from FPGA
           */
          uint32_t getFPGAVccInt() {
            return readReg(getDeviceBaseNode(),"ADC.VCCINT"); };

          /**
           * Read the maximum FPGA Vcc int from the ADC
           * @returns uint32_t raw ADC maximum internal Vcc value from FPGA
           */
          uint32_t getFPGAMaxVccInt() {
            return readReg(getDeviceBaseNode(),"ADC.VCCINT.MAX"); };

          /**
           * Read the minimum FPGA Vcc int from the ADC
           * @returns uint32_t raw ADC minimum internal Vcc value from FPGA
           */
          uint32_t getFPGAMinVccInt() {
            return readReg(getDeviceBaseNode(),"ADC.VCCINT.MIN"); };

          /**
           * Read the FPGA Vcc aux from the ADC
           * @returns uint32_t raw ADC auxiliary Vcc value from FPGA
           */
          uint32_t getFPGAVccAux() {
            return readReg(getDeviceBaseNode(),"ADC.VCCAUX"); };

          /**
           * Read the maximum FPGA Vcc aux from the ADC
           * @returns uint32_t raw ADC maximum auxiliary Vcc value from FPGA
           */
          uint32_t getFPGAMaxVccAux() {
            return readReg(getDeviceBaseNode(),"ADC.VCCAUX.MAX"); };

          /**
           * Read the minimum FPGA Vcc aux from the ADC
           * @returns uint32_t raw ADC minimum auxiliary Vcc value from FPGA
           */
          uint32_t getFPGAMinVccAux() {
            return readReg(getDeviceBaseNode(),"ADC.VCCAUX.MIN"); };

          /**
           * Read the VP/VN from the ADC
           * @returns uint32_t raw ADC VP/VN value
           */
          uint32_t getADCVPVN() {
            return readReg(getDeviceBaseNode(),"ADC.VPVN"); };

          /**
           * Read the VAUX input specified from the ADC
           * @param uint8_t specify the VAUX input to read
           * @returns uint32_t raw ADC VAUX value
           */
          uint32_t getADCVAUX(uint8_t const& vaux);

          /**
           * Read the DAC output voltage value from the VFAT on the specified column
           * @param uint8_t specify the column
           * @returns uint32_t raw VFAT DACo-V value
           */
          uint32_t getVFATDACOutV(uint8_t const& column);

          /**
           * Read the DAC output current value from the VFAT on the specified column
           * @param uint8_t specify the column
           * @returns uint32_t raw VFAT DACo-I value
           */
          uint32_t getVFATDACOutI(uint8_t const& column);
          /** @} */ // end of adcinterface


          /** @defgroup scanmodule Scan Modules
           *  Functions related to the OptoHybrid firmware scan modules
           *  @{
           */
          /**
           * @brief This function controls the firmware module that runs any of the scans
           * @param uint8_t mode can be any of
           *  - 0 Threshold scan per VFAT with trigger data
           *  - 1 Threshold scan per channel using tracking data
           *  - 2 Latency scan per VFAT
           *  - 3 S-curve scan per channel
           *  - 4 Threshold scan per VFAT with tracking data
           * @param uint8_t min is the minimum value to start the scan at (depending on the mode selected)
           *  - 0 VT1
           *  - 1 VT1
           *  - 2 Latency
           *  - 3 VCal
           *  - 4 VT1
           * @param uint8_t max is the maximum value to start the scan at (depending on the mode selected)
           *  - 0 VT1
           *  - 1 VT1
           *  - 2 Latency
           *  - 3 VCal
           *  - 4 VT1
           * @param uint8_t step is the size of the step between successive points
           * @param uint8_t chip is the VFAT to run the scan on (if useUltra is true, this will be the mask)
           * @param uint8_t channel is the channel to run the scan on (for modes 1 and 3 only)
           * @param uint8_t min is the minimum value of the parameter to scan from
           * @param uint8_t max is the maximum value of the paramter to scan to (must be greater than min)
           * @param uint8_t step is the step size between successive scan points
           * @param uint32_t nevts number of events to process before moving to the next scan point
           * @param bool useUltra says whether to use the 24 VFATs in parallel mode (default is true)
           * @param bool reset says whether to reset the module or not (default is false)
           */
          void configureScanModule(uint8_t const& mode, uint8_t const& chip, uint8_t const& channel,
                                   uint8_t const& min,  uint8_t const& max,
                                   uint8_t const& step, uint32_t const& nevts,
                                   bool useUltra=true, bool reset=false);

          /**
           * @brief Start the Scan controller (must be configured first or have a configuration already loaded)
           * @param uint32_t evts number of events to process before moving to the next scan point
           * @param bool useUltra says whether to use the 24 VFATs in parallel mode (default is true)
           */
          void startScanModule(uint32_t const& nevts, bool useUltra=true);

          /**
           * @brief Stop the Scan module
           * @param bool useUltra says whether to use the 24 VFATs in parallel mode (default is true)
           * @param bool reset tells whether to reset the state of the module
           */
          void stopScanModule(bool useUltra=true, bool reset=false);

          /**
           * @brief Status of the Scan module
           * @returns uint8_t the status of the Scan module, telling which mode is running
           * (0 is nothing running)
           */
          uint8_t getScanStatus(bool useUltra=true);

          /**
           * @brief Get the results of the Scan controller
           * @returns std::vector<uint32_t> the data from the scan
           *          data format is 0xYYZZZZZZ where
           *          YY is the scan parameter value
           *          ZZZZZZ is the number of events seen at that point
           */
          std::vector<uint32_t> getScanResults(uint32_t const& npoints);

          /**
           * @brief Get the results of the ULTRA Scan controller
           * @returns std::vector<std::vector<uint32_t> > the data from the scan
           *          one set of results for each VFAT
           *          data format is 0xYYZZZZZZ where
           *          YY is the scan parameter value
           *          ZZZZZZ is the number of events seen at that point
           */
          std::vector<std::vector<uint32_t> > getUltraScanResults(uint32_t const& npoints);
          /** @} */ // end of scanmodule


          /**
           * @defgroup ohcounters OptoHybrid Counters
           * Functions related to the various counters on the OptoHybrid
           */

          /**
           * @ingroup ohcounters
           * FIXME UPDATE FOR V3
           * Get the recorded number of TTC signals sent/received by the OptoHybrid
           * @returns OptoHybridTTCCounters struct, with updated values
           */
          /* OptoHybridTTCCounters getTTCCounters() { return m_t1Counters; }; */
          void updateTTCCounters();
          void resetTTCCounters();

          /** @defgroup t1counters TTC Counters
           *  Functions related to the various TTC counters on the OptoHybrid
           *  @ingroup ohcounters
           * @param signal specifies which TTC signal counter to read
           * ** FIXME MAKE THESE ENUMS **
           *  - 0 L1A
           *  - 1 Resync
           *  - 2 BC0
           *  - 3 BXN
           * @param mode specifies which TTC counter to read
           * ** FIXME (OBSOLETE) MAKE THESE ENUMS **
           *  - 0 from AMC over GBT
           *  - 1 from the generator in the OptoHybrid firmware
           *  - 2 all (reset only)
           * @returns uint32_t the requested counter value
           *  @{
           */
          /**
           * Get the recorded number of TTC signals
           */
          uint32_t getTTCCount(uint8_t const& signal, uint8_t const& mode);

          /**
           * Get the recorded number of L1A signals
           */
          uint32_t getL1ACount(uint8_t const& mode) { return getTTCCount(0x0, mode); };

          /**
           * Get the recorded number of Resync signals
           */
          uint32_t getResyncCount(uint8_t const& mode=0x0) { return getTTCCount(0x1, mode); };

          /**
           * Get the recorded number of BC0 signals
           */
          uint32_t getBC0Count(uint8_t const& mode=0x0) { return getTTCCount(0x2, mode); };

          /**
           * Get the recorded number of BXN signals
           */
          uint32_t getBXNCount(uint8_t const& mode=0x0) { return getTTCCount(0x3, mode); };

          /**
           * Reset the recorded number of TTC signals
           * @param signal
           * @param mode
           */
          void resetTTCCount(uint8_t const& signal, uint8_t const& mode=0x0);

          /**
           * Reset recorded number of L1A signals
           */
          void resetL1ACount(uint8_t const& mode=0x0) { resetTTCCount(0x0,mode); };

          /**
           * Reset recorded number of Resync signals
           */
          void resetResyncCount(uint8_t const& mode=0x0) { resetTTCCount(0x1, mode); };

          /**
           * Reset recorded number of BC0 signals
           */
          void resetBC0Count(uint8_t const& mode=0x0) { resetTTCCount(0x2, mode); };

          /**
           * Reset recorded number of BXN signals
           */
          void resetBXNCount(uint8_t const& mode=0x0) { resetTTCCount(0x3, mode); };

          /** @} */ // end of ttccounters

          //////// UNCATEGORIZED/OBSOLETE? \\\\\\\\*
          /**
           * Setup the VFAT clock
           * @param bool source true uses the external clock, false uses the onboard clock
           * @param bool fallback uses the external clock, false uses the onboard clock
           * NOT YET AVAILABLE IN V2 FIRMWARE
           */
          void setVFATClock(bool source, bool fallback) {
            //std::stringstream regName;
            //regName << "OptoHybrid_LINKS.LINK" << static_cast<uint32_t>(m_link);
            //writeReg(getDeviceBaseNode(),regName.str()+".CLOCKING.VFAT.SOURCE"  ,(uint32_t)source  );
            //writeReg(getDeviceBaseNode(),regName.str()+".CLOCKING.VFAT.FALLBACK",(uint32_t)fallback);
          };

          /**
           * VFAT clock status
           * @param bool source true uses the external clock, false uses the onboard clock
           * @param bool fallback uses the external clock, false uses the onboard clock
           * NOT YET AVAILABLE IN V2 FIRMWARE
           */
          std::pair<bool,bool> StatusVFATClock() {
            // std::stringstream regName;
            // regName << "OptoHybrid_LINKS.LINK" << static_cast<uint32_t>(m_link);
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
           */
          void setCDCEClock(bool source, bool fallback) {
            //std::stringstream regName;
            //regName << "OptoHybrid_LINKS.LINK" << static_cast<uint32_t>(m_link);
            //writeReg(getDeviceBaseNode(),regName.str()+".CLOCKING.CDCE.SOURCE"  ,(uint32_t)source  );
            //writeReg(getDeviceBaseNode(),regName.str()+".CLOCKING.CDCE.FALLBACK",(uint32_t)fallback);
          };

          /**
           * CDCE clock status
           * @param bool source true uses the external clock, false uses the onboard clock
           * @param bool fallback uses the external clock, false uses the onboard clock
           * NOT YET AVAILABLE IN V2 FIRMWARE
           */
          std::pair<bool,bool> StatusCDCEClock() {
            //std::stringstream regName;
            //regName << "OptoHybrid_LINKS.LINK" << static_cast<uint32_t>(m_link);
            //uint32_t src = readReg(getDeviceBaseNode(),regName.str()+".CLOCKING.CDCE.SOURCE");
            //uint32_t flb = readReg(getDeviceBaseNode(),regName.str()+".CLOCKING.CDCE.FALLBACK");
            ////maybe do a check to ensure that the value has been read properly?
            return std::make_pair(0,0);
          };

          ///** Read the VFAT clock source
          // * @retval bool true uses the external clock, false uses the onboard clock
          // */
          //bool getVFATClockSource();
          //
          //
          ///** Set the VFAT onboard clock fallback
          // * Allows the clock to fall back to
          // * @param bool true allows the optohybrid to switch back to the onboard clock
          // * if it fails to lock
          // */
          //void setVFATFallback(bool);
          //
          ///** Read the VFAT clock fallback status
          // * @retval returns status of VFAT clock fallback (false does not allow automatic
          // * fallback to onboard clock
          // */
          //bool getVFATFallback();
          //
          //
          ///** Set the CDCE clock source
          // * @param bool true uses the external clock, false uses the onboard clock
          // */
          //void setCDCEClockSource(bool);
          //
          ///** Read the CDCE clock source
          // * @retval bool true uses the external clock, false uses the onboard clock
          // */
          //bool getCDCEClockSource();
          //
          ///** Set the CDCE clock fallback mode
          // * @param bool true allows the optohybrid to switch back to the onboard clock if the
          // * CDCE doesn't lock
          // */
          //void setCDCEFallback(bool);
          //
          ///** Read the CDCE clock fallback mode
          // * @retval bool false does not allow the optohybrid to switch the clock back
          // */
          //bool getCDCEFallback();

          ////// pertaining to VFATs \\\\\\*

          /**
           * Sends a read request to all (un-masked) VFATs on the same register
           * @param std::string name name of the register to broadcast the request to
           * @param uint32_t mask specifying which VFATs will receive the broadcast command
           * @param bool reset specifying whether to reset the firmware module first
           * @returns a std::vector of uint32_t words, one response for each VFAT
           */
          std::vector<uint32_t> broadcastRead(std::string const& name,
                                              uint32_t    const& mask=gem::hw::utils::ALL_VFATS_BCAST_MASK,
                                              bool               reset=false);

          /**
           * Sends a write request to all (un-masked) VFATs on the same register
           * @param std::string name name of the register to broadcast the request to
           * @param uint32_t value to be written to all VFATs receiving the broadcast
           * @param uint32_t mask specifying which VFATs will receive the broadcast command
           * @param bool reset specifying whether to reset the firmware module first
           */
          void broadcastWrite(std::string const& name,
                              uint32_t    const& value,
                              uint32_t    const& mask=gem::hw::utils::ALL_VFATS_BCAST_MASK,
                              bool               reset=false);


          /**
           * Returns the slot number and chip IDs for connected VFATs
           * @returns a std::vector of pairs of uint8_t and uint32_t words, one response for each VFAT
           */
          std::vector<std::pair<uint8_t,uint32_t> > getConnectedVFATs(bool update=false);

          /**
           * Uses a broadcast read to determine which slots are occupied and returns the
           * corresponding broadcast mask
           * @param bool whether to use the stored result or to update the stored result
           * @returns uint32_t 24 bit mask
           * The mask has a 1 for VFATs that will not receive a broadcast request
           * The mask has a 1 for VFATs whose tracking data will be ignored
           */
          uint32_t getConnectedVFATMask(bool update=false);

          /**
           * Sends a write request for all setup registers on each connected VFAT
           */
          void setVFATsToDefaults();

          /**
           * Sends a write request for all setup registers on each VFAT specified by the mask
           * @param std::map<std::string, uint16_t> map of VFAT register name to value to broadcast
           */
          void setVFATsToDefaults(std::map<std::string, uint16_t> const& regvals);


        protected:
          //OptoHybridMonitor *monOptoHybrid_;

        private:
          // Do not use default constructor. HwOptoHybrid object should only be made using
          // either connection file method or with a list of URIs and address tables
          HwOptoHybrid();

          // Prevent copying of HwOptoHybrid objects
          HwOptoHybrid(const HwOptoHybrid& other);      ///< prevents construction-copy
          HwOptoHybrid& operator=(const HwOptoHybrid&); ///< prevents copying

          bool b_is_initial;  ///<
          uint32_t m_disabledMask;   ///<
          uint32_t m_connectedMask;  ///<
          uint8_t m_link;            ///< Link on the AMC the OptoHybrid is connected to

          std::vector<std::pair<uint8_t, uint32_t> > m_chipIDs;
        };  // class HwOptoHybrid
    }  // namespace gem::hw::optohybrid
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_OPTOHYBRID_HWOPTOHYBRID_H
