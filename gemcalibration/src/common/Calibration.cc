/**
 * class: Calibration
 * description: Calibration application for GEM system
 *              structure borrowed from gemSupervisor
 * author: 
 * date:
 */

#include "gem/calib/Calibration.h"

#include <cstdlib>
#include <iomanip>

#include <map>
#include <set>
#include <vector>
#include <algorithm>

#include <boost/algorithm/string.hpp>

#include "gem/calib/CalibrationWeb.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"
#include "gem/utils/exception/Exception.h"

typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

XDAQ_INSTANTIATOR_IMPL(gem::calib::Calibration);

gem::calib::Calibration::Calibration(xdaq::ApplicationStub* stub) :
    gem::base::GEMApplication(stub),
    m_nShelves(0)
{
    CMSGEMOS_DEBUG("gem::calib::Calibration : Creating the CalibrationWeb interface");
    CMSGEMOS_DEBUG("gem::calib::Calibration : Retrieving configuration");
    p_appInfoSpace->fireItemAvailable("nShelves",     &m_nShelves);
    p_appInfoSpace->addItemRetrieveListener("nShelves", this);
    p_appInfoSpace->addItemChangedListener("nShelves", this);
    CMSGEMOS_DEBUG("gem::calib::Calibration : configuration retrieved");
    CMSGEMOS_INFO("gem::calib::Calibration:init() number of shelves: " << m_nShelves.value_);
    p_gemWebInterface = new gem::calib::CalibrationWeb(this);
    m_calType = NDEF;
  
    xgi::bind(this, &Calibration::applyAction, "applyAction");
    xgi::bind(this, &Calibration::setCalType, "setCalType");
}

gem::calib::Calibration::~Calibration()
{
    CMSGEMOS_DEBUG("gem::calib::Calibration : Destructor called");
    // make sure to empty the v_supervisedApps  vector and free the pointers
}

void gem::calib::Calibration::actionPerformed(xdata::Event& event)
{
    if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
        CMSGEMOS_DEBUG("gem::calib::Calibration::actionPerformed() setDefaultValues" <<
                       "Default configuration values have been loaded from xml profile");
        init();
    }

    // item is changed, update it
    if (event.type() == "ItemChangedEvent" || event.type() == "urn:xdata-event:ItemChangedEvent") {
        CMSGEMOS_DEBUG("gem::calib::Calibration::actionPerformed() ItemChangedEvent");
    }
    // update calibration variables
    gem::base::GEMApplication::actionPerformed(event);
}

void gem::calib::Calibration::init()
{
    CMSGEMOS_INFO("gem::calib::Calibration:init() number of shelves: " << m_nShelves.value_);
 
}

bool gem::calib::Calibration::isGEMApplication(const std::string& classname) const
{
    if (classname.find("gem::") != std::string::npos)
        return true;  // handle all GEM applications
    /*
      if (m_otherClassesToSupport.count(classname) != 0)
      return true;  // include from list
    */
    return false;
}

void gem::calib::Calibration::applyAction(xgi::Input* in, xgi::Output* out)
    throw (xgi::exception::Exception)
{
    bool t_errorsOccured = false;
    cgicc::Cgicc cgi(in);
    for (auto it: m_scanParams.find(m_calType)->second) {
        it.second = cgi[it.first]->getIntegerValue();
        CMSGEMOS_DEBUG("Calibration::applyAction : " << it.first << " = " << it.second);
    }
    
    std::stringstream t_stream;
    //for (unsigned int i = 0; i < NSHELF; ++i) {
    for ( int i = 0; i < m_nShelves.value_; ++i) {
        t_stream.clear();
        t_stream.str(std::string());
        t_stream << "shelf"<< std::setfill('0') << std::setw(2) << i+1;
        bool checked = false;
        checked = cgi.queryCheckbox(t_stream.str());
        if (checked) {
            for (unsigned int j = 0; j < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++j) { //SHELF.AMC
                t_stream.clear();
                t_stream.str(std::string());
                t_stream << "shelf"<< std::setfill('0') << std::setw(2) << i+1 << ".amc" << std::setfill('0') << std::setw(2) << j+1;
                checked = cgi.queryCheckbox(t_stream.str());
                if (checked) {
                    std::string amc_id = t_stream.str();
                    m_amcOpticalLinks.emplace(amc_id, 0);
                    t_stream << ".ohMask";
                    uint32_t ohMask = std::stoul(cgi[t_stream.str()]->getValue(), 0, 16);
                    if (ohMask > 0xffc) {
                        t_errorsOccured = true;
                        CMSGEMOS_ERROR("Calibration::applyAction : OH mask for " << t_stream.str() << " is out of allowed boundaries! Ignoring it");
                    } else{
                        m_amcOpticalLinks.find(amc_id)->second = ohMask;
                    }
                } // end if checked for amc
            } // end loop over NAMC 
        } // end if checked for shelf
    } //end loop over shelves
    //TODO: should we check if at least one link is selected??
    

    //DACSCAN type
    if (m_calType==DACSCANV3) {
        for (auto & it : m_dacScanTypeParams) {
            t_stream.clear();
            t_stream.str(std::string());
            t_stream << it.second.label;
            bool checked = false;
            checked = cgi.queryCheckbox(t_stream.str());
            if (checked) {
                for (auto dacScan_parameter: it.second.range) {
                    dacScan_parameter.second = cgi[dacScan_parameter.first]->getIntegerValue();
                }	  
            }
        }
    }
    //TODO: should we check the range of the parameters?

    out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
    if (t_errorsOccured) {
        *out << "{\"status\":1,\"alert\":\"There was an error in the parameters retrieval. Please check the XDAQ log for more information\"}";
    } else {
        *out << "{\"status\":0,\"alert\":\"Parameters successfully applied. Now you can run the scan.\"}";
    }
    

}

void gem::calib::Calibration::setCalType(xgi::Input* in, xgi::Output* out)
    throw (xgi::exception::Exception)
{
    CMSGEMOS_INFO("Calibration::setCalType");
    out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
    cgicc::Cgicc cgi(in);
    m_calType = m_calTypeSelector.find(cgi["cal_type_select"]->getValue())->second;
    switch (m_calType) {
    case NDEF: 
        CMSGEMOS_DEBUG("Calibration::setCalType : Selected Cal Type: NDEF");
        break;
    default: 
        CMSGEMOS_DEBUG("Calibration::setCalType : Selected Cal Type: SCURVE");
        dynamic_cast<gem::calib::CalibrationWeb*>(p_gemWebInterface)->settingsInterface(m_calType, out, m_nShelves); 
        break;
    }
}
