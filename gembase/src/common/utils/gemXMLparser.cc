///////////////////////////////////////////////
//
// Parse XML File
//
///////////////////////////////////////////////
#include "gem/utils/gemXMLparser.h"
#include "gem/hw/vfat/HwVFAT2.h"

gem::base::utils::gemXMLparser::gemXMLparser(std::string& xmlFile, gem::hw::vfat::HwVFAT2 *vfatDevice)
{
    xmlFile_    = xmlFile;  
    //vfatDevice_ = vfatDevice;
}


gem::base::utils::gemXMLparser::~gemXMLparser()
{
    //vfatDevice_ = 0;
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

    //delete parser;
    //xercesc::XMLPlatformUtils::Terminate();

    vfatDevice_->getAllSettings();
    //vfatParams_ = vfatDevice_->getVFAT2Params();
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
                parseCrate(n);
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
                parseGLIB(n);
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
                parseOH(n);
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
            if (strcmp("VFATSettings",xercesc::XMLString::transcode(n->getNodeName()))==0) parseVFAT2Settings(n);
        }    
        n = n->getNextSibling();
    }    
}

void gem::base::utils::gemXMLparser::parseVFAT2Settings(xercesc::DOMNode * pNode)
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseVFAT2Settings");
    xercesc::DOMNode * n = pNode->getFirstChild();
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {
            //if (strcmp("ChipID",xercesc::XMLString::transcode(n->getNodeName()))==0) VFAT2Ids.push_back(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
            if (strcmp("ControlRegisters",xercesc::XMLString::transcode(n->getNodeName()))==0) parseControlRegisters(n);
            if (strcmp("BiasSettings",xercesc::XMLString::transcode(n->getNodeName()))==0) parseBiasSettings(n);
        }
        n = n->getNextSibling();
    }
}

void gem::base::utils::gemXMLparser::parseControlRegisters(xercesc::DOMNode * pNode)
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseControlRegisters");
    xercesc::DOMNode * n = pNode->getFirstChild();
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {   
            if (strcmp("ControlRegister0",xercesc::XMLString::transcode(n->getNodeName()))==0) parseControlRegister0(n);
            if (strcmp("ControlRegister1",xercesc::XMLString::transcode(n->getNodeName()))==0) parseControlRegister1(n);
            if (strcmp("ControlRegister2",xercesc::XMLString::transcode(n->getNodeName()))==0) parseControlRegister2(n);
            if (strcmp("ControlRegister3",xercesc::XMLString::transcode(n->getNodeName()))==0) parseControlRegister3(n);
        }   
        n = n->getNextSibling();
    }
}

