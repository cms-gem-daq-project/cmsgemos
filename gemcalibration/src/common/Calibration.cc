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

//#include "gem/utils/soap/GEMSOAPToolBox.h"
//#include "gem/utils/exception/Exception.h"

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

void gem::calib::Calibration::applyAction(xgi::Input* in, xgi::Output* out)
{
    bool t_errorsOccured = false;
    cgicc::Cgicc cgi(in);
      
    std::map<std::string, std::string >::iterator it = (m_scanParams.find(m_calType)->second).begin();
    while (it!= (m_scanParams.find(m_calType)->second).end() ){
        
        it->second = cgi[it->first]->getValue();
        CMSGEMOS_DEBUG("Calibration::applyAction for calibration "<< m_calType <<" : " << it->first << " = " << it->second);
        it++;
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
                    
                    CMSGEMOS_DEBUG("Calibration::applyAction : OH mask for " << t_stream.str() << " ohMask is " << ohMask );
                    if (ohMask > 0xffc) {
                        t_errorsOccured = true;
                        CMSGEMOS_ERROR("Calibration::applyAction : OH mask for " << t_stream.str() << " is out of allowed boundaries! Ignoring it");
                    } else{
                        m_amcOpticalLinks.find(amc_id)->second = ohMask;
                    }
                } else{// end if checked for amc
                    std::string amc_id = t_stream.str();
                    m_amcOpticalLinks.emplace(amc_id, 0);
                    m_amcOpticalLinks.find(amc_id)->second = 0;
                }
            } // end loop over NAMC 
        } // end if checked for shelf
        else{ /// filling also the empty slot.amc with OHMask=0 
           for (unsigned int j = 0; j < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++j) { //SHELF.AMC
                t_stream.clear();
                t_stream.str(std::string());
                t_stream << "shelf"<< std::setfill('0') << std::setw(2) << i+1 << ".amc" << std::setfill('0') << std::setw(2) << j+1;
                std::string amc_id = t_stream.str();
                m_amcOpticalLinks.emplace(amc_id, 0);
                m_amcOpticalLinks.find(amc_id)->second = 0;
                CMSGEMOS_DEBUG("Calibration::applyAction: ****Empty OH mask for " << t_stream.str() << " ohMask is 0");
           } // end if checked for amc
           
        }
            
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
                it.second.min = cgi[it.second.label+"_Min"]->getIntegerValue();
                it.second.max = cgi[it.second.label+"_Max"]->getIntegerValue();
                it.second.scan = true;
                //CMSGEMOS_DEBUG("Calibration::applyAction DAC SCAN V3 for calibration: checked "<< it.second.label << " with par  min " << it.second.min << " max " << it.second.max);
            } else {
                it.second.scan = false;
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
    ///Just for debugging TODO:Remove
    for (auto it: m_scanParams.find(m_calType)->second) {
        CMSGEMOS_DEBUG("Calibration::end of applyAction for calibration "<< m_calType <<" : " << it.first << " = " << it.second);
      
    }
      
    if (m_calType==DACSCANV3){
        sendSOAPMessageForDacScan();
    } else {
        sendSOAPMessageForCalibration();
    }
    
}

void gem::calib::Calibration::setCalType(xgi::Input* in, xgi::Output* out)
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


void gem::calib::Calibration::initializeCalibConfigMap(std::map<std::string, xdata::Integer>* calibConfigMap, std::map<std::string,xdata::String>* calibTypeConfigMap) {
    for (auto const  &element : m_scanParams ){
        for (auto it : element.second) {
            
            if(it.first.find("armDac")!=std::string::npos || it.first.find("trimValues")!=std::string::npos) {
                std::map<std::string,xdata::String>::iterator iterStr;
                iterStr = calibTypeConfigMap->find(it.first);
                if (iterStr==calibTypeConfigMap->end()){
                    calibTypeConfigMap->insert( std::pair <std::string,xdata::String> (it.first, "") );
                  
                }else {
                    calibTypeConfigMap->find(it.first)->second= "";
                
                }
            }else{

                std::map<std::string,xdata::Integer>::iterator iter;
                iter = calibConfigMap->find(it.first);
                if (iter==calibConfigMap->end()){
                    calibConfigMap->insert( std::pair <std::string,xdata::Integer> (it.first, 0) );
                    
                }else {
                    calibConfigMap->find(it.first)->second= 0;
                }
            }
        }
    }
     
}


