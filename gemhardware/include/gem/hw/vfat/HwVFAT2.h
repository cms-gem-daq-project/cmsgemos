#ifndef GEM_HW_VFAT_HWVFAT2_H
#define GEM_HW_VFAT_HWVFAT2_H

#include "gem/hw/GEMHwDevice.h"

#include "gem/hw/vfat/VFAT2Settings.h"
#include "gem/hw/vfat/VFAT2SettingsEnums.h"
#include "gem/hw/vfat/VFAT2Enums2Strings.h"
#include "gem/hw/vfat/VFAT2Strings2Enums.h"
//#include "gem/hw/vfat/VFAT2EnumStrings.h"

#include "gem/hw/vfat/exception/Exception.h"

typedef std::pair<std::string, uint8_t> vfat_reg_pair;
typedef std::vector<vfat_reg_pair>      vfat_reg_pair_list;

typedef std::pair<std::string, uhal::ValWord<uint8_t> > vfat_reg_value;
typedef std::vector<vfat_reg_value>                     vfat_reg_val_list;

namespace uhal {
  class HwInterface;
}

namespace gem {
  namespace hw {
    namespace vfat {
      //class VFAT2ChannelData;
      //class VFAT2ChannelSettings;
      //class VFAT2Settings;

      class HwVFAT2: public gem::hw::GEMHwDevice
        {
        public:
          static const unsigned N_VFAT2_CHANNELS = 128;

          typedef struct TransactionErrors {
            int Error     ;
            int Invalid   ;
            int RWMismatch;

          TransactionErrors() : Error(0),Invalid(0),RWMismatch(0) {}
            void reset()       {Error=0; Invalid=0; RWMismatch=0;return; }
          } TransactionErrors;

          HwVFAT2(std::string const& vfatDevice, std::string const& connectionFile);
          HwVFAT2(std::string const& vfatDevice, std::string const& connectionURI,
                  std::string const& addressTable);
          HwVFAT2(std::string const& vfatDevice, uhal::HwInterface& uhalDevice);
          HwVFAT2(std::string const& vfatDevice="VFAT13");

          virtual ~HwVFAT2();

          /**
           * @brief  Print the error counts for the device (calls also the GEMHwDevice method
           * @returns string of error counts
           */
          virtual std::string printErrorCounts() const;

          /**
           * @brief  Load some default values into the VFAT registers
           *
           */
          void loadDefaults();
	  void printDefaults(std::ofstream& SetupFile);

          /**
           * @brief  bool isHwConnected()
           * Checks to see if the VFAT device is connected
           * @returns true if the hardware pointer is valid and a successful read has occurred
           */
          virtual bool isHwConnected();

          /**
           * @brief  uint8_t  readVFATReg( std::string const& regName, bool debug)
           * Reads a register on the VFAT2 chip, returns the 8-bit value of the register,
           * used only in isHwConnected
           * @param regName is the name of the VFAT2 register to read
           * @param debug
           * @returns 8-bit register from the VFAT chip
           */
          uint8_t  readVFATReg( std::string const& regName, bool debug);

          /**
           * @brief  uint8_t  readVFATReg( std::string const& regName)
           * Reads a register on the VFAT2 chip, returns the 8-bit value of the register
           * @param regName is the name of the VFAT2 register to read
           * @returns 8-bit register from the VFAT chip
           * @info
           * check the transaction status
           * bit 31:27 - unused
           * bit 26 - error
           * bit 25 - valid
           * bit 24 - r/w
           * bit 23:16 - VFAT number
           * bit 15:8  - VFAT register
           * bit 7:0   - register value
           */
          uint8_t  readVFATReg( std::string const& regName);

          /**
           * @brief  readVFATRegs( vfat_reg_pair_list &regList)
           * Reads a list of registers on the VFAT2 chip into the provided key pair
           * @param regList is the list of pairs of register names to read, and values to return
           */
          void     readVFATRegs( vfat_reg_pair_list &regList);

          /**
           * @brief  readVFAT2Counters()
           * Reads the counters on the VFAT2 chip and writes the values into the m_vfatParams object
           */
          //void     readVFAT2Counters(gem::hw::vfat::VFAT2ControlParams &params);
          void     readVFAT2Counters();

