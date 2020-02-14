/**
 * class: DaqMonitor
 * description: Monitor application for GEM DAQ
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: M. Dalchenko
 * date:
 */

#include "gem/daqmon/DaqMonitor.h"
#include "gem/hw/utils/GEMCrateUtils.h"
#include <iomanip>
#include <bitset>

typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

//FIXME establish required arguments, eventually retrieve from the config
gem::daqmon::DaqMonitor::DaqMonitor(const std::string& board_domain_name,log4cplus::Logger& logger, gem::base::GEMApplication* gemApp, int const& index):
  gem::base::GEMMonitor::GEMMonitor(logger, gemApp, index),
  //xhal::client::XHALInterface(board_domain_name, logger) //FIXME: if using shared logger, then XHALInterface overtakes everything and logging from XDAQ doesn't go through
  xhal::client::XHALInterface(board_domain_name) //Works as is, providing a bit messy logging, but with all info in place
{
  CMSGEMOS_DEBUG("DaqMonitor::DaqMonitor:: entering constructor");
  if (isConnected) { //TODO Add to the app monitoring space? Need to know in order to mask in web interface the boards which failed to connect
    this->loadModule("daq_monitor", "daq_monitor v1.0.1");
    CMSGEMOS_DEBUG("DaqMonitor::DaqMonitor:: daq_monitor module loaded");
  } else {
    CMSGEMOS_INFO("DaqMonitor::DaqMonitor:: RPC interface failed to connect");
  }
  toolbox::net::URN hwCfgURN("urn:gem:hw:"+board_domain_name);
  CMSGEMOS_DEBUG("DaqMonitor::DaqMonitor:: infospace " << hwCfgURN.toString() << " does not exist, creating");
  is_daqmon =  is_toolbox_ptr(new gem::base::utils::GEMInfoSpaceToolBox(p_gemApp,
                                                                        hwCfgURN.toString(),
                                                                        true));
  addInfoSpace("DAQ_MONITORING", is_daqmon, toolbox::TimeInterval(2,  0));

  setupDaqMonitoring();

  logCnt = 0;

  CMSGEMOS_DEBUG("gem::daqmon::DaqMonitor : constructor done");
}

gem::daqmon::DaqMonitor::~DaqMonitor()
{
  CMSGEMOS_DEBUG("gem::daqmon::DaqMonitor : destructor called");
//TODO
}

void gem::daqmon::DaqMonitor::reconnect()
{
  if (!isConnected){
    this->connect();
    this->loadModule("daq_monitor", "daq_monitor v1.0.1");
  } else {
    CMSGEMOS_ERROR("Interface already connected. Reconnection failed");
    throw xhal::common::utils::XHALRPCException("RPC exception: Interface already connected. Reconnection failed");
  }
}

void gem::daqmon::DaqMonitor::reset()
{
//TODO
}

void gem::daqmon::DaqMonitor::addDaqMonitorable(const std::string& m_name, const std::string& m_monset, const std::string& m_spacename)
{
  //FIXME Putting "DUMMY" as reg full name at the moment. May want to define all tables here and pass as a list to RPC
  is_daqmon->createUInt32(m_name,    0xFFFFFFFF,        NULL, GEMUpdateType::HW32);
  addMonitorable(m_monset, m_spacename,
                 std::make_pair(m_name,"DUMMY"),
                 GEMUpdateType::HW32, "hex");
  m_LabelData.insert(std::make_pair(m_name,new LabelData{m_board_domain_name+"."+m_name, "label label-default", "FFFFFFFF"}));
}

