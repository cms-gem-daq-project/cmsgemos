#include "gem/calib/CalibrationWeb.h"
#include <boost/algorithm/string.hpp>
#include "xcept/tools.h"
#include "gem/base/utils/GEMInfoSpaceToolBox.h"
#include "gem/utils/exception/Exception.h"
#include "gem/calib/Calibration.h" 
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
  *out << cgicc::script().set("type", "text/javascript")
    .set("src", "/gemdaq/gemcalibration/html/scripts/cal_routines.js") //let it be here for the moment, update with the relevant scripts later
       << cgicc::script() << std::endl;
  
  *out << "<link  rel=\"stylesheet\" href=\"/gemdaq/gemcalibration/html/css/bootstrap.css\" type=\"text/css\">" << std::endl;
  *out << "<link  rel=\"stylesheet\" href=\"/gemdaq/gemcalibration/html/css/bootstrap.min.css\" type=\"text/css\">" << std::endl;
  //*out << "<script src=\"/gemdaq/gemdcalibration/html/scripts/bootstrap.min.js\"></script>" << std::endl; //CG
  *out << "<script src=\"/gemdaq/gemcalibration/html/scripts/bootstrap.js\"></script>" << std::endl; //CG
  //*out << "<script src=\"/gemdaq/gemcalibration/html/scripts/jquery.min.js\"></script>" << std::endl; //CG

  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
      *out << "  <div class=\"xdaq-tab\" title=\"Calibration Control\">"  << std::endl;
          this->calibrationPage(in, out);
      *out << "  </div>" << std::endl;
      *out << "  <div class=\"xdaq-tab\" title=\"Monitoring page\">"  << std::endl;
          this->monitorPage(in, out);
      *out << "  </div>" << std::endl;
      *out << "  <div class=\"xdaq-tab\" title=\"Expert page\">"  << std::endl;
          this->expertPage(in, out);
      *out << "  </div>" << std::endl;
  *out << "  </div>" << std::endl;
  GEMWebApplication::webFooterGEM(in, out);

  std::string updateLink = "/" + p_gemApp->m_urn + "/";
 
  *out<<"<script type=\"text/javascript\">"           <<std::endl
      <<"store_actionURL(\""<<updateLink<<"\");"<<std::endl
      <<"</script>"<<std::endl;
}

void gem::calib::CalibrationWeb::calibrationPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
    CMSGEMOS_DEBUG("CalibrationWeb::calibrationPage");
    *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
        *out << "<div align=\"center\">" << std::endl;
            *out<< "<form id=\"cal_select\">"<<std::endl;   
                *out << "<div class=\"form-group row\">" << std::endl;
                    *out << "<h2><span class=\"label label-info col-md-3\" id=\"cal_scurve\">Select calibration type:" << "</span></h2>" << std::endl; 
                    *out << "<div class=\"col-md-3\">" << std::endl;
                    *out << "<select class=\"form-control form-control-lg\" id=\"cal_type_select\" name=\"cal_type_select\" onChange=\"selectCalType()\">" << std::endl;
                        *out << "<option disabled selected value> -- select an option -- </option>" << std::endl;
                        *out << "<option>Phase Scan</option>" << std::endl;
                        *out << "<option>Latency Scan</option>" << std::endl;
                        *out << "<option>S-curve Scan</option>" << std::endl;
                        *out << "<option>S-bit Rate Scan</option>" << std::endl;
                        *out << "<option disabled value>Whatever else...</option>" << std::endl;
                    *out << "</select>" << std::endl;
                    *out << "</div>" << std::endl; //<div class=\"col-md-6\">
                *out << "</div>" << std::endl; //<div class=\"form-group row\">
            *out<<"</form>"<< std::endl;
        *out << "</div>" << std::endl;
    *out << "</div>" << std::endl;

    *out << "<div class=\"container col-md-12\" id=\"cal_interface\">" << std::endl;
    *out << "</div>" << std::endl;
}

void gem::calib::CalibrationWeb::applicationPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CalibrationWeb::applicationPage : Do nothing for the moment, will be eventually filled later");
  //*out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  //  
  //*out << "</div>" << std::endl;
}

void gem::calib::CalibrationWeb::phaseInterface(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<div id=\"cal_interface\">" << std::endl;
        *out<<"<h1><span class=\"label label-danger\">PHASE SCAN INTERFACE... TBI"<< "</span></h1>"<<std::endl; 
    *out << "</div>" << std::endl;
}