          /**
           * @brief  writeVFATReg( std::string const& regName, uint8_t const& writeVal)
           * Writes a value to a register on the VFAT2 chip
           * @param regName is the name of the VFAT2 register to write to
           * @param writeValue is the value to write into the VFAT register
           */
          void     writeVFATReg(std::string const& regName,
                                uint8_t     const& writeVal) {
            writeReg(getDeviceBaseNode(), regName, static_cast<uint32_t>(writeVal)); }

          /**
           * @brief  writeVFATReg( vfat_reg_pair_list const& regList)
           * Writes to a list of VFAT2 registers from a list of pairs of register name and value
           * done with a single dispatch call
           * @param regList is the list of pairs of register names and values to write
           */
          void     writeVFATRegs(vfat_reg_pair_list const& regList) {
            register_pair_list fullRegList;
            for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg)
              fullRegList.push_back(std::make_pair(getDeviceBaseNode()+"."+curReg->first,static_cast<uint32_t>(curReg->second)));
            writeRegs(fullRegList);
          }

          /**
           * @brief  writeValueToVFATRegs( std::vector<std::string> const& regList, uint8_t const& regValue)
           * Writes a single value to a list of VFAT2 registers, all done with a single dispatch call
           * @param regList is the list of registers to write a specific value with
           * @param regValue is the value to write to each of the registers in the list
           */
          void     writeValueToVFATRegs(std::vector<std::string> const& regList, uint8_t const& regValue) {
            std::vector<std::string > fullRegList;
            for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg)
              fullRegList.push_back(getDeviceBaseNode()+"."+*curReg);
            writeValueToRegs(regList,static_cast<uint32_t>(regValue)); }

          //control functions
          //void reset();

          //get read only registers
          /**
           * @brief  getSlot()
           * @returns the GEB slot the VFAT is connected to
           */
          uint16_t getSlot() { return m_slot; }

          /**
           * @brief  getChipID()
           * @returns the 16 bit chipID for the chip
           */
          uint16_t getChipID() {
            return ((readVFATReg("ChipID1"))<<8)|(readVFATReg("ChipID0")); }

          /**
           * @brief  getHitCount()
           * @returns the hit counter value (from the three hit count registers)
           */
          uint32_t getHitCount() {
            return (((readVFATReg("HitCount2"))<<16)|((readVFATReg("HitCount1")))<<8)|(readVFATReg("HitCount0")); }

          /**
           * @brief  getUpsetCount()
           * @returns value in the upset counter on the VFAT
           */
          uint8_t  getUpsetCount() { return readVFATReg("UpsetReg");    }

          //Set control register settings
          void setAllSettings(const gem::hw::vfat::VFAT2ControlParams &params);

          //Control register settings
          /// might be good to overload them to act on local variables
          /// and do a single IPBus transaction...

          void setRunMode(VFAT2RunMode mode) {
            uint8_t settings = readVFATReg("ContReg0");
            writeVFATReg("ContReg0",
                         (settings&~VFAT2ContRegBitMasks::RUNMODE)|
                         (mode<<VFAT2ContRegBitShifts::RUNMODE)); }

          void setRunMode(VFAT2RunMode mode, uint8_t& settings) {
            settings = (settings&~VFAT2ContRegBitMasks::RUNMODE)|
              (mode<<VFAT2ContRegBitShifts::RUNMODE); }

          void setRunMode(uint8_t mode) {
            setRunMode(static_cast<VFAT2RunMode>(mode)); }
          void setRunMode(uint8_t mode, uint8_t&  settings) {
            setRunMode(static_cast<VFAT2RunMode>(mode), settings); }

          void setTriggerMode(VFAT2TrigMode mode) {
            uint8_t settings = readVFATReg("ContReg0");
            writeVFATReg("ContReg0",
                         (settings&~VFAT2ContRegBitMasks::TRIGMODE)|
                         (mode<<VFAT2ContRegBitShifts::TRIGMODE)); }

