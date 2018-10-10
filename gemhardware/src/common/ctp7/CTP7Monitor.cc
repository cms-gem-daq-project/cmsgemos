/**
 * class: CTP7Monitor
 * description: Monitor application for CTP7 cards
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date:
 */

#include "gem/hw/ctp7/HwCTP7.h"

#include "gem/hw/ctp7/CTP7Monitor.h"
#include "gem/hw/ctp7/CTP7Manager.h"
#include "gem/base/GEMApplication.h"
#include "gem/base/GEMFSMApplication.h"

typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

gem::hw::ctp7::CTP7Monitor::CTP7Monitor(std::shared_ptr<HwCTP7> ctp7, CTP7Manager* ctp7Manager, int const& index) :
  GEMMonitor(ctp7Manager->getApplicationLogger(), static_cast<xdaq::Application*>(ctp7Manager), index),
  p_ctp7(ctp7)
{
  // application info space is added in the base class constructor
  // addInfoSpace("Application", ctp7Manager->getApplicationInfoSpace());
  // addInfoSpace("CTP7", p_ctp7->getHwInfoSpace());

  // if (!p_ctp7->getHwInfoSpace()->hasItem(monname))
  //   p_ctp7->getHwInfoSpace()->fireItemAvailable(monname, &monvar);

  updateMonitorables();
}


