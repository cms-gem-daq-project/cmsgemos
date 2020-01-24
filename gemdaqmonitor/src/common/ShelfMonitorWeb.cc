#include "gem/daqmon/ShelfMonitorWeb.h"
#include <boost/algorithm/string.hpp>
#include "xcept/tools.h"
#include "gem/base/utils/GEMInfoSpaceToolBox.h"
#include "gem/daqmon/exception/Exception.h"
#include "gem/daqmon/ShelfMonitor.h"
#include "gem/daqmon/DaqMonitor.h"
#include <iomanip>


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
{
  *out << cgicc::script().set("type", "text/javascript")
    .set("src", "/gemdaq/gemdaqmonitor/html/scripts/shelfMonitor.js") //let it be here for the moment, update with the relevant scripts later
       << cgicc::script() << std::endl;
  *out << "<link  rel=\"stylesheet\" href=\"/gemdaq/gemdaqmonitor/html/css/bootstrap.css\" type=\"text/css\">" << std::endl;
  //*out << "<script src=\"/gemdaq/gemdaqmonitor/html/js/bootstrap.min.js\"></script>" << std::endl;

  GEMWebApplication::webDefault(in, out);
}

void gem::daqmon::ShelfMonitorWeb::applicationPage(xgi::Input* in, xgi::Output* out)
{
  CMSGEMOS_DEBUG("ShelfMonitoringWeb::applicationPage : Do nothing for the moment, will be eventually filled later");
  //*out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  //*out << "Hello World" << std::endl;
  //*out << "</div>" << std::endl;
}

void gem::daqmon::ShelfMonitorWeb::expertPage(xgi::Input* in, xgi::Output* out)
{
  CMSGEMOS_DEBUG("ShelfMonitoringWeb::expertPage");
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
    *out << "<div align=\"center\">" << std::endl;
    *out<< "<h1><span class=\"label label-info\" id=\"mon_state\">MONITORING STATE: "
        << dynamic_cast<gem::daqmon::ShelfMonitor*>(p_gemApp)->monitoringState() << "</span></h1>" << std::endl;
    *out << "<button class=\"btn btn-danger\" type=\"button\" onclick=\"expert_action(this.id)\" id=\"stop\" name=\"stop\">STOP MONITORING</button>" << std::endl;
    *out << "<button class=\"btn btn-warning\" type=\"button\" onclick=\"expert_action(this.id)\" id=\"pause\" name=\"pause\">PAUSE MONITORING</button>" << std::endl;
    *out << "<button class=\"btn btn-success\" type=\"button\" onclick=\"expert_action(this.id)\" id=\"resume\" name=\"resume\">RESUME MONITORING</button>" << std::endl;
    *out << "</div>" << std::endl;
  *out << "</div>" << std::endl;

  std::string updateLink = "/" + p_gemApp->m_urn + "/";
  *out << "<script type=\"text/javascript\">"            << std::endl
       << "store_actionURL(\"" << updateLink << "\");" << std::endl
       << "</script>" << std::endl;

}

void gem::daqmon::ShelfMonitorWeb::monitorPage(xgi::Input* in, xgi::Output* out)
{
  CMSGEMOS_INFO("ShelfMonitorWeb::monitorPage");
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  for (unsigned int i = 0; i < NAMC; ++i){
    auto daqmon = dynamic_cast<gem::daqmon::ShelfMonitor*>(p_gemApp)->v_daqmon.at(i);
    if (daqmon) {
      *out << "<div class=\"xdaq-tab\" title=\"AMC" << std::setfill('0') << std::setw(2) << i+1 << "\" >"  << std::endl;
      daqmon->buildMonitorPage(out);
      *out << "</div>" << std::endl;
    }
  }
  *out << "</div>" << std::endl;
}

void gem::daqmon::ShelfMonitorWeb::jsonUpdate(xgi::Input* in, xgi::Output* out)
{
  CMSGEMOS_DEBUG("ShelfMonitorWeb::jsonUpdate");
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