          void setTriggerMode(VFAT2TrigMode mode, uint8_t& settings) {
            settings = (settings&~VFAT2ContRegBitMasks::TRIGMODE)|
              (mode<<VFAT2ContRegBitShifts::TRIGMODE); }

          void setTriggerMode(uint8_t mode) {
            setTriggerMode(static_cast<VFAT2TrigMode>(mode)); }
          void setTriggerMode(uint8_t mode, uint8_t& settings) {
            setTriggerMode(static_cast<VFAT2TrigMode>(mode), settings); }

          void setCalibrationMode(VFAT2CalibMode mode) {
            uint8_t settings = readVFATReg("ContReg0");
            writeVFATReg("ContReg0",
                         (settings&~VFAT2ContRegBitMasks::CALMODE)|
                         (mode<<VFAT2ContRegBitShifts::CALMODE)); }

          void setCalibrationMode(VFAT2CalibMode mode, uint8_t& settings) {
            settings = (settings&~VFAT2ContRegBitMasks::CALMODE)|
              (mode<<VFAT2ContRegBitShifts::CALMODE); }

          void setCalibrationMode(uint8_t mode) {
            setCalibrationMode(static_cast<VFAT2CalibMode>(mode)); }
          void setCalibrationMode(uint8_t mode, uint8_t& settings) {
            setCalibrationMode(static_cast<VFAT2CalibMode>(mode), settings); }

          void setMSPolarity(VFAT2MSPol polarity) {
            uint8_t settings = readVFATReg("ContReg0");
            writeVFATReg("ContReg0",
                         (settings&~VFAT2ContRegBitMasks::MSPOL)|
                         (polarity<<VFAT2ContRegBitShifts::MSPOL)); }

          void setMSPolarity(VFAT2MSPol polarity, uint8_t& settings) {
            settings = (settings&~VFAT2ContRegBitMasks::MSPOL)|
              (polarity<<VFAT2ContRegBitShifts::MSPOL); }

          void setMSPolarity(uint8_t mode) {
            setMSPolarity(static_cast<VFAT2MSPol>(mode)); }
          void setMSPolarity(uint8_t mode, uint8_t& settings) {
            setMSPolarity(static_cast<VFAT2MSPol>(mode), settings); }

          void setCalPolarity(VFAT2CalPol polarity) {
            uint8_t settings = readVFATReg("ContReg0");
            writeVFATReg("ContReg0",
                         (settings&~VFAT2ContRegBitMasks::CALPOL)|
                         (polarity<<VFAT2ContRegBitShifts::CALPOL)); }

          void setCalPolarity(VFAT2CalPol polarity, uint8_t& settings) {
            settings = (settings&~VFAT2ContRegBitMasks::CALPOL)|
              (polarity<<VFAT2ContRegBitShifts::CALPOL); }

          void setCalPolarity(uint8_t mode) {
            setCalPolarity(static_cast<VFAT2CalPol>(mode)); }
          void setCalPolarity(uint8_t mode, uint8_t& settings) {
            setCalPolarity(static_cast<VFAT2CalPol>(mode), settings); }

          void setProbeMode(VFAT2ProbeMode mode) {
            uint8_t settings = readVFATReg("ContReg1");
            writeVFATReg("ContReg1",
                         (settings&~VFAT2ContRegBitMasks::PROBEMODE)|
                         (mode<<VFAT2ContRegBitShifts::PROBEMODE)); }

          void setProbeMode(VFAT2ProbeMode mode, uint8_t& settings) {
            settings = (settings&~VFAT2ContRegBitMasks::PROBEMODE)|
              (mode<<VFAT2ContRegBitShifts::PROBEMODE); }

          void setProbeMode(uint8_t mode) {
            setProbeMode(static_cast<VFAT2ProbeMode>(mode)); }
          void setProbeMode(uint8_t mode, uint8_t& settings) {
            setProbeMode(static_cast<VFAT2ProbeMode>(mode), settings); }

          void setLVDSMode(VFAT2LVDSMode mode) {
            uint8_t settings = readVFATReg("ContReg1");
            writeVFATReg("ContReg1",
                         (settings&~VFAT2ContRegBitMasks::LVDSMODE)|
                         (mode<<VFAT2ContRegBitShifts::LVDSMODE)); }

