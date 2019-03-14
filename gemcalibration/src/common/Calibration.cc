/**
 * class: Calibration
 * description: Calibration application for GEM system
 *              structure borrowed from gemSupervisor
 * author: 
 * date:
 */

#include "gem/calib/Calibration.h"

#include <cstdlib>
#include <iomanip>

#include <map>
#include <set>
#include <vector>
#include <algorithm>

#include <boost/algorithm/string.hpp>

#include "gem/calib/CalibrationWeb.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"
#include "gem/utils/exception/Exception.h"

typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

XDAQ_INSTANTIATOR_IMPL(gem::calib::Calibration);

gem::calib::Calibration::Calibration(xdaq::ApplicationStub* stub) :
  gem::base::GEMApplication(stub),
  m_shelfID(-1) ///CG
{
  CMSGEMOS_DEBUG("gem::calib::Calibration : Creating the CalibrationWeb interface");
  p_gemWebInterface = new gem::calib::CalibrationWeb(this);
  CMSGEMOS_DEBUG("gem::calib::Calibration : Retrieving configuration");
  //p_appInfoSpace->fireItemAvailable("shelfID",     &m_shelfID);
  //p_appInfoSpace->addItemRetrieveListener("shelfID", this);
  //p_appInfoSpace->addItemChangedListener("shelfID", this);
  CMSGEMOS_DEBUG("gem::calib::Calibration : configuration retrieved");
/*
  xgi::bind(this, &Calibration::stopAction, "stopAction");
  xgi::bind(this, &Calibration::resumeAction, "resumeAction");
  xgi::bind(this, &Calibration::pauseAction, "pauseAction");
  //init();
  //*/
  xgi::bind(this, &Calibration::pauseAction, "pauseAction");
  xgi::bind(this, &Calibration::applyAction, "applyAction");
}

gem::calib::Calibration::~Calibration()
{
  CMSGEMOS_DEBUG("gem::calib::Calibration : Destructor called");
  // make sure to empty the v_supervisedApps  vector and free the pointers
}


// This is the callback used for handling xdata:Event objects
void gem::calib::Calibration::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    CMSGEMOS_DEBUG("gem::calib::Calibration::actionPerformed() setDefaultValues" <<
          "Default configuration values have been loaded from xml profile");
    //importConfigurationParameters();
    //importMonitoringParameters();
    //init();
    //startMonitoring();
  }

  // item is changed, update it
  if (event.type() == "ItemChangedEvent" || event.type() == "urn:xdata-event:ItemChangedEvent") {
    CMSGEMOS_DEBUG("gem::calib::Calibration:actionPerformed() ItemChangedEvent");
  }

  // update monitoring variables
   gem::base::GEMApplication::actionPerformed(event);
}

void gem::calib::Calibration::init()
{
  /*  v_daqmon.clear();
      v_daqmon.reserve(NAMC);
      for (int i = 1; i <= NAMC; ++i)
      {
      char t_board_name[20];
      sprintf(t_board_name, "gem-shelf%02d-amc%02d", m_shelfID.value_, i);
      CMSGEMOS_DEBUG("gem::daqmon::ShelfMonitor::init :  Domain name for the board " << std::dec << i << " : " << t_board_name);
      v_daqmon.push_back(new gem::daqmon::DaqMonitor(t_board_name, this->getApplicationLogger(), this, i));
      CMSGEMOS_DEBUG("gem::daqmon::ShelfMonitor::init : DaqMonitor pointer created");
  */
}


bool gem::calib::Calibration::isGEMApplication(const std::string& classname) const
{
  if (classname.find("gem::") != std::string::npos)
    return true;  // handle all GEM applications
  /*
    if (m_otherClassesToSupport.count(classname) != 0)
    return true;  // include from list
  */
  return false;
}

void gem::calib::Calibration::stopAction(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
/*
  CMSGEMOS_INFO("ShelfMonitor::stopAction");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  //this->stopMonitoring();
  *out << " { \"mon_state\":\"STOPPED\"}" << std::endl;
*/
}

void gem::calib::Calibration::resumeAction(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
/*
  CMSGEMOS_INFO("ShelfMonitor::startAction");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  //this->startMonitoring();
  *out << " { \"mon_state\":\"RUNNING\"}" << std::endl;
*/
}

void gem::calib::Calibration::pauseAction(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  
  CMSGEMOS_INFO("Calibration::pauseAction");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  //this->stopMonitoring();
  *out << " { \"mon_state\":\"PAUSED\"}" << std::endl;
  std::cout << "ciao! " << std::endl;
}

void gem::calib::Calibration::applyAction(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{

  CMSGEMOS_INFO("Calibration::applyAction");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  //this->stopMonitoring();
  //*out << " { \"cal_scurve\":\"APPLIED\"}" << std::endl;
  std::cout << "ciao2" << std::endl;
  PulseDelay=*out->getHTTPResponseHeader().getElementById("PulseDelay").value;
  std::cout<< "Calibration.cc PulseDelay " << PulseDelay <<std::endl;
  
}
