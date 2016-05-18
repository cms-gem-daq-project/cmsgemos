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

        static const unsigned N_GTX = 2; ///< maximum number of GTX links on the GenericAMC

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
        HwGenericAMC();
        HwGenericAMC(std::string const& amcDevice);
        HwGenericAMC(std::string const& amcDevice, int const& crate, int const& slot);
        HwGenericAMC(std::string const& amcDevice, std::string const& connectionFile);
        HwGenericAMC(std::string const& amcDevice, std::string const& connectionURI,
                     std::string const& addressTable);
        HwGenericAMC(std::string const& amcDevice, uhal::HwInterface& uhalDevice);

        virtual ~HwGenericAMC();

        /**
         * Check if one can read/write to the registers on the GenericAMC
         * @returns true if the GenericAMC is accessible
         */
        virtual bool isHwConnected();

        /**
         * Read the board ID registers
         * @returns the GLIB board ID
         */
        virtual std::string getBoardID();

        /**
         * Read the board ID registers
         * @returns the GLIB board ID as 32 bit unsigned
         */
        virtual uint32_t getBoardIDRaw();

        /**
         * Check how many OptoHybrids the AMC can support
         * @returns the number of supported OptoHybrid boards
         */
        virtual uint32_t getSupportedOptoHybrids() { return N_GTX; }

        /**
         * Read the system firmware register
         * @param system determines whether to read the system or user firmware register
         * @returns a string corresponding to firmware version
         */
        virtual std::string getFirmwareVer(bool const& system=true);

        /**
         * Read the system firmware register
         * @param system determines whether to read the system or user firmware register
         * @returns the firmware version as a 32 bit unsigned
         */
        virtual uint32_t getFirmwareVerRaw(bool const& system=true);

        /**
         * Read the system firmware register
         * @returns a string corresponding to the build date dd-mm-yyyy
         */
        virtual std::string getFirmwareDate(bool const& system=true);

        /**
         * Read the system firmware register
         * @param system determines whether to read the system or user firmware register
         * @returns the build date as a 32 bit unsigned
         */
        virtual uint32_t getFirmwareDateRaw(bool const& system=true);

        //user core functionality
        /**
         * Read the user firmware register
         * @returns a hex number corresponding to the build date
         * OBSOLETE in V2 firmware
         */
        virtual uint32_t getUserFirmware();

        /**
         * Read the user firmware register
         * @returns a std::string corresponding to the build date
         * OBSOLETE in V2 firmware
         */
        virtual std::string getUserFirmwareDate();

      private:
        /**
         * Check if the gtx requested is known to be operational
         * @param uint8_t gtx GTX gtx to be queried
         * @param std::string opMsg Operation message to append to the log message
         * @returns true if the gtx is in range and active, false otherwise
         */
        virtual bool linkCheck(uint8_t const& gtx, std::string const& opMsg);

      public:
        /**
         * Read the gtx status registers, store the information in a struct
         * @param uint8_t gtx is the number of the gtx to query
         * @retval _status a struct containing the status bits of the optical link
         * @throws gem::hw::exception::InvalidLink if the gtx number is outside of 0-N_GTX
         */
        virtual GEMHwDevice::OpticalLinkStatus LinkStatus(uint8_t const& gtx);

        /**
         * Reset the gtx status registers
         * @param uint8_t gtx is the number of the gtx to query
         * @param uint8_t resets control which bits to reset
         * bit 1 - TRK_ErrCnt         0x1
         * bit 2 - TRG_ErrCnt         0x2
         * bit 3 - Data_Rec           0x4
         * @throws gem::hw::exception::InvalidLink if the gtx number is outside of 0-N_GTX
         */
        virtual void LinkReset(uint8_t const& gtx, uint8_t const& resets);

        /**
         * Reset the all gtx status registers
         * @param uint8_t resets control which bits to reset
         */
        virtual void ResetLinks(uint8_t const& resets) {
          for (auto gtx = v_activeLinks.begin(); gtx != v_activeLinks.end(); ++gtx)
            LinkReset(gtx->first,resets);
        }

        /**
         * Set the Trigger source
         * @param uint8_t mode 0 from software, 1 from TTC decoder (AMC13), 2 from both
         * OBSOLETE in V2 firmware, taken care of in the OptoHybrid
         */
        virtual void setTrigSource(uint8_t const& mode, uint8_t const& gtx=0x0) {
          return;
        }

        /**
         * Read the Trigger source
         * @retval uint8_t 0 from GenericAMC, 1 from AMC13, 2 from both
         * OBSOLETE in V2 firmware, taken care of in the OptoHybrid
         */
        virtual uint8_t getTrigSource(uint8_t const& gtx=0x0) {
          return 0;
        }

        /**
         * Set the S-bit source
         * @param uint8_t chip
         * OBSOLETE in V2 firmware
         */
        virtual void setSBitSource(uint8_t const& mode, uint8_t const& gtx=0x0) {
          return;
        }

        /**
         * Read the S-bit source
         * @retval uint8_t which VFAT chip is sending the S-bits
         * OBSOLETE in V2 firmware
         */
        virtual uint8_t getSBitSource(uint8_t const& gtx=0x0) {
          return 0;
        }

        ///Counters
        /**
         * Get the recorded number of IPBus signals sent/received by the GenericAMC
         * @param uint8_t gtx which GTX
         * @param uint8_t mode which counter
         * bit 1 OptoHybridStrobe
         * bit 2 OptoHybridAck
         * bit 3 TrackingStrobe
         * bit 4 TrackingAck
         * bit 5 CounterStrobe
         * bit 6 CounterAck
         * @returns AMCIPBusCounters struct, with updated values for the ones specified in the mask
         */
        virtual AMCIPBusCounters getIPBusCounters(uint8_t const& gtx, uint8_t const& mode);

        /**
         * Get the recorded number of L1A signals received from the TTC decoder
         */
        virtual uint32_t getL1ACount() {
          return readReg(getDeviceBaseNode(),"COUNTERS.T1.L1A"); }

        /**
         * Get the recorded number of CalPulse signals received from the TTC decoder
         */
        virtual uint32_t getCalPulseCount() {
          return readReg(getDeviceBaseNode(),"COUNTERS.T1.CalPulse"); }

        /**
         * Get the recorded number of Resync signals received from the TTC decoder
         */
        virtual uint32_t getResyncCount() {
          return readReg(getDeviceBaseNode(),"COUNTERS.T1.Resync"); }

        /**
         * Get the recorded number of BC0 signals
         */
        virtual uint32_t getBC0Count() {
          return readReg(getDeviceBaseNode(),"COUNTERS.T1.BC0"); }

        ///Counter resets
        /**
         * Get the recorded number of IPBus signals sent/received by the GenericAMC
         * @param uint8_t gtx which GTX
         * @param uint8_t mode which counter
         * bit 0 OptoHybridStrobe
         * bit 1 OptoHybridAck
         * bit 2 TrackingStrobe
         * bit 3 TrackingAck
         * bit 4 CounterStrobe
         * bit 5 CounterAck
         */
        virtual void resetIPBusCounters(uint8_t const& gtx, uint8_t const& mode);

        /**
         * Reset the recorded number of L1A signals received from the TTC decoder
         */
        virtual void resetL1ACount() {
          return writeReg(getDeviceBaseNode(),"COUNTERS.T1.L1A.Reset", 0x1); }

        /**
         * Reset the recorded number of CalPulse signals received from the TTC decoder
         */
        virtual void resetCalPulseCount() {
          return writeReg(getDeviceBaseNode(),"COUNTERS.T1.CalPulse.Reset", 0x1); }

        /**
         * Reset the recorded number of Resync signals received from the TTC decoder
         */
        virtual void resetResyncCount() {
          return writeReg(getDeviceBaseNode(),"COUNTERS.T1.Resync.Reset", 0x1); }

        /**
         * Reset the recorded number of BC0 signals
         */
        virtual void resetBC0Count() {
          return writeReg(getDeviceBaseNode(),"COUNTERS.T1.BC0.Reset", 0x1); }

        /**
         * Read the trigger data
         * @retval uint32_t returns 32 bits 6 bits for s-bits and 26 for bunch countrr
         */
        virtual uint32_t readTriggerFIFO(uint8_t const& gtx);

        /**
         * Empty the trigger data FIFO
         */
        virtual void flushTriggerFIFO(uint8_t const& gtx);

        /**
         * Read the tracking data FIFO occupancy in terms of raw 32bit words
         * @param uint8_t gtx is the number of the gtx to query
         * @retval uint32_t returns the number of words in the tracking data FIFO
         */
        virtual uint32_t getFIFOOccupancy(uint8_t const& gtx);

        /**
         * Read the tracking data FIFO occupancy in terms of the number of 7x32bit words
         * composing a single VFAT block
         * @param uint8_t gtx is the number of the gtx to query
         * @retval uint32_t returns the number of VFAT blocks in the tracking data FIFO
         */
        virtual uint32_t getFIFOVFATBlockOccupancy(uint8_t const& gtx);

        /**
         * see if there is tracking data available
         * @param uint8_t gtx is the number of the column of the tracking data to read
         * @retval bool returns true if there is tracking data in the FIFO
         TRK_DATA.COLX.DATA_RDY
        */
        virtual bool hasTrackingData(uint8_t const& gtx);

        /**
         * get the tracking data, have to do this intelligently, as IPBus transactions are expensive
         * and need to pack all events together
         * @param uint8_t gtx is the number of the GTX tracking data to read
         * @param size_t nBlocks is the number of VFAT data blocks (7*32bit words) to read
         * @retval std::vector<uint32_t> returns the 7*nBlocks data words in the buffer
         */
        std::vector<uint32_t> getTrackingData(uint8_t const& gtx, size_t const& nBlocks=1);
        //which of these will be better and do what we want
        virtual uint32_t getTrackingData(uint8_t const& gtx, uint32_t* data, size_t const& nBlocks=1);
        //which of these will be better and do what we want
        virtual uint32_t getTrackingData(uint8_t const& gtx, std::vector<toolbox::mem::Reference*>& data,
                                 size_t const& nBlocks=1);

        /**
         * Empty the tracking data FIFO
         * @param uint8_t gtx is the number of the gtx to query
         *
         */
        virtual void flushFIFO(uint8_t const& gtx);

        /**************************/
        /** DAQ link information **/
        /**************************/
        /**
         * @brief Set the enable mask and enable the DAQ link
         * @param enableMask 32 bit word for the 24 bit enable mask
         */
        virtual void enableDAQLink(uint32_t const& enableMask=0x1);

        /**
         * @brief Set the DAQ link off and disable all inputs
         */
        virtual void disableDAQLink();

        /**
         * @brief reset the DAQ link and write the DAV timout
         * @param davTO value to use for the DAV timeout
         */
        virtual void resetDAQLink(uint32_t const& davTO=0x3d090);

        /**
         * @returns Returns the 32 bit word corresponding to the DAQ link control register
         */
        virtual uint32_t getDAQLinkControl();

        /**
         * @returns Returns the 32 bit word corresponding to the DAQ link status register
         */
        virtual uint32_t getDAQLinkStatus();

        /**
         * @returns Returns true if the DAQ link is ready
         */
        virtual bool daqLinkReady();

        /**
         * @returns Returns true if the DAQ link is clock is locked
         */
        virtual bool daqClockLocked();

        /**
         * @returns Returns true if the TTC is ready
         */
        virtual bool daqTTCReady();

        /**
         * @returns Returns true if the event FIFO is almost full (70%)
         */
        virtual bool daqAlmostFull();

        /**
         * @returns Returns the current TTS state asserted by the DAQ link firmware
         */
        virtual uint8_t daqTTSState();

        /**
         * @returns Returns the number of events built and sent on the DAQ link
         */
        virtual uint32_t getDAQLinkEventsSent();

        /**
         * @returns Returns the curent L1AID (number of L1As received)
         */
        virtual uint32_t getDAQLinkL1AID();

        /**
         * @returns Returns
         */
        virtual uint32_t getDAQLinkDisperErrors();

        /**
         * @returns Returns
         */
        virtual uint32_t getDAQLinkNonidentifiableErrors();

        /**
         * @returns Returns the DAQ link input enable mask
         */
        virtual uint32_t getDAQLinkInputMask();

        /**
         * @returns Returns the timeout before the event builder firmware will close the event and send the data
         */
        virtual uint32_t getDAQLinkDAVTimeout();

        /**
         * @param max is a bool specifying whether to query the max timer or the last timer
         * @returns Returns the timeout before the event builder firmware will close the event and send the data
         */
        virtual uint32_t getDAQLinkDAVTimer(bool const& max);

        /***************************************/
        /** GTX specific DAQ link information **/
        /***************************************/
        /**
         * @param gtx is the input link status to query
         * @returns Returns the the 32-bit word corresponding DAQ status for the specified link
         */
        virtual uint32_t getDAQLinkStatus(   uint8_t const& gtx);

        /**
         * @param gtx is the input link counter to query
         * @param mode specifies whether to query the corrupt VFAT count (0x0) or the event number
         * @returns Returns the link counter for the specified mode
         */
        virtual uint32_t getDAQLinkCounters( uint8_t const& gtx, uint8_t const& mode);

        /**
         * @param gtx is the input link status to query
         * @returns Returns a block of the last 7 words received from the OH on the link specified
         */
        virtual uint32_t getDAQLinkLastBlock(uint8_t const& gtx);

        /**
         * @returns Returns the timeout before the event builder firmware will close the event and send the data
         */
        virtual uint32_t getDAQLinkInputTimeout();

        /**
         * @returns Returns the run type stored in the data stream
         */
        virtual uint32_t getDAQLinkRunType();

        /**
         * @returns Special run parameters 1,2,3 as a single 24 bit word
         */
        virtual uint32_t getDAQLinkRunParameters();

        /**
         * @returns Special run parameter written into data stream
         */
        virtual uint32_t getDAQLinkRunParameter(uint8_t const& parameter);


        /**
         * @brief Set DAQ link timeout
         * @param value is the number of clock cycles to wait after receipt of last L1A and
         *        last packet received from the optical link before closing an "event"
         */
        virtual void setDAQLinkInputTimeout(uint32_t const& value);

        /**
         * @brief Special run type to be written into data stream
         * @param value is the run type
         */
        virtual void setDAQLinkRunType(uint32_t const& value);

        /**
         * @returns Set special run parameter to be written into data stream
         * @param value is a 24 bit word to write into the run paramter portion of the GEM header
         */
        virtual void setDAQLinkRunParameters(uint32_t const& value);

        /**
         * @returns Special run parameter written into data stream
         * @param parameter is the number of parameter to be written (1-3)
         * @param value is the run paramter to write into the specified parameter
         */
        virtual void setDAQLinkRunParameter(uint8_t const& parameter, uint8_t const& value);


        /**************************/
        /** TTC link information **/
        /**************************/
        /**
         * @returns TTC control register value
         */
        virtual uint32_t getTTCControl();

        /**
         * @returns TTC encoding in use on the GenericAMC
         */
        virtual AMCTTCEncoding getTTCEncoding();

        /**
         * @param select which TTC encoding to use on the GenericAMC
         */
        virtual void setTTCEncoding(AMCTTCEncoding ttc_enc);

        /**
         * @returns whether or not L1As are currently inhibited on the GenericAMC
         */
        virtual bool getL1AInhibit();

        /**
         * @param whether or not to inhibit L1As on the GenericAMC
         */
        virtual void setL1AInhibit(bool inhibit);

        /**
         * @brief resets the TTC on the GenericAMC
         */
        virtual void resetTTC();

        /**
         * @returns 32-bit word corresponding to the 8 most recent TTC commands received
         */
        virtual uint32_t getTTCSpyBuffer();

        /**************************/
        /** DAQ link information **/
        /**************************/
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
        virtual void resetLinkCounters();

        /**
         * @brief performs a reset of the GenericAMC link
         * @param link is the link to perform the reset on
         */
        virtual void linkReset(uint8_t const& link);

        std::vector<AMCIPBusCounters> m_ipBusCounters; /** for each gtx, IPBus counters */

      protected:
        //GenericAMCMonitor *monGenericAMC_;

        bool b_links[N_GTX];

        std::vector<linkStatus> v_activeLinks;

        /**
         * @brief sets the expected board ID string to be matched when reading from the firmware
         * @param boardID is the expected board ID
         */
        void setExpectedBoardID(std::string const& boardID) { m_boardID = boardID; }
        
        std::string m_boardID;  ///< expected boardID in the firmware

        int m_crate;  ///< Crate number the AMC is housed in
        int m_slot;   ///< Slot number in the uTCA shelf the AMC is sitting in
        
      private:

      };  // class HwGenericAMC
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_HWGENERICAMC_H
