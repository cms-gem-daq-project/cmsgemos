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
    .set("src", "/gemdaq/gemcalibration/html/scripts/cal_routines.js") 
       << cgicc::script() << std::endl;
  
  *out << "<link  rel=\"stylesheet\" href=\"/gemdaq/gemcalibration/html/css/bootstrap.css\" type=\"text/css\">" << std::endl;
  *out << "<link  rel=\"stylesheet\" href=\"/gemdaq/gemcalibration/html/css/bootstrap.min.css\" type=\"text/css\">" << std::endl;
  *out << "<script src=\"/gemdaq/gemcalibration/html/scripts/bootstrap.js\"></script>" << std::endl;
  *out << "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>" << std::endl;

  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
      *out << "<div class=\"xdaq-tab\" title=\"Calibration Control\">" << std::endl;
          this->calibrationPage(in, out);
      *out << "</div>" << std::endl;
      *out << "<div class=\"xdaq-tab\" title=\"Monitoring page\">" << std::endl;
          this->monitorPage(in, out);
      *out << "</div>" << std::endl;
      *out << "<div class=\"xdaq-tab\" title=\"Expert page\">" << std::endl;
          this->expertPage(in, out);
      *out << "</div>" << std::endl;
  *out << "</div>" << std::endl;
  GEMWebApplication::webFooterGEM(in, out);

  std::string updateLink = "/" + p_gemApp->m_urn + "/";
 
  *out << "<script type=\"text/javascript\">"           << std::endl
      << "store_actionURL(\"" << updateLink << "\");"<< std::endl
      << "</script>"<< std::endl;
}

void gem::calib::CalibrationWeb::calibrationPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
    CMSGEMOS_DEBUG("CalibrationWeb::calibrationPage");
    *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
        *out << "<div align=\"center\">" << std::endl;
            *out << "<form id=\"cal_select\">"<< std::endl; *out << "<div class=\"form-group row\">" << std::endl;
                    *out << "<h2><span class=\"label label-info col-md-3\" id=\"cal_scurve\">Select calibration type:" << "</span></h2>" << std::endl; 
                    *out << "<div class=\"col-md-3\">" << std::endl;
                    *out << "<select class=\"form-control form-control-lg\" id=\"cal_type_select\" name=\"cal_type_select\" onChange=\"selectCalType()\">" << std::endl;
                        *out << "<option disabled selected value> -- select an option -- </option>" << std::endl;
                        *out << "<option>Phase Scan</option>" << std::endl;
                        *out << "<option>Latency Scan</option>" << std::endl;
                        *out << "<option>S-curve Scan</option>" << std::endl;
                        *out << "<option>S-bit Rate Scan</option>" << std::endl;
			*out << "<option>Threshold DAC Scan</option>" << std::endl;
			*out << "<option>DAC trimming</option>" << std::endl;
			*out << "<option>DAC Scan on VFAT3</option>" << std::endl;
			*out << "<option>Temperature Scan</option>" << std::endl;
			*out << "<option>SbitReadOut Scan</option>" << std::endl;
			*out << "<option>Sbit Map And Rate Scan</option>" << std::endl;
                        *out << "<option disabled value>Whatever else...</option>" << std::endl;
                    *out << "</select>" << std::endl;
                    *out << "</div>" << std::endl; //<div class=\"col-md-6\">
                *out << "</div>" << std::endl; //<div class=\"form-group row\">
            *out << "</form>"<< std::endl;
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

void gem::calib::CalibrationWeb::triggerSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-warning col-md-6\">TRIGGER SOURCE"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<div class=\"form-check\">"
            << "<div class=\"form-check-input radio-inline\"> <label><input type=\"radio\" name=\"trigType\" id=\"trig_radio_0\" value=0>TTC</label></div>"
            << "<div class=\"form-check-input radio-inline\"> <label><input type=\"radio\" name=\"trigType\" id=\"trig_radio_1\" value=1 checked>Loopback</label></div>"
            << "<div class=\"form-check-input radio-inline\"> <label><input type=\"radio\" name=\"trigType\" id=\"trig_radio_2\" value=2>Lemo/T3</label></div>"
        << "</div>"
    << "</div>" << std::endl;
}

