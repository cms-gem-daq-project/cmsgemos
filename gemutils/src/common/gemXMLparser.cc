///////////////////////////////////////////////
//
// Parse XML File
//
///////////////////////////////////////////////
#include "gem/utils/gemXMLparser.h"
#include "gem/utils/gemComplexDeviceProperties.h"
#include "gem/utils/GEMLogging.h"

gem::utils::gemXMLparser::gemXMLparser(const std::string& xmlFile)
{
    xmlFile_ = xmlFile;
    gemSystem_ = new gemSystemProperties();
    gemSystem_->setDeviceId("GEM");
    gemLogger_ = log4cplus::Logger::getInstance("gem:utils:GEMXMLParser");
}


gem::utils::gemXMLparser::~gemXMLparser()
{
    delete gemSystem_;
}

void gem::utils::gemXMLparser::parseXMLFile()
{
    INFO("Parsing XML file: " << xmlFile_);

    //
    /// Initialize XML4C system
    try{
        xercesc::XMLPlatformUtils::Initialize();
        INFO("Successfully initialized XML4C system");
    }
    catch(const xercesc::XMLException& toCatch){
        ERROR("Error during Xerces-c Initialization.\n"
	      << "  Exception message:"
	      << xercesc::XMLString::transcode(toCatch.getMessage()));
        return ;
    }


    //  Create our parser, then attach an error handler to the parser.
    //  The parser will call back to methods of the ErrorHandler if it
    //  discovers errors during the course of parsing the XML document.
    //
    xercesc::XercesDOMParser *parser = new xercesc::XercesDOMParser;
    DEBUG("[XML PARSER]: Xerces parcer created ");
    parser->setValidationScheme(xercesc::XercesDOMParser::Val_Auto);
    parser->setDoNamespaces(false);
    parser->setCreateEntityReferenceNodes(false);
    //parser->setToCreateXMLDeclTypeNode(true);
    DEBUG("[XML PARSER]: Xerces parcer tuned up ");


    //  Parse the XML file, catching any XML exceptions that might propogate
    //  out of it.
    //
    bool errorsOccured = false;
    try{
        parser->parse(xmlFile_.c_str());
    }


    catch (const xercesc::XMLException& e){
        ERROR("An error occured during parsing\n   Message: "
	      << xercesc::XMLString::transcode(e.getMessage()));
        errorsOccured = true;
        //fileError = "An error occured during parsing of selected file. Please select another configuration file.";
    }


    catch (const xercesc::DOMException& e){
        ERROR("An error occured during parsing\n   Message: "
	      << xercesc::XMLString::transcode(e.msg));
        errorsOccured = true;
        //fileError = "An error occured during parsing of selected file. Please select another configuration file.";
    }

    catch (...){
        ERROR("An error occured during parsing");
        errorsOccured = true;
        //fileError = "An error occured during parsing of selected file. Please select another configuration file.";
    }

    // If the parse was successful, output the document data from the DOM tree
    //crateIds.clear();
    //crateNodes.clear();

    if (!errorsOccured){
        DEBUG("[XML PARSER]: DOM tree created succesfully");
        xercesc::DOMNode * pDoc = parser->getDocument();
        DEBUG("[XML PARSER]: Base node (getDocument) obtained");
        xercesc::DOMNode * n = pDoc->getFirstChild();
        DEBUG("[XML PARSER]: First child node obtained");
        while (n) {
            DEBUG("[XML PARSER]: Loop on child nodes");
            if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) 
            {
                DEBUG("[XML PARSER]: Element node found");
                if (strcmp("GEMSystem",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                    DEBUG("[XML PARSER]: GEM system found");
                    parseGEMSystem(n);
                }
            }
            n = n->getNextSibling();
        } 
    } 

    DEBUG("[XML PARSER]: Parser pointer " << parser);
    delete parser;
    DEBUG("[XML PARSER]: Xerces parcer deleted ");
    xercesc::XMLPlatformUtils::Terminate();
}

