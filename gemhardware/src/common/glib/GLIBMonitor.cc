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

gem::hw::glib::GLIBMonitor::GLIBMonitor(std::shared_ptr<HwGLIB> glib, GLIBManager* glibManager) :
  GEMMonitor(glibManager->getApplicationLogger(), static_cast<gem::base::GEMFSMApplication*>(glibManager)),
  p_glib(glib)
{
  // application info space is added in the base class constructor
  // addInfoSpace("Application", glibManager->getApplicationInfoSpace());
  // addInfoSpace("GLIB", p_glib->getHwInfoSpace());

  // if (!p_glib->getHwInfoSpace()->hasItem(monname))
  //   p_glib->getHwInfoSpace()->fireItemAvailable(monname, &monvar);

  // create the values to be monitored in the info space
  /*
  addMonitorableSet("SYSTEM", p_glib->getHwInfoSpace());
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair("BOARD_ID",      ""),
                 GEMUpdateType::NOUPDATE, "char");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair("SYSTEM_ID",     ""),
                 GEMUpdateType::NOUPDATE, "char");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair("FIRMWARE_ID",   ""),
                 GEMUpdateType::NOUPDATE, "char");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair("FIRMWARE_DATE", ""),
                 GEMUpdateType::NOUPDATE, "date");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair("IP_ADDRESS",    ""),
                 GEMUpdateType::NOUPDATE, "ip");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair("MAC_ADDRESS",   ""),
                 GEMUpdateType::NOUPDATE, "mac");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair("SFP1_STATUS",   ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair("SFP2_STATUS",   ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair("SFP3_STATUS",   ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair("SFP4_STATUS",   ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair("FMC1_STATUS",   ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair("FMC2_STATUS",   ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair("FPGA_RESET",    ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair("GBE_INT",       ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair("V6_CPLD",       ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair("CPLD_LOCK",     ""),
                 GEMUpdateType::HW32, "hex");
  */
}

gem::hw::glib::GLIBMonitor::~GLIBMonitor()
{

}

void gem::hw::glib::GLIBMonitor::updateMonitorables()
{
  // define how to update the desired values
  // get SYSTEM monitorables
  for (auto monlist = m_monitorableSetsMap.begin(); monlist != m_monitorableSetsMap.end(); ++monlist) {
    DEBUG("Updating monitorables in set " << monlist->first);
    for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
      DEBUG("Updating monitorable " << monitem->first);
      if (monitem->second.updatetype == GEMUpdateType::HW8) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,
                                               p_glib->readReg(p_glib->getGEMHwInterface().getNode(monitem->first).getAddress()));
      } else if (monitem->second.updatetype == GEMUpdateType::HW16) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,
                                               p_glib->readReg(p_glib->getGEMHwInterface().getNode(monitem->first).getAddress()));
      } else if (monitem->second.updatetype == GEMUpdateType::HW24) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,
                                               p_glib->readReg(p_glib->getGEMHwInterface().getNode(monitem->first).getAddress()));
      } else if (monitem->second.updatetype == GEMUpdateType::HW32) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,
                                               p_glib->readReg(p_glib->getGEMHwInterface().getNode(monitem->first).getAddress()));
      } else if (monitem->second.updatetype == GEMUpdateType::HW64) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,
                                               p_glib->readReg(p_glib->getGEMHwInterface().getNode(monitem->first).getAddress()));
      } else if (monitem->second.updatetype == GEMUpdateType::PROCESS) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,
                                               p_glib->readReg(p_glib->getGEMHwInterface().getNode(monitem->first).getAddress()));
      } else if (monitem->second.updatetype == GEMUpdateType::TRACKER) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,
                                               p_glib->readReg(p_glib->getGEMHwInterface().getNode(monitem->first).getAddress()));
      } else if (monitem->second.updatetype == GEMUpdateType::NOUPDATE) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,
                                               p_glib->readReg(p_glib->getGEMHwInterface().getNode(monitem->first).getAddress()));
      } else {
        ERROR("Unknown update type encountered");
        continue;
      }
    } // end loop over items in list
  } // end loop over monitorableSets
}

