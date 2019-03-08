#include "gem/calib/CalibrationWeb.h"
#include <boost/algorithm/string.hpp>
#include "xcept/tools.h"
#include "gem/base/utils/GEMInfoSpaceToolBox.h"
//#include "gem/daqmon/exception/Exception.h"  //CG
#include "gem/utils/exception/Exception.h"
#include "gem/calib/Calibration.h" //CG
//#include "gem/daqmon/DaqMonitor.h" //CG
#include <iomanip>



gem::calib::CalibrationWeb::CalibrationWeb(gem::calib::Calibration* CalibrationApp) :
  gem::base::GEMWebApplication(CalibrationApp)
{
 
// default constructor
}



gem::calib::CalibrationWeb::~CalibrationWeb()
{
  // default destructor
}

void gem::calib::CalibrationWeb::webDefault(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  *out << "<link  rel=\"stylesheet\" href=\"/gemdaq/gemcalibration/html/css/bootstrap.css\" type=\"text/css\">" << std::endl;
  *out << "<link  rel=\"stylesheet\" href=\"/gemdaq/gemcalibration/html/css/bootstrap.min.css\" type=\"text/css\">" << std::endl;
  *out << "<script src=\"/gemdaq/gemdcalibration/html/scripts/bootstrap.min.js\"></script>" << std::endl; //CG
  *out << "<script src=\"/gemdaq/gemcalibration/html/scripts/bootstrap.js\"></script>" << std::endl; //CG
  *out << "<script src=\"/gemdaq/gemcalibration/html/scripts/jquery.min.js\"></script>" << std::endl; //CG
  

  GEMWebApplication::webDefault(in, out);
}

void gem::calib::CalibrationWeb::applicationPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CalibrationWeb::applicationPage : Do nothing for the moment, will be eventually filled later");
  //*out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  //*out << "</div>" << std::endl;
}

