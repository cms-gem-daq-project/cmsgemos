/** @file HwGenericAMC.h */

#ifndef GEM_HW_HWGENERICAMC_H
#define GEM_HW_HWGENERICAMC_H

#include "gem/hw/GEMHwDevice.h"

#include "gem/hw/exception/Exception.h"
#include "gem/hw/GenericAMCSettingsEnums.h"

namespace gem {
  namespace hw {

    class HwGenericAMC : public gem::hw::GEMHwDevice
      {
      public:

        /**
         * @struct AMCIPBusCounters
         * @brief This structure stores retrieved counters related to the GenericAMC IPBus transactions
         * @var AMCIPBusCounters::OptoHybridStrobe
         * OptoHybridStrobe is a counter for the number of errors on the tracking data link
         * @var AMCIPBusCounters::OptoHybridAck
         * OptoHybridAck is a counter for the number of errors on the trigger data link
         * @var AMCIPBusCounters::TrackingStrobe
         * TrackingStrobe is a counter for the number of errors on the tracking data link
         * @var AMCIPBusCounters::TrackingAck
         * TrackingAck is a counter for the number of errors on the trigger data link
         * @var AMCIPBusCounters::CounterStrobe
         * CounterStrobe is a counter for the number of errors on the tracking data link
         * @var AMCIPBusCounters::CounterAck
         * CounterAck is a counter for the number of errors on the trigger data link
         */
        typedef struct AMCIPBusCounters {
          uint32_t OptoHybridStrobe;
          uint32_t OptoHybridAck   ;
          uint32_t TrackingStrobe  ;
          uint32_t TrackingAck     ;
          uint32_t CounterStrobe   ;
          uint32_t CounterAck      ;

        AMCIPBusCounters() :
          OptoHybridStrobe(0),OptoHybridAck(0),
            TrackingStrobe(0),TrackingAck(0),
            CounterStrobe(0),CounterAck(0) {}
          void reset() {
            OptoHybridStrobe=0; OptoHybridAck=0;
            TrackingStrobe=0;   TrackingAck=0;
            CounterStrobe=0;    CounterAck=0;
            return; }
        } AMCIPBusCounters;


        /**
         * Constructors, the preferred constructor is with a connection file and device name
         * as the IP address and address table can be managed there, rather than hard coded
         * Constrution from crateID and slotID uses this constructor as the back end
         */
        /* HwGenericAMC(); */
        /* HwGenericAMC(std::string const& amcDevice); */
        /* HwGenericAMC(int const& crate, int const& slot, bool uhalNative=false); */
        HwGenericAMC(std::string const& amcDevice,
                     std::string const& connectionFile);

        HwGenericAMC(std::string const& amcDevice,
                     std::string const& connectionURI,
                     std::string const& addressTable);

        HwGenericAMC(std::string const& amcDevice,
                     uhal::HwInterface& uhalDevice);

        virtual ~HwGenericAMC();

        /**
         * Check if one can read/write to the registers on the GenericAMC
         * @returns true if the GenericAMC is accessible
         */
        virtual bool isHwConnected() override;

        /****************************/
        /** GEM system information **/
        /****************************/
        /**
         * Read the board ID registers
         * @returns the AMC board ID as a std::string
         */
        virtual std::string getBoardID();

        /**
         * Read the board ID registers
         * @returns the AMC board ID as 32 bit unsigned value
         */
        virtual uint32_t getBoardIDRaw();

        /**
         * Check how many OptoHybrids the AMC FW can support
         * @returns the number of OptoHybrid boards supported by the FW
         */
        uint32_t getSupportedOptoHybrids() {
          return readReg(getDeviceBaseNode(),"GEM_SYSTEM.CONFIG.NUM_OF_OH"); }

        /**
         * FIXME: OBSOLETE
         * Check if the AMC FW supports GBT communication
         * @returns whether or not the AMC FW supports GBT communication
         */
        uint32_t supportsGBTLink() {
          return readReg(getDeviceBaseNode(),"GEM_SYSTEM.CONFIG.USE_GBT"); }

        /**
         * Check if the AMC FW supports trigger links
         * @returns whether or not the AMC FW supports trigger links
         */
        uint32_t supportsTriggerLink() {
          return readReg(getDeviceBaseNode(),"GEM_SYSTEM.CONFIG.USE_TRIG_LINKS"); }

