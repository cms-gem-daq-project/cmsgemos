///////////////////////////////////////////////
//
// Parse XML File
//
///////////////////////////////////////////////
#include "gem/supervisor/tbutils/VFAT2XMLParser.h"
#include "gem/hw/vfat/HwVFAT2.h"

gem::supervisor::tbutils::VFAT2XMLParser::VFAT2XMLParser(const std::string& xmlFile, gem::hw::vfat::HwVFAT2 *vfatDevice)
{
  xmlFile_    = xmlFile;  
  vfatDevice_ = vfatDevice;
}


gem::supervisor::tbutils::VFAT2XMLParser::~VFAT2XMLParser()
{
  vfatDevice_ = 0;
}


void gem::supervisor::tbutils::VFAT2XMLParser::parseXMLFile()
{
  //LOG4CPLUS_INFO(this->getApplicationLogger(), "Parsing XML file: " << xmlFile_);
    
  //
  /// Initialize XML4C system
  try{
    xercesc::XMLPlatformUtils::Initialize();
    // LOG4CPLUS_INFO(this->getApplicationLogger(), "Successfully initialized XML4C system");
  }
  catch (const xercesc::XMLException& toCatch) {
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


  catch (const xercesc::XMLException& e) {
    std::cerr << "An error occured during parsing\n   Message: "
              << xercesc::XMLString::transcode(e.getMessage()) << std::endl;
    errorsOccured = true;
  }


  catch (const xercesc::DOMException& e) {
    std::cerr << "An error occured during parsing\n   Message: "
              << xercesc::XMLString::transcode(e.msg) << std::endl;
    errorsOccured = true;
  }

  catch (...) {
    std::cerr << "An error occured during parsing" << std::endl;
    errorsOccured = true;
  }

  //LOG4CPLUS_INFO(this->getApplicationLogger(), "noErrors");

  // If the parse was successful, output the document data from the DOM tree

  if (!errorsOccured) {
    xercesc::DOMNode * pDoc = parser->getDocument();
    xercesc::DOMNode * n = pDoc->getFirstChild();
    while (n) {
      if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
        {
          if (strcmp("TURBO",xercesc::XMLString::transcode(n->getNodeName()))==0) {
            parseTURBO(n);
          }
        }
      n = n->getNextSibling();
    }
  }

  delete parser;
  xercesc::XMLPlatformUtils::Terminate();

  //vfatDevice_->getAllSettings();
  //vfatParams_ = vfatDevice_->getVFAT2Params();
}

///////////////////////////////////////////////
//
// Parse TURBO
//
///////////////////////////////////////////////

void gem::supervisor::tbutils::VFAT2XMLParser::parseTURBO(xercesc::DOMNode * pNode)
{
  //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseTURBO");
  xercesc::DOMNode * n = pNode->getFirstChild();
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
      {    
        if (strcmp("VFAT",xercesc::XMLString::transcode(n->getNodeName()))==0) {
          parseVFAT(n);
        }    
      }    
    n = n->getNextSibling();
  }    
}

///////////////////////////////////////////////
//
// Parse VFAT
//
///////////////////////////////////////////////