void gem::calib::CalibrationWeb::expertPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CalibrationWeb::expertPage");
  *out<<"<div class=\"xdaq-tab-wrapper\">" << std::endl;
  //*out << "<div align=\"center\">" << std::endl;
  //*out<< "<h1><span class=\"label label-info\" id=\"mon_state\">MONITORING STATE: "
  //    << dynamic_cast<gem::calib::Calibration*>(p_gemApp)->monitoringState() << "</span></h1>" << std::endl; //CG
  //    *out << cgicc::script().set("type", "text/javascript").set("src", "/gemdaq/gemcalibration/html/scripts/checkall.js")
 
  *out<<"<h1><span class=\"label label-info\" id=\"cal_scurve\">SCURVE INTERFACE"<< "</span></h1>"<<std::endl; 
  *out<<"<p> To run the Scurve select the cards, the optohibryds, the VFATs and links. Indicate the number of events for each position and the latency and pulse stratch configuration.</p>"<<std::endl;
  *out<<"<br>"<<std::endl;
  *out<<"<div align=\"left\">"<< std::endl;
  *out<<"<p>Source type:</p>"<< std::endl;
  *out<<"<form>"<< std::endl;
  *out<<"<div class=\"radio-inline\"> <label><input type=\"radio\" name=\"optradio\" checked>TTC</label></div>"<<std::endl;
  *out<<"<div class=\"radio-inline\"> <label><input type=\"radio\" name=\"optradio\" checked>Loopback</label></div>"<<std::endl;
  *out<<"<div class=\"radio-inline\"> <label><input type=\"radio\" name=\"optradio\" checked>Lemo/T3</label></div>"<<std::endl;
  *out<<"</form>"<< std::endl;
  *out<<"<br>"<<std::endl;
  *out<<"</div>"<<std::endl;
  *out<<"<div align=\"left\">"<< std::endl;
  *out<<"<p>Settings:</p>"<<std::endl; 
  *out<<"</div>"<<std::endl;
  *out<<"<div align=\"right\">"<< std::endl;
  *out<<"<table class=\"table table-borderless\">"<< std::endl;
  *out<<"<tbody>"<< std::endl;
  *out<<"<tr>"<< std::endl;
  *out<<"<td>"<< std::endl;
   // *out<< "<form class=\"form-inline\">"<<std::endl;   //action=\"/action_page.php\">"<<std::endl;
  *out<<"<label for=\"L1Atime\">L1Atime:</label>"<< std::endl;
  *out<<"<input type=\"text\" placeholder=\"250\" class=\"form-control\" id=\"L1Atime\">"<< std::endl;
  *out<<"</td>"<< std::endl;*out<<"<td>"<<std::endl;
  *out<<"<label for=\"Latency\">Latency:</label>"<<std::endl;
  *out<<"<input type=\"text\" placeholder=\"33\" class=\"form-control\" id=\"Latency\">"<<std::endl;
  *out<<"</td> "<< std::endl;*out<<"<td> "<< std::endl;
  *out<<"<label for=\"PulseDelay\">Pulse delay:</label>"<<std::endl;
  *out<<"<input type=\"text\" placeholder=\"40\" class=\"form-control\" id=\"PulseDelay\">"<<std::endl;
  *out<<"</td>"<< std::endl;
  *out<<"</tr>"<< std::endl;*out<<"<tr>"<< std::endl;
  *out<<"<td>"<< std::endl;
  *out<<"<label for=\"CalPhase\">Calpulse phase:</label>"<<std::endl;
  *out<<"<input type=\"text\" placeholder=\"0\" class=\"form-control\" id=\"CalPhase\">"<<std::endl;
  *out<<"</td>"<<std::endl;*out<<"<td>"<< std::endl;
  // *out<< "<br>"<<std::endl;
  *out<<"<label for=\"vfatChMin\">VFAT channel Min:</label>"<<std::endl;
  *out<<"<input type=\"text\" placeholder=\"0\" class=\"form-control\" id=\"vfatChMin\">"<<std::endl;
    *out<<"</td>"<< std::endl;*out<<"<td>"<< std::endl;

  *out<<"<label for=\"vfatChMax\">VFAT channel Max:</label>"<<std::endl;
  *out<<"<input type=\"text\" placeholder=\"127\" class=\"form-control\" id=\"vfatChMax\">"<<std::endl;
  *out<<"</td>"<< std::endl;
  *out<<"</tr>"<< std::endl;*out<<"<tr>"<< std::endl;
  *out<<"<td>"<< std::endl;
  *out<<"<label for=\"NSamples\">Number of samples:</label>"<<std::endl;
  *out<<"<input type=\"text\" placeholder=\"100\" class=\"form-control\" id=\"NSamples\">"<<std::endl;
  *out<<"</td>"<< std::endl;*out<<"<td>"<< std::endl;
  *out<<"</td>"<< std::endl;*out<<"<td>"<< std::endl;
  *out<<"<br>"<< std::endl;
   //*out<< "</form>"<<std::endl;
  *out<<"</td>"<< std::endl;
  *out<<"</tr>"<< std::endl;
  *out<<"</tbody>"<< std::endl;
  *out<<"</table>"<< std::endl;
  *out<<"</div>"<< std::endl;

  *out<<"<br>"<< std::endl;
  *out<<"<div align=\"right\">"<<std::endl;
  *out<<"<button class=\"btn btn-info\" type=\"button\" onclick=\"expert_action(this.id)\" id=\"applyall\" name=\"allpyall\">APPLY TO ALL</button>"<<std::endl;
  *out<< "</div>"<<std::endl;
  *out<< "<br><br>"<<std::endl;
  
  *out<<"<div align=\"center\">"<<std::endl;
  *out<<"<table class=\"table table-borderless\">"<< std::endl;
  *out<<"<thead>"<< std::endl;
  *out<<"<tr>"<< std::endl;
  *out<<"<th> Shelf </th> "<< std::endl;
  *out<<"<th> AMC slot and OH mask </th> "<< std::endl;
  *out<<"</tr>"<< std::endl;
  *out<<"</thead>"<< std::endl;
  *out<<"<tbody>"<< std::endl;
    
  for (unsigned int i = 0; i < NSHELF; ++i){

    *out<<"<tr>"<< std::endl;
    *out<<"<td>"<< std::endl;
    *out<<"<div class=\"checkbox\"> <label> <input type=\"checkbox\" class=\"check\" id=\"SHELF" <<std::setfill('0')<<std::setw(2)<<i+1<<"\">  SHELF "<<std::setfill('0')<<std::setw(2)<<i+1<<" </label> </div>"<<std::endl;
    *out<<"</td>"<< std::endl;
    *out<<"<td>"<< std::endl;

    *out<<"<div class=\"dropdown\">"<< std::endl;
    *out<<"<button type=\"button\" class=\"btn btn-outline dropdown-toggle\" data-toggle=\"dropdown\">Select AMC and OH</button>"<< std::endl;
   
    *out<<"<div class=\"dropdown-menu\">"<< std::endl;
     
    for (unsigned int j = 0; j < NAMC; ++j){ //SHELF.AMC
      *out<<"<span class =\"dropdown-item-text\">"<< std::endl;
      *out<<"<form class=\"form-inline\"> "<<std::endl;
      *out <<"<div class=\"checkbox\"><label><input type=\"checkbox\" class=\"check\" id=\"AMC"<<std::setfill('0')<<std::setw(2)<<i+1 <<"."<<std::setfill('0')<<std::setw(2)<<j+1<<"\">  AMC"<<std::setfill('0')<<std::setw(2)<<j+1<<" </label> </div>"  <<std::endl; 
     
      *out<<"<form> <div class=\"form-group\"><label for=\"ohMask\">OH mask:</label> <input type=\"text\" class=\"form-control\"  placeholder=\"0x000\" id=\"ohMask"<<std::setfill('0')<<std::setw(2)<<i+1 <<"."<<std::setfill('0')<<std::setw(2)<<j+1<<"\"> </div> </form>"<< std::endl;     

      *out<<"</form>"<< std::endl;
      *out<<"</span>"<< std::endl;    
    }
    *out<<"</td>"<< std::endl;
    *out<<"</tr>"<< std::endl;
    *out<<"</div>"<< std::endl;
    *out<<"</div>"<< std::endl;
    
  }
  *out<<"</tbody>"<< std::endl;
  *out<<"</table>"<< std::endl;
  *out<<"<div align=\"right\">"<<std::endl;
  *out<<"<button class=\"btn btn-info\" type=\"button\" onclick=\"expert_action(this.id)\" id=\"selectall\" name=\"allpyall\">SELECT ALL</button>"<<std::endl;
  *out<<"</div>"<< std::endl;
  *out<< "<br>"<<std::endl; 
 
  *out<<"<div align=\"right\">"<<std::endl;
 *out<<"<button class=\"btn btn-info\" type=\"button\" onclick=\"expert_action(this.id)\" id=\"calibrate\" name=\"allpyall\">RUN</button>"<<std::endl;
 *out<<"</div>"<< std::endl;
 
 *out<<"</div>"<<std::endl;
 *out<<"</div>"<<std::endl;
 