void gem::calib::CalibrationWeb::comparatorSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-warning col-md-6\">COMPARATOR TYPE"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<div class=\"form-check\">"
            << "<div class=\"form-check-input radio-inline\"> <label><input type=\"radio\" name=\"comparator_radio\" id=\"comparator_radio_0\" value=0>Arming comparator</label></div>"
            << "<div class=\"form-check-input radio-inline\"> <label><input type=\"radio\" name=\"comparator_radio\" id=\"comparator_radio_1\" value=1 checked>CFD</label></div>"
        << "</div>"
    << "</div>" << std::endl;    
}

void gem::calib::CalibrationWeb::nSamplesSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">Number of samples"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"100\" class=\"form-control\" name=\"nSamples\">"
    << "</div>" << std::endl;
}

void gem::calib::CalibrationWeb::l1aTimeSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">L1A time"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"250\" class=\"form-control\" name=\"l1aTime\">"
    << "</div>" << std::endl;
}

void gem::calib::CalibrationWeb::latencySelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">Latency"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"33\" class=\"form-control\" name=\"latency\">"
    << "</div>" << std::endl;    
}

void gem::calib::CalibrationWeb::pulseDelaySelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">Pulse Delay"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"40\" class=\"form-control\" name=\"pulseDelay\">"
    << "</div>" << std::endl;    
}
void gem::calib::CalibrationWeb::calPhaseSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">Calpulse phase"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"0\" class=\"form-control\" name=\"calPhase\">"
    << "</div>" << std::endl;    
}

void gem::calib::CalibrationWeb::vfatChMinSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">VFAT channel Min"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"0\" class=\"form-control\" name=\"vfatChMin\">"
    << "</div>" << std::endl;    
}

void gem::calib::CalibrationWeb::vfatChMaxSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">VFAT channel Max"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"127\" class=\"form-control\" name=\"vfatChMax\">"
    << "</div>" << std::endl;    
}

void gem::calib::CalibrationWeb::scanMinSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">Scan Min"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"153\" class=\"form-control\" name=\"scanMin\">"
    << "</div>" << std::endl;    
}

void gem::calib::CalibrationWeb::scanMaxSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">Scan Max"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"172\" class=\"form-control\" name=\"scanMax\">"
    << "</div>" << std::endl;    
}
void gem::calib::CalibrationWeb::throttleSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">Throttle"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"100\" class=\"form-control\" name=\"throttle\">"
    << "</div>" << std::endl;    
}

void gem::calib::CalibrationWeb::vt2ThresholdSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">V2 Threshold"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"0\" class=\"form-control\" name=\"vt2\">"
    << "</div>" << std::endl;    
}

void gem::calib::CalibrationWeb::msplSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">MSPL"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"4\" class=\"form-control\" name=\"mspl\">"
    << "</div>" << std::endl;    
}

void gem::calib::CalibrationWeb::timeIntervalSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">Time interval"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"0\" class=\"form-control\" name=\"timeInterval\">"
    << "</div>" << std::endl;
}

void gem::calib::CalibrationWeb::timeAcquisitionSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">Acquire time"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"60\" class=\"form-control\" name=\"acquireTime\">"
    << "</div>" << std::endl;
}

void gem::calib::CalibrationWeb::timemsSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">Time interval(ms)"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"1\" class=\"form-control\" name=\"timemsInterval\">"
    << "</div>" << std::endl;    
}

void gem::calib::CalibrationWeb::rateArraySelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">Rates"<< "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"10,2\" class=\"form-control\" name=\"rateArray\">"
    << "</div>" << std::endl;    
}

