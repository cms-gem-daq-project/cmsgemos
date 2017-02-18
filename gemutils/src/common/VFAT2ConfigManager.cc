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
    this->outputXML(parser->getDocument(), "test.xml");
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
    this->outputXML(parser->getDocument(), "test.xml");
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

void gem::utils::vfat::VFAT2ConfigManager::writeGLXMLFile()
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
    XMLCh* tempFilePath = xercesc::XMLString::transcode("testGLfile.xml");

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

    //Adding the header node
    XMLString::transcode("HEADER", tempStr, 99);
    DOMElement*   n_header = p_DOMDocument->createElement(tempStr);
    root->appendChild(n_header);

    //Adding the data set node
    XMLString::transcode("DATA_SET", tempStr, 99);
    DOMElement*   n_ds = p_DOMDocument->createElement(tempStr);
    root->appendChild(n_ds);

    // optionally, call release() to release the resource associated with the range after done
    DOMRange* range = p_DOMDocument->createRange();
    range->release();    

    //Christine working point - using "Create a small document tree" example from https://xerces.apache.org/xerces-c/program-dom-3.html#Objectives
    
    // Write the serialized output to the destination.
    serializer->write(p_DOMDocument, output);

    DEBUG("Successfully wrote domdocument");

    // Cleanup.
    serializer->release();
    xercesc::XMLString::release(&tempFilePath);
    delete formatTarget;
    output->release();

    DEBUG("Done!");
  /*
  //  Create our writer, then attach an error handler to the writer.
  //  The writer will call back to methods of the ErrorHandler if it
  //  discovers errors during the course of writing the XML document.
  //
  xercesc::XercesDOMWriter* writer = new xercesc::XercesDOMWriter;
  DEBUG("Xerces writer created ");
  writer->setValidationScheme(xercesc::XercesDOMWriter::Val_Auto);
  writer->setDoNamespaces(false);
  writer->setCreateEntityReferenceNodes(false);
  // writer->setCreateEntityReferenceNodes(true);
  // writer->setExpandEntityReferences(true);
  writer->setDoXInclude(true);
  // writer->setToCreateXMLDeclTypeNode(true);
  DEBUG("Xerces writer tuned up ");

  //  Write the XML file, catching any XML exceptions that might propogate
  //  out of it.
  //
  bool errorsOccured = false;
  try {
    writer->write(m_glxmlFile.c_str());
  } catch (const xercesc::XMLException& e) {
    ERROR("An error occured during writing" << std::endl
          << "   Message: "
          << xercesc::XMLString::transcode(e.getMessage()));
    errorsOccured = true;
    // fileError = "An error occured during writing of selected file. Please select another configuration file.";
  } catch (const xercesc::DOMException& e) {
    ERROR("An error occured during writing" << std::endl
          << "   Message: "
          << xercesc::XMLString::transcode(e.msg));
    errorsOccured = true;
    // fileError = "An error occured during writing of selected file. Please select another configuration file.";
  } catch (...) {
    ERROR("An error occured during writing");
    errorsOccured = true;
    // fileError = "An error occured during writing of selected file. Please select another configuration file.";
  }

  // If the write was successful, output the document data from the DOM tree
  // crateIds.clear();
  // crateNodes.clear();

  if (!errorsOccured) {
    DEBUG("DOM tree created succesfully");
    this->outputXML(writer->getDocument(), "test.xml");
    xercesc::DOMNode* pDoc = writer->getDocument();
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
          writeGLheader(n);
        }
        if (strcmp("DATA_SET", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
        {
          DEBUG("VFAT Global Dataset found");
          writeGLdataset(n);
        }
      }
      n = n->getNextSibling();
    }
  }

  DEBUG("Writer pointer " << writer);
  delete writer;
  DEBUG("Xerces writer deleted ");
  xercesc::XMLPlatformUtils::Terminate();
  */
}

