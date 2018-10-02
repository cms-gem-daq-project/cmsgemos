/**
 * class: DaqMonitor
 * description: Monitor application for GEM DAQ
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: M. Dalchenko
 * date:
 */

#include "gem/daqmon/DaqMonitor.h"
#include <iomanip>

typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

//FIXME establish required arguments, eventually retrieve from the config
gem::daqmon::DaqMonitor::DaqMonitor(const std::string& board_domain_name,log4cplus::Logger& logger, gem::base::GEMApplication* gemApp, int const& index):
  gem::base::GEMMonitor::GEMMonitor(logger, gemApp, index),
  //xhal::XHALInterface(board_domain_name, logger) //FIXME: if using shared logger, then XHALInterface overtakes everything and logging from XDAQ doesn't go through
  xhal::XHALInterface(board_domain_name) //Works as is, providing a bit messy logging, but with all info in place
{
  DEBUG("DaqMonitor::DaqMonitor:: entering constructor");
  if (isConnected) { //TODO Add to the app monitoring space? Need to know in order to mask in web interface the boards which failed to connect
    this->loadModule("amc", "amc v1.0.1");
    DEBUG("DaqMonitor::DaqMonitor:: amc module loaded");
  } else {
    INFO("DaqMonitor::DaqMonitor:: RPC interface failed to connect");
  }
  toolbox::net::URN hwCfgURN("urn:gem:hw:"+board_domain_name);
  DEBUG("DaqMonitor::DaqMonitor:: infospace " << hwCfgURN.toString() << " does not exist, creating");
  is_daqmon =  is_toolbox_ptr(new gem::base::utils::GEMInfoSpaceToolBox(p_gemApp,
                                                                        hwCfgURN.toString(),
                                                                        true));
  addInfoSpace("DAQ_MONITORING", is_daqmon, toolbox::TimeInterval(1,  0));
  setupDaqMonitoring();
  updateMonitorables();
  DEBUG("gem::daqmon::DaqMonitor : constructor done");
}