        /**
         * Read the AMC FW register
         * @param system determines whether to read the system (default) or user FW register
         * @returns a string corresponding to AMC FW version
         */
        virtual std::string getFirmwareVer(bool const& system=true);

        /**
         * Read the AMC FW register
         * @param system determines whether to read the system (default) or user FW register
         * @returns the AMC FW version as a 32 bit unsigned
         */
        virtual uint32_t getFirmwareVerRaw(bool const& system=true);

        /**
         * Read the AMC FW register
         * @param system determines whether to read the system (default) or user FW register
         * @returns a string corresponding to the build date dd-mm-yyyy
         */
        virtual std::string getFirmwareDate(bool const& system=true);

        /**
         * Read the AMC FW register
         * @param system determines whether to read the system (default) or user FW register
         * @returns the build date as a 32 bit unsigned
         */
        virtual uint32_t getFirmwareDateRaw(bool const& system=true);

        //user core functionality
        /**
         * Read the user AMC FW register
         * @returns the user FW build date as a 32-bit unsigned value
         * OBSOLETE in V2 AMC FW
         */
        virtual uint32_t getUserFirmware();

        /**
         * Read the user AMC FW register
         * @returns the user FW build date as a std::string
         * OBSOLETE in V2 AMC FW
         */
        virtual std::string getUserFirmwareDate();

      private:
        /**
         * Connect to te RPC manager and load necessary modules
         * @param `reconnect` determine if the conection should be reestablished and the modules reloaded
         */
        void connectRPC(bool reconnect=false) override;

        /**
         * Check if the gtx requested is known to be operational
         * @param gtx GTX to be queried
         * @param opMsg Operation message to append to the log message
         * @returns true if the GTX is in range and active, false otherwise
         */
        virtual bool linkCheck(uint8_t const& gtx, std::string const& opMsg);

      public:
        /**
         * Set the trigger source to the front end
         * @param uint8_t mode 0 from software, 1 from TTC decoder (AMC13), 2 from both
         * OBSOLETE in V2 firmware, taken care of in the OptoHybrid
         * UNOBSOLETE? in V3 firmware?
         */
        virtual void setTrigSource(uint8_t const& mode, uint8_t const& gtx=0x0) { return; }

        /**
         * Read the front end trigger source
         * @retval uint8_t 0 from GenericAMC, 1 from AMC13, 2 from both
         * OBSOLETE in V2 firmware, taken care of in the OptoHybrid
         * UNOBSOLETE? in V3 firmware?
         */
        virtual uint8_t getTrigSource(uint8_t const& gtx=0x0) { return 0; }

        /**
         * Set the S-bit source
         * @param uint8_t chip
         * OBSOLETE in V2 firmware
         */
        virtual void setSBitSource(uint8_t const& mode, uint8_t const& gtx=0x0) { return; }

        /**
         * Read the S-bit source
         * @retval uint8_t which VFAT chip is sending the S-bits
         * OBSOLETE in V2 firmware
         */
        virtual uint8_t getSBitSource(uint8_t const& gtx=0x0) { return 0; }

        ///Counters
        /**
         * Get the recorded number of L1A signals received from the TTC decoder
         */
        uint32_t getL1ACount() { return readReg(getDeviceBaseNode(),"TTC.CMD_COUNTERS.L1A"); }

        /**
         * Get the recorded number of CalPulse signals received from the TTC decoder
         */
        uint32_t getCalPulseCount() { return readReg(getDeviceBaseNode(),"TTC.CMD_COUNTERS.CALPULSE"); }

        /**
         * Get the recorded number of Resync signals received from the TTC decoder
         */
        uint32_t getResyncCount() { return readReg(getDeviceBaseNode(),"TTC.CMD_COUNTERS.RESYNC"); }

        /**
         * Get the recorded number of BC0 signals
         */
        uint32_t getBC0Count() { return readReg(getDeviceBaseNode(),"TTC.CMD_COUNTERS.BC0"); }

        ///Counter resets

        /** FIXME specific counter resets from legacy, counter resets are module wide now  **/
        /**
         * Reset the recorded number of L1A signals received from the TTC decoder
         */
        void resetL1ACount() { return writeReg(getDeviceBaseNode(),"TTC.CTRL.CNT_RESET", 0x1); }

