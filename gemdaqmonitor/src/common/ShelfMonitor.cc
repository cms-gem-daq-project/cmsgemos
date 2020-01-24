/**
 * class: ShelfMonitor
 * description: Shelf monitoring application for GEM system
 *              structure borrowed from gemSupervisor
 * author: M. Dalchenko
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
  CMSGEMOS_DEBUG("gem::daqmon::ShelfMonitor : Creating the ShelfMonitorWeb interface");
  p_gemWebInterface = new gem::daqmon::ShelfMonitorWeb(this);
  CMSGEMOS_DEBUG("gem::daqmon::ShelfMonitor : Retrieving configuration");
  p_appInfoSpace->fireItemAvailable("shelfID",     &m_shelfID);
  p_appInfoSpace->addItemRetrieveListener("shelfID", this);
  p_appInfoSpace->addItemChangedListener("shelfID", this);
  CMSGEMOS_DEBUG("gem::daqmon::ShelfMonitor : configuration retrieved");
  xgi::bind(this, &ShelfMonitor::stopAction, "stopAction");
  xgi::bind(this, &ShelfMonitor::resumeAction, "resumeAction");
  xgi::bind(this, &ShelfMonitor::pauseAction, "pauseAction");
  //init();
}

gem::daqmon::ShelfMonitor::~ShelfMonitor()
{
  CMSGEMOS_DEBUG("gem::daqmon::ShelfMonitor : Destructor called");
  // make sure to empty the v_supervisedApps  vector and free the pointers
}


// This is the callback used for handling xdata:Event objects
void gem::daqmon::ShelfMonitor::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    CMSGEMOS_DEBUG("gem::daqmon::ShelfMonitor::actionPerformed() setDefaultValues" <<
          "Default configuration values have been loaded from xml profile");
    importConfigurationParameters();
    importMonitoringParameters();
    init();
    startMonitoring();
  }

  // item is changed, update it
  if (event.type() == "ItemChangedEvent" || event.type() == "urn:xdata-event:ItemChangedEvent") {
    CMSGEMOS_DEBUG("gem::daqmon::ShelfMonitor::actionPerformed() ItemChangedEvent");
  }

  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::daqmon::ShelfMonitor::init()
{
  v_daqmon.clear();
  v_daqmon.reserve(NAMC);
  for (int i = 1; i <= NAMC; ++i)
  {
    char t_board_name[20];
    sprintf(t_board_name, "gem-shelf%02d-amc%02d", m_shelfID.value_, i);
    CMSGEMOS_DEBUG("gem::daqmon::ShelfMonitor::init :  Domain name for the board " << std::dec << i << " : " << t_board_name);
    v_daqmon.push_back(new gem::daqmon::DaqMonitor(t_board_name, this->getApplicationLogger(), this, i));
    CMSGEMOS_DEBUG("gem::daqmon::ShelfMonitor::init : DaqMonitor pointer created");
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

void gem::daqmon::ShelfMonitor::startMonitoring()
{
    int cnt = 0;
    for (auto daqmon: v_daqmon)
    {
      if (daqmon->is_connected()) {
        daqmon->startMonitoring();
        ++cnt;
      } else {
        CMSGEMOS_INFO("gem::daqmon::ShelfMonitor::actionPerformed() setDefaultValues : Connection to the board "
                      << daqmon->boardName()
        << " cannot be established. Monitoring for this board is OFF");
      }
    }
    (cnt>0)?m_state="RUNNING":"FAILED";
}

void gem::daqmon::ShelfMonitor::stopMonitoring()
{
    for (auto daqmon: v_daqmon)
    {
      if (daqmon->is_connected()) {
        daqmon->stopMonitoring();
      } else {
        CMSGEMOS_INFO("gem::daqmon::ShelfMonitor::actionPerformed() setDefaultValues : Connection to the board "
                      << daqmon->boardName()
        << " cannot be established. Monitoring for this board is OFF");
      }
    }
    m_state="STOPPED";
}

void gem::daqmon::ShelfMonitor::stopAction(xgi::Input* in, xgi::Output* out)
{
  CMSGEMOS_INFO("ShelfMonitor::stopAction");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  this->stopMonitoring();
  *out << " { \"mon_state\":\"STOPPED\"}" << std::endl;

}

void gem::daqmon::ShelfMonitor::resumeAction(xgi::Input* in, xgi::Output* out)
{
  CMSGEMOS_INFO("ShelfMonitor::startAction");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  this->startMonitoring();
  *out << " { \"mon_state\":\"RUNNING\"}" << std::endl;
}

void gem::daqmon::ShelfMonitor::pauseAction(xgi::Input* in, xgi::Output* out)
{
  CMSGEMOS_INFO("ShelfMonitor::pauseAction");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  this->stopMonitoring();
  *out << " { \"mon_state\":\"PAUSED\"}" << std::endl;
}