void gem::base::utils::gemXMLparser::parseControlRegister0(xercesc::DOMNode * pNode)
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseControlRegister0");
    xercesc::DOMNode * n = pNode->getFirstChild();
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {    
            if (strcmp("CalMode",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "CalMode: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
                vfatDevice_->setCalibrationMode((gem::hw::vfat::StringToCalibrationMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
            }
            if (strcmp("CalPolarity",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "CalPolarity: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
                vfatDevice_->setCalPolarity((gem::hw::vfat::StringToCalPolarity.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
            }
            if (strcmp("MSPolarity",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "MSPolarity: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
                vfatDevice_->setMSPolarity((gem::hw::vfat::StringToMSPolarity.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
            }
            if (strcmp("TriggerMode",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "TriggerMode: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
                vfatDevice_->setTriggerMode((gem::hw::vfat::StringToTriggerMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
            }
            if (strcmp("RunMode",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "RunMode: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
                vfatDevice_->setRunMode((gem::hw::vfat::StringToRunMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
            }
        }    
        n = n->getNextSibling();
    }
}

void gem::base::utils::gemXMLparser::parseControlRegister1(xercesc::DOMNode * pNode)
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseControlRegister1");
    xercesc::DOMNode * n = pNode->getFirstChild();
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {    
            if (strcmp("ReHitCT",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "ReHitCT: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
                vfatDevice_->setHitCountCycleTime((gem::hw::vfat::StringToReHitCT.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
            }
            if (strcmp("LVDSPowerSave",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "LVDSPowerSave: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
                vfatDevice_->setLVDSMode((gem::hw::vfat::StringToLVDSPowerSave.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
            }
            if (strcmp("ProbeMode",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "ProbeMode: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
                vfatDevice_->setProbeMode((gem::hw::vfat::StringToProbeMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
            }
            if (strcmp("DACSel",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "DACSel: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
                vfatDevice_->setDACMode((gem::hw::vfat::StringToDACMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
            }
        }    
        n = n->getNextSibling();
    }
}

void gem::base::utils::gemXMLparser::parseControlRegister2(xercesc::DOMNode * pNode)
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseControlRegister2");
    xercesc::DOMNode * n = pNode->getFirstChild();
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {    
            if (strcmp("DigInSel",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "DigInSel: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
                vfatDevice_->setInputPadMode((gem::hw::vfat::StringToDigInSel.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
            }
            if (strcmp("MSPulseLength",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "MSPulseLength: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
                vfatDevice_->setMSPulseLength((gem::hw::vfat::StringToMSPulseLength.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
            }
            if (strcmp("HitCountSel",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "HitCountSel: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
                vfatDevice_->setHitCountMode((gem::hw::vfat::StringToHitCountMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
            }
        }    
        n = n->getNextSibling();
    }
}

void gem::base::utils::gemXMLparser::parseControlRegister3(xercesc::DOMNode * pNode)
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseControlRegister3");
    xercesc::DOMNode * n = pNode->getFirstChild();
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {    
            if (strcmp("DFTest",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "DFTest: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
                vfatDevice_->sendTestPattern((gem::hw::vfat::StringToDFTestPattern.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
            }
            if (strcmp("PbBG",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "PbBG: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
                vfatDevice_->setBandgapPad((gem::hw::vfat::StringToPbBG.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
            }
            if (strcmp("TrimDACRange",xercesc::XMLString::transcode(n->getNodeName()))==0)
            {
                //LOG4CPLUS_INFO(this->getApplicationLogger(), "TrimDACRange: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
                vfatDevice_->setTrimDACRange((gem::hw::vfat::StringToTrimDACRange.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
            }
        }    
        n = n->getNextSibling();
    }
}

void gem::base::utils::gemXMLparser::parseBiasSettings(xercesc::DOMNode * pNode)
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseBiasSettings");
    xercesc::DOMNode * n = pNode->getFirstChild();
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {    
            if (strcmp("CurrentBias",xercesc::XMLString::transcode(n->getNodeName()))==0) parseCurrentBias(n);
            if (strcmp("Latency",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice_->writeVFATReg("Latency",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
            if (strcmp("VCal",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice_->writeVFATReg("VCal",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
            if (strcmp("VThreshold1",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice_->writeVFATReg("VThreshold1",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
            if (strcmp("VThreshold2",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice_->writeVFATReg("VThreshold2",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
            if (strcmp("CalPhase",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice_->writeVFATReg("CalPhase",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
        }    
        n = n->getNextSibling();
    }
}

void gem::base::utils::gemXMLparser::parseCurrentBias(xercesc::DOMNode * pNode)
{
    //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseCurrentBias");
    xercesc::DOMNode * n = pNode->getFirstChild();
    while (n) {
        if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {    
            if (strcmp("IPreampIn",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice_->writeVFATReg("IPreampIn",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
            if (strcmp("IPreampFeed",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice_->writeVFATReg("IPreampFeed",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
            if (strcmp("IPreampOut",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice_->writeVFATReg("IPreampOut",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
            if (strcmp("IShaper",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice_->writeVFATReg("IShaper",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
            if (strcmp("IShaperFeed",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice_->writeVFATReg("IShaperFeed",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
            if (strcmp("IComp",xercesc::XMLString::transcode(n->getNodeName()))==0) vfatDevice_->writeVFATReg("IComp",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
        }    
        n = n->getNextSibling();
    }
}
