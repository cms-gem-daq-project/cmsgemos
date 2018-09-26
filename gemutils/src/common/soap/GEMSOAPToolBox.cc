#include <gem/utils/soap/GEMSOAPToolBox.h>

xoap::MessageReference gem::utils::soap::GEMSOAPToolBox::makeSOAPReply(std::string const& command,
                                                                       std::string const& response)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMSOAPToolBoxLogger"));
  xoap::MessageReference reply        = xoap::createMessage();
  xoap::SOAPEnvelope     envelope     = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName         responseName = envelope.createName(command, "xdaq", XDAQ_NS_URI);
  xoap::SOAPElement      bodyElement  = envelope.getBody().addBodyElement(responseName);
  bodyElement.addTextNode(response);
  return reply;
}

xoap::MessageReference gem::utils::soap::GEMSOAPToolBox::makeSOAPFaultReply(std::string const& faultString,
                                                                            std::string const& faultCode,
                                                                            std::string const& detail,
                                                                            std::string const& faultActor)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMSOAPToolBoxLogger"));
  xoap::MessageReference reply       = xoap::createMessage();
  xoap::SOAPEnvelope     envelope    = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName         faultName   = envelope.createName("Fault", "xdaq", XDAQ_NS_URI);
  xoap::SOAPElement      bodyElement = envelope.getBody().addBodyElement(faultName);

  xoap::SOAPName    faultStringName    = envelope.createName("faultstring", "xdaq", XDAQ_NS_URI);
  xoap::SOAPElement faultStringElement = bodyElement.addChildElement(faultStringName);
  faultStringElement.addTextNode(faultString);

  xoap::SOAPName    faultCodeName    = envelope.createName("faultcode", "xdaq", XDAQ_NS_URI);
  xoap::SOAPElement faultCodeElement = bodyElement.addChildElement(faultCodeName);
  faultCodeElement.addTextNode(faultCode);

  if (detail.size() > 0) {
    xoap::SOAPName    detailName    = envelope.createName("detail", "xdaq", XDAQ_NS_URI);
    xoap::SOAPElement detailElement = bodyElement.addChildElement(detailName);
    detailElement.addTextNode(detail);
  }

  if (faultActor.size() > 0) {
    xoap::SOAPName faultActorName = envelope.createName("faultactor", "xdaq", XDAQ_NS_URI);
    xoap::SOAPElement faultActorElement = bodyElement.addChildElement(faultActorName);
    faultActorElement.addTextNode(faultActor);
  }

  return reply;
}

xoap::MessageReference gem::utils::soap::GEMSOAPToolBox::makeFSMSOAPReply(std::string const& event,
                                                                          std::string const& state)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMSOAPToolBoxLogger"));
  // xoap::MessageFactory* messageFactory = xoap::MessageFactory::getInstance(soapProtocolVersion);
  xoap::MessageReference reply           = xoap::createMessage();
  xoap::SOAPEnvelope     envelope        = reply->getSOAPPart().getEnvelope();
  xoap::SOAPBody         body            = envelope.getBody();
  std::string            responseString  = event + "Response";
  TRACE("GEMSOAPToolBox::makeFSMSOAPReply responseString "
            << responseString);
  xoap::SOAPName         responseName    = envelope.createName(responseString, "xdaq", XDAQ_NS_URI);
  TRACE("GEMSOAPToolBox::makeFSMSOAPReply responseName "
            << responseName.getLocalName());
  xoap::SOAPBodyElement  responseElement = body.addBodyElement(responseName);
  TRACE("GEMSOAPToolBox::makeFSMSOAPReply responseElement "
            << responseElement.getTextContent());
  xoap::SOAPName         stateName       = envelope.createName("state", "xdaq", XDAQ_NS_URI);
  TRACE("GEMSOAPToolBox::makeFSMSOAPReply stateName "
            << stateName.getLocalName());
  xoap::SOAPElement      stateElement    = responseElement.addChildElement(stateName);
  TRACE("GEMSOAPToolBox::makeFSMSOAPReply stateElement"
            << stateElement.getTextContent());
  xoap::SOAPName         attributeName   = envelope.createName("stateName", "xdaq", XDAQ_NS_URI);
  TRACE("GEMSOAPToolBox::makeFSMSOAPReplyattributeName "
            << attributeName.getLocalName());
  stateElement.addAttribute(attributeName, state);
  DEBUG("GEMSOAPToolBox::makeFSMSOAPReply reply ");
  std::string tool;
  reply->writeTo(tool);
  DEBUG(tool);
  return reply;
}


