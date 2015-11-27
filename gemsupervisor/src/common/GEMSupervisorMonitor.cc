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
