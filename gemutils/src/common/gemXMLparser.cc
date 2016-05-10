///////////////////////////////////////////////
//
// Parse XML File
//
///////////////////////////////////////////////
#include <gem/utils/gemXMLparser.h>

gem::utils::gemXMLparser::gemXMLparser(const std::string& xmlFile) :
  m_xmlFile(xmlFile),
  p_gemSystem(new gemSystemProperties()),
  m_gemLogger(log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("gem:utils:GEMXMLParser")))
{
  // op_gemSystem = new gemSystemProperties();
  p_gemSystem->setDeviceId("GEM");
  m_gemLogger.setLogLevel(log4cplus::DEBUG_LOG_LEVEL);
}

gem::utils::gemXMLparser::~gemXMLparser()
{
  delete p_gemSystem;
}

void gem::utils::gemXMLparser::parseXMLFile()
{
  INFO("Parsing XML file: " << m_xmlFile);

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
    parser->parse(m_xmlFile.c_str());
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
        if (strcmp("GEMSystem", xercesc::XMLString::transcode(n->getNodeName())) == 0) {
          DEBUG("GEM system found");
          parseGEMSystem(n);
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

void gem::utils::gemXMLparser::parseGEMSystem(xercesc::DOMNode* pNode)
{
  INFO("parseGEMSystem");
  DEBUG("GEM system parsing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("GEM system parsing: get first child");
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      if (strcmp("uTCACrate", xercesc::XMLString::transcode(n->getNodeName())) == 0) {
        DEBUG("GEM system parsing: uTCA crate found");
        if (countChildElementNodes(n)) {
          DEBUG("GEM system parsing: uTCA crate is not empty");
          gemCrateProperties* crate = new gemCrateProperties();
          DEBUG("GEM system parsing: new crate properties object created");
          crate->setDeviceId(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("CrateId"))->getNodeValue()));
          p_gemSystem->addSubDeviceRef(crate);
          p_gemSystem->addSubDeviceId(crate->getDeviceId());
          DEBUG("GEM system parsing: new crate properties object added to crateRefs");
          parseCrate(n);
        }
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::gemXMLparser::parseCrate(xercesc::DOMNode* pNode)
{
  INFO("parseCrate");
  DEBUG("GEM system parsing: starting parseCrate");
  xercesc::DOMNode* n = pNode->getFirstChild();
  DEBUG("crate parsing: look for children");
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      if (strcmp("MCH", xercesc::XMLString::transcode(n->getNodeName())) == 0) {
        INFO("parseMCH");
      }
      if (strcmp("AMC", xercesc::XMLString::transcode(n->getNodeName())) == 0) {
        INFO("parseAMC");
      }
      if (strcmp("GLIB", xercesc::XMLString::transcode(n->getNodeName())) == 0) {
        INFO("parseGLIB");
        DEBUG("crate parsing: GLIB found");
        if (countChildElementNodes(n)) {
          DEBUG("crate parsing: GLIB is not empty");
          gemGLIBProperties* glib = new gemGLIBProperties();
          DEBUG("crate parsing: create new GLIBproperties object");
          glib->setDeviceId(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("GLIBId"))->getNodeValue()));
          p_gemSystem->getSubDevicesRefs().back()->addSubDeviceRef(glib);
          p_gemSystem->getSubDevicesRefs().back()->addSubDeviceId(glib->getDeviceId());
          DEBUG("crate parsing: Add new GLIBproperties to the subdevices of parent crate");
          parseGLIB(n);
        }
      }
    }
    n = n->getNextSibling();
  }
}


void gem::utils::gemXMLparser::parseGLIB(xercesc::DOMNode* pNode)
{
  DEBUG("crate parsing: start GLIB parsing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  gemGLIBProperties* glib_ = p_gemSystem->getSubDevicesRefs().back()->getSubDevicesRefs().back();
  DEBUG("GLIB parsing: retrieve GLIB device from the devices parent tree");
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      addProperty("Station",   n, glib_);
      addProperty("FW",        n, glib_);
      addProperty("IP",        n, glib_);
      addProperty("DEPTH",     n, glib_);
      addProperty("TDC_SBits", n, glib_);
      if (strcmp("OH", xercesc::XMLString::transcode(n->getNodeName())) == 0) {
        DEBUG("GLIB parsing: OH found");
        if (countChildElementNodes(n)) {
          gemOHProperties* oh = new gemOHProperties();
          DEBUG("GLIB parsing: create new OHproperties obect");
          oh->setDeviceId(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("OHId"))->getNodeValue()));
          p_gemSystem->getSubDevicesRefs().back()->getSubDevicesRefs().back()->addSubDeviceRef(oh);
          p_gemSystem->getSubDevicesRefs().back()->getSubDevicesRefs().back()->addSubDeviceId(oh->getDeviceId());
          DEBUG("GLIB parsing: Add new OHproperties to the subdevices of parent device");
          parseOH(n);
        }
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::gemXMLparser::parseOH(xercesc::DOMNode* pNode)
{
  DEBUG("GLIB parsing: start OH parsing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  gemOHProperties* oh_ = p_gemSystem->getSubDevicesRefs().back()->getSubDevicesRefs().back()->getSubDevicesRefs().back();
  DEBUG("OH parsing: retrieve OH device from the devices parent tree");
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      addProperty("TrigSource",   n, oh_);
      addProperty("TDC_SBits",    n, oh_);
      addProperty("VFATClock",    n, oh_);
      addProperty("VFATFallback", n, oh_);
      addProperty("CDCEClock",    n, oh_);
      addProperty("CDCEFallback", n, oh_);
      addProperty("FPGAPLLLock",  n, oh_);
      addProperty("CDCELock",     n, oh_);
      addProperty("GTPLock",      n, oh_);
      addProperty("FW",           n, oh_);
      if (strcmp("VFATSettings", xercesc::XMLString::transcode(n->getNodeName())) == 0) {
        DEBUG("OH parsing: VFATSettings tag found");
        if (countChildElementNodes(n)) {
          gemVFATProperties* vfat = new gemVFATProperties();
          DEBUG("OH parsing: create new VFATproperties object");
          vfat->setDeviceId(xercesc::XMLString::transcode(n->getAttributes()->getNamedItem(xercesc::XMLString::transcode("VFATId"))->getNodeValue()));
          DEBUG("OH parsing: retrieve VFAT device ID");
          p_gemSystem->getSubDevicesRefs().back()->getSubDevicesRefs().back()->getSubDevicesRefs().back()->addSubDeviceRef(vfat);
          DEBUG("OH parsing: add new VFATproperties to the subdevices of the parent device");
          p_gemSystem->getSubDevicesRefs().back()->getSubDevicesRefs().back()->getSubDevicesRefs().back()->addSubDeviceId(vfat->getDeviceId());
          DEBUG("OH parsing: add VFAT device ID to the subdevices of the parent device");
          parseVFAT2Settings(n);
        }
      }
    }
    n = n->getNextSibling();
  }
}