void gem::calib::CalibrationWeb::slotsAndMasksSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
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
        *out << "<form id=\"slot_and_masks_select\">" << std::endl;
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
                                << "<button id=\"amc_dropdown_button\" class=\"btn btn-lg btn-outline dropdown-toggle\" data-toggle=\"dropdown\">Select AMC and OH</button>"
                                << "<div class=\"dropdown-menu pre-scrollable\">" << std::endl;
                                    for (unsigned int j = 0; j < NAMC; ++j) { //SHELF.AMC
                                        *out << "<span class =\"dropdown-item-text\">"
                                            << "<div class=\"row\">"
                                                << "<div class=\"col-md-3\">"
                                                    << "<div class=\"checkbox\"><label><input type=\"checkbox\" class=\"check\" id=\"AMC"
                                                    << std::setfill('0') << std::setw(2) << i+1 << "." << std::setfill('0') << std::setw(2) << j+1 << "\">  AMC"
                                                    << std::setfill('0') << std::setw(2) << j+1 << " </label> </div>"
                                                << "</div>"
                                                << "<div class=\"col-md-1\">"
                                                << "</div>"
                                                << "<div class=\"col-md-8\">"
                                                    << "<input type=\"text\" value=\"0x000\" size=\"4\" id=\"ohMask"
                                                    << std::setfill('0') << std::setw(2) << i+1 << "." << std::setfill('0') << std::setw(2) << j+1<< "\">OH mask </input>"
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
        *out << "</form>"<< std::endl;
    *out << "<div align=\"center\">"<< std::endl;
        *out << "<button class=\"btn btn-lg btn-info\" onclick=\"select_links()\">SELECT ALL</button>" << std::endl;
        *out << "<button class=\"btn btn-lg btn-warning\" onclick=\"deselect_links()\">DESELECT ALL</button>" << std::endl;
    *out << "</div>"<< std::endl;
    *out << "</div>" << std::endl; // end panel
}

void gem::calib::CalibrationWeb::phaseInterface(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<div id=\"cal_interface\">" << std::endl;
        *out << "<h1><span class=\"label label-danger\">PHASE SCAN INTERFACE... TBI"<< "</span></h1>"<< std::endl; 
    *out << "</div>" << std::endl;
}

void gem::calib::CalibrationWeb::latencyInterface(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<div id=\"cal_interface\">" << std::endl;
	    *out << "<div align=\"center\">" << std::endl;
            *out << "<div class=\"alert alert-warning\">"
                << "<button type=\"button\" class=\"close\" data-dismiss=\"alert\">&times;</button>"
                << "To run the routine select the cards, the optohybrids, the VFATs and links, indicate the number of events " 
                << "for each position, the throttle, the  pulse length configuration, the minimum and maximum scan values, and the v2 threshold." 
            << "</div>" << std::endl;
        *out << "</div>" << std::endl;
        *out << "<div class=\"row\">" << std::endl;
            *out << "<form id=\"settings_select\">" << std::endl;
                *out << "<div class=\"col-md-6\">" << std::endl;
                    this->triggerSelector(out);
	                *out << "<br><br><br>" << std::endl;
		            this->nSamplesSelector(out);
                    this->l1aTimeSelector(out);
		            this->msplSelector(out);
		            this->calPhaseSelector(out);
		            this->vfatChMinSelector(out);
		            this->vfatChMaxSelector(out);
                    this->scanMinSelector(out);
		            this->scanMaxSelector(out);
                    this->throttleSelector(out);
		            this->vt2ThresholdSelector(out);
                *out << "</div>" << std::endl;
            *out << "</form>"<< std::endl;
            *out << "<div class=\"col-md-6\">" << std::endl;
	            *out << "<br>" << std::endl;
                this->slotsAndMasksSelector(out);
		        *out << "<br><br><br>" << std::endl;
		        *out << "<div align=\"center\">" << std::endl;
		            *out << "<button class=\"btn btn-lg btn-info\" type=\"button\" onclick=\"apply_action()\" id=\"apply\" name=\"apply_all\">APPLY SETTINGS</button>"<< std::endl;
		            *out << "<button class=\"btn btn-lg btn-success\" onclick=\"run_scan()\" id=\"run_button\" name=\"run_button\" disabled>RUN</button>"<< std::endl;
		        *out << "</div>"<< std::endl;	
            *out << "</div>" << std::endl;
        *out << "</div>" << std::endl;
    *out << "</div>" << std::endl;
    *out << "<br>" << std::endl;
}