void gem::calib::Calibration::fillCalibConfigMap(calType_t cal, std::map<std::string, xdata::Integer>* calibConfigMap) {
   
    for ( auto it: m_scanParams.find(cal)->second ) {
        if (it.first.find("armDac")!=std::string::npos || it.first.find("trimValues")!=std::string::npos) continue;
        (calibConfigMap->find(it.first))->second =  std::stoi(it.second.c_str());
        
    }
}

void gem::calib::Calibration::fillCalibTypeConfigMap(calType_t cal, std::map<std::string, xdata::String>* calibTypeConfigMap) {
    
    for ( auto it: m_scanParams.find(cal)->second ) {
        if (it.first.find("armDac")!=std::string::npos || it.first.find("trimValues")!=std::string::npos) {
         calibTypeConfigMap->find(it.first)->second = it.second;
        }
    }
}

void gem::calib::Calibration::printCalibConfigMap(  std::map<std::string, xdata::Integer>* calibConfigMap ) {
     std::map<std::string,xdata::Integer>::iterator iter;
     CMSGEMOS_DEBUG("Calibration::printCalibConfigMap  Printing CalibConfigMap");
     for ( iter = calibConfigMap->begin(); iter != calibConfigMap->end() ;++iter ) {
         CMSGEMOS_DEBUG(" iter->first "<<iter->first << " second " << iter->second.toString());
    }
}
       
 void gem::calib::Calibration::fillBagFromConfigMap( std::unordered_map<std::string, xdata::Serializable*>* bag, std::map<std::string, xdata::Integer>* calibConfigMap, std::map<std::string, xdata::String>* calibTypeConfigMap) {
    
    std::map<std::string,xdata::Integer>::iterator iter;
    for ( iter = calibConfigMap->begin(); iter != calibConfigMap->end() ;++iter ) {
        bag->insert(std::make_pair(iter->first,    &(iter->second)));
    }
    std::map<std::string,xdata::String>::iterator iterStr;
    for ( iterStr = calibTypeConfigMap->begin(); iterStr != calibTypeConfigMap->end() ;++iterStr ) {
        bag->insert(std::make_pair(iterStr->first,    &(iterStr->second)));  
    }
}

void gem::calib::Calibration::initializeAndFillDacScanConfigMap(std::map<std::string, dacFeatureBag>* dacScanConfigMap) {
    for (auto it :  m_dacScanTypeParams ) {  
            
        
        if (dacScanConfigMap->find(it.second.label)==dacScanConfigMap->end()) {
            dacFeatureBag tmp;
            tmp.label= it.second.label;
            tmp.min=it.second.min;
            tmp.max=it.second.max;
            tmp.scan=it.second.scan;
            dacScanConfigMap->insert( std::pair <std::string,dacFeatureBag> (it.second.label, tmp ) );
            
        }else{
            dacScanConfigMap->find(it.second.label)->second.min = it.second.min;
            dacScanConfigMap->find(it.second.label)->second.max = it.second.max;
            dacScanConfigMap->find(it.second.label)->second.scan = it.second.scan;
        }           
        
    }
}