void gem::calib::CalibrationWeb::latencyInterface(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<div id=\"cal_interface\">" << std::endl;
        *out<<"<h1><span class=\"label label-danger\">LATENCY SCAN INTERFACE... TBI"<< "</span></h1>"<<std::endl; 
    *out << "</div>" << std::endl;
}

void gem::calib::CalibrationWeb::triggerSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out<<"<form id=\"trigger_select\">"<< std::endl;
    *out << "<div class=\"form-group row\">"
        <<"<h2><span class=\"label label-warning col-md-6\">TRIGGER SOURCE"<< "</span></h2>"
        << "<div class=\"col-md-6\">"
            << "<div class=\"form-check\">"
                <<"<div class=\"form-check-input radio-inline\"> <label><input type=\"radio\" name=\"trig_radio\" id=\"trig_radio_0\" value=0>TTC</label></div>"
                <<"<div class=\"form-check-input radio-inline\"> <label><input type=\"radio\" name=\"trig_radio\" id=\"trig_radio_1\" value=1 checked>Loopback</label></div>"
                <<"<div class=\"form-check-input radio-inline\"> <label><input type=\"radio\" name=\"trig_radio\" id=\"trig_radio_2\" value=2>Lemo/T3</label></div>"
            << "</div>"
        << "</div>"
    << "</div>" << std::endl;    
    *out<<"</form>"<< std::endl;
}

void gem::calib::CalibrationWeb::nSamplesSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out<<"<form id=\"n_samples_select\">"<< std::endl;
    *out << "<div class=\"form-group row\">"
        <<"<h2><span class=\"label label-success col-md-6\">Number of samples"<< "</span></h2>"
        << "<div class=\"col-md-6\">"
            << "<input type=\"text\" value=\"100\" class=\"form-control\" name=\"n_samples\">"
        << "</div>"
    << "</div>" << std::endl;    
    *out<<"</form>"<< std::endl;
}

void gem::calib::CalibrationWeb::L1AtimeSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out<<"<form id=\"L1A_time_select\">"<< std::endl;
    *out << "<div class=\"form-group row\">"
        <<"<h2><span class=\"label label-success col-md-6\">L1A time"<< "</span></h2>"
        << "<div class=\"col-md-6\">"
            << "<input type=\"text\" value=\"250\" class=\"form-control\" name=\"L1A_time\">"
        << "</div>"
    << "</div>" << std::endl;    
    *out<<"</form>"<< std::endl;
}

void gem::calib::CalibrationWeb::latencySelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out<<"<form id=\"latency_select\">"<< std::endl;
    *out << "<div class=\"form-group row\">"
        <<"<h2><span class=\"label label-success col-md-6\">Latency"<< "</span></h2>"
        << "<div class=\"col-md-6\">"
            << "<input type=\"text\" value=\"33\" class=\"form-control\" name=\"latency\">"
        << "</div>"
    << "</div>" << std::endl;    
    *out<<"</form>"<< std::endl;
}

void gem::calib::CalibrationWeb::pulseDelaySelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out<<"<form id=\"pulseDelay_select\">"<< std::endl;
    *out << "<div class=\"form-group row\">"
        <<"<h2><span class=\"label label-success col-md-6\">Pulse Delay"<< "</span></h2>"
        << "<div class=\"col-md-6\">"
            << "<input type=\"text\" value=\"40\" class=\"form-control\" name=\"pulseDelay\">"
        << "</div>"
    << "</div>" << std::endl;    
    *out<<"</form>"<< std::endl;
}
void gem::calib::CalibrationWeb::CalPhaseSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out<<"<form id=\"CalPhase_select\">"<< std::endl;
    *out << "<div class=\"form-group row\">"
        <<"<h2><span class=\"label label-success col-md-6\">Calpulse phase"<< "</span></h2>"
        << "<div class=\"col-md-6\">"
            << "<input type=\"text\" value=\"0\" class=\"form-control\" name=\"CalPhase\">"
        << "</div>"
    << "</div>" << std::endl;    
    *out<<"</form>"<< std::endl;
}

void gem::calib::CalibrationWeb::vfatChMinSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out<<"<form id=\"vfatChMin_select\">"<< std::endl;
    *out << "<div class=\"form-group row\">"
        <<"<h2><span class=\"label label-success col-md-6\">VFAT channel Min"<< "</span></h2>"
        << "<div class=\"col-md-6\">"
            << "<input type=\"text\" value=\"0\" class=\"form-control\" name=\"vfatChMin\">"
        << "</div>"
    << "</div>" << std::endl;    
    *out<<"</form>"<< std::endl;
}