void gem::calib::CalibrationWeb::thresholdInterface(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<div id=\"cal_interface\">" << std::endl;
        *out << "<div align=\"center\">" << std::endl;
            *out << "<div class=\"alert alert-warning\">"
                << "<button type=\"button\" class=\"close\" data-dismiss=\"alert\">&times;</button>"
                << "To run the routine select the cards, the optohybrids, the VFATs and links, indicate the number of events"
                << "for each position, the minimum and maximum scan values, and the v2 threshold."
            << "</div>" << std::endl;
        *out << "</div>" << std::endl;
        *out << "<div class=\"row\">" << std::endl;
            *out << "<form id=\"settings_select\">" << std::endl;
                *out << "<div class=\"col-md-6\">" << std::endl;
                    this->triggerSelector(out);
	                *out << "<br><br><br>" << std::endl;
                    this->nSamplesSelector(out);
                    this->l1aTimeSelector(out);// TODO: need to be removed
                    this->latencySelector(out);// TODO: need to be removed
                    this->pulseDelaySelector(out);// TODO: need to be removed
                    this->calPhaseSelector(out);// TODO: need to be removed
                    this->vfatChMinSelector(out);
                    this->vfatChMaxSelector(out);
                    this->scanMinSelector(out);// TODO: need to be removed
                    this->scanMaxSelector(out);// TODO: need to be removed
                    this->throttleSelector(out);// TODO: need to be removed
                    this->vt2ThresholdSelector(out);//TODO: need to be removed
                    this->msplSelector(out);//TODO: need to be removed
		            this->timeIntervalSelector(out);// TODO: need to be removed
                *out << "</div>" << std::endl;
            *out << "</form>"<< std::endl;
            *out << "<div class=\"col-md-6\">" << std::endl;
	            *out << "<br>" << std::endl;
                this->slotsAndMasksSelector(out);
                *out << "<br><br><br>" << std::endl;
                *out << "<div align=\"center\">" << std::endl;
                    *out << "<button class=\"btn btn-lg btn-info\" type=\"button\" onclick=\"apply_action()\" id=\"apply\" name=\"apply_all\">APPLY SETTINGS</button>"<< std::endl;
                    *out << "<button class=\"btn btn-lg btn-success\" onclick=\"run_scan()\" id=\"run_button\" name=\"run_button\" disabled>RUN</button>"<< std::endl;
                *out << "</div>"<< std::endl;   
            *out << "</div>" << std::endl;
        *out << "</div>" << std::endl;
    *out << "</div>" << std::endl;
    *out << "<br>" << std::endl;
}

