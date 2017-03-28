/** @file VFAT2ConfigManager.cc */
//read and write vfat xml files
//gl = global, ch = channel
    

#include <gem/utils/vfat/VFAT2ConfigManager.h>

gem::utils::vfat::VFAT2ConfigManager::VFAT2ConfigManager(const std::string& glxmlFile,const std::string& chxmlFile) :
  m_glxmlFile(glxmlFile),
  m_chxmlFile(chxmlFile),
  m_gemLogger(log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("gem:utils:GEMXMLParser")))
{
  m_gemLogger.setLogLevel(log4cplus::DEBUG_LOG_LEVEL);
}

gem::utils::vfat::VFAT2ConfigManager::~VFAT2ConfigManager()
{
}

void gem::utils::vfat::VFAT2ConfigManager::setGLfile(const std::string& glxmlFile)
{
  m_glxmlFile = glxmlFile;
}

void gem::utils::vfat::VFAT2ConfigManager::setCHfile(const std::string& chxmlFile)
{
  m_chxmlFile = chxmlFile;
}

///////////////////////////////////////////////
//
// Set Timestamp
//
///////////////////////////////////////////////
void gem::utils::vfat::VFAT2ConfigManager::setTime(const std::string& currentTime)
{
  GLhead.run.buildTime = currentTime; 
  CHhead.run.buildTime = currentTime;
}

///////////////////////////////////////////////
//
// Set User
//
///////////////////////////////////////////////
void gem::utils::vfat::VFAT2ConfigManager::setUser(const std::string& xmlUser)
{
  GLhead.run.user = xmlUser; 
  CHhead.run.user = xmlUser;
}


///////////////////////////////////////////////
//
// Parse XML File
//
///////////////////////////////////////////////
void gem::utils::vfat::VFAT2ConfigManager::parseXMLFiles()
{
    parseGLXMLFile();
    parseCHXMLFile();
}