/*
 std::string updateLink = "/" + p_gemApp->m_urn + "/";
 *out<<"<script type=\"text/javascript\">"           <<std::endl
     <<"store_actionURL(\""<<updateLink<<"\");"<<std::endl
     <<"</script>"<<std::endl;

*/
}

void gem::calib::CalibrationWeb::monitorPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{

  CMSGEMOS_DEBUG("CalibrationWeb::LatencyScanPage");
  *out<<"<div class=\"xdaq-tab-wrapper\">" << std::endl;
  //*out << "<div align=\"center\">" << std::endl;
  //*out<< "<h1><span class=\"label label-info\" id=\"mon_state\">MONITORING STATE: "
  //    << dynamic_cast<gem::calib::Calibration*>(p_gemApp)->monitoringState() << "</span></h1>" << std::endl; //CG
  //    *out << cgicc::script().set("type", "text/javascript").set("src", "/gemdaq/gemcalibration/html/scripts/checkall.js")
 
  *out<<"<h1><span class=\"label label-info\" id=\"cal_scurve\">LATENCY SCAN INTERFACE"<< "</span></h1>"<<std::endl; 
  *out<<"<p> To run the latency scan select the cards, the optohibryds, the VFATs and links. Indicate the number of events for each position and the latency and pulse stratch configuration.</p>"<<std::endl;
  *out<<"<br>"<<std::endl;
  *out<<"<div align=\"left\">"<< std::endl;
  *out<<"<p>Source type:</p>"<< std::endl;
  *out<<"<form>"<< std::endl;
  *out<<"<div class=\"radio-inline\"> <label><input type=\"radio\" name=\"optradio\" checked>TTC</label></div>"<<std::endl;
  *out<<"<div class=\"radio-inline\"> <label><input type=\"radio\" name=\"optradio\" checked>Loopback</label></div>"<<std::endl;
  *out<<"<div class=\"radio-inline\"> <label><input type=\"radio\" name=\"optradio\" checked>Lemo/T3</label></div>"<<std::endl;
  *out<<"</form>"<< std::endl;
  *out<<"<br>"<<std::endl;
  *out<<"</div>"<<std::endl;
  *out<<"<div align=\"left\">"<< std::endl;
  *out<<"<p>Settings:</p>"<<std::endl; 
  *out<<"</div>"<<std::endl;
  *out<<"<div align=\"right\">"<< std::endl;
  *out<<"<table class=\"table table-borderless\">"<< std::endl;
  *out<<"<tbody>"<< std::endl;
  *out<<"<tr>"<< std::endl;
  *out<<"<td>"<< std::endl;
   // *out<< "<form class=\"form-inline\">"<<std::endl;   //action=\"/action_page.php\">"<<std::endl;
  *out<<"<label for=\"L1Atime\">L1Atime:</label>"<< std::endl;
  *out<<"<input type=\"text\" placeholder=\"250\" class=\"form-control\" id=\"L1Atime\">"<< std::endl;
  *out<<"</td>"<< std::endl;*out<<"<td>"<<std::endl;
  *out<<"<label for=\"Latency\">Latency:</label>"<<std::endl;
  *out<<"<input type=\"text\" placeholder=\"33\" class=\"form-control\" id=\"Latency\">"<<std::endl;
  *out<<"</td> "<< std::endl;*out<<"<td> "<< std::endl;
  *out<<"<label for=\"PulseDelay\">Pulse delay:</label>"<<std::endl;
  *out<<"<input type=\"text\" placeholder=\"40\" class=\"form-control\" id=\"PulseDelay\">"<<std::endl;
  *out<<"</td>"<< std::endl;
  *out<<"</tr>"<< std::endl;*out<<"<tr>"<< std::endl;
  *out<<"<td>"<< std::endl;
  *out<<"<label for=\"CalPhase\">Calpulse phase:</label>"<<std::endl;
  *out<<"<input type=\"text\" placeholder=\"0\" class=\"form-control\" id=\"CalPhase\">"<<std::endl;
  *out<<"</td>"<<std::endl;*out<<"<td>"<< std::endl;
  // *out<< "<br>"<<std::endl;
  *out<<"<label for=\"scanMin\">Scan Min:</label>"<<std::endl;
  *out<<"<input type=\"text\" placeholder=\"153\" class=\"form-control\" id=\"scanMin\">"<<std::endl;
  *out<<"</td>"<< std::endl;*out<<"<td>"<< std::endl;
  *out<<"<label for=\"scanMax\">Scan Max:</label>"<<std::endl;
  *out<<"<input type=\"text\" placeholder=\"172\" class=\"form-control\" id=\"scanMax\">"<<std::endl;
   *out<<"</td>"<< std::endl;
  *out<<"</tr>"<< std::endl;*out<<"<tr>"<< std::endl;
  *out<<"<td>"<< std::endl;
  *out<<"<label for=\"NSamples\">Number of samples:</label>"<<std::endl;
  *out<<"<input type=\"text\" placeholder=\"100\" class=\"form-control\" id=\"NSamples\">"<<std::endl;
  *out<<"</td>"<< std::endl;*out<<"<td>"<< std::endl;
  *out<<"<label for=\"throttle\">Throttle:</label>"<<std::endl;
  *out<<"<input type=\"text\" placeholder=\"100\" class=\"form-control\" id=\"throttle\">"<<std::endl;

  *out<<"</td>"<< std::endl;*out<<"<td>"<< std::endl;
 *out<<"<label for=\"vt2\">VThreshold2:</label>"<<std::endl;
  *out<<"<input type=\"text\" placeholder=\"0\" class=\"form-control\" id=\"vt2\">"<<std::endl;

  
  *out<<"<br>"<< std::endl;
   //*out<< "</form>"<<std::endl;
  *out<<"</td>"<< std::endl;
  *out<<"</tr>"<< std::endl;
  *out<<"</tbody>"<< std::endl;
  *out<<"</table>"<< std::endl;
  *out<<"</div>"<< std::endl;

  *out<<"<br>"<< std::endl;
  *out<<"<div align=\"right\">"<<std::endl;
  *out<<"<button class=\"btn btn-info\" type=\"button\" onclick=\"expert_action(this.id)\" id=\"applyall\" name=\"allpyall\">APPLY TO ALL</button>"<<std::endl;
  *out<< "</div>"<<std::endl;
  *out<< "<br><br>"<<std::endl;
  
  *out<<"<div align=\"center\">"<<std::endl;
  *out<<"<table class=\"table table-borderless\">"<< std::endl;
  *out<<"<thead>"<< std::endl;
  *out<<"<tr>"<< std::endl;
  *out<<"<th> Shelf </th> "<< std::endl;
  *out<<"<th> AMC slot and OH mask </th> "<< std::endl;
  *out<<"</tr>"<< std::endl;
  *out<<"</thead>"<< std::endl;
  *out<<"<tbody>"<< std::endl;
    
  for (unsigned int i = 0; i < NSHELF; ++i){

    *out<<"<tr>"<< std::endl;
    *out<<"<td>"<< std::endl;
    *out<<"<div class=\"checkbox\"> <label> <input type=\"checkbox\" class=\"check\" id=\"SHELF" <<std::setfill('0')<<std::setw(2)<<i+1<<"\">  SHELF "<<std::setfill('0')<<std::setw(2)<<i+1<<" </label> </div>"<<std::endl;
    *out<<"</td>"<< std::endl;
    *out<<"<td>"<< std::endl;

    *out<<"<div class=\"dropdown\">"<< std::endl;
    *out<<"<button type=\"button\" class=\"btn btn-outline dropdown-toggle\" data-toggle=\"dropdown\">Select AMC and OH</button>"<< std::endl;
   
    *out<<"<div class=\"dropdown-menu\">"<< std::endl;
     
    for (unsigned int j = 0; j < NAMC; ++j){ //SHELF.AMC
      *out<<"<span class =\"dropdown-item-text\">"<< std::endl;
      *out<<"<form class=\"form-inline\"> "<<std::endl;
      *out <<"<div class=\"checkbox\"><label><input type=\"checkbox\" class=\"check\" id=\"AMC"<<std::setfill('0')<<std::setw(2)<<i+1 <<"."<<std::setfill('0')<<std::setw(2)<<j+1<<"\">  AMC"<<std::setfill('0')<<std::setw(2)<<j+1<<" </label> </div>"  <<std::endl; 
     
      *out<<"<form> <div class=\"form-group\"><label for=\"ohMask\">OH mask:</label> <input type=\"text\" class=\"form-control\"  placeholder=\"0x000\" id=\"ohMask"<<std::setfill('0')<<std::setw(2)<<i+1 <<"."<<std::setfill('0')<<std::setw(2)<<j+1<<"\"> </div> </form>"<< std::endl;     

      *out<<"</form>"<< std::endl;
      *out<<"</span>"<< std::endl;    
    }
    *out<<"</td>"<< std::endl;
    *out<<"</tr>"<< std::endl;
    *out<<"</div>"<< std::endl;
    *out<<"</div>"<< std::endl;
    
  }
  *out<<"</tbody>"<< std::endl;
  *out<<"</table>"<< std::endl;
  *out<<"<div align=\"right\">"<<std::endl;
  *out<<"<button class=\"btn btn-info\" type=\"button\" onclick=\"expert_action(this.id)\" id=\"selectall\" name=\"allpyall\">SELECT ALL</button>"<<std::endl;
  *out<<"</div>"<< std::endl;
  *out<< "<br>"<<std::endl; 
 
  *out<<"<div align=\"right\">"<<std::endl;
 *out<<"<button class=\"btn btn-info\" type=\"button\" onclick=\"expert_action(this.id)\" id=\"calibrate\" name=\"allpyall\">RUN</button>"<<std::endl;
 *out<<"</div>"<< std::endl;
 
 *out<<"</div>"<<std::endl;
 *out<<"</div>"<<std::endl;
 

  
}

