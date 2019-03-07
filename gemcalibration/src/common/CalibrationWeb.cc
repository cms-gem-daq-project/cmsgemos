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
  //*out << cgicc::script().set("type", "text/javascript")
  //  .set("src", "/gemdaq/gemdaqmonitor/html/scripts/shelfMonitor.js") //CG //let it be here for the moment, update with the relevant scripts later
  //     << cgicc::script() << std::endl;
  *out << "<link  rel=\"stylesheet\" href=\"/gemdaq/gemcalibration/html/css/bootstrap.css\" type=\"text/css\">" << std::endl;
  *out << "<link  rel=\"stylesheet\" href=\"/gemdaq/gemcalibration/html/css/bootstrap.min.css\" type=\"text/css\">" << std::endl;
  *out << "<script src=\"/gemdaq/gemdcalibration/html/scripts/bootstrap.min.js\"></script>" << std::endl; //CG
  *out << "<script src=\"/gemdaq/gemcalibration/html/scripts/bootstrap.js\"></script>" << std::endl; //CG
  *out << "<script src=\"/gemdaq/gemcalibration/html/scripts/jquery.min.js\"></script>" << std::endl; //CG
  // *out << "<script src=\"/gemcal/gemcalibration/html/scripts/checkAll.js\"></script>" << std::endl; //CG
  

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
  //    *out << cgicc::script().set("type", "text/javascript").set("src", "/gemdaq/gemcalibration/html/scripts/checkall.js")
 
  *out<< "<h1><span class=\"label label-info\" id=\"cal_scurve\">SCURVE INTERFACE"<< "</span></h1>" << std::endl; 
  *out<< "<p>To run the Scurve select the cards, the optohibryds, the VFATs and links. Indicate the number of events for each position and the latency and pulse stratch configuration.</p>" << std::endl; 


  *out << "<div class=\"container\"> "<< std::endl;
*out<< "<div class=\"dropdown\">" << std::endl;
 *out<< "   <button type=\"button\" class=\"btn btn-primary dropdown-toggle\" data-toggle=\"dropdown\">" << std::endl;
 *out<< "     Dropdown button" << std::endl;
*out<< "    </button>" << std::endl;
*out<< "    <ul class=\"dropdown-menu\">"  << std::endl;
*out<< "      <a class=\"dropdown-item\" href=\"#\">Link 1</a>" << std::endl;
*out<< "      <a class=\"dropdown-item\" href=\"#\">Link 2</a>" << std::endl;
 *out<< "     <a class=\"dropdown-item-text\" href=\"#\">Text Link</a>" << std::endl;
 *out<< "     <span class=\"dropdown-item-text\">Just Text</span>" << std::endl;
 *out<< "   </ul>" << std::endl;
 *out<< " </div>" << std::endl;

  

  
  /*  *out << " <table class=\"table table-borderless\"> "<< std::endl;
  *out << "  <thead>  "<< std::endl;
  *out << "  <tr> "<< std::endl;
  *out << "  <th> Shelf </th> "<< std::endl;
  *out << "  <th> AMC slot and OH mask </th> "<< std::endl;
  *out << "  </tr> "<< std::endl;
  *out << "  </thead> "<< std::endl;
  *out << "  <tbody> "<< std::endl;
    
  for (unsigned int i = 0; i < NSHELF; ++i){

    *out << "   <tr> "<< std::endl;
    *out << "   <td> "<< std::endl;
    *out << "<div class=\"checkbox\"> <label> <input type=\"checkbox\" class=\"check\" id=\"SHELF"  << std::setfill('0') << std::setw(2) << i+1 << "\">  SHELF " << std::setfill('0') << std::setw(2) << i+1 << " </label> </div>" << std::endl;
    *out << "  </td> "<< std::endl;
    *out << "  <td> "<< std::endl;

   *out << " <div class=\"dropdown\">"<< std::endl;
   *out << " <button type=\"button\" class=\"btn btn-primary dropdown-toggle\" data-toggle=\"dropdown\">Dropdown button</button>"<< std::endl;
   
    *out << "  <div class=\"dropdown-menu\">"<< std::endl;
    *out << "  <a class=\"dropdown-item\" href=\"#\">Tablet</a>" << std::endl;
    
    for (unsigned int j = 0; j < NAMC; ++j){ //SHELF.AMC
     *out << "<span class =\"dropdown-item-text\">"<< std::endl;
     *out << " <form class=\"form-inline\"> "<<std::endl;
      
       *out <<"<div class=\"checkbox\"> <label> <input type=\"checkbox\" class=\"check\" id=\"AMC" << std::setfill('0') << std::setw(2) << i+1 <<"." << std::setfill('0') << std::setw(2) << j+1 << "\">  AMC" << std::setfill('0') << std::setw(2) << j+1 << " </label> </div>"   << std::endl; 
     
       *out << " <form> <div class=\"form-group\"> <label for=\"ohMask\">OH mask:</label> <input type=\"text\" class=\"form-control\"  placeholder=\"0x000\" id=\"ohMask" << std::setfill('0') << std::setw(2) << i+1 <<"." << std::setfill('0') << std::setw(2) << j+1 << "\"> </div> </form>"<< std::endl;     

       *out << "  </form> "<< std::endl;
       *out << "  </span> "<< std::endl;    
    }
    *out << "  </td> "<< std::endl;
    *out << "  </tr> "<< std::endl;
    *out << " </div> "<< std::endl;
    *out << " </div> "<< std::endl;
    
  }
  *out << "  </tbody> "<< std::endl;
  *out << "  </table> "<< std::endl;
  */
    
  *out<< "<p>Cosa ho scritto?</p>" << std::endl; 
    


 *out << "<button class=\"btn btn-dark\" type=\"button\" onclick=\"expert_action(this.id)\" id=\"selectall\" name=\"allpyall\">SELECT ALL</button>" << std::endl;
 *out << "<button class=\"btn btn-dark\" type=\"button\" onclick=\"expert_action(this.id)\" id=\"applyall\" name=\"allpyall\">APPLY TO ALL</button>" << std::endl;
 *out << "<button class=\"btn btn-dark\" type=\"button\" onclick=\"expert_action(this.id)\" id=\"calibrate\" name=\"allpyall\">CALIBRATE</button>" << std::endl;
 
 
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