void gem::utils::vfat::VFAT2ConfigManager::parseGLXMLFile()
{
  INFO("Parsing Global XML file: " << m_glxmlFile);

  //
  /// Initialize XML4C system
  try {
    xercesc::XMLPlatformUtils::Initialize();
    INFO("Successfully initialized XML4C system");
  } catch(const xercesc::XMLException& toCatch) {
    ERROR("Error during Xerces-c Initialization." << std::endl
          << "  Exception message:"
          << xercesc::XMLString::transcode(toCatch.getMessage()));
    return;
  }

  //  Create our parser, then attach an error handler to the parser.
  //  The parser will call back to methods of the ErrorHandler if it
  //  discovers errors during the course of parsing the XML document.
  //
  xercesc::XercesDOMParser* parser = new xercesc::XercesDOMParser;
  DEBUG("Xerces parser created ");
  parser->setValidationScheme(xercesc::XercesDOMParser::Val_Auto);
  parser->setDoNamespaces(false);
  parser->setCreateEntityReferenceNodes(false);
  // parser->setCreateEntityReferenceNodes(true);
  // parser->setExpandEntityReferences(true);
  parser->setDoXInclude(true);
  // parser->setToCreateXMLDeclTypeNode(true);
  DEBUG("Xerces parser tuned up ");

  //  Parse the XML file, catching any XML exceptions that might propogate
  //  out of it.
  //
  bool errorsOccured = false;
  try {
    parser->parse(m_glxmlFile.c_str());
  } catch (const xercesc::XMLException& e) {
    ERROR("An error occured during parsing" << std::endl
          << "   Message: "
          << xercesc::XMLString::transcode(e.getMessage()));
    errorsOccured = true;
    // fileError = "An error occured during parsing of selected file. Please select another configuration file.";
  } catch (const xercesc::DOMException& e) {
    ERROR("An error occured during parsing" << std::endl
          << "   Message: "
          << xercesc::XMLString::transcode(e.msg));
    errorsOccured = true;
    // fileError = "An error occured during parsing of selected file. Please select another configuration file.";
  } catch (...) {
    ERROR("An error occured during parsing");
    errorsOccured = true;
    // fileError = "An error occured during parsing of selected file. Please select another configuration file.";
  }

  // If the parse was successful, output the document data from the DOM tree
  // crateIds.clear();
  // crateNodes.clear();

  if (!errorsOccured) {
    DEBUG("DOM tree created succesfully");
    xercesc::DOMNode* pDoc = parser->getDocument();
    DEBUG("Base node (getDocument) obtained");
    xercesc::DOMNode* rNode = pDoc->getFirstChild();
    xercesc::DOMNode* n = rNode->getFirstChild();
    DEBUG("First child node obtained");
    while (n) {
      DEBUG("Loop on child nodes");
      if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
        DEBUG("Element node found");
        DEBUG(xercesc::XMLString::transcode(n->getNodeName()));
        if (strcmp("HEADER", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
        {
          DEBUG("VFAT Global Header found");
          parseGLheader(n);
        }
        if (strcmp("DATA_SET", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
        {
          DEBUG("VFAT Global Dataset found");
          parseGLdataset(n);
        }
      }
      n = n->getNextSibling();
    }
  }

  DEBUG("Parser pointer " << parser);
  delete parser;
  DEBUG("Xerces parser deleted ");
  xercesc::XMLPlatformUtils::Terminate();
}

void gem::utils::vfat::VFAT2ConfigManager::parseCHXMLFile()
{
  INFO("Parsing Channel XML file: " << m_chxmlFile);

  //
  /// Initialize XML4C system
  try {
    xercesc::XMLPlatformUtils::Initialize();
    INFO("Successfully initialized XML4C system");
  } catch(const xercesc::XMLException& toCatch) {
    ERROR("Error during Xerces-c Initialization." << std::endl
          << "  Exception message:"
          << xercesc::XMLString::transcode(toCatch.getMessage()));
    return;
  }

  //  Create our parser, then attach an error handler to the parser.
  //  The parser will call back to methods of the ErrorHandler if it
  //  discovers errors during the course of parsing the XML document.
  //
  xercesc::XercesDOMParser* parser = new xercesc::XercesDOMParser;
  DEBUG("Xerces parser created ");
  parser->setValidationScheme(xercesc::XercesDOMParser::Val_Auto);
  parser->setDoNamespaces(false);
  parser->setCreateEntityReferenceNodes(false);
  // parser->setCreateEntityReferenceNodes(true);
  // parser->setExpandEntityReferences(true);
  parser->setDoXInclude(true);
  // parser->setToCreateXMLDeclTypeNode(true);
  DEBUG("Xerces parser tuned up ");

  //  Parse the XML file, catching any XML exceptions that might propogate
  //  out of it.
  //
  bool errorsOccured = false;
  try {
    parser->parse(m_chxmlFile.c_str());
  } catch (const xercesc::XMLException& e) {
    ERROR("An error occured during parsing" << std::endl
          << "   Message: "
          << xercesc::XMLString::transcode(e.getMessage()));
    errorsOccured = true;
    // fileError = "An error occured during parsing of selected file. Please select another configuration file.";
  } catch (const xercesc::DOMException& e) {
    ERROR("An error occured during parsing" << std::endl
          << "   Message: "
          << xercesc::XMLString::transcode(e.msg));
    errorsOccured = true;
    // fileError = "An error occured during parsing of selected file. Please select another configuration file.";
  } catch (...) {
    ERROR("An error occured during parsing");
    errorsOccured = true;
    // fileError = "An error occured during parsing of selected file. Please select another configuration file.";
  }

  // If the parse was successful, output the document data from the DOM tree
  // crateIds.clear();
  // crateNodes.clear();

  if (!errorsOccured) {
    DEBUG("DOM tree created succesfully");
    xercesc::DOMNode* pDoc = parser->getDocument();
    DEBUG("Base node (getDocument) obtained");
    xercesc::DOMNode* rNode = pDoc->getFirstChild();
    xercesc::DOMNode* n = rNode->getFirstChild();
    DEBUG("First child node obtained");
    while (n) {
      DEBUG("Loop on child nodes");
      if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
        DEBUG("Element node found");
        if (strcmp("HEADER", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
        {
          DEBUG("VFAT Channel Header found");
          parseCHheader(n);
        }
        if (strcmp("DATA_SET", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
        {
          DEBUG("VFAT Channel Dataset found");
          parseCHdataset(n);
        }
      }
      n = n->getNextSibling();
    }
  }

  DEBUG("Parser pointer " << parser);
  delete parser;
  DEBUG("Xerces parser deleted ");
  xercesc::XMLPlatformUtils::Terminate();
}

void gem::utils::vfat::VFAT2ConfigManager::parseGLheader(xercesc::DOMNode* pNode)
{
  INFO("parseGLheader");
  DEBUG("VFAT Global XML file header parsing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Global XML file header parsing: get first child");
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      if (strcmp("TYPE", xercesc::XMLString::transcode(n->getNodeName())) == 0) {
        DEBUG("VFAT Global XML file header parsing: VFAT global header-type found");
        parseGLheaderType(n);
      }
      if (strcmp("RUN", xercesc::XMLString::transcode(n->getNodeName())) == 0) {
        DEBUG("VFAT Global XML file header parsing: VFAT global header-run found");
        parseGLheaderRun(n);
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::vfat::VFAT2ConfigManager::parseGLheaderType(xercesc::DOMNode* pNode)
{
  INFO("parseGLheaderType");
  DEBUG("VFAT Global XML file header-type parsing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Global XML file header-type parsing: get first child");
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) 
    {
      if (strcmp("EXTENSION_TABLE_NAME", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("EXTENSION_TABLE_NAME found in Header-Type");
        GLhead.type.ETN = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("NAME", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("NAME found in Header-Type");
        GLhead.type.name = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::vfat::VFAT2ConfigManager::parseGLheaderRun(xercesc::DOMNode* pNode)
{
  INFO("parseGLheaderRun");
  DEBUG("VFAT Global XML file header-run parsing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Global XML file header-run parsing: get first child");
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) 
    {
      if (strcmp("RUN_NAME", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("RUN_NAME found in Header-Run");
        GLhead.run.configName = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("RUN_BEGIN_TIMESTAMP", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("RUN_BEGIN_TIMESTAMP found in Header-Run");
        GLhead.run.buildTime = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("COMMENT_DESCRIPTION", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("COMMENT_DESCRIPTION found in Header-Run");
        GLhead.run.comment = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("LOCATION", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("LOCATION found in Header-Run");
        GLhead.run.location = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("INITIATED_BY_USER", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("INITIATED_BY_USER found in Header-Run");
        GLhead.run.user = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::vfat::VFAT2ConfigManager::parseGLdataset(xercesc::DOMNode* pNode)
{
  INFO("parseGLdataset");
  DEBUG("VFAT Global XML file dataset parsing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Global XML file dataset parsing: get first child");
  while (n) 
  {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) 
    {
      if (strcmp("COMMENT_DESCRIPTION", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("COMMENT_DESCRIPTION found in dataset");
        glmetadata.comment = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("VERSION", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("VERSION found in dataset");
        glmetadata.version = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("PART", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("VFAT Global XML file header parsing: VFAT global dataset-part found");
        parseGLdatasetPart(n);
      }
      if (strcmp("DATA", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("VFAT Global XML file header parsing: VFAT global dataset-data found");
        parseGLdata(n);
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::vfat::VFAT2ConfigManager::parseGLdatasetPart(xercesc::DOMNode* pNode)
{
  INFO("parseGLdatasetPart");
  DEBUG("VFAT Global XML file dataset-part parsing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Global XML file dataset-part parsing: get first child");
  while (n) 
  {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) 
    {
      if (strcmp("KIND_OF_PART", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("KIND_OF_PART found in dataset-part");
        glmetadata.partType = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("SERIAL_NUMBER", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("VERSION found in dataset-part");
        glmetadata.serialN = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::vfat::VFAT2ConfigManager::parseGLdata(xercesc::DOMNode* pNode)//CAMHERE working point
{
  INFO("parseGLdata");
  DEBUG("VFAT Global XML file dataset-data parsing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Global XML file dataset-data parsing: get first child");
  while (n) 
  {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) 
    {
      if (strcmp("CR0_CALMODE", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CR0_CALMODE found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
	DEBUG(strBuf.c_str());
        localParams.calibMode = gem::hw::vfat::StringToCalibrationMode.at(boost::to_upper_copy(strBuf));
      }
      if (strcmp("CR0_CALPOLARITY", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CR0_CALPOLARITY found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.calPol = gem::hw::vfat::StringToCalPolarity.at(boost::to_upper_copy(strBuf));
      }
      if (strcmp("CR0_MSPOLARITY", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CR0_MSPOLARITY found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.msPol = gem::hw::vfat::StringToMSPolarity.at(boost::to_upper_copy(strBuf));
      }
      if (strcmp("CR0_TRGMODE", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CR0_TRGMODE found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.trigMode = gem::hw::vfat::StringToTriggerMode.at(boost::to_upper_copy(strBuf));
      }
      if (strcmp("CR0_RUNMODE", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CR0_RUNMODE found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.runMode = gem::hw::vfat::StringToRunMode.at(boost::to_upper_copy(strBuf));
      }
      if (strcmp("CR1_REHITCT", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CR1_REHITCT found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.reHitCT = gem::hw::vfat::StringToReHitCT.at(boost::to_upper_copy(strBuf));
      }
      if (strcmp("CR1_LVDSPWRSAV", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CR1_LVDSPWRSAV found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.lvdsMode = gem::hw::vfat::StringToLVDSPowerSave.at(boost::to_upper_copy(strBuf));
      }
      if (strcmp("CR1_PROBEMODE", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CR1_PROBEMODE found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.probeMode = gem::hw::vfat::StringToProbeMode.at(boost::to_upper_copy(strBuf));
      }
      if (strcmp("CR1_DACSELECT", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CR1_DACSELECT found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.dacMode = gem::hw::vfat::StringToDACMode.at(boost::to_upper_copy(strBuf));
      }
      if (strcmp("CR2_DIGINSEL", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CR2_DIGINSEL found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.digInSel = gem::hw::vfat::StringToDigInSel.at(boost::to_upper_copy(strBuf));
      }
      if (strcmp("CR2_MSPLSLEN", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CR2_MSPLSLEN found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.msPulseLen = gem::hw::vfat::StringToMSPulseLength.at(boost::to_upper_copy(strBuf));
      }
      if (strcmp("CR2_HITCNTSEL", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CR2_HITCNTSEL found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.hitCountMode = gem::hw::vfat::StringToHitCountMode.at(boost::to_upper_copy(strBuf));
      }
      if (strcmp("CR3_DFTST", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CR3_DFTST found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.sendTestPattern = gem::hw::vfat::StringToDFTestPattern.at(boost::to_upper_copy(strBuf));
      }
      if (strcmp("CR3_PB_BNDGAP", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CR3_PB_BNDGAP found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.padBandGap = gem::hw::vfat::StringToPbBG.at(boost::to_upper_copy(strBuf));
      }
      if (strcmp("CR3_TRIMDACRNG", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CR3_TRIMDACRNG found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.trimDACRange = gem::hw::vfat::StringToTrimDACRange.at(boost::to_upper_copy(strBuf));
      }
      if (strcmp("BIAS_IPREAMPIN", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("BIAS_IPREAMPIN found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.iPreampIn = std::atoi(strBuf.c_str());
      }
      if (strcmp("BIAS_IPREAMPFEED", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("BIAS_IPREAMPFEED found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.iPreampFeed = std::atoi(strBuf.c_str());
      }
      if (strcmp("BIAS_IPREAMPOUT", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("BIAS_IPREAMPOUT found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.iPreampOut = std::atoi(strBuf.c_str());
      }
      if (strcmp("BIAS_ISHAPER", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("BIAS_ISHAPER found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.iShaper = std::atoi(strBuf.c_str());
      }
      if (strcmp("BIAS_ISHAPERFEED", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("BIAS_ISHAPERFEED found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.iShaperFeed = std::atoi(strBuf.c_str());
      }
      if (strcmp("BIAS_ICOMP", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("BIAS_ICOMP found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.iComp = std::atoi(strBuf.c_str());
      }
      if (strcmp("BIAS_LATENCY", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("BIAS_LATENCY found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.latency = std::atoi(strBuf.c_str());
      }
      if (strcmp("BIAS_VCAL", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("BIAS_VCAL found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.vCal = std::atoi(strBuf.c_str());
      }
      if (strcmp("BIAS_VTHRESHOLD1", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("BIAS_VTHRESHOLD1 found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.vThresh1 = std::atoi(strBuf.c_str());
      }
      if (strcmp("BIAS_VTHRESHOLD2", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("BIAS_VTHRESHOLD2 found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.vThresh2 = std::atoi(strBuf.c_str());
      }
      if (strcmp("BIAS_CALPHASE", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("BIAS_CALPHASE found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        localParams.calPhase = std::atoi(strBuf.c_str());
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::vfat::VFAT2ConfigManager::parseCHheader(xercesc::DOMNode* pNode)
{
  INFO("parseCHheader");
  DEBUG("VFAT Channel XML file header parsing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Channel XML file header parsing: get first child");
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      if (strcmp("TYPE", xercesc::XMLString::transcode(n->getNodeName())) == 0) {
        DEBUG("VFAT Channel XML file header parsing: VFAT channel header-type found");
        parseCHheaderType(n);
      }
      if (strcmp("RUN", xercesc::XMLString::transcode(n->getNodeName())) == 0) {
        DEBUG("VFAT Channel XML file header parsing: VFAT channel header-run found");
        parseCHheaderRun(n);
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::vfat::VFAT2ConfigManager::parseCHheaderType(xercesc::DOMNode* pNode)
{
  INFO("parseCHheaderType");
  DEBUG("VFAT Channel XML file header-type parsing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Channel XML file header-type parsing: get first child");
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) 
    {
      if (strcmp("EXTENSION_TABLE_NAME", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("EXTENSION_TABLE_NAME found in Header-Type");
        CHhead.type.ETN = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("NAME", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("NAME found in Header-Type");
        CHhead.type.name = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::vfat::VFAT2ConfigManager::parseCHheaderRun(xercesc::DOMNode* pNode)
{
  INFO("parseCHheaderRun");
  DEBUG("VFAT Channel XML file header-run parsing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Channel XML file header-run parsing: get first child");
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) 
    {
      if (strcmp("RUN_NAME", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("RUN_NAME found in Header-Run");
        CHhead.run.configName = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("RUN_BEGIN_TIMESTAMP", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("RUN_BEGIN_TIMESTAMP found in Header-Run");
        CHhead.run.buildTime = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("COMMENT_DESCRIPTION", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("COMMENT_DESCRIPTION found in Header-Run");
        CHhead.run.comment = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("LOCATION", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("LOCATION found in Header-Run");
        CHhead.run.location = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("INITIATED_BY_USER", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("INITIATED_BY_USER found in Header-Run");
        CHhead.run.user = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::vfat::VFAT2ConfigManager::parseCHdataset(xercesc::DOMNode* pNode)
{
  INFO("parseCHdataset");
  DEBUG("VFAT Channel XML file dataset parsing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Channel XML file dataset parsing: get first child");
  while (n) 
  {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) 
    {
      if (strcmp("COMMENT_DESCRIPTION", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("COMMENT_DESCRIPTION found in dataset");
        chmetadata.comment = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("VERSION", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("VERSION found in dataset");
        chmetadata.version = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("PART", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("VFAT Channel XML file header parsing: VFAT channel dataset-part found");
        parseCHdatasetPart(n);
      }
      if (strcmp("DATA", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("VFAT Channel XML file header parsing: VFAT channel dataset-data found");
        parseCHdata(n);
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::vfat::VFAT2ConfigManager::parseCHdatasetPart(xercesc::DOMNode* pNode)
{
  INFO("parseCHdatasetPart");
  DEBUG("VFAT Channel XML file dataset-part parsing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Channel XML file dataset-part parsing: get first child");
  while (n) 
  {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) 
    {
      if (strcmp("KIND_OF_PART", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("KIND_OF_PART found in dataset-part");
        chmetadata.partType = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("SERIAL_NUMBER", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("VERSION found in dataset-part");
        chmetadata.serialN = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::vfat::VFAT2ConfigManager::parseCHdata(xercesc::DOMNode* pNode)
{
  INFO("parseCHdata");
  DEBUG("VFAT Channel XML file dataset-data parsing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Channel XML file dataset-data parsing: get first child");
  int CHN = 200;
  int CHcal = -99;
  int CHmask = -99;
  int CHtrim = -99;
  while (n) 
  {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) 
    {
      if (strcmp("VFAT_CHANNEL", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("VFAT_CHANNEL found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        CHN = std::atoi(strBuf.c_str());
      }
      if (strcmp("CHAN_CHCAL_VAL", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CHAN_CHCAL_VAL found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        CHcal = std::atoi(strBuf.c_str());
      }
      if (strcmp("CHAN_MASK_VAL", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CHAN_CHCAL_VAL found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        CHmask = std::atoi(strBuf.c_str());
      }
      if (strcmp("CHAN_TRIMDAC_VAL", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CHAN_TRIMDAC_VAL found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
        DEBUG(strBuf);
        CHtrim = std::atoi(strBuf.c_str());
      }
    }
    n = n->getNextSibling();
  }
  if(CHN > 128 || CHcal == -99 || CHmask == -99 || CHtrim == -99)
  {
      INFO("Data block incomplete!");
      DEBUG("Data block incomplete!");
  }
  else
  {
    std::stringstream msg;
    msg << "Loading data for Channel " << CHN << "  trim is " << CHtrim;
    DEBUG(msg.str());
    localParams.channels[CHN-1].calPulse = bool(CHcal);
    localParams.channels[CHN-1].mask = bool(CHmask);
    localParams.channels[CHN-1].trimDAC = CHtrim;
    localParams.channels[CHN-1].buildFullReg();
  }
}

///////////////////////////////////////////////
//
// Write XML File
//
///////////////////////////////////////////////
void gem::utils::vfat::VFAT2ConfigManager::writeXMLFiles()
{
    writeGLXMLFile();
    writeCHXMLFile();
}

void gem::utils::vfat::VFAT2ConfigManager::writeGLXMLFile() //Followed "Create a small document tree" example from https://xerces.apache.org/xerces-c/program-dom-3.html#Objectives
{
  INFO("Writing Global XML file: " << m_glxmlFile);

  //
  /// Initialize XML4C system
  try {
    xercesc::XMLPlatformUtils::Initialize();
    INFO("Successfully initialized XML4C system");
  } catch(const xercesc::XMLException& toCatch) {
    ERROR("Error during Xerces-c Initialization." << std::endl
          << "  Exception message:"
          << xercesc::XMLString::transcode(toCatch.getMessage()));
    return;
  }

    // Return the first registered implementation that has the desired features. In this case, we are after a DOM implementation that has the LS feature... or Load/Save.
    xercesc::DOMImplementation* implementation = xercesc::DOMImplementationRegistry::getDOMImplementation(xercesc::XMLString::transcode("LS"));

    DEBUG("Successfully initialized DOMImplementation");

    // Create a DOMLSSerializer which is used to serialize a DOM tree into an XML document.
    xercesc::DOMLSSerializer* serializer = ((xercesc::DOMImplementationLS*)implementation)->createLSSerializer();

    DEBUG("Successfully initialized DOMSerializer");

    // Make the output more human readable by inserting line feeds.
    if (serializer->getDomConfig()->canSetParameter(xercesc::XMLUni::fgDOMWRTFormatPrettyPrint, true))
        serializer->getDomConfig()->setParameter(xercesc::XMLUni::fgDOMWRTFormatPrettyPrint, true);

    // The end-of-line sequence of characters to be used in the XML being written out.
    serializer->setNewLine(xercesc::XMLString::transcode("\n"));

    // Convert the path into Xerces compatible XMLCh*.
    // xercesc::XMLCh* tempFilePath = xercesc::XMLString::transcode(filePath.c_str());
    std::string filePath = (std::string) m_glxmlFile.c_str();
    XMLCh* tempFilePath = xercesc::XMLString::transcode(filePath.c_str());

    DEBUG("Successfully set file path");

    // Specify the target for the XML output.
    xercesc::XMLFormatTarget* formatTarget = new xercesc::LocalFileFormatTarget(tempFilePath);

    DEBUG("Set format target to file path.");

    // Create a new empty output destination object.
    xercesc::DOMLSOutput* output = ((xercesc::DOMImplementationLS*)implementation)->createLSOutput();

    DEBUG("Successfully created output destination object");

    // Set the stream to our target.
    output->setByteStream(formatTarget);

    DEBUG("Successfully set stream to target");

    // Pointer to our DOMDocument.
    DOMDocument*        p_DOMDocument = NULL;

    DEBUG("Successfully created DOM document pointer.");

    //Adding the root node
    XMLCh tempStr[100];

    XMLString::transcode("ROOT", tempStr, 99);
    p_DOMDocument = implementation->createDocument(0, tempStr, 0);
    DOMElement*   root = p_DOMDocument->getDocumentElement();

    DEBUG("Successfully added root node");

    //`7MMF'  `7MMF'                     `7MM                  
    //  MM      MM                         MM                  
    //  MM      MM  .gP"Ya   ,6"Yb.   ,M""bMM  .gP"Ya `7Mb,od8 
    //  MMmmmmmmMM ,M'   Yb 8)   MM ,AP    MM ,M'   Yb  MM' "' 
    //  MM      MM 8M""""""  ,pm9MM 8MI    MM 8M""""""  MM     
    //  MM      MM YM.    , 8M   MM `Mb    MM YM.    ,  MM     
    //.JMML.  .JMML.`Mbmmd' `Moo9^Yo.`Wbmd"MML.`Mbmmd'.JMML. 

    //Adding the header node
    XMLString::transcode("HEADER", tempStr, 99);
    DOMElement*   n_header = p_DOMDocument->createElement(tempStr);
    root->appendChild(n_header);

    DEBUG("Successfully added header node");

    ////////////////////////////////////
    //Adding type node (header subnode)
    ////////////////////////////////////
    XMLString::transcode("TYPE", tempStr, 99);
    DOMElement* n_type = p_DOMDocument->createElement(tempStr);
    n_header->appendChild(n_type);

    DEBUG("Successfully added header-type node");
    
    //Adding extension table name node (header-type subnode)
    XMLString::transcode("EXTENSION_TABLE_NAME", tempStr, 99);
    DOMElement* n_etn = p_DOMDocument->createElement(tempStr);
    n_type->appendChild(n_etn);

    DEBUG("Successfully added header-type-etn node");    

    //Adding extension table name text
    XMLString::transcode(GLhead.type.ETN.c_str(), tempStr, 99);
    DOMText* ntxt_etn = p_DOMDocument->createTextNode(tempStr);
    n_etn->appendChild(ntxt_etn);

    DEBUG("Successfully added header-type-etn text");

    //Adding name node (header-type subnode)
    XMLString::transcode("NAME", tempStr, 99);
    DOMElement* n_name = p_DOMDocument->createElement(tempStr);
    n_type->appendChild(n_name);

    DEBUG("Successfully added header-type-name node");    

    //Adding name text
    XMLString::transcode(GLhead.type.name.c_str(), tempStr, 99);
    DOMText* ntxt_name = p_DOMDocument->createTextNode(tempStr);
    n_name->appendChild(ntxt_name);

    DEBUG("Successfully added header-type-name text");

    ////////////////////////////////////
    //Adding run node (header subnode)
    ////////////////////////////////////
    XMLString::transcode("RUN", tempStr, 99);
    DOMElement* n_run = p_DOMDocument->createElement(tempStr);
    n_header->appendChild(n_run);

    DEBUG("Successfully added header-run node");
    
    //Adding config name node (header-run subnode)
    XMLString::transcode("RUN_NAME", tempStr, 99);
    DOMElement* n_configName = p_DOMDocument->createElement(tempStr);
    n_run->appendChild(n_configName);

    DEBUG("Successfully added header-run-configName node");    

    //Adding config name text
    XMLString::transcode(GLhead.run.configName.c_str(), tempStr, 99);
    DOMText* ntxt_configName = p_DOMDocument->createTextNode(tempStr);
    n_configName->appendChild(ntxt_configName);

    DEBUG("Successfully added header-run-configName text");
    
    //Adding build timestamp node (header-run subnode)
    XMLString::transcode("RUN_BEGIN_TIMESTAMP", tempStr, 99);
    DOMElement* n_buildTime = p_DOMDocument->createElement(tempStr);
    n_run->appendChild(n_buildTime);

    DEBUG("Successfully added header-run-buildTime node");    

    //Adding build timestamp text
    XMLString::transcode(GLhead.run.buildTime.c_str(), tempStr, 99);
    DOMText* ntxt_buildTime = p_DOMDocument->createTextNode(tempStr);
    n_buildTime->appendChild(ntxt_buildTime);

    DEBUG("Successfully added header-run-buildTime text");
    
    //Adding comment node (header-run subnode)
    XMLString::transcode("COMMENT_DESCRIPTION", tempStr, 99);
    DOMElement* n_comment = p_DOMDocument->createElement(tempStr);
    n_run->appendChild(n_comment);

    DEBUG("Successfully added header-run-comment node");    

    //Adding comment text
    XMLString::transcode(GLhead.run.comment.c_str(), tempStr, 99);
    DOMText* ntxt_comment = p_DOMDocument->createTextNode(tempStr);
    n_comment->appendChild(ntxt_comment);

    DEBUG("Successfully added header-run-comment text");
    
    //Adding location node (header-run subnode)
    XMLString::transcode("LOCATION", tempStr, 99);
    DOMElement* n_location = p_DOMDocument->createElement(tempStr);
    n_run->appendChild(n_location);

    DEBUG("Successfully added header-run-location node");    

    //Adding location text
    XMLString::transcode(GLhead.run.location.c_str(), tempStr, 99);
    DOMText* ntxt_location = p_DOMDocument->createTextNode(tempStr);
    n_location->appendChild(ntxt_location);

    DEBUG("Successfully added header-run-location text");
    
    //Adding user node (header-run subnode)
    XMLString::transcode("INITIATED_BY_USER", tempStr, 99);
    DOMElement* n_user = p_DOMDocument->createElement(tempStr);
    n_run->appendChild(n_user);

    DEBUG("Successfully added header-run-user node");    

    //Adding user text
    XMLString::transcode(GLhead.run.user.c_str(), tempStr, 99);
    DOMText* ntxt_user = p_DOMDocument->createTextNode(tempStr);
    n_user->appendChild(ntxt_user);

    DEBUG("Successfully added header-run-user text");

    //`7MM"""Yb.            mm            
    //  MM    `Yb.          MM            
    //  MM     `Mb  ,6"Yb.mmMMmm  ,6"Yb.  
    //  MM      MM 8)   MM  MM   8)   MM  
    //  MM     ,MP  ,pm9MM  MM    ,pm9MM  
    //  MM    ,dP' 8M   MM  MM   8M   MM  
    //.JMMmmmdP'   `Moo9^Yo.`Mbmo`Moo9^Yo.

    //Adding the data set node
    XMLString::transcode("DATA_SET", tempStr, 99);
    DOMElement*   n_dataSet = p_DOMDocument->createElement(tempStr);
    root->appendChild(n_dataSet);

    ////////////////////////////////////
    //Adding comment node (dataSet subnode)
    ////////////////////////////////////
    XMLString::transcode("COMMENT_DESCRIPTION", tempStr, 99);
    DOMElement* n_dsComment = p_DOMDocument->createElement(tempStr);
    n_dataSet->appendChild(n_dsComment);

    DEBUG("Successfully added dataSet-comment node");

    //Adding comment text
    XMLString::transcode(glmetadata.comment.c_str(), tempStr, 99);
    DOMText* ntxt_dsComment = p_DOMDocument->createTextNode(tempStr);
    n_dsComment->appendChild(ntxt_dsComment);

    DEBUG("Successfully added dataSet-comment text");

    ////////////////////////////////////
    //Adding version node (dataSet subnode)
    ////////////////////////////////////
    XMLString::transcode("VERSION", tempStr, 99);
    DOMElement* n_version = p_DOMDocument->createElement(tempStr);
    n_dataSet->appendChild(n_version);

    DEBUG("Successfully added dataSet-version node");

    //Adding version text
    XMLString::transcode(glmetadata.version.c_str(), tempStr, 99);
    DOMText* ntxt_version = p_DOMDocument->createTextNode(tempStr);
    n_version->appendChild(ntxt_version);

    DEBUG("Successfully added dataSet-version text");
    
    ////////////////////////////////////
    //Adding part node (dataSet subnode)
    ////////////////////////////////////
    XMLString::transcode("PART", tempStr, 99);
    DOMElement* n_part = p_DOMDocument->createElement(tempStr);
    n_dataSet->appendChild(n_part);

    DEBUG("Successfully added dataSet-part node");
    
    //Adding part type node (dataSet-part subnode)
    XMLString::transcode("KIND_OF_PART", tempStr, 99);
    DOMElement* n_partType = p_DOMDocument->createElement(tempStr);
    n_part->appendChild(n_partType);

    DEBUG("Successfully added dataSet-part-partType node");    

    //Adding part type text
    XMLString::transcode(glmetadata.partType.c_str(), tempStr, 99);
    DOMText* ntxt_partType = p_DOMDocument->createTextNode(tempStr);
    n_partType->appendChild(ntxt_partType);

    DEBUG("Successfully added dataSet-part-partType text");
    
    //Adding serial number node (dataSet-part subnode)
    XMLString::transcode("SERIAL_NUMBER", tempStr, 99);
    DOMElement* n_serialN = p_DOMDocument->createElement(tempStr);
    n_part->appendChild(n_serialN);

    DEBUG("Successfully added dataSet-part-serialN node");    

    //Adding serial number text
    XMLString::transcode(glmetadata.serialN.c_str(), tempStr, 99);
    DOMText* ntxt_serialN = p_DOMDocument->createTextNode(tempStr);
    n_serialN->appendChild(ntxt_serialN);

    DEBUG("Successfully added dataSet-part-serialN text");

    ////////////////////////////////////
    //Adding data node (dataSet subnode)
    ////////////////////////////////////
    XMLString::transcode("DATA", tempStr, 99);
    DOMElement* n_data = p_DOMDocument->createElement(tempStr);
    n_dataSet->appendChild(n_data);

    DEBUG("Successfully added dataSet-data node");
        
    //Adding calibMode node (dataSet-part subnode)
    XMLString::transcode("CR0_CALMODE", tempStr, 99);
    DOMElement* n_calibMode = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_calibMode);

    DEBUG("Successfully added dataSet-data-calibMode node");    

    //Adding calibMode text
    
    std::string s_calibMode = gem::hw::vfat::CalibrationModeToString.at(localParams.calibMode);
    XMLString::transcode(s_calibMode.c_str(), tempStr, 99);
    DOMText* ntxt_calibMode = p_DOMDocument->createTextNode(tempStr);
    n_calibMode->appendChild(ntxt_calibMode);

    DEBUG("Successfully added dataSet-data-calibMode text");

    //Adding calPol node (dataSet-data subnode)
    XMLString::transcode("CR0_CALPOLARITY", tempStr, 99);
    DOMElement* n_calPol = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_calPol);

    DEBUG("Successfully added dataSet-data-calPol node");    

    //Adding calPol text
    std::string s_calPol = gem::hw::vfat::CalPolarityToString.at(localParams.calPol);
    XMLString::transcode(s_calPol.c_str(), tempStr, 99);
    DOMText* ntxt_calPol = p_DOMDocument->createTextNode(tempStr);
    n_calPol->appendChild(ntxt_calPol);

    DEBUG("Successfully added dataSet-data-calPol text");

    //Adding msPol node (dataSet-data subnode)
    XMLString::transcode("CR0_MSPOLARITY", tempStr, 99);
    DOMElement* n_msPol = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_msPol);

    DEBUG("Successfully added dataSet-data-msPol node");    

    //Adding msPol text
    std::string s_msPol = gem::hw::vfat::MSPolarityToString.at(localParams.msPol);
    XMLString::transcode(s_msPol.c_str(), tempStr, 99);
    DOMText* ntxt_msPol = p_DOMDocument->createTextNode(tempStr);
    n_msPol->appendChild(ntxt_msPol);

    DEBUG("Successfully added dataSet-data-msPol text");

    //Adding trigMode node (dataSet-data subnode)
    XMLString::transcode("CR0_TRGMODE", tempStr, 99);
    DOMElement* n_trigMode = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_trigMode);

    DEBUG("Successfully added dataSet-data-trigMode node");    

    //Adding trigMode text
    std::string s_trigMode = gem::hw::vfat::TriggerModeToString.at(localParams.trigMode);
    XMLString::transcode(s_trigMode.c_str(), tempStr, 99);
    DOMText* ntxt_trigMode = p_DOMDocument->createTextNode(tempStr);
    n_trigMode->appendChild(ntxt_trigMode);

    DEBUG("Successfully added dataSet-data-trigMode text");

    //Adding runMode node (dataSet-data subnode)
    XMLString::transcode("CR0_RUNMODE", tempStr, 99);
    DOMElement* n_runMode = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_runMode);

    DEBUG("Successfully added dataSet-data-runMode node");    

    //Adding runMode text
    std::string s_runMode = gem::hw::vfat::RunModeToString.at(localParams.runMode);
    XMLString::transcode(s_runMode.c_str(), tempStr, 99);
    DOMText* ntxt_runMode = p_DOMDocument->createTextNode(tempStr);
    n_runMode->appendChild(ntxt_runMode);

    DEBUG("Successfully added dataSet-data-runMode text");

    //Adding reHitCT node (dataSet-data subnode)
    XMLString::transcode("CR1_REHITCT", tempStr, 99);
    DOMElement* n_reHitCT = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_reHitCT);

    DEBUG("Successfully added dataSet-data-reHitCT node");    

    //Adding reHitCT text
    std::string s_reHitCT = gem::hw::vfat::ReHitCTToString.at(localParams.reHitCT);
    XMLString::transcode(s_reHitCT.c_str(), tempStr, 99);
    DOMText* ntxt_reHitCT = p_DOMDocument->createTextNode(tempStr);
    n_reHitCT->appendChild(ntxt_reHitCT);

    DEBUG("Successfully added dataSet-data-reHitCT text");

    //Adding lvdsMode node (dataSet-data subnode)
    XMLString::transcode("CR1_LVDSPWRSAV", tempStr, 99);
    DOMElement* n_lvdsMode = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_lvdsMode);

    DEBUG("Successfully added dataSet-data-lvdsMode node");    

    //Adding lvdsMode text
    std::string s_lvdsMode = gem::hw::vfat::LVDSPowerSaveToString.at(localParams.lvdsMode);
    XMLString::transcode(s_lvdsMode.c_str(), tempStr, 99);
    DOMText* ntxt_lvdsMode = p_DOMDocument->createTextNode(tempStr);
    n_lvdsMode->appendChild(ntxt_lvdsMode);

    DEBUG("Successfully added dataSet-data-lvdsMode text");

    //Adding probeMode node (dataSet-data subnode)
    XMLString::transcode("CR1_PROBEMODE", tempStr, 99);
    DOMElement* n_probeMode = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_probeMode);

    DEBUG("Successfully added dataSet-data-probeMode node");    

    //Adding probeMode text
    std::string s_probeMode = gem::hw::vfat::ProbeModeToString.at(localParams.probeMode);
    XMLString::transcode(s_probeMode.c_str(), tempStr, 99);
    DOMText* ntxt_probeMode = p_DOMDocument->createTextNode(tempStr);
    n_probeMode->appendChild(ntxt_probeMode);

    DEBUG("Successfully added dataSet-data-probeMode text");

    //Adding dacMode node (dataSet-data subnode)
    XMLString::transcode("CR1_DACSELECT", tempStr, 99);
    DOMElement* n_dacMode = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_dacMode);

    DEBUG("Successfully added dataSet-data-dacMode node");    

    //Adding dacMode text
    std::string s_dacMode = gem::hw::vfat::DACModeToString.at(localParams.dacMode);
    XMLString::transcode(s_dacMode.c_str(), tempStr, 99);
    DOMText* ntxt_dacMode = p_DOMDocument->createTextNode(tempStr);
    n_dacMode->appendChild(ntxt_dacMode);

    DEBUG("Successfully added dataSet-data-dacMode text");

    //Adding digInSel node (dataSet-data subnode)
    XMLString::transcode("CR2_DIGINSEL", tempStr, 99);
    DOMElement* n_digInSel = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_digInSel);

    DEBUG("Successfully added dataSet-data-digInSel node");    

    //Adding digInSel text
    std::string s_digInSel = gem::hw::vfat::DigInSelToString.at(localParams.digInSel);
    XMLString::transcode(s_digInSel.c_str(), tempStr, 99);
    DOMText* ntxt_digInSel = p_DOMDocument->createTextNode(tempStr);
    n_digInSel->appendChild(ntxt_digInSel);

    DEBUG("Successfully added dataSet-data-digInSel text");

    //Adding msPulseLen node (dataSet-data subnode)
    XMLString::transcode("CR2_MSPLSLEN", tempStr, 99);
    DOMElement* n_msPulseLen = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_msPulseLen);

    DEBUG("Successfully added dataSet-data-msPulseLen node");    

    //Adding msPulseLen text
    std::string s_msPulseLen = gem::hw::vfat::MSPulseLengthToString.at(localParams.msPulseLen);
    XMLString::transcode(s_msPulseLen.c_str(), tempStr, 99);
    DOMText* ntxt_msPulseLen = p_DOMDocument->createTextNode(tempStr);
    n_msPulseLen->appendChild(ntxt_msPulseLen);

    DEBUG("Successfully added dataSet-data-msPulseLen text");

    //Adding hitCountMode node (dataSet-data subnode)
    XMLString::transcode("CR2_HITCNTSEL", tempStr, 99);
    DOMElement* n_hitCountMode = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_hitCountMode);

    DEBUG("Successfully added dataSet-data-hitCountMode node");    

    //Adding hitCountMode text
    std::string s_hitCountMode = gem::hw::vfat::HitCountModeToString.at(localParams.hitCountMode);
    XMLString::transcode(s_hitCountMode.c_str(), tempStr, 99);
    DOMText* ntxt_hitCountMode = p_DOMDocument->createTextNode(tempStr);
    n_hitCountMode->appendChild(ntxt_hitCountMode);

    DEBUG("Successfully added dataSet-data-hitCountMode text");

    //Adding sendTestPattern node (dataSet-data subnode)
    XMLString::transcode("CR3_DFTST", tempStr, 99);
    DOMElement* n_sendTestPattern = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_sendTestPattern);

    DEBUG("Successfully added dataSet-data-sendTestPattern node");    

    //Adding sendTestPattern text
    std::string s_sendTestPattern = gem::hw::vfat::DFTestPatternToString.at(localParams.sendTestPattern);
    XMLString::transcode(s_sendTestPattern.c_str(), tempStr, 99);
    DOMText* ntxt_sendTestPattern = p_DOMDocument->createTextNode(tempStr);
    n_sendTestPattern->appendChild(ntxt_sendTestPattern);

    DEBUG("Successfully added dataSet-data-sendTestPattern text");

    //Adding padBandGap node (dataSet-data subnode)
    XMLString::transcode("CR3_PB_BNDGAP", tempStr, 99);
    DOMElement* n_padBandGap = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_padBandGap);

    DEBUG("Successfully added dataSet-data-padBandGap node");    

    //Adding padBandGap text
    std::string s_padBandGap = gem::hw::vfat::PbBGToString.at(localParams.padBandGap);
    XMLString::transcode(s_padBandGap.c_str(), tempStr, 99);
    DOMText* ntxt_padBandGap = p_DOMDocument->createTextNode(tempStr);
    n_padBandGap->appendChild(ntxt_padBandGap);

    DEBUG("Successfully added dataSet-data-padBandGap text");

    //Adding trimDACRange node (dataSet-data subnode)
    XMLString::transcode("CR3_TRIMDACRNG", tempStr, 99);
    DOMElement* n_trimDACRange = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_trimDACRange);

    DEBUG("Successfully added dataSet-data-trimDACRange node");    

    //Adding trimDACRange text
    std::string s_trimDACRange = gem::hw::vfat::TrimDACRangeToString.at(localParams.trimDACRange);
    XMLString::transcode(s_trimDACRange.c_str(), tempStr, 99);
    DOMText* ntxt_trimDACRange = p_DOMDocument->createTextNode(tempStr);
    n_trimDACRange->appendChild(ntxt_trimDACRange);

    DEBUG("Successfully added dataSet-data-trimDACRange text");
    
    //Adding iPreampIn node (dataSet-data subnode)
    XMLString::transcode("BIAS_IPREAMPIN", tempStr, 99);
    DOMElement* n_iPreampIn = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_iPreampIn);

    DEBUG("Successfully added dataSet-data-iPreampIn node");    
    
    //Adding iPreampIn text
    std::string s_iPreampIn = std::to_string((long long int)localParams.iPreampIn);
    XMLString::transcode(s_iPreampIn.c_str(), tempStr, 99);
    DOMText* ntxt_iPreampIn = p_DOMDocument->createTextNode(tempStr);
    n_iPreampIn->appendChild(ntxt_iPreampIn);

    DEBUG("Successfully added dataSet-data-iPreampIn text");
    
    //Adding iPreampFeed node (dataSet-data subnode)
    XMLString::transcode("BIAS_IPREAMPFEED", tempStr, 99);
    DOMElement* n_iPreampFeed = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_iPreampFeed);

    DEBUG("Successfully added dataSet-data-iPreampFeed node");    

    //Adding iPreampFeed text
    std::string s_iPreampFeed = std::to_string((long long int)localParams.iPreampFeed);
    XMLString::transcode(s_iPreampFeed.c_str(), tempStr, 99);
    DOMText* ntxt_iPreampFeed = p_DOMDocument->createTextNode(tempStr);
    n_iPreampFeed->appendChild(ntxt_iPreampFeed);

    DEBUG("Successfully added dataSet-data-iPreampFeed text");

    //Adding iPreampOut node (dataSet-data subnode)
    XMLString::transcode("BIAS_IPREAMPOUT", tempStr, 99);
    DOMElement* n_iPreampOut = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_iPreampOut);

    DEBUG("Successfully added dataSet-data-iPreampOut node");    

    //Adding iPreampOut text
    std::string s_iPreampOut = std::to_string((long long int)localParams.iPreampOut);
    XMLString::transcode(s_iPreampOut.c_str(), tempStr, 99);
    DOMText* ntxt_iPreampOut = p_DOMDocument->createTextNode(tempStr);
    n_iPreampOut->appendChild(ntxt_iPreampOut);

    DEBUG("Successfully added dataSet-data-iPreampOut text");

    //Adding iShaper node (dataSet-data subnode)
    XMLString::transcode("BIAS_ISHAPER", tempStr, 99);
    DOMElement* n_iShaper = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_iShaper);

    DEBUG("Successfully added dataSet-data-iShaper node");    

    //Adding iShaper text
    std::string s_iShaper = std::to_string((long long int)localParams.iShaper);
    XMLString::transcode(s_iShaper.c_str(), tempStr, 99);
    DOMText* ntxt_iShaper = p_DOMDocument->createTextNode(tempStr);
    n_iShaper->appendChild(ntxt_iShaper);

    DEBUG("Successfully added dataSet-data-iShaper text");

    //Adding iShaperFeed node (dataSet-data subnode)
    XMLString::transcode("BIAS_ISHAPERFEED", tempStr, 99);
    DOMElement* n_iShaperFeed = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_iShaperFeed);

    DEBUG("Successfully added dataSet-data-iShaperFeed node");    

    //Adding iShaperFeed text
    std::string s_iShaperFeed = std::to_string((long long int)localParams.iShaperFeed);
    XMLString::transcode(s_iShaperFeed.c_str(), tempStr, 99);
    DOMText* ntxt_iShaperFeed = p_DOMDocument->createTextNode(tempStr);
    n_iShaperFeed->appendChild(ntxt_iShaperFeed);

    DEBUG("Successfully added dataSet-data-iShaperFeed text");

    //Adding iComp node (dataSet-data subnode)
    XMLString::transcode("BIAS_ICOMP", tempStr, 99);
    DOMElement* n_iComp = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_iComp);

    DEBUG("Successfully added dataSet-data-iComp node");    

    //Adding iComp text
    std::string s_iComp = std::to_string((long long int)localParams.iComp);
    XMLString::transcode(s_iComp.c_str(), tempStr, 99);
    DOMText* ntxt_iComp = p_DOMDocument->createTextNode(tempStr);
    n_iComp->appendChild(ntxt_iComp);

    DEBUG("Successfully added dataSet-data-iComp text");

    //Adding latency node (dataSet-data subnode)
    XMLString::transcode("BIAS_LATENCY", tempStr, 99);
    DOMElement* n_latency = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_latency);

    DEBUG("Successfully added dataSet-data-latency node");    

    //Adding latency text
    std::string s_latency = std::to_string((long long int)localParams.latency);
    XMLString::transcode(s_latency.c_str(), tempStr, 99);
    DOMText* ntxt_latency = p_DOMDocument->createTextNode(tempStr);
    n_latency->appendChild(ntxt_latency);

    DEBUG("Successfully added dataSet-data-latency text");

    //Adding vCal node (dataSet-data subnode)
    XMLString::transcode("BIAS_VCAL", tempStr, 99);
    DOMElement* n_vCal = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_vCal);

    DEBUG("Successfully added dataSet-data-vCal node");    

    //Adding vCal text
    std::string s_vCal = std::to_string((long long int)localParams.vCal);
    XMLString::transcode(s_vCal.c_str(), tempStr, 99);
    DOMText* ntxt_vCal = p_DOMDocument->createTextNode(tempStr);
    n_vCal->appendChild(ntxt_vCal);

    DEBUG("Successfully added dataSet-data-vCal text");

    //Adding vThresh1 node (dataSet-data subnode)
    XMLString::transcode("BIAS_VTHRESHOLD1", tempStr, 99);
    DOMElement* n_vThresh1 = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_vThresh1);

    DEBUG("Successfully added dataSet-data-vThresh1 node");    

    //Adding vThresh1 text
    std::string s_vThresh1 = std::to_string((long long int)localParams.vThresh1);
    XMLString::transcode(s_vThresh1.c_str(), tempStr, 99);
    DOMText* ntxt_vThresh1 = p_DOMDocument->createTextNode(tempStr);
    n_vThresh1->appendChild(ntxt_vThresh1);

    DEBUG("Successfully added dataSet-data-vThresh1 text");

    //Adding vThresh2 node (dataSet-data subnode)
    XMLString::transcode("BIAS_VTHRESHOLD2", tempStr, 99);
    DOMElement* n_vThresh2 = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_vThresh2);

    DEBUG("Successfully added dataSet-data-vThresh2 node");    

    //Adding vThresh2 text
    std::string s_vThresh2 = std::to_string((long long int)localParams.vThresh2);
    XMLString::transcode(s_vThresh2.c_str(), tempStr, 99);
    DOMText* ntxt_vThresh2 = p_DOMDocument->createTextNode(tempStr);
    n_vThresh2->appendChild(ntxt_vThresh2);

    DEBUG("Successfully added dataSet-data-vThresh2 text");

    //Adding calPhase node (dataSet-data subnode)
    XMLString::transcode("BIAS_CALPHASE", tempStr, 99);
    DOMElement* n_calPhase = p_DOMDocument->createElement(tempStr);
    n_data->appendChild(n_calPhase);

    DEBUG("Successfully added dataSet-data-calPhase node");    

    //Adding calPhase text
    std::string s_calPhase = std::to_string((long long int)localParams.calPhase);
    XMLString::transcode(s_calPhase.c_str(), tempStr, 99);
    DOMText* ntxt_calPhase = p_DOMDocument->createTextNode(tempStr);
    n_calPhase->appendChild(ntxt_calPhase);

    DEBUG("Successfully added dataSet-data-calPhase text");

    // optionally, call release() to release the resource associated with the range after done
    DOMRange* range = p_DOMDocument->createRange();
    range->release();    

    // Write the serialized output to the destination.
    serializer->write(p_DOMDocument, output);

    DEBUG("Successfully wrote domdocument");

    // Cleanup.
    serializer->release();
    xercesc::XMLString::release(&tempFilePath);
    delete formatTarget;
    output->release();

    DEBUG("Done!");
}

void gem::utils::vfat::VFAT2ConfigManager::writeCHXMLFile()
{
  INFO("Writing Channel XML file: " << m_chxmlFile);

  ///
  /// Initialize XML4C system
  try {
    xercesc::XMLPlatformUtils::Initialize();
    INFO("Successfully initialized XML4C system");
  } catch(const xercesc::XMLException& toCatch) {
    ERROR("Error during Xerces-c Initialization." << std::endl
          << "  Exception message:"
          << xercesc::XMLString::transcode(toCatch.getMessage()));
    return;
  }

    // Return the first registered implementation that has the desired features. In this case, we are after a DOM implementation that has the LS feature... or Load/Save.
    xercesc::DOMImplementation* implementation = xercesc::DOMImplementationRegistry::getDOMImplementation(xercesc::XMLString::transcode("LS"));

    DEBUG("Successfully initialized DOMImplementation");

    // Create a DOMLSSerializer which is used to serialize a DOM tree into an XML document.
    xercesc::DOMLSSerializer* serializer = ((xercesc::DOMImplementationLS*)implementation)->createLSSerializer();

    DEBUG("Successfully initialized DOMSerializer");

    // Make the output more human readable by inserting line feeds.
    if (serializer->getDomConfig()->canSetParameter(xercesc::XMLUni::fgDOMWRTFormatPrettyPrint, true))
        serializer->getDomConfig()->setParameter(xercesc::XMLUni::fgDOMWRTFormatPrettyPrint, true);

    // The end-of-line sequence of characters to be used in the XML being written out.
    serializer->setNewLine(xercesc::XMLString::transcode("\n"));

    // Convert the path into Xerces compatible XMLCh*.
    // xercesc::XMLCh* tempFilePath = xercesc::XMLString::transcode(filePath.c_str());
    std::string filePath = (std::string) m_chxmlFile.c_str();
    XMLCh* tempFilePath = xercesc::XMLString::transcode(filePath.c_str());

    DEBUG("Successfully set file path");

    // Specify the target for the XML output.
    xercesc::XMLFormatTarget* formatTarget = new xercesc::LocalFileFormatTarget(tempFilePath);

    DEBUG("Set format target to file path.");

    // Create a new empty output destination object.
    xercesc::DOMLSOutput* output = ((xercesc::DOMImplementationLS*)implementation)->createLSOutput();

    DEBUG("Successfully created output destination object");

    // Set the stream to our target.
    output->setByteStream(formatTarget);

    DEBUG("Successfully set stream to target");

    // Pointer to our DOMDocument.
    DOMDocument*        p_DOMDocument = NULL;

    DEBUG("Successfully created DOM document pointer.");

    //Adding the root node
    XMLCh tempStr[100];

    XMLString::transcode("ROOT", tempStr, 99);
    p_DOMDocument = implementation->createDocument(0, tempStr, 0);
    DOMElement*   root = p_DOMDocument->getDocumentElement();

    DEBUG("Successfully added root node");

    //`7MMF'  `7MMF'                     `7MM                  
    //  MM      MM                         MM                  
    //  MM      MM  .gP"Ya   ,6"Yb.   ,M""bMM  .gP"Ya `7Mb,od8 
    //  MMmmmmmmMM ,M'   Yb 8)   MM ,AP    MM ,M'   Yb  MM' "' 
    //  MM      MM 8M""""""  ,pm9MM 8MI    MM 8M""""""  MM     
    //  MM      MM YM.    , 8M   MM `Mb    MM YM.    ,  MM     
    //.JMML.  .JMML.`Mbmmd' `Moo9^Yo.`Wbmd"MML.`Mbmmd'.JMML. 

    //Adding the header node
    XMLString::transcode("HEADER", tempStr, 99);
    DOMElement*   n_header = p_DOMDocument->createElement(tempStr);
    root->appendChild(n_header);

    DEBUG("Successfully added header node");

    ////////////////////////////////////
    //Adding type node (header subnode)
    ////////////////////////////////////
    XMLString::transcode("TYPE", tempStr, 99);
    DOMElement* n_type = p_DOMDocument->createElement(tempStr);
    n_header->appendChild(n_type);

    DEBUG("Successfully added header-type node");
    
    //Adding extension table name node (header-type subnode)
    XMLString::transcode("EXTENSION_TABLE_NAME", tempStr, 99);
    DOMElement* n_etn = p_DOMDocument->createElement(tempStr);
    n_type->appendChild(n_etn);

    DEBUG("Successfully added header-type-etn node");    

    //Adding extension table name text
    XMLString::transcode(CHhead.type.ETN.c_str(), tempStr, 99);
    DOMText* ntxt_etn = p_DOMDocument->createTextNode(tempStr);
    n_etn->appendChild(ntxt_etn);

    DEBUG("Successfully added header-type-etn text");

    //Adding name node (header-type subnode)
    XMLString::transcode("NAME", tempStr, 99);
    DOMElement* n_name = p_DOMDocument->createElement(tempStr);
    n_type->appendChild(n_name);

    DEBUG("Successfully added header-type-name node");    

    //Adding name text
    XMLString::transcode(CHhead.type.name.c_str(), tempStr, 99);
    DOMText* ntxt_name = p_DOMDocument->createTextNode(tempStr);
    n_name->appendChild(ntxt_name);

    DEBUG("Successfully added header-type-name text");

    ////////////////////////////////////
    //Adding run node (header subnode)
    ////////////////////////////////////
    XMLString::transcode("RUN", tempStr, 99);
    DOMElement* n_run = p_DOMDocument->createElement(tempStr);
    n_header->appendChild(n_run);

    DEBUG("Successfully added header-run node");
    
    //Adding config name node (header-run subnode)
    XMLString::transcode("RUN_NAME", tempStr, 99);
    DOMElement* n_configName = p_DOMDocument->createElement(tempStr);
    n_run->appendChild(n_configName);

    DEBUG("Successfully added header-run-configName node");    

    //Adding config name text
    XMLString::transcode(CHhead.run.configName.c_str(), tempStr, 99);
    DOMText* ntxt_configName = p_DOMDocument->createTextNode(tempStr);
    n_configName->appendChild(ntxt_configName);

    DEBUG("Successfully added header-run-configName text");
    
    //Adding build timestamp node (header-run subnode)
    XMLString::transcode("RUN_BEGIN_TIMESTAMP", tempStr, 99);
    DOMElement* n_buildTime = p_DOMDocument->createElement(tempStr);
    n_run->appendChild(n_buildTime);

    DEBUG("Successfully added header-run-buildTime node");    

    //Adding build timestamp text
    XMLString::transcode(CHhead.run.buildTime.c_str(), tempStr, 99);
    DOMText* ntxt_buildTime = p_DOMDocument->createTextNode(tempStr);
    n_buildTime->appendChild(ntxt_buildTime);

    DEBUG("Successfully added header-run-buildTime text");
    
    //Adding comment node (header-run subnode)
    XMLString::transcode("COMMENT_DESCRIPTION", tempStr, 99);
    DOMElement* n_comment = p_DOMDocument->createElement(tempStr);
    n_run->appendChild(n_comment);

    DEBUG("Successfully added header-run-comment node");    

    //Adding comment text
    XMLString::transcode(CHhead.run.comment.c_str(), tempStr, 99);
    DOMText* ntxt_comment = p_DOMDocument->createTextNode(tempStr);
    n_comment->appendChild(ntxt_comment);

    DEBUG("Successfully added header-run-comment text");
    
    //Adding location node (header-run subnode)
    XMLString::transcode("LOCATION", tempStr, 99);
    DOMElement* n_location = p_DOMDocument->createElement(tempStr);
    n_run->appendChild(n_location);

    DEBUG("Successfully added header-run-location node");    

    //Adding location text
    XMLString::transcode(CHhead.run.location.c_str(), tempStr, 99);
    DOMText* ntxt_location = p_DOMDocument->createTextNode(tempStr);
    n_location->appendChild(ntxt_location);

    DEBUG("Successfully added header-run-location text");
    
    //Adding user node (header-run subnode)
    XMLString::transcode("INITIATED_BY_USER", tempStr, 99);
    DOMElement* n_user = p_DOMDocument->createElement(tempStr);
    n_run->appendChild(n_user);

    DEBUG("Successfully added header-run-user node");    

    //Adding user text
    XMLString::transcode(CHhead.run.user.c_str(), tempStr, 99);
    DOMText* ntxt_user = p_DOMDocument->createTextNode(tempStr);
    n_user->appendChild(ntxt_user);

    DEBUG("Successfully added header-run-user text");

    //`7MM"""Yb.            mm            
    //  MM    `Yb.          MM            
    //  MM     `Mb  ,6"Yb.mmMMmm  ,6"Yb.  
    //  MM      MM 8)   MM  MM   8)   MM  
    //  MM     ,MP  ,pm9MM  MM    ,pm9MM  
    //  MM    ,dP' 8M   MM  MM   8M   MM  
    //.JMMmmmdP'   `Moo9^Yo.`Mbmo`Moo9^Yo.

    //Adding the data set node
    XMLString::transcode("DATA_SET", tempStr, 99);
    DOMElement*   n_dataSet = p_DOMDocument->createElement(tempStr);
    root->appendChild(n_dataSet);

    ////////////////////////////////////
    //Adding comment node (dataSet subnode)
    ////////////////////////////////////
    XMLString::transcode("COMMENT_DESCRIPTION", tempStr, 99);
    DOMElement* n_dsComment = p_DOMDocument->createElement(tempStr);
    n_dataSet->appendChild(n_dsComment);

    DEBUG("Successfully added dataSet-comment node");

    //Adding comment text
    XMLString::transcode(chmetadata.comment.c_str(), tempStr, 99);
    DOMText* ntxt_dsComment = p_DOMDocument->createTextNode(tempStr);
    n_dsComment->appendChild(ntxt_dsComment);

    DEBUG("Successfully added dataSet-comment text");

    ////////////////////////////////////
    //Adding version node (dataSet subnode)
    ////////////////////////////////////
    XMLString::transcode("VERSION", tempStr, 99);
    DOMElement* n_version = p_DOMDocument->createElement(tempStr);
    n_dataSet->appendChild(n_version);

    DEBUG("Successfully added dataSet-version node");

    //Adding version text
    XMLString::transcode(chmetadata.version.c_str(), tempStr, 99);
    DOMText* ntxt_version = p_DOMDocument->createTextNode(tempStr);
    n_version->appendChild(ntxt_version);

    DEBUG("Successfully added dataSet-version text");
    
    ////////////////////////////////////
    //Adding part node (dataSet subnode)
    ////////////////////////////////////
    XMLString::transcode("PART", tempStr, 99);
    DOMElement* n_part = p_DOMDocument->createElement(tempStr);
    n_dataSet->appendChild(n_part);

    DEBUG("Successfully added dataSet-part node");
    
    //Adding part type node (dataSet-part subnode)
    XMLString::transcode("KIND_OF_PART", tempStr, 99);
    DOMElement* n_partType = p_DOMDocument->createElement(tempStr);
    n_part->appendChild(n_partType);

    DEBUG("Successfully added dataSet-part-partType node");    

    //Adding part type text
    XMLString::transcode(chmetadata.partType.c_str(), tempStr, 99);
    DOMText* ntxt_partType = p_DOMDocument->createTextNode(tempStr);
    n_partType->appendChild(ntxt_partType);

    DEBUG("Successfully added dataSet-part-partType text");
    
    //Adding serial number node (dataSet-part subnode)
    XMLString::transcode("SERIAL_NUMBER", tempStr, 99);
    DOMElement* n_serialN = p_DOMDocument->createElement(tempStr);
    n_part->appendChild(n_serialN);

    DEBUG("Successfully added dataSet-part-serialN node");    

    //Adding serial number text
    XMLString::transcode(chmetadata.serialN.c_str(), tempStr, 99);
    DOMText* ntxt_serialN = p_DOMDocument->createTextNode(tempStr);
    n_serialN->appendChild(ntxt_serialN);

    DEBUG("Successfully added dataSet-part-serialN text");

    ////////////////////////////////////
    //Adding data node (dataSet subnode)
    ////////////////////////////////////
    for (int i_chan = 0; i_chan < 128; i_chan++){
      int i_chanXML = i_chan + 1;//channel number starts at 1 in the xml document

      XMLString::transcode("DATA", tempStr, 99);
      DOMElement* n_data = p_DOMDocument->createElement(tempStr);
      n_dataSet->appendChild(n_data);

      std::stringstream msg;
      msg << "Successfully added dataSet-data node for channel " << i_chanXML;
      DEBUG(msg.str());

      //Adding channel number node (dataSet-data subnode)
      XMLString::transcode("VFAT_CHANNEL", tempStr, 99);
      DOMElement* n_channel = p_DOMDocument->createElement(tempStr);
      n_data->appendChild(n_channel);

      DEBUG("Successfully added dataSet-data-channel node");    
    
      //Adding channel text
      std::string s_channel = std::to_string((long long int) i_chanXML);
      XMLString::transcode(s_channel.c_str(), tempStr, 99);
      DOMText* ntxt_channel = p_DOMDocument->createTextNode(tempStr);
      n_channel->appendChild(ntxt_channel);

      DEBUG("Successfully added dataSet-data-channel text");
      
      //Adding calPulse node (dataSet-data subnode)
      XMLString::transcode("CHAN_CHCAL_VAL", tempStr, 99);
      DOMElement* n_calPulse = p_DOMDocument->createElement(tempStr);
      n_data->appendChild(n_calPulse);

      DEBUG("Successfully added dataSet-data-calPulse node");    
    
      //Adding calPulse text
      std::stringstream ss_calPulse;
      ss_calPulse << localParams.channels[i_chan].calPulse;
      string s_calPulse = ss_calPulse.str();
      XMLString::transcode(s_calPulse.c_str(), tempStr, 99);
      DOMText* ntxt_calPulse = p_DOMDocument->createTextNode(tempStr);
      n_calPulse->appendChild(ntxt_calPulse);

      DEBUG("Successfully added dataSet-data-calPulse text");
      
      //Adding mask node (dataSet-data subnode)
      XMLString::transcode("CHAN_MASK_VAL", tempStr, 99);
      DOMElement* n_mask = p_DOMDocument->createElement(tempStr);
      n_data->appendChild(n_mask);

      DEBUG("Successfully added dataSet-data-mask node");    
    
      //Adding mask text
      std::stringstream ss_mask;
      ss_mask << localParams.channels[i_chan].mask;
      string s_mask = ss_mask.str();
      XMLString::transcode(s_mask.c_str(), tempStr, 99);
      DOMText* ntxt_mask = p_DOMDocument->createTextNode(tempStr);
      n_mask->appendChild(ntxt_mask);

      DEBUG("Successfully added dataSet-data-mask text");
      
      //Adding trimDAC node (dataSet-data subnode)
      XMLString::transcode("CHAN_TRIMDAC_VAL", tempStr, 99);
      DOMElement* n_trimDAC = p_DOMDocument->createElement(tempStr);
      n_data->appendChild(n_trimDAC);

      DEBUG("Successfully added dataSet-data-trimDAC node");    
    
      //Adding trimDAC text
      std::string s_trimDAC = std::to_string((long long int) localParams.channels[i_chan].trimDAC);
      XMLString::transcode(s_trimDAC.c_str(), tempStr, 99);
      DOMText* ntxt_trimDAC = p_DOMDocument->createTextNode(tempStr);
      n_trimDAC->appendChild(ntxt_trimDAC);

      DEBUG("Successfully added dataSet-data-trimDAC text");
    }

    // optionally, call release() to release the resource associated with the range after done
    DOMRange* range = p_DOMDocument->createRange();
    range->release();    

    // Write the serialized output to the destination.
    serializer->write(p_DOMDocument, output);

    DEBUG("Successfully wrote domdocument");

    // Cleanup.
    serializer->release();
    xercesc::XMLString::release(&tempFilePath);
    delete formatTarget;
    output->release();

    DEBUG("Done!");
}