gem::daqmon::DaqMonitor::~DaqMonitor()
{
  DEBUG("gem::daqmon::DaqMonitor : destructor called");
//TODO
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

void gem::daqmon::DaqMonitor::reset()
{
//TODO
}

void gem::daqmon::DaqMonitor::setupDaqMonitoring()
{
  // create the values to be monitored in the info space
  //FIXME understand the infoSpace creation and naming...
  addMonitorableSet("DAQ_MAIN","DAQ_MONITORING");
  //FIXME review GEMUpdateType for the set
  //DAQ_MAIN monitorables
  is_daqmon->createUInt32("DAQ_ENABLE",    0,        NULL, GEMUpdateType::HW32);
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("DAQ_ENABLE","GEM_AMC.DAQ.CONTROL.DAQ_ENABLE"),
                 GEMUpdateType::HW32, "hex");
  is_daqmon->createUInt32("DAQ_LINK_READY",    0,        NULL, GEMUpdateType::HW32);
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("DAQ_LINK_READY","GEM_AMC.DAQ.STATUS.DAQ_LINK_RDY"),
                 GEMUpdateType::HW32, "hex");
  is_daqmon->createUInt32("DAQ_LINK_AFULL",    0,        NULL, GEMUpdateType::HW32);
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("DAQ_LINK_AFULL","GEM_AMC.DAQ.STATUS.DAQ_LINK_AFULL"),
                 GEMUpdateType::HW32, "hex");
  is_daqmon->createUInt32("DAQ_OFIFO_HAD_OFLOW",    0,        NULL, GEMUpdateType::HW32);
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("DAQ_OFIFO_HAD_OFLOW","GEM_AMC.DAQ.STATUS.DAQ_OUTPUT_FIFO_HAD_OVERFLOW"),
                 GEMUpdateType::HW32, "hex");
  is_daqmon->createUInt32("L1A_FIFO_HAD_OFLOW",    0,        NULL, GEMUpdateType::HW32);
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("L1A_FIFO_HAD_OFLOW","GEM_AMC.DAQ.STATUS.L1A_FIFO_HAD_OVERFLOW"),
                 GEMUpdateType::HW32, "hex");
  is_daqmon->createUInt32("L1A_FIFO_DATA_COUNT",    0,        NULL, GEMUpdateType::HW32);
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("L1A_FIFO_DATA_COUNT","GEM_AMC.DAQ.EXT_STATUS.L1A_FIFO_DATA_CNT"),
                 GEMUpdateType::HW32, "hex");
  is_daqmon->createUInt32("DAQ_FIFO_DATA_COUNT",    0,        NULL, GEMUpdateType::HW32);
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("DAQ_FIFO_DATA_COUNT","GEM_AMC.DAQ.EXT_STATUS.DAQ_FIFO_DATA_CNT"),
                 GEMUpdateType::HW32, "hex");
  is_daqmon->createUInt32("EVENT_SENT",    0,        NULL, GEMUpdateType::HW32);
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("EVENT_SENT","GEM_AMC.DAQ.EXT_STATUS.EVT_SENT"),
                 GEMUpdateType::HW32, "hex");
  is_daqmon->createUInt32("TTS_STATE",    0,        NULL, GEMUpdateType::HW32);
  addMonitorable("DAQ_MAIN", "DAQ_MONITORING",
                 std::make_pair("TTS_STATE","GEM_AMC.DAQ.STATUS.TTS_STATE"),
                 GEMUpdateType::HW32, "hex");
  //end of DAQ_MAIN monitorables

  addMonitorableSet("DAQ_OH_MAIN","DAQ_MONITORING");
  //DAQ_OH_MAIN monitorables
  //FIXME Putting "DUMMY" as reg full name at the moment. May want to define all tables here and pass as a list to RPC
  for (unsigned int i = 0; i < NOH; ++i) {
    is_daqmon->createUInt32("OH"+std::to_string(i)+".STATUS.EVT_SIZE_ERR",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("DAQ_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".STATUS.EVT_SIZE_ERR","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".STATUS.EVENT_FIFO_HAD_OFLOW",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("DAQ_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".STATUS.EVENT_FIFO_HAD_OFLOW","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".STATUS.INPUT_FIFO_HAD_OFLOW",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("DAQ_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".STATUS.INPUT_FIFO_HAD_OFLOW","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".STATUS.INPUT_FIFO_HAD_UFLOW",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("DAQ_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".STATUS.INPUT_FIFO_HAD_UFLOW","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".STATUS.VFAT_TOO_MANY",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("DAQ_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".STATUS.VFAT_TOO_MANY","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".STATUS.VFAT_NO_MARKER",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("DAQ_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".STATUS.VFAT_NO_MARKER","DUMMY"),
                 GEMUpdateType::HW32, "hex");
  }
  //end of DAQ_OH_MAIN monitorables

  addMonitorableSet("DAQ_TTC_MAIN","DAQ_MONITORING");
  //DAQ_TTC_MAIN monitorables
  is_daqmon->createUInt32("MMCM_LOCKED",    0,        NULL, GEMUpdateType::HW32);
  addMonitorable("DAQ_TTC_MAIN", "DAQ_MONITORING",
               std::make_pair("MMCM_LOCKED","DUMMY"),
               GEMUpdateType::HW32, "hex");
  is_daqmon->createUInt32("TTC_SINGLE_ERROR_CNT",    0,        NULL, GEMUpdateType::HW32);
  addMonitorable("DAQ_TTC_MAIN", "DAQ_MONITORING",
               std::make_pair("TTC_SINGLE_ERROR_CNT","DUMMY"),
               GEMUpdateType::HW32, "hex");
  is_daqmon->createUInt32("BC0_LOCKED",    0,        NULL, GEMUpdateType::HW32);
  addMonitorable("DAQ_TTC_MAIN", "DAQ_MONITORING",
               std::make_pair("BC0_LOCKED","DUMMY"),
               GEMUpdateType::HW32, "hex");
  is_daqmon->createUInt32("L1A_ID",    0,        NULL, GEMUpdateType::HW32);
  addMonitorable("DAQ_TTC_MAIN", "DAQ_MONITORING",
               std::make_pair("L1A_ID","DUMMY"),
               GEMUpdateType::HW32, "hex");
  is_daqmon->createUInt32("L1A_RATE",    0,        NULL, GEMUpdateType::HW32);
  addMonitorable("DAQ_TTC_MAIN", "DAQ_MONITORING",
               std::make_pair("L1A_RATE","DUMMY"),
               GEMUpdateType::HW32, "hex");
  //end of DAQ_TTC_MAIN monitorables

  addMonitorableSet("DAQ_TRIGGER_MAIN","DAQ_MONITORING");
  //DAQ_TRIGGER_MAIN monitorables
  for (unsigned int i = 0; i < NOH; ++i) {
    is_daqmon->createUInt32("OH"+std::to_string(i)+".TRIGGER_RATE",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("DAQ_TRIGGER_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".TRIGGER_RATE","DUMMY"),
                 GEMUpdateType::HW32, "hex");
  }
  //end of DAQ_TRIGGER_MAIN monitorables

  addMonitorableSet("DAQ_TRIGGER_OH_MAIN","DAQ_MONITORING");
  //DAQ_TRIGGER_OH_MAIN monitorables
  for (unsigned int i = 0; i < NOH; ++i) {
    is_daqmon->createUInt32("OH"+std::to_string(i)+".LINK0_MISSED_COMMA_CNT",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".LINK0_MISSED_COMMA_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".LINK1_MISSED_COMMA_CNT",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".LINK1_MISSED_COMMA_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".LINK0_OVERFLOW_CNT",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".LINK0_OVERFLOW_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".LINK1_OVERFLOW_CNT",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".LINK1_OVERFLOW_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".LINK0_UNDERFLOW_CNT",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".LINK0_UNDERFLOW_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".LINK1_UNDERFLOW_CNT",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".LINK1_UNDERFLOW_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".LINK0_SBIT_OVERFLOW_CNT",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".LINK0_SBIT_OVERFLOW_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".LINK1_SBIT_OVERFLOW_CNT",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".LINK1_SBIT_OVERFLOW_CNT","DUMMY"),
                 GEMUpdateType::HW32, "hex");
  }
  //end of DAQ_TRIGGER_OH_MAIN monitorables

  addMonitorableSet("OH_MAIN","DAQ_MONITORING");
  //OH_MAIN monitorables
  for (unsigned int i = 0; i < NOH; ++i) {
    is_daqmon->createUInt32("OH"+std::to_string(i)+".FW_VERSION",    0xdeaddead,        NULL, GEMUpdateType::HW32);
    addMonitorable("OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".FW_VERSION","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".EVENT_COUNTER",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".EVENT_COUNTER","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".EVENT_RATE",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".EVENT_RATE","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".GTX.TRK_ERR",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".GTX.TRK_ERR","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".GTX.TRG_ERR",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".GTX.TRG_ERR","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".GBT.TRK_ERR",    0,        NULL, GEMUpdateType::HW32);
    addMonitorable("OH_MAIN", "DAQ_MONITORING",
                 std::make_pair("OH"+std::to_string(i)+".GBT.TRK_ERR","DUMMY"),
                 GEMUpdateType::HW32, "hex");
    is_daqmon->createUInt32("OH"+std::to_string(i)+".CORR_VFAT_BLK_CNT",    0,        NULL, GEMUpdateType::HW32);
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
  req = wisc::RPCMsg("amc.getmonDAQmain");
  try {
    rsp = rpc.call_method(req);
  }
  STANDARD_CATCH;
  try{
    if (rsp.get_key_exists("error")) {
      ERROR("DAQ_MAIN update error:" << rsp.get_string("error").c_str());
      throw xhal::utils::XHALException("DAQ_MAIN update failed");
    } else {
      auto monlist = m_monitorableSetsMap.find("DAQ_MAIN");
      for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,rsp.get_word(monitem->first));
      }
    }
  }
  STANDARD_CATCH;
}

void gem::daqmon::DaqMonitor::updateDAQOHmain()
{
  DEBUG("DaqMonitor: Update DAQ OH main table");
  req = wisc::RPCMsg("amc.getmonDAQOHmain");
  try {
    rsp = rpc.call_method(req);
  }
  STANDARD_CATCH;
  try{
    if (rsp.get_key_exists("error")) {
      ERROR("DAQ_OH_MAIN update error:" << rsp.get_string("error").c_str());
      throw xhal::utils::XHALException("DAQ_OH_MAIN update failed");
    } else {
      auto monlist = m_monitorableSetsMap.find("DAQ_OH_MAIN");
      for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,rsp.get_word(monitem->first));
      }
    }
  }
  STANDARD_CATCH;
}

void gem::daqmon::DaqMonitor::updateTTCmain()
{
  DEBUG("DaqMonitor: Update TTC main table");
  req = wisc::RPCMsg("amc.getmonTTCmain");
  try {
    rsp = rpc.call_method(req);
  }
  STANDARD_CATCH;
  try{
    if (rsp.get_key_exists("error")) {
      ERROR("DAQ_TTC_MAIN update error:" << rsp.get_string("error").c_str());
      throw xhal::utils::XHALException("DAQ_TTC_MAIN update failed");
    } else {
      auto monlist = m_monitorableSetsMap.find("DAQ_TTC_MAIN");
      for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,rsp.get_word(monitem->first));
      }
    }
  }
  STANDARD_CATCH;
}

void gem::daqmon::DaqMonitor::updateTRIGGERmain()
{
  DEBUG("DaqMonitor: Update TRIGGER main table");
  req = wisc::RPCMsg("amc.getmonTRIGGERmain");
  try {
    rsp = rpc.call_method(req);
  }
  STANDARD_CATCH;
  try{
    if (rsp.get_key_exists("error")) {
      ERROR("DAQ_TRIGGER_MAIN update error:" << rsp.get_string("error").c_str());
      throw xhal::utils::XHALException("DAQ_TRIGGER_MAIN update failed");
    } else {
      auto monlist = m_monitorableSetsMap.find("DAQ_TRIGGER_MAIN");
      for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,rsp.get_word(monitem->first));
      }
    }
  }
  STANDARD_CATCH;
}