/*
void gem::calib::CalibrationWeb::monitorPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_INFO("CalibrationWeb::monitorPage");
  *out<<"<div class=\"xdaq-tab-wrapper\">"<<std::endl;
  for (unsigned int i = 0; i < NAMC; ++i){
    auto gemcal = dynamic_cast<gem::calib::Calibration*>(p_gemApp)->v_gemcal.at(i);
    if (gemcal) {
      *out<<"<div class=\"xdaq-tab\" title=\"AMC"<<std::setfill('0')<<std::setw(2)<<i+1<<"\" >" <<std::endl;
      gemcal->buildMonitorPage(out);
      *out<<"</div>"<<std::endl;
    }
  }
  *out<<"</div>"<<std::endl;
}
*/

void gem::calib::CalibrationWeb::jsonUpdate(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CalibrationWeb::jsonUpdate");
/*
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out<<" { "<<std::endl;
  for (unsigned int i = 0; i < NAMC; ++i){
    auto gemcal = dynamic_cast<gem::calib::Calibration*>(p_gemApp)->v_gemcal.at(i);
    if (gemcal) {
      *out<<"\"amc"<<i+1<<"\" : "  <<std::endl;
      gemcal->jsonContentUpdate(out);
      if (i!=NAMC-1) *out<<","<<std::endl; // Add comma if not the last entry
    }
  }
  *out<<" } "<<std::endl;
*/
}