void gem::daqmon::DaqMonitor::setupDaqMonitoring()
{
  // create the values to be monitored in the info space
  addMonitorableSet("DAQ_MAIN","DAQ_MONITORING");
  //DAQ_MAIN monitorables
  v_daq_main = { "DAQ_ENABLE",
                 "DAQ_LINK_READY",
                 "DAQ_LINK_AFULL",
                 "DAQ_OFIFO_HAD_OFLOW",
                 "L1A_FIFO_HAD_OFLOW",
                 "L1A_FIFO_DATA_COUNT",
                 "DAQ_FIFO_DATA_COUNT",
                 "EVENT_SENT",
                 "TTS_STATE",
                 "INPUT_ENABLE_MASK",
                 "INPUT_AUTOKILL_MASK"};
  for (auto monname: v_daq_main) {
    addDaqMonitorable(monname, "DAQ_MAIN", "DAQ_MONITORING");
  }
  //end of DAQ_MAIN monitorables

  addMonitorableSet("DAQ_OH_MAIN","DAQ_MONITORING");
  //DAQ_OH_MAIN monitorables
  v_daq_oh_main = { ".STATUS.EVT_SIZE_ERR",
                    ".STATUS.EVENT_FIFO_HAD_OFLOW",
                    ".STATUS.INPUT_FIFO_HAD_OFLOW",
                    ".STATUS.INPUT_FIFO_HAD_UFLOW",
                    ".STATUS.VFAT_TOO_MANY",
                    ".STATUS.VFAT_NO_MARKER"};
  for (unsigned int i = 0; i < NOH; ++i) {
    for (auto monname: v_daq_oh_main) {
      addDaqMonitorable("OH"+std::to_string(i)+monname, "DAQ_OH_MAIN", "DAQ_MONITORING");
    }
  }
  //end of DAQ_OH_MAIN monitorables

  addMonitorableSet("DAQ_TTC_MAIN","DAQ_MONITORING");
  //DAQ_TTC_MAIN monitorables
  v_daq_ttc_main = { "MMCM_LOCKED",
                     "TTC_SINGLE_ERROR_CNT",
                     "BC0_LOCKED",
                     "L1A_ID",
                     "L1A_RATE"};
  for (auto monname: v_daq_ttc_main) {
    addDaqMonitorable(monname, "DAQ_TTC_MAIN", "DAQ_MONITORING");
  }
  //end of DAQ_TTC_MAIN monitorables

  addMonitorableSet("DAQ_TRIGGER_MAIN","DAQ_MONITORING");
  //DAQ_TRIGGER_MAIN monitorables
  for (unsigned int i = 0; i < NOH; ++i) {
    addDaqMonitorable("OH"+std::to_string(i)+".TRIGGER_RATE", "DAQ_TRIGGER_MAIN", "DAQ_MONITORING");
  }
  //end of DAQ_TRIGGER_MAIN monitorables

  addMonitorableSet("DAQ_TRIGGER_OH_MAIN","DAQ_MONITORING");
  //DAQ_TRIGGER_OH_MAIN monitorables
  v_daq_trigger_oh_main = { ".LINK0_MISSED_COMMA_CNT",
                            ".LINK1_MISSED_COMMA_CNT",
                            ".LINK0_OVERFLOW_CNT",
                            ".LINK1_OVERFLOW_CNT",
                            ".LINK0_UNDERFLOW_CNT",
                            ".LINK1_UNDERFLOW_CNT",
                            ".LINK0_SBIT_OVERFLOW_CNT",
                            ".LINK1_SBIT_OVERFLOW_CNT"};
  for (unsigned int i = 0; i < NOH; ++i) {
    for (auto monname: v_daq_trigger_oh_main) {
      addDaqMonitorable("OH"+std::to_string(i)+monname, "DAQ_TRIGGER_OH_MAIN", "DAQ_MONITORING");
    }
  }
  //end of DAQ_TRIGGER_OH_MAIN monitorables

  addMonitorableSet("OH_MAIN","DAQ_MONITORING");
  //OH_MAIN monitorables
  v_oh_main = { ".FW_VERSION",
                ".EVENT_COUNTER",
                ".EVENT_RATE",
                ".GTX.TRK_ERR",
                ".GTX.TRG_ERR",
                ".GBT.TRK_ERR",
                ".CORR_VFAT_BLK_CNT",
                ".COUNTERS.SEU",
                ".STATUS.SEU"};
  for (unsigned int i = 0; i < NOH; ++i) {
    for (auto monname: v_oh_main) {
      addDaqMonitorable("OH"+std::to_string(i)+monname, "OH_MAIN", "DAQ_MONITORING");
    }
  }
  //end of OH_MAIN monitorables
  addMonitorableSet("OH_SCA","DAQ_MONITORING");
  //OH_SCA monitorables
  v_oh_sca = { ".AVCCN",
               ".AVTTN",
               ".1V0_INT",
               ".1V8F",
               ".1V5",
               ".2V5_IO",
               ".3V0",
               ".1V8",
               ".VTRX_RSSI2",
               ".VTRX_RSSI1",
               ".SCA_TEMP"};
  for (int i = 1; i < 10; ++i) {
    v_oh_sca.push_back(".BOARD_TEMP"+std::to_string(i));
  }
  for (unsigned int i = 0; i < NOH; ++i) {
    for (auto monname: v_oh_sca) {
      addDaqMonitorable("OH"+std::to_string(i)+monname, "OH_SCA", "DAQ_MONITORING");
    }
  }
  //end of OH_SCA monitorables
  addMonitorableSet("OH_Sysmon","DAQ_MONITORING");
  //OH_Sysmon monitorables
  v_oh_sysmon = { ".FPGA_CORE_TEMP",
                  ".FPGA_CORE_1V0",
                  ".FPGA_CORE_2V5_IO"};
  for (unsigned int i = 0; i < NOH; ++i) {
    for (auto monname: v_oh_sysmon) {
      addDaqMonitorable("OH"+std::to_string(i)+monname, "OH_Sysmon", "DAQ_MONITORING");
    }
  }
}

