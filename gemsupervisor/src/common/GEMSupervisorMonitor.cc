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

void gem::supervisor::GEMSupervisorMonitor::updateMonitorables()
{

}

void gem::supervisor::GEMSupervisorMonitor::buildStateTable(xgi::Output* out)
{
  //auto monsets = m_monitorableSetInfoSpaceMap.find("ManagedAppStates")->second;

  try {
    std::vector<xdaq::ApplicationDescriptor*> managedApps =
      std::dynamic_pointer_cast<gem::supervisor::GEMSupervisor>(p_gemApp)->getSupervisedAppDescriptors();
    *out << "<table class=\"xdaq-table\">" << std::endl
         << cgicc::thead() << std::endl
         << cgicc::tr()    << std::endl //open
         << cgicc::th()    << "Application Class (instance)" << cgicc::th() << std::endl
         << cgicc::th()    << "State" << cgicc::th() << std::endl
         << cgicc::tr()    << std::endl //close
         << cgicc::thead() << std::endl 
      
         << "<tbody>" << std::endl;
    
    for (auto managedApp = managedApps.begin(); managedApp != managedApps.end(); ++managedApp) {
      *out << "<tr>"  << std::endl
           << "<td>"  << std::endl
           << cgicc::h3() ;
      //<< dynamic_cast<gem::base::GEMFSMApplication*>(*managedApp)->getURN()
      std::string classname = (*managedApp)->getClassName();
      INFO("managed class name is " << classname);
      *out << classname;
      *out << "(";
      unsigned int instance = (*managedApp)->getInstance();
      INFO("managed class instance is " << instance);
      *out << instance;
      *out << ")"
           << cgicc::h3() << std::endl
           << "</td>"     << std::endl
           << "<td>"      << std::endl
           << cgicc::h3();
      
      INFO("trying to get the FSM class object for object " << std::hex << *managedApp << std::dec);
      std::string classstate
        = gem::base::utils::GEMInfoSpaceToolBox::getString(xdata::getInfoSpaceFactory()->get((*managedApp)->getURN()),"FSMState");
      *out << classstate;
      INFO("managed class FSM state is " << classstate);
      *out << cgicc::h3() << std::endl
           << "</td>"     << std::endl
           << "</tr>"     << std::endl;
    }
    *out << "</tbody>"  << std::endl
         << "</table>"  << std::endl;
  } catch (const xgi::exception::Exception& e) {
    INFO("Something went wrong displaying managed application state table(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception& e) {
    INFO("Something went wrong displaying managed application state table(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }  
}
