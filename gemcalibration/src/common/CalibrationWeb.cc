#include "gem/gemcal/CalibrationWeb.h"
#include <boost/algorithm/string.hpp>
#include "xcept/tools.h"
#include "gem/base/utils/GEMInfoSpaceToolBox.h"
//#include "gem/daqmon/exception/Exception.h"  //CG
#include "gem/utils/exception/Exception.h"
#include "gem/gemcal/Calibration.h" //CG
//#include "gem/daqmon/DaqMonitor.h" //CG
#include <iomanip>



gem::gemcal::CalibrationWeb::CalibrationWeb(gem::gemcal::Calibration* CalibrationApp) :
  gem::base::GEMWebApplication(CalibrationApp)
{
 
// default constructor
}



gem::gemcal::CalibrationWeb::~CalibrationWeb()
{
  // default destructor
}

void gem::gemcal::CalibrationWeb::webDefault(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  *out << cgicc::script().set("type", "text/javascript")
    .set("src", "/gemdaq/gemdaqmonitor/html/scripts/shelfMonitor.js") //CG //let it be here for the moment, update with the relevant scripts later
       << cgicc::script() << std::endl;
  *out << "<link  rel=\"stylesheet\" href=\"/gemdaq/gemdaqmonitor/html/css/bootstrap.css\" type=\"text/css\">" << std::endl;
  //*out << "<script src=\"/gemdaq/gemdaqmonitor/html/js/bootstrap.min.js\"></script>" << std::endl; //CG

  GEMWebApplication::webDefault(in, out);
}

void gem::gemcal::CalibrationWeb::applicationPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CalibrationWeb::applicationPage : Do nothing for the moment, will be eventually filled later");
  //*out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  //*out << "Hello World" << std::endl;
  //*out << "</div>" << std::endl;
}

void gem::gemcal::CalibrationWeb::expertPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CalibrationWeb::expertPage");
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
    *out << "<div align=\"center\">" << std::endl;
    *out<< "<h1><span class=\"label label-info\" id=\"mon_state\">MONITORING STATE: "
        << dynamic_cast<gem::gemcal::Calibration*>(p_gemApp)->monitoringState() << "</span></h1>" << std::endl; //CG
    *out << "<button class=\"btn btn-danger\" type=\"button\" onclick=\"expert_action(this.id)\" id=\"stop\" name=\"stop\">STOP CALIBRATING </button>" << std::endl;
    *out << "<button class=\"btn btn-warning\" type=\"button\" onclick=\"expert_action(this.id)\" id=\"pause\" name=\"pause\">PAUSE CALIBRATING </button>" << std::endl;
    *out << "<button class=\"btn btn-success\" type=\"button\" onclick=\"expert_action(this.id)\" id=\"resume\" name=\"resume\">RESUME CALIBRATING </button>" << std::endl;
    *out << "</div>" << std::endl;
  *out << "</div>" << std::endl;

  std::string updateLink = "/" + p_gemApp->m_urn + "/";
  *out << "<script type=\"text/javascript\">"            << std::endl
       << "store_actionURL(\"" << updateLink << "\");" << std::endl
       << "</script>" << std::endl;

}
/*
void gem::gemcal::CalibrationWeb::monitorPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_INFO("CalibrationWeb::monitorPage");
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  for (unsigned int i = 0; i < NAMC; ++i){
    auto gemcal = dynamic_cast<gem::gemcal::Calibration*>(p_gemApp)->v_gemcal.at(i);
    if (gemcal) {
      *out << "<div class=\"xdaq-tab\" title=\"AMC" << std::setfill('0') << std::setw(2) << i+1 << "\" >"  << std::endl;
      gemcal->buildMonitorPage(out);
      *out << "</div>" << std::endl;
    }
  }
  *out << "</div>" << std::endl;
}

void gem::gemcal::CalibrationWeb::jsonUpdate(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CalibrationWeb::jsonUpdate");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out << " { " << std::endl;
  for (unsigned int i = 0; i < NAMC; ++i){
    auto gemcal = dynamic_cast<gem::gemcal::Calibration*>(p_gemApp)->v_gemcal.at(i);
    if (gemcal) {
      *out << "\"amc" << i+1 << "\" : "   << std::endl;
      gemcal->jsonContentUpdate(out);
      if (i!=NAMC-1) *out << "," << std::endl; // Add comma if not the last entry
    }
  }
  *out << " } " << std::endl;
}
*/
