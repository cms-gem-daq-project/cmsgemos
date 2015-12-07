/**
 * class: GEMSupervisorMonitor
 * description: Monitor application for GEMSupervisor application
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date: 
 */

#include "gem/supervisor/GEMSupervisorMonitor.h"
#include "gem/supervisor/GEMSupervisor.h"
#include "gem/base/GEMFSMApplication.h"
//#include "xdata/InfoSpaceFactory.h"
//
//#include "gem/base/utils/GEMInfoSpaceToolBox.h"

typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

gem::supervisor::GEMSupervisorMonitor::GEMSupervisorMonitor(GEMSupervisor* gemSupervisor) :
  GEMMonitor(gemSupervisor->getApplicationLogger(), static_cast<gem::base::GEMFSMApplication*>(gemSupervisor), 0)
{
}

gem::supervisor::GEMSupervisorMonitor::~GEMSupervisorMonitor()
{

}

void gem::supervisor::GEMSupervisorMonitor::setupAppStateMonitoring()
{
  DEBUG("GEMSupervisorMonitor::setupAppStateMonitoring");
  // create the values to be monitored in the info space
  addMonitorableSet("AppStates", "AppStateMonitoring");
  // loop over all supervised applications and 
  std::vector<xdaq::ApplicationDescriptor*> managedApps =
    dynamic_cast<gem::supervisor::GEMSupervisor*>(p_gemApp)->getSupervisedAppDescriptors();
  for (auto managedApp = managedApps.begin(); managedApp != managedApps.end(); ++managedApp) {
    std::stringstream appNameID;
    appNameID << (*managedApp)->getClassName() << "-lid:" << (*managedApp)->getLocalId();
    std::stringstream appURN;
    appURN << (*managedApp)->getURN();
    DEBUG("GEMSupervisorMonitor::setupAppStateMonitoring adding monitored app "
          << appNameID.str() << " with source URN " << appURN.str() << " to AppStates in info space AppStateMonitoring");
    addMonitorable("AppStates", "AppStateMonitoring",
                   std::make_pair(appNameID.str(), appURN.str()),
                   GEMUpdateType::PROCESS, "");
  }
}

void gem::supervisor::GEMSupervisorMonitor::updateMonitorables()
{

  DEBUG("GEMSupervisorMonitor: Updating AppStates monitorables");
  auto monlist = m_monitorableSetsMap.find("AppStates");
  if (monlist == m_monitorableSetsMap.end()) {
    ERROR("Unable to find AppStates in monitor");
    return;
  }
  DEBUG("GEMSupervisorMonitor: Updating monitorables in set " << monlist->first);
  for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
    DEBUG("GEMSupervisorMonitor: Updating monitorable " << monitem->first);
    if (monitem->second.updatetype == GEMUpdateType::PROCESS) {
      std::string state;
      try {
        xdata::InfoSpace* is = xdata::getInfoSpaceFactory()->get(monitem->second.regname);
        DEBUG("infospace: " << is->name() << " has item FSMState " << is->hasItem("FSMState"));
        state = gem::base::utils::GEMInfoSpaceToolBox::getString(is,"FSMState");
      } catch (xdata::exception::Exception const& err) {
        std::string msg = "Error trying to read from InfoSpace '" + monitem->second.regname + "' state " + state + ".";
        ERROR(msg << " " << err.what());
        XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
        return;
      } catch (std::exception const& err) {
        std::string msg = "Error trying to read from InfoSpace '" + monitem->second.regname + "' state " + state + ".";
        ERROR(msg << " " << err.what());
        XCEPT_RAISE(gem::base::utils::exception::InfoSpaceProblem, msg);
        return;
      }
      DEBUG("GEMSupervisorMonitor::updating "
            << monitem->first << " with state " << state);
      (monitem->second.infoSpace)->setString(monitem->first,state);
    }
  }
}

void gem::supervisor::GEMSupervisorMonitor::buildStateTable(xgi::Output* out)
{
  DEBUG("GEMSupervisorMonitor::buildStateTable");
  *out << "<table class=\"xdaq-table\">" << std::endl
       << cgicc::thead() << std::endl
       << cgicc::tr()    << std::endl //open
       << cgicc::th()    << "Application Class (local ID)" << cgicc::th() << std::endl
       << cgicc::th()    << "State" << cgicc::th() << std::endl
       << cgicc::tr()    << std::endl //close
       << cgicc::thead() << std::endl 
    
       << "<tbody>" << std::endl;
  
  for (auto monitem = m_monitorableSetsMap.find("AppStates")->second.begin();
       monitem != m_monitorableSetsMap.find("AppStates")->second.end(); ++monitem) {
    DEBUG("GEMSupervisorMonitor::buildStateTable " << monitem->first);
    *out << "<tr>"    << std::endl;
    
    *out << "<td>"    << std::endl
         << cgicc::h3()
         << monitem->first
         << cgicc::h3()
         << "</td>"   << std::endl;
    
    DEBUG(monitem->first << " formatted to "
          << (monitem->second.infoSpace)->getFormattedItem(monitem->first,monitem->second.format));
    *out << "<td id=\"" << monitem->second.infoSpace->name() << "-" << monitem->first << "\">" << std::endl
         << cgicc::h3()
         << (monitem->second.infoSpace)->getFormattedItem(monitem->first,monitem->second.format)
         << cgicc::h3()
         << "</td>"   << std::endl;
    *out << "</tr>"   << std::endl;
  }
  *out << "</tbody>"  << std::endl
       << "</table>"  << std::endl;
}