          void setLVDSMode(VFAT2LVDSMode mode, uint8_t& settings) {
            settings = (settings&~VFAT2ContRegBitMasks::LVDSMODE)|
              (mode<<VFAT2ContRegBitShifts::LVDSMODE); }

          void setLVDSMode(uint8_t mode) {
            setLVDSMode(static_cast<VFAT2LVDSMode>(mode)); }
          void setLVDSMode(uint8_t mode, uint8_t& settings) {
            setLVDSMode(static_cast<VFAT2LVDSMode>(mode), settings); }

          void setDACMode(VFAT2DACMode mode) {
            uint8_t settings = readVFATReg("ContReg1");
            writeVFATReg("ContReg1",
                         (settings&~VFAT2ContRegBitMasks::DACMODE)|
                         (mode<<VFAT2ContRegBitShifts::DACMODE)); }

          void setDACMode(VFAT2DACMode mode, uint8_t& settings) {
            settings = (settings&~VFAT2ContRegBitMasks::DACMODE)|
              (mode<<VFAT2ContRegBitShifts::DACMODE); }

          void setDACMode(uint8_t mode) {
            setDACMode(static_cast<VFAT2DACMode>(mode)); }
          void setDACMode(uint8_t mode, uint8_t& settings) {
            setDACMode(static_cast<VFAT2DACMode>(mode), settings); }

          void setHitCountCycleTime(VFAT2ReHitCT cycleTime) {
            uint8_t settings = readVFATReg("ContReg1");
            writeVFATReg("ContReg1",
                         (settings&~VFAT2ContRegBitMasks::REHITCT)|
                         (cycleTime<<VFAT2ContRegBitShifts::REHITCT)); }

          void setHitCountCycleTime(VFAT2ReHitCT cycleTime, uint8_t& settings) {
            settings = (settings&~VFAT2ContRegBitMasks::REHITCT)|
              (cycleTime<<VFAT2ContRegBitShifts::REHITCT); }

          void setHitCountCycleTime(uint8_t mode) {
            setHitCountCycleTime(static_cast<VFAT2ReHitCT>(mode)); }
          void setHitCountCycleTime(uint8_t mode, uint8_t& settings) {
            setHitCountCycleTime(static_cast<VFAT2ReHitCT>(mode), settings); }

          void setHitCountMode(VFAT2HitCountMode mode) {
            uint8_t settings = readVFATReg("ContReg2");
            writeVFATReg("ContReg2",
                         (settings&~VFAT2ContRegBitMasks::HITCOUNTMODE)|
                         (mode<<VFAT2ContRegBitShifts::HITCOUNTMODE)); }

          void setHitCountMode(VFAT2HitCountMode mode, uint8_t& settings) {
            settings = (settings&~VFAT2ContRegBitMasks::HITCOUNTMODE)|
              (mode<<VFAT2ContRegBitShifts::HITCOUNTMODE); }

          void setHitCountMode(uint8_t mode) {
            setHitCountMode(static_cast<VFAT2HitCountMode>(mode)); }
          void setHitCountMode(uint8_t mode, uint8_t& settings) {
            setHitCountMode(static_cast<VFAT2HitCountMode>(mode), settings); }

          void setMSPulseLength(VFAT2MSPulseLength length) {
            uint8_t settings = readVFATReg("ContReg2");
            writeVFATReg("ContReg2",
                         (settings&~VFAT2ContRegBitMasks::MSPULSELENGTH)|
                         (length<<VFAT2ContRegBitShifts::MSPULSELENGTH)); }

          void setMSPulseLength(VFAT2MSPulseLength length, uint8_t& settings) {
            settings = (settings&~VFAT2ContRegBitMasks::MSPULSELENGTH)|
              (length<<VFAT2ContRegBitShifts::MSPULSELENGTH); }

          void setMSPulseLength(uint8_t mode) {
            setMSPulseLength(static_cast<VFAT2MSPulseLength>(mode)); }
          void setMSPulseLength(uint8_t mode, uint8_t& settings) {
            setMSPulseLength(static_cast<VFAT2MSPulseLength>(mode), settings); }