void gem::daqmon::DaqMonitor::updateMonitorables()
{
  CMSGEMOS_DEBUG("DaqMonitor: Updating Monitorables");
  try {
    updateDAQmain();
  } catch (...) {return;} //FIXME Define meaningful exceptions and intercept here or eventually at a different level... If even a updateDAQmain fails, do not attempt to do anything else for this AMC board
  try {
    updateDAQOHmain();
  } catch (...) {} //FIXME Define meaningful exceptions and intercept here or eventually at a different level...
  try {
    updateTTCmain();
  } catch (...) {} //FIXME Define meaningful exceptions and intercept here or eventually at a different level...
  try {
    updateTRIGGERmain();
  } catch (...) {} //FIXME Define meaningful exceptions and intercept here or eventually at a different level...
  try {
    updateTRIGGEROHmain();
  } catch (...) {} //FIXME Define meaningful exceptions and intercept here or eventually at a different level...
  try {
    updateOHmain();
  } catch (...) {} //FIXME Define meaningful exceptions and intercept here or eventually at a different level...
  try {
    updateOHSCA();
  } catch (...) {} //FIXME Define meaningful exceptions and intercept here or eventually at a different level...
  try {
    updateOHSysmon();
  } catch (...) {} //FIXME Define meaningful exceptions and intercept here or eventually at a different level...
  try {
    updateDAQmainTableContent();
    updateTTCmainTableContent();
    updateOHmainTableContent();

    // dump the monitorables every ten updates
    if ( logCnt % 10 == 0) {
      CMSGEMOS_INFO("gem::daqmon::DaqMonitor Logging monitorables");
      time_t t = time(0);   // get time now
      struct tm * now = localtime( & t );
      char buffer [80];
      strftime (buffer,80,"%Y-%m-%d-%H:%M:%S",now);
      std::string logfilename = "/var/log/gemdaq/daqmon_log_";
      logfilename += buffer;
      logfilename += ".gz";

      std::ofstream logfile(logfilename, std::ios_base::out | std::ios_base::binary);
      bo::filtering_ostream m_out;
      m_out.push(bo::gzip_compressor());
      m_out.push(logfile);
      m_out << "Timestamp: " << buffer << std::endl;

      m_out << " { " << std::endl;
      for (auto ld = m_LabelData.begin(); ld != m_LabelData.end();) {
        m_out << "\"" << ld->second->labelId << "\" : { \"class_name\" : \"" << ld->second->labelClass << "\", \"value\" : \"" << ld->second->labelValue << "\" }";
        if (++ld == m_LabelData.end()) m_out << std::endl; else m_out << "," << std::endl;
      }
      m_out << " } " << std::endl;
    }
    ++logCnt;

  } catch (...) {} //FIXME Define meaningful exceptions and intercept here or eventually at a different level...
}

void gem::daqmon::DaqMonitor::updateDAQmain()
{
  CMSGEMOS_DEBUG("DaqMonitor: Update DAQ main table");
  req = wisc::RPCMsg("daq_monitor.getmonDAQmain");
  try {
    rsp = rpc.call_method(req);
  } STANDARD_CATCH;

  try {
    if (rsp.get_key_exists("error")) {
      CMSGEMOS_ERROR("DAQ_MAIN update error: " << rsp.get_string("error").c_str());
      //throw xhal::common::utils::XHALException("DAQ_MAIN update failed");
    }
    auto monlist = m_monitorableSetsMap.find("DAQ_MAIN");
    for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
      (monitem->second.infoSpace)->setUInt32(monitem->first,rsp.get_word(monitem->first));
    }
  } STANDARD_CATCH;
}

void gem::daqmon::DaqMonitor::updateDAQOHmain()
{
  CMSGEMOS_DEBUG("DaqMonitor: Update DAQ OH main table");
  req = wisc::RPCMsg("daq_monitor.getmonDAQOHmain");
  req.set_word("NOH",NOH);
  try {
    rsp = rpc.call_method(req);
  } STANDARD_CATCH;

  try {
    if (rsp.get_key_exists("error")) {
      CMSGEMOS_ERROR("DAQ_OH_MAIN update error: " << rsp.get_string("error").c_str());
      //throw xhal::common::utils::XHALException("DAQ_OH_MAIN update failed");
    }
    auto monlist = m_monitorableSetsMap.find("DAQ_OH_MAIN");
    for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
      (monitem->second.infoSpace)->setUInt32(monitem->first,rsp.get_word(monitem->first));
    }
  } STANDARD_CATCH;
}