void gem::calib::CalibrationWeb::vfatChMaxSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out<<"<form id=\"vfatChMax_select\">"<< std::endl;
    *out << "<div class=\"form-group row\">"
        <<"<h2><span class=\"label label-success col-md-6\">VFAT channel Max"<< "</span></h2>"
        << "<div class=\"col-md-6\">"
            << "<input type=\"text\" value=\"127\" class=\"form-control\" name=\"vfatChMax\">"
        << "</div>"
    << "</div>" << std::endl;    
    *out<<"</form>"<< std::endl;
}

void gem::calib::CalibrationWeb::slotsAndMasksSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<form id=\"slot_and_masks_select\">" << std::endl;
        *out << "<div class=\"panel panel-default\">"
            << "<div class=\"panel-heading\">"
                << "<div class=\"row\">"
                    << "<div class=\"col-md-6\">"
                        << "<h4>SHELF</h4>"
                    << "</div>"
                    << "<div class=\"col-md-6\">"
                        << "<h4>AMC slot and OH mask</h4>"
                    << "</div>"
                << "</div>" 
            << "</div>" << std::endl; // end panel heaing
            // panel body
            *out << "<div class=\"container\" id=\"links_selection\">" << std::endl;
                for (unsigned int i = 0; i < NSHELF; ++i) {
                    *out << "<div class=\"row\">"
                        << "<div class=\"col-md-3\">"
                            << "<div class=\"checkbox\">" 
                                << "<label> <input type=\"checkbox\" class=\"check\" id=\"SHELF"
                                << std::setfill('0') << std::setw(2) << i+1 << "\">  SHELF " 
                                << std::setfill('0') << std::setw(2) << i+1 << " </label>"
                            << " </div>"
                        << "</div>"
                        << "<div class=\"col-md-9\">"
                            << "<div class=\"dropdown\">"
                                <<"<button id=\"amc_dropdown_button\" class=\"btn btn-lg btn-outline dropdown-toggle\" data-toggle=\"dropdown\">Select AMC and OH</button>"
                                <<"<div class=\"dropdown-menu\">" << std::endl;
                                    for (unsigned int j = 0; j < NAMC; ++j) { //SHELF.AMC
                                        *out << "<span class =\"dropdown-item-text\">"
                                            <<"<div class=\"row\">"
                                                <<"<div class=\"col-md-4\">"
                                                    << "<div class=\"checkbox\"><label><input type=\"checkbox\" class=\"check\" id=\"AMC"
                                                    << std::setfill('0') << std::setw(2) << i+1 << "." << std::setfill('0') << std::setw(2) << j+1 <<"\">  AMC"
                                                    << std::setfill('0') << std::setw(2) << j+1 << " </label> </div>"
                                                << "</div>"
                                                <<"<div class=\"col-md-4\">"
                                                    << "<label for=\"ohMask\">OH mask:</label>"
                                                << "</div>"
                                                <<"<div class=\"col-md-4\">"
                                                    << "<input type=\"text\" value=\"0x000\" id=\"ohMask"
                                                    << std::setfill('0') << std::setw(2) << i+1 << "." << std::setfill('0') << std::setw(2) << j+1<<"\">"
                                                << "</div>"
                                            << "</div>"
                                        << "</span>" << std::endl;
                                    } // end loop over NAMC 
                                *out << "</div>"
                            << "</div>" // end <div class="dropdown">
                        << "</div>" // end <div class="col">
                    << "</div>" << std::endl; // end <div class="row">
                } // end loop over NSHELF
            *out << "</div>" << std::endl; // end container
        *out << "</div>" << std::endl; // end panel
    *out << "</form>"<< std::endl;
    *out<<"<div align=\"center\">"<<std::endl;
        *out << "<button class=\"btn btn-lg btn-info\" onclick=\"select_links()\">SELECT ALL</button>" << std::endl;
    *out<<"</div>"<< std::endl;
}

