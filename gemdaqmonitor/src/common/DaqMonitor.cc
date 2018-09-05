/**
 * class: DaqMonitor
 * description: Monitor application for GEM DAQ
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: M. Dalchenko
 * date:
 */

#include "gem/daqmon/DaqMonitor.h"

typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

//FIXME establish required arguments, eventually retrieve from the config
gem::daqmon::DaqMonitor::DaqMonitor():
  xhal::XHALInterface(board_domain_name),
{
  this->loadModule("amc", "amc v1.0.1");
  //updateMonitorables();
}

void gem::daqmon::DaqMonitor::reconnect()
{
  if (!isConnected){
    this->connect();
    this->loadModule("amc", "amc v1.0.1");
  } else {
    ERROR("Interface already connected. Reconnection failed");
    throw xhal::utils::XHALRPCException("RPC exception: Interface already connected. Reconnection failed");
  }
}

void gem::daqmon::DaqMonitor::setupDaqMonitoring()
{
  // create the values to be monitored in the info space
  //FIXME understand the infoSpace creation and naming...
  addMonitorableSet("DAQ_MAIN","DAQ_MONITORING");
  //FIXME review GEMUpdateType for the set
  //DAQ_MAIN monitorables
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("DAQ_ENABLE","GEM_AMC.DAQ.CONTROL.DAQ_ENABLE"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("DAQ_LINK_READY","GEM_AMC.DAQ.STATUS.DAQ_LINK_RDY"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("DAQ_LINK_AFULL","GEM_AMC.DAQ.STATUS.DAQ_LINK_AFULL"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("DAQ_OFIFO_HAD_OFLOW","GEM_AMC.DAQ.STATUS.DAQ_OUTPUT_FIFO_HAD_OVERFLOW"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("L1A_FIFO_HAD_OFLOW","GEM_AMC.DAQ.STATUS.L1A_FIFO_HAD_OVERFLOW"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("L1A_FIFO_DATA_COUNT","GEM_AMC.DAQ.EXT_STATUS.L1A_FIFO_DATA_CNT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("DAQ_FIFO_DATA_COUNT","GEM_AMC.DAQ.EXT_STATUS.DAQ_FIFO_DATA_CNT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("EVENT_SENT","GEM_AMC.DAQ.EXT_STATUS.EVT_SENT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("TTS_STATE","GEM_AMC.DAQ.STATUS.TTS_STATE"),
                 GEMUpdateType::HW32, "hex");
  //end of DAQ_MAIN monitorables

  addMonitorableSet("DAQ_OH_MAIN","DAQ_MONITORING");
  //DAQ_OH_MAIN monitorables
  //FIXME Putting "DUMMY" as reg full name at the moment. May want to define all tables here and pass as a list to RPC
  for (unsigned int i = 0; i < NOH; ++i) {
    addMonitorable("DAQ_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".STATUS.EVT_SIZE_ERR","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("DAQ_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".STATUS.EVENT_FIFO_HAD_OFLOW","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("DAQ_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".STATUS.INPUT_FIFO_HAD_OFLOW","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("DAQ_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".STATUS.INPUT_FIFO_HAD_UFLOW","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("DAQ_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".STATUS.VFAT_TOO_MANY","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("DAQ_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".STATUS.VFAT_NO_MARKER","DUMMY"),
                 GEMUpdateType::HW32, "hex");
  }
  //end of DAQ_OH_MAIN monitorables

  addMonitorableSet("DAQ_TTC_MAIN","DAQ_MONITORING");
  //DAQ_TTC_MAIN monitorables
  addMonitorable("DAQ_TTC_MAIN", "DAQ_MONITORING",
               std::make_pair("MMCM_LOCKED","DUMMY"),
               GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ_TTC_MAIN", "DAQ_MONITORING",
               std::make_pair("TTC_SINGLE_ERROR_CNT","DUMMY"),
               GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ_TTC_MAIN", "DAQ_MONITORING",
               std::make_pair("BC0_LOCKED","DUMMY"),
               GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ_TTC_MAIN", "DAQ_MONITORING",
               std::make_pair("L1A_ID","DUMMY"),
               GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ_TTC_MAIN", "DAQ_MONITORING",
               std::make_pair("L1A_RATE","DUMMY"),
               GEMUpdateType::HW32, "hex");
  //end of DAQ_TTC_MAIN monitorables

  addMonitorableSet("DAQ_TRIGGER_MAIN","DAQ_MONITORING");
  //DAQ_TRIGGER_MAIN monitorables
  for (unsigned int i = 0; i < NOH; ++i) {
    addMonitorable("DAQ_TRIGGER_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+"TRIGGER_RATE","DUMMY"),
                 GEMUpdateType::HW32, "hex");
  }
  //end of DAQ_TRIGGER_MAIN monitorables

  addMonitorableSet("DAQ_TRIGGER_OH_MAIN","DAQ_MONITORING");
  //DAQ_TRIGGER_OH_MAIN monitorables
  for (unsigned int i = 0; i < NOH; ++i) {
    addMonitorable("DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+"LINK0_MISSED_COMMA_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".LINK1_MISSED_COMMA_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".LINK0_OVERFLOW_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".LINK1_OVERFLOW_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".LINK0_UNDERFLOW_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".LINK1_UNDERFLOW_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".LINK0_SBIT_OVERFLOW_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".LINK1_SBIT_OVERFLOW_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
  }
  //end of DAQ_TRIGGER_OH_MAIN monitorables

  addMonitorableSet("OH_MAIN","DAQ_MONITORING");
  //OH_MAIN monitorables
  for (unsigned int i = 0; i < NOH; ++i) {
    addMonitorable("OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+"FW_VERSION","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".EVENT_COUNTER","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".EVENT_RATE","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".GTX.TRK_ERR","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".GTX.TRG_ERR","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".GBT.TRK_ERR","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    addMonitorable("OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".CORR_VFAT_BLK_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
  }
  //end of OH_MAIN monitorables

}

void gem::daqmon::DaqMonitor::updateMonitorables()
{
  DEBUG("DaqMonitor: Updating Monitorables");
  try {
    updateDAQmain();
    updateDAQOHmain();
    updateTTCmain();
    updateTRIGGERmain();
    updateTRIGGEROHmain();
    updateOHmain();
  } catch (...) {} //FIXME Define meaningful exceptions and intercept here or eventually at a different level...
}

void gem::daqmon::DaqMonitor::updateDAQmain()
{
  DEBUG("DaqMonitor: Update DAQ main table");
}
