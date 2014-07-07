#ifndef _gemsupervisor_SimpleWeb_h
#define _gemsupervisor_SimpleWeb_h

#include "xdaq/Application.h"
#include "xdaq/WebApplication.h"

#include "xdata/String.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"

#include "uhal/uhal.hpp"

#include "xgi/framework/Method.h"
#include "xgi/framework/UIManager.h"
#include "cgicc/HTMLClasses.h"

typedef uhal::exception::exception uhalException;

namespace gemsupervisor {
  
  class GemSupervisorWeb: public xdaq::Application, public xgi::framework::UIManager
  {
    public:
      XDAQ_INSTANTIATOR();
      
      GemSupervisorWeb(xdaq::ApplicationStub * s)
	throw (xdaq::exception::Exception);
      
      void Default(xgi::Input * in, xgi::Output * out )
	throw (xgi::exception::Exception);
      void Views(xgi::Input * in, xgi::Output * out )
	throw (xgi::exception::Exception);
      void setParameter(xgi::Input * in, xgi::Output * out)
	throw (xgi::exception::Exception);
      void Read(xgi::Input * in, xgi::Output * out )
	throw (xgi::exception::Exception);
      void Write(xgi::Input * in, xgi::Output * out )
	throw (xgi::exception::Exception);

      void getTestReg();
      void setTestReg(uint32_t);
      
    protected:
      xdata::UnsignedLong myParameter_;
      xdata::String myAction_;

      xdata::UnsignedInteger32 testReg_;
      xdata::UnsignedInteger32 boardID_;
      xdata::UnsignedInteger32 systemID_;
      xdata::UnsignedInteger32 systemFirmwareID_;
      xdata::UnsignedInteger32 userFirmwareID_;

      xdata::UnsignedInteger32 xreg_ctrl;
      xdata::UnsignedInteger32 xreg_ctrl2;
      xdata::UnsignedInteger32 xreg_status;
      xdata::UnsignedInteger32 xreg_status2;
      xdata::UnsignedInteger32 xreg_ctrl_sram;
      xdata::UnsignedInteger32 xreg_status_sram;
      xdata::UnsignedInteger32 xreg_spi_txdata;
      xdata::UnsignedInteger32 xreg_spi_command;
      xdata::UnsignedInteger32 xreg_spi_rxdata;
      xdata::UnsignedInteger32 xreg_i2c_settings;
      xdata::UnsignedInteger32 xreg_i2c_command;
      xdata::UnsignedInteger32 xreg_i2c_reply;
      xdata::UnsignedInteger32 xreg_sfp_phase_mon_ctrl;
      xdata::UnsignedInteger32 xreg_sfp_phase_mon_stats;
      xdata::UnsignedInteger32 xreg_fmc_phase_mon_ctrl;
      xdata::UnsignedInteger32 xreg_fmc_phase_mon_stats;

      xdata::UnsignedInteger32 xreg_mac_info1;
      xdata::UnsignedInteger32 xreg_mac_info2;
      xdata::UnsignedInteger32 xreg_ip_info;

      xdata::UnsignedInteger32 xreg_sram1;
      xdata::UnsignedInteger32 xreg_sram2;
      xdata::UnsignedInteger32 xreg_icap;

      std::vector<xdata::UnsignedInteger32 > xreg_users;

    private:
      void initializeConnection();
      std::string registerToChar(xdata::UnsignedInteger32);
      std::string formatSystemID(xdata::UnsignedInteger32);
      std::string formatBoardID(xdata::UnsignedInteger32);
      std::string formatFW(xdata::UnsignedInteger32, int type);
      std::string getIPAddress();

      uhal::ConnectionManager *manager;
      uhal::HwInterface *hw;
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

  };
}
#endif