        /**
         * Reset the recorded number of CalPulse signals received from the TTC decoder
         */
        void resetCalPulseCount() { return writeReg(getDeviceBaseNode(),"TTC.CTRL.CNT_RESET", 0x1); }

        /**
         * Reset the recorded number of Resync signals received from the TTC decoder
         */
        void resetResyncCount() { return writeReg(getDeviceBaseNode(),"TTC.CTRL.CNT_RESET", 0x1); }

        /**
         * Reset the recorded number of BC0 signals
         */
        void resetBC0Count() { return writeReg(getDeviceBaseNode(),"TTC.CTRL.CNT_RESET", 0x1); }

        /**
         * Read the trigger data
         * @retval uint32_t returns 32 bits 6 bits for s-bits and 26 for bunch counter
         */
        uint32_t readTriggerFIFO(uint8_t const& gtx);

        /**
         * Empty the trigger data FIFO
         */
        void flushTriggerFIFO(uint8_t const& gtx);

        /**************************/
        /** DAQ link information **/
        /**************************/
        /**
         * @defgroup AMCDAQModule
         */


        /**
         * @brief Set the enable mask and enable the DAQ link
         * @param enableZS enable/disable zero suppression
         * @param doPhaseScan turn on/off the phase shifting during configure
         * @param runType parameter
         * @param relock when doing the phase shift
         * @param bc0LockPSMode use the BC0 locked phase shifting mode
         */
        void configureDAQModule(bool enableZS, bool doPhaseShift, uint32_t const& runType=0x0, uint32_t const& marker=0xfaac, bool relock=false, bool bc0LockPSMode=false);

        /**
         * @brief Set the enable mask and enable the DAQ link
         * @param enableMask 32 bit word for the 24 bit enable mask
         */
        void enableDAQLink(uint32_t const& enableMask=0x1);

        /**
         * @brief Set the DAQ link off and disable all inputs
         */
        void disableDAQLink();

        /**
         * @brief Set the zero suppression mode (handled by the AMC)
         * @param enable true means any VFAT data packet with all 0's will be suppressed
         * FIXME: toggle different VFAT3 modes?
         */
        void setZS(bool enable=true);

        /**
         * @brief Disable zero suppression of VFAT data
         */
        void disableZS() { setZS(false); };

        /**
         * @brief reset the DAQ link and write the DAV timout
         *        - assert RESET and then release
         *        - disable DAQ link
         *        - set each link EOE_TIMEOUT to default value
         *        - set DAV_TIMEOUT  to supplied value
         *        - set TTS_OVERRIDE to supplied value
         * @param davTO value to use for the DAV timeout
         * @param ttsOverride value to use for the TTS override
         */
        void resetDAQLink(uint32_t const& davTO=0x500, uint32_t const& ttsOverride=0x0);

        /**
         * @returns Returns the 32 bit word corresponding to the DAQ link control register
         */
        uint32_t getDAQLinkControl();

        /**
         * @returns Returns the 32 bit word corresponding to the DAQ link status register
         */
        uint32_t getDAQLinkStatus();

        /**
         * @returns Returns true if the DAQ link is ready
         */
        bool daqLinkReady();

        /**
         * @returns Returns true if the DAQ link is clock is locked
         */
        bool daqClockLocked();

        /**
         * @returns Returns true if the TTC is ready
         */
        bool daqTTCReady();

        /**
         * @returns Returns the current TTS state asserted by the DAQ link firmware
         */
        uint8_t daqTTSState();

        /**
         * @returns Returns true if the event FIFO is almost full (70%)
         */
        bool daqAlmostFull();

        /**
         * @returns Returns true if the L1A FIFO is empty (0%)
         */
        bool l1aFIFOIsEmpty();

        /**
         * @returns Returns true if the L1A FIFO is almost full (70%)
         */
        bool l1aFIFOIsAlmostFull();

        /**
         * @returns Returns true if the L1A FIFO is full (100%)
         */
        bool l1aFIFOIsFull();

        /**
         * @returns Returns true if the L1A FIFO is underflos
         */
        bool l1aFIFOIsUnderflow();

