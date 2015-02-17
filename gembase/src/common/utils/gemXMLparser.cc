///////////////////////////////////////////////
//
// Parse XML File
//
///////////////////////////////////////////////
#include "gem/utils/gemXMLparser.h"
#include "gem/utils/gemDeviceProperties.h"
#include "gem/utils/gemCrateProperties.h"
#include "gem/utils/gemGLIBProperties.h"
#include "gem/utils/gemOHProperties.h"
#include "gem/utils/gemVFATProperties.h"

gem::base::utils::gemXMLparser::gemXMLparser(std::string& xmlFile)
{
    xmlFile_ = xmlFile;
}


gem::base::utils::gemXMLparser::~gemXMLparser()
{
    for (std::vector<gemCrateProperties*>::iterator crate = crateRefs_.begin(); crate != crateRefs_.end(); ++crate) {
        for (std::vector<gem::base::utils::gemGLIBProperties*>::iterator glib = (*crate)->subDevicesRefs_.begin(); glib != (*crate)->subDevicesRefs_.end(); ++glib) {
            for (std::vector<gem::base::utils::gemOHProperties*>::iterator oh = (*glib)->subDevicesRefs_.begin(); oh != (*glib)->subDevicesRefs_.end(); ++oh) {
                for (std::vector<gem::base::utils::gemVFATProperties*>::iterator vfat = (*oh)->subDevicesRefs_.begin(); vfat != (*oh)->subDevicesRefs_.end(); ++vfat) {
                    delete *vfat;
                }
                delete *oh;
            }
            delete *glib;
        }
        delete *crate;
    }
}

void gem::base::utils::gemXMLparser::parseXMLFile()
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "Parsing XML file: " << settingsFile_.toString());

    //
    /// Initialize XML4C system
    try{
        xercesc::XMLPlatformUtils::Initialize();
        //LOG4CPLUS_INFO(this->getApplicationLogger(), "Successfully initialized XML4C system");
    }
    catch(const xercesc::XMLException& toCatch){
        std::cerr << "Error during Xerces-c Initialization.\n"
            << "  Exception message:"
            << xercesc::XMLString::transcode(toCatch.getMessage()) << std::endl;
        return ;
    }


    //  Create our parser, then attach an error handler to the parser.
    //  The parser will call back to methods of the ErrorHandler if it
    //  discovers errors during the course of parsing the XML document.
    //
    xercesc::XercesDOMParser *parser = new xercesc::XercesDOMParser;
    parser->setValidationScheme(xercesc::XercesDOMParser::Val_Auto);
    parser->setDoNamespaces(false);
    parser->setCreateEntityReferenceNodes(false);
    //parser->setToCreateXMLDeclTypeNode(true);


    //  Parse the XML file, catching any XML exceptions that might propogate
    //  out of it.
    //
    bool errorsOccured = false;
    try{
        parser->parse(xmlFile_.c_str());
    }


    catch (const xercesc::XMLException& e){
        std::cerr << "An error occured during parsing\n   Message: "
            << xercesc::XMLString::transcode(e.getMessage()) << std::endl;
        errorsOccured = true;
        //fileError = "An error occured during parsing of selected file. Please select another configuration file.";
    }


    catch (const xercesc::DOMException& e){
        std::cerr << "An error occured during parsing\n   Message: "
            << xercesc::XMLString::transcode(e.msg) << std::endl;
        errorsOccured = true;
        //fileError = "An error occured during parsing of selected file. Please select another configuration file.";
    }

    catch (...){
        std::cerr << "An error occured during parsing" << std::endl;
        errorsOccured = true;
        //fileError = "An error occured during parsing of selected file. Please select another configuration file.";
    }

    // If the parse was successful, output the document data from the DOM tree
    //crateIds.clear();
    //crateNodes.clear();

    if (!errorsOccured){
        xercesc::DOMNode * pDoc = parser->getDocument();
        xercesc::DOMNode * n = pDoc->getFirstChild();
        while (n) {
            if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) 
            {
                if (strcmp("GEMSystem",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                    parseGEMSystem(n);
                }
            }
            n = n->getNextSibling();
        } 
    } 

    delete parser;
    xercesc::XMLPlatformUtils::Terminate();
}

void gem::base::utils::gemXMLparser::parseGEMSystem(xercesc::DOMNode * pNode)
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseGEMSystem");
    xercesc::DOMNode * n = pNode->getFirstChild();
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {    
            if (strcmp("uTCACrate",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                //LOG4CPLUS_INFO(this->getApplicationLogger(),xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("CrateID"))->getNodeValue()));
                //crateIds.push_back(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("CrateID"))->getNodeValue()));
                //crateNodes.push_back(n);
                //if (crateIds.size() == 1) {
                    //currentCrate = 0;
                    //currentCrateId = crateIds[0];
                //}
                if (n->hasChildNodes()) {
                    gem::base::utils::gemCrateProperties* crate = new gem::base::utils::gemCrateProperties::gemCrateProperties();
                    crate->deviceId_ = xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("CrateId"))->getNodeValue());
                    crateRefs_.push_back(crate);
                    parseCrate(n);
                }
            }
        }    
        n = n->getNextSibling();
    }
}

