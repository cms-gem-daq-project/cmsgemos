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

typedef gem::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

gem::hw::glib::GLIBMonitor::GLIBMonitor(std::shared_ptr<HwGLIB> glib, GLIBManager* glibManager) :
  GEMMonitor(glibManager->getApplicationLogger(), static_cast<gem::base::GEMFSMApplication*>(glibManager)),
  p_glib(glib)
{
  // application info space is added in the base class constructor
  // addInfoSpace("Application", glibManager->getApplicationInfoSpace());
  addInfoSpace("GLIB", p_glib->getHwInfoSpace());

  if (!p_glib->getHwInfoSpace()->hasItem(monname))
    p_glib->getHwInfoSpace()->fireItemAvailable(monname, &monvar);

  // create the values to be monitored in the info space
  addMonitorableSet("SYSTEM", p_glib->getHwInfoSpace());
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair<std::string,std::string>("BOARD_ID",      ""),
                 GEMUpdateType::NOUPDATE, "char");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair<std::string,std::string>("SYSTEM_ID",     ""),
                 GEMUpdateType::NOUPDATE, "char");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair<std::string,std::string>("FIRMWARE_ID",   ""),
                 GEMUpdateType::NOUPDATE, "char");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair<std::string,std::string>("FIRMWARE_DATE", ""),
                 GEMUpdateType::NOUPDATE, "date");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair<std::string,std::string>("IP_ADDRESS",    ""),
                 GEMUpdateType::NOUPDATE, "ip");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair<std::string,std::string>("MAC_ADDRESS",   ""),
                 GEMUpdateType::NOUPDATE, "mac");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair<std::string,std::string>("SFP1_STATUS",   ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair<std::string,std::string>("SFP2_STATUS",   ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair<std::string,std::string>("SFP3_STATUS",   ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair<std::string,std::string>("SFP4_STATUS",   ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair<std::string,std::string>("FMC1_STATUS",   ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair<std::string,std::string>("FMC2_STATUS",   ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair<std::string,std::string>("FPGA_RESET",    ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair<std::string,std::string>("GBE_INT",       ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair<std::string,std::string>("V6_CPLD",       ""),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", p_glib->getHwInfoSpace(), std::make_pair<std::string,std::string>("CPLD_LOCK",     ""),
                 GEMUpdateType::HW32, "hex");
  
}

gem::hw::glib::GLIBMonitor::~GLIBMonitor()
{

}

void gem::hw::glib::GLIBMonitor::updateMonitorables()
{
  // define how to update the desired values
  // get SYSTEM monitorables
  for (auto monset = m_monitorableSetsMap.begin(); monset != m_monitorableSetsMap.end(); ++monset) {
    DEBUG("Updating monitorables in set " << monset->first);
    for (auto monlist = monset->second.begin(); monlist != monset->second.end(); ++monlist) {
      DEBUG("Updating monitorable " << monlist->first);
      if (monlist->second.updatetype == GEMUpdateType::HW8) {
        gem::utils::GEMInfoSpaceToolBox::setUInt32(monlist->second.infoSpace, monlist->first,
                                                   p_glib->readReg(p_glib->getGEMHwInterface().getNode(monlist->first).getAddress()));
      } else if (monlist->second.updatetype == GEMUpdateType::HW16) {
        gem::utils::GEMInfoSpaceToolBox::setUInt32(monlist->second.infoSpace, monlist->first,
                                                   p_glib->readReg(p_glib->getGEMHwInterface().getNode(monlist->first).getAddress()));
        
      } else if (monlist->second.updatetype == GEMUpdateType::HW24) {
        gem::utils::GEMInfoSpaceToolBox::setUInt32(monlist->second.infoSpace, monlist->first,
                                                   p_glib->readReg(p_glib->getGEMHwInterface().getNode(monlist->first).getAddress()));
        
      } else if (monlist->second.updatetype == GEMUpdateType::HW32) {
        gem::utils::GEMInfoSpaceToolBox::setUInt32(monlist->second.infoSpace, monlist->first,
                                                   p_glib->readReg(p_glib->getGEMHwInterface().getNode(monlist->first).getAddress()));
        
      } else if (monlist->second.updatetype == GEMUpdateType::HW64) {
        gem::utils::GEMInfoSpaceToolBox::setUInt32(monlist->second.infoSpace, monlist->first,
                                                   p_glib->readReg(p_glib->getGEMHwInterface().getNode(monlist->first).getAddress()));
        
      } else if (monlist->second.updatetype == GEMUpdateType::PROCESS) {
        gem::utils::GEMInfoSpaceToolBox::setUInt32(monlist->second.infoSpace, monlist->first,
                                                   p_glib->readReg(p_glib->getGEMHwInterface().getNode(monlist->first).getAddress()));
        
      } else if (monlist->second.updatetype == GEMUpdateType::TRACKER) {
        gem::utils::GEMInfoSpaceToolBox::setUInt32(monlist->second.infoSpace, monlist->first,
                                                   p_glib->readReg(p_glib->getGEMHwInterface().getNode(monlist->first).getAddress()));
        
      } else if (monlist->second.updatetype == GEMUpdateType::NOUPDATE) {
        gem::utils::GEMInfoSpaceToolBox::setUInt32(monlist->second.infoSpace, monlist->first,
                                                   p_glib->readReg(p_glib->getGEMHwInterface().getNode(monlist->first).getAddress()));
        
      } else {
        ERROR("Unknown update type encountered");
        continue;
      }
    } // end loop over items in list
  } // end loop over monitorableSets
}

