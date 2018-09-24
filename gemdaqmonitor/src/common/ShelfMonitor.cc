/**
 * class: GEMSupervisor
 * description: Supervisor application for GEM system
 *              structure borrowed from TCDS core, with nods to HCAL (hcalSupervisor)
 * author: J. Sturdy
 * date:
 */

#include "gem/daqmon/ShelfMonitor.h"

#include <cstdlib>
#include <iomanip>

#include <map>
#include <set>
#include <vector>
#include <algorithm>

#include <boost/algorithm/string.hpp>

#include "gem/daqmon/ShelfMonitorWeb.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"
#include "gem/utils/exception/Exception.h"

typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

XDAQ_INSTANTIATOR_IMPL(gem::daqmon::ShelfMonitor);

gem::daqmon::ShelfMonitor::ShelfMonitor(xdaq::ApplicationStub* stub) :
  gem::base::GEMApplication(stub),
  m_shelfID(-1) 
{
  DEBUG("Creating the ShelfMonitorWeb interface");
  p_gemWebInterface = new gem::daqmon::ShelfMonitorWeb(this);
  DEBUG("Retrieving configuration");
  p_appInfoSpace->fireItemAvailable("shelfID",     &m_shelfID);
  p_appInfoSpace->addItemRetrieveListener("shelfID", this);
  p_appInfoSpace->addItemChangedListener("shelfID", this);
  DEBUG("done");
  //init();
}

gem::daqmon::ShelfMonitor::~ShelfMonitor()
{
  DEBUG("gem::daqmon::ShelfMonitor : Destructor called");
  // make sure to empty the v_supervisedApps  vector and free the pointers
}


// This is the callback used for handling xdata:Event objects
void gem::daqmon::ShelfMonitor::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("GEMSupervisor::actionPerformed() setDefaultValues" <<
          "Default configuration values have been loaded from xml profile");
    importConfigurationParameters();
    importMonitoringParameters();
    init();
    for (auto daqmon: v_daqmon)
    {
      daqmon->startMonitoring();
    }
  }

  // item is changed, update it
  if (event.type() == "ItemChangedEvent" || event.type() == "urn:xdata-event:ItemChangedEvent") {
    DEBUG("GEMSupervisor::actionPerformed() ItemChangedEvent");
  }

  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::daqmon::ShelfMonitor::init()
{
  v_daqmon.clear();
  v_daqmon.reserve(NAMC);
  for (int i = 1; i <= 12; ++i)
  {
    char t_board_name[20];
    sprintf(t_board_name, "gem.shelf%02d.amc%02d", m_shelfID.value_, i);
    DEBUG("gem::daqmon::ShelfMonitor::init :  Domain name for the board " << std::dec << i << " : " << t_board_name);
    v_daqmon.push_back(new gem::daqmon::DaqMonitor(t_board_name, this->getApplicationLogger(), this, i));
    DEBUG("gem::daqmon::ShelfMonitor::init : DaqMonitor pointer created");
  }
}

bool gem::daqmon::ShelfMonitor::isGEMApplication(const std::string& classname) const
{
  if (classname.find("gem::") != std::string::npos)
    return true;  // handle all GEM applications
  /*
    if (m_otherClassesToSupport.count(classname) != 0)
    return true;  // include from list
  */
  return false;
}
