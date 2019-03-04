/**
 * class: OptoHybridMonitor
 * description: Monitor application for OptoHybrid cards
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date:
 */

#include "gem/hw/optohybrid/HwOptoHybrid.h"

#include <algorithm>
#include <array>
#include <iterator>

#include "gem/hw/optohybrid/OptoHybridMonitor.h"
#include "gem/hw/optohybrid/OptoHybridManager.h"
#include "gem/base/GEMApplication.h"
#include "gem/base/GEMFSMApplication.h"

typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

gem::hw::optohybrid::OptoHybridMonitor::OptoHybridMonitor(std::shared_ptr<HwOptoHybrid> optohybrid,
                                                          OptoHybridManager* optohybridManager,
                                                          int const& index) :
  GEMMonitor(optohybridManager->getApplicationLogger(), static_cast<xdaq::Application*>(optohybridManager), index),
  p_optohybrid(optohybrid)
{
  // application info space is added in the base class constructor
  // addInfoSpace("Application", optohybridManager->getApplicationInfoSpace());
  // addInfoSpace("OptoHybrid", p_optohybrid->getHwInfoSpace());

  // if (!p_optohybrid->getHwInfoSpace()->hasItem(monname))
  //   p_optohybrid->getHwInfoSpace()->fireItemAvailable(monname, &monvar);

  updateMonitorables();
}