void gem::utils::gemXMLparser::parseVFAT2Settings(xercesc::DOMNode* pNode)
{
  DEBUG("OH parsing: start VFAT parsing");
  xercesc::DOMNode* n = pNode->getFirstChild();
  gemVFATProperties* vfat_ = p_gemSystem->getSubDevicesRefs().back()->getSubDevicesRefs().back()->getSubDevicesRefs().back()->getSubDevicesRefs().back();
  DEBUG("VFAT parsing: retrieve VFAT device from the devices parent tree");
  while (n) {
    if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) {
      addProperty("CalMode",       n, vfat_);
      addProperty("CalPolarity",   n, vfat_);
      addProperty("MSPolarity",    n, vfat_);
      addProperty("TriggerMode",   n, vfat_);
      addProperty("RunMode",       n, vfat_);
      addProperty("ReHitCT",       n, vfat_);
      addProperty("LVDSPowerSave", n, vfat_);
      addProperty("ProbeMode",     n, vfat_);
      addProperty("DACMode",       n, vfat_);
      addProperty("DigInSel",      n, vfat_);
      addProperty("MSPulseLength", n, vfat_);
      addProperty("HitCountMode",  n, vfat_);
      addProperty("DFTest",        n, vfat_);
      addProperty("PbBG",          n, vfat_);
      addProperty("TrimDACRange",  n, vfat_);
      addProperty("IPreampIn",     n, vfat_);
      addProperty("IPreampFeed",   n, vfat_);
      addProperty("IPreampOut",    n, vfat_);
      addProperty("IShaper",       n, vfat_);
      addProperty("IShaperFeed",   n, vfat_);
      addProperty("IComp",         n, vfat_);
      addProperty("Latency",       n, vfat_);
      addProperty("VCal",          n, vfat_);
      addProperty("VThreshold1",   n, vfat_);
      addProperty("VThreshold2",   n, vfat_);
      addProperty("CalPhase",      n, vfat_);
    }
    n = n->getNextSibling();
  }
}

void gem::utils::gemXMLparser::addProperty(const char* key, const xercesc::DOMNode* n, gemGLIBProperties* glib)
{
  if (strcmp(key, xercesc::XMLString::transcode(n->getNodeName())) == 0) {
    std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
    glib->addDeviceProperty(key, value);
  }
}

void gem::utils::gemXMLparser::addProperty(const char* key, const xercesc::DOMNode* n, gemOHProperties* oh)
{
  if (strcmp(key, xercesc::XMLString::transcode(n->getNodeName())) == 0) {
    std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
    oh->addDeviceProperty(key, value);
  }
}

void gem::utils::gemXMLparser::addProperty(const char* key, const xercesc::DOMNode* n, gemVFATProperties* vfat)
{
  if (strcmp(key, xercesc::XMLString::transcode(n->getNodeName())) == 0) {
    std::string value = (std::string)xercesc::XMLString::transcode(n->getFirstChild()->getNodeValue());
    vfat->addDeviceProperty(key, value);
  }
}

int gem::utils::gemXMLparser::countChildElementNodes(xercesc::DOMNode* pNode) {
  int count = 0;
  if (pNode->hasChildNodes()) {
    xercesc::DOMNode* n = pNode->getFirstChild();
    while (n) {
      if (n->getNodeType() == xercesc::DOMNode::ELEMENT_NODE) count++;
      n = n->getNextSibling();
    }
  }
  return count;
}
void gem::utils::gemXMLparser::outputXML(xercesc::DOMDocument* pmyDOMDocument, std::string filePath)
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