void gem::daqmon::DaqMonitor::updateTRIGGEROHmain()
{
  DEBUG("DaqMonitor: Update TRIGGER OH main table");
  req = wisc::RPCMsg("amc.getmonTRIGGEROHmain");
  try {
    rsp = rpc.call_method(req);
  }
  STANDARD_CATCH;
  try{
    if (rsp.get_key_exists("error")) {
      ERROR("DAQ_TRIGGER_OH_MAIN update error:" << rsp.get_string("error").c_str());
      throw xhal::utils::XHALException("DAQ_TRIGGER_OH_MAIN update failed");
    } else {
      auto monlist = m_monitorableSetsMap.find("DAQ_TRIGGER_OH_MAIN");
      for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,rsp.get_word(monitem->first));
      }
    }
  }
  STANDARD_CATCH;
}

void gem::daqmon::DaqMonitor::updateOHmain()
{
  DEBUG("DaqMonitor: Update OH main table");
  req = wisc::RPCMsg("amc.getmonOHmain");
  try {
    rsp = rpc.call_method(req);
  }
  STANDARD_CATCH;
  try{
    if (rsp.get_key_exists("error")) {
      ERROR("OH_MAIN update error:" << rsp.get_string("error").c_str());
      throw xhal::utils::XHALException("OH_MAIN update failed");
    } else {
      auto monlist = m_monitorableSetsMap.find("OH_MAIN");
      for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,rsp.get_word(monitem->first));
      }
    }
  }
  STANDARD_CATCH;
}