void gem::calib::CalibrationWeb::trimDACInterface(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<div id=\"cal_interface\">" << std::endl;
        *out << "<div align=\"center\">" << std::endl;
            *out << "<div class=\"alert alert-warning\">"
                << "<button type=\"button\" class=\"close\" data-dismiss=\"alert\">&times;</button>"
                << "To run the routine select the cards, the optohybrids."
            << "</div>" << std::endl;
        *out << "</div>" << std::endl;
        *out << "<div class=\"row\">" << std::endl;
            *out << "<form id=\"settings_select\">" << std::endl;
                *out << "<div class=\"col-md-6\">" << std::endl;
                    this->triggerSelector(out);
                    *out << "<br><br><br>" << std::endl;
                    this->nSamplesSelector(out);
                    this->l1aTimeSelector(out);// TODO: need to be removed
                    this->latencySelector(out);// TODO: need to be removed
                    this->pulseDelaySelector(out);// TODO: need to be removed
                    this->calPhaseSelector(out);// TODO: need to be removed
                    this->vfatChMinSelector(out);
                    this->vfatChMaxSelector(out);
                    this->scanMinSelector(out);// TODO: need to be removed
                    this->scanMaxSelector(out);// TODO: need to be removed
                    this->throttleSelector(out);// TODO: need to be removed
                    this->vt2ThresholdSelector(out);//TODO: need to be removed
                    this->msplSelector(out);//TODO: need to be removed
                    this->timeIntervalSelector(out);// TODO: need to be removed
                *out << "</div>" << std::endl;
            *out << "</form>"<< std::endl;
            *out << "<div class=\"col-md-6\">" << std::endl;
                 *out << "<br>" << std::endl;
                 this->slotsAndMasksSelector(out);
                 *out << "<br><br><br>" << std::endl;
                 *out << "<div align=\"center\">" << std::endl;
                     *out << "<button class=\"btn btn-lg btn-info\" type=\"button\" onclick=\"apply_action()\" id=\"apply\" name=\"apply_all\">APPLY SETTINGS</button>"<< std::endl;
                     *out << "<button class=\"btn btn-lg btn-success\" onclick=\"run_scan()\" id=\"run_button\" name=\"run_button\" disabled>RUN</button>"<< std::endl;
                *out << "</div>"<< std::endl;    
            *out << "</div>" << std::endl;
        *out << "</div>" << std::endl;
    *out << "</div>" << std::endl;     
    *out << "<br>" << std::endl;
}

void gem::calib::CalibrationWeb::scurveInterface(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<div id=\"cal_interface\">" << std::endl;
        *out << "<div align=\"center\">" << std::endl;
            *out << "<div class=\"alert alert-warning\">"
                << "<button type=\"button\" class=\"close\" data-dismiss=\"alert\">&times;</button>"
                << " To run the routine select the cards, the optohybrids, the VFATs and links."
                << " Indicate the number of events for each position and the latency and pulse stratch configuration."
            << "</div>" << std::endl;
        *out << "</div>" << std::endl;
        *out << "<div class=\"row\">" << std::endl;
            *out << "<form id=\"settings_select\">" << std::endl;
                *out << "<div class=\"col-md-6\">" << std::endl;
                    this->triggerSelector(out);
	                *out << "<br><br><br>" << std::endl;
                    this->nSamplesSelector(out);
                    this->l1aTimeSelector(out);
		            this->latencySelector(out);
		            this->pulseDelaySelector(out);
		            this->calPhaseSelector(out);
		            this->vfatChMinSelector(out);
		            this->vfatChMaxSelector(out);
                *out << "</div>" << std::endl;
            *out << "</form>"<< std::endl;
            *out << "<div class=\"col-md-6\">" << std::endl;
	            *out << "<br>" << std::endl;
                this->slotsAndMasksSelector(out);
		        *out << "<br><br><br>" << std::endl;
		        *out << "<div align=\"center\">" << std::endl;
		            *out << "<button class=\"btn btn-lg btn-info\" type=\"button\" onclick=\"apply_action()\" id=\"apply\" name=\"apply_all\">APPLY SETTINGS</button>"<< std::endl;
		            *out << "<button class=\"btn btn-lg btn-success\" onclick=\"run_scan()\" id=\"run_button\" name=\"run_button\" disabled>RUN</button>"<< std::endl;
		        *out << "</div>"<< std::endl;	
	        *out << "</div>" << std::endl;
	    *out << "</div>" << std::endl;
    *out << "</div>" << std::endl;
    *out << "<br>" << std::endl;
}

