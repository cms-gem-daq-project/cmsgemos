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
  //*out << "</div>" << std::endl;
}

void gem::gemcal::CalibrationWeb::expertPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CalibrationWeb::expertPage");
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
    *out << "<div align=\"center\">" << std::endl;
    //*out<< "<h1><span class=\"label label-info\" id=\"mon_state\">MONITORING STATE: "
    //    << dynamic_cast<gem::gemcal::Calibration*>(p_gemApp)->monitoringState() << "</span></h1>" << std::endl; //CG
    //    *out << cgicc::script().set("type", "text/javascript").set("src", "/gemdaq/gemdaqmonitor/html/scripts/checkall.js")
 
*out<< "<h1><span class=\"label label-info\" id=\"cal_scurve\">SCURVE CALIBRATION "<< "</span></h1>" << std::endl; 
*out<< "<p>To run the Scurve calibration select the cards, the optohibryds, the VFATs and links. Indicate the number of events for each position and the latency and pulse stratch configuration.</p>" << std::endl; 

 *out << "<div class=\"container\"> "<< std::endl;
     
 *out << " <table class=\"table table-borderless\"> "<< std::endl;
 *out << "  <thead>  "<< std::endl;
 *out << "   <tr> "<< std::endl;
 *out << "  <th>  Card Name </th> "<< std::endl;
 *out << "  <th> OptoHybrid Mask </th> "<< std::endl;
 *out << "  <th> VFAT Mask </th> "<< std::endl;
 *out << "  <th> VFATChMin </th> "<< std::endl;
 *out << "  <th> VFATChMax </th> "<< std::endl;
 *out << "  <th> Number of events </th> "<< std::endl;
 *out << "  <th> Latency </th> "<< std::endl;
 *out << "  <th> Pulse stretch </th> "<< std::endl;
 *out << "  </tr> "<< std::endl;
 *out << "  </thead> "<< std::endl;
 *out << "  <tbody> "<< std::endl;
 *out << "  <tr> "<< std::endl;
 *out << "  <td> "<< std::endl;
 *out << "<div class=\"checkbox\">"<< std::endl;
 *out << "<label>"<< std::endl;
 *out << "<input type=\"checkbox\" class=\"check\"> eagleA" << std::endl;
 *out << "</label>"<< std::endl;
 *out << "</div>"<< std::endl;
 *out << "  </td> "<< std::endl;
 *out << "  <td>"<< std::endl;
 *out << " <form>"<< std::endl;
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"ohMask\">OH mask:</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\"  placeholder=\"0x000\" id=\"ohMask\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << "</form>"<< std::endl;
 *out << "</td> "<< std::endl;

 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"VFatMask\">VFat Mask</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\" placeholder=\"path to mask\" id=\"VFatMask\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 
 *out << " </td> "<< std::endl;

 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;	
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"VFatChMin\">VFATChMin</label>"<< std::endl;
 *out << " <input type=\"int\" class=\"form-control\" placeholder=\"Min Ch to scan:e.g. 0\" id=\"VFatChMin\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td> "<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"VFatChMax\">VFATChMax</label>"<< std::endl;
 *out << " <input type=\"int\" class=\"form-control\" placeholder=\"Max Ch to scan:e.g. 128\" id=\"VFatChMax\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td> "<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"nEvts\">Number of events</label>"<< std::endl;
 *out << " <input type=\"int\" class=\"form-control\" placeholder=\"100?\" id=\"nEvts\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td>"<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"MSPL\">Pulse stretch</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\" placeholder=\"CFG_PULSE_STRETCH\" id=\"MSPL\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td>"<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"Latency\"> Latency</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\" placeholder=\"CFG_LATENCY\" id=\"Latency\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td>"<< std::endl;
 *out << " </tr> "<< std::endl;

 *out << "  <tr> "<< std::endl;
 *out << "  <td> "<< std::endl;
 *out << "<div class=\"checkbox\">"<< std::endl;
 *out << "<label>"<< std::endl;
 *out << "<input type=\"checkbox\" class=\"check\"> eagleB" << std::endl;
 *out << "</label>"<< std::endl;
 *out << "</div>"<< std::endl;
 *out << "  </td> "<< std::endl;
 *out << "  <td>"<< std::endl;
 *out << " <form>"<< std::endl;
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"ohMask\">OH mask:</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\"  placeholder=\"0x000\" id=\"ohMask\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << "</form>"<< std::endl;
 *out << "</td> "<< std::endl;

 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"VFatMask\">VFat Mask</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\" placeholder=\"path to mask\" id=\"VFatMask\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 
 *out << " </td> "<< std::endl;

 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;	
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"VFatChMin\">VFATChMin</label>"<< std::endl;
 *out << " <input type=\"int\" class=\"form-control\" placeholder=\"Min Ch to scan:e.g. 0\" id=\"VFatChMin\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td> "<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"VFatChMax\">VFATChMax</label>"<< std::endl;
 *out << " <input type=\"int\" class=\"form-control\" placeholder=\"Max Ch to scan:e.g. 128\" id=\"VFatChMax\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td> "<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"nEvts\">Number of events</label>"<< std::endl;
 *out << " <input type=\"int\" class=\"form-control\" placeholder=\"100?\" id=\"nEvts\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td>"<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"MSPL\">Pulse stretch</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\" placeholder=\"CFG_PULSE_STRETCH\" id=\"MSPL\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td>"<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"Latency\"> Latency</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\" placeholder=\"CFG_LATENCY\" id=\"Latency\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td>"<< std::endl;
 *out << " </tr> "<< std::endl;



 *out << "  <tr> "<< std::endl;
 *out << "  <td> "<< std::endl;
 *out << "<div class=\"checkbox\">"<< std::endl;
 *out << "<label>"<< std::endl;
 *out << "<input type=\"checkbox\" class=\"check\"> eagleC" << std::endl;
 *out << "</label>"<< std::endl;
 *out << "</div>"<< std::endl;
 *out << "  </td> "<< std::endl;
 *out << "  <td>"<< std::endl;
 *out << " <form>"<< std::endl;
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"ohMask\">OH mask:</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\"  placeholder=\"0x000\" id=\"ohMask\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << "</form>"<< std::endl;
 *out << "</td> "<< std::endl;

 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"VFatMask\">VFat Mask</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\" placeholder=\"path to mask\" id=\"VFatMask\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 
 *out << " </td> "<< std::endl;

 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;	
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"VFatChMin\">VFATChMin</label>"<< std::endl;
 *out << " <input type=\"int\" class=\"form-control\" placeholder=\"Min Ch to scan:e.g. 0\" id=\"VFatChMin\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td> "<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"VFatChMax\">VFATChMax</label>"<< std::endl;
 *out << " <input type=\"int\" class=\"form-control\" placeholder=\"Max Ch to scan:e.g. 128\" id=\"VFatChMax\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td> "<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"nEvts\">Number of events</label>"<< std::endl;
 *out << " <input type=\"int\" class=\"form-control\" placeholder=\"100?\" id=\"nEvts\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td>"<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"MSPL\">Pulse stretch</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\" placeholder=\"CFG_PULSE_STRETCH\" id=\"MSPL\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td>"<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"Latency\"> Latency</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\" placeholder=\"CFG_LATENCY\" id=\"Latency\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td>"<< std::endl;
 *out << " </tr> "<< std::endl;

 *out << "  <tr> "<< std::endl;
 *out << "  <td> "<< std::endl;
 *out << "<div class=\"checkbox\">"<< std::endl;
 *out << "<label>"<< std::endl;
 *out << "<input type=\"checkbox\" class=\"check\"> eagleD" << std::endl;
 *out << "</label>"<< std::endl;
 *out << "</div>"<< std::endl;
 *out << "  </td> "<< std::endl;
 *out << "  <td>"<< std::endl;
 *out << " <form>"<< std::endl;
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"ohMask\">OH mask:</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\"  placeholder=\"0x000\" id=\"ohMask\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << "</form>"<< std::endl;
 *out << "</td> "<< std::endl;

 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"VFatMask\">VFat Mask</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\" placeholder=\"path to mask\" id=\"VFatMask\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 
 *out << " </td> "<< std::endl;

 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;	
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"VFatChMin\">VFATChMin</label>"<< std::endl;
 *out << " <input type=\"int\" class=\"form-control\" placeholder=\"Min Ch to scan:e.g. 0\" id=\"VFatChMin\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td> "<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"VFatChMax\">VFATChMax</label>"<< std::endl;
 *out << " <input type=\"int\" class=\"form-control\" placeholder=\"Max Ch to scan:e.g. 128\" id=\"VFatChMax\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td> "<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"nEvts\">Number of events</label>"<< std::endl;
 *out << " <input type=\"int\" class=\"form-control\" placeholder=\"100?\" id=\"nEvts\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td>"<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"MSPL\">Pulse stretch</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\" placeholder=\"CFG_PULSE_STRETCH\" id=\"MSPL\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td>"<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"Latency\"> Latency</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\" placeholder=\"CFG_LATENCY\" id=\"Latency\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td>"<< std::endl;
 *out << " </tr> "<< std::endl;


 *out << "  <tr> "<< std::endl;
 *out << "  <td> "<< std::endl;
 *out << "<div class=\"checkbox\">"<< std::endl;
 *out << "<label>"<< std::endl;
 *out << "<input type=\"checkbox\" class=\"check\"> eagleE" << std::endl;
 *out << "</label>"<< std::endl;
 *out << "</div>"<< std::endl;
 *out << "  </td> "<< std::endl;
 *out << "  <td>"<< std::endl;
 *out << " <form>"<< std::endl;
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"ohMask\">OH mask:</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\"  placeholder=\"0x000\" id=\"ohMask\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << "</form>"<< std::endl;
 *out << "</td> "<< std::endl;

 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"VFatMask\">VFat Mask</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\" placeholder=\"path to mask\" id=\"VFatMask\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 
 *out << " </td> "<< std::endl;

 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;	
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"VFatChMin\">VFATChMin</label>"<< std::endl;
 *out << " <input type=\"int\" class=\"form-control\" placeholder=\"Min Ch to scan:e.g. 0\" id=\"VFatChMin\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td> "<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"VFatChMax\">VFATChMax</label>"<< std::endl;
 *out << " <input type=\"int\" class=\"form-control\" placeholder=\"Max Ch to scan:e.g. 128\" id=\"VFatChMax\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td> "<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"nEvts\">Number of events</label>"<< std::endl;
 *out << " <input type=\"int\" class=\"form-control\" placeholder=\"100?\" id=\"nEvts\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td>"<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"MSPL\">Pulse stretch</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\" placeholder=\"CFG_PULSE_STRETCH\" id=\"MSPL\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td>"<< std::endl;
 
 *out << " <td>"<< std::endl;
 *out << "<form>"<< std::endl;			 
 *out << " <div class=\"form-group\">"<< std::endl;
 *out << " <label for=\"Latency\"> Latency</label>"<< std::endl;
 *out << " <input type=\"text\" class=\"form-control\" placeholder=\"CFG_LATENCY\" id=\"Latency\">"<< std::endl;
 *out << " </div>"<< std::endl;
 *out << " </form>"<< std::endl;
 *out << " </td>"<< std::endl;
 *out << " </tr> "<< std::endl;

 *out << " </tbody> "<< std::endl;
 *out << " </table> "<< std::endl;
 


 *out << "<button class=\"btn btn-dark\" type=\"button\" onclick=\"expert_action(this.id)\" id=\"applyall\" name=\"allpyall\">SELECT ALL</button>" << std::endl;
 
 
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