void gem::daqmon::DaqMonitor::buildDAQmainTable(xgi::Output* out)
{
  DEBUG("DaqMonitor: Build DAQ main table");
  *out << "<font size=\"1\">" << std::endl;
  *out << "<small>" << std::endl;
  std::vector<std::array<std::string,5>> daqlist;
  daqlist.push_back({{"DAQ_ENABLE","YES","NO","success","warning"}});
  daqlist.push_back({{"DAQ_LINK_READY","YES","NO","success","warning"}});
  daqlist.push_back({{"DAQ_LINK_AFULL","YES","NO","warning","success"}});
  daqlist.push_back({{"DAQ_OFIFO_HAD_OFLOW","YES","NO","danger","success"}});
  daqlist.push_back({{"L1A_FIFO_HAD_OFLOW","YES","NO","danger","success"}});
  daqlist.push_back({{"L1A_FIFO_DATA_COUNT","YES","NO","info","info"}});
  daqlist.push_back({{"DAQ_FIFO_DATA_COUNT","YES","NO","info","info"}});
  daqlist.push_back({{"EVENT_SENT","YES","NO","info","info"}});

  int val = 0;

  *out << "<table align=\"center\" class=\"table table-bordered table-condensed\" style=\"width:100%\">" << std::endl;
  for (auto daq: daqlist) {
    *out << "    <tr>" << std::endl;
    *out << "    <td style=\"width:10%\">"<< daq[0] << "</td>" << std::endl;
    val = is_daqmon->getUInt32(daq[0]);
    if ((daq[0].find("DATA_COUNT") != std::string::npos) || (daq[0].find("EVENT_SENT") != std::string::npos)){
      daq[1] = std::to_string(val);
      daq[2] = std::to_string(val);
    }
    if (val>0) {
      *out << "<td><span class=\"label label-" << daq[3] << "\">" << daq[1] << "</span></td>" << std::endl;
    } else {
      *out << "<td><span class=\"label label-" << daq[4] << "\">" << daq[2] << "</span></td>" << std::endl;
    }
    *out << "    </tr>" << std::endl;
  }
  *out << "    <tr>" << std::endl;
  *out << "    <td style=\"width:10%\">"<< "TTS_STATE" << "</td>" << std::endl;
  val = is_daqmon->getUInt32("TTS_STATE");
  switch (val) {
    case 1:
      *out << "<td><span class=\"label label-info\" style=\"min-width:5em;\">BUSY</span></td>" << std::endl;
    case 2:
      *out << "<td><span class=\"label label-danger\" style=\"min-width:5em;\">ERROR</span></td>" << std::endl;
    case 3:
      *out << "<td><span class=\"label label-warning\" style=\"min-width:5em;\">WARN</span></td>" << std::endl;
    case 4:
      *out << "<td><span class=\"label label-danger\" style=\"min-width:5em;\">OOS</span></td>" << std::endl;
    case 8:
      *out << "<td><span class=\"label label-success\" style=\"min-width:5em;\">READY</span></td>" << std::endl;
    default:
      *out << "<td><span class=\"label label-default\" style=\"min-width:5em;\">NDF</span></td>" << std::endl;
  }
  *out << "    </tr>" << std::endl;

  *out << "</table>" << std::endl;
  *out << "</small>" << std::endl;
  *out << "</font>" << std::endl;
}