void gem::hw::optohybrid::OptoHybridMonitor::setupHwMonitoring()
{
  // create the values to be monitored in the info space
  addMonitorableSet("Status and Control", "HWMonitoring");
  addMonitorable("Status and Control", "HWMonitoring",
                 std::make_pair("VFAT_Mask", "VFAT_MASK"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("Status and Control", "HWMonitoring",
                 std::make_pair("SBit_Mask", "FPGA.TRIG.CTRL.VFAT_MASK"),
                 GEMUpdateType::HW32, "hex");
  // addMonitorable("Status and Control", "HWMonitoring",
  //                std::make_pair("HDMI SBitsOut", "CONTROL.HDMI_OUTPUT.SBITS"),
  //                GEMUpdateType::HW32, "hex");
  // addMonitorable("Status and Control", "HWMonitoring",
  //                std::make_pair("HDMI SBitMode", "CONTROL.HDMI_OUTPUT.SBIT_MODE"),
  //                GEMUpdateType::HW32, "hex");
  addMonitorable("Status and Control", "HWMonitoring",
                 std::make_pair("CLK_STATUS", "FPGA.CLOCKING"),
                 GEMUpdateType::HW32, "bit");

  addMonitorable("Status and Control", "HWMonitoring",
                 std::make_pair("FIRMWARE_DATE", "FPGA.CONTROL.RELEASE.DATE"),
                 GEMUpdateType::HW32, "dateoh");
  addMonitorable("Status and Control", "HWMonitoring",
                 std::make_pair("FIRMWARE_VERSION", "FPGA.CONTROL.RELEASE.VERSION"),
                 GEMUpdateType::HW32, "fwveroh");


  // FIXME addresses need update, montoring needs revision
  addMonitorableSet("TTC Counters", "HWMonitoring");
  std::array<std::string, 2> ttcsources = {{"GBT_TTC","LOCAL_TTC"}};
  for (auto const& ttcsrc : ttcsources) {
    addMonitorable("TTC Counters", "HWMonitoring",
                   std::make_pair(ttcsrc+"L1A", "COUNTERS.TTC."+ttcsrc+".L1A"),
                   GEMUpdateType::HW32, "dec");
    addMonitorable("TTC Counters", "HWMonitoring",
                   std::make_pair(ttcsrc+"Resync", "COUNTERS.TTC."+ttcsrc+".Resync"),
                   GEMUpdateType::HW32, "dec");
    addMonitorable("TTC Counters", "HWMonitoring",
                   std::make_pair(ttcsrc+"BC0", "COUNTERS.TTC."+ttcsrc+".BC0"),
                   GEMUpdateType::HW32, "dec");
    addMonitorable("TTC Counters", "HWMonitoring",
                   std::make_pair(ttcsrc+"BXN", "COUNTERS.TTC."+ttcsrc+".BXN"),
                   GEMUpdateType::HW32, "dec");
  }

  addMonitorableSet("Other Counters", "HWMonitoring");
  addMonitorable("Other Counters", "HWMonitoring",
                 std::make_pair("GTX_TrackingLinkErrors","COUNTERS.GTX_LINK.TRK_ERR"),
                 GEMUpdateType::PROCESS, "raw/rate");
  addMonitorable("Other Counters", "HWMonitoring",
                 std::make_pair("GTX_TriggerLinkErrors", "COUNTERS.GTX_LINK.TRG_ERR"),
                 GEMUpdateType::PROCESS, "raw/rate");
  addMonitorable("Other Counters", "HWMonitoring",
                 std::make_pair("GTX_DataPackets",       "COUNTERS.GTX_LINK.DATA_Packets"),
                 GEMUpdateType::PROCESS, "raw/rate");

  addMonitorable("Other Counters", "HWMonitoring",
                 std::make_pair("GBT_TrackingLinkErrors","COUNTERS.GBT_LINK.TRK_ERR"),
                 GEMUpdateType::PROCESS, "raw/rate");
  addMonitorable("Other Counters", "HWMonitoring",
                 std::make_pair("GBT_DataPackets",       "COUNTERS.GBT_LINK.DATA_Packets"),
                 GEMUpdateType::PROCESS, "raw/rate");

  addMonitorableSet("ADC", "HWMonitoring");
  /*
  addMonitorable("ADC", "HWMonitoring",
                 std::make_pair("ADC_CONTROL","ADC.CONTROL"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("ADC", "HWMonitoring",
                 std::make_pair("ADC_SPY",     "ADC.SPY"),
                 GEMUpdateType::HW32, "hex");

  for (int adc = 0; adc < N_ADC_CHANNELS; ++adc) {
    std::stringstream ss;
    ss << "ADC" << adc;
    addMonitorable("ADC", "HWMonitoring",
                   std::make_pair(ss.str(), "OptoHybrid.ADC."+ss.str()),
                   GEMUpdateType::HW32, "hex");
  }
  */

  updateMonitorables();
}

gem::hw::optohybrid::OptoHybridMonitor::~OptoHybridMonitor()
{

}

void gem::hw::optohybrid::OptoHybridMonitor::updateMonitorables()
{
  // define how to update the desired values
  // get SYSTEM monitorables
  // can this be split into two loops, one just to do a list read, the second to fill the InfoSpace with the returned values
  CMSGEMOS_DEBUG("OptoHybridMonitor: Updating monitorables");
  for (auto monlist : m_monitorableSetsMap) {
    CMSGEMOS_DEBUG("OptoHybridMonitor: Updating monitorables in set " << monlist.first);
    for (auto monitem : monlist.second) {
      CMSGEMOS_DEBUG("OptoHybridMonitor: Updating monitorable " << monitem.first);
      std::stringstream regName;
      regName << p_optohybrid->getDeviceBaseNode() << "." << monitem.second.regname;
      uint32_t address = p_optohybrid->getNode(regName.str()).getAddress();
      uint32_t mask    = p_optohybrid->getNode(regName.str()).getMask();
      if (monitem.second.updatetype == GEMUpdateType::HW8) {
        (monitem.second.infoSpace)->setUInt32(monitem.first,p_optohybrid->readReg(address,mask));
      } else if (monitem.second.updatetype == GEMUpdateType::HW16) {
        (monitem.second.infoSpace)->setUInt32(monitem.first,p_optohybrid->readReg(address,mask));
      } else if (monitem.second.updatetype == GEMUpdateType::HW24) {
        (monitem.second.infoSpace)->setUInt32(monitem.first,p_optohybrid->readReg(address,mask));
      } else if (monitem.second.updatetype == GEMUpdateType::HW32) {
        (monitem.second.infoSpace)->setUInt32(monitem.first,p_optohybrid->readReg(address,mask));
      } else if (monitem.second.updatetype == GEMUpdateType::HW64) {
        address = p_optohybrid->getNode(regName.str()+".LOWER").getAddress();
        mask    = p_optohybrid->getNode(regName.str()+".LOWER").getMask();
        uint32_t lower = p_optohybrid->readReg(address,mask);
        address = p_optohybrid->getNode(regName.str()+".UPPER").getAddress();
        mask    = p_optohybrid->getNode(regName.str()+".UPPER").getMask();
        uint32_t upper = p_optohybrid->readReg(address,mask);
        (monitem.second.infoSpace)->setUInt64(monitem.first, (((uint64_t)upper) << 32) + lower);
      } else if (monitem.second.updatetype == GEMUpdateType::I2CSTAT) {
        std::stringstream strobeReg;
        strobeReg << regName.str() << ".Strobe." << monitem.first;
        address = p_optohybrid->getNode(strobeReg.str()).getAddress();
        mask    = p_optohybrid->getNode(strobeReg.str()).getMask();
        uint32_t strobe = p_optohybrid->readReg(address,mask);
        std::stringstream ackReg;
        ackReg << regName.str() << ".Ack." << monitem.first;
        address = p_optohybrid->getNode(ackReg.str()).getAddress();
        mask    = p_optohybrid->getNode(ackReg.str()).getMask();
        uint32_t ack = p_optohybrid->readReg(address,mask);
        (monitem.second.infoSpace)->setUInt64(monitem.first, (((uint64_t)ack) << 32) + strobe);
      } else if (monitem.second.updatetype == GEMUpdateType::PROCESS) {
        (monitem.second.infoSpace)->setUInt32(monitem.first,p_optohybrid->readReg(address,mask));
      } else if (monitem.second.updatetype == GEMUpdateType::TRACKER) {
        (monitem.second.infoSpace)->setUInt32(monitem.first,p_optohybrid->readReg(address,mask));
      } else if (monitem.second.updatetype == GEMUpdateType::NOUPDATE) {
        continue;
      } else {
        CMSGEMOS_ERROR("OptoHybridMonitor: Unknown update type encountered");
        continue;
      }
    } // end loop over items in list
  } // end loop over monitorableSets
}

void gem::hw::optohybrid::OptoHybridMonitor::buildMonitorPage(xgi::Output* out)
{
  CMSGEMOS_DEBUG("OptoHybridMonitor::buildMonitorPage");
  if (m_infoSpaceMonitorableSetMap.find("HWMonitoring") == m_infoSpaceMonitorableSetMap.end()) {
    CMSGEMOS_WARN("Unable to find item set HWMonitoring in monitor");
    return;
  }

  auto monsets = m_infoSpaceMonitorableSetMap.find("HWMonitoring")->second;

  // IMPROVEMENT make the tables dynamically with something like angular/react
  // loop over the list of monitor sets and grab the monitorables from each one
  // create a div tab for each set, and a table for each set of values
  // for I2C request counters, put strobe/ack in separate columns in same table, rows are the specific request
  // for VFAT CRC counters, put valid/invalid in separate columns in same table, rowas are the specific VFAT
  // for TTC counters, put each source in separate columns in same table, rows are the commands
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  for (auto const& monset : monsets) {
    *out << "<div class=\"xdaq-tab\" title=\""  << monset << "\" >"  << std::endl;
    if (monset.rfind("Firmware Scan Controller") == std::string::npos) {
      *out << "<table class=\"xdaq-table\" id=\"" << monset << "_table\">" << std::endl
           << cgicc::thead() << std::endl
           << cgicc::tr()    << std::endl // open
           << cgicc::th()    << "Register name"    << cgicc::th() << std::endl;
      if (monset.rfind("TTC Counters") != std::string::npos) {
        * out << cgicc::th() << "GBT_TTC"   << cgicc::th() << std::endl
              << cgicc::th() << "LOCAL_TTC" << cgicc::th() << std::endl;
      } else if (monset.rfind("Other Counters") != std::string::npos) {
        *out << cgicc::th() << "Count" << cgicc::th() << std::endl
             << cgicc::th() << "Rate"  << cgicc::th() << std::endl;
      } else {
        *out << cgicc::th() << "Value"            << cgicc::th() << std::endl;
      }
      *out << cgicc::th()    << "Register address" << cgicc::th() << std::endl
           << cgicc::th()    << "Description"      << cgicc::th() << std::endl
           << cgicc::tr()    << std::endl // close
           << cgicc::thead() << std::endl
           << "<tbody>" << std::endl;
    }

    if (monset.rfind("TTC Counters") != std::string::npos) {
      buildTTCCounterTable(out);
    } else if (monset.rfind("Other Counters") != std::string::npos) {
      buildOtherCounterTable(out);
    } else {
      for (auto const& monitem : m_monitorableSetsMap.find(monset)->second) {
        *out << "<tr>"    << std::endl;

        std::string formatted = (monitem.second.infoSpace)->getFormattedItem(monitem.first,monitem.second.format);

        *out << "<td>"    << std::endl
             << monitem.first
             << "</td>"   << std::endl;

        CMSGEMOS_DEBUG("OptoHybridMonitor::" << monitem.first << " formatted to "
              << formatted);
        //this will be repeated for every OptoHybridMonitor in the OptoHybridManager..., need a better unique ID
        *out << "<td id=\"" << monitem.second.infoSpace->name() << "-" << monitem.first << "\">" << std::endl
             << formatted
             << "</td>"   << std::endl;

        *out << "<td>"    << std::endl
             << monitem.second.regname
             << "</td>"   << std::endl;

        *out << "<td>"    << std::endl
             << "description"
             << "</td>"   << std::endl;

        *out << "</tr>"   << std::endl;
      }
    }  // end normal register view class

    if (monset.rfind("Firmware Scan Controller") == std::string::npos) {
      *out << "</tbody>"  << std::endl
           << "</table>"  << std::endl;
    }

    *out   << "</div>"    << std::endl;
  }
  *out << "</div>"  << std::endl;

}

void gem::hw::optohybrid::OptoHybridMonitor::buildTTCCounterTable(xgi::Output* out)
{
  CMSGEMOS_DEBUG("OptoHybridMonitor::buildTTCCounterTable");
  if (m_infoSpaceMonitorableSetMap.find("HWMonitoring") == m_infoSpaceMonitorableSetMap.end()) {
    CMSGEMOS_WARN("Unable to find item set HWMonitoring in monitor");
    return;
  }

  auto monsets = m_infoSpaceMonitorableSetMap.find("HWMonitoring")->second;

  if (std::find(monsets.begin(),monsets.end(),"TTC Counters") == monsets.end()) {
    CMSGEMOS_WARN("Unable to find item set 'TTC Counters' in list of HWMonitoring monitor sets");
    return;
  }

  // get the list of pairs of monitorables in the TTC Counters monset
  auto monset = m_monitorableSetsMap.find("TTC Counters")->second;

  std::array<std::string, 2> ttcsources = {{"GBT_TTC","LOCAL_TTC"}};
  std::array<std::string, 4> ttcsignals = {{"L1A","Resync","BC0","BXN"}};

  for (auto const& ttcsignal : ttcsignals) {
    *out << "<tr>"    << std::endl;

    *out << "<td>"    << std::endl
         << ttcsignal
         << "</td>"   << std::endl;

    for (auto const& ttcsource : ttcsources) {
      std::string keyname = (ttcsource)+(ttcsignal);
      for (auto const& monpair : monset) {
        if ((monpair.first).rfind(keyname) == std::string::npos) {
          continue;
        }

        auto monitem = monpair;

        std::string formatted = (monitem.second.infoSpace)->getFormattedItem(monitem.first,
                                                                             monitem.second.format);

        CMSGEMOS_DEBUG("OptoHybridMonitor::" << monitem.first << " formatted to "
              << formatted);

        *out << "<td id=\"" << monitem.second.infoSpace->name() << "-" << monitem.first << "\">" << std::endl
             << formatted
             << "</td>"   << std::endl;
      }
    }
    *out << "<td>"    << std::endl
         << "COUNTERS.TTC.&lt;source&gt;."+(ttcsignal)
         << "</td>"   << std::endl;

    *out << "<td>"    << std::endl
         << "Number of " << ttcsignal << " signals received"
         << "</td>"   << std::endl;

    *out << "</tr>"   << std::endl;
  }
}


void gem::hw::optohybrid::OptoHybridMonitor::buildOtherCounterTable(xgi::Output* out)
{
  CMSGEMOS_DEBUG("OptoHybridMonitor::buildOtherCounterTable");
  if (m_infoSpaceMonitorableSetMap.find("HWMonitoring") == m_infoSpaceMonitorableSetMap.end()) {
    CMSGEMOS_WARN("Unable to find item set HWMonitoring in monitor");
    return;
  }

  auto monsets = m_infoSpaceMonitorableSetMap.find("HWMonitoring")->second;

  if (std::find(monsets.begin(),monsets.end(),"Other Counters") == monsets.end()) {
    CMSGEMOS_WARN("Unable to find item set 'Other Counters' in list of HWMonitoring monitor sets");
    return;
  }

  // get the list of pairs of monitorables in the Other Counters monset
  auto monset = m_monitorableSetsMap.find("Other Counters")->second;

  for (auto const& monitem : monset) {
    *out << "<tr>"    << std::endl;

    std::string formatted = (monitem.second.infoSpace)->getFormattedItem(monitem.first,monitem.second.format);

    *out << "<td>"    << std::endl
         << monitem.first
         << "</td>"   << std::endl;

    CMSGEMOS_DEBUG("OptoHybridMonitor::" << monitem.first << " formatted to "
          << formatted);

    // count
    *out << "<td id=\"" << monitem.second.infoSpace->name() << "-" << monitem.first << "\">" << std::endl
         << formatted
         << "</td>"   << std::endl;

    // rate
    *out << "<td id=\"" << monitem.second.infoSpace->name() << "-" << monitem.first << "\">" << std::endl
         << formatted
         << "</td>"   << std::endl;

    *out << "<td>"    << std::endl
         << monitem.second.regname
         << "</td>"   << std::endl;

    *out << "<td>"    << std::endl
         << "description"
         << "</td>"   << std::endl;

    *out << "</tr>"   << std::endl;
  }
}

void gem::hw::optohybrid::OptoHybridMonitor::reset()
{
  // have to get rid of the timer
  CMSGEMOS_DEBUG("GEMMonitor::reset");
  for (auto infoSpace = m_infoSpaceMap.begin(); infoSpace != m_infoSpaceMap.end(); ++infoSpace) {
    CMSGEMOS_DEBUG("OptoHybridMonitor::reset removing " << infoSpace->first << " from p_timer");
    try {
      p_timer->remove(infoSpace->first);
    } catch (toolbox::task::exception::Exception& te) {
      CMSGEMOS_ERROR("OptoHybridMonitor::Caught exception while removing timer task " << infoSpace->first << " " << te.what());
    }
  }
  stopMonitoring();
  CMSGEMOS_DEBUG("GEMMonitor::reset removing timer " << m_timerName << " from timerFactory");
  try {
    toolbox::task::getTimerFactory()->removeTimer(m_timerName);
  } catch (toolbox::task::exception::Exception& te) {
    CMSGEMOS_ERROR("OptoHybridMonitor::Caught exception while removing timer " << m_timerName << " " << te.what());
  }

  CMSGEMOS_DEBUG("OptoHybridMonitor::reset - clearing all maps");
  m_infoSpaceMap.clear();
  m_infoSpaceMonitorableSetMap.clear();
  m_monitorableSetInfoSpaceMap.clear();
  m_monitorableSetsMap.clear();
}