          void setInputPadMode(VFAT2DigInSel mode) {
            uint8_t settings = readVFATReg("ContReg2");
            writeVFATReg("ContReg2",
                         (settings&~VFAT2ContRegBitMasks::DIGINSEL)|
                         (mode<<VFAT2ContRegBitShifts::DIGINSEL)); }

          void setInputPadMode(VFAT2DigInSel mode, uint8_t& settings) {
            settings = (settings&~VFAT2ContRegBitMasks::DIGINSEL)|
              (mode<<VFAT2ContRegBitShifts::DIGINSEL); }

          void setInputPadMode(uint8_t mode) {
            setInputPadMode(static_cast<VFAT2DigInSel>(mode)); }
          void setInputPadMode(uint8_t mode, uint8_t& settings) {
            setInputPadMode(static_cast<VFAT2DigInSel>(mode), settings); }

          void setTrimDACRange(VFAT2TrimDACRange range) {
            uint8_t settings = readVFATReg("ContReg3");
            writeVFATReg("ContReg3",
                         (settings&~VFAT2ContRegBitMasks::TRIMDACRANGE)|
                         (range<<VFAT2ContRegBitShifts::TRIMDACRANGE)); }

          void setTrimDACRange(VFAT2TrimDACRange range, uint8_t& settings) {
            settings = (settings&~VFAT2ContRegBitMasks::TRIMDACRANGE)|
              (range<<VFAT2ContRegBitShifts::TRIMDACRANGE); }

          void setTrimDACRange(uint8_t mode) {
            setTrimDACRange(static_cast<VFAT2TrimDACRange>(mode)); }
          void setTrimDACRange(uint8_t mode, uint8_t& settings) {
            setTrimDACRange(static_cast<VFAT2TrimDACRange>(mode), settings); }

          void setBandgapPad(VFAT2PadBandgap mode) {
            uint8_t settings = readVFATReg("ContReg3");
            writeVFATReg("ContReg3",
                         (settings&~VFAT2ContRegBitMasks::PADBANDGAP)|
                         (mode<<VFAT2ContRegBitShifts::PADBANDGAP)); }

          void setBandgapPad(VFAT2PadBandgap mode, uint8_t& settings) {
            settings = (settings&~VFAT2ContRegBitMasks::PADBANDGAP)|
              (mode<<VFAT2ContRegBitShifts::PADBANDGAP); }

          void setBandgapPad(uint8_t mode) {
            setBandgapPad(static_cast<VFAT2PadBandgap>(mode)); }
          void setBandgapPad(uint8_t mode, uint8_t& settings) {
            setBandgapPad(static_cast<VFAT2PadBandgap>(mode), settings); }

          void sendTestPattern(VFAT2DFTestPattern send) {
            uint8_t settings = readVFATReg("ContReg3");
            writeVFATReg("ContReg3",
                         (settings&~VFAT2ContRegBitMasks::DFTESTMODE)|
                         (send<<VFAT2ContRegBitShifts::DFTESTMODE)); }

          void sendTestPattern(VFAT2DFTestPattern send, uint8_t& settings) {
            settings = (settings&~VFAT2ContRegBitMasks::DFTESTMODE)|
              (send<<VFAT2ContRegBitShifts::DFTESTMODE); }

          void sendTestPattern(uint8_t mode) {
            sendTestPattern(static_cast<VFAT2DFTestPattern>(mode)); }
          void sendTestPattern(uint8_t mode, uint8_t& settings) {
            sendTestPattern(static_cast<VFAT2DFTestPattern>(mode), settings); }

          //////////////////////////////
          void setLatency(uint8_t latency) {writeVFATReg("Latency",latency); }

          void setIPreampIn(  uint8_t value) { writeVFATReg("IPreampIn",  value); }
          void setIPreampFeed(uint8_t value) { writeVFATReg("IPreampFeed",value); }
          void setIPreampOut( uint8_t value) { writeVFATReg("IPreampOut", value); }
          void setIShaper(    uint8_t value) { writeVFATReg("IShaper",    value); }
          void setIShaperFeed(uint8_t value) { writeVFATReg("IShaperFeed",value); }
          void setIComp(      uint8_t value) { writeVFATReg("IComp",      value); }