void gem::daqmon::DaqMonitor::buildTTCmainTable(xgi::Output* out)
{
  DEBUG("DaqMonitor: Build TTC main table");
  *out << "<font size=\"1\">" << std::endl;
  *out << "<small>" << std::endl;
  std::vector<std::array<std::string,5>> ttclist;
  ttclist.push_back({{"MMCM_LOCKED","YES","NO","success","danger"}});
  ttclist.push_back({{"TTC_SINGLE_ERROR_CNT","YES","NO","danger","success"}});
  ttclist.push_back({{"BC0_LOCKED","YES","NO","success","danger"}});
  ttclist.push_back({{"L1A_ID","YES","NO","info","info"}});
  ttclist.push_back({{"L1A_RATE","YES","NO","info","info"}});

  int val = 0;

  *out << "<table align=\"center\" class=\"table table-bordered table-condensed\" style=\"width:100%\">" << std::endl;
  for (auto ttc: ttclist) {
    *out << "    <tr>" << std::endl;
    *out << "    <td style=\"width:10%\">"<< ttc[0] << "</td>" << std::endl;
    val = is_daqmon->getUInt32(ttc[0]);
    if ((ttc[0].find("L1A") != std::string::npos) || (ttc[0].find("TTC") != std::string::npos)){
      ttc[1] = std::to_string(val);
      ttc[2] = std::to_string(val);
    }
    if (val>0) {
      *out << "<td><span class=\"label label-" << ttc[3] << "\">" << ttc[1] << "</span></td>" << std::endl;
    } else {
      *out << "<td><span class=\"label label-" << ttc[4] << "\">" << ttc[2] << "</span></td>" << std::endl;
    }
    *out << "    </tr>" << std::endl;
  }

  *out << "</table>" << std::endl;
  *out << "</small>" << std::endl;
  *out << "</font>" << std::endl;
}

