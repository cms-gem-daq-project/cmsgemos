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
  
  *out << "<link  rel=\"stylesheet\" href=\"/gemdaq/gembase/html/css/bootstrap.css\" type=\"text/css\">" << std::endl;
  *out << "<link  rel=\"stylesheet\" href=\"/gemdaq/gembase/html/css/bootstrap.min.css\" type=\"text/css\">" << std::endl;
  *out << "<script src=\"/gemdaq/gembase/html/scripts/bootstrap.js\"></script>" << std::endl;
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

            *out<< "<form id=\"cal_select\">"<<std::endl; *out << "<div class=\"form-group row\">" << std::endl;
                    *out << "<h2><span class=\"label label-info col-md-3\" id=\"cal_scurve\">Select calibration type:" << "</span></h2>" << std::endl; 
                   *out << "<div class=\"col-md-3\">" << std::endl;

                    *out << "<select class=\"form-control form-control-lg\" id=\"cal_type_select\" name=\"cal_type_select\" onChange=\"selectCalType()\">" << std::endl;
                        *out << "<option disabled selected value> -- select an option -- </option>" << std::endl;
                        *out << "<option>GBT Phase Scan</option>" << std::endl;
                        *out << "<option>Latency Scan</option>" << std::endl;
                        *out << "<option>S-curve Scan</option>" << std::endl;
                        *out << "<option>S-bit ARM DAC Scan</option>" << std::endl;
			*out << "<option>ARM DAC Scan</option>" << std::endl;
			*out << "<option>Derive DAC Trim Registers</option>" << std::endl;
			*out << "<option>DAC Scan on VFAT3</option>" << std::endl;
			*out << "<option>Calibrate CFG_THR_ARM_DAC</option>" << std::endl;
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


void gem::calib::CalibrationWeb::genericRadioSelector( std::string paramName, gem::calib::Calibration::scanParamsRadioSelector radio_param  ,xgi::Output* out)
  throw (xgi::exception::Exception)
{
  *out << "<h2><span class=\"label label-warning col-md-6\">"<< radio_param.label << "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<div class=\"form-check\">" << std::endl;
    
      for (unsigned int i=0; i< radio_param.radio_options.size() ;i++)
	
	if (i==0)
	  *out<<"<div class=\"form-check-input radio-inline\"> <label><input type=\"radio\" name=\""<<paramName<<"\" id=\""<<paramName<<"_radio_"<<i<<"\" value="<<i<<" checked>"<<radio_param.radio_options[i]<<"</label></div>"<< std::endl;
	else *out<< "<div class=\"form-check-input radio-inline\"> <label><input type=\"radio\" name=\""<<paramName<<"\" id=\""<<paramName<<"_radio_"<<i<<"\" value="<<i<<">"<<radio_param.radio_options[i]<<"</label></div>"<< std::endl;
      
        *out<< "</div>"
    << "</div>" << std::endl;
}

void gem::calib::CalibrationWeb::genericParamSelector(std::string labelName, std::string paramName, int defaultValue, xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<h2><span class=\"label label-success col-md-6\">" << labelName << "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"" << defaultValue << "\" class=\"form-control\" name=\"" << paramName << "\">"
    << "</div>" << std::endl;    
}

void gem::calib::CalibrationWeb::genericParamSelector_dacScan( std::string paramName, int defaultValue, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  std::string  labelName_tmp;
  if (paramName.find("Max")!=std::string::npos) labelName_tmp="Max";
  else if (paramName.find("Min")!=std::string::npos) labelName_tmp="Min";
  else labelName_tmp=paramName;
  
    *out << "<h4><span class=\"label label-success col-md-6\">" << labelName_tmp << "</span></h2>"
    << "<div class=\"col-md-6\">"
        << "<input type=\"text\" value=\"" << defaultValue << "\" class=\"form-control\" name=\"" << paramName << "\">"
    << "</div>" << std::endl;    
}