          void setVCal(       uint8_t value  ) { writeVFATReg("VCal",        value); }
          void setVThreshold1(uint8_t value  ) { writeVFATReg("VThreshold1", value); }
          void setVThreshold2(uint8_t value=0) { writeVFATReg("VThreshold2", value); }
          void setCalPhase(   uint8_t value  ) { writeVFATReg("CalPhase",    value); }

          /*** may want to be able to set these values to a human readable number
               lookup done through a LUT
               void setIPreampIn(  float value) { writeVFATReg("IPreampIn",  value); }  ;
               void setIPreampFeed(float value) { writeVFATReg("IPreampFeed",value); }  ;
               void setIPreampOut( float value) { writeVFATReg("IPreampOut", value); }  ;
               void setIShaper(    float value) { writeVFATReg("IShaper",    value); }  ;
               void setIShaperFeed(float value) { writeVFATReg("IShaperFeed",value); }  ;
               void setIComp(      float value) { writeVFATReg("IComp",      value); }  ;

               void setVCal(       float value  ) { writeVFATReg("VCal",        value); }  ;
               void setVThreshold1(float value  ) { writeVFATReg("VThreshold1", value); }  ;
               void setVThreshold2(float value=0) { writeVFATReg("VThreshold2", value); }  ;
               void setCalPhase(   float value  ) { writeVFATReg("CalPhase",    value); }  ;
          **/
          //////////////////////////////

          //void getAllSettings(gem::hw::vfat::VFAT2ControlParams &params);
          //void getAllSettings() {
          //  return getAllSettings(m_vfatParams); }

          /**
           * @brief  Get all the chip settings
           * should be private
           */
          void getAllSettings();

          //Get control register settings
          //CR0:<7:0::calMode<7:5>,calPol<4>.msPol<3>,trigMode<2:1>,runMode<0>>
          uint8_t getCR0()     {
            return (readVFATReg("ContReg0")); }
          uint8_t getRunMode()     {
            return (getCR0()&(VFAT2ContRegBitMasks::RUNMODE))>>VFAT2ContRegBitShifts::RUNMODE; }
          uint8_t getRunMode(uint8_t regVal)     {
            return (regVal&(VFAT2ContRegBitMasks::RUNMODE))>>VFAT2ContRegBitShifts::RUNMODE; }
          uint8_t getTriggerMode() {
            return (getCR0()&(VFAT2ContRegBitMasks::TRIGMODE))>>VFAT2ContRegBitShifts::TRIGMODE; }
          uint8_t getTriggerMode(uint8_t regVal) {
            return (regVal&(VFAT2ContRegBitMasks::TRIGMODE))>>VFAT2ContRegBitShifts::TRIGMODE; }
          uint8_t getMSPolarity() {
            return (getCR0()&(VFAT2ContRegBitMasks::MSPOL))>>VFAT2ContRegBitShifts::MSPOL; }
          uint8_t getMSPolarity(uint8_t regVal) {
            return (regVal&(VFAT2ContRegBitMasks::MSPOL))>>VFAT2ContRegBitShifts::MSPOL; }
          uint8_t getCalPolarity() {
            return (getCR0()&(VFAT2ContRegBitMasks::CALPOL))>>VFAT2ContRegBitShifts::CALPOL; }
          uint8_t getCalPolarity(uint8_t regVal) {
            return (regVal&(VFAT2ContRegBitMasks::CALPOL))>>VFAT2ContRegBitShifts::CALPOL; }
          uint8_t getCalibrationMode() {
            return (getCR0()&(VFAT2ContRegBitMasks::CALMODE))>>VFAT2ContRegBitShifts::CALMODE; }
          uint8_t getCalibrationMode(uint8_t regVal) {
            return (regVal&(VFAT2ContRegBitMasks::CALMODE))>>VFAT2ContRegBitShifts::CALMODE; }