void gem::supervisor::tbutils::VFAT2XMLParser::parseVFAT(xercesc::DOMNode * pNode)
{
  //LOG4CPLUS_INFO(this->getApplicationLogger(), "parseVFAT");
  xercesc::DOMNode * n = pNode->getFirstChild();
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE)
      {
        //
        //  CR0
        //
        if (strcmp("CalMode",xercesc::XMLString::transcode(n->getNodeName()))==0)
          {
            //LOG4CPLUS_INFO(this->getApplicationLogger(), "CR0 CalMode: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
            vfatDevice_->setCalibrationMode((gem::hw::vfat::StringToCalibrationMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
          }
        if (strcmp("CalPolarity",xercesc::XMLString::transcode(n->getNodeName()))==0)
          {
            //LOG4CPLUS_INFO(this->getApplicationLogger(), "CR0 CalPolarity: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
            vfatDevice_->setCalPolarity((gem::hw::vfat::StringToCalPolarity.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
          }
        if (strcmp("MSPolarity",xercesc::XMLString::transcode(n->getNodeName()))==0)
          {
            //LOG4CPLUS_INFO(this->getApplicationLogger(), "CR0 MSPolarity: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
            vfatDevice_->setMSPolarity((gem::hw::vfat::StringToMSPolarity.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
          }
        if (strcmp("TrigMode",xercesc::XMLString::transcode(n->getNodeName()))==0)
          {
            //LOG4CPLUS_INFO(this->getApplicationLogger(), "CR0 TrigMode: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
            vfatDevice_->setTriggerMode((gem::hw::vfat::StringToTriggerMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
          }
        if (strcmp("WorkingMode",xercesc::XMLString::transcode(n->getNodeName()))==0)
          {
            //LOG4CPLUS_INFO(this->getApplicationLogger(), "CR0 WorkingMode: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
            vfatDevice_->setRunMode((gem::hw::vfat::StringToRunMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
          }
        //
        //  CR1
        //
        if (strcmp("ReHitCT",xercesc::XMLString::transcode(n->getNodeName()))==0)
          {
            //LOG4CPLUS_INFO(this->getApplicationLogger(), "CR1 ReHitCT: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
            vfatDevice_->setHitCountCycleTime((gem::hw::vfat::StringToReHitCT.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
          }
        if (strcmp("LVDSPowerSave",xercesc::XMLString::transcode(n->getNodeName()))==0)
          {
            //LOG4CPLUS_INFO(this->getApplicationLogger(), "CR1 LVDSPowerSave: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
            vfatDevice_->setLVDSMode((gem::hw::vfat::StringToLVDSPowerSave.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
          }
        if (strcmp("ProbeMode",xercesc::XMLString::transcode(n->getNodeName()))==0)
          {
            //LOG4CPLUS_INFO(this->getApplicationLogger(), "CR1 ProbeMode: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
            vfatDevice_->setProbeMode((gem::hw::vfat::StringToProbeMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
          }
        if (strcmp("DACsel",xercesc::XMLString::transcode(n->getNodeName()))==0)
          {
            //LOG4CPLUS_INFO(this->getApplicationLogger(), "CR1 DACsel: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
            vfatDevice_->setDACMode((gem::hw::vfat::StringToDACMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
          }
        //
        //  CR2
        //
        if (strcmp("DigInSel",xercesc::XMLString::transcode(n->getNodeName()))==0)
          {
            //LOG4CPLUS_INFO(this->getApplicationLogger(), "CR2 DigInSel: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
            vfatDevice_->setInputPadMode((gem::hw::vfat::StringToDigInSel.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
          }
        if (strcmp("MSPulseLenght",xercesc::XMLString::transcode(n->getNodeName()))==0)
          {
            //LOG4CPLUS_INFO(this->getApplicationLogger(), "CR2 MSPulseLength: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
            vfatDevice_->setMSPulseLength((gem::hw::vfat::StringToMSPulseLength.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
          }
        if (strcmp("HitCountSel",xercesc::XMLString::transcode(n->getNodeName()))==0)
          {
            //LOG4CPLUS_INFO(this->getApplicationLogger(), "CR2 HitCountSel: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
            vfatDevice_->setHitCountMode((gem::hw::vfat::StringToHitCountMode.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
          }
        //
        //  CR3
        //
        if (strcmp("DFTestPattern",xercesc::XMLString::transcode(n->getNodeName()))==0)
          {
            //LOG4CPLUS_INFO(this->getApplicationLogger(), "CR3 DFTestPattern: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
            vfatDevice_->sendTestPattern((gem::hw::vfat::StringToDFTestPattern.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
          }
        if (strcmp("PbBG",xercesc::XMLString::transcode(n->getNodeName()))==0)
          {
            //LOG4CPLUS_INFO(this->getApplicationLogger(), "CR3 PbBG: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
            vfatDevice_->setBandgapPad((gem::hw::vfat::StringToPbBG.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
          }
        if (strcmp("TrimDAC-range",xercesc::XMLString::transcode(n->getNodeName()))==0)
          {
            //LOG4CPLUS_INFO(this->getApplicationLogger(), "CR3 TrimDAC-range: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
            vfatDevice_->setTrimDACRange((gem::hw::vfat::StringToTrimDACRange.at(boost::to_upper_copy((std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())))));
          }
        //
        //  Current Bias
        //
        if (strcmp("IPreampIN",xercesc::XMLString::transcode(n->getNodeName()))==0) {
          //LOG4CPLUS_INFO(this->getApplicationLogger(), "IPreampIN: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
          vfatDevice_->writeVFATReg("IPreampIn",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
        }
        if (strcmp("IPreampFeed",xercesc::XMLString::transcode(n->getNodeName()))==0) {
          //LOG4CPLUS_INFO(this->getApplicationLogger(), "IPreampFeed: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
          vfatDevice_->writeVFATReg("IPreampFeed",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
        }
        if (strcmp("IPreampOut",xercesc::XMLString::transcode(n->getNodeName()))==0) {
          //LOG4CPLUS_INFO(this->getApplicationLogger(), "IPreampOut: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
          vfatDevice_->writeVFATReg("IPreampOut",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
        }
        if (strcmp("IShaper",xercesc::XMLString::transcode(n->getNodeName()))==0) {
          //LOG4CPLUS_INFO(this->getApplicationLogger(), "IShaper: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
          vfatDevice_->writeVFATReg("IShaper",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
        }
        if (strcmp("IShaperFeed",xercesc::XMLString::transcode(n->getNodeName()))==0) {
          //LOG4CPLUS_INFO(this->getApplicationLogger(), "IShaperFeed: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
          vfatDevice_->writeVFATReg("IShaperFeed",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
        }
        if (strcmp("IComp",xercesc::XMLString::transcode(n->getNodeName()))==0) {
          //LOG4CPLUS_INFO(this->getApplicationLogger(), "IComp: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
          vfatDevice_->writeVFATReg("IComp",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
        }
        //
        //  Bias Settings
        //
        if (strcmp("VFATLatency",xercesc::XMLString::transcode(n->getNodeName()))==0) {
          //LOG4CPLUS_INFO(this->getApplicationLogger(), "VFATLatency: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
          vfatDevice_->writeVFATReg("Latency",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
        }
        if (strcmp("VCal",xercesc::XMLString::transcode(n->getNodeName()))==0) {
          //LOG4CPLUS_INFO(this->getApplicationLogger(), "VCal: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
          vfatDevice_->writeVFATReg("VCal",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
        }
        if (strcmp("VThreshold1",xercesc::XMLString::transcode(n->getNodeName()))==0) {
          //LOG4CPLUS_INFO(this->getApplicationLogger(), "VThreshold1: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
          vfatDevice_->writeVFATReg("VThreshold1",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
        }
        if (strcmp("VThreshold2",xercesc::XMLString::transcode(n->getNodeName()))==0) {
          //LOG4CPLUS_INFO(this->getApplicationLogger(), "VThreshold2: " << xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue()));
          vfatDevice_->writeVFATReg("VThreshold2",atoi(xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue())));
        }
      }    
    n = n->getNextSibling();
  }    
}