void gem::calib::CalibrationWeb::scurveInterface(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<div class=\"container col-md-12\" id=\"cal_interface\">" << std::endl;
        *out <<"<div align=\"center\">" << std::endl;
            *out <<"<h3><span class=\"label label-danger\" id=\"cal_type\" name=\"cal_type\">To run the routine select the cards, the optohybrids, the VFATs and links. Indicate the number of events for each position and the latency and pulse stratch configuration."<< "</span></h3>" << std::endl; 
        *out << "</div>" << std::endl;
        *out << "<div class=\"row\">" << std::endl;
            *out << "<div class=\"col-md-6\">" << std::endl;
                this->triggerSelector(out);
	        *out<<"<div align=\"center\">"<< std::endl;
		    *out<<"<h2><span class=\"label label-primary\">SETTINGS"<< "</span></h2>"<<std::endl; 
		*out<<"</div>"<<std::endl;
		this->nSamplesSelector(out);
                this->L1AtimeSelector(out);
		this->latencySelector(out);
		this->pulseDelaySelector(out);
		this->CalPhaseSelector(out);
		this->vfatChMinSelector(out);
		this->vfatChMaxSelector(out);
             *out<< "</div>" << std::endl;
	     *out<< "<div class=\"col-md-6\">" << std::endl;
                this->slotsAndMasksSelector(out);
		*out<< "<br><br><br><br><br><br><br><br>" << std::endl;
		*out<< "<div align=\"center\">" << std::endl;
		    *out<<"<button class=\"btn btn-lg btn-info\" type=\"button\" onclick=\"apply_action()\" id=\"apply\" name=\"apply_all\">APPLY SETTINGS</button>"<<std::endl;
		    *out << "<button class=\"btn btn-lg btn-success\" onclick=\"run_scan()\" id=\"run_button\" name=\"run_button\" disabled>RUN</button>"<<std::endl;
		*out << "</div>"<< std::endl;	
	     *out << "</div>" << std::endl;
	/*     
        *out<< "</div>" << std::endl;
        *out<< "<form id=\"scurve_input_params\">"<<std::endl;
        *out<<"<div align=\"center\">"<< std::endl;
            *out<<"<h2><span class=\"label label-primary\">SETTINGS"<< "</span></h2>"<<std::endl; 
	*out<<"</div>"<<std::endl;
        *out << "<div class=\"form-group row\">"
            << "<div class=\"col-md-4\">"
                << "<label for=\"L1Atime\">L1Atime:</label>"
                << "<input type=\"text\" value=\"250\" class=\"form-control\" id=\"L1Atime\">"
            << "</div>"
            << "<div class=\"col-md-4\">"
                << "<label for=\"Latency\">Latency:</label>"
                << "<input type=\"text\" value=\"33\" class=\"form-control\" id=\"Latency\">"
            << "</div>"
            << "<div class=\"col-md-4\">"
                << "<label for=\"PulseDelay\">Pulse delay:</label>"
                << "<input type=\"text\" value=\"40\" class=\"form-control\" id=\"PulseDelay\">"
            << "</div>"
        << "</div>"
        << "<div class=\"form-group row\">"
            << "<div class=\"col-md-4\">"
                << "<label for=\"CalPhase\">Calpulse phase:</label>"
                << "<input type=\"text\" value=\"0\" class=\"form-control\" id=\"CalPhase\">"
            << "</div>"
            << "<div class=\"col-md-4\">"
                <<"<label for=\"vfatChMin\">VFAT channel Min:</label>"
                <<"<input type=\"text\" value=\"0\" class=\"form-control\" id=\"vfatChMin\">"
            << "</div>"
            << "<div class=\"col-md-4\">"
                << "<label for=\"vfatChMax\">VFAT channel Max:</label>"
                << "<input type=\"text\" value=\"127\" class=\"form-control\" id=\"vfatChMax\">"
            << "</div>"
        << "</div>" << std::endl;
        *out<< "</form>"<<std::endl;
        
        *out << "<div align=\"left\">" << std::endl;
            *out<<"<button class=\"btn btn-lg btn-info\" type=\"button\" onclick=\"apply_action()\" id=\"apply\" name=\"apply_all\">APPLY SETTINGS</button>"<<std::endl;
            *out << "<button class=\"btn btn-lg btn-success\" onclick=\"run_scan()\" id=\"run_button\" name=\"run_button\" disabled>RUN</button>"<<std::endl;
        *out << "</div>"<< std::endl;
        */
    *out << "</div>" << std::endl;
}

void gem::calib::CalibrationWeb::sbitRateInterface(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<div id=\"cal_interface\">" << std::endl;
        *out<<"<h1><span class=\"label label-danger\">SBit Rate SCAN INTERFACE... TBI"<< "</span></h1>"<<std::endl; 
    *out << "</div>" << std::endl;
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
 

}

