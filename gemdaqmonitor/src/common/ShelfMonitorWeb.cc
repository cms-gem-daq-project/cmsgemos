#include "gem/daqmon/ShelfMonitorWeb.h"
#include <boost/algorithm/string.hpp>
#include "xcept/tools.h"
#include "gem/base/utils/GEMInfoSpaceToolBox.h"
#include "gem/daqmon/exception/Exception.h"
#include "gem/daqmon/ShelfMonitor.h"
#include "gem/daqmon/DaqMonitor.h"


gem::daqmon::ShelfMonitorWeb::ShelfMonitorWeb(gem::daqmon::ShelfMonitor* shelfMonitorApp) :
  gem::base::GEMWebApplication(shelfMonitorApp)
{
  level = 5;
  // p_gemMonitor = dynamic_cast<gem::supervisor::GEMSupervisorMonitor*> gemSupervisorApp->getMonitor();
}

gem::daqmon::ShelfMonitorWeb::~ShelfMonitorWeb()
{
  // default destructor
}

void gem::daqmon::ShelfMonitorWeb::webDefault(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  *out << cgicc::script().set("type", "text/javascript")
    .set("src", "/gemdaq/gemsupervisor/html/scripts/gemsupervisor.js")
       << cgicc::script() << std::endl;

  GEMWebApplication::webDefault(in, out);
}

void gem::daqmon::ShelfMonitorWeb::applicationPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  INFO("ShelfMonitorWeb::applicationPage");
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  //for (unsigned int i = 0; i < NAMC; ++i){
  for (unsigned int i = 3; i < 4; ++i){
    //auto daqmon = dynamic_cast<gem::daqmon::ShelfMonitor*>(p_gemApp)->v_daqmon.at(i);
    auto daqmon = dynamic_cast<gem::daqmon::ShelfMonitor*>(p_gemApp)->v_daqmon.at(0);
    if (daqmon) {
      *out << "<div class=\"xdaq-tab\" title=\"AMC-" << i << "\" >"  << std::endl;
      daqmon->buildMonitorPage(out);
      *out << "</div>" << std::endl;
    }
  }
}