void gem::utils::gemXMLparser::parseGEMSystem(xercesc::DOMNode * pNode)
{
    INFO("parseGEMSystem");
    DEBUG("[XML PARSER]: GEM system parsing");
    xercesc::DOMNode * n = pNode->getFirstChild();
    DEBUG("[XML PARSER]: GEM system parsing: get first child");
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {    
            if (strcmp("uTCACrate",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                INFO(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("CrateID"))->getNodeValue()));
                //crateIds.push_back(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("CrateID"))->getNodeValue()));
                //crateNodes.push_back(n);
                //if (crateIds.size() == 1) {
                //currentCrate = 0;
                //currentCrateId = crateIds[0];
                //}
                DEBUG("[XML PARSER]: GEM system parsing: uTCA crate found");
                if (countChildElementNodes(n)) {
                    DEBUG("[XML PARSER]: GEM system parsing: uTCA crate is not empty");
                    gemCrateProperties* crate = new gemCrateProperties();
                    DEBUG("[XML PARSER]: GEM system parsing: new crate properties object created");
                    crate->setDeviceId(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("CrateId"))->getNodeValue()));
                    gemSystem_->addSubDeviceRef(crate);
                    gemSystem_->addSubDeviceId(crate->getDeviceId());
                    DEBUG("[XML PARSER]: GEM system parsing: new crate properties object added to crateRefs");
                    parseCrate(n);
                }
            }
        }    
        n = n->getNextSibling();
    }
}

void gem::utils::gemXMLparser::parseCrate(xercesc::DOMNode * pNode)
{
    INFO("parseCrate");
    DEBUG("[XML PARSER]: GEM system parsing: starting parseCrate");
    xercesc::DOMNode * n = pNode->getFirstChild();
    DEBUG("[XML PARSER]: crate parsing: look for children");
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {
            if (strcmp("MCH",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                INFO("parseMCH");
            } 
            if (strcmp("AMC",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                INFO("parseAMC");
            } 
            if (strcmp("GLIB",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                INFO("parseGLIB");
                DEBUG("[XML PARSER]: crate parsing: GLIB found");
                if (countChildElementNodes(n)) {
                    DEBUG("[XML PARSER]: crate parsing: GLIB is not empty");
                    gemGLIBProperties* glib = new gemGLIBProperties();
                    DEBUG("[XML PARSER]: crate parsing: create new GLIBproperties object");
                    glib->setDeviceId(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("GLIBId"))->getNodeValue()));
                    gemSystem_->getSubDevicesRefs().back()->addSubDeviceRef(glib);
                    gemSystem_->getSubDevicesRefs().back()->addSubDeviceId(glib->getDeviceId());
                    DEBUG("[XML PARSER]: crate parsing: Add new GLIBproperties to the subdevices of parent crate");
                    parseGLIB(n);
                }
            } 
        }
        n = n->getNextSibling();
    }
}


void gem::utils::gemXMLparser::parseGLIB(xercesc::DOMNode * pNode)
{
    DEBUG("[XML PARSER]: crate parsing: start GLIB parsing");
    xercesc::DOMNode * n = pNode->getFirstChild();
    gemGLIBProperties* glib_ = gemSystem_->getSubDevicesRefs().back()->getSubDevicesRefs().back();
    DEBUG("[XML PARSER]: GLIB parsing: retrieve GLIB device from the devices parent tree");
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {
            addProperty("Station", n, glib_);
            addProperty("FW", n, glib_);
            addProperty("IP", n, glib_);
            addProperty("DEPTH", n, glib_);
            addProperty("TDC_SBits", n, glib_);
            if (strcmp("OH",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                DEBUG("[XML PARSER]: GLIB parsing: OH found");
                if (countChildElementNodes(n)) {
                    gemOHProperties* oh = new gemOHProperties();
                    DEBUG("[XML PARSER]: GLIB parsing: create new OHproperties obect");
                    oh->setDeviceId(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("OHId"))->getNodeValue()));
                    gemSystem_->getSubDevicesRefs().back()->getSubDevicesRefs().back()->addSubDeviceRef(oh);
                    gemSystem_->getSubDevicesRefs().back()->getSubDevicesRefs().back()->addSubDeviceId(oh->getDeviceId());
                    DEBUG("[XML PARSER]: GLIB parsing: Add new OHproperties to the subdevices of parent device");
                    parseOH(n);
                }
            }
        }    
        n = n->getNextSibling();
    }    
}