void gem::daqmon::DaqMonitor::updateTTCmain()
{
  CMSGEMOS_DEBUG("DaqMonitor: Update TTC main table");
  req = wisc::RPCMsg("daq_monitor.getmonTTCmain");
  try {
    rsp = rpc.call_method(req);
  } STANDARD_CATCH;

  try {
    if (rsp.get_key_exists("error")) {
      CMSGEMOS_ERROR("DAQ_TTC_MAIN update error: " << rsp.get_string("error").c_str());
      //throw xhal::common::utils::XHALException("DAQ_TTC_MAIN update failed");
    }
    auto monlist = m_monitorableSetsMap.find("DAQ_TTC_MAIN");
    for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
      (monitem->second.infoSpace)->setUInt32(monitem->first,rsp.get_word(monitem->first));
    }
  } STANDARD_CATCH;
}

void gem::daqmon::DaqMonitor::updateTRIGGERmain()
{
  CMSGEMOS_DEBUG("DaqMonitor: Update TRIGGER main table");
  req = wisc::RPCMsg("daq_monitor.getmonTRIGGERmain");
  req.set_word("NOH",NOH);
  try {
    rsp = rpc.call_method(req);
  } STANDARD_CATCH;

  try {
    if (rsp.get_key_exists("error")) {
      CMSGEMOS_ERROR("DAQ_TRIGGER_MAIN update error: " << rsp.get_string("error").c_str());
      //throw xhal::common::utils::XHALException("DAQ_TRIGGER_MAIN update failed");
    }
    auto monlist = m_monitorableSetsMap.find("DAQ_TRIGGER_MAIN");
    for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
      (monitem->second.infoSpace)->setUInt32(monitem->first,rsp.get_word(monitem->first));
    }
  } STANDARD_CATCH;
}

void gem::daqmon::DaqMonitor::updateTRIGGEROHmain()
{
  CMSGEMOS_DEBUG("DaqMonitor: Update TRIGGER OH main table");
  req = wisc::RPCMsg("daq_monitor.getmonTRIGGEROHmain");
  req.set_word("NOH",NOH);
  try {
    rsp = rpc.call_method(req);
  } STANDARD_CATCH;

  try {
    if (rsp.get_key_exists("error")) {
      CMSGEMOS_ERROR("DAQ_TRIGGER_OH_MAIN update error: " << rsp.get_string("error").c_str());
      //throw xhal::common::utils::XHALException("DAQ_TRIGGER_OH_MAIN update failed");
    }
    auto monlist = m_monitorableSetsMap.find("DAQ_TRIGGER_OH_MAIN");
    for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
      (monitem->second.infoSpace)->setUInt32(monitem->first,rsp.get_word(monitem->first));
    }
  } STANDARD_CATCH;
}

void gem::daqmon::DaqMonitor::updateOHmain()
{
  CMSGEMOS_DEBUG("DaqMonitor: Update OH main table");
  req = wisc::RPCMsg("daq_monitor.getmonOHmain");
  req.set_word("NOH",NOH);
  try {
    rsp = rpc.call_method(req);
  } STANDARD_CATCH;

  try {
    if (rsp.get_key_exists("error")) {
      CMSGEMOS_ERROR("OH_MAIN update error: " << rsp.get_string("error").c_str());
      //throw xhal::common::utils::XHALException("OH_MAIN update failed");
    }
    auto monlist = m_monitorableSetsMap.find("OH_MAIN");
    for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
      (monitem->second.infoSpace)->setUInt32(monitem->first,rsp.get_word(monitem->first));
    }
  } STANDARD_CATCH;
}


void gem::daqmon::DaqMonitor::updateOHSCA()
{
  CMSGEMOS_DEBUG("DaqMonitor: Update OH SCA table");
  req = wisc::RPCMsg("daq_monitor.getmonOHSCAmain");
  req.set_word("NOH",NOH);
  try {
    rsp = rpc.call_method(req);
  } STANDARD_CATCH;

  try {
    if (rsp.get_key_exists("error")) {
      CMSGEMOS_ERROR("OH_SCA update error: " << rsp.get_string("error").c_str());
      //throw xhal::common::utils::XHALException("OH_SCA update failed"); FIXME instead of throwing an exception there should be an alert propagating
    }
    auto monlist = m_monitorableSetsMap.find("OH_SCA");
    for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
      (monitem->second.infoSpace)->setUInt32(monitem->first,rsp.get_word(monitem->first));
    }
  } STANDARD_CATCH;
}

