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
}

gem::daqmon::ShelfMonitorWeb::~ShelfMonitorWeb()
{
  // default destructor
}

void gem::daqmon::ShelfMonitorWeb::webDefault(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  *out << cgicc::script().set("type", "text/javascript")
    .set("src", "/gemdaq/gemdaqmonitor/html/scripts/shelfMonitor.js") //let it be here for the moment, update with the relevant scripts later
       << cgicc::script() << std::endl;
  *out << "<link  rel=\"stylesheet\" href=\"/gemdaq/gemdaqmonitor/html/css/bootstrap.css\" type=\"text/css\">" << std::endl;
  //*out << "<script src=\"/gemdaq/gemdaqmonitor/html/js/bootstrap.min.js\"></script>" << std::endl;

  GEMWebApplication::webDefault(in, out);
}

void gem::daqmon::ShelfMonitorWeb::applicationPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  INFO("ShelfMonitorWeb::applicationPage");
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  for (unsigned int i = 0; i < NAMC; ++i){
    auto daqmon = dynamic_cast<gem::daqmon::ShelfMonitor*>(p_gemApp)->v_daqmon.at(i);
    if (daqmon) {
      *out << "<div class=\"xdaq-tab\" title=\"AMC-" << i+1 << "\" >"  << std::endl;
      daqmon->buildMonitorPage(out);
      *out << "</div>" << std::endl;
    }
  }
}

void gem::daqmon::ShelfMonitorWeb::jsonUpdate(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  DEBUG("ShelfMonitorWeb::jsonUpdate");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out << " { " << std::endl;
  for (unsigned int i = 0; i < NAMC; ++i){
    auto daqmon = dynamic_cast<gem::daqmon::ShelfMonitor*>(p_gemApp)->v_daqmon.at(i);
    if (daqmon) {
      *out << "\"amc" << i+1 << "\" : "   << std::endl;
      daqmon->jsonContentUpdate(out);
      if (i!=NAMC-1) *out << "," << std::endl; // Add comma if not the last entry
    }
  }
  *out << " } " << std::endl;
}