        /**
         * @returns Returns the number of events built and sent on the DAQ link
         */
        uint32_t getDAQLinkEventsSent();

        /**
         * @returns Returns the curent L1AID (number of L1As received)
         */
        uint32_t getDAQLinkL1AID();

        /* /\** */
        /*  * @returns Returns the curent L1A rate (in Hz) */
        /*  *\/ */
        /* uint32_t getDAQLinkL1ARate(); */

        /**
         * @returns Returns
         */
        uint32_t getDAQLinkDisperErrors();

        /**
         * @returns Returns
         */
        uint32_t getDAQLinkNonidentifiableErrors();

        /**
         * @returns Returns the DAQ link input enable mask
         */
        uint32_t getDAQLinkInputMask();

        /**
         * @returns Returns the timeout used in the event builder before closing the event and sending the (potentially incomplete) data
         */
        uint32_t getDAQLinkDAVTimeout();

        /**
         * @param max is a bool specifying whether to query the max timer or the last timer
         * @returns Returns the spent building an event
         */
        uint32_t getDAQLinkDAVTimer(bool const& max);

        /***************************************/
        /** GTX specific DAQ link information **/
        /***************************************/
        /**
         * @param gtx is the input link status to query
         * @returns Returns the the 32-bit word corresponding DAQ status for the specified link
         */
        // FIXME: renamed from DAQLink to LinkDAQ
        uint32_t getLinkDAQStatus(uint8_t const& gtx);

        /**
         * @param gtx is the input link counter to query
         * @param mode specifies whether to query the corrupt VFAT count (0x0) or the event number
         * @returns Returns the link counter for the specified mode
         */
        // FIXME: renamed from DAQLink to LinkDAQ
        uint32_t getLinkDAQCounters(uint8_t const& gtx, uint8_t const& mode);

        /**
         * @param gtx is the input link status to query
         * @returns Returns a block of the last 7 words received from the OH on the link specified
         */
        // FIXME: renamed from DAQLink to LinkDAQ
        uint32_t getLinkLastDAQBlock(uint8_t const& gtx);

        /**
         * @returns Returns the timeout before the event builder FW will close the event and send the data
         */
        uint32_t getDAQLinkInputTimeout();

        /**
         * @returns Returns the run type stored in the data stream
         */
        uint32_t getDAQLinkRunType();

        /**
         * @returns Special run parameters 1,2,3 as a single 24 bit word
         */
        uint32_t getDAQLinkRunParameters();

        /**
         * @returns Special run parameter written into data stream
         */
        uint32_t getDAQLinkRunParameter(uint8_t const& parameter);


        /**
         * @brief Set DAQ link timeout
         * @param value is the number of clock cycles to wait after receipt of last L1A and
         *        last packet received from the optical link before closing an "event"
         *        (in units of 160MHz clock cycles, value/4 for 40MHz clock cycles)
         */
        void setDAQLinkInputTimeout(uint32_t const& value=0x100);

        /**
         * @brief Special run type to be written into data stream
         * @param value is the run type
         */
        void setDAQLinkRunType(uint32_t const& value);

        /**
         * @returns Set special run parameter to be written into data stream
         * @param value is a 24 bit word to write into the run paramter portion of the GEM header
         */
        void setDAQLinkRunParameters(uint32_t const& value);

        /**
         * @returns Special run parameter written into data stream
         * @param parameter is the number of parameter to be written (1-3)
         * @param value is the run paramter to write into the specified parameter
         */
        void setDAQLinkRunParameter(uint8_t const& parameter, uint8_t const& value);


        /**************************/
        /** TTC module information **/
        /**************************/
        /**
         * @defgroup AMCTTCModule
         */

        /*** CTRL submodule ***/
        /**
         * @brief Reset the TTC module
         */
        void ttcModuleReset();

        /**
         * @brief Reset the MMCM of the TTC module
         */
        void ttcMMCMReset();

        /**
         * @brief Shift the phase of the MMCM of the TTC module
         * @param relock to shift of lock before looking for a good lock
         * @param modeBC0 to determine the good phase region, rather than the PLL lock status
         * @param scan whether to roll around multiple times for monitoring purposes
         */
        void ttcMMCMPhaseShift(bool relock=false, bool modeBC0=false, bool scan=false);