void gem::calib::CalibrationWeb::slotsAndMasksSelector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    std::stringstream t_stream;
    t_stream.clear();
    t_stream.str(std::string());

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
                    t_stream.clear();
                    t_stream.str(std::string());
                    t_stream << "shelf"<< std::setfill('0') << std::setw(2) << i+1;
                    *out << "<div class=\"row\">"
                        << "<div class=\"col-md-3\">"
                            << "<div class=\"checkbox\">" 
                                << "<label> <input type=\"checkbox\" class=\"check\" name=\""
                                <<  t_stream.str() << "\" id=\"" << t_stream.str() 
                                << "\">" << t_stream.str() << "</label>"
                            << " </div>"
                        << "</div>"
                        << "<div class=\"col-md-9\">"
                            << "<div class=\"dropdown\">"
                                << "<button id=\"amc_dropdown_button\" class=\"btn btn-lg btn-outline dropdown-toggle\" data-toggle=\"dropdown\">Select AMC and OH</button>"
                                << "<div class=\"dropdown-menu pre-scrollable\">" << std::endl;
                                    for (unsigned int j = 0; j < NAMC; ++j) { //SHELF.AMC
                                        t_stream.clear();
                                        t_stream.str(std::string());
                                        t_stream << "shelf"<< std::setfill('0') << std::setw(2) << i+1 << ".amc" << std::setfill('0') << std::setw(2) << j+1;
                                        *out << "<span class =\"dropdown-item-text\">"
                                            << "<div class=\"row\">"
                                                << "<div class=\"col-md-3\">"
                                                    << "<div class=\"checkbox\"><label><input type=\"checkbox\" class=\"check\" name=\""
                                                    << t_stream.str() << "\" id=\"" << t_stream.str() << "\">  AMC"
                                                    << std::setfill('0') << std::setw(2) << j+1 << " </label> </div>"
                                                << "</div>"
                                                << "<div class=\"col-md-1\">"
                                                << "</div>"
                                                << "<div class=\"col-md-8\">" << std::endl;
                                                    t_stream << ".ohMask";
                                                    *out << "<input type=\"text\" value=\"0x000\" size=\"4\" name=\""
                                                    << t_stream.str() << "\" id =\"" << t_stream.str() << "\">OH mask </input>"
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

void gem::calib::CalibrationWeb::dacScanV3Selector(xgi::Output* out)
  throw (xgi::exception::Exception)
{
    std::stringstream t_stream;
    t_stream.clear();
    t_stream.str(std::string());

    *out << "<div class=\"panel panel-default\">"
        << "<div class=\"panel-heading\">"
            << "<div class=\"row\">"
                << "<div class=\"col-md-12\">"
                    << "<h4>DAC scan type</h4>"
                << "</div>"
            << "</div>" 
        << "</div>" << std::endl; // end panel heaing
        // panel body
        *out << "<form id=\"dacScanV3_select\">" << std::endl;
            *out << "<div class=\"container\" id=\"dacScanV3_selection\">" << std::endl;
	    *out << "<div class=\"dropdown\">"
                                << "<button id=\"dacScan_dropdown_button\" class=\"btn btn-lg btn-outline dropdown-toggle\" data-toggle=\"dropdown\">Settings</button>"
                                << "<div class=\"dropdown-menu pre-scrollable\">" << std::endl;
		   
    	    std::map<dacScanType_t, std::map<std::string, uint32_t> >::iterator it;
    	    for(it=dynamic_cast<gem::calib::Calibration*>(p_gemApp)->m_dacScanTypeParams.begin();it!=dynamic_cast<gem::calib::Calibration*>(p_gemApp)->m_dacScanTypeParams.end();it++){
    	            t_stream.clear();
                    t_stream.str(std::string());
                    t_stream << dynamic_cast<gem::calib::Calibration*>(p_gemApp)->m_dacScanTypeParams_label.find(it->first)->second ;
                    
                          *out << "<div class=\"row\">" 
			       << "<div class=\"col-md-4\">"   << std::endl;
		                    *out<< "<div class=\"checkbox\">" 
					<< "<label> <input type=\"checkbox\" class=\"check\" name=\""
					<<  t_stream.str() << "\" id=\"" << t_stream.str() 
					<< "\">" << t_stream.str() << "</label>"
					<< " </div>" << std::endl;
			         *out<< "</div>"<< std::endl;//end column
                      
                        
                          
			 
					 std::map<std::string, uint32_t> t_dacScan_parameters = it->second;
		                         for (auto dacScan_parameter: t_dacScan_parameters) {
					   *out<< "<div class=\"col-md-4\">" <<std::endl;   
    				           this->genericParamSelector_dacScan(  dacScan_parameter.first, dacScan_parameter.second, out);
					   *out << "</div>"<< std::endl;//end column	 
					 }
			 
                   *out << "</div>" << std::endl; // end <div class="row">
					 
                } // end loop over DAC scan type
            *out << "</div>" << std::endl; // end drop-down scrollable
	    *out << "</div>" << std::endl; // end drop-down
	    *out << "</div>" << std::endl; // end container
        *out << "</form>"<< std::endl;

    *out << "<div align=\"center\">"<< std::endl;
        *out << "<button class=\"btn btn-lg btn-info\" onclick=\"select_dacscans()\">SELECT ALL</button>" << std::endl;
        *out << "<button class=\"btn btn-lg btn-warning\" onclick=\"deselect_dacscans()\">DESELECT ALL</button>" << std::endl;
    *out << "</div>"<< std::endl;
*out << "</div>" << std::endl; // end panel

 
    
}



void gem::calib::CalibrationWeb::settingsInterface(calType_t m_calType, xgi::Output* out)
  throw (xgi::exception::Exception)
{
    *out << "<div id=\"cal_interface\">" << std::endl;
        *out << "<div align=\"center\">" << std::endl;
            *out << "<div class=\"alert alert-warning\">"
                << "<button type=\"button\" class=\"close\" data-dismiss=\"alert\">&times;</button>"
                << alertMap.find(m_calType)->second
            << "</div>" << std::endl;
        *out << "</div>" << std::endl;
        *out << "<div class=\"row\">" << std::endl;
	*out << "<div class=\"col-md-6\">" << std::endl;
            *out << "<form id=\"settings_select\">" << std::endl;
              
                    // create a temporary parameters map,
                    // check whether requested calibration type needs a trigger selector
                    // if it is, create it and pop out the trigger setting
                    std::map<std::string, uint32_t> t_parameters = dynamic_cast<gem::calib::Calibration*>(p_gemApp)->m_scanParams.find(m_calType)->second;
                   

                    for (auto parameter: t_parameters) {
		      if (dynamic_cast<gem::calib::Calibration*>(p_gemApp)->m_scanParamsNonForm.find(parameter.first) !=dynamic_cast<gem::calib::Calibration*>(p_gemApp)->m_scanParamsNonForm.end() ){
			this->genericRadioSelector( parameter.first,  dynamic_cast<gem::calib::Calibration*>(p_gemApp)->m_scanParamsRadioSelector.find(parameter.first)->second, out);
			*out << "<br>" << std::endl;
			t_parameters.erase(parameter.first);
		      } else continue; 
	      
		    }
		    for (auto parameter: t_parameters) {
		      
		      
		      this->genericParamSelector( dynamic_cast<gem::calib::Calibration*>(p_gemApp)->m_scanParamsLabels.find(parameter.first)->second, parameter.first, parameter.second, out);
		      *out << "<br>" << std::endl;
		    }
		    
                    
               
            *out << "</form>"<< std::endl;
	    //*out << "</div>" << std::endl; //column ends
            *out << "<br>" << std::endl;

	    //*out << "</div>" << std::endl;
	    //*out << "<div class=\"row\">" << std::endl;    
	    //*out << "<div class=\"col-md-6\">" << std::endl;
		if (m_calType == DACSCANV3){
		  *out << "<br>" << std::endl;
		  this->dacScanV3Selector(out);
		  *out << "<br>" << std::endl;
		
		}
		//*out << "</div>" << std::endl;
	    *out << "</div>" << std::endl; //end row    
	 
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