void gem::daqmon::DaqMonitor::updateOHSysmon()
{
  CMSGEMOS_DEBUG("DaqMonitor: Update OH Sysmon table");
  req = wisc::RPCMsg("daq_monitor.getmonOHSysmon");
  req.set_word("NOH",NOH);
  req.set_word("doReset",0);
  try {
    rsp = rpc.call_method(req);
  } STANDARD_CATCH;

  try {
    if (rsp.get_key_exists("error")) {
      CMSGEMOS_ERROR("OH_Sysmon update error: " << rsp.get_string("error").c_str());
      //throw xhal::common::utils::XHALException("OH_Sysmon update failed"); FIXME instead of throwing an exception there should be an alert propagating
    }
    auto monlist = m_monitorableSetsMap.find("OH_Sysmon");
    for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
      (monitem->second.infoSpace)->setUInt32(monitem->first,rsp.get_word(monitem->first));
    }
  } STANDARD_CATCH;
}

double gem::daqmon::DaqMonitor::scaVconv(uint32_t val)
{
  return static_cast<double>(val*(0.244/1000)*3.0);
}

double gem::daqmon::DaqMonitor::scaTconv(uint32_t val)
{
  return static_cast<double>(val*(-0.53)+381.2);
}

double gem::daqmon::DaqMonitor::scaPTconv(uint32_t val)
{
  return static_cast<double>(0.2597*(9891.8-2.44*val));
}

double gem::daqmon::DaqMonitor::sysmonTconv(uint32_t val)
{
  return static_cast<double>(val*0.49-273.15);
}

double gem::daqmon::DaqMonitor::sysmonVconv(uint32_t val)
{
  return static_cast<double>(val*2.93/1000);
}

void gem::daqmon::DaqMonitor::updateDAQmainTableContent()
{
  CMSGEMOS_DEBUG("DaqMonitor::updateDAQmainTableContent");
  uint32_t val;
  LabelData * ld;
  for (auto monname: {"DAQ_ENABLE","DAQ_LINK_READY"})
  {
    val = is_daqmon->getUInt32(monname);
    ld = m_LabelData.find(monname)->second;
    switch (val) {
      case 0:
        ld->labelValue="N";
        ld->labelClass="label label-warning";
        break;
      case 0xFFFFFFFF:
        ld->labelValue="X";
        ld->labelClass="label label-default";
        break;
      case 0xDEADDEAD:
        ld->labelValue="X";
        ld->labelClass="label label-default";
        break;
      default:
        ld->labelValue="Y";
        ld->labelClass="label label-success";
        break;
    }
  }
  val = is_daqmon->getUInt32("DAQ_LINK_AFULL");
  ld = m_LabelData.find("DAQ_LINK_AFULL")->second;
  switch (val) {
    case 0:
      ld->labelValue="N";
      ld->labelClass="label label-success";
      break;
    case 0xFFFFFFFF:
      ld->labelValue="X";
      ld->labelClass="label label-default";
      break;
    case 0xDEADDEAD:
      ld->labelValue="X";
      ld->labelClass="label label-default";
      break;
    default:
      ld->labelValue="Y";
      ld->labelClass="label label-warning";
      break;
  }
  for (auto monname: {"DAQ_OFIFO_HAD_OFLOW","L1A_FIFO_HAD_OFLOW"})
  {
    val = is_daqmon->getUInt32(monname);
    ld = m_LabelData.find(monname)->second;
    switch (val) {
      case 0:
        ld->labelValue="N";
        ld->labelClass="label label-success";
        break;
      case 0xFFFFFFFF:
        ld->labelValue="X";
        ld->labelClass="label label-default";
        break;
      case 0xDEADDEAD:
        ld->labelValue="X";
        ld->labelClass="label label-default";
        break;
      default:
        ld->labelValue="Y";
        ld->labelClass="label label-danger";
        break;
    }
  }
  for (auto monname: {"L1A_FIFO_DATA_COUNT","DAQ_FIFO_DATA_COUNT","EVENT_SENT"})
  {
    val = is_daqmon->getUInt32(monname);
    ld = m_LabelData.find(monname)->second;
    if (val == 0xFFFFFFFF || val == 0xDEADDEAD) {
      ld->labelValue="X";
      ld->labelClass="label label-default";
    } else {
      ld->labelValue=std::to_string(val);
      ld->labelClass="label label-info";
    }
  }
  val = is_daqmon->getUInt32("TTS_STATE");
  ld = m_LabelData.find("TTS_STATE")->second;
  switch (val) {
    case 1:
      ld->labelValue="BUSY";
      ld->labelClass="label label-warning";
      break;
    case 2:
      ld->labelValue="ERROR";
      ld->labelClass="label label-danger";
      break;
    case 3:
      ld->labelValue="WARN";
      ld->labelClass="label label-warning";
      break;
    case 4:
      ld->labelValue="OOS";
      ld->labelClass="label label-danger";
      break;
    case 8:
      ld->labelValue="READY";
      ld->labelClass="label label-success";
      break;
    default:
      ld->labelValue="NDF";
      ld->labelClass="label label-default";
      break;
  }
  for (auto monname: {"INPUT_ENABLE_MASK","INPUT_AUTOKILL_MASK"})
  {
    val = is_daqmon->getUInt32(monname);
    ld = m_LabelData.find(monname)->second;
    if (val == 0xFFFFFFFF || val == 0xDEADDEAD) {
      ld->labelValue="X";
      ld->labelClass="label label-default";
    } else {
      ld->labelValue=std::bitset<12>(val).to_string();
      ld->labelClass="label label-info";
    }
  }
}