          uint8_t getCR1()     {
            return (readVFATReg("ContReg1")); }
          //CR1:<7:0::ReHitCT<7:5>,lvdsMode<4>,probeMode<3>,dacMode<2:0> >
          uint8_t getDACMode() {
            return (getCR1()&(VFAT2ContRegBitMasks::DACMODE))>>VFAT2ContRegBitShifts::DACMODE; }
          uint8_t getDACMode(uint8_t regVal) {
            return (regVal&(VFAT2ContRegBitMasks::DACMODE))>>VFAT2ContRegBitShifts::DACMODE; }
          uint8_t getProbeMode() {
            return (getCR1()&(VFAT2ContRegBitMasks::PROBEMODE))>>VFAT2ContRegBitShifts::PROBEMODE; }
          uint8_t getProbeMode(uint8_t regVal) {
            return (regVal&(VFAT2ContRegBitMasks::PROBEMODE))>>VFAT2ContRegBitShifts::PROBEMODE; }
          uint8_t getLVDSMode() {
            return (getCR1()&(VFAT2ContRegBitMasks::LVDSMODE))>>VFAT2ContRegBitShifts::LVDSMODE; }
          uint8_t getLVDSMode(uint8_t regVal) {
            return (regVal&(VFAT2ContRegBitMasks::LVDSMODE))>>VFAT2ContRegBitShifts::LVDSMODE; }
          uint8_t getHitCountCycleTime() {
            return (getCR1()&(VFAT2ContRegBitMasks::REHITCT))>>VFAT2ContRegBitShifts::REHITCT; }
          uint8_t getHitCountCycleTime(uint8_t regVal) {
            return (regVal&(VFAT2ContRegBitMasks::REHITCT))>>VFAT2ContRegBitShifts::REHITCT; }

          uint8_t getCR2()     {
            return (readVFATReg("ContReg2")); }
          uint8_t getHitCountMode() {
            return (getCR2()&(VFAT2ContRegBitMasks::HITCOUNTMODE))>>VFAT2ContRegBitShifts::HITCOUNTMODE; }
          uint8_t getHitCountMode(uint8_t regVal) {
            return (regVal&(VFAT2ContRegBitMasks::HITCOUNTMODE))>>VFAT2ContRegBitShifts::HITCOUNTMODE; }
          uint8_t getMSPulseLength() {
            return (getCR2()&(VFAT2ContRegBitMasks::MSPULSELENGTH))>>VFAT2ContRegBitShifts::MSPULSELENGTH; }
          uint8_t getMSPulseLength(uint8_t regVal) {
            return (regVal&(VFAT2ContRegBitMasks::MSPULSELENGTH))>>VFAT2ContRegBitShifts::MSPULSELENGTH; }
          uint8_t getInputPadMode() {
            return (getCR2()&(VFAT2ContRegBitMasks::DIGINSEL))>>VFAT2ContRegBitShifts::DIGINSEL; }
          uint8_t getInputPadMode(uint8_t regVal) {
            return (regVal&(VFAT2ContRegBitMasks::DIGINSEL))>>VFAT2ContRegBitShifts::DIGINSEL; }

          uint8_t getCR3()     {
            return (readVFATReg("ContReg3")); } //maybe mask by 0x1F to eliminate the unused 3 MSBs?
          uint8_t getTrimDACRange() {
            return (getCR3()&(VFAT2ContRegBitMasks::TRIMDACRANGE))>>VFAT2ContRegBitShifts::TRIMDACRANGE; }
          uint8_t getTrimDACRange(uint8_t regVal) {
            return (regVal&(VFAT2ContRegBitMasks::TRIMDACRANGE))>>VFAT2ContRegBitShifts::TRIMDACRANGE; }
          uint8_t getBandgapPad() {
            return (getCR3()&(VFAT2ContRegBitMasks::PADBANDGAP))>>VFAT2ContRegBitShifts::PADBANDGAP; }
          uint8_t getBandgapPad(uint8_t regVal) {
            return (regVal&(VFAT2ContRegBitMasks::PADBANDGAP))>>VFAT2ContRegBitShifts::PADBANDGAP; }
          uint8_t getTestPatternMode() {
            return (getCR3()&(VFAT2ContRegBitMasks::DFTESTMODE))>>VFAT2ContRegBitShifts::DFTESTMODE; }
          uint8_t getTestPatternMode(uint8_t regVal) {
            return (regVal&(VFAT2ContRegBitMasks::DFTESTMODE))>>VFAT2ContRegBitShifts::DFTESTMODE; }