void gem::utils::gemXMLparser::parseOH(xercesc::DOMNode * pNode)
{
    DEBUG("[XML PARSER]: GLIB parsing: start OH parsing");
    xercesc::DOMNode * n = pNode->getFirstChild();
    gemOHProperties* oh_ = gemSystem_->getSubDevicesRefs().back()->getSubDevicesRefs().back()->getSubDevicesRefs().back();
    DEBUG("[XML PARSER]: OH parsing: retrieve OH device from the devices parent tree");
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {   
            addProperty("TrigSource", n, oh_);
            addProperty("TDC_SBits", n, oh_);
            addProperty("VFATClock", n, oh_);
            addProperty("VFATFallback", n, oh_);
            addProperty("CDCEClock", n, oh_);
            addProperty("CDCEFallback", n, oh_);
            addProperty("FPGAPLLLock", n, oh_);
            addProperty("CDCELock", n, oh_);
            addProperty("GTPLock", n, oh_);
            addProperty("FW", n, oh_);
            if (strcmp("VFATSettings",xercesc::XMLString::transcode(n->getNodeName()))==0) {
                DEBUG("[XML PARSER]: OH parsing: VFATSettings tag found");
                if (countChildElementNodes(n)) {
                    gemVFATProperties* vfat = new gemVFATProperties();
                    DEBUG("[XML PARSER]: OH parsing: create new VFATproperties object");
                    vfat->setDeviceId(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("VFATId"))->getNodeValue()));
                    DEBUG("[XML PARSER]: OH parsing: retrieve VFAT device ID");
                    gemSystem_->getSubDevicesRefs().back()->getSubDevicesRefs().back()->getSubDevicesRefs().back()->addSubDeviceRef(vfat);
                    DEBUG("[XML PARSER]: OH parsing: add new VFATproperties to the subdevices of the parent device");
                    gemSystem_->getSubDevicesRefs().back()->getSubDevicesRefs().back()->getSubDevicesRefs().back()->addSubDeviceId(vfat->getDeviceId());
                    DEBUG("[XML PARSER]: OH parsing: add VFAT device ID to the subdevices of the parent device");
                    parseVFAT2Settings(n);
                }
            }
        }    
        n = n->getNextSibling();
    }    
}

void gem::utils::gemXMLparser::parseVFAT2Settings(xercesc::DOMNode * pNode)
{
    DEBUG("[XML PARSER]: OH parsing: start VFAT parsing");
    xercesc::DOMNode * n = pNode->getFirstChild();
    gemVFATProperties* vfat_ = gemSystem_->getSubDevicesRefs().back()->getSubDevicesRefs().back()->getSubDevicesRefs().back()->getSubDevicesRefs().back();
    DEBUG("[XML PARSER]: VFAT parsing: retrieve VFAT device from the devices parent tree");
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {    
            addProperty("CalMode", n, vfat_);
            addProperty("CalPolarity", n, vfat_);
            addProperty("MSPolarity", n, vfat_);
            addProperty("TriggerMode", n, vfat_);
            addProperty("RunMode", n, vfat_);
            addProperty("ReHitCT", n, vfat_);
            addProperty("LVDSPowerSave", n, vfat_);
            addProperty("ProbeMode", n, vfat_);
            addProperty("DACSel", n, vfat_);
            addProperty("DigInSel", n, vfat_);
            addProperty("MSPulseLength", n, vfat_);
            addProperty("HitCountSel", n, vfat_);
            addProperty("DFTest", n, vfat_);
            addProperty("PbBG", n, vfat_);
            addProperty("TrimDACRange", n, vfat_);
            addProperty("IPreampIn", n, vfat_);
            addProperty("IPreampFeed", n, vfat_);
            addProperty("IPreampOut", n, vfat_);
            addProperty("IShaper", n, vfat_);
            addProperty("IShaperFeed", n, vfat_);
            addProperty("IComp", n, vfat_);
            addProperty("Latency", n, vfat_);
            addProperty("VCal", n, vfat_);
            addProperty("VThreshold1", n, vfat_);
            addProperty("VThreshold2", n, vfat_);
            addProperty("CalPhase", n, vfat_);
        }    
        n = n->getNextSibling();
    }
}

void gem::utils::gemXMLparser::addProperty(const char* key, const xercesc::DOMNode* n, gemGLIBProperties* glib)
{
    if (strcmp(key,xercesc::XMLString::transcode(n->getNodeName()))==0)
    {
        std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        glib->addDeviceProperty(key,value);
    }
}

void gem::utils::gemXMLparser::addProperty(const char* key, const xercesc::DOMNode* n, gemOHProperties* oh)
{
    if (strcmp(key,xercesc::XMLString::transcode(n->getNodeName()))==0)
    {
        std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        oh->addDeviceProperty(key,value);
    }
}

void gem::utils::gemXMLparser::addProperty(const char* key, const xercesc::DOMNode* n, gemVFATProperties* vfat)
{
    if (strcmp(key,xercesc::XMLString::transcode(n->getNodeName()))==0)
    {
        std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        vfat->addDeviceProperty(key,value);
    }
}

int gem::utils::gemXMLparser::countChildElementNodes(xercesc::DOMNode * pNode) {
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