void gem::daqmon::DaqMonitor::buildOHmainTable(xgi::Output* out)
{
  DEBUG("DaqMonitor: Build OH main table");

  *out << "<font size=\"1\">" << std::endl;
  *out << "<table align=\"center\" class=\"table table-bordered table-condensed\" style=\"width:100%\">" << std::endl;
  *out << "    <tr>" << std::endl;
  *out << "    <td style=\"width:10%\">"<< "REGISTER|OH" << "</td>" << std::endl;
  for (int i=0; i<12; ++i) {
    *out << "<td>" << std::to_string(i) << "</td>";
  }
  *out << "    </tr>" << std::endl;

  std::vector<std::string> ohlist;
  ohlist.push_back("FW_VERSION");
  ohlist.push_back("EVENT_COUNTER");
  ohlist.push_back("EVENT_RATE");
  ohlist.push_back("GTX.TRK_ERR");
  ohlist.push_back("GTX.TRG_ERR");
  ohlist.push_back("GBT.TRK_ERR");
  ohlist.push_back("CORR_VFAT_BLK_CNT");
  ohlist.push_back("LINK0_MISSED_COMMA_CNT");
  ohlist.push_back("LINK1_MISSED_COMMA_CNT");
  ohlist.push_back("LINK0_OVERFLOW_CNT");
  ohlist.push_back("LINK1_OVERFLOW_CNT");
  ohlist.push_back("LINK0_UNDERFLOW_CNT");
  ohlist.push_back("LINK1_UNDERFLOW_CNT");
  ohlist.push_back("LINK0_SBIT_OVERFLOW_CNT");
  ohlist.push_back("LINK1_SBIT_OVERFLOW_CNT");
  
  int val = -1;

  for (auto oh: ohlist) {
    *out << "    <tr>" << std::endl;
    *out << "<small>" << std::endl;
    *out << "    <td style=\"width:10%\">"<< oh << "</td>" << std::endl;
    *out << "</small>" << std::endl;
    for (int j = 0; j < NOH; ++j) {
      val = is_daqmon->getUInt32("OH"+std::to_string(j)+"."+oh);
      if (val == -1) {
        *out << "<td><span class=\"label label-danger\">" << val << "</span></td>" << std::endl;
      } else {
        if ((val > 0) and (oh.find("FW") == std::string::npos) and (oh.find("EVENT") == std::string::npos)) {
          *out << "<td><span class=\"label label-warning\">" << val << "</span></td>" << std::endl;
        } else {
          if (oh.find("FW") != std::string::npos) {
            if (val == 0xdeaddead) {
              *out << "<td><span class=\"label label-danger\">ERROR</span></td>" << std::endl;
            } else {
              //FIXME eventually overlook the FW version representation
              *out << "<td><span class=\"label label-info\">" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << val << std::dec << "</span></td>" << std::endl;
            }
          } else {
            *out << "<td><span class=\"label label-info\">" << val << "</span></td>" << std::endl;
          }
        }
      }
    }
    *out << "    </tr>" << std::endl;
  }

  std::vector<std::string> ohlist_flags;
  ohlist_flags.push_back("EVT_SIZE_ERR");
  ohlist_flags.push_back("EVENT_FIFO_HAD_OFLOW");
  ohlist_flags.push_back("INPUT_FIFO_HAD_OFLOW");
  ohlist_flags.push_back("INPUT_FIFO_HAD_UFLOW");
  ohlist_flags.push_back("VFAT_TOO_MANY");
  ohlist_flags.push_back("VFAT_NO_MARKER");
  for (auto oh: ohlist_flags) {
    *out << "    <tr>" << std::endl;
    *out << "<small>" << std::endl;
    *out << "    <td style=\"width:10%\">"<< oh << "</td>" << std::endl;
    *out << "</small>" << std::endl;
    for (int j = 0; j < NOH; ++j) {
      val = is_daqmon->getUInt32("OH"+std::to_string(j)+".STATUS."+oh);
      if (val == -1) {
        *out << "<td><span class=\"label label-default\">" << "X" << "</span></td>" << std::endl;
      } else {
        if (val == 0) {
          *out << "<td><span class=\"label label-success\">" << "N" << "</span></td>" << std::endl;
        } else {
          *out << "<td><span class=\"label label-danger\">" << "Y" << "</span></td>" << std::endl;
        }
      }
    }
    *out << "    </tr>" << std::endl;
  }
  
  *out << "</table>" << std::endl;
  *out << "</font>" << std::endl;
}