void gem::calib::CalibrationWeb::monitorPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{

/*
  CMSGEMOS_DEBUG("CalibrationWeb::LatencyScanPage");
  *out<<"<div class=\"xdaq-tab-wrapper\">" << std::endl;
  //*out << "<div align=\"center\">" << std::endl;
  //*out<< "<h1><span class=\"label label-info\" id=\"mon_state\">MONITORING STATE: "
  //    << dynamic_cast<gem::calib::Calibration*>(p_gemApp)->monitoringState() << "</span></h1>" << std::endl; //CG
  //    *out << cgicc::script().set("type", "text/javascript").set("src", "/gemdaq/gemcalibration/html/scripts/checkall.js")
 
  *out<<"<h1><span class=\"label label-info\" id=\"cal_scurve\">LATENCY SCAN INTERFACE"<< "</span></h1>"<<std::endl; 
  *out<<"<p> To run the latency scan select the cards, the optohybrids, the VFATs and links. Indicate the number of events for each position and the latency and pulse stratch configuration.</p>"<<std::endl;
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
  *out<<"<input type=\"text\" value=\"250\" class=\"form-control\" id=\"L1Atime\">"<< std::endl;
  *out<<"</td>"<< std::endl;*out<<"<td>"<<std::endl;
  *out<<"<label for=\"Latency\">Latency:</label>"<<std::endl;
  *out<<"<input type=\"text\" value=\"33\" class=\"form-control\" id=\"Latency\">"<<std::endl;
  *out<<"</td> "<< std::endl;*out<<"<td> "<< std::endl;
  *out<<"<label for=\"PulseDelay\">Pulse delay:</label>"<<std::endl;
  *out<<"<input type=\"text\" value=\"40\" class=\"form-control\" id=\"PulseDelay\">"<<std::endl;
  *out<<"</td>"<< std::endl;
  *out<<"</tr>"<< std::endl;*out<<"<tr>"<< std::endl;
  *out<<"<td>"<< std::endl;
  *out<<"<label for=\"CalPhase\">Calpulse phase:</label>"<<std::endl;
  *out<<"<input type=\"text\" value=\"0\" class=\"form-control\" id=\"CalPhase\">"<<std::endl;
  *out<<"</td>"<<std::endl;*out<<"<td>"<< std::endl;
  // *out<< "<br>"<<std::endl;
  *out<<"<label for=\"scanMin\">Scan Min:</label>"<<std::endl;
  *out<<"<input type=\"text\" value=\"153\" class=\"form-control\" id=\"scanMin\">"<<std::endl;
  *out<<"</td>"<< std::endl;*out<<"<td>"<< std::endl;
  *out<<"<label for=\"scanMax\">Scan Max:</label>"<<std::endl;
  *out<<"<input type=\"text\" value=\"172\" class=\"form-control\" id=\"scanMax\">"<<std::endl;
   *out<<"</td>"<< std::endl;
  *out<<"</tr>"<< std::endl;*out<<"<tr>"<< std::endl;
  *out<<"<td>"<< std::endl;
  *out<<"<label for=\"NSamples\">Number of samples:</label>"<<std::endl;
  *out<<"<input type=\"text\" value=\"100\" class=\"form-control\" id=\"NSamples\">"<<std::endl;
  *out<<"</td>"<< std::endl;*out<<"<td>"<< std::endl;
  *out<<"<label for=\"throttle\">Throttle:</label>"<<std::endl;
  *out<<"<input type=\"text\" value=\"100\" class=\"form-control\" id=\"throttle\">"<<std::endl;

  *out<<"</td>"<< std::endl;*out<<"<td>"<< std::endl;
 *out<<"<label for=\"vt2\">VThreshold2:</label>"<<std::endl;
  *out<<"<input type=\"text\" value=\"0\" class=\"form-control\" id=\"vt2\">"<<std::endl;

  
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
     
      *out<<"<form> <div class=\"form-group\"><label for=\"ohMask\">OH mask:</label> <input type=\"text\" class=\"form-control\"  value=\"0x000\" id=\"ohMask"<<std::setfill('0')<<std::setw(2)<<i+1 <<"."<<std::setfill('0')<<std::setw(2)<<j+1<<"\"> </div> </form>"<< std::endl;     

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
 

*/  
}

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
