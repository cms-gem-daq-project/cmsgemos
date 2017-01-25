///////////////////////////////////////////////
//
// Parse XML File
//
///////////////////////////////////////////////
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

void gem::utils::vfat::VFAT2ConfigManager::parseXMLFiles()
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
    xercesc::DOMNode* n = pDoc->getFirstChild();
    DEBUG("First child node obtained");
    while (n) {
      DEBUG("Loop on child nodes");
      if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
        DEBUG("Element node found");
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
        if (countChildElementNodes(n)) {
          DEBUG("VFAT global header-type is not empty");
          parseGLheaderType(n);
        }
      }
      if (strcmp("RUN", xercesc::XMLString::transcode(n->getNodeName())) == 0) {
        DEBUG("VFAT Global XML file header parsing: VFAT global header-run found");
        if (countChildElementNodes(n)) {
          DEBUG("VFAT global header-run is not empty");
          parseGLheaderRun(n);
        }
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
        vhead.type.ETN = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("NAME", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("NAME found in Header-Type");
        vhead.type.name = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
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
      if (strcmp("RUN_TYPE", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("RUN_TYPE found in Header-Run");
        vhead.run.runType = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("RUN_NUMBER", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("RUN_NUMBER found in Header-Run");
        vhead.run.runNumber = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("RUN_BEGIN_TIMESTAMP", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("RUN_BEGIN_TIMESTAMP found in Header-Run");
        vhead.run.runBtime = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("RUN_END_TIMESTAMP", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("RUN_END_TIMESTAMP found in Header-Run");
        vhead.run.runEtime = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("COMMENT_DESCRIPTION", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("COMMENT_DESCRIPTION found in Header-Run");
        vhead.run.comment = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("LOCATION", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("LOCATION found in Header-Run");
        vhead.run.location = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("INITIATED_BY_USER", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("INITIATED_BY_USER found in Header-Run");
        vhead.run.user = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
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
        metadata.comment = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("VERSION", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("VERSION found in dataset");
        metadata.version = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("PART", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("VFAT Global XML file header parsing: VFAT global dataset-part found");
        if (countChildElementNodes(n)) 
        {
          DEBUG("VFAT global dataset-part is not empty");
          parseGLdatasetPart(n);
        }
      }
      if (strcmp("DATA", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("VFAT Global XML file header parsing: VFAT global dataset-data found");
        if (countChildElementNodes(n)) 
        {
          DEBUG("VFAT global dataset-part is not empty");
          parseGLdata(n);
        }
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
        metadata.partType = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
      }
      if (strcmp("SERIAL_NUMBER", xercesc::XMLString::transcode(n->getNodeName())) == 0) 
      {
        DEBUG("VERSION found in dataset-part");
        metadata.serialN = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
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
    }
    n = n->getNextSibling();
  }
}

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
