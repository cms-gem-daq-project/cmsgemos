#ifndef gem_hw_optohybrid_HwOPTOHYBRID_h
#define gem_hw_optohybrid_HwOPTOHYBRID_h

#include "xdaq/Application.h"

#include "xdata/String.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"
#include "xdata/ActionListener.h"

#include "gem/hw/GEMHwDevice.h"

#include "gem/hw/optohybrid/OptoHybridMonitor.h"

#include "uhal/uhal.hpp"

#define MAX_NVFATS 6

typedef uhal::exception::exception uhalException;

namespace uhal {
  class HwInterface;
}

namespace xdaq {
  class Application;;
}

namespace gem {
  namespace hw {
    namespace optohybrid {
      class OptoHybridMonitor;
      
      class HwOptoHybrid: public gem::hw::GEMHwDevice
	{
	public:
	  //XDAQ_INSTANTIATOR();
	
	  HwOptoHybrid(xdaq::Application * optohybridApp);
	  //throw (xdaq::exception::Exception);
	
	  //virtual void connectDevice();
	  //virtual void releaseDevice();
	  //virtual void initDevice();
	  //virtual void enableDevice();
	  virtual void configureDevice();
	  virtual void configureDevice(std::string const& xmlSettings);
	  //virtual void configureDevice(std::string const& dbConnectionString);
	  //virtual void disableDevice();
	  //virtual void pauseDevice();
	  //virtual void startDevice();
	  //virtual void stopDevice();
	  //virtual void resumeDevice();
	  //virtual void haltDevice();

	  bool isHwOptoHybridConnected();

	  /** Read the board ID registers
	   * @returns the OptoHybrid board ID 
	   **/
	  std::string getBoardID()   const;

	  /** Read the firmware register
	   * @returns a string corresponding to the build date
	   **/
	  std::string getFirmware()  const;
	  
	  /** Read the link status registers, store the information in a struct
	   * @param uint8_t link is the number of the link to query
	   * @retval _status a struct containing the status bits of the optical link
	   **/
	  OHLinkStatus LinkStatus(uint8_t link);

	  /** Reset the link status registers
	   * @param uint8_t link is the number of the link to query
	   * @param uint8_t resets control which bits to reset
	   * 0x00
	   * bit 1 - ErrCnt      0x01
	   * bit 2 - VFATI2CRec  0x02
	   * bit 3 - VFATI2CSnt  0x04
	   * bit 4 - RegisterRec 0x08
	   * bit 5 - RegisterSnt 0x10
	   **/
	  void LinkReset(uint8_t link, uint8_t resets) const {
	    if (resets&0x01)
	      writeReg("OPTICAL_LINKS.LINKX.Resets.LinkErr",0x1);
	    if (resets&0x02)
	      writeReg("OPTICAL_LINKS.LINKX.Resets.RecI2CReqests",0x1);
	    if (resets&0x04)
	      writeReg("OPTICAL_LINKS.LINKX.Resets.SntI2CReqests",0x1);
	    if (resets&0x08)
	      writeReg("OPTICAL_LINKS.LINKX.Resets.RecRegReqests",0x1);
	    if (resets&0x10)
	      writeReg("OPTICAL_LINKS.LINKX.Resets.SntRegReqests",0x1);
	  };

	  /** Read the trigger data
	   * @retval uint32_t returns 32 bits 6 bits for s-bits and 26 for bunch countrr
	   **/
	  uint32_t readTriggerData();

	  
	  ///Clocking
	  /** Setup the VFAT clock 
	   * @param bool source true uses the external clock, false uses the onboard clock
	   * @param bool fallback uses the external clock, false uses the onboard clock
	   **/
	  void SetVFATClock(bool source, bool fallback) const {
	    writeReg("CLOCKING.VFAT.SOURCE"  ,(uint32_t)source  );
	    writeReg("CLOCKING.VFAT.FALLBACK",(uint32_t)fallback);
	  };

	  /** VFAT clock status
	   * @param bool source true uses the external clock, false uses the onboard clock
	   * @param bool fallback uses the external clock, false uses the onboard clock
	   **/
	  std::pair<bool,bool> StatusVFATClock() const {
	    uint32_t src = readReg("CLOCKING.VFAT.SOURCE");
	    uint32_t flb = readReg("CLOCKING.VFAT.FALLBACK");
	    //maybe do a check to ensure that the value has been read properly?
	    return std::make_pair(src,flb);
	  };

