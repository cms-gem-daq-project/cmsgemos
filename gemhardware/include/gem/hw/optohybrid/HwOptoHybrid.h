/** @file HwOptoHybrid.h */

#ifndef GEM_HW_OPTOHYBRID_HWOPTOHYBRID_H
#define GEM_HW_OPTOHYBRID_HWOPTOHYBRID_H

#include "gem/hw/GEMHwDevice.h"
#include "gem/hw/glib/HwGLIB.h"
#include "gem/hw/vfat/HwVFAT2.h"

#include "gem/hw/optohybrid/exception/Exception.h"
#include "gem/hw/optohybrid/OptoHybridSettingsEnums.h"
//#include "gem/hw/optohybrid/OptoHybridMonitor.h"

namespace gem {
  namespace hw {
    /* namespace vfat { */
    /*   class HwVFAT2; */
    /* } */

    namespace optohybrid {

      static const int MAX_VFATS = 24;  ///< maximum number of VFATs that can be connected to an OptoHybrid
      static const uint32_t ALL_VFATS_BCAST_MASK = 0xff000000; ///< send broadcast I2C requests to all chips
      static const uint32_t ALL_VFATS_DATA_MASK  = 0xffffffff; ///< mask tracking data packets from all VFATs

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
           */
          typedef struct OptoHybridWBMasterCounters {
            std::pair<uint32_t,uint32_t> GTX   ;
            std::pair<uint32_t,uint32_t> GBT   ;
            std::pair<uint32_t,uint32_t> ExtI2C;
            std::pair<uint32_t,uint32_t> Scan  ;
            std::pair<uint32_t,uint32_t> DAC   ;

          OptoHybridWBMasterCounters() :
            GTX(std::make_pair(0,0)),
              GBT(std::make_pair(0,0)),
              ExtI2C(std::make_pair(0,0)),
              Scan(std::make_pair(0,0)),
              DAC(std::make_pair(0,0)) {};

            void reset() {
              GTX=std::make_pair(0,0);
              GBT=std::make_pair(0,0);
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
           */
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
           * @var OptoHybridT1Counters::GTX_TTC
           * GTX_TTC contains the counters for T1 signals received from the AMC TTC decder, sent along the 8b10b GTX link
           * @var OptoHybridT1Counters::GBT_TTC
           * GBT_TTC contains the counters for T1 signals received from the AMC TTC decder, sent along the GBT link
           * @var OptoHybridT1Counters::Firmware
           * Firmware contains the counters for T1 signals generated in the firmware module
           * @var OptoHybridT1Counters::External
           * External contains the counters for T1 signals received an external sourcd
           * @var OptoHybridT1Counters::Loopback
           * Loopback contains the counters for T1 signals looped back onto the system
           * @var OptoHybridT1Counters::Sent
           * Sent contains the counters for T1 signals sent by the OptoHybrid to the GEB
           */
          typedef struct OptoHybridT1Counters {
            std::vector<uint32_t> GTX_TTC ;
            std::vector<uint32_t> GBT_TTC ;
            std::vector<uint32_t> Firmware;
            std::vector<uint32_t> External;
            std::vector<uint32_t> Loopback;
            std::vector<uint32_t> Sent    ;

          OptoHybridT1Counters() :
            GTX_TTC(4, 0),GBT_TTC(4, 0),Firmware(4, 0),External(4, 0),Loopback(4, 0),Sent(4, 0) {};
            void reset() {
              std::fill(GTX_TTC.begin(),  GTX_TTC.end(),  0);
              std::fill(GBT_TTC.begin(),  GBT_TTC.end(),  0);
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
           */
          typedef struct OptoHybridVFATCRCCounters {
            std::vector<std::pair<uint32_t,uint32_t> > CRCCounters;

          OptoHybridVFATCRCCounters() :
            CRCCounters(24, std::make_pair(0,0)) {};
            void reset() {
              std::fill(CRCCounters.begin(), CRCCounters.end(), std::make_pair(0,0));
              return; };
          } OptoHybridVFATCRCCounters;

          /* HwOptoHybrid(); */

          HwOptoHybrid(std::string const& optohybridDevice, std::string const& connectionFile);
          HwOptoHybrid(std::string const& optohybridDevice, std::string const& connectionURI,
                       std::string const& addressTable);
          HwOptoHybrid(std::string const& optohybridDevice, uhal::HwInterface& uhalDevice);
          HwOptoHybrid(gem::hw::glib::HwGLIB const& glib,
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

          virtual bool isHwConnected();

          /** Read the board ID registers
           * @returns the OptoHybrid board ID
           */
          //std::string getBoardID()   const;

          /**
           * Read the firmware register
           * @returns a hex number corresponding to the release version
           */

          uint32_t getFirmwareVersion() {
            uint32_t fwver = readReg(getDeviceBaseNode(),"STATUS.FW.VERSION");
            TRACE("OH has firmware version 0x" << std::hex << fwver << std::dec << std::endl);
            return fwver;
          };

          /**
           * Read the firmware register
           * @returns a string corresponding to the firmware maj.min.ver.patch
           */
          std::string getFirmwareVersionString() {
            std::stringstream retval;
            uint32_t fwver = getFirmwareVersion();
            retval << std::hex
                   << ((fwver>>24) & 0xff) << "."
                   << ((fwver>>16) & 0xff) << "."
                   << ((fwver>>8)  & 0xff) << "."
                   << ((fwver)     & 0xff)
                   << std::dec;
            return retval.str();
          };

          /**
           * Read the firmware register
           * @returns a hex number corresponding to the build date
           */
          uint32_t getFirmwareDate() {
            uint32_t fwver = readReg(getDeviceBaseNode(),"STATUS.FW.DATE");
            TRACE("OH has firmware date 0x" << std::hex << fwver << std::dec << std::endl);
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

          /////Inherited from GEMHwDevice
          /**
           * @brief performs a general reset of the AMC
           */
          virtual void generalReset();

          /**
           * @brief performs a reset of the AMC counters
           */
          virtual void counterReset();

          /**
           * @brief performs a reset of the AMC link
           * @param link is the link to perform the reset on
           */
          virtual void linkReset(uint8_t const& link);


          /////////  Specific to the OptoHybrid board  \\\\\\\\\\\*

          /**
           * @brief Get the GEMHwDevice corresponding to the specified VFAT
           * @param vfat is the VFAT position
           */
          gem::hw::vfat::HwVFAT2& getVFATDevice(uint8_t const& vfat) const {
            // want to pass in the device name to append to it the VFAT
            // this is almost guaranteed to leak
            return static_cast<gem::hw::vfat::HwVFAT2&>(*(new gem::hw::vfat::HwVFAT2(*this, vfat))); };

          /**
           * Read the link status registers, store the information in a struct
           * @retval _status a struct containing the status bits of the optical link
           */
          GEMHwDevice::OpticalLinkStatus LinkStatus();

          /**
           * Reset the link status registers
           * @param uint8_t resets control which bits to reset
           * bit 1 - GBT_TRK_ErrCnt         0x01
           * bit 2 - GBT_TRG_ErrCnt         0x02
           * bit 3 - GBT_Data_Rec           0x04
           * bit 4 - GTX_TRK_ErrCnt         0x08
           * bit 5 - GTX_Data_Rec           0x10
           */
          void LinkReset(uint8_t const& resets);

          /**
           * Reset the all link status registers
           * @param uint8_t resets control which bits to reset
           * OBSOLETE in new V2 firmware
           */
          void ResetLinks(uint8_t const& resets) {
            return;
            /*
              for (auto link = v_activeLinks.begin(); link != v_activeLinks.end(); ++link)
              LinkReset(link->first,resets);
            */
          };


          /**
           * OptoHybrid CONTROL module
           * Functions realted to the registers in the CONTROL block
           * in the OptoHybrid firmware
           **/

          //// VFAT \\\\*
          /**
           * Returns the VFAT tracking data mask that the OptoHybrid uses to determine which data
           * packets to send to the AMC
           *
           */
          uint32_t getVFATMask() {
            return readReg(getDeviceBaseNode(),toolbox::toString("CONTROL.VFAT.TRK_MASK")); };

          /**
           * Sets the VFAT tracking data mask that the OptoHybrid uses to determine which data
           *  packets to send to the AMC
           *  a 0 means the VFAT will NOT be masked, and it's data packets will go to the AMC
           *  a 1 means the VFAT WILL be masked, and it's data packets will NOT go to the AMC
           */
          void setVFATMask(uint32_t const mask) {
            DEBUG("HwOptoHybrid::setVFATMask setting tracking mask to "
                  << std::hex << std::setw(8) << std::setfill('0') << mask << std::dec);
            return writeReg(getDeviceBaseNode(),toolbox::toString("CONTROL.VFAT.TRK_MASK"),mask&0x00ffffff); };

          /**
           * Returns VFATs to the 0 run mode
           *
           */
          void resetVFATs() {
            return writeReg(getDeviceBaseNode(),toolbox::toString("CONTROL.VFAT.RESET"),0x1); };

          /**
           * Set the S-bit mask
           * @param uint32_t mask s-bits coming from specific GEB slots
           */
          void setSBitMask(uint32_t const mask) {
            writeReg(getDeviceBaseNode(),"CONTROL.VFAT.SBIT_MASK", mask&0x00ffffff); };

          /**
           * Read the S-bit mask
           * @retval uint32_t which slots s-bits are processed
           */
          uint32_t getSBitMask() {
            return readReg(getDeviceBaseNode(),"CONTROL.VFAT.SBIT_MASK"); };

          /**
           * Returns whether data with a bad CRC will be dropped at the OptoHybrid
           * and not passed on to the AMC
           */
          uint32_t getDropBadCRCStatus() {
            return readReg(getDeviceBaseNode(),toolbox::toString("CONTROL.VFAT.DROP_BAD_CRC")); };

          /**
           * Set whether the OptoHybrid will drop VFAT data packets received with a bad CRC
           *  a 0 means the OptoHybrid will NOT drop bad packets
           *  a 1 means the OptoHybrid WILL drop bad packets
           */
          void setDropBadCRCStatus(bool const drop) {
            DEBUG("HwOptoHybrid::setVFATMask setting DROP_BAD_CRC to "
                  << std::hex << drop << std::dec);
            return writeReg(getDeviceBaseNode(),toolbox::toString("CONTROL.VFAT.DROP_BAD_CRC"),drop); };

          //// TRIGGER \\\\*
          /**
           * Set the Trigger source
           * @param uint8_t mode
           * 0 from AMC TTC decoder over GTX
           * 1 from OptoHybrid firmware (T1 module)
           * 2 from external source (LEMO)
           * 3 Internal loopback of s-bits
           * 4 from all
           * 5 from AMC TTC decoder over GBT
           */
          void setTrigSource(uint8_t const& mode) {
            switch (mode) {
            case(OptoHybridTrigSrc::GTX_TTC):
              writeReg(getDeviceBaseNode(),"CONTROL.TRIGGER.SOURCE",mode);
              return;
            case(OptoHybridTrigSrc::INTERNAL):
              writeReg(getDeviceBaseNode(),"CONTROL.TRIGGER.SOURCE",mode);
              return;
            case(OptoHybridTrigSrc::EXTERNAL):
              writeReg(getDeviceBaseNode(),"CONTROL.TRIGGER.SOURCE",mode);
              return;
            case(OptoHybridTrigSrc::LOOPBACK):
              writeReg(getDeviceBaseNode(),"CONTROL.TRIGGER.SOURCE",mode);
              return;
            case(OptoHybridTrigSrc::GBT_TTC):
              writeReg(getDeviceBaseNode(),"CONTROL.TRIGGER.SOURCE",mode);
              return;
            case(OptoHybridTrigSrc::ALL):
              writeReg(getDeviceBaseNode(),"CONTROL.TRIGGER.SOURCE",mode);
              return;
            default:
              writeReg(getDeviceBaseNode(),"CONTROL.TRIGGER.SOURCE", OptoHybridTrigSrc::GBT_TTC);
              return;
            }
          };

          /**
           * Read the Trigger source
           * @retval uint8_t
           * 0 from AMC TTC decoder over GTX
           * 1 from OptoHybrid firmware (T1 module)
           * 2 from external source (LEMO)
           * 3 Internal loopback of s-bits
           * 4 from all
           * 5 from AMC TTC decoder over GBT
           */
          uint8_t getTrigSource() {
            return readReg(getDeviceBaseNode(),"CONTROL.TRIGGER.SOURCE"); };

          /**
           * Set the s-bit signal to loopback as a trigger source
           * @param uint8_t which s-bit to loopback
           */
          void setLoopbackSBitSource(uint8_t const& sbit) {
            writeReg(getDeviceBaseNode(),"CONTROL.TRIGGER.LOOPBACK",sbit); };

          /**
           * Get the s-bit signal loopedback as a trigger source
           * @returns uint8_t which s-bit is currently being loopedback
           */
          uint8_t getLoopbackSBitSource() {
            return readReg(getDeviceBaseNode(),"CONTROL.TRIGGER.LOOPBACK"); };

          /**
           * Set the trigger throttle on the OptoHyrbid
           * @param uint32_t how often to throttle the trigger
           */
          void setTriggerThrottle(uint32_t const& throttle) {
            writeReg(getDeviceBaseNode(),"CONTROL.TRIGGER.THROTTLE",throttle); };

          /**
           * Get current trigger throttling parameter
           * @returns uint32_t trigger throttling parameter
           */
          uint8_t getTriggerThrottle() {
            return readReg(getDeviceBaseNode(),"CONTROL.TRIGGER.THROTTLE"); };

          //// HDMI_OUTPUT \\\\*
          /**
           * Set the S-bit source
           * @param uint32_t mask which s-bits to forward (maximum 6)
           */
          void setHDMISBitSource(uint32_t const mask) {
            writeReg(getDeviceBaseNode(),"CONTROL.HDMI_OUTPUT.SBITS",mask); };

          /**
           * Set the S-bit source
           * @param std::array<uint8_t, 6> which s-bits to forward (maximum 6)
           */
          void setHDMISBitSource(std::array<uint8_t, 6> const sources) {
            uint32_t mask = 0x0;
            for (int i = 0; i < 6; ++i)
              mask |= (sources[i] << (5*i));
            setHDMISBitSource(mask);
          };

          /**
           * Read the S-bit source
           * @retval uint32_t which VFAT chips are sending S-bits
           */
          uint32_t getHDMISBitSource() {
            return readReg(getDeviceBaseNode(),"CONTROL.HDMI_OUTPUT.SBITS"); };

          /* /\** */
          /*  * Set the S-bit source */
          /*  * @param uint32_t mask which s-bits to forward (maximum 6, depending on the mode) */
          /*  * OBSOLETE: replaced by setHDMISBitSource */
          /*  *\/ */
          /* void setSBitSource(uint32_t const mask) { */
          /*   writeReg(getDeviceBaseNode(),"CONTROL.HDMI_OUTPUT.SBITS",mask); }; */

          /* /\** */
          /*  * Set the S-bit source */
          /*  * @param std::array<uint8_t, 6> which s-bits to forward (maximum 6) */
          /*  * OBSOLETE: replaced by setHDMISBitSource */
          /*  *\/ */
          /* void setSBitSource(std::array<uint8_t, 6> const sources) { */
          /*   uint32_t mask = 0x0; */
          /*   for (int i = 0; i < 6; ++i) */
          /*     mask |= (sources[i] << (5*i)); */
          /*   setSBitSource(mask); */
          /* }; */

          /* /\** */
          /*  * Read the S-bit source */
          /*  * @retval uint32_t which VFAT chips are sending S-bits */
          /*  * OBSOLETE: replaced by getHDMISBitSource */
          /*  *\/ */
          /* uint32_t getSBitSource() { */
          /*   return readReg(getDeviceBaseNode(),"CONTROL.HDMI_OUTPUT.SBITS"); }; */

          /**
           * Set the S-bit mode
           * @param uint32_t mode of sending s-bits out the HDMI connector
           *        if mode is 0 - the SBITS parameter will correspond to a given VFAT [0-23]
           *        if mode is 1 - the SBITS parameter will correspond to a given iEta sector [0-7]
           *        if mode is 2 - the output sbits will correspond to the I2C sector
           *        if mode is 3 - the output sbits will be constant 0's
           */
          void setHDMISBitMode(uint8_t const mode) {
            writeReg(getDeviceBaseNode(),"CONTROL.HDMI_OUTPUT.SBIT_MODE", mode); };

          /**
           * Read the S-bit mode
           * @retval uint32_t which mode the OptoHybrid is sending s-bits to the HDMI connector
           */
          uint32_t getHDMISBitMode() {
            return readReg(getDeviceBaseNode(),"CONTROL.HDMI_OUTPUT.SBIT_MODE"); };

          //// Zero Suppression \\\\*
          /**
           * Turn the zero-suppression on or off
           * @param bool whether or not ZS is turned on in the OptoHybrid
           */
          void setZS(bool const& on) {
            writeReg(getDeviceBaseNode(),"CONTROL.ZS", uint32_t(on)); };

          /**
           * Read the zero-suppression mode
           * @retval bool whether or not the OptoHybrid is zero-suppressing the data
           */
          uint32_t getZSStatus() {
            return readReg(getDeviceBaseNode(),"CONTROL.ZS"); };

          //// Clocking \\\\*
          /**
           * Setup the OptoHybrid clock
           * @param uint8_t source
           * in V2B switch between GBT clock and HDMI clock
           * 0x0 on board oscillator
           * 0x1 GBT recovered clock
           * ***in V2A only***
           * 0x0 on board oscillator
           * 0x1 GTX recovered clock
           * 0x2 external clock from LEMO expansion module
           */
          void setReferenceClock(uint8_t const& source) {
            writeReg(getDeviceBaseNode(),"CONTROL.CLOCK.REF_CLK", (uint32_t)source);
          };

          /**
           * Get is the current clock source
           * @returns uint32_t clock source
           * in V2B switch between GBT clock and HDMI clock
           * 0x0 on board oscillator
           * 0x1 GBT recovered clock
           * ***in V2A only***
           * 0x0 on board oscillator
           * 0x1 GTX recovered clock
           * 0x2 external clock from LEMO expansion module
           */
          uint32_t getReferenceClock() {
            return readReg(getDeviceBaseNode(),"CONTROL.CLOCK.REF_CLK");
          };

          /**
           * OptoHybrid STATUS module
           * Functions realted to the registers in the STATUS block
           * in the OptoHybrid firmware
           **/
          // FW


          //// PLL Status \\\\*
          /**
           * @returns whether the QPLL is locked
           */
          bool isQPLLLocked() { return readReg(getDeviceBaseNode(),"STATUS.QPLL_LOCK"); };

          /**
           * @returns whether the QPLL read by the FPGA PLL is locked
           */
          bool isQPLLFPGAPLLLocked() { return readReg(getDeviceBaseNode(),"STATUS.QPLL_FPGA_PLL_LOCK"); };

          /**
           * @returns whether there is a critical error detected due to SEU
           */
          bool hasSEUError() { return readReg(getDeviceBaseNode(),"STATUS.SEU"); };


          /** @defgroup t1generator Optohybrid T1 Command Generator
           *  Functions related to the OptoHybrid T1 controller module
           *  @{
           */

          /**
           * @brief the T1 module is very different between V1/1.5 and V2
           * One must select the mode
           * One must select the signal
           */
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
           */
          void configureT1Generator(uint8_t const& mode, uint8_t const& type,
                                    T1Sequence sequence, bool reset);

          /**
           * @brief Start the T1 generator (must be configured first or have a configuration already loaded)
           * @param uint32_t ntrigs number of signals to send before stopping (0 will send continuously)
           * @param uint32_t rate rate at which to repeat the sending
           * @param uint32_t delay delay between CalPulse and L1A (only for T1 mode 1)
           */
          void startT1Generator(uint32_t const& ntrigs, uint32_t const& rate, uint32_t const& delay);

          /**
           * @brief Stop the T1 generator
           * @param bool reset tells whether to reset the state of the module
           */
          void stopT1Generator(bool reset);

          /**
           * @brief Reset the T1 generator firmware module
           */
          void resetT1Generator() { writeReg(getDeviceBaseNode(), "T1Controller.RESET", 0x1); };

          /**
           * @brief Status of the T1 generator
           * @returns uint8_t the status of the T1 generator, telling which mode is running
           * (0 is nothing running)
           */
          uint8_t getT1GeneratorStatus() {
            return readReg(getDeviceBaseNode(),"T1Controller.MONITOR");
          };

          /**
           * Send an internal L1A
           * @param uint32_t ntrigs, how many L1As to send
           * @param uint32_t rate, rate at which signals will be generated
           */
          void sendL1A(uint32_t const& ntrigs, uint32_t const& rate=1);

          /**
           * Send an internal CalPulse
           * @param uint32_t npulse, how many CalPulses to send
           * @param uint32_t rate, rate at which signals will be generated
           */
          void sendCalPulse(uint32_t const& npulse, uint32_t const& rate=1);

          /**
           * Send a CalPulse followed by an L1A
           * @param uint32_t npulse, how many pairs to send
           * @param uint32_t delay, how long between L1A and CalPulse
           * @param uint32_t rate, rate at which signals will be generated
           */
          void sendL1ACalPulse(uint32_t const& npulse, uint32_t const& delay, uint32_t const& rate=1);

          /**
           * Send an internal Resync
           * @param uint32_t nresync, total number of resync signals to send
           * @param uint32_t rate, rate at which signals will be generated
           */
          void sendResync(uint32_t const& nresync=1, uint32_t const& rate=1);

          /**
           * Send an internal BC0
           * @param uint32_t nbc0, total number of BC0 signals to send
           * @param uint32_t rate, rate at which signals will be generated
           */
          void sendBC0(uint32_t const& nbc0=1, uint32_t const& rate=1);
          /** @} */ // end of t1generator


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
           * Get the recorded number of signals sent/received by the OptoHybrid wishbone master
           * @returns OptoHybridWBMasterCounters struct, with updated values
           */
          OptoHybridWBMasterCounters getWBMasterCounters() { return m_wbMasterCounters; };
          void updateWBMasterCounters();
          void resetWBMasterCounters();

          /**
           * @ingroup ohcounters
           * Get the recorded number of signals sent/received by the OptoHybrid wishbone slave
           * @returns OptoHybridWBSlaveCounters struct, with updated values
           */
          OptoHybridWBSlaveCounters getWBSlaveCounters() { return m_wbSlaveCounters; };
          void updateWBSlaveCounters();
          void resetWBSlaveCounters();

          /**
           * @ingroup ohcounters
           * Get the recorded number of signals sent/received by the OptoHybrid wishbone slave
           * @returns OptoHybridT1Counters struct, with updated values
           */
          OptoHybridT1Counters getT1Counters() { return m_t1Counters; };
          void updateT1Counters();
          void resetT1Counters();

          /**
           * @ingroup ohcounters
           * Get the recorded number of valid/incorrect CRCs performed by the OptoHybrid
           *  on the data packets received from the VFATs
           * @returns OptoHybridT1Counters struct, with updated values
           */
          OptoHybridVFATCRCCounters getVFATCRCCounters() { return m_vfatCRCCounters; };
          void updateVFATCRCCounters();
          void resetVFATCRCCounters();

          /**
           * @ingroup ohcounters
           * Get the number of valid/incorrect CRCs performed by the OptoHybrid
           * on the received data packets from a given VFAT
           * @param slot specifies which VFAT counters to read
           * 0-23
           */
          std::pair<uint32_t,uint32_t> getVFATCRCCount(uint8_t const& chip);

          /**
           * @ingroup ohcounters
           * Reset the number of valid/incorrect CRCs performed by the OptoHybrid
           * on the received data packets from a given VFAT
           * @param slot specifies which VFAT counters to read
           * 0-23
           */
          void resetVFATCRCCount(uint8_t const& chip);

          /**
           * @ingroup ohcounters
           * Reset the number of valid/incorrect CRCs performed by the OptoHybrid
           * on the received data packets from all VFATs
           */
          void resetVFATCRCCount();

          /** @defgroup t1counters T1 Counters
           *  Functions related to the various T1 counters on the OptoHybrid
           *  @ingroup ohcounters
           * @param signal specifies which T1 signal counter to read
           * ** FIXME MAKE THESE ENUMS **
           *  - 0 L1A
           *  - 1 CalPulse
           *  - 2 Resync
           *  - 3 BC0
           * @param mode specifies which T1 counter to read
           * ** FIXME MAKE THESE ENUMS **
           *  - 0 from AMC TTC decoder over GTX
           *  - 1 from the T1 generator in the firmware
           *  - 2 from an external source
           *  - 3 from looping back the sbits
           *  - 4 sent along the GEB
           *  - 5 from AMC TTC decoder over GBT
           *  - 6 all (reset only)
           * @returns uint32_t the requested counter value
           *  @{
           */
          /**
           * Get the recorded number of T1 signals
           */
          uint32_t getT1Count(uint8_t const& signal, uint8_t const& mode);

          /**
           * Get the recorded number of L1A signals
           */
          uint32_t getL1ACount(uint8_t const& mode) { return getT1Count(0x0, mode); };

          /**
           * Get the recorded number of CalPulse signals
           */
          uint32_t getCalPulseCount(uint8_t const& mode) { return getT1Count(0x1, mode); };

          /**
           * Get the recorded number of Resync signals
           */
          uint32_t getResyncCount(uint8_t const& mode=0x0) { return getT1Count(0x2, mode); };

          /**
           * Get the recorded number of BC0 signals
           */
          uint32_t getBC0Count(uint8_t const& mode=0x0) { return getT1Count(0x3, mode); };

          /**
           * Get the recorded number of BXCount signals
           * OBSOLETE in V2 firmware
           */
          uint32_t getBXCountCount() { return 0x0; };

          /**
           * Reset the recorded number of T1 signals
           */
          void resetT1Count(uint8_t const& signal, uint8_t const& mode=0x5);

          /**
           * Reset recorded number of L1A signals
           */
          void resetL1ACount(uint8_t const& mode=0x5) {
            resetT1Count(0x0,mode);
          };

          /**
           * Reset recorded number of CalPulse signals
           */
          void resetCalPulseCount(uint8_t const& mode=0x5) {
            return resetT1Count(0x1, mode); };

          /**
           * Reset recorded number of Resync signals
           */
          void resetResyncCount(uint8_t const& mode=0x5) {
            return resetT1Count(0x2, mode); };

          /**
           * Reset recorded number of BC0 signals
           */
          void resetBC0Count(uint8_t const& mode=0x5) {
            return resetT1Count(0x3, mode); };

          /**
           * Reset recorded number of BXCount signals
           * OBSOLETE in V2 firmawre
           */
          void resetBXCount() { return; };

          /** @} */ // end of t1counters

          /**
           * @ingroup ohcounters
           * @returns uint32_t showing the number of times the QPLL unlocked and re-locked
           */
          uint32_t getQPLLUnlockCount() { return readReg(getDeviceBaseNode(),"COUNTERS.QPLL_LOCK"); };
          void resetQPLLUnlockCount() { writeReg(getDeviceBaseNode(),"COUNTERS.QPLL_LOCK.Reset", 0x1); };

          /**
           * @ingroup ohcounters
           * @returns uint32_t showing the number of times the QPLL FPGA PLL unlocked and re-locked
           */
          uint32_t getQPLLFPGAPLLUnlockCount() { return readReg(getDeviceBaseNode(),"COUNTERS.QPLL_FPGA_PLL_LOCK"); };
          void resetQPLLFPGAPLLUnlockCount() { writeReg(getDeviceBaseNode(),"COUNTERS.QPLL_FPGA_PLL_LOCK.Reset", 0x1); };


          //////// UNCATEGORIZED/OBSOLETE? \\\\\\\\*
          /**
           * Setup the VFAT clock
           * @param bool source true uses the external clock, false uses the onboard clock
           * @param bool fallback uses the external clock, false uses the onboard clock
           * NOT YET AVAILABLE IN V2 FIRMWARE
           */
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
           */
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
           */
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
           */
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
                                              uint32_t    const& mask=ALL_VFATS_BCAST_MASK,
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
                              uint32_t    const& mask=ALL_VFATS_BCAST_MASK,
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
           * Sends a write request for all setup registers on each VFAT specified by the mask
           * @param uint8_t vthreshold1 value to write
           * @param uint8_t vthreshold2 value to write
           * @param uint8_t latency value to write
           * @param uint32_t broadcastMask is the list of VFATs to send the broadcast commands to
           */
          void setVFATsToDefaults(uint8_t  const& vt1,
                                  uint8_t  const& vt2,
                                  uint8_t  const& latency,
                                  uint32_t const& broadcastMask);

          /**
           * Sends a write request for all setup registers on each VFAT specified by the mask
           * @param std::map<std::string,uint8_t> map of VFAT register name to value to broadcast
           * @param uint32_t broadcastMask is the list of VFATs to send the broadcast commands to
           */
          void setVFATsToDefaults(std::map<std::string, uint8_t> const& regvals,
                                  uint32_t const& broadcastMask);


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
          bool b_is_initial;  ///<
          std::vector<std::pair<uint8_t,uint32_t> > m_chipIDs;
          uint32_t m_disabledMask;   ///<
          uint32_t m_connectedMask;  ///<
          uint8_t m_controlLink;     ///<
          int m_slot;                ///<

        };  // class HwOptoHybrid
    }  // namespace gem::hw::glib
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_OPTOHYBRID_HWOPTOHYBRID_H