void gem::utils::vfat::VFAT2ConfigManager::writeCHXMLFile()
{
  /*  INFO("Writing Channel XML file: " << m_chxmlFile);

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

  //  Create our writer, then attach an error handler to the writer.
  //  The writer will call back to methods of the ErrorHandler if it
  //  discovers errors during the course of writing the XML document.
  //
  xercesc::XercesDOMWriter* writer = new xercesc::XercesDOMWriter;
  DEBUG("Xerces writer created ");
  writer->setValidationScheme(xercesc::XercesDOMWriter::Val_Auto);
  writer->setDoNamespaces(false);
  writer->setCreateEntityReferenceNodes(false);

  // writer->setCreateEntityReferenceNodes(true);
  // writer->setExpandEntityReferences(true);
  writer->setDoXInclude(true);
  // writer->setToCreateXMLDeclTypeNode(true);
  DEBUG("Xerces writer tuned up ");

  //  Write the XML file, catching any XML exceptions that might propogate
  //  out of it.
  //
  bool errorsOccured = false;
  try {
    writer->write(m_chxmlFile.c_str());
  } catch (const xercesc::XMLException& e) {
    ERROR("An error occured during writing" << std::endl
          << "   Message: "
          << xercesc::XMLString::transcode(e.getMessage()));
    errorsOccured = true;
    // fileError = "An error occured during writing of selected file. Please select another configuration file.";
  } catch (const xercesc::DOMException& e) {
    ERROR("An error occured during writing" << std::endl
          << "   Message: "
          << xercesc::XMLString::transcode(e.msg));
    errorsOccured = true;
    // fileError = "An error occured during writing of selected file. Please select another configuration file.";
  } catch (...) {
    ERROR("An error occured during writing");
    errorsOccured = true;
    // fileError = "An error occured during writing of selected file. Please select another configuration file.";
  }

  // If the write was successful, output the document data from the DOM tree
  // crateIds.clear();
  // crateNodes.clear();

  if (!errorsOccured) {
    DEBUG("DOM tree created succesfully");
    this->outputXML(writer->getDocument(), "test.xml");
    xercesc::DOMNode* pDoc = writer->getDocument();
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
          writeCHheader(n);
        }
        if (strcmp("DATA_SET", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
        {
          DEBUG("VFAT Channel Dataset found");
          writeCHdataset(n);
        }
      }
      n = n->getNextSibling();
    }
  }

  DEBUG("Writer pointer " << writer);
  delete writer;
  DEBUG("Xerces writer deleted ");
  xercesc::XMLPlatformUtils::Terminate();
  */
}