void gem::base::utils::gemXMLparser::parseCrate(xercesc::DOMNode * pNode)
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseCrate");
    //bool cur = false;
    //if (xercesc::XMLString::transcode(pNode->getAttributes()->getNamedItem(xercesc::XMLString::transcode("CrateID"))->getNodeValue()) == currentCrateId) {
    //    cur = true;
    //}
    //if (cur) {
    //    MCHIds.clear();
    //    AMCIds.clear();
    //    GLIBIds.clear();
    //}

    xercesc::DOMNode * n = pNode->getFirstChild();
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {
            if (strcmp("MCH",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseMCH");
                //if (cur) MCHIds.push_back(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("MCHId"))->getNodeValue()));
            } 
            if (strcmp("AMC",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseAMC");
                //if (cur) AMCIds.push_back(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("AMCId"))->getNodeValue()));
            } 
            if (strcmp("GLIB",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseGLIB");
                //GLIBIds.push_back(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("GLIBId"))->getNodeValue()));
                //if (cur) parseGLIB(n);
                if (n->hasChildNodes()) {
                    gem::base::utils::gemGLIBProperties* glib = new gem::base::utils::gemGLIBProperties::gemGLIBProperties();
                    glib->deviceId_ = xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("GLIBId"))->getNodeValue());
                    crateRefs_.back()->subDevicesRefs_.push_back(glib);
                    crateRefs_.back()->subDevicesIDs_.push_back(glib->deviceId_);
                    parseGLIB(n);
                }
            } 
        }
        n = n->getNextSibling();
    }
}


void gem::base::utils::gemXMLparser::parseGLIB(xercesc::DOMNode * pNode)
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseGLIB");
    //OHIds.clear();
    xercesc::DOMNode * n = pNode->getFirstChild();
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {    
            if (strcmp("OH",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                //OHIds.push_back(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("OHId"))->getNodeValue()));
                if (n->hasChildNodes()) {
                    gem::base::utils::gemOHProperties* oh = new gem::base::utils::gemOHProperties::gemOHProperties();
                    oh->deviceId_ = xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("OHId"))->getNodeValue());
                    crateRefs_.back()->subDevicesRefs_.back()->subDevicesRefs_.push_back(oh);
                    crateRefs_.back()->subDevicesRefs_.back()->subDevicesIDs_.push_back(oh->deviceId_);
                    parseOH(n);
                }
            }
        }    
        n = n->getNextSibling();
    }    
}

void gem::base::utils::gemXMLparser::parseOH(xercesc::DOMNode * pNode)
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseOH");
    //VFAT2Ids.clear();
    xercesc::DOMNode * n = pNode->getFirstChild();
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {    
            if (strcmp("VFATSettings",xercesc::XMLString::transcode(n->getNodeName()))==0) 
                if (n->hasChildNodes()) {
                    gem::base::utils::gemVFATProperties* vfat = new gem::base::utils::gemVFATProperties::gemVFATProperties();
                    vfat->deviceId_ = xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("VFATId"))->getNodeValue());
                    crateRefs_.back()->subDevicesRefs_.back()->subDevicesRefs_.back()->subDevicesRefs_.push_back(vfat);
                    crateRefs_.back()->subDevicesRefs_.back()->subDevicesRefs_.back()->subDevicesIDs_.push_back(vfat->deviceId_);
                    parseVFAT2Settings(n);
                }
        }    
        n = n->getNextSibling();
    }    
}

void gem::base::utils::gemXMLparser::parseVFAT2Settings(xercesc::DOMNode * pNode)
{
    xercesc::DOMNode * n = pNode->getFirstChild();
    gemVFATProperties* vfat = crateRefs_.back()->subDevicesRefs_.back()->subDevicesRefs_.back()->subDevicesRefs_.back();
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {    
            if (strcmp("CalMode",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("CalMode",value));
            }
            if (strcmp("CalPolarity",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("CalPolarity",value));
            }
            if (strcmp("MSPolarity",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("MSPolarity",value));
            }
            if (strcmp("TriggerMode",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("TriggerMode",value));
            }
            if (strcmp("RunMode",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("RunMode",value));
            }
            if (strcmp("ReHitCT",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("ReHitCT",value));
            }
            if (strcmp("LVDSPowerSave",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("LVDSPowerSave",value));
            }
            if (strcmp("ProbeMode",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("ProbeMode",value));
            }
            if (strcmp("DACSel",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("DACSel",value));
            }
            if (strcmp("DigInSel",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("DigInSel",value));
            }
            if (strcmp("MSPulseLength",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("MSPulseLength",value));
            }
            if (strcmp("HitCountSel",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("HitCountSel",value));
            }
            if (strcmp("DFTest",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("DFTest",value));
            }
            if (strcmp("PbBG",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("PbBG",value));
            }
            if (strcmp("TrimDACRange",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("TrimDACRange",value));
            }
            if (strcmp("IPreampIn",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("IPreampIn",value));
            }
            if (strcmp("IPreampFeed",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("IPreampFeed",value));
            }
            if (strcmp("IPreampOut",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("IPreampOut",value));
            }
            if (strcmp("IShaper",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("IShaper",value));
            }
            if (strcmp("IShaperFeed",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("IShaperFeed",value));
            }
            if (strcmp("IComp",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("IComp",value));
            }
            if (strcmp("Latency",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("Latency",value));
            }
            if (strcmp("VCal",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("VCal",value));
            }
            if (strcmp("VThreshold1",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("VThreshold1",value));
            }
            if (strcmp("VThreshold2",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("VThreshold2",value));
            }
            if (strcmp("CalPhase",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
                vfat->deviceProperties_.insert(std::pair<std::string, std::string>("CalPhase",value));
            }
        }    
        n = n->getNextSibling();
    }
}