void gem::daqmon::DaqMonitor::updateTTCmainTableContent()
{
  uint32_t val;
  LabelData * ld;
  for (auto monname: {"MMCM_LOCKED","BC0_LOCKED"})
  {
    val = is_daqmon->getUInt32(monname);
    ld = m_LabelData.find(monname)->second;
    switch (val) {
      case 0:
        ld->labelValue="N";
        ld->labelClass="label label-danger";
        break;
      case 0xFFFFFFFF:
        ld->labelValue="X";
        ld->labelClass="label label-default";
        break;
      case 0xDEADDEAD:
        ld->labelValue="X";
        ld->labelClass="label label-default";
        break;
      default:
        ld->labelValue="Y";
        ld->labelClass="label label-success";
        break;
    }
  }
  val = is_daqmon->getUInt32("TTC_SINGLE_ERROR_CNT");
  ld = m_LabelData.find("TTC_SINGLE_ERROR_CNT")->second;
  switch (val) {
    case 0:
      ld->labelValue="0";
      ld->labelClass="label label-success";
      break;
    case 0xFFFFFFFF:
      ld->labelValue="X";
      ld->labelClass="label label-default";
      break;
    case 0xDEADDEAD:
      ld->labelValue="X";
      ld->labelClass="label label-default";
      break;
    default:
      ld->labelValue=std::to_string(val);
      ld->labelClass="label label-danger";
      break;
  }
  for (auto monname: {"L1A_ID","L1A_RATE"})
  {
    val = is_daqmon->getUInt32(monname);
    ld = m_LabelData.find(monname)->second;
    if (val == 0xFFFFFFFF || val == 0xDEADDEAD) {
      ld->labelValue="X";
      ld->labelClass="label label-default";
    } else {
      ld->labelValue=std::to_string(val);
      ld->labelClass="label label-info";
    }
  }
}