        /**
         * @brief Check the lock status of the MMCM PLL
         * @param Number of times to read the PLL lock status
         * @returns Lock count of the MMCM PLL
         */
        int checkPLLLock(uint32_t readAttempts);

        /**
         * @brief Check the phase mean of the MMCM PLL
         * @param Number of times to read the phase mean
         *        * 0 reads means take the mean calculated in FW
         *        * 1+ reads means take the mean of the specified reads of the phase directly
         * @returns Mean value of the MMCH phase
         */
        double getMMCMPhaseMean(uint32_t readAttempts);

        /**
         * @brief Check the phase median of the MMCM PLL
         * @param Number of times to read the phase and compute the median
         * @returns Median value of the MMCH phase
         */
        double getMMCMPhaseMedian(uint32_t readAttempts);

        /**
         * @brief Check the phase mean of the GTH PLL
         * @param Number of times to read the phase mean
         *        * 0 reads means take the mean calculated in FW
         *        * 1+ reads means take the mean of the specified reads of the phase directly
         * @returns Mean value of the GTH phase
         */
        double getGTHPhaseMean(uint32_t readAttempts);

        /**
         * @brief Check the phase median of the GTH PLL
         * @param Number of times to read the phase and compute the median
         * @returns Median value of the MMCH phase
         */
        double getGTHPhaseMedian(uint32_t readAttempts);

        /**
         * @brief Reset the counters of the TTC module
         */
        virtual void ttcCounterReset();

        /**
         * @returns whether or not L1As are currently enabled on the GenericAMC
         */
        virtual bool getL1AEnable();

        /**
         * @param whether or not to enable L1As on the GenericAMC
         */
        virtual void setL1AEnable(bool enable=true);

        /*** CONFIG submodule ***/
        /**
         * @param cmd AMCTTCCommandT enum type to retrieve the current configuration of
         * @returns TTC configuration register values
         */
        virtual uint32_t getTTCConfig(AMCTTCCommandT const& cmd);

        /**
         * @param cmd AMCTTCCommandT to set the current configuration of
         */
        virtual void setTTCConfig(AMCTTCCommandT const& cmd, uint8_t const& value);

        /*** STATUS submodule ***/
        /**
         * @brief Returns the first status register of the TTC module
         */
        virtual uint32_t getTTCStatus();

        /**
         * @brief Returns the error count of the TTC module
         * @param specify whether single or double error count
         */
        virtual uint32_t getTTCErrorCount(bool const& single=true);

        /*** CMD_COUNTERS submodule ***/
        /**
         * @param cmd AMCTTCCommandT to get the current configuration of
         * @returns Returns the counter for the specified TTC command
         */
        virtual uint32_t getTTCCounter(AMCTTCCommandT const& cmd);

        /**
         * @returns Returns the L1A ID received by the TTC module
         */
        virtual uint32_t getL1AID();

        /**
         * @returns Returns the curent L1A rate (in Hz)
         */
        virtual uint32_t getL1ARate();

        /**
         * @returns 32-bit word corresponding to the 8 most recent TTC commands received
         */
        virtual uint32_t getTTCSpyBuffer();

        /**************************/
        /** SLOW_CONTROL module information **/
        /**************************/

        /*** SCA submodule ***/
        /**
         * @brief Enable the SCA to forward HardReset to the OptoHybrid
         * @param bool switches on/off the HardReset forwarding (default is false)
         */
        virtual void scaHardResetEnable(bool const& en=false);

        /********************************/
        /** TRIGGER module information **/
        /********************************/

        /*** CTRL submodule ***/
        /**
         * @brief Reset the TRIGGER module
         */
        virtual void triggerReset();

        /**
         * @brief Reset the counters of the TRIGGER module
         */
        virtual void triggerCounterReset();

        /**
         * @brief Reset the counters of the TRIGGER module
         * @returns the mask for which OHs will have their sbits blocked
         */
        virtual uint32_t getOptoHybridKillMask();

        /**
         * @brief Reset the counters of the TRIGGER module
         * @param mask of which OptoHybrids will have their sbits blocked
         */
        virtual void setOptoHybridKillMask(uint32_t const& mask);