void gem::hw::ctp7::CTP7Monitor::setupHwMonitoring()
{
  // create the values to be monitored in the info space
  addMonitorableSet("SYSTEM", "HWMonitoring");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("BOARD_ID", "CTP7.SYSTEM.BOARD_ID"),
                 GEMUpdateType::NOUPDATE, "id");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("SYSTEM_ID", "CTP7.SYSTEM.SYSTEM_ID"),
                 GEMUpdateType::NOUPDATE, "id");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("FIRMWARE_ID", "CTP7.SYSTEM.FIRMWARE.ID"),
                 GEMUpdateType::NOUPDATE, "fwver");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("FIRMWARE_DATE", "CTP7.SYSTEM.FIRMWARE.DATE"),
                 GEMUpdateType::NOUPDATE, "date");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("IP_ADDRESS", "CTP7.SYSTEM.IP_INFO"),
                 GEMUpdateType::NOUPDATE, "ip");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("MAC_ADDRESS", "CTP7.SYSTEM.MAC"),
                 GEMUpdateType::NOUPDATE, "mac");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("SFP1_STATUS", "CTP7.SYSTEM.STATUS.SFP1.STATUS"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("SFP2_STATUS", "CTP7.SYSTEM.STATUS.SFP2.STATUS"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("SFP3_STATUS", "CTP7.SYSTEM.STATUS.SFP3.STATUS"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("SFP4_STATUS", "CTP7.SYSTEM.STATUS.SFP4.STATUS"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("FMC1_STATUS", "CTP7.SYSTEM.STATUS.FMC1_PRESENT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("FMC2_STATUS", "CTP7.SYSTEM.STATUS.FMC2_PRESENT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("FPGA_RESET", "CTP7.SYSTEM.STATUS.FPGA_RESET"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("GBE_INT",  "CTP7.SYSTEM.STATUS.GBE_INT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("V6_CPLD",  "CTP7.SYSTEM.STATUS.V6_CPLD"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("CPLD_LOCK", "CTP7.SYSTEM.STATUS.CDCE_LOCK"),
                 GEMUpdateType::HW32, "hex");


  addMonitorableSet("IPBus", "HWMonitoring");
  addMonitorable("IPBus", "HWMonitoring",
                 std::make_pair("OptoHybrid_0", "CTP7.COUNTERS.IPBus"),
                 GEMUpdateType::I2CSTAT, "i2c/hex");
  addMonitorable("IPBus", "HWMonitoring",
                 std::make_pair("OptoHybrid_1", "CTP7.COUNTERS.IPBus"),
                 GEMUpdateType::I2CSTAT, "i2c/hex");
  addMonitorable("IPBus", "HWMonitoring",
                 std::make_pair("TRK_0", "CTP7.COUNTERS.IPBus"),
                 GEMUpdateType::I2CSTAT, "i2c/hex");
  addMonitorable("IPBus", "HWMonitoring",
                 std::make_pair("TRK_1", "CTP7.COUNTERS.IPBus"),
                 GEMUpdateType::I2CSTAT, "i2c/hex");
  addMonitorable("IPBus", "HWMonitoring",
                 std::make_pair("Counters", "CTP7.COUNTERS.IPBus"),
                 GEMUpdateType::I2CSTAT, "i2c/hex");

  addMonitorableSet("GTX_LINKS", "HWMonitoring");
  addMonitorable("GTX_LINKS", "HWMonitoring",
                 std::make_pair("GTX0_TRK_ERR", "CTP7.COUNTERS.GTX0.TRK_ERR"),
                 GEMUpdateType::PROCESS, "raw/rate");
  addMonitorable("GTX_LINKS", "HWMonitoring",
                 std::make_pair("GTX0_TRG_ERR", "CTP7.COUNTERS.GTX0.TRG_ERR"),
                 GEMUpdateType::PROCESS, "raw/rate");
  addMonitorable("GTX_LINKS", "HWMonitoring",
                 std::make_pair("GTX0_DATA_Packets", "CTP7.COUNTERS.GTX0.DATA_Packets"),
                 GEMUpdateType::PROCESS, "raw/rate");
  addMonitorable("GTX_LINKS", "HWMonitoring",
                 std::make_pair("GTX1_TRK_ERR", "CTP7.COUNTERS.GTX1.TRK_ERR"),
                 GEMUpdateType::PROCESS, "raw/rate");
  addMonitorable("GTX_LINKS", "HWMonitoring",
                 std::make_pair("GTX1_TRG_ERR", "CTP7.COUNTERS.GTX1.TRG_ERR"),
                 GEMUpdateType::PROCESS, "raw/rate");
  addMonitorable("GTX_LINKS", "HWMonitoring",
                 std::make_pair("GTX1_DATA_Packets", "CTP7.COUNTERS.GTX1.DATA_Packets"),
                 GEMUpdateType::PROCESS, "raw/rate");

  addMonitorableSet("COUNTERS", "HWMonitoring");
  addMonitorable("COUNTERS", "HWMonitoring",
                 std::make_pair("L1A", "CTP7.COUNTERS.T1.L1A"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("COUNTERS", "HWMonitoring",
                 std::make_pair("CalPulse", "CTP7.COUNTERS.T1.CalPulse"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("COUNTERS", "HWMonitoring",
                 std::make_pair("Resync", "CTP7.COUNTERS.T1.Resync"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("COUNTERS", "HWMonitoring",
                 std::make_pair("BC0", "CTP7.COUNTERS.T1.BC0"),
                 GEMUpdateType::HW32, "hex");

  addMonitorableSet("DAQ", "HWMonitoring");
  addMonitorable("DAQ", "HWMonitoring",
                 std::make_pair("CONTROL", "CTP7.DAQ.CONTROL"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ", "HWMonitoring",
                 std::make_pair("STATUS", "CTP7.DAQ.STATUS"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ", "HWMonitoring",
                 std::make_pair("NOTINTABLE_ERR", "CTP7.DAQ.EXT_STATUS.NOTINTABLE_ERR"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ", "HWMonitoring",
                 std::make_pair("DISPER_ERR", "CTP7.DAQ.EXT_STATUS.DISPER_ERR"),
                 GEMUpdateType::HW32, "hex");
  /*
  addMonitorable("DAQ", "HWMonitoring",
                 std::make_pair("INPUT_KILL_MASK", "CTP7.DAQ.CONTROL.INPUT_KILL_MASK"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ", "HWMonitoring",
                 std::make_pair("DAV_TIMEOUT", "CTP7.DAQ.CONTROL.DAV_TIMEOUT"),
                 GEMUpdateType::HW32, "hex");
  */
  addMonitorable("DAQ", "HWMonitoring",
                 std::make_pair("EVT_SENT", "CTP7.DAQ.EXT_STATUS.EVT_SENT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ", "HWMonitoring",
                 std::make_pair("L1AID", "CTP7.DAQ.EXT_STATUS.L1AID"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ", "HWMonitoring",
                 std::make_pair("MAX_DAV_TIMER", "CTP7.DAQ.EXT_STATUS.MAX_DAV_TIMER"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ", "HWMonitoring",
                 std::make_pair("LAST_DAV_TIMER", "CTP7.DAQ.EXT_STATUS.LAST_DAV_TIMER"),
                 GEMUpdateType::HW32, "hex");

  addMonitorable("DAQ", "HWMonitoring",
                 std::make_pair("GTX0_DAQ_STATUS", "CTP7.DAQ.GTX0.STATUS"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ", "HWMonitoring",
                 std::make_pair("GTX0_DAQ_CORRUPT_VFAT_BLK_CNT", "CTP7.DAQ.GTX0.COUNTERS.CORRUPT_VFAT_BLK_CNT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ", "HWMonitoring",
                 std::make_pair("GTX0_DAQ_EVN", "CTP7.DAQ.GTX0.COUNTERS.EVN"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ", "HWMonitoring",
                 std::make_pair("GTX1_DAQ_STATUS", "CTP7.DAQ.GTX1.STATUS"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ", "HWMonitoring",
                 std::make_pair("GTX1_DAQ_CORRUPT_VFAT_BLK_CNT", "CTP7.DAQ.GTX1.COUNTERS.CORRUPT_VFAT_BLK_CNT"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DAQ", "HWMonitoring",
                 std::make_pair("GTX1_DAQ_EVN", "CTP7.DAQ.GTX1.COUNTERS.EVN"),
                 GEMUpdateType::HW32, "hex");

  addMonitorableSet("TTC", "HWMonitoring");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("TTC_CONTROL", "CTP7.TTC.CONTROL"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("TTC", "HWMonitoring",
                 std::make_pair("TTC_SPY", "CTP7.TTC.SPY"),
                 GEMUpdateType::HW32, "hex");
  updateMonitorables();
}

gem::hw::ctp7::CTP7Monitor::~CTP7Monitor()
{

}

void gem::hw::ctp7::CTP7Monitor::updateMonitorables()
{
  // define how to update the desired values
  // get SYSTEM monitorables
  // can this be split into two loops, one just to do a list read, the second to fill the InfoSpace with the returned values
  CMSGEMOS_DEBUG("CTP7Monitor: Updating monitorables");
  for (auto monlist = m_monitorableSetsMap.begin(); monlist != m_monitorableSetsMap.end(); ++monlist) {
    CMSGEMOS_DEBUG("CTP7Monitor: Updating monitorables in set " << monlist->first);
    for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
      CMSGEMOS_DEBUG("CTP7Monitor: Updating monitorable " << monitem->first);
      std::stringstream regName;
      regName << monitem->second.regname;
      uint32_t address = p_ctp7->getNode(regName.str()).getAddress();
      uint32_t mask    = p_ctp7->getNode(regName.str()).getMask();
      if (monitem->second.updatetype == GEMUpdateType::HW8) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_ctp7->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::HW16) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_ctp7->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::HW24) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_ctp7->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::HW32) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_ctp7->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::HW64) {
        address = p_ctp7->getNode(regName.str()+".LOWER").getAddress();
        mask    = p_ctp7->getNode(regName.str()+".LOWER").getMask();
        uint32_t lower = p_ctp7->readReg(address,mask);
        address = p_ctp7->getNode(regName.str()+".UPPER").getAddress();
        mask    = p_ctp7->getNode(regName.str()+".UPPER").getMask();
        uint32_t upper = p_ctp7->readReg(address,mask);
        (monitem->second.infoSpace)->setUInt64(monitem->first, (((uint64_t)upper) << 32) + lower);
      } else if (monitem->second.updatetype == GEMUpdateType::I2CSTAT) {
        std::stringstream strobeReg;
        strobeReg << regName.str() << ".Strobe." << monitem->first;
        address = p_ctp7->getNode(strobeReg.str()).getAddress();
        mask    = p_ctp7->getNode(strobeReg.str()).getMask();
        uint32_t strobe = p_ctp7->readReg(address,mask);
        std::stringstream ackReg;
        ackReg << regName.str() << ".Ack." << monitem->first;
        address = p_ctp7->getNode(ackReg.str()).getAddress();
        mask    = p_ctp7->getNode(ackReg.str()).getMask();
        uint32_t ack = p_ctp7->readReg(address,mask);
        (monitem->second.infoSpace)->setUInt64(monitem->first, (((uint64_t)ack) << 32) + strobe);
      } else if (monitem->second.updatetype == GEMUpdateType::PROCESS) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_ctp7->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::TRACKER) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_ctp7->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::NOUPDATE) {
        continue;
      } else {
        CMSGEMOS_ERROR("CTP7Monitor: Unknown update type encountered");
        continue;
      }
    } // end loop over items in list
  } // end loop over monitorableSets
}

void gem::hw::ctp7::CTP7Monitor::buildMonitorPage(xgi::Output* out)
{
  CMSGEMOS_DEBUG("CTP7Monitor::buildMonitorPage");
  if (m_infoSpaceMonitorableSetMap.find("HWMonitoring") == m_infoSpaceMonitorableSetMap.end()) {
    CMSGEMOS_WARN("Unable to find item set HWMonitoring in monitor");
    return;
  }

  auto monsets = m_infoSpaceMonitorableSetMap.find("HWMonitoring")->second;

  // loop over the list of monitor sets and grab the monitorables from each one
  // create a div tab for each set, and a table for each set of values
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  for (auto monset = monsets.begin(); monset != monsets.end(); ++monset) {
    *out << "<div class=\"xdaq-tab\" title=\""  << *monset << "\" >"  << std::endl;
    *out << "<table class=\"xdaq-table\" id=\"" << *monset << "_table\">" << std::endl
         << cgicc::thead() << std::endl
         << cgicc::tr()    << std::endl //open
         << cgicc::th()    << "Register name"    << cgicc::th() << std::endl
         << cgicc::th()    << "Value"            << cgicc::th() << std::endl
         << cgicc::th()    << "Register address" << cgicc::th() << std::endl
         << cgicc::th()    << "Description"      << cgicc::th() << std::endl
         << cgicc::tr()    << std::endl //close
         << cgicc::thead() << std::endl
         << "<tbody>" << std::endl;

    for (auto monitem = m_monitorableSetsMap.find(*monset)->second.begin();
         monitem != m_monitorableSetsMap.find(*monset)->second.end(); ++monitem) {
      *out << "<tr>"    << std::endl;

      *out << "<td>"    << std::endl
           << monitem->first
           << "</td>"   << std::endl;

      CMSGEMOS_DEBUG("CTP7Monitor::" << monitem->first << " formatted to "
            << (monitem->second.infoSpace)->getFormattedItem(monitem->first,monitem->second.format));
      //this will be repeated for every CTP7Monitor in the CTP7Manager..., need a better unique ID
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
         << "</div>"    << std::endl;
  }
  *out << "</div>"  << std::endl;
}

void gem::hw::ctp7::CTP7Monitor::reset()
{
  //have to get rid of the timer
  CMSGEMOS_DEBUG("GEMMonitor::reset");
  for (auto infoSpace = m_infoSpaceMap.begin(); infoSpace != m_infoSpaceMap.end(); ++infoSpace) {
    CMSGEMOS_DEBUG("CTP7Monitor::reset removing " << infoSpace->first << " from p_timer");
    try {
      p_timer->remove(infoSpace->first);
    } catch (toolbox::task::exception::Exception& te) {
      CMSGEMOS_ERROR("CTP7Monitor::Caught exception while removing timer task " << infoSpace->first << " " << te.what());
    }
  }
  stopMonitoring();
  CMSGEMOS_DEBUG("GEMMonitor::reset removing timer " << m_timerName << " from timerFactory");
  try {
    toolbox::task::getTimerFactory()->removeTimer(m_timerName);
  } catch (toolbox::task::exception::Exception& te) {
    CMSGEMOS_ERROR("CTP7Monitor::Caught exception while removing timer " << m_timerName << " " << te.what());
  }

  CMSGEMOS_DEBUG("CTP7Monitor::reset - clearing all maps");
  m_infoSpaceMap.clear();
  m_infoSpaceMonitorableSetMap.clear();
  m_monitorableSetInfoSpaceMap.clear();
  m_monitorableSetsMap.clear();
}