void gem::calib::CalibrationWeb::sbitRateInterface(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<div class=\"container col-md-12\" id=\"cal_interface\">" << std::endl;
        *out << "<div align=\"center\">" << std::endl;
            *out << "<div class=\"alert alert-warning\">"
                << "<button type=\"button\" class=\"close\" data-dismiss=\"alert\">&times;</button>"
                << "To run the routine select the cards, the optohybrids, the VFATs and links."
            << "</div>" << std::endl;
        *out << "</div>" << std::endl;
        *out << "<div class=\"row\">" << std::endl;
            *out << "<form id=\"settings_select\">" << std::endl;
                *out << "<div class=\"col-md-6\">" << std::endl;
                    this->triggerSelector(out);
                    *out << "<br><br><br>" << std::endl;
                    this->nSamplesSelector(out);// TODO: need to be removed
                    this->l1aTimeSelector(out);// TODO: need to be removed
                    this->latencySelector(out);// TODO: need to be removed
                    this->pulseDelaySelector(out);// TODO: need to be removed
                    this->calPhaseSelector(out);// TODO: need to be removed
                    this->vfatChMinSelector(out);
                    this->vfatChMaxSelector(out);
                    this->scanMinSelector(out);// TODO: need to be removed
                    this->scanMaxSelector(out);// TODO: need to be removed, taken from default 
                    this->throttleSelector(out);// TODO: need to be removed
                    this->vt2ThresholdSelector(out);//TODO: need to be removed
                    this->msplSelector(out);//TODO: need to be removed, taken from default
                    this->timeAcquisitionSelector(out);//TODO: need to be removed
                    this->timeIntervalSelector(out);// TODO: need to be removed
                *out << "</div>" << std::endl;
            *out << "</form>"<< std::endl;
            *out << "<div class=\"col-md-6\">" << std::endl;
                *out << "<br>" << std::endl;	
                this->comparatorSelector(out);
                this->slotsAndMasksSelector(out);
                *out << "<br><br><br>" << std::endl;
                *out << "<div align=\"center\">" << std::endl;
                    *out << "<button class=\"btn btn-lg btn-info\" type=\"button\" onclick=\"apply_action()\" id=\"apply\" name=\"apply_all\">APPLY SETTINGS</button>"<< std::endl;
                    *out << "<button class=\"btn btn-lg btn-success\" onclick=\"run_scan()\" id=\"run_button\" name=\"run_button\" disabled>RUN</button>"<< std::endl;
                *out << "</div>"<< std::endl;   
             *out << "</div>" << std::endl;
        *out << "</div>" << std::endl;
    *out << "</div>" << std::endl;     
    *out << "<br>" << std::endl;
}

void gem::calib::CalibrationWeb::scanDACInterface(xgi::Output* out)
  throw (xgi::exception::Exception)
{ 
    *out << "<div class=\"container col-md-12\" id=\"cal_interface\">" << std::endl;
        *out << "<div align=\"center\">" << std::endl;
            *out << "<div class=\"alert alert-warning\">"
                << "<button type=\"button\" class=\"close\" data-dismiss=\"alert\">&times;</button>"
                << "To run the routine select the cards, and the optohybrids. Really??"//TODO: clarify!!
            << "</div>" << std::endl;
        *out << "</div>" << std::endl;
        *out << "<div class=\"row\">" << std::endl;
            *out << "<form id=\"settings_select\">" << std::endl;
                *out << "<div class=\"col-md-6\">" << std::endl;
                    this->triggerSelector(out);
                    *out << "<br><br><br>" << std::endl;
                    this->nSamplesSelector(out);// TODO: need to be removed
                    this->l1aTimeSelector(out);// TODO: need to be removed
                    this->latencySelector(out);// TODO: need to be removed
                    this->pulseDelaySelector(out);// TODO: need to be removed
                    this->calPhaseSelector(out);// TODO: need to be removed
                    this->vfatChMinSelector(out);// TODO: need to be removed
                    this->vfatChMaxSelector(out);// TODO: need to be removed
                    this->scanMinSelector(out);// TODO: need to be removed
                    this->scanMaxSelector(out);// TODO: need to be removed
                    this->throttleSelector(out);// TODO: need to be removed
                    this->vt2ThresholdSelector(out);//TODO: need to be removed
                    this->msplSelector(out);//TODO: need to be removed
                    this->timeAcquisitionSelector(out);//TODO: need to be removed
                    this->timeIntervalSelector(out);// TODO: need to be removed
                *out << "</div>" << std::endl;
            *out << "</form>"<< std::endl;
            *out << "<div class=\"col-md-6\">" << std::endl;
                *out << "<br>" << std::endl;	
                this->slotsAndMasksSelector(out);
                *out << "<br><br><br>" << std::endl;
                *out << "<div align=\"center\">" << std::endl;
                    *out << "<button class=\"btn btn-lg btn-info\" type=\"button\" onclick=\"apply_action()\" id=\"apply\" name=\"apply_all\">APPLY SETTINGS</button>"<< std::endl;
                    *out << "<button class=\"btn btn-lg btn-success\" onclick=\"run_scan()\" id=\"run_button\" name=\"run_button\" disabled>RUN</button>"<< std::endl;
                *out << "</div>"<< std::endl;   
            *out << "</div>" << std::endl;
        *out << "</div>" << std::endl;
    *out << "</div>" << std::endl;         
    *out << "<br>" << std::endl;
}