        /*** STATUS submodule ***/
        /**
         * @brief Returns the first status register of the TRIGGER module
         */
        virtual uint32_t getORTriggerRate();

        /**
         * @brief Returns the error count of the TRIGGER module
         * @param specify whether single or double error count
         */
        virtual uint32_t getORTriggerCount();

        /*** OH{IDXX} submodule ***/
        /**
         * @brief Returns the first status register of the TRIGGER module
         * @param OptoHybrid to obtain the rate for
         * @returns Rate of the triggers seen
         */
        virtual uint32_t getOptoHybridTriggerRate(uint8_t const& oh);

        /**
         * @brief Returns the error count of the TRIGGER module
         * @param OptoHybrid to obtain the count for
         * @returns Count of the triggers seen
         */
        virtual uint32_t getOptoHybridTriggerCount(uint8_t const& oh);

        /**
         * @brief Returns the rate of seen sbit clusters of a given size from a specific OptoHybrid
         * @param OptoHybrid to obtain the rate for
         * @param Cluster size to return the rate for
         * @returns Rate of the sbits for a specified cluster size
         */
        virtual uint32_t getOptoHybridClusterRate(uint8_t const& oh, uint8_t const& cs);

        /**
         * @brief Returns the count of seen sbit clusters of a given size from a specific OptoHybrid
         * @param OptoHybrid to obtain the count for
         * @param Cluster size to return the count for
         * @returns Count of the sbits for a specified cluster size
         */
        virtual uint32_t getOptoHybridClusterCount(uint8_t const& oh, uint8_t const& cs);

        /**
         * @brief Returns the last cluster of seen sbit clusters of a given size from a specific OptoHybrid
         * @param OptoHybrid to obtain the last cluster for
         * @param Cluster size to return the last cluster for
         * @returns Last cluster of specified size
         */
        virtual uint32_t getOptoHybridDebugLastCluster(uint8_t const& oh, uint8_t const& cs);

        /**
         * @brief Returns the count of seen sbit clusters of a given size from a specific OptoHybrid
         * @param OptoHybrid to obtain the count for
         * @param Link size to return the count for
         * @param Which counter to query
         * @returns Count of the sbits for a specified cluster size
         */
        virtual uint32_t getOptoHybridTriggerLinkCount(uint8_t const& oh, uint8_t const& link, AMCOHLinkCountT const& count);

        /****************************/
        /** DAQ moudle information **/
        /****************************/
        /**
         * @brief performs a general reset of the GenericAMC
         */
        virtual void generalReset();

        /**
         * @brief performs a reset of the GenericAMC counters
         */
        virtual void counterReset();

        /**
         * @brief performs a reset of the GenericAMC T1 counters
         */
        virtual void resetT1Counters();

        /**
         * @brief performs a reset of the GenericAMC GTX link counters
         */
        virtual void linkCounterReset();

        /**
         * @brief performs a reset of the GenericAMC link
         * @param link is the link to perform the reset on
         */
        virtual void linkReset(uint8_t const& link);

        std::vector<AMCIPBusCounters> m_ipBusCounters; /** for each gtx, IPBus counters */

      protected:
        //GenericAMCMonitor *monGenericAMC_;

        bool b_links[gem::hw::utils::N_GTX]; // have to figure out how to make this dynamic, or if we can just drop it... FIXME
        uint32_t m_links;    ///< Connected links mask
        uint32_t m_maxLinks; ///< Maximum supported OptoHybrids as reported by the firmware

        /* std::vector<linkStatus> v_activeLinks; ///< vector keeping track of the active links */

        /**
         * @brief sets the expected board ID string to be matched when reading from the firmware
         * @param boardID is the expected board ID
         */
        void setExpectedBoardID(std::string const& boardID) { m_boardID = boardID; }

        std::string m_boardID;  ///< expected boardID in the firmware

      private:
        // Do not use default constructor. HwGenericAMC object should only be made using
        // either connection file method or with a list of URIs and address tables
        HwGenericAMC();

        // Prevent copying of HwGenericAMC objects
        HwGenericAMC(const HwGenericAMC& other);      // prevents construction-copy
        HwGenericAMC& operator=(const HwGenericAMC&); // prevents copying

      };  // class HwGenericAMC
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_HWGENERICAMC_H