std::string gem::utils::soap::GEMSOAPToolBox::extractFSMCommandName(xoap::MessageReference const& msg)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMSOAPToolBoxLogger"));
  xoap::SOAPPart     part = msg->getSOAPPart();
  xoap::SOAPEnvelope env  = part.getEnvelope();
  xoap::SOAPBody     body = env.getBody();

  DOMNode*     node     = body.getDOMNode();
  DOMNodeList* bodyList = node->getChildNodes();

  // The body should contain a single node with the name of the FSM
  // command to execute.
  if (bodyList->getLength() != 1) {
    XCEPT_RAISE(xoap::exception::Exception,
                toolbox::toString("Expected exactly one element "
                                  "in GEMFSM command SOAP message, "
                                  "but found %d.", bodyList->getLength()));
  }
  return xoap::XMLCh2String((bodyList->item(0))->getLocalName());
}

bool gem::utils::soap::GEMSOAPToolBox::sendCommand(std::string const& cmd,
                                                   xdaq::ApplicationContext* appCxt,
                                                   xdaq::ApplicationDescriptor* srcDsc,
                                                   xdaq::ApplicationDescriptor* destDsc
                                                   // log4cplus::Logger* logger,
                                                   // std::string const& param
                                                   )
  throw (gem::utils::exception::SOAPException)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMSOAPToolBoxLogger"));
  try {
    xoap::MessageReference msg = xoap::createMessage();

    xoap::SOAPEnvelope env = msg->getSOAPPart().getEnvelope();
    xoap::SOAPName soapcmd = env.createName(cmd, "xdaq", XDAQ_NS_URI);
    xoap::SOAPElement cont = env.getBody().addBodyElement(soapcmd);

    DEBUG("GEMSOAPToolBox::sendCommand '" << cmd << "'"
          << " in '"   << appCxt->getContextDescriptor()->getURL() << "'"
          << " from '" << srcDsc->getClassName() << "'"
          << " to '"   << destDsc->getClassName() << "'");

    if (destDsc->getClassName().find("tcds") != std::string::npos) {
      xoap::SOAPName cmdtype = env.createName("actionRequestorId", "xdaq", srcDsc->getClassName());
      std::stringstream tcdscmd;
      tcdscmd << cmd << " xdaq:actionRequestorId=\""
              << srcDsc->getClassName() << "\"";
      DEBUG("GEMSOAPToolBox::sendTCDSCommand '" << tcdscmd.str() << "'" << " to '" << destDsc->getClassName() << "'");
      cont.addAttribute(cmdtype,srcDsc->getClassName());
    }

    std::string tool;
    // xoap::dumpTree(msg->getSOAPPart().getEnvelope().getDOMNode(),tool);
    msg->writeTo(tool);
    DEBUG("GEMSOAPToolBox::sendCommand '" << cmd << "': SOAP msg " << tool);
    // BUG FIXME: if this throws, we get a terminate, why???
    tool.clear();

    xoap::MessageReference reply = appCxt->postSOAP(msg, *srcDsc, *destDsc);
    // xoap::dumpTree(reply->getSOAPPart().getEnvelope().getDOMNode(),tool);
    reply->writeTo(tool);
    DEBUG("GEMSOAPToolBox::sendCommand '" << cmd << "': SOAP reply " << tool);
  } catch (xdaq::exception::Exception& e) {
    std::string errMsg = toolbox::toString("Command %s failed [%s]", cmd.c_str(), e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (xcept::Exception& e) {
    std::string errMsg = toolbox::toString("Command %s failed [%s]", cmd.c_str(), e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (std::exception& e) {
    std::string errMsg = toolbox::toString("Command %s failed [%s]", cmd.c_str(), e.what());
    XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
  } catch (...) {
    std::string errMsg = toolbox::toString("Command %s failed",      cmd.c_str());
    XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
  }
  return true;
}

bool gem::utils::soap::GEMSOAPToolBox::sendTCDSCommand(std::string const& cmd,
                                                       xdaq::ApplicationContext* appCxt,
                                                       xdaq::ApplicationDescriptor* srcDsc,
                                                       xdaq::ApplicationDescriptor* destDsc
                                                       // log4cplus::Logger* logger,
                                                       // std::string const& param
                                                       )
  throw (gem::utils::exception::SOAPException)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMSOAPToolBoxLogger"));
  try {
    xoap::MessageReference msg = xoap::createMessage();

    xoap::SOAPEnvelope env = msg->getSOAPPart().getEnvelope();
    xoap::SOAPName soapcmd = env.createName(cmd, "xdaq", XDAQ_NS_URI);
    xoap::SOAPElement cont = env.getBody().addBodyElement(soapcmd);

    DEBUG("GEMSOAPToolBox::sendTCDSCommand '" << cmd << "'"
          << " in '"   << appCxt->getContextDescriptor()->getURL() << "'"
          << " from '" << srcDsc->getClassName() << "'"
          << " to '"   << destDsc->getClassName() << "'");

    if (destDsc->getClassName().find("tcds") != std::string::npos) {
      xoap::SOAPName cmdtype = env.createName("actionRequestorId", "xdaq", srcDsc->getClassName());
      std::stringstream tcdscmd;
      tcdscmd << cmd << " xdaq:actionRequestorId=\""
              << srcDsc->getClassName() << "\"";
      DEBUG("GEMSOAPToolBox::sendTCDSCommand '" << tcdscmd.str() << "'" << " to '" << destDsc->getClassName() << "'");
      cont.addAttribute(cmdtype,"");
    }

    std::string tool;
    // xoap::dumpTree(msg->getSOAPPart().getEnvelope().getDOMNode(),tool);
    msg->writeTo(tool);
    INFO("GEMSOAPToolBox::sendTCDSCommand '" << cmd << "': SOAP msg " << tool);
    tool.clear();

    xoap::MessageReference reply = appCxt->postSOAP(msg, *srcDsc, *destDsc);
    // xoap::dumpTree(reply->getSOAPPart().getEnvelope().getDOMNode(),tool);
    reply->writeTo(tool);
    INFO("GEMSOAPToolBox::sendTCDSCommand '" << cmd << "': SOAP reply " << tool);
  } catch (xdaq::exception::Exception& e) {
    std::string errMsg = toolbox::toString("Command %s failed [%s]", cmd.c_str(), e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (xcept::Exception& e) {
    std::string errMsg = toolbox::toString("Command %s failed [%s]", cmd.c_str(), e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (std::exception& e) {
    std::string errMsg = toolbox::toString("Command %s failed [%s]", cmd.c_str(), e.what());
    XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
  } catch (...) {
    std::string errMsg = toolbox::toString("Command %s failed",      cmd.c_str());
    XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
  }
  return true;
}

bool gem::utils::soap::GEMSOAPToolBox::sendParameter(std::vector<std::string> const& parameter,
                                                     xdaq::ApplicationContext* appCxt,
                                                     xdaq::ApplicationDescriptor* srcDsc,
                                                     xdaq::ApplicationDescriptor* destDsc
                                                     // log4cplus::Logger* logger
                                                     )
  throw (gem::utils::exception::SOAPException)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMSOAPToolBoxLogger"));
  if (parameter.size() != 3)
    return false;

  try {
    xoap::MessageReference msg = xoap::createMessage();

    xoap::SOAPEnvelope env       = msg->getSOAPPart().getEnvelope();
    xoap::SOAPName     soapcmd   = env.createName("ParameterSet", "xdaq", XDAQ_NS_URI);
    xoap::SOAPElement  container = env.getBody().addBodyElement(soapcmd);

    // from hcal supervisor
    env.addNamespaceDeclaration("xsd", "http://www.w3.org/2001/XMLSchema");
    env.addNamespaceDeclaration("xsi", "http://www.w3.org/2001/XMLSchema-instance");
    env.addNamespaceDeclaration("soapenc", "http://schemas.xmlsoap.org/soap/encoding/");
    xoap::SOAPName    type       = env.createName("type", "xsi", "http://www.w3.org/2001/XMLSchema-instance");
    std::string       appURN     = "urn:xdaq-application:"+destDsc->getClassName();
    xoap::SOAPName    properties = env.createName("properties", "props", appURN);
    xoap::SOAPElement property   = container.addChildElement(properties);
    property.addAttribute(type, "soapenc:Struct");
    xoap::SOAPName    cfgStyleName = env.createName(parameter.at(0), "props", appURN);
    xoap::SOAPElement cs           = property.addChildElement(cfgStyleName);
    cs.addAttribute(type, parameter.at(2));
    cs.addTextNode(parameter.at(1));
    // end from hcal supervisor

    std::string tool;
    // xoap::dumpTree(msg->getSOAPPart().getEnvelope().getDOMNode(),tool);
    msg->writeTo(tool);
    INFO("GEMSOAPToolBox::sendParameter SOAP msg " << tool);
    tool.clear();

    xoap::MessageReference reply = appCxt->postSOAP(msg, *srcDsc, *destDsc);
    // xoap::dumpTree(reply->getSOAPPart().getEnvelope().getDOMNode(),tool);
    reply->writeTo(tool);
    INFO("GEMSOAPToolBox::sendParameter SOAP msg " << tool);
  } catch (xdaq::exception::Exception& e) {
    std::string errMsg = toolbox::toString("Send Parameter %s failed [%s]", parameter.at(0).c_str(), e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (xcept::Exception& e) {
    std::string errMsg = toolbox::toString("Send Parameter %s failed [%s]", parameter.at(0).c_str(), e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (std::exception& e) {
    std::string errMsg = toolbox::toString("Send Parameter %s failed [%s]", parameter.at(0).c_str(), e.what());
    XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
  } catch (...) {
    std::string errMsg = toolbox::toString("Send Parameter %s failed",      parameter.at(0).c_str());
    XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
  }
  return true;
}

std::pair<std::string, std::string> gem::utils::soap::GEMSOAPToolBox::extractCommandWithParameter(xoap::MessageReference const& msg)
{
  xoap::SOAPPart     part = msg->getSOAPPart();
  xoap::SOAPEnvelope env  = part.getEnvelope();
  xoap::SOAPBody     body = env.getBody();

  DOMNode*     node     = body.getDOMNode();
  DOMNodeList* bodyList = node->getChildNodes();

  // The body should contain a single node with the name of the
  // command to execute and the command should have a child text node with the parameter.
  if (bodyList->getLength() != 1) {
    XCEPT_RAISE(xoap::exception::Exception,
                toolbox::toString("Expected exactly one element "
                                  "in CommandWithParameter SOAP message, "
                                  "but found %d.", bodyList->getLength()));
  }
  std::string commandName    = xoap::XMLCh2String((bodyList->item(0))->getLocalName());
  std::string parameterValue = xoap::XMLCh2String((bodyList->item(0))->getNodeValue());
  return std::make_pair(commandName, parameterValue);
}

bool gem::utils::soap::GEMSOAPToolBox::sendCommandWithParameter(std::string const& cmd, int const& parameter,
                                                                xdaq::ApplicationContext* appCxt,
                                                                xdaq::ApplicationDescriptor* srcDsc,
                                                                xdaq::ApplicationDescriptor* destDsc
                                                                )
  throw (gem::utils::exception::SOAPException)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMSOAPToolBoxLogger"));
  try {
    xoap::MessageReference msg = xoap::createMessage();

    xoap::SOAPEnvelope env = msg->getSOAPPart().getEnvelope();
    xoap::SOAPName soapcmd = env.createName(cmd, "xdaq", XDAQ_NS_URI);
    xoap::SOAPBodyElement paramValue = env.getBody().addBodyElement(soapcmd);
    paramValue.addTextNode(toolbox::toString("%d", parameter));

    std::string tool;
    // xoap::dumpTree(msg->getSOAPPart().getEnvelope().getDOMNode(),tool);
    msg->writeTo(tool);
    INFO("GEMSOAPToolBox::sendCommandWithParameter SOAP msg " << tool);
    tool.clear();

    xoap::MessageReference reply = appCxt->postSOAP(msg, *srcDsc, *destDsc);
    // xoap::dumpTree(reply->getSOAPPart().getEnvelope().getDOMNode(),tool);
    reply->writeTo(tool);
    INFO("GEMSOAPToolBox::sendCommandWithParameter SOAP msg " << tool);
  } catch (xdaq::exception::Exception& e) {
    std::string errMsg = toolbox::toString("Sending parameter %s (value %d) failed [%s]", cmd.c_str(), parameter, e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException,errMsg, e);
  } catch (xcept::Exception& e) {
    std::string errMsg = toolbox::toString("Sending parameter %s (value %d) failed [%s]", cmd.c_str(), parameter, e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (std::exception& e) {
    std::string errMsg = toolbox::toString("Sending parameter %s (value %d) failed [%s]", cmd.c_str(), parameter, e.what());
    XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
  } catch (...) {
    std::string errMsg = toolbox::toString("Sending parameter %s (value %d) failed",      cmd.c_str(), parameter);
    XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
  }
  return true;
}

bool gem::utils::soap::GEMSOAPToolBox::sendCommandWithParameterBag(std::string const& cmd,
                                                                   // xdata::Bag<T> const& bag,
                                                                   std::unordered_map<std::string, xdata::Serializable*> const& bag,
                                                                   xdaq::ApplicationContext* appCxt,
                                                                   xdaq::ApplicationDescriptor* srcDsc,
                                                                   xdaq::ApplicationDescriptor* destDsc
                                                                   )
  throw (gem::utils::exception::SOAPException) {
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMSOAPToolBoxLogger"));
  try {
    xoap::MessageReference msg = xoap::createMessage(), reply;

    xoap::SOAPEnvelope env = msg->getSOAPPart().getEnvelope();
    env.addNamespaceDeclaration("xsd",     "http://www.w3.org/2001/XMLSchema");
    env.addNamespaceDeclaration("xsi",     "http://www.w3.org/2001/XMLSchema-instance");
    env.addNamespaceDeclaration("soapenc", "http://schemas.xmlsoap.org/soap/encoding/");

    xoap::SOAPName     soapcmd   = env.createName(cmd, "xdaq", XDAQ_NS_URI);
    xoap::SOAPElement  container = env.getBody().addBodyElement(soapcmd);
    xoap::SOAPName     tname     = env.createName("type", "xsi", "http://www.w3.org/2001/XMLSchema-instance");

    if (destDsc->getClassName().find("tcds") != std::string::npos) {
      xoap::SOAPName cmdtype = env.createName("actionRequestorId", "xdaq", srcDsc->getClassName());
      container.addAttribute(cmdtype,srcDsc->getClassName());

      DEBUG("GEMSOAPToolBox::sendCommandWithParameterBag: '" << cmd << "' adding attribute " << cmdtype.getQualifiedName());
    }

    for (auto b = bag.begin(); b != bag.end(); ++b) {
      // xdata::Serializable* s = bag.getField((*b).first);
      xdata::Serializable* s        = b->second;
      xoap::SOAPName       soapName = env.createName(b->first, "xdaq", XDAQ_NS_URI);
      xoap::SOAPElement    cs       = container.addChildElement(soapName);
      cs.addAttribute(tname, getXSDType(*s));
      cs.addTextNode(s->toString());
    }
    // std::vector<xoap::SOAPElement> paramsFound = container.getChildElements();
    DEBUG("GEMSOAPToolBox::sendCommandWithParameterBag Inspecting command elements with qualified name "
          << container.getElementName().getQualifiedName() << " with memory address "
          << std::hex << &container << std::dec << " has value "
          << container.getValue() << " and has "
          << container.getChildElements().size() << " children"
          );
    std::vector<xoap::SOAPElement> elems = container.getChildElements();
    // for (auto elem = elems.begin(); elem != elems.end(); ++elem) {
    for (std::vector<xoap::SOAPElement>::iterator elem = elems.begin(); elem != elems.end(); ++elem) {
      DEBUG("GEMSOAPToolBox::sendCommandWithParameterBag found element elem != elems.end() ? " << (elem != elems.end()));
      DEBUG("GEMSOAPToolBox::sendCommandWithParameterBag  qualifiedName: "
            << elem->getElementName().getQualifiedName());
      DEBUG("GEMSOAPToolBox::sendCommandWithParameterBag  prefix: "
            << elem->getElementName().getPrefix());
      DEBUG("GEMSOAPToolBox::sendCommandWithParameterBag  localName: "
            << elem->getElementName().getLocalName());
      DEBUG("GEMSOAPToolBox::sendCommandWithParameterBag  URI: "
            << elem->getElementName().getURI());
      // // DEBUG("GEMSOAPToolBox::sendCommandWithParameterBag memory address: "
      // //       << std::hex << elem << std::dec);
      DEBUG("GEMSOAPToolBox::sendCommandWithParameterBag  has value: "
            << elem->getValue());
      DEBUG("GEMSOAPToolBox::sendCommandWithParameterBag and has "
            << elem->getChildElements().size() << " subchildren");
    }

    std::string tool;
    xoap::dumpTree(msg->getSOAPPart().getEnvelope().getDOMNode(),tool);
    // msg->writeTo(tool);
    INFO("GEMSOAPToolBox::sendCommandWithParameterBag: SOAP message is: " << tool);
    tool.clear();

    reply = appCxt->postSOAP(msg, *srcDsc, *destDsc);
    xoap::dumpTree(reply->getSOAPPart().getEnvelope().getDOMNode(),tool);
    // reply->writeTo(tool);
    INFO("GEMSOAPToolBox::sendCommandWithParameterBag: SOAP reply is" << tool);
  } catch (gem::utils::exception::Exception& e) {
    std::string errMsg = toolbox::toString("Send command with parameter bag failed [%s]", e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (xdaq::exception::Exception& e) {
    std::string errMsg = toolbox::toString("Send command with parameter bag failed [%s]", e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (xcept::Exception& e) {
    std::string errMsg = toolbox::toString("Send command with parameter bag failed [%s]", e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (std::exception& e) {
    std::string errMsg = toolbox::toString("Send command with parameter bag failed [%s]", e.what());
    XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
  } catch (...) {
    std::string errMsg = toolbox::toString("Send command with parameter bag failed");
    XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
  }
  return true;
}


bool gem::utils::soap::GEMSOAPToolBox::sendApplicationParameter(std::string const& parName,
                                                                std::string const& parType,
                                                                std::string const& parValue,
                                                                xdaq::ApplicationContext* appCxt,
                                                                xdaq::ApplicationDescriptor* srcDsc,
                                                                xdaq::ApplicationDescriptor* destDsc
                                                                )
  throw (gem::utils::exception::SOAPException)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMSOAPToolBoxLogger"));
  try {
    xoap::MessageReference msg = xoap::createMessage(), reply;

    xoap::SOAPEnvelope env       = msg->getSOAPPart().getEnvelope();
    xoap::SOAPName     soapcmd   = env.createName("ParameterSet", "xdaq", XDAQ_NS_URI);
    xoap::SOAPElement  container = env.getBody().addBodyElement(soapcmd);
    env.addNamespaceDeclaration("xsd", "http://www.w3.org/2001/XMLSchema");
    env.addNamespaceDeclaration("xsi", "http://www.w3.org/2001/XMLSchema-instance");
    env.addNamespaceDeclaration("soapenc", "http://schemas.xmlsoap.org/soap/encoding/");
    xoap::SOAPName    tname    = env.createName("type", "xsi", "http://www.w3.org/2001/XMLSchema-instance");
    std::string       appURN   = "urn:xdaq-application:"+destDsc->getClassName();
    xoap::SOAPName    pboxname = env.createName("Properties", "props", appURN);
    xoap::SOAPElement pbox     = container.addChildElement(pboxname);
    pbox.addAttribute(tname, "soapenc:Struct");
    xoap::SOAPName    soapName = env.createName(parName, "props", appURN);
    xoap::SOAPElement cs       = pbox.addChildElement(soapName);
    cs.addAttribute(tname, parType);
    cs.addTextNode(parValue);

    std::string tool;
    INFO("GEMSOAPToolBox::sendApplicationParameter message:");
    msg->writeTo(tool);
    INFO(tool);
    tool.clear();

    reply = appCxt->postSOAP(msg, *srcDsc, *destDsc);
    INFO("GEMSOAPToolBox::sendApplicationParameter reply:");
    reply->writeTo(tool);
    INFO(tool);
  } catch (gem::utils::exception::Exception& e) {
    std::string errMsg = toolbox::toString("Send application parameter %s[%s,%s] failed [%s]",
                                           parName.c_str(), parType.c_str(), parValue.c_str(), e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (xdaq::exception::Exception& e) {
    std::string errMsg = toolbox::toString("Send application parameter %s[%s,%s] failed [%s]",
                                           parName.c_str(), parType.c_str(), parValue.c_str(), e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (xcept::Exception& e) {
    std::string errMsg = toolbox::toString("Send application parameter %s[%s,%s] failed [%s]",
                                           parName.c_str(), parType.c_str(), parValue.c_str(), e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (std::exception& e) {
    std::string errMsg = toolbox::toString("Send application parameter %s[%s,%s] failed [%s]",
                                           parName.c_str(), parType.c_str(), parValue.c_str(), e.what());
    XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
  } catch (...) {
    std::string errMsg = toolbox::toString("Send application parameter %s[%s,%s] failed",
                                           parName.c_str(), parType.c_str(), parValue.c_str());
    XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
  }
  return true;
}

// template <typename T>
// bool gem::utils::soap::GEMSOAPToolBox::sendApplicationParameterBag(std::string const& bagName,
//                                                                    xdata::Bag<T> const& bag,
//                                                                    xdaq::ApplicationContext* appCxt,
//                                                                    xdaq::ApplicationDescriptor* srcDsc,
//                                                                    xdaq::ApplicationDescriptor* destDsc
//                                                                    )
//   throw (gem::utils::exception::SOAPException)
// {
//   try {
//     xoap::MessageReference msg = xoap::createMessage(), reply;

//     xoap::SOAPEnvelope env       = msg->getSOAPPart().getEnvelope();
//     xoap::SOAPName     soapcmd   = env.createName("ParameterSet", "xdaq", XDAQ_NS_URI);
//     xoap::SOAPElement  container = env.getBody().addBodyElement(soapcmd);
//     container.addNamespaceDeclaration("xsd", "http://www.w3.org/2001/XMLSchema");
//     container.addNamespaceDeclaration("xsi", "http://www.w3.org/2001/XMLSchema-instance");
//     container.addNamespaceDeclaration("soapenc", "http://schemas.xmlsoap.org/soap/encoding/");
//     xoap::SOAPName    tname    = env.createName("type", "xsi", "http://www.w3.org/2001/XMLSchema-instance");
//     std::string       appURN   = "urn:xdaq-application:"+destDsc->getClassName();
//     xoap::SOAPName    pboxname = env.createName("Properties", "props", appURN);
//     xoap::SOAPElement pbox     = container.addChildElement(pboxname);
//     pbox.addAttribute(tname, "soapenc:Struct");
//     xoap::SOAPName    soapBagName = env.createName(bagName, "props", appURN);
//     xoap::SOAPElement csbag       = pbox.addChildElement(soapBagName);
//     csbag.addAttribute(tname, "soapenc:Struct");
//     for (auto b = bag.begin(); b != bag.end(); ++b) {
//       xdata::Serializable* s = bag.getField((*b).first);
//       xoap::SOAPName    soapName = env.createName(b->first, "props", appURN);
//       xoap::SOAPElement cs       = csbag.addChildElement(soapName);
//       csbag.addAttribute(tname, getXSDType(*s));
//       csbag.addTextNode(s->toString());
//     }
//     reply = appCxt->postSOAP(msg, *srcDsc, *destDsc);
//   } catch (gem::utils::exception::Exception& e) {
//     std::string errMsg = toolbox::toString("Send application parameter bag %s failed [%s] (gem::utils::exception::Exception)",
//                                            bagName.c_str(), e.what());
//     XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
//   } catch (xcept::Exception& e) {
//     std::string errMsg = toolbox::toString("Send application parameter bag %s failed [%s] (xcept::Exception)",
//                                            bagName.c_str(), e.what());
//     XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
//   } catch (std::exception& e) {
//     std::string errMsg = toolbox::toString("Send application parameter bag %s failed [%s] (std::exception)",
//                                            bagName.c_str(), e.what());
//     XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
//   } catch (...) {
//     std::string errMsg = toolbox::toString("Send application parameter bag %s failed (...)",
//                                            bagName.c_str());
//     XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
//   }
//   return true;
// }

xoap::MessageReference gem::utils::soap::GEMSOAPToolBox::createStateRequestMessage(std::string const& nstag,
                                                                                   std::string const& appURN,
                                                                                   bool const& isGEMApp)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMSOAPToolBoxLogger"));
  xoap::MessageReference msg = xoap::createMessage();

  xoap::SOAPEnvelope env       = msg->getSOAPPart().getEnvelope();
  xoap::SOAPName     soapcmd   = env.createName("ParameterGet", "xdaq", XDAQ_NS_URI);
  xoap::SOAPName     tname     = env.createName("type", "xsi", "http://www.w3.org/2001/XMLSchema-instance");
  xoap::SOAPElement  container = env.getBody().addBodyElement(soapcmd);
  env.addNamespaceDeclaration("xsd", "http://www.w3.org/2001/XMLSchema");
  env.addNamespaceDeclaration("xsi", "http://www.w3.org/2001/XMLSchema-instance");
  env.addNamespaceDeclaration("soapenc", "http://schemas.xmlsoap.org/soap/encoding/");
  xoap::SOAPName    pboxname = env.createName("properties", nstag, appURN);
  xoap::SOAPElement prop     = container.addChildElement(pboxname);
  prop.addAttribute(tname, "soapenc:Struct");

  if (isGEMApp) {
    xoap::SOAPName    msgN   = env.createName("StateMessage",  nstag, appURN);
    xoap::SOAPElement msgE   = prop.addChildElement(msgN);
    xoap::SOAPName    progN  = env.createName("StateProgress", nstag, appURN);
    xoap::SOAPElement progE  = prop.addChildElement(progN);
    xoap::SOAPName    stateN = env.createName("StateName",     nstag, appURN);
    xoap::SOAPElement stateE = prop.addChildElement(stateN);
    msgE.addAttribute(  tname, "xsd:string");
    progE.addAttribute( tname, "xsd:double");
    stateE.addAttribute(tname, "xsd:string");
  } else {
    xoap::SOAPName    stateN = env.createName("stateName", nstag, appURN);
    xoap::SOAPElement stateE = prop.addChildElement(stateN);
    stateE.addAttribute(tname, "xsd:string");
  }

  return msg;
}

std::string gem::utils::soap::GEMSOAPToolBox::getApplicationState(xdaq::ApplicationContext*    appCxt,
                                                                  xdaq::ApplicationDescriptor* srcDsc,
                                                                  xdaq::ApplicationDescriptor* destDsc)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMSOAPToolBoxLogger"));
  try {
    std::string  appClass    = destDsc->getClassName();
    // std::string    appURN    = destDsc->getURN();
    std::string    appURN    = "urn:xdaq-application:" + appClass;
    std::string responseName = "StateName";
    bool isGEMApp = true;

    if (appURN.find("tcds") != std::string::npos) {
      responseName = "stateName";
      isGEMApp     = false;
    }

    xoap::MessageReference msg =
      xoap::createMessage(gem::utils::soap::GEMSOAPToolBox::createStateRequestMessage("app", appURN, isGEMApp));
    std::string nstag = "gemapp";

    std::stringstream debugstream;
    msg->writeTo(debugstream);

    xoap::MessageReference answer = appCxt->postSOAP(msg, *srcDsc, *destDsc);

    xoap::SOAPName stateReply(responseName, nstag, appURN);
    xoap::SOAPElement props = answer->getSOAPPart().getEnvelope().getBody().getChildElements()[0].getChildElements()[0];
    std::vector<xoap::SOAPElement> basic = props.getChildElements(stateReply);

    if (basic.size() == 1) {
      std::string stateString = basic[0].getValue();
      DEBUG("GEMSOAPToolBox::createStateRequestMessage application " << destDsc->getClassName()
            << " returned state " << stateString);
      return stateString;
    } else {
      std::string toolInput;
      xoap::dumpTree(msg->getSOAPPart().getEnvelope().getDOMNode(),    toolInput);
      std::string  toolOutput;
      xoap::dumpTree(answer->getSOAPPart().getEnvelope().getDOMNode(), toolOutput);

      if (answer->getSOAPPart().getEnvelope().getBody().hasFault()) {
        std::stringstream errMsg;
        errMsg << "SOAP fault getting state: " << std::endl
               << "SOAP request:"      << std::endl << toolInput  << std::endl
               << "SOAP reply:"        << std::endl << toolOutput << std::endl
               << "SOAP fault string:" << std::endl
               << answer->getSOAPPart().getEnvelope().getBody().getFault().getFaultString();
        XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg.str());
      }
    }
  } catch (gem::utils::exception::SOAPException& e) {
    std::string errMsg = toolbox::toString("Get application state parameter failed [%s]", e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (xoap::exception::Exception& e) {
    std::string errMsg = toolbox::toString("Get application state parameter failed [%s]", e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (xdaq::exception::Exception& e) {
    std::string errMsg = toolbox::toString("Get application state parameter failed [%s]", e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (xcept::Exception& e) {
    std::string errMsg = toolbox::toString("Get application state parameter failed [%s]", e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (std::exception& e) {
    std::string errMsg = toolbox::toString("Get application state parameter failed [%s]", e.what());
    XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
  } catch (...) {
    std::string errMsg = toolbox::toString("Get application state parameter failed");
    XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
  }
}

// example for sending nested parameters, as are stored in an xdata::Bag
void gem::utils::soap::GEMSOAPToolBox::sendAMC13Config(xdaq::ApplicationContext* appCxt,
                                                       xdaq::ApplicationDescriptor* srcDsc,
                                                       xdaq::ApplicationDescriptor* destDsc)
{
  log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMSOAPToolBoxLogger"));
  try {
    xoap::MessageReference msg = xoap::createMessage(), reply;

    xoap::SOAPEnvelope env       = msg->getSOAPPart().getEnvelope();
    xoap::SOAPName     soapcmd   = env.createName("ParameterSet", "xdaq", XDAQ_NS_URI);
    xoap::SOAPElement  container = env.getBody().addBodyElement(soapcmd);
    env.addNamespaceDeclaration("xsd", "http://www.w3.org/2001/XMLSchema");
    env.addNamespaceDeclaration("xsi", "http://www.w3.org/2001/XMLSchema-instance");
    env.addNamespaceDeclaration("soapenc", "http://schemas.xmlsoap.org/soap/encoding/");

    xoap::SOAPName tname    = env.createName("type", "xsi", "http://www.w3.org/2001/XMLSchema-instance");
    std::string    appURN   = "urn:xdaq-application:"+destDsc->getClassName();
    xoap::SOAPName pboxname = env.createName("Properties", "props", appURN);
    xoap::SOAPElement pbox  = container.addChildElement(pboxname);
    pbox.addAttribute(tname, "soapenc:Struct");

    xoap::SOAPName    amc13soapName = env.createName("amc13ConfigParams", "props", appURN);
    xoap::SOAPElement amc13pbox     = pbox.addChildElement(amc13soapName);
    amc13pbox.addAttribute(tname, "soapenc:Struct");

    xoap::SOAPName    l1aBurstName = env.createName("L1Aburst",   "props", appURN);
    xoap::SOAPName    bgoChanName  = env.createName("BGOChannel", "props", appURN);
    xoap::SOAPName    bgocmdName   = env.createName("BGOcmd",     "props", appURN);
    xoap::SOAPElement l1a_cs       = amc13pbox.addChildElement(l1aBurstName);
    l1a_cs.addAttribute(tname, "xsd:unsignedInt");
    l1a_cs.addTextNode("1357");
    xoap::SOAPElement chan_cs  = amc13pbox.addChildElement(bgoChanName);
    chan_cs.addAttribute(tname, "xsd:integer");
    chan_cs.addTextNode("3");
    xoap::SOAPElement cmd_cs   = amc13pbox.addChildElement(bgocmdName);
    cmd_cs.addAttribute(tname, "xsd:unsignedInt");
    cmd_cs.addTextNode("0x18");

    reply = appCxt->postSOAP(msg, *srcDsc, *destDsc);
  } catch (gem::utils::exception::Exception& e) {
    std::string errMsg = toolbox::toString("Send application parameter failed [%s]", e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (xcept::Exception& e) {
    std::string errMsg = toolbox::toString("Send application parameter failed [%s]", e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
  } catch (std::exception& e) {
    std::string errMsg = toolbox::toString("Send application parameter failed [%s]", e.what());
    XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
  } catch (...) {
    std::string errMsg = toolbox::toString("Send application parameter failed");
    XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
  }
}

std::string gem::utils::soap::GEMSOAPToolBox::getXSDType(xdata::Serializable const& item)
{
  if (item.type() == "bool")
    return "xsd:boolean";

  else if (item.type() == "double")
    return "xsd:double";

  else if (item.type() == "float")
    return "xsd:float";

  else if (item.type() == "int 32")
    return "xsd:int";

  else if (item.type() == "int 64")
    return "xsd:long";

  else if (item.type() == "int")
    return "xsd:integer";

  else if (item.type() == "mime")
    return "xsd:mime";

  else if (item.type() == "properties")
    return "xsd:properties";

  else if (item.type() == "string")
    return "xsd:string";

  else if (item.type() == "table")
    return "xsd:table";

  else if (item.type() == "time")
    return "xsd:dateTime";

  else if (item.type() == "unsigned int 32")
    return "xsd:unsignedInt";

  else if (item.type() == "unsigned int 64")
    return "xsd:unsignedLong";

  else if (item.type() == "unsigned int")
    return "xsd:unsignedInt";

  else if (item.type() == "unsigned long")
    return "xsd:unsignedLong";

  else if (item.type() == "unsigned short")
    return "xsd:unsignedShort";
}