void gem::calib::CalibrationWeb::temperatureInterface(xgi::Output* out)
  throw (xgi::exception::Exception)
{ 
    *out << "<div class=\"container col-md-12\" id=\"cal_interface\">" << std::endl;
        *out << "<div align=\"center\">" << std::endl;
            *out << "<div class=\"alert alert-warning\">"
                << "<button type=\"button\" class=\"close\" data-dismiss=\"alert\">&times;</button>"
                << "To run the routine select the cards, the optohybrids, the VFATs, and the update frequency."
            << "</div>" << std::endl;
        *out << "</div>" << std::endl;
        *out << "<div class=\"row\">" << std::endl;
            *out << "<form id=\"settings_select\">" << std::endl;
                *out << "<div class=\"col-md-6\">" << std::endl;
                    this->timeIntervalSelector(out);// TODO: need to be removed
                *out << "</div>" << std::endl;
            *out << "</form>"<< std::endl;
            *out << "<div class=\"col-md-6\">" << std::endl;
                *out << "<br>" << std::endl;	
                this->slotsAndMasksSelector(out);
                *out << "<br><br><br>" << std::endl;
                *out << "<div align=\"center\">" << std::endl;
                    *out << "<button class=\"btn btn-lg btn-info\" type=\"button\" onclick=\"apply_action()\" id=\"apply\" name=\"apply_all\">APPLY SETTINGS</button>"<< std::endl;
                    *out << "<button class=\"btn btn-lg btn-success\" onclick=\"run_scan()\" id=\"run_button\" name=\"run_button\" disabled>RUN</button>"<< std::endl;
                *out << "</div>"<< std::endl;   
            *out << "</div>" << std::endl;
        *out << "</div>" << std::endl;
    *out << "</div>" << std::endl;         
    *out << "<br>" << std::endl;
}

void gem::calib::CalibrationWeb::sbitReadOutInterface(xgi::Output* out)
  throw (xgi::exception::Exception)
{ 
    *out << "<div class=\"container col-md-12\" id=\"cal_interface\">" << std::endl;
        *out << "<div align=\"center\">" << std::endl;
            *out << "<div class=\"alert alert-warning\">"
                << "<button type=\"button\" class=\"close\" data-dismiss=\"alert\">&times;</button>"
                << "To run the routine select the cards, the optohybrids, and the acquisition time in second to acquire sbits for."
            << "</div>" << std::endl;
        *out << "</div>" << std::endl;
        *out << "<div class=\"row\">" << std::endl;
            *out << "<form id=\"settings_select\">" << std::endl;
                *out << "<div class=\"col-md-6\">" << std::endl;
                    this->timeAcquisitionSelector(out);
                *out << "</div>" << std::endl;
            *out << "</form>"<< std::endl;
            *out << "<div class=\"col-md-6\">" << std::endl;
                *out << "<br>" << std::endl;	
                this->slotsAndMasksSelector(out);
                *out << "<br><br><br>" << std::endl;
                *out << "<div align=\"center\">" << std::endl;
                    *out << "<button class=\"btn btn-lg btn-info\" type=\"button\" onclick=\"apply_action()\" id=\"apply\" name=\"apply_all\">APPLY SETTINGS</button>"<< std::endl;
                    *out << "<button class=\"btn btn-lg btn-success\" onclick=\"run_scan()\" id=\"run_button\" name=\"run_button\" disabled>RUN</button>"<< std::endl;
                *out << "</div>"<< std::endl;   
             *out << "</div>" << std::endl;
        *out << "</div>" << std::endl;
    *out << "</div>" << std::endl;         
    *out << "<br>" << std::endl;
}