	  /** Setup the CDCE clock 
	   * @param bool source true uses the external clock, false uses the onboard clock
	   * @param bool fallback uses the external clock, false uses the onboard clock
	   **/
	  void SetCDCEClock(bool source, bool fallback) const {
	    writeReg("CLOCKING.CDCE.SOURCE"  ,(uint32_t)source  );
	    writeReg("CLOCKING.CDCE.FALLBACK",(uint32_t)fallback);
	  };

	  /** CDCE clock status
	   * @param bool source true uses the external clock, false uses the onboard clock
	   * @param bool fallback uses the external clock, false uses the onboard clock
	   **/
	  std::pair<bool,bool> StatusCDCEClock() const {
	    uint32_t src = readReg("CLOCKING.CDCE.SOURCE");
	    uint32_t flb = readReg("CLOCKING.CDCE.FALLBACK");
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
	   * @param uint8_t mode 0 from GLIB, 1 from external, 2 from both
	   **/
	  void setTrigSource(uint8_t mode) const {
	    switch (mode) {
	    case(0):
	      writeReg("TRIGGER.SOURCE",mode);
	      return;
	    case(1):
	      writeReg("TRIGGER.SOURCE",mode);
	      return;
	    case(2):
	      writeReg("TRIGGER.SOURCE",mode);
	      return;
	    default:
	      writeReg("TRIGGER.SOURCE",0x2);
	      return;
	    }
	  };

	  /** Read the Trigger source
	   * @retval uint8_t 0 from GLIB, 1 from external, 2 from both
	   **/
	  uint8_t getTrigSource() const { return readReg("TRIGGER.SOURCE"); };


	  /** Set the S-bit source
	   * @param uint8_t chip
	   **/
	  void setSBitSource(uint8_t mode) const { writeReg("TRIGGER.TDC.SBits",mode); };

	  /** Read the S-bit source
	   * @retval uint8_t which VFAT chip is sending the S-bits
	   **/
	  uint8_t getSBitSource() const { return readReg("TRIGGER.TDC_SBits"); };


	  /* Generate and send specific T1 commands on the OptoHybrid */
	  /** Send an internal L1A
	   * @param uint64_t ntrigs, how many L1As to send
	   **/
	  void SendL1A(uint64_t ntrigs) const {
	    for (uint64_t i = 0; i < ntrigs; ++i) 
	      writeReg("FAST_COM.Send.L1A",0x1);
	  };

	  /** Send an internal CalPulse
	   * @param uint64_t npulse, how many CalPulses to send
	   **/
	  void SendCalPulse(uint64_t npulse) const {
	    for (uint64_t i = 0; i < npulse; ++i) 
	      writeReg("FAST_COM.Send.CalPulse",0x1);
	  };

	  /** Send an internal L1A and CalPulse
	   * @param uint64_t npulse, how many pairs to send
	   * @param uint32_t delay, how long between L1A and CalPulse
	   **/
	  void SendL1ACal(uint64_t npulse, uint32_t delay) const {
	    for (uint64_t i = 0; i < ntrigs; ++i) 
	      writeReg("FAST_COM.Send.L1ACalPulse",delay);
	  };

	  /** Send an internal Resync
	   * 
	   **/
	  void SendResync() const { writeReg("FAST_COM.Send.Resync",0x1); };


	  /** Send an internal BC0
	   * 
	   **/
	  void SendBC0() const { writeReg("FAST_COM.Send.BC0",0x1); };

	  ///Counters
	  /** Get the recorded number of L1A signals
	   * @param mode specifies which L1A counter to read
	   * 0 external
	   * 1 internal
	   * 2 delayed (sent along with a CalPulse)
	   * 3 total
	   **/
	  uint32_t GetL1ACount(uint8_t mode) const {
	    switch(mode) {
	    case 0:
	      return readReg("COUNTERS.L1A.External");
	    case 1:
	      return readReg("COUNTERS.L1A.Internal");
	    case 2:
	      return readReg("COUNTERS.L1A.Delayed");
	    case 3:
	      return readReg("COUNTERS.L1A.Total");
	    default:
	      return readReg("COUNTERS.L1A.Total");
	    }
	  };
	  
	  /** Get the recorded number of CalPulse signals
	   * @param mode specifies which CalPulse counter to read
	   * 0 internal
	   * 1 delayed (sent along with a L1A)
	   * 2 total
	   **/
	  uint32_t GetCalPulseCount(uint8_t mode) const {
	    switch(mode) {
	    case 0:
	      return readReg("COUNTERS.CalPulse.Internal");
	    case 1:
	      return readReg("COUNTERS.CalPulse.Delayed");
	    case 2:
	      return readReg("COUNTERS.CalPulse.Total");
	    default:
	      return readReg("COUNTERS.CalPulse.Total");
	    }
	  };
	  
	  /** Get the recorded number of Resync signals
	   **/
	  uint32_t GetResyncCount() const { return readReg("COUNTERS.Resync"); };

	  /** Get the recorded number of BC0 signals
	   **/
	  uint32_t GetBC0Count() const { return readReg("COUNTERS.BC0"); };

	  /** Get the recorded number of BXCount signals
	   **/
	  uint32_t GetBXCountCount() const { return readReg("COUNTERS.BXCount"); };
	  
	  ///Resets
	  /** Reset recorded number of L1A signals
	   * @param mode specifies which L1A counter to reset
	   * 0 external
	   * 1 internal
	   * 2 delayed (sent along with a CalPulse)
	   * 3 total
	   **/
	  void ResetL1ACount(uint8_t mode) const {
	    switch(mode) {
	    case 0:
	      return writeReg("RESETS.L1A.External", 0x1);
	    case 1:
	      return writeReg("RESETS.L1A.Internal", 0x1);
	    case 2:
	      return writeReg("RESETS.L1A.Delayed", 0x1);
	    case 3:
	      return writeReg("RESETS.L1A.Total", 0x1);
	    default:
	      return writeReg("RESETS.L1A.Total", 0x1);
	    }
	  };
	  
	  /** Get the recorded number of CalPulse signals
	   * @param mode specifies which CalPulse counter to read
	   * 0 internal
	   * 1 delayed (sent along with a L1A)
	   * 2 total
	   **/
	  void ResetCalPulseCount(uint8_t mode) const {
	    switch(mode) {
	    case 0:
	      return writeReg("RESETS.CalPulse.Internal", 0x1);
	    case 1:
	      return writeReg("RESETS.CalPulse.Delayed", 0x1);
	    case 2:
	      return writeReg("RESETS.CalPulse.Total", 0x1);
	    default:
	      return writeReg("RESETS.CalPulse.Total", 0x1);
	    }
	  };
	  
	  /** Get the recorded number of Resync signals
	   **/
	  void ResetResyncCount() const { return writeReg("RESETS.Resync", 0x1); };

	  /** Get the recorded number of BC0 signals
	   **/
	  void ResetBC0Count() const { return writeReg("RESETS.BC0", 0x1); };

	  /** Get the recorded number of BXCount signals
	   **/
	  void ResetBXCountCount() const { return writeReg("RESETS.BXCount", 0x1); };
	  
	protected:
	  uhal::ConnectionManager *manageOptoHybridConnection;
	  log4cplus::Logger logOptoHybrid_;
	  uhal::HwInterface *hwOptoHybrid_;

	  uhal::HwInterface& getOptoHybridHwDevice() const;

	  OptoHybridMonitor *monOptoHybrid_;

	  xdata::UnsignedLong myParameter_;
	  xdata::String myAction_;
	
	  xdata::UnsignedInteger32 testReg_;
	  xdata::UnsignedInteger32 boardID_;
	  xdata::UnsignedInteger32 systemID_;
	  xdata::UnsignedInteger32 systemFirmwareID_;
	
	private:
	  std::string registerToChar(uint32_t value);
	
	  uhal::ValWord< uint32_t > r_test;
	  uhal::ValWord< uint32_t > r_sysid;
	  uhal::ValWord< uint32_t > r_boardid;
	  uhal::ValWord< uint32_t > r_fwid;
	
	}; //end class HwOptoHybrid
    } //end namespace gem::hw::glib
  } //end namespace gem::hw
} //end namespace gem
#endif