/*void gem::utils::vfat::VFAT2ConfigManager::writeGLheader(xercesc::DOMNode* pNode)
{
  INFO("writeGLheader");
  DEBUG("VFAT Global XML file header writing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Global XML file header writing: get first child");
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      if (strcmp("TYPE", xercesc::XMLString::transcode(n->getNodeName())) == 0) {
        DEBUG("VFAT Global XML file header writing: VFAT global header-type found");
        writeGLheaderType(n);
      }
      if (strcmp("RUN", xercesc::XMLString::transcode(n->getNodeName())) == 0) {
        DEBUG("VFAT Global XML file header writing: VFAT global header-run found");
        writeGLheaderRun(n);
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::vfat::VFAT2ConfigManager::writeGLheaderType(xercesc::DOMNode* pNode)
{
  INFO("writeGLheaderType");
  DEBUG("VFAT Global XML file header-type writing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Global XML file header-type writing: get first child");
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

void gem::utils::vfat::VFAT2ConfigManager::writeGLheaderRun(xercesc::DOMNode* pNode)
{
  INFO("writeGLheaderRun");
  DEBUG("VFAT Global XML file header-run writing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Global XML file header-run writing: get first child");
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

void gem::utils::vfat::VFAT2ConfigManager::writeGLdataset(xercesc::DOMNode* pNode)
{
  INFO("writeGLdataset");
  DEBUG("VFAT Global XML file dataset writing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Global XML file dataset writing: get first child");
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
        DEBUG("VFAT Global XML file header writing: VFAT global dataset-part found");
        writeGLdatasetPart(n);
      }
      if (strcmp("DATA", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("VFAT Global XML file header writing: VFAT global dataset-data found");
        writeGLdata(n);
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::vfat::VFAT2ConfigManager::writeGLdatasetPart(xercesc::DOMNode* pNode)
{
  INFO("writeGLdatasetPart");
  DEBUG("VFAT Global XML file dataset-part writing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Global XML file dataset-part writing: get first child");
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

void gem::utils::vfat::VFAT2ConfigManager::writeGLdata(xercesc::DOMNode* pNode)
{
  INFO("writeGLdata");
  DEBUG("VFAT Global XML file dataset-data writing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Global XML file dataset-data writing: get first child");
  while (n) 
  {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) 
    {
      if (strcmp("CR0_CALMODE", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("CR0_CALMODE found in dataset-data");
        std::string strBuf = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
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

void gem::utils::vfat::VFAT2ConfigManager::writeCHheader(xercesc::DOMNode* pNode)
{
  INFO("writeCHheader");
  DEBUG("VFAT Channel XML file header writing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Channel XML file header writing: get first child");
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      if (strcmp("TYPE", xercesc::XMLString::transcode(n->getNodeName())) == 0) {
        DEBUG("VFAT Channel XML file header writing: VFAT channel header-type found");
        writeCHheaderType(n);
      }
      if (strcmp("RUN", xercesc::XMLString::transcode(n->getNodeName())) == 0) {
        DEBUG("VFAT Channel XML file header writing: VFAT channel header-run found");
        writeCHheaderRun(n);
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::vfat::VFAT2ConfigManager::writeCHheaderType(xercesc::DOMNode* pNode)
{
  INFO("writeCHheaderType");
  DEBUG("VFAT Channel XML file header-type writing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Channel XML file header-type writing: get first child");
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

void gem::utils::vfat::VFAT2ConfigManager::writeCHheaderRun(xercesc::DOMNode* pNode)
{
  INFO("writeCHheaderRun");
  DEBUG("VFAT Channel XML file header-run writing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Channel XML file header-run writing: get first child");
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

void gem::utils::vfat::VFAT2ConfigManager::writeCHdataset(xercesc::DOMNode* pNode)
{
  INFO("writeCHdataset");
  DEBUG("VFAT Channel XML file dataset writing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Channel XML file dataset writing: get first child");
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
        DEBUG("VFAT Channel XML file header writing: VFAT channel dataset-part found");
        writeCHdatasetPart(n);
      }
      if (strcmp("DATA", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("VFAT Channel XML file header writing: VFAT channel dataset-data found");
        writeCHdata(n);
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::vfat::VFAT2ConfigManager::writeCHdatasetPart(xercesc::DOMNode* pNode)
{
  INFO("writeCHdatasetPart");
  DEBUG("VFAT Channel XML file dataset-part writing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Channel XML file dataset-part writing: get first child");
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

void gem::utils::vfat::VFAT2ConfigManager::writeCHdata(xercesc::DOMNode* pNode)
{
  INFO("writeCHdata");
  DEBUG("VFAT Channel XML file dataset-data writing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("VFAT Channel XML file dataset-data writing: get first child");
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
*/





void gem::utils::vfat::VFAT2ConfigManager::outputXML(xercesc::DOMDocument* pmyDOMDocument, std::string filePath)
{
    // Return the first registered implementation that has the desired features. In this case, we are after a DOM implementation that has the LS feature... or Load/Save.
    xercesc::DOMImplementation* implementation = xercesc::DOMImplementationRegistry::getDOMImplementation(xercesc::XMLString::transcode("LS"));

    // Create a DOMLSSerializer which is used to serialize a DOM tree into an XML document.
    xercesc::DOMLSSerializer* serializer = ((xercesc::DOMImplementationLS*)implementation)->createLSSerializer();

    // Make the output more human readable by inserting line feeds.
    if (serializer->getDomConfig()->canSetParameter(xercesc::XMLUni::fgDOMWRTFormatPrettyPrint, true))
        serializer->getDomConfig()->setParameter(xercesc::XMLUni::fgDOMWRTFormatPrettyPrint, true);

    // The end-of-line sequence of characters to be used in the XML being written out.
    serializer->setNewLine(xercesc::XMLString::transcode("\n"));

    // Convert the path into Xerces compatible XMLCh*.
    // xercesc::XMLCh* tempFilePath = xercesc::XMLString::transcode(filePath.c_str());
    XMLCh* tempFilePath = xercesc::XMLString::transcode(filePath.c_str());

    // Specify the target for the XML output.
    xercesc::XMLFormatTarget* formatTarget = new xercesc::LocalFileFormatTarget(tempFilePath);

    // Create a new empty output destination object.
    xercesc::DOMLSOutput* output = ((xercesc::DOMImplementationLS*)implementation)->createLSOutput();

    // Set the stream to our target.
    output->setByteStream(formatTarget);

    // Write the serialized output to the destination.
    serializer->write(pmyDOMDocument, output);

    // Cleanup.
    serializer->release();
    xercesc::XMLString::release(&tempFilePath);
    delete formatTarget;
    output->release();
}