void gem::daqmon::DaqMonitor::updateOHmainTableContent()
{
  uint32_t val;
  LabelData * ld;
  for (int i = 0; i < NOH; ++i)
  {
    std::string monname = "OH"+std::to_string(i)+".FW_VERSION";
    val = is_daqmon->getUInt32(monname);
    ld = m_LabelData.find(monname)->second;
    switch (val) {
      case 0xDEADDEAD:
        ld->labelValue="ERROR";
        ld->labelClass="label label-danger";
        break;
      case 0xADADADAD:// erroneous return in case of V3
        ld->labelValue="ERROR";
        ld->labelClass="label label-danger";
        break;
      case 0xFFFFFFFF:
        ld->labelValue="X";
        ld->labelClass="label label-default";
        break;
      default:
        std::stringstream ss;
        ss << std::uppercase << std::setfill('0')
           << std::setw(8) << std::hex << val << std::dec;
        ld->labelValue=ss.str();
        ld->labelClass="label label-info";
        break;
    }
    for (auto monname: {".EVENT_COUNTER",".EVENT_RATE"})
    {
      val = is_daqmon->getUInt32("OH"+std::to_string(i)+monname);
      ld = m_LabelData.find("OH"+std::to_string(i)+monname)->second;
      if (val == 0xFFFFFFFF || val == 0xDEADDEAD) {
        ld->labelValue="X";
        ld->labelClass="label label-default";
      } else {
        ld->labelValue=std::to_string(val);
        ld->labelClass="label label-info";
      }
    }
    std::vector<std::string> v_daq(v_oh_main.begin()+3,v_oh_main.end());
    v_daq.insert(v_daq.end(),v_daq_trigger_oh_main.begin(),v_daq_trigger_oh_main.end());
    for (auto monname: v_daq)
    {
      val = is_daqmon->getUInt32("OH"+std::to_string(i)+monname);
      ld = m_LabelData.find("OH"+std::to_string(i)+monname)->second;
      if (val == 0xFFFFFFFF || val == 0xDEADDEAD) {
        ld->labelValue="X";
        ld->labelClass="label label-default";
      } else {
        if (val == 0){
          ld->labelValue="0";
          ld->labelClass="label label-info";
        } else {
          ld->labelValue=std::to_string(val);
          ld->labelClass="label label-warning";
        }
      }
    }
    for (auto monname: v_daq_oh_main)
    {
      //DEBUG("Monitorable name : " << monname);
      val = is_daqmon->getUInt32("OH"+std::to_string(i)+monname);
      ld = m_LabelData.find("OH"+std::to_string(i)+monname)->second;
      if (val == 0xFFFFFFFF || val == 0xDEADDEAD) {
        ld->labelValue="X";
        ld->labelClass="label label-default";
      } else {
        if (val == 0){
          ld->labelValue="N";
          ld->labelClass="label label-success";
        } else {
          ld->labelValue="Y";
          ld->labelClass="label label-danger";
        }
      }
    }
    for (auto monname: v_oh_sca)
    {
      val = is_daqmon->getUInt32("OH"+std::to_string(i)+monname);
      ld = m_LabelData.find("OH"+std::to_string(i)+monname)->second;
      if (val == 0xFFFFFFFF || val == 0xDEADDEAD) {
        ld->labelValue="X";
        ld->labelClass="label label-default";
      } else {
        ld->labelClass="label label-info";
        std::stringstream ss;
        if (monname.find("SCA_TEMP") != std::string::npos){
          // ss << std::setprecision(2) << scaTconv(val);
          ss << std::setprecision(2)
             << gem::hw::utils::scaConversion(gem::hw::utils::ADC_T,val);
          ld->labelValue=ss.str();
        } else {
          if (monname.find("BOARD_TEMP") != std::string::npos){
            // ss << std::setprecision(2) << scaPTconv(val);
            ss << std::setprecision(2)
               << gem::hw::utils::scaConversion(gem::hw::utils::PT100,val);
            ld->labelValue=ss.str();
          } else {
            // ss << std::setprecision(2) << scaVconv(val);
            ss << std::setprecision(2)
               << gem::hw::utils::scaConversion(gem::hw::utils::ADC_V,val);
            ld->labelValue=ss.str();
          }
        }
      }
    }
    for (auto monname: v_oh_sysmon)
    {
      val = is_daqmon->getUInt32("OH"+std::to_string(i)+monname);
      ld = m_LabelData.find("OH"+std::to_string(i)+monname)->second;
      if (val == 0xFFFFFFFF || val == 0xDEADDEAD) {
        ld->labelValue="X";
        ld->labelClass="label label-default";
      } else {
        ld->labelClass="label label-info";
        std::stringstream ss;
        if (monname.find("TEMP") != std::string::npos){
          // ss << std::setprecision(2) << sysmonTconv(val);
          ss << std::setprecision(2)
             << gem::hw::utils::scaConversion(gem::hw::utils::SYSMON_T,val);
          ld->labelValue=ss.str();
        } else {
          // ss << std::setprecision(2) << sysmonVconv(val);
          ss << std::setprecision(2)
             << gem::hw::utils::scaConversion(gem::hw::utils::SYSMON_V,val);
          ld->labelValue=ss.str();
        }
      }
    }
  }
}