void gem::daqmon::DaqMonitor::buildMonitorPage(xgi::Output* out)
{
  *out << "<div class=\"col-lg-3\">" << std::endl;
    *out << "<div class=\"panel panel-default\">" << std::endl;
      *out << "<div class=\"panel-heading\">" << std::endl;
        *out << "<h4 align=\"center\">" << std::endl;
          *out << "DAQ" << std::endl;
        *out << "</h4>" << std::endl;
        //FIXME add IEMASK later
        buildDAQmainTable(out);
      *out << "</div>" << std::endl; // end panel head
      // There could be a panel body here
    *out << "</div>" << std::endl; // end panel
    // There could be other elements in the column...
    *out << "<div class=\"panel panel-default\">" << std::endl;
      *out << "<div class=\"panel-heading\">" << std::endl;
        *out << "<h4 align=\"center\">" << std::endl;
          *out << "TTC" << std::endl;
        *out << "</h4>" << std::endl;
        buildTTCmainTable(out);
      *out << "</div>" << std::endl; // end panel head
      // There could be a panel body here
    *out << "</div>" << std::endl; // end panel
   *out << "</div>" << std::endl; // end column
/*
  *out << "<div class=\"col-lg-2\">" << std::endl;
    *out << "<div class=\"panel panel-default\">" << std::endl;
      *out << "<div class=\"panel-heading\">" << std::endl;
        *out << "<h4 align=\"center\">" << std::endl;
          *out << "TTC" << std::endl;
        *out << "</h4>" << std::endl;
        buildTTCmainTable(out);
      *out << "</div>" << std::endl; // end panel head
      // There could be a panel body here
    *out << "</div>" << std::endl; // end panel
    // There could be other elements in the column...
  *out << "</div>" << std::endl; // end column
  *out << "<div class=\"col-lg-8\">" << std::endl;
*/
  *out << "<div class=\"col-lg-9\">" << std::endl;
    *out << "<div class=\"panel panel-default\">" << std::endl;
      *out << "<div class=\"panel-heading\">" << std::endl;
        *out << "<h4 align=\"center\">" << std::endl;
          *out << "OPTICAL LINKS" << std::endl;
        *out << "</h4>" << std::endl;
        //FIXME add IEMASK later
        buildOHmainTable(out);
      *out << "</div>" << std::endl; // end panel head
      // There could be a panel body here
    *out << "</div>" << std::endl; // end panel
    // There could be other elements in the column...
  *out << "</div>" << std::endl; // end column
}