void gem::calib::Calibration::fillDacScanBagFromConfigMap( std::unordered_map<std::string, xdata::Serializable*>* bag, std::map<std::string, dacFeatureBag>* dacScanConfigMap ) {
    
    std::map<std::string, dacFeatureBag>::iterator iter;
    for ( iter = dacScanConfigMap->begin(); iter != dacScanConfigMap->end() ;++iter ) {
        bag->insert(std::make_pair(iter->first,    &(iter->second.label)));
        bag->insert(std::make_pair(iter->first+"_min",    &(iter->second.min)));
        bag->insert(std::make_pair(iter->first+"_max",    &(iter->second.max)));
        bag->insert(std::make_pair(iter->first+"_scan",    &(iter->second.scan)));
        
    }
}
void gem::calib::Calibration::sendSOAPMessageForCalibration() {

   
    
    //std::set<xdaq::ApplicationDescriptor*> used;
    std::set<std::string> groups = p_appZone->getGroupNames();
    for (auto i =groups.begin(); i != groups.end(); ++i) {
        CMSGEMOS_DEBUG("GEMCalibration:::init::xDAQ group: " << *i
                      << " getApplicationGroup() " << p_appZone->getApplicationGroup(*i)->getName());
        
        xdaq::ApplicationGroup* ag = const_cast<xdaq::ApplicationGroup*>(p_appZone->getApplicationGroup(*i));
        std::set<const xdaq::ApplicationDescriptor*> allApps = ag->getApplicationDescriptors();
        for (auto j = allApps.begin(); j != allApps.end(); ++j) {
            std::string classname = (*j)->getClassName();

            CMSGEMOS_DEBUG("GEMCalibration:::init::xDAQ group: " << *i
                      << " allApp class name " << (*j)->getClassName());
                         
                xdaq::ApplicationDescriptor* app=(xdaq::ApplicationDescriptor*) *j;
                std::string command = "calibParamRetrieve";
                
                initializeCalibConfigMap( &calibConfigMap, &calibTypeConfigMap);
                
                fillCalibConfigMap(m_calType,&calibConfigMap);
               
                printCalibConfigMap(&calibConfigMap);
                fillCalibTypeConfigMap(m_calType,&calibTypeConfigMap);
                std::unordered_map<std::string, xdata::Serializable*> bagFromMap;
                xdata::Integer calType= m_calType;
                bagFromMap.insert(std::make_pair("calType",    &(calType)));        
                fillBagFromConfigMap(&bagFromMap,&calibConfigMap,&calibTypeConfigMap);

                initializeAndFillOpticalLinksMap( &amcOpticalLinksMap);
                fillBagFromOpticalLinksMap(&bagFromMap,&amcOpticalLinksMap);
               
                gem::utils::soap::GEMSOAPToolBox::sendCommandWithParameterBag(command, bagFromMap, p_appContext, p_appDescriptor, app);
                printCalibConfigMap(&calibConfigMap);
                                   
        }
    }
    
}

void gem::calib::Calibration::sendSOAPMessageForDacScan() {
    
    std::set<std::string> groups = p_appZone->getGroupNames();
    for (auto i =groups.begin(); i != groups.end(); ++i) {
        CMSGEMOS_DEBUG("GEMCalibration:::init::xDAQ group: " << *i
                      << "getApplicationGroup() " << p_appZone->getApplicationGroup(*i)->getName());
        
        xdaq::ApplicationGroup* ag = const_cast<xdaq::ApplicationGroup*>(p_appZone->getApplicationGroup(*i));
        std::set<const xdaq::ApplicationDescriptor*> allApps = ag->getApplicationDescriptors();
        
        for (auto j = allApps.begin(); j != allApps.end(); ++j) {
            std::string classname = (*j)->getClassName();
                             
                xdaq::ApplicationDescriptor* app=(xdaq::ApplicationDescriptor*) *j;
                std::string command = "calibrateAction"; 
                initializeAndFillDacScanConfigMap( &dacScanConfigMap);
                std::unordered_map<std::string, xdata::Serializable*> bagDacScanFromMap;
                xdata::Integer calType= m_calType;
                bagDacScanFromMap.insert(std::make_pair("calType",    &(calType)));        
                fillDacScanBagFromConfigMap(&bagDacScanFromMap,&dacScanConfigMap);
                initializeAndFillOpticalLinksMap( &amcOpticalLinksMap);
                fillBagFromOpticalLinksMap(&bagDacScanFromMap,&amcOpticalLinksMap);
                gem::utils::soap::GEMSOAPToolBox::sendCommandWithParameterBag(command, bagDacScanFromMap, p_appContext, p_appDescriptor, app);
               
        }
    }
    
}

void gem::calib::Calibration::initializeAndFillOpticalLinksMap(std::map<std::string, xdata::Integer>* amcOpticalLinksMap) {

    for (auto it :  m_amcOpticalLinks ) {  
        if (amcOpticalLinksMap->find(it.first)== amcOpticalLinksMap->end()) {
            xdata::Integer tmp;
            tmp = it.second;
            amcOpticalLinksMap->insert( std::pair <std::string,xdata::Integer> (it.first, tmp ) );
        
        }else{
            amcOpticalLinksMap->find(it.first)->second = it.second;
        }              
    }
}

void gem::calib::Calibration::fillBagFromOpticalLinksMap( std::unordered_map<std::string, xdata::Serializable*>* bag, std::map<std::string, xdata::Integer>* amcOpticalLinksMap ) {
    
     std::map<std::string,xdata::Integer>::iterator iter;
     for ( iter = amcOpticalLinksMap->begin(); iter != amcOpticalLinksMap->end() ;++iter ) {
         bag->insert(std::make_pair(iter->first,    &(iter->second)));
         CMSGEMOS_DEBUG("GEMCalibration::fillBagFromOpticalLinksMap: mask " << iter->first << " value "<< iter->second );
    }
     
    
}