void gem::calib::CalibrationWeb::sbitMapAndRateInterface(xgi::Output* out)
  throw (xgi::exception::Exception)
{ 
    *out << "<div class=\"container col-md-12\" id=\"cal_interface\">" << std::endl;
        *out << "<div align=\"center\">" << std::endl;
            *out << "<div class=\"alert alert-warning\">"
                << "<button type=\"button\" class=\"close\" data-dismiss=\"alert\">&times;</button>"
                << "To run the routine select the cards, the optohybrids and the acquisition time in millisecond to acquire sbits for."
            << "</div>" << std::endl;
        *out << "</div>" << std::endl;
        *out << "<div class=\"row\">" << std::endl;
            *out << "<form id=\"settings_select\">" << std::endl;
                *out << "<div class=\"col-md-6\">" << std::endl;
                    this->triggerSelector(out);
	                *out << "<br><br><br>" << std::endl;
                    this->timemsSelector(out);
                    this->rateArraySelector(out);
                *out << "</div>" << std::endl;
            *out << "</form>"<< std::endl;
            *out << "<div class=\"col-md-6\">" << std::endl;
                *out << "<br>" << std::endl;	
                this->slotsAndMasksSelector(out);
                *out << "<br><br><br>" << std::endl;
                *out << "<div align=\"center\">" << std::endl;
                    *out << "<button class=\"btn btn-lg btn-info\" type=\"button\" onclick=\"apply_action()\" id=\"apply\" name=\"apply_all\">APPLY SETTINGS</button>"<< std::endl;
                    *out << "<button class=\"btn btn-lg btn-success\" onclick=\"run_scan()\" id=\"run_button\" name=\"run_button\" disabled>RUN</button>"<< std::endl;
                *out << "</div>"<< std::endl;   
            *out << "</div>" << std::endl;
        *out << "</div>" << std::endl;
    *out << "</div>" << std::endl;         
    *out << "<br>" << std::endl;
}

void gem::calib::CalibrationWeb::expertPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CalibrationWeb::expertPage");
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  //*out << "<div align=\"center\">" << std::endl;
  //*out << "<h1><span class=\"label label-info\" id=\"mon_state\">MONITORING STATE: "
  //    << dynamic_cast<gem::calib::Calibration*>(p_gemApp)->monitoringState() << "</span></h1>" << std::endl; //CG
  //    *out << cgicc::script().set("type", "text/javascript").set("src", "/gemdaq/gemcalibration/html/scripts/checkall.js")
 

}

void gem::calib::CalibrationWeb::monitorPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CalibrationWeb::monitorPage : Do nothing for the moment, will be eventually filled later");
  //*out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  //  
  //*out << "</div>" << std::endl;
}

void gem::calib::CalibrationWeb::jsonUpdate(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CalibrationWeb::jsonUpdate");
/*
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out << " { "<< std::endl;
  for (unsigned int i = 0; i < NAMC; ++i){
    auto gemcal = dynamic_cast<gem::calib::Calibration*>(p_gemApp)->v_gemcal.at(i);
    if (gemcal) {
      *out << "\"amc"<<i+1<< "\" : "  << std::endl;
      gemcal->jsonContentUpdate(out);
      if (i!=NAMC-1) *out << ","<< std::endl; // Add comma if not the last entry
    }
  }
  *out << " } "<< std::endl;
*/
}
