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

gem::base::utils::gemXMLparser::gemXMLparser(const std::string& xmlFile)
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
    std::cout << "[XML PARSER]: Xerces parcer created " << std::endl;
    parser->setValidationScheme(xercesc::XercesDOMParser::Val_Auto);
    parser->setDoNamespaces(false);
    parser->setCreateEntityReferenceNodes(false);
    //parser->setToCreateXMLDeclTypeNode(true);
    std::cout << "[XML PARSER]: Xerces parcer tuned up " << std::endl;


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
        std::cout << "[XML PARSER]: DOM tree created succesfully" << std::endl;
        xercesc::DOMNode * pDoc = parser->getDocument();
        std::cout << "[XML PARSER]: Base node (getDocument) obtained" << std::endl;
        xercesc::DOMNode * n = pDoc->getFirstChild();
        std::cout << "[XML PARSER]: First child node obtained" << std::endl;
        while (n) {
            std::cout << "[XML PARSER]: Loop on child nodes" << std::endl;
            if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) 
            {
                std::cout << "[XML PARSER]: Element node found" << std::endl;
                if (strcmp("GEMSystem",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                    std::cout << "[XML PARSER]: GEM system found" << std::endl;
                    parseGEMSystem(n);
                }
            }
            n = n->getNextSibling();
        } 
    } 

    std::cout << "[XML PARSER]: Parser pointer " << parser << std::endl;
    delete parser;
    std::cout << "[XML PARSER]: Xerces parcer deleted " << std::endl;
    xercesc::XMLPlatformUtils::Terminate();
}

void gem::base::utils::gemXMLparser::parseGEMSystem(xercesc::DOMNode * pNode)
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseGEMSystem");
    std::cout << "[XML PARSER]: GEM system parsing" << std::endl;
    xercesc::DOMNode * n = pNode->getFirstChild();
    std::cout << "[XML PARSER]: GEM system parsing: get first child" << std::endl;
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
                std::cout << "[XML PARSER]: GEM system parsing: uTCA crate found" << std::endl;
                if (countChildElementNodes(n)) {
                    std::cout << "[XML PARSER]: GEM system parsing: uTCA crate is not empty" << std::endl;
                    gem::base::utils::gemCrateProperties* crate = new gem::base::utils::gemCrateProperties::gemCrateProperties();
                    std::cout << "[XML PARSER]: GEM system parsing: new crate properties object created" << std::endl;
                    crate->deviceId_ = xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("CrateId"))->getNodeValue());
                    crateRefs_.push_back(crate);
                    std::cout << "[XML PARSER]: GEM system parsing: new crate properties object added to crateRefs" << std::endl;
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

    std::cout << "[XML PARSER]: GEM system parsing: starting parseCrate" << std::endl;
    xercesc::DOMNode * n = pNode->getFirstChild();
    std::cout << "[XML PARSER]: crate parsing: look for children" << std::endl;
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
                std::cout << "[XML PARSER]: crate parsing: GLIB found" << std::endl;
                if (countChildElementNodes(n)) {
                    std::cout << "[XML PARSER]: crate parsing: GLIB is not empty" << std::endl;
                    gem::base::utils::gemGLIBProperties* glib = new gem::base::utils::gemGLIBProperties::gemGLIBProperties();
                    std::cout << "[XML PARSER]: crate parsing: create new GLIBproperties object" << std::endl;
                    glib->deviceId_ = xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("GLIBId"))->getNodeValue());
                    crateRefs_.back()->subDevicesRefs_.push_back(glib);
                    crateRefs_.back()->subDevicesIDs_.push_back(glib->deviceId_);
                    std::cout << "[XML PARSER]: crate parsing: Add new GLIBproperties to the subdevices of parent crate" << std::endl;
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
    std::cout << "[XML PARSER]: crate parsing: start GLIB parsing" << std::endl;
    xercesc::DOMNode * n = pNode->getFirstChild();
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {    
            if (strcmp("OH",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                //OHIds.push_back(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("OHId"))->getNodeValue()));
                std::cout << "[XML PARSER]: GLIB parsing: OH found" << std::endl;
                if (countChildElementNodes(n)) {
                    gem::base::utils::gemOHProperties* oh = new gem::base::utils::gemOHProperties::gemOHProperties();
                    std::cout << "[XML PARSER]: GLIB parsing: create new OHproperties obect" << std::endl;
                    oh->deviceId_ = xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("OHId"))->getNodeValue());
                    crateRefs_.back()->subDevicesRefs_.back()->subDevicesRefs_.push_back(oh);
                    crateRefs_.back()->subDevicesRefs_.back()->subDevicesIDs_.push_back(oh->deviceId_);
                    std::cout << "[XML PARSER]: GLIB parsing: Add new OHproperties to the subdevices of parent device" << std::endl;
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
    std::cout << "[XML PARSER]: GLIB parsing: start OH parsing" << std::endl;
    xercesc::DOMNode * n = pNode->getFirstChild();
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {    
            if (strcmp("VFATSettings",xercesc::XMLString::transcode(n->getNodeName()))==0) 
                std::cout << "[XML PARSER]: OH parsing: VFATSettings tag found" << std::endl;
                if (countChildElementNodes(n)) {
                    gem::base::utils::gemVFATProperties* vfat = new gem::base::utils::gemVFATProperties::gemVFATProperties();
                    std::cout << "[XML PARSER]: OH parsing: create new VFATproperties object" << std::endl;
                    vfat->deviceId_ = xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("VFATId"))->getNodeValue());
                    std::cout << "[XML PARSER]: OH parsing: retrieve VFAT device ID" << std::endl;
                    crateRefs_.back()->subDevicesRefs_.back()->subDevicesRefs_.back()->subDevicesRefs_.push_back(vfat);
                    std::cout << "[XML PARSER]: OH parsing: add new VFATproperties to the subdevices of the parent device" << std::endl;
                    crateRefs_.back()->subDevicesRefs_.back()->subDevicesRefs_.back()->subDevicesIDs_.push_back(vfat->deviceId_);
                    std::cout << "[XML PARSER]: OH parsing: add VFAT device ID to the subdevices of the parent device" << std::endl;
                    parseVFAT2Settings(n);
                }
        }    
        n = n->getNextSibling();
    }    
}

void gem::base::utils::gemXMLparser::parseVFAT2Settings(xercesc::DOMNode * pNode)
{
    std::cout << "[XML PARSER]: OH parsing: start VFAT parsing" << std::endl;
    xercesc::DOMNode * n = pNode->getFirstChild();
    gemVFATProperties* vfat = crateRefs_.back()->subDevicesRefs_.back()->subDevicesRefs_.back()->subDevicesRefs_.back();
    std::cout << "[XML PARSER]: VFAT parsing: retrieve VFAT device from the devices parent tree" << std::endl;
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

int gem::base::utils::gemXMLparser::countChildElementNodes(xercesc::DOMNode * pNode) {
    int count = 0;
    if (pNode->hasChildNodes()) {
        xercesc::DOMNode * n = pNode->getFirstChild();
        while (n) {
            if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) count++;
            n = n->getNextSibling();
        }
    }
    return count;
}
