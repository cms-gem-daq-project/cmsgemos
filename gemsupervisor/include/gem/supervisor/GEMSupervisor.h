#ifndef gem_supervisor_GEMSupervisor_h
#define gem_supervisor_GEMSupervisor_h

#include "xdaq/Application.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"

#include "uhal/uhal.hpp"


namespace gem {
  namespace supervisor {
    
    class GEMSupervisor : public GEMFSMApplication
    {
    public:
      GEMSupervisor();
      ~GEMSupervisor();
      
      //      void getSystemFW()
      //      void getUserFW()
      //      void getBoardID()
      //      void getSystemID()
      //      void getChipID()
      void main();
      void getTestReg();
      //uhal::HwInterface &hw, uhal::ValWord< uint32_t> &mem)
      void setTestReg(uint32_t);
      //uhal::HwInterface &hw, uhal::ValWord< uint32_t> &mem)
      
    protected:
      xdata::UnsignedLong myParameter_;
      
      xdata::UnsignedInteger32 testReg_;
      xdata::UnsignedInteger32 boardID_;
      xdata::UnsignedInteger32 systemID_;
      xdata::UnsignedInteger32 systemFirmwareID_;
      xdata::UnsignedInteger32 userFirmwareID_;
      
    private:
      void initializeConnection();
      
      uhal::ConnectionManager *manager;
      //uhal::HwInterface *hw;
      uhal::ValWord< uint32_t > r_test;
      uhal::ValWord< uint32_t > r_sysid;
      uhal::ValWord< uint32_t > r_boardid;
      uhal::ValWord< uint32_t > r_fwid;
      //uhal::ValWord< uint32_t > r_test;
      
    };
  } //end namespace supervisor
} //end namespace gem
#endif
