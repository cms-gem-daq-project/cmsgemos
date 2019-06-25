/**
 * class: GLIBMonitor
 * description: Monitor application for GLIB cards
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date:
 */

#include "gem/hw/glib/HwGLIB.h"

#include "gem/hw/glib/GLIBMonitor.h"
#include "gem/hw/glib/GLIBManager.h"
#include "gem/base/GEMApplication.h"
#include "gem/base/GEMFSMApplication.h"

typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

gem::hw::glib::GLIBMonitor::GLIBMonitor(std::shared_ptr<HwGLIB> glib, GLIBManager* glibManager, int const& index) :
  GEMMonitor(glibManager->getApplicationLogger(), static_cast<xdaq::Application*>(glibManager), index),
  p_glib(glib)
{
  // application info space is added in the base class constructor
  // addInfoSpace("Application", glibManager->getApplicationInfoSpace());
  // addInfoSpace("GLIB", p_glib->getHwInfoSpace());

  // if (!p_glib->getHwInfoSpace()->hasItem(monname))
  //   p_glib->getHwInfoSpace()->fireItemAvailable(monname, &monvar);

  updateMonitorables();
}


void gem::hw::glib::GLIBMonitor::setupHwMonitoring()
{
  // create the values to be monitored in the info space
  addMonitorableSet("SYSTEM", "HWMonitoring");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("BOARD_ID", "GLIB_SYSTEM.SYSTEM.BOARD_ID"),
                 GEMUpdateType::NOUPDATE, "id");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("SYSTEM_ID", "GLIB_SYSTEM.SYSTEM.SYSTEM_ID"),
                 GEMUpdateType::NOUPDATE, "id");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("FIRMWARE_VERSION", "GLIB_SYSTEM.SYSTEM.FIRMWARE.ID"),
                 GEMUpdateType::NOUPDATE, "fwverglib");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("FIRMWARE_DATE", "GLIB_SYSTEM.SYSTEM.FIRMWARE.DATE"),
                 GEMUpdateType::NOUPDATE, "dateglib");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("AMC_FIRMWARE_VERSION", "GEM_SYSTEM.RELEASE"),
                 GEMUpdateType::NOUPDATE, "fwverglib");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("AMC_FIRMWARE_DATE", "GEM_SYSTEM.RELEASE.DATE"),
                 GEMUpdateType::NOUPDATE, "dateoh");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("IP_ADDRESS", "GLIB_SYSTEM.SYSTEM.IP_INFO"),
                 GEMUpdateType::NOUPDATE, "ip");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("MAC_ADDRESS", "GLIB_SYSTEM.SYSTEM.MAC"),
                 GEMUpdateType::NOUPDATE, "mac");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("SFP1_STATUS", "GLIB_SYSTEM.SYSTEM.STATUS.SFP1.STATUS"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("SFP2_STATUS", "GLIB_SYSTEM.SYSTEM.STATUS.SFP2.STATUS"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("SFP3_STATUS", "GLIB_SYSTEM.SYSTEM.STATUS.SFP3.STATUS"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("SFP4_STATUS", "GLIB_SYSTEM.SYSTEM.STATUS.SFP4.STATUS"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("FMC1_STATUS", "GLIB_SYSTEM.SYSTEM.STATUS.FMC1_PRESENT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("FMC2_STATUS", "GLIB_SYSTEM.SYSTEM.STATUS.FMC2_PRESENT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("FPGA_RESET", "GLIB_SYSTEM.SYSTEM.STATUS.FPGA_RESET"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("GBE_INT",  "GLIB_SYSTEM.SYSTEM.STATUS.GBE_INT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("V6_CPLD",  "GLIB_SYSTEM.SYSTEM.STATUS.V6_CPLD"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("CPLD_LOCK", "GLIB_SYSTEM.SYSTEM.STATUS.CDCE_LOCK"),
                 GEMUpdateType::HW32, "hex");

  addMonitorableSet("COUNTERS", "HWMonitoring");
  addMonitorable("COUNTERS", "HWMonitoring",
                 std::make_pair("L1A", "TTC.CMD_COUNTERS.L1A"),
                 GEMUpdateType::HW32, "dec");
  addMonitorable("COUNTERS", "HWMonitoring",
                 std::make_pair("BC0", "TTC.CMD_COUNTERS.BC0"),
                 GEMUpdateType::HW32, "dec");
  addMonitorable("COUNTERS", "HWMonitoring",
                 std::make_pair("EC0", "TTC.CMD_COUNTERS.EC0"),
                 GEMUpdateType::HW32, "dec");
  addMonitorable("COUNTERS", "HWMonitoring",
                 std::make_pair("RESYNC", "TTC.CMD_COUNTERS.RESYNC"),
                 GEMUpdateType::HW32, "dec");
  addMonitorable("COUNTERS", "HWMonitoring",
                 std::make_pair("OC0", "TTC.CMD_COUNTERS.OC0"),
                 GEMUpdateType::HW32, "dec");
  addMonitorable("COUNTERS", "HWMonitoring",
                 std::make_pair("HARD_RESET", "TTC.CMD_COUNTERS.HARD_RESET"),
                 GEMUpdateType::HW32, "dec");
  addMonitorable("COUNTERS", "HWMonitoring",
                 std::make_pair("CalPulse", "TTC.CMD_COUNTERS.CALPULSE"),
                 GEMUpdateType::HW32, "dec");
  addMonitorable("COUNTERS", "HWMonitoring",
                 std::make_pair("START", "TTC.CMD_COUNTERS.START"),
                 GEMUpdateType::HW32, "dec");
  addMonitorable("COUNTERS", "HWMonitoring",
                 std::make_pair("STOP", "TTC.CMD_COUNTERS.STOP"),
                 GEMUpdateType::HW32, "dec");
  addMonitorable("COUNTERS", "HWMonitoring",
                 std::make_pair("TEST_SYNC", "TTC.CMD_COUNTERS.TEST_SYNC"),
                 GEMUpdateType::HW32, "dec");

  addMonitorableSet("DAQ Status", "HWMonitoring");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("CONTROL", "DAQ.CONTROL"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("STATUS", "DAQ.STATUS"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("NOTINTABLE_ERR", "DAQ.EXT_STATUS.NOTINTABLE_ERR"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("DISPER_ERR", "DAQ.EXT_STATUS.DISPER_ERR"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("EVT_SENT", "DAQ.EXT_STATUS.EVT_SENT"),
                 GEMUpdateType::HW32, "dec");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("L1AID", "DAQ.EXT_STATUS.L1AID"),
                 GEMUpdateType::HW32, "dec");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("MAX_DAV_TIMER", "DAQ.EXT_STATUS.MAX_DAV_TIMER"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("LAST_DAV_TIMER", "DAQ.EXT_STATUS.LAST_DAV_TIMER"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("L1A_FIFO_DATA_CNT", "DAQ.EXT_STATUS.L1A_FIFO_DATA_CNT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("DAQ_FIFO_DATA_CNT", "DAQ.EXT_STATUS.DAQ_FIFO_DATA_CNT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("L1A_FIFO_NEAR_FULL_CNT", "DAQ.EXT_STATUS.L1A_FIFO_NEAR_FULL_CNT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("DAQ_FIFO_NEAR_FULL_CNT", "DAQ.EXT_STATUS.DAQ_FIFO_NEAR_FULL_CNT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("DAQ_ALMOST_FULL_CNT", "DAQ.EXT_STATUS.DAQ_ALMOST_FULL_CNT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("DAQ_ALMOST_FULL_CNT", "DAQ.EXT_STATUS.DAQ_ALMOST_FULL_CNT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("TTS_WARN_CNT", "DAQ.EXT_STATUS.TTS_WARN_CNT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("DAQ_WORD_RATE", "DAQ.EXT_STATUS.DAQ_WORD_RATE"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("RUN_TYPE", "DAQ.EXT_CONTROL.RUN_TYPE"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ Status", "HWMonitoring",
                 std::make_pair("RUN_PARAMS", "DAQ.EXT_CONTROL.RUN_PARAMS"),
                 GEMUpdateType::HW32, "hex");

  addMonitorableSet("TTC", "HWMonitoring");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("TTC_SPY", "TTC.TTC_SPY_BUFFER"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("MMCM_LOCKED", "TTC.STATUS.CLK.MMCM_LOCKED"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("BC0_LOCKED", "TTC.STATUS.BC0.LOCKED"),
                 GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("PHASE_LOCKED", "TTC.STATUS.CLK.PHASE_LOCKED"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("SYNC_DONE", "TTC.STATUS.CLK.SYNC_DONE"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("L1A_RATE", "TTC.L1A_RATE"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("MMCM_UNLOCK_CNT", "TTC.STATUS.CLK.MMCM_UNLOCK_CNT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("BC0_UNLOCK_CNT", "TTC.STATUS.BC0.UNLOCK_CNT"),
                 GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("PHASE_UNLOCK_CNT", "TTC.STATUS.CLK.PHASE_UNLOCK_CNT"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("PHASE_UNLOCK_TIME", "TTC.STATUS.CLK.PHASE_UNLOCK_TIME"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("SYNC_DONE_TIME", "TTC.STATUS.CLK.SYNC_DONE_TIME"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("BC0_OVERFLOW_CNT", "TTC.STATUS.BC0.OVERFLOW_CNT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("BC0_UNDERFLOW_CNT", "TTC.STATUS.BC0.UNDERFLOW_CNT"),
                 GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("PA_PLL_LOCK_WINDOW", "TTC.STATUS.CLK.PA_PLL_LOCK_WINDOW"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("PA_PHASE_SHIFT_CNT", "TTC.STATUS.CLK.PA_PHASE_SHIFT_CNT"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("PA_PLL_LOCK_CLOCKS", "TTC.STATUS.CLK.PA_PLL_LOCK_CLOCKS"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("PA_FSM_STATE", "TTC.STATUS.CLK.PA_FSM_STATE"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("TTC_PM_PHASE", "TTC.STATUS.CLK.TTC_PM_PHASE"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("TTC_PM_PHASE_MEAN", "TTC.STATUS.CLK.TTC_PM_PHASE_MEAN"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("TTC_PM_PHASE_MAX", "TTC.STATUS.CLK.TTC_PM_PHASE_MAX"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("TTC_PM_PHASE_MIN", "TTC.STATUS.CLK.TTC_PM_PHASE_MIN"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("TTC_PM_PHASE_JUMP_CNT", "TTC.STATUS.CLK.TTC_PM_PHASE_JUMP_CNT"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("TTC_PM_PHASE_JUMP_SIZE", "TTC.STATUS.CLK.TTC_PM_PHASE_JUMP_SIZE"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("TTC_PM_PHASE_JUMP_TIME", "TTC.STATUS.CLK.TTC_PM_PHASE_JUMP_TIME"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("GTH_PM_PHASE", "TTC.STATUS.CLK.GTH_PM_PHASE"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("GTH_PM_PHASE_MEAN", "TTC.STATUS.CLK.GTH_PM_PHASE_MEAN"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("GTH_PM_PHASE_MAX", "TTC.STATUS.CLK.GTH_PM_PHASE_MAX"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("GTH_PM_PHASE_MIN", "TTC.STATUS.CLK.GTH_PM_PHASE_MIN"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("GTH_PM_PHASE_JUMP_CNT", "TTC.STATUS.CLK.GTH_PM_PHASE_JUMP_CNT"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("GTH_PM_PHASE_JUMP_SIZE", "TTC.STATUS.CLK.GTH_PM_PHASE_JUMP_SIZE"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");
  // FIXME NOT YET PRESENT // addMonitorable("TTC", "HWMonitoring",
  // FIXME NOT YET PRESENT //                std::make_pair("GTH_PM_PHASE_JUMP_TIME", "TTC.STATUS.CLK.GTH_PM_PHASE_JUMP_TIME"),
  // FIXME NOT YET PRESENT //                GEMUpdateType::HW32, "hex");

  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("BC0_CMD", "TTC.CONFIG.CMD_BC0"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("EC0_CMD", "TTC.CONFIG.CMD_EC0"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("RESYNC_CMD", "TTC.CONFIG.CMD_RESYNC"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("OC0_CMD", "TTC.CONFIG.CMD_OC0"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("HARD_RESET_CMD", "TTC.CONFIG.CMD_HARD_RESET"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("CALPULSE_CMD", "TTC.CONFIG.CMD_CALPULSE"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("START_CMD", "TTC.CONFIG.CMD_START"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("STOP_CMD", "TTC.CONFIG.CMD_STOP"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("TEST_SYNC_CMD", "TTC.CONFIG.CMD_TEST_SYNC"),
                 GEMUpdateType::HW32, "hex");

  addMonitorableSet("Trigger Status", "HWMonitoring");
  for (uint8_t oh = 0; oh < p_glib->getSupportedOptoHybrids(); ++oh) {
    std::stringstream ohname;
    ohname << "OH" <<  (int)oh;
    addMonitorableSet("DAQ Status", "HWMonitoring");
    addMonitorable("DAQ Status", "HWMonitoring",
                   std::make_pair(ohname.str()+"_STATUS", "DAQ."+ohname.str()+".STATUS"),
                   GEMUpdateType::HW32, "hex");
    addMonitorable("DAQ Status", "HWMonitoring",
                   std::make_pair(ohname.str()+"_CORRUPT_VFAT_BLK_CNT", "DAQ."+ohname.str()+".COUNTERS.CORRUPT_VFAT_BLK_CNT"),
                   GEMUpdateType::HW32, "dec");
    addMonitorable("DAQ Status", "HWMonitoring",
                   std::make_pair(ohname.str()+"_EVN", "DAQ."+ohname.str()+".COUNTERS.EVN"),
                   GEMUpdateType::HW32, "dec");
    addMonitorable("DAQ Status", "HWMonitoring",
                   std::make_pair(ohname.str()+"_EOE_TIMEOUT", "DAQ."+ohname.str()+".CONTROL.EOE_TIMEOUT"),
                   GEMUpdateType::HW32, "hex");
    addMonitorable("DAQ Status", "HWMonitoring",
                   std::make_pair(ohname.str()+"_MAX_EOE_TIMER", "DAQ."+ohname.str()+".COUNTERS.MAX_EOE_TIMER"),
                   GEMUpdateType::HW32, "hex");
    addMonitorable("DAQ Status", "HWMonitoring",
                   std::make_pair(ohname.str()+"_LAST_EOE_TIMER", "DAQ."+ohname.str()+".COUNTERS.LAST_EOE_TIMER"),
                   GEMUpdateType::HW32, "hex");

    addMonitorable("Trigger Status", "HWMonitoring",
                   std::make_pair(ohname.str()+"_TRIGGER_RATE", "TRIGGER."+ohname.str()+".TRIGGER_RATE"),
                   GEMUpdateType::HW32, "dec");
    addMonitorable("Trigger Status", "HWMonitoring",
                   std::make_pair(ohname.str()+"_TRIGGER_CNT", "TRIGGER."+ohname.str()+".TRIGGER_CNT"),
                   GEMUpdateType::HW32, "hex");

    for (int cluster = 0; cluster < 8; ++cluster) {
      std::stringstream cluname;
      cluname << "CLUSTER_SIZE_" << cluster;
      addMonitorable("Trigger Status", "HWMonitoring",
                     std::make_pair(ohname.str()+"_"+cluname.str()+"_RATE", "TRIGGER."+ohname.str()+"."+cluname.str()+"_RATE"),
                     GEMUpdateType::HW32, "dec");
      addMonitorable("Trigger Status", "HWMonitoring",
                     std::make_pair(ohname.str()+"_"+cluname.str()+"_CNT", "TRIGGER."+ohname.str()+"."+cluname.str()+"_CNT"),
                     GEMUpdateType::HW32, "hex");
      // cluname.str("");
      // cluname.clear();
      // cluname << "DEBUG_LAST_CLUSTER_" << cluster;
      // addMonitorable("Trigger Status", "HWMonitoring",
      //                std::make_pair(ohname.str()+"_"+cluname.str(), "TRIGGER."+ohname.str()+"."+cluname.str()),
      //                GEMUpdateType::HW32, "hex");
    }
  }
  updateMonitorables();
}

gem::hw::glib::GLIBMonitor::~GLIBMonitor()
{

}

void gem::hw::glib::GLIBMonitor::updateMonitorables()
{
  // define how to update the desired values
  // get SYSTEM monitorables
  // can this be split into two loops, one just to do a list read, the second to fill the InfoSpace with the returned values
  CMSGEMOS_DEBUG("GLIBMonitor: Updating monitorables");
  for (auto monlist = m_monitorableSetsMap.begin(); monlist != m_monitorableSetsMap.end(); ++monlist) {
    CMSGEMOS_DEBUG("GLIBMonitor: Updating monitorables in set " << monlist->first);
    for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
      CMSGEMOS_DEBUG("GLIBMonitor: Updating monitorable " << monitem->first);
      std::stringstream regName;
      regName << p_glib->getDeviceBaseNode() << "." << monitem->second.regname;
      uint32_t address = p_glib->getNode(regName.str()).getAddress();
      uint32_t mask    = p_glib->getNode(regName.str()).getMask();
      if (monitem->second.updatetype == GEMUpdateType::HW8) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_glib->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::HW16) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_glib->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::HW24) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_glib->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::HW32) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_glib->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::HW64) {
        address = p_glib->getNode(regName.str()+".LOWER").getAddress();
        mask    = p_glib->getNode(regName.str()+".LOWER").getMask();
        uint32_t lower = p_glib->readReg(address,mask);
        address = p_glib->getNode(regName.str()+".UPPER").getAddress();
        mask    = p_glib->getNode(regName.str()+".UPPER").getMask();
        uint32_t upper = p_glib->readReg(address,mask);
        (monitem->second.infoSpace)->setUInt64(monitem->first, (((uint64_t)upper) << 32) + lower);
      } else if (monitem->second.updatetype == GEMUpdateType::I2CSTAT) {
        std::stringstream strobeReg;
        strobeReg << regName.str() << ".Strobe." << monitem->first;
        address = p_glib->getNode(strobeReg.str()).getAddress();
        mask    = p_glib->getNode(strobeReg.str()).getMask();
        uint32_t strobe = p_glib->readReg(address,mask);
        std::stringstream ackReg;
        ackReg << regName.str() << ".Ack." << monitem->first;
        address = p_glib->getNode(ackReg.str()).getAddress();
        mask    = p_glib->getNode(ackReg.str()).getMask();
        uint32_t ack = p_glib->readReg(address,mask);
        (monitem->second.infoSpace)->setUInt64(monitem->first, (((uint64_t)ack) << 32) + strobe);
      } else if (monitem->second.updatetype == GEMUpdateType::PROCESS) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_glib->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::TRACKER) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_glib->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::NOUPDATE) {
        continue;
      } else {
        CMSGEMOS_ERROR("GLIBMonitor: Unknown update type encountered");
        continue;
      }
    } // end loop over items in list
  } // end loop over monitorableSets
}

void gem::hw::glib::GLIBMonitor::buildMonitorPage(xgi::Output* out)
{
  CMSGEMOS_DEBUG("GLIBMonitor::buildMonitorPage");
  if (m_infoSpaceMonitorableSetMap.find("HWMonitoring") == m_infoSpaceMonitorableSetMap.end()) {
    CMSGEMOS_WARN("Unable to find item set HWMonitoring in monitor");
    return;
  }

  auto monsets = m_infoSpaceMonitorableSetMap.find("HWMonitoring")->second;

  // IMPROVEMENT make the tables dynamically with something like angular/react
  // loop over the list of monitor sets and grab the monitorables from each one
  // create a div tab for each set, and a table for each set of values
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  for (auto monset = monsets.begin(); monset != monsets.end(); ++monset) {
    if ((*monset).rfind("DAQ Status") != std::string::npos) {
      buildDAQStatusTable(out);
    } else if ((*monset).rfind("Trigger Status") != std::string::npos) {
      buildTriggerStatusTable(out);
    } else {
      CMSGEMOS_DEBUG("GLIBMonitor::buildMonitorPage building table " << *monset);
      *out << "<div class=\"xdaq-tab\" title=\""  << *monset << "\" >"  << std::endl
           << "<table class=\"xdaq-table\" id=\"" << *monset << "_table\">" << std::endl
           << cgicc::thead() << std::endl
           << cgicc::tr()    << std::endl // open
           << cgicc::th()    << "Register name"    << cgicc::th() << std::endl
           << cgicc::th()    << "Value"            << cgicc::th() << std::endl
           << cgicc::th()    << "Register address" << cgicc::th() << std::endl
           << cgicc::th()    << "Description"      << cgicc::th() << std::endl
           << cgicc::tr()    << std::endl // close
           << cgicc::thead() << std::endl
           << "<tbody>" << std::endl;

      for (auto monitem = m_monitorableSetsMap.find(*monset)->second.begin();
           monitem != m_monitorableSetsMap.find(*monset)->second.end(); ++monitem) {
        *out << "<tr>"    << std::endl;

        *out << "<td>"    << std::endl
             << monitem->first
             << "</td>"   << std::endl;

        CMSGEMOS_DEBUG("GLIBMonitor::" << monitem->first << " formatted to "
              << (monitem->second.infoSpace)->getFormattedItem(monitem->first,monitem->second.format));
        // this will be repeated for every GLIBMonitor in the GLIBManager..., need a better unique ID
        *out << "<td id=\"" << monitem->second.infoSpace->name() << "-" << monitem->first << "\">" << std::endl
             << (monitem->second.infoSpace)->getFormattedItem(monitem->first,monitem->second.format)
             << "</td>"   << std::endl;

        *out << "<td>"    << std::endl
             << monitem->second.regname
             << "</td>"   << std::endl;

        *out << "<td>"    << std::endl
             << "description"
             << "</td>"   << std::endl;

        *out << "</tr>"   << std::endl;
      }
      *out << "</tbody>"  << std::endl
           << "</table>"  << std::endl
           << "</div>"    << std::endl;  // closes monset tab
    }
  }
  *out << "</div>"  << std::endl;  // closes cardPage tab wrapper
}

void gem::hw::glib::GLIBMonitor::buildDAQStatusTable(xgi::Output* out)
{
  CMSGEMOS_DEBUG("GLIBMonitor::buildDAQStatusTable");
  if (m_infoSpaceMonitorableSetMap.find("HWMonitoring") == m_infoSpaceMonitorableSetMap.end()) {
    CMSGEMOS_WARN("Unable to find item set HWMonitoring in monitor");
    return;
  }

  auto monsets = m_infoSpaceMonitorableSetMap.find("HWMonitoring")->second;

  if (std::find(monsets.begin(),monsets.end(),"DAQ Status") == monsets.end()) {
    CMSGEMOS_WARN("Unable to find item set 'DAQ Status' in list of HWMonitoring monitor sets");
    return;
  }

  auto monset = m_monitorableSetsMap.find("DAQ Status")->second;
  CMSGEMOS_DEBUG("GLIBMonitor::buildDAQStatusTable building DAQ Status table");
  *out << "<div class=\"xdaq-tab\" title=\"DAQ Status\">" << std::endl
       << "<div class=\"xdaq-tab-wrapper\">" << std::endl;

  CMSGEMOS_DEBUG("GLIBMonitor::buildDAQStatusTable building Common DAQ Status table");
  *out << "<div class=\"xdaq-tab\" title=\"" << "Common DAQ Status" << "\">" << std::endl
       << "<table class=\"xdaq-table\" id=\"CommonDAQStatus_table\">" << std::endl
       << cgicc::thead() << std::endl
       << cgicc::tr()    << std::endl // open
       << cgicc::th()    << "Register name"    << cgicc::th() << std::endl
       << cgicc::th()    << "Value"            << cgicc::th() << std::endl
       << cgicc::th()    << "Register address" << cgicc::th() << std::endl
       << cgicc::th()    << "Description"      << cgicc::th() << std::endl
       << cgicc::tr()    << std::endl // close
       << cgicc::thead() << std::endl
       << "<tbody>" << std::endl;
  for (auto monpair = monset.begin(); monpair != monset.end(); ++monpair) {
    if (monpair->first.find("OH") == std::string::npos) {
      *out << "<tr>"    << std::endl;

      *out << "<td>"    << std::endl
           << monpair->first
           << "</td>"   << std::endl;

      CMSGEMOS_INFO("GLIBMonitor::" << monpair->first << " formatted to "
            << (monpair->second.infoSpace)->getFormattedItem(monpair->first,monpair->second.format));
      // this will be repeated for every GLIBMonitor in the GLIBManager..., need a better unique ID
      *out << "<td id=\"" << monpair->second.infoSpace->name() << "-" << monpair->first << "\">" << std::endl
           << (monpair->second.infoSpace)->getFormattedItem(monpair->first,monpair->second.format)
           << "</td>"   << std::endl;

      *out << "<td>"    << std::endl
           << monpair->second.regname
           << "</td>"   << std::endl;

      *out << "<td>"    << std::endl
           << "description"
           << "</td>"   << std::endl;

      *out << "</tr>"   << std::endl;
    }
  }
  *out << "</tbody>" << std::endl;
  *out << "</table>" << std::endl;
  *out << "</div>"   << std::endl;  // closes Common DAQ Status tab

  CMSGEMOS_INFO("GLIBMonitor::buildDAQStatusTable building Per-link DAQ Status table");
  *out << "<div class=\"xdaq-tab\" title=\""  << "Per-link DAQ Status" << "\" >" << std::endl
       << "<table class=\"xdaq-table\" id=\"Per-linkDAQStatus_table\">" << std::endl
       << cgicc::thead() << std::endl
       << cgicc::tr()    << std::endl // open
       << cgicc::th()    << "Register name"    << cgicc::th() << std::endl;
  for (int i = 0; i < 12; ++i)
    *out << cgicc::th() << "Link " << std::setw(2) << std::setfill(' ') << i << cgicc::th() << std::endl;

  *out << cgicc::th()    << "Register address" << cgicc::th() << std::endl
       << cgicc::th()    << "Description"      << cgicc::th() << std::endl
       << cgicc::tr()    << std::endl // close
       << cgicc::thead() << std::endl
       << "<tbody>" << std::endl;
  for (auto monpair = monset.begin(); monpair != monset.end(); ++monpair) {
    if (monpair->first.find("OH0_STATUS") != std::string::npos) {
      CMSGEMOS_INFO("GLIBMonitor::buildDAQStatusTable " << monpair->first << " found, building per-link structure");
      std::array<std::string, 6> linkarray = {{"STATUS",
                                               "EVN",
                                               "EOE_TIMEOUT",
                                               "MAX_EOE_TIMER",
                                               "LAST_EOE_TIMER",
                                               "CORRUPT_VFAT_BLK_CNT"}};
      for (auto regname = linkarray.begin(); regname != linkarray.end(); ++regname) {
        // std::string regname = monpair->first;
        // regname.erase(regname.rfind("OH0_"),4);
        *out << "<tr>"    << std::endl
             << "<td>"    << std::endl
             << *regname
             << "</td>"   << std::endl;

        for (int i = 0; i < 12; ++i) {
          CMSGEMOS_INFO("GLIBMonitor::buildDAQStatusTable creating OH" << i << " table header");
          std::stringstream substr;
          substr << "OH" << i;
          *out << "<td id=\"" << monpair->second.infoSpace->name() << "-OH" << i << "_" << *regname  << "\">" << std::endl
               << "N/A"
               << "</td>"   << std::endl;
        }
        *out << "<td>"    << std::endl
             << "add"
             << "</td>"   << std::endl
             << "<td>"    << std::endl
             << "desc"
             << "</td>"   << std::endl
             << "</tr>"   << std::endl;
      }
      break;
    }
  }
  *out << "</tbody>" << std::endl
       << "</table>" << std::endl
       << "</div>"   << std::endl  // closes Per-link DAQ Status tab
       << "</div>"   << std::endl  // closes DAQ Status tab
       << "</div>"   << std::endl;  // closes DAQ Status tab-wrapper
}

void gem::hw::glib::GLIBMonitor::buildTriggerStatusTable(xgi::Output* out)
{
  if (m_infoSpaceMonitorableSetMap.find("HWMonitoring") == m_infoSpaceMonitorableSetMap.end()) {
    CMSGEMOS_WARN("Unable to find item set HWMonitoring in monitor");
    return;
  }

  auto monsets = m_infoSpaceMonitorableSetMap.find("HWMonitoring")->second;

  if (std::find(monsets.begin(),monsets.end(),"Trigger Status") == monsets.end()) {
    CMSGEMOS_WARN("Unable to find item set 'Trigger Status' in list of HWMonitoring monitor sets");
    return;
  }

  auto monset = m_monitorableSetsMap.find("Trigger Status")->second;

  CMSGEMOS_INFO("GLIBMonitor::buildTriggerStatusTable building Trigger Status table");
  *out << "<div class=\"xdaq-tab\" title=\""  << "Trigger Status" << "\" >" << std::endl
       << "<table class=\"xdaq-table\" id=\"TriggerStatus_table\">" << std::endl
       << cgicc::thead() << std::endl
       << cgicc::tr()    << std::endl // open
       << cgicc::th()    << "Register name" << cgicc::th() << std::endl;
  for (int i = 0; i < 12; ++i)
    *out << cgicc::th() << "Link " << std::setw(2) << std::setfill(' ') << i << cgicc::th() << std::endl;

  *out << cgicc::th()    << "Register address" << cgicc::th() << std::endl
       << cgicc::th()    << "Description"      << cgicc::th() << std::endl
       << cgicc::tr()    << std::endl // close
       << cgicc::thead() << std::endl
       << "<tbody>" << std::endl;
  for (auto monpair = monset.begin(); monpair != monset.end(); ++monpair) {
    if (monpair->first.find("OH0_TRIGGER_RATE") != std::string::npos) {
      std::array<std::string, 2> linkarray = {{"TRIGGER_RATE", "TRIGGER_CNT"}};
      for (auto regname = linkarray.begin(); regname != linkarray.end(); ++regname) {
        CMSGEMOS_INFO("GLIBMonitor::buildTriggerStatusTable " << monpair->first << " found, building per-link structure");
        *out << "<tr>"    << std::endl
             << "<td>"    << std::endl
             << *regname
             << "</td>"   << std::endl;

        for (int i = 0; i < 12; ++i) {
          CMSGEMOS_INFO("GLIBMonitor::buildTriggerStatusTable creating OH" << i << " table header");
          *out << "<td id=\"" << monpair->second.infoSpace->name() << "-OH" << i << "_" << *regname << "\">" << std::endl
               << "N/A"
               << "</td>"   << std::endl;
        }
        *out << "<td>"    << std::endl
             << "add"
             << "</td>"   << std::endl
             << "<td>"    << std::endl
             << "desc"
             << "</td>"   << std::endl
             << "</tr>"   << std::endl;
      }

      // std::array<std::string, 2> linkarray = {{"CLUSTER_SIZE", "DEBUG_LAST_CLUSTER"}};
      std::string regname = "CLUSTER_SIZE";
      for (int j = 0; j < 8; ++j) {
        std::stringstream specregname;
        specregname << regname << "_" << j << "_CNT";
        *out << "<tr>"    << std::endl
             << "<td>"    << std::endl
             << specregname.str()
             << "</td>"   << std::endl;

        for (int i = 0; i < 12; ++i) {
          CMSGEMOS_INFO("GLIBMonitor::buildTriggerStatusTable creating OH" << i << " table header");
          *out << "<td id=\"" << monpair->second.infoSpace->name() << "-OH" << i << "_" << specregname.str() << "\">" << std::endl
               << "N/A"
               << "</td>"   << std::endl;
        }
        *out << "<td>"    << std::endl
             << "add"
             << "</td>"   << std::endl
             << "<td>"    << std::endl
             << "desc"
             << "</td>"   << std::endl
             << "</tr>"   << std::endl;
      }

      regname = "CLUSTER_SIZE";
      for (int j = 0; j < 8; ++j) {
        std::stringstream specregname;
        specregname << regname << "_" << j << "_RATE";
        *out << "<tr>"    << std::endl
             << "<td>"    << std::endl
             << specregname.str()
             << "</td>"   << std::endl;

        for (int i = 0; i < 12; ++i) {
          CMSGEMOS_INFO("GLIBMonitor::buildTriggerStatusTable creating OH" << i << " table header");
          *out << "<td id=\"" << monpair->second.infoSpace->name() << "-OH" << i << "_" << specregname.str() << "\">" << std::endl
               << "N/A"
               << "</td>"   << std::endl;
        }
        *out << "<td>"    << std::endl
             << "add"
             << "</td>"   << std::endl
             << "<td>"    << std::endl
             << "desc"
             << "</td>"   << std::endl
             << "</tr>"   << std::endl;
      }

      regname = "DEBUG_LAST_CLUSTER";
      for (int j = 0; j < 8; ++j) {
        std::stringstream specregname;
        specregname << regname << "_" << j;
        *out << "<tr>"    << std::endl
             << "<td>"    << std::endl
             << specregname.str()
             << "</td>"   << std::endl;

        for (int i = 0; i < 12; ++i) {
          CMSGEMOS_INFO("GLIBMonitor::buildTriggerStatusTable creating OH" << i << " table header");
          *out << "<td id=\"" << monpair->second.infoSpace->name() << "-OH" << i << "_" << specregname.str() << "\">" << std::endl
               << "N/A"
               << "</td>"   << std::endl;
        }
        *out << "<td>"    << std::endl
             << "add"
             << "</td>"   << std::endl
             << "<td>"    << std::endl
             << "desc"
             << "</td>"   << std::endl
             << "</tr>"   << std::endl;
      }
      break;
    }
  }
  *out << "</tbody>" << std::endl
       << "</table>" << std::endl
       << "</div>"   << std::endl;  // closes Trigger Status tab
}

void gem::hw::glib::GLIBMonitor::reset()
{
  // have to get rid of the timer
  CMSGEMOS_DEBUG("GEMMonitor::reset");
  for (auto infoSpace = m_infoSpaceMap.begin(); infoSpace != m_infoSpaceMap.end(); ++infoSpace) {
    CMSGEMOS_DEBUG("GLIBMonitor::reset removing " << infoSpace->first << " from p_timer");
    try {
      p_timer->remove(infoSpace->first);
    } catch (toolbox::task::exception::Exception& te) {
      CMSGEMOS_ERROR("GLIBMonitor::Caught exception while removing timer task " << infoSpace->first << " " << te.what());
    }
  }
  stopMonitoring();
  CMSGEMOS_DEBUG("GEMMonitor::reset removing timer " << m_timerName << " from timerFactory");
  try {
    toolbox::task::getTimerFactory()->removeTimer(m_timerName);
  } catch (toolbox::task::exception::Exception& te) {
    CMSGEMOS_ERROR("GLIBMonitor::Caught exception while removing timer " << m_timerName << " " << te.what());
  }

  CMSGEMOS_DEBUG("GLIBMonitor::reset - clearing all maps");
  m_infoSpaceMap.clear();
  m_infoSpaceMonitorableSetMap.clear();
  m_monitorableSetInfoSpaceMap.clear();
  m_monitorableSetsMap.clear();
}