          //////////////////////////////
          /**
           * @brief  Get <chip setting>
           * @returns uint8_t value of the register
           */
          uint8_t getLatency()     { return readVFATReg("Latency");     }

          uint8_t getIPreampIn()   { return readVFATReg("IPreampIn");   }
          uint8_t getIPreampFeed() { return readVFATReg("IPreampFeed"); }
          uint8_t getIPreampOut()  { return readVFATReg("IPreampOut");  }
          uint8_t getIShaper()     { return readVFATReg("IShaper");     }
          uint8_t getIShaperFeed() { return readVFATReg("IShaperFeed"); }
          uint8_t getIComp()       { return readVFATReg("IComp");       }

          uint8_t getVCal()        { return readVFATReg("VCal");        }
          uint8_t getVThreshold1() { return readVFATReg("VThreshold1"); }
          uint8_t getVThreshold2() { return readVFATReg("VThreshold2"); }
          uint8_t getCalPhase()    { return readVFATReg("CalPhase");    }

          /*** may want to be able to get these values to a human readable number
               lookup done through a LUT
               float getIPreampIn()   { return readVFATReg("IPreampIn");   }
               float getIPreampFeed() { return readVFATReg("IPreampFeed"); }
               float getIPreampOut()  { return readVFATReg("IPreampOut");  }
               float getIShaper()     { return readVFATReg("IShaper");     }
               float getIShaperFeed() { return readVFATReg("IShaperFeed"); }
               float getIComp()       { return readVFATReg("IComp");       }

               float getVCal()        { return readVFATReg("VCal");        }
               float getVThreshold1() { return readVFATReg("VThreshold1"); }
               float getVThreshold2() { return readVFATReg("VThreshold2"); }
               float getCalPhase()    { return readVFATReg("CalPhase");    }
          **/
          //////////////////////////////

          //channel specific settings
          /**
           * @brief  Read channel settings
           * @param uint8_t which channel to read
           */
          void    readVFAT2Channel(uint8_t channel);
          //void    readVFAT2Channel(gem::hw::vfat::VFAT2ControlParams &params, uint8_t channel);

          /**
           * @brief  Read all VFAT channels
           */
          void    readVFAT2Channels();
          //void    readVFAT2Channels(gem::hw::vfat::VFAT2ControlParams &params);

          /**
           * @brief  Enable a cal pulse to specified channel
           * @param uint8_t which channel to modify
           * @param bool enable or not
           */
          void    enableCalPulseToChannel(uint8_t channel, bool on=true);

          /**
           * @brief Mask a specific channel from the trigger
           * @param uint8_t which channel to modify
           * @param bool mask or not
           */
          void    maskChannel(uint8_t channel, bool on=true);
          uint8_t getChannelSettings(uint8_t channel) {
            return readVFATReg(toolbox::toString("VFATChannels.ChanReg%d",(unsigned)channel)); }
          uint8_t getChannelTrimDAC(uint8_t channel);
          void    setChannelTrimDAC(uint8_t channel, uint8_t trimDAC);
          //void    setChannelTrimDAC(uint8_t channel, double trimDAC);

          uhal::HwInterface& getVFAT2HwInterface() {
            return gem::hw::GEMHwDevice::getGEMHwInterface();
          }

          gem::hw::vfat::VFAT2ControlParams getVFAT2Params() {
            return m_vfatParams; }

          void setActiveChannelWeb(uint8_t chan) {
            m_vfatParams.activeChannel = chan; }

        protected:

          TransactionErrors m_vfatErrors;
          gem::hw::vfat::VFAT2ControlParams m_vfatParams;

          //VFATMonitor *monVFAT_;

        private:
          uint8_t m_slot;

        };  // class HwVFAT2
    }  // namespace gem::hw::vfat
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_VFAT_HWVFAT2_H