void gem::daqmon::DaqMonitor::buildTable(const std::string& table_name, xgi::Output* out)
{
  CMSGEMOS_DEBUG("DaqMonitor: Build DAQ main table");
  std::vector<std::string> v_daq;
  if (table_name == "DAQ_MAIN") {
    updateDAQmainTableContent();
    v_daq = v_daq_main;
  }
  if (table_name == "DAQ_TTC_MAIN") {
    updateTTCmainTableContent();
    v_daq = v_daq_ttc_main;
  }
  if (table_name == "OH_MAIN") {
    updateOHmainTableContent();
    v_daq = v_oh_main;
    v_daq.insert(v_daq.end(),v_daq_trigger_oh_main.begin(),v_daq_trigger_oh_main.end());
    v_daq.insert(v_daq.end(),v_daq_oh_main.begin(),v_daq_oh_main.end());
    v_daq.insert(v_daq.end(),v_oh_sysmon.begin(),v_oh_sysmon.end());
    v_daq.insert(v_daq.end(),v_oh_sca.begin(),v_oh_sca.end());
  }
  *out << "<font size=\"1\">" << std::endl;
  *out << "<small>" << std::endl;
  *out << "<table align=\"center\" class=\"table table-bordered table-condensed\" style=\"width:100%\">" << std::endl;
  LabelData * ld;
  if (table_name == "OH_MAIN") {
    *out << "    <tr>" << std::endl;
    *out << "    <td style=\"width:10%\">"<< "REGISTER|OH" << "</td>" << std::endl;
    for (int i=0; i<NOH; ++i) {
      *out << "<td>" << std::to_string(i) << "</td>";
    }
    *out << "    </tr>" << std::endl;
  }

  if (table_name == "OH_MAIN") {
    for (auto monname: v_daq){
      *out << "    <tr>" << std::endl;
      *out << "    <td style=\"width:10%\">"<< monname.substr(1) << "</td>" << std::endl;
      for (int i = 0; i < NOH; ++i) {
        ld = m_LabelData.find("OH"+std::to_string(i)+monname)->second;
        *out << "<td><span id=\"" << ld->labelId << "\" class=\"" << ld->labelClass << "\">" << ld->labelValue << "</span></td>" << std::endl;
      }
      *out << "    </tr>" << std::endl;
    }
  } else {
    for (auto monname: v_daq){
      ld = m_LabelData.find(monname)->second;
      *out << "    <tr>" << std::endl;
      *out << "    <td style=\"width:10%\">"<< monname << "</td>" << std::endl;
      *out << "<td><span id=\"" << ld->labelId << "\" class=\"" << ld->labelClass << "\">" << ld->labelValue << "</span></td>" << std::endl;
      *out << "    </tr>" << std::endl;
    }
  }
  *out << "</table>" << std::endl;
  *out << "</small>" << std::endl;
  *out << "</font>" << std::endl;
}

void gem::daqmon::DaqMonitor::buildMonitorPage(xgi::Output* out)
{
  *out << "<div class=\"col-lg-3\">" << std::endl
       << "  <div class=\"panel panel-default\">" << std::endl
       << "    <div class=\"panel-heading\">" << std::endl
       << "      <h4 align=\"center\">" << std::endl
       << "        DAQ" << std::endl
       << "      </h4>" << std::endl;
        //FIXME add IEMASK later
  buildTable("DAQ_MAIN",out);
  *out << "    </div>" << std::endl // end panel head
    // There could be a panel body here
       << "  </div>" << std::endl // end panel
    // There could be other elements in the column...
       << "  <div class=\"panel panel-default\">" << std::endl
       << "    <div class=\"panel-heading\">" << std::endl
       << "      <h4 align=\"center\">" << std::endl
       << "        TTC" << std::endl
       << "      </h4>" << std::endl;
  buildTable("DAQ_TTC_MAIN",out);
  *out << "    </div>" << std::endl // end panel head
    // There could be a panel body here
       << "  </div>" << std::endl // end panel
       << "</div>" << std::endl // end column
       << "<div class=\"col-lg-9\">" << std::endl
       << "  <div class=\"panel panel-default\">" << std::endl
       << "    <div class=\"panel-heading\">" << std::endl
       << "      <h4 align=\"center\">" << std::endl
       << "        OPTICAL LINKS" << std::endl
       << "      </h4>" << std::endl;
        //FIXME add IEMASK later
  buildTable("OH_MAIN",out);
  *out << "    </div>" << std::endl // end panel head
    // There could be a panel body here
       << "  </div>" << std::endl // end panel
    // There could be other elements in the column...
       << "</div>" << std::endl; // end column
}

void gem::daqmon::DaqMonitor::jsonContentUpdate(xgi::Output* out)
{
  CMSGEMOS_DEBUG("DaqMonitor::jsonContentUpdate");
  *out << " { " << std::endl;
  for (auto ld = m_LabelData.begin(); ld != m_LabelData.end();) {
    *out << "\"" << ld->second->labelId << "\" : { \"class_name\" : \"" << ld->second->labelClass << "\", \"value\" : \"" << ld->second->labelValue << "\" }";
    if (++ld == m_LabelData.end()) *out << std::endl; else *out << "," << std::endl;
  }
  *out << " } " << std::endl;

}
