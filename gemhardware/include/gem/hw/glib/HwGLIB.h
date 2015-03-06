#ifndef gem_hw_glib_HwGLIB_h
#define gem_hw_glib_HwGLIB_h

#include "xdaq/Application.h"

#include "xdata/String.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"
#include "xdata/ActionListener.h"

#include "gem/hw/GEMHwDevice.h"

#include "gem/hw/glib/GLIBMonitor.h"

#include "uhal/uhal.hpp"

typedef uhal::exception::exception uhalException;

namespace uhal {
  class HwInterface;
}

namespace xdaq {
  class Application;;
}

namespace gem {
  namespace hw {
    namespace glib {
      class GLIBMonitor;
    
      class HwGLIB: public gem::hw::GEMHwDevice
	{
	public:
	  XDAQ_INSTANTIATOR();
	
	  HwGLIB(xdaq::ApplicationStub * s)
	    throw (xdaq::exception::Exception);
	
	  virtual void connectDevice();
	  virtual void releaseDevice();
	  virtual void initDevice();
	  virtual void enableDevice();
	  virtual void disableDevice();
	  virtual void pauseDevice();
	  virtual void startDevice();
	  virtual void stopDevice();
	  virtual void resumeDevice();
	  virtual void haltDevice();

	  //bool isHwGLIBConnected();

	  std::string getBoardID()   const;
	  std::string getSystemID()  const;
	  std::string getIPAddress() const;
	  std::string getUserFirmware() const;
	  std::string getFirmwareVer( std::string const& fwRegPrefix="sysregs") const;
	  std::string getFirmwareDate(std::string const& fwRegPrefix="sysregs") const;

	protected:
	  uhal::ConnectionManager *manageGLIBConnection;
	  log4cplus::Logger logGLIB_;
	  uhal::HwInterface *hwGLIB_;

	  uhal::HwInterface& getGLIBHwDevice() const;

	  GLIBMonitor *monGLIB_;

	  xdata::UnsignedLong myParameter_;
	  xdata::String myAction_;
	
	  xdata::UnsignedInteger32 testReg_;
	  xdata::UnsignedInteger32 boardID_;
	  xdata::UnsignedInteger32 systemID_;
	  xdata::UnsignedInteger32 systemFirmwareID_;
	  xdata::UnsignedInteger32 userFirmwareID_;
	
	  xdata::UnsignedInteger32 xreg_ctrl_;
	  xdata::UnsignedInteger32 xreg_ctrl2_;
	  xdata::UnsignedInteger32 xreg_status_;
	  xdata::UnsignedInteger32 xreg_status2_;
	  xdata::UnsignedInteger32 xreg_ctrl_sram_;
	  xdata::UnsignedInteger32 xreg_status_sram_;
	  xdata::UnsignedInteger32 xreg_spi_txdata_;
	  xdata::UnsignedInteger32 xreg_spi_command_;
	  xdata::UnsignedInteger32 xreg_spi_rxdata_;
	  xdata::UnsignedInteger32 xreg_i2c_settings_;
	  xdata::UnsignedInteger32 xreg_i2c_command_;
	  xdata::UnsignedInteger32 xreg_i2c_reply_;
	  xdata::UnsignedInteger32 xreg_sfp_phase_mon_ctrl_;
	  xdata::UnsignedInteger32 xreg_sfp_phase_mon_stats_;
	  xdata::UnsignedInteger32 xreg_fmc_phase_mon_ctrl_;
	  xdata::UnsignedInteger32 xreg_fmc_phase_mon_stats_;
	
	  xdata::UnsignedInteger32 xreg_mac_info1_;
	  xdata::UnsignedInteger32 xreg_mac_info2_;
	  xdata::UnsignedInteger32 xreg_ip_info_;
	
	  xdata::UnsignedInteger32 xreg_sram1_;
	  xdata::UnsignedInteger32 xreg_sram2_;
	  xdata::UnsignedInteger32 xreg_icap_;
	
	  std::vector<xdata::UnsignedInteger32 > xreg_users_;
	
	private:
	  std::string registerToChar(uint32_t value);
	
	  uhal::ValWord< uint32_t > r_test;
	  uhal::ValWord< uint32_t > r_sysid;
	  uhal::ValWord< uint32_t > r_boardid;
	  uhal::ValWord< uint32_t > r_fwid;
	  uhal::ValWord< uint32_t > r_ctrl;
	  uhal::ValWord< uint32_t > r_ctrl2;
	  uhal::ValWord< uint32_t > r_status;
	  uhal::ValWord< uint32_t > r_status2;
	  uhal::ValWord< uint32_t > r_ctrl_sram;
	  uhal::ValWord< uint32_t > r_status_sram;
	  uhal::ValWord< uint32_t > r_spi_txdata;
	  uhal::ValWord< uint32_t > r_spi_command;
	  uhal::ValWord< uint32_t > r_spi_rxdata;
	  uhal::ValWord< uint32_t > r_i2c_settings;
	  uhal::ValWord< uint32_t > r_i2c_command;
	  uhal::ValWord< uint32_t > r_i2c_reply;
	  uhal::ValWord< uint32_t > r_sfp_phase_mon_ctrl;
	  uhal::ValWord< uint32_t > r_sfp_phase_mon_stats;
	  uhal::ValWord< uint32_t > r_fmc_phase_mon_ctrl;
	  uhal::ValWord< uint32_t > r_fmc_phase_mon_stats;
	
	  uhal::ValWord< uint32_t > r_mac_info1;
	  uhal::ValWord< uint32_t > r_mac_info2;
	  uhal::ValWord< uint32_t > r_ip_info;
	
	  uhal::ValWord< uint32_t > r_sram1;
	  uhal::ValWord< uint32_t > r_sram2;
	  uhal::ValWord< uint32_t > r_icap;                 
	
	  std::vector<uhal::ValWord< uint32_t > > r_users;
	
	}; //end class HwGLIB
    } //end namespace gem::hw::glib
  } //end namespace gem::hw
} //end namespace gem
#endif
