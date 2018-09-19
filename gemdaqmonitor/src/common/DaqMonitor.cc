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
gem::daqmon::DaqMonitor::DaqMonitor(const std::string& board_domain_name,log4cplus::Logger& logger, gem::base::GEMApplication* gemApp, int const& index):
  xhal::XHALInterface(board_domain_name, logger),
  gem::base::GEMMonitor::GEMMonitor(logger, gemApp, index)
{
  this->loadModule("amc", "amc v1.0.1");
  toolbox::net::URN hwCfgURN("urn:gem:hw:"+board_domain_name);
  DEBUG("DaqMonitor::DaqMonitor:: infospace " << hwCfgURN.toString() << " does not exist, creating");
  is_daqmon =  is_toolbox_ptr(new gem::base::utils::GEMInfoSpaceToolBox(p_gemApp,
                                                                        xdata::getInfoSpaceFactory()->get(hwCfgURN.toString()),
                                                                        true));
  addInfoSpace("DAQ_MONITORING", is_daqmon, toolbox::TimeInterval(1,  0));
  setupDaqMonitoring();
  updateMonitorables();
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
  std::vector<std::array<std::string,5>> daqlist;
  daqlist.push_back({{"DAQ_ENABLE","YES","NO","success","warning"}});
  daqlist.push_back({{"DAQ_LINK_READY","YES","NO","success","warning"}});
  daqlist.push_back({{"DAQ_LINK_AFULL","YES","NO","warning","success"}});
  daqlist.push_back({{"DAQ_OFIFO_HAD_OFLOW","YES","NO","danger","success"}});
  daqlist.push_back({{"L1A_OFIFO_HAD_OFLOW","YES","NO","danger","success"}});

  int val = 0;
  //auto daqmon_is = m_infoSpaceMap.find("DAQ_MONITORING")->first;
  //auto daqmon_is = m_infoSpaceMap.find("DAQ_MONITORING")->second.first->infoSpace;
  //auto monsets = m_infoSpaceMonitorableSetMap.find("DAQ_MONITORING")->second;
  //auto monset  = m_monitorableSetsMap.find("DAQ_MAIN")->second;

  *out << "<table align=\"center\" class=\"table table-bordered table-condensed\" style=\"width:100%\">" << std::endl;
  for (auto daq: daqlist) {
    *out << "    <tr>" << std::endl;
    *out << "    <td style=\"width:10%\">"<< daq[0] << "</td>" << std::endl;
    val = is_daqmon->getInteger(daq[0]);
    if (val>0) {
      *out << "<td><span class=\"label label-" << daq[3] << "\">" << daq[1] << "</span></td>\"" << std::endl;
    } else {
      *out << "<td><span class=\"label label-" << daq[4] << "\">" << daq[2] << "</span></td>\"" << std::endl;
    }
    *out << "    </tr>" << std::endl;
  }

  *out << "</table>" << std::endl;
}

void gem::daqmon::DaqMonitor::buildMonitorPage(xgi::Output* out)
{
  *out << "<div class=\"col-lg-6\">" << std::endl;
  *out << "<div class=\"panel panel-default\">" << std::endl;
  *out << "<div class=\"panel-heading\">" << std::endl;
  *out << "<h4 align=\"center\">" << std::endl;
  *out << "DAQ" << std::endl;
  *out << "</h4>" << std::endl;
  //FIXME add IEMASK later
  buildDAQmainTable(out);
  //out << "<small>" << std::endl;
  //out << "<table align=\"center\" class=\"table table-bordered table-condensed\" style=\"width:100%\">" << std::endl;

  *out << "</div>" << std::endl; // end panel head
  // There could be a panel body here
  *out << "</div>" << std::endl; // end panel
  // There could be other elements in the column...
  *out << "</div>" << std::endl; // end column


/*
  *out << "{% for daqoh in daqohlist %}" << std::endl;
  *out << "    <tr>" << std::endl;
  *out << "    <td style=\"width:10%\">{{daqoh.0}}</td>" << std::endl;
  *out << "    {{daqoh.1}}" << std::endl;
  *out << "    </tr>" << std::endl;
  *out << "{% endfor %}" << std::endl;
  *out << "</table>" << std::endl;
  *out << "</small>" << std::endl;
*/
}
