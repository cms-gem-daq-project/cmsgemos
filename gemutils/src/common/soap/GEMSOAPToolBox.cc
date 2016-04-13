#include "gem/utils/soap/GEMSOAPToolBox.h"

#include "gem/utils/exception/Exception.h"
#include "log4cplus/logger.h"
#include "log4cplus/loggingmacros.h"
#include "gem/utils/GEMLogging.h"

#include "toolbox/string.h"
#include "xdaq/NamespaceURI.h"

#include "xoap/MessageFactory.h"
#include "xoap/domutils.h"
#include "xoap/SOAPBody.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPPart.h"
#include "xoap/SOAPSerializer.h"

xoap::MessageReference gem::utils::soap::GEMSOAPToolBox::makeSOAPReply(std::string const& command,
                                                                       std::string const& response)
{
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
  //xoap::MessageFactory* messageFactory = xoap::MessageFactory::getInstance(soapProtocolVersion);
  //xoap::MessageReference reply         = messageFactory->createMessage();
  xoap::MessageReference reply         = xoap::createMessage();
  xoap::SOAPEnvelope     envelope      = reply->getSOAPPart().getEnvelope();
  xoap::SOAPBody         body          = envelope.getBody();
  std::string            replyString   = event + "Response";
  std::cout << "GEMSOAPToolBox::makeFSMSOAPReply::DEBUG replyString "
            << replyString << std::endl;
  xoap::SOAPName         replyName     = envelope.createName(replyString, "xdaq", XDAQ_NS_URI);
  std::cout << "GEMSOAPToolBox::makeFSMSOAPReply::DEBUG replyName "
            << replyName.getLocalName() << std::endl;
  xoap::SOAPBodyElement  replyElement  = body.addBodyElement(replyName);
  std::cout << "GEMSOAPToolBox::makeFSMSOAPReply::DEBUG replyElement "
            << replyElement.getTextContent() << std::endl;
  xoap::SOAPName         stateName     = envelope.createName("state", "xdaq", XDAQ_NS_URI);
  std::cout << "GEMSOAPToolBox::makeFSMSOAPReply::DEBUG stateName "
            << stateName.getLocalName() << std::endl;
  xoap::SOAPElement      stateElement  = replyElement.addChildElement(stateName);
  std::cout << "GEMSOAPToolBox::makeFSMSOAPReply::DEBUG stateElement"
            << stateElement.getTextContent() << std::endl;
  xoap::SOAPName         attributeName = envelope.createName("stateName", "xdaq", XDAQ_NS_URI);
  std::cout << "GEMSOAPToolBox::makeFSMSOAPReply::DEBUG attributeName "
            << attributeName.getLocalName() << std::endl;
  stateElement.addAttribute(attributeName, state);
  std::cout << "GEMSOAPToolBox::makeFSMSOAPReply::DEBUG reply " << std::endl;
  reply->writeTo(std::cout);
  std::cout << std::endl;
  return reply;
}


std::string gem::utils::soap::GEMSOAPToolBox::extractFSMCommandName(xoap::MessageReference const& msg)
{
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
                                                   //log4cplus::Logger* logger,
                                                   //std::string const& param
                                                   )
  throw (gem::utils::exception::Exception)
{
  try {
    xoap::MessageReference msg = xoap::createMessage();
    
    xoap::SOAPEnvelope env = msg->getSOAPPart().getEnvelope();
    xoap::SOAPName soapcmd = env.createName(cmd,"xdaq", XDAQ_NS_URI);
    env.getBody().addBodyElement(soapcmd);
    
    xoap::MessageReference response = appCxt->postSOAP(msg,*srcDsc,*destDsc);
    std::string  tool;
    xoap::dumpTree(response->getSOAPPart().getEnvelope().getDOMNode(),tool);
    
    //LOG4CPLUS_INFO(logger, "sendCommand(" + cmd + ") received response: " + tool);
    
  } catch (xcept::Exception& e) {
    XCEPT_RETHROW(gem::utils::exception::SOAPException,
                  toolbox::toString("Command %s failed [%s]",cmd.c_str(),e.what()),e);
  } catch (std::exception& e) {
    XCEPT_RAISE(gem::utils::exception::SOAPException,
                toolbox::toString("Command %s failed [%s]",cmd.c_str(),e.what()));
  } catch (...) {
    XCEPT_RAISE(gem::utils::exception::SOAPException,
                toolbox::toString("Command %s failed",cmd.c_str()));
  }
  return true;
}

bool gem::utils::soap::GEMSOAPToolBox::sendParameter(std::vector<std::string> const& parameter,
                                                     xdaq::ApplicationContext* appCxt,
                                                     xdaq::ApplicationDescriptor* srcDsc,
                                                     xdaq::ApplicationDescriptor* destDsc
                                                     //log4cplus::Logger* logger
                                                     )
  throw (gem::utils::exception::Exception)
{
  if (parameter.size() != 3)
    return false;
  
  try {
    xoap::MessageReference msg = xoap::createMessage();
    
    xoap::SOAPEnvelope env       = msg->getSOAPPart().getEnvelope();
    xoap::SOAPName     soapcmd   = env.createName("ParameterSet","xdaq", XDAQ_NS_URI);
    xoap::SOAPElement  container = env.getBody().addBodyElement(soapcmd);

    // from hcal supervisor
    container.addNamespaceDeclaration("xsd","http://www.w3.org/2001/XMLSchema");
    container.addNamespaceDeclaration("xsi","http://www.w3.org/2001/XMLSchema-instance");
    container.addNamespaceDeclaration("soapenc","http://schemas.xmlsoap.org/soap/encoding/");
    xoap::SOAPName    type       = env.createName("type","xsi","http://www.w3.org/2001/XMLSchema-instance");
    std::string       appUrn     = "urn:xdaq-application:"+destDsc->getClassName();
    xoap::SOAPName    properties = env.createName("Properties","props",appUrn);
    xoap::SOAPElement property   = container.addChildElement(properties);
    property.addAttribute(type,"soapenc:Struct");
    xoap::SOAPName    cfgStyleName = env.createName(parameter.at(0),"props",appUrn);
    xoap::SOAPElement cs           = property.addChildElement(cfgStyleName);
    cs.addAttribute(type,parameter.at(2));
    cs.addTextNode(parameter.at(1));
    // end from hcal supervisor
    
    xoap::MessageReference response = appCxt->postSOAP(msg,*srcDsc,*destDsc);
    std::string  tool;
    xoap::dumpTree(response->getSOAPPart().getEnvelope().getDOMNode(),tool);
    
    //LOG4CPLUS_INFO(logger, "sendParameter(" + cmd + ") received response: " + tool);
    // need to catch failures somehow
    std::cout << "GEMSOAPToolBox::" << appUrn << " ParameterSet received response: " << tool << std::endl;
    
  } catch (xcept::Exception& e) {
    XCEPT_RETHROW(gem::utils::exception::SOAPException,
                  toolbox::toString("Parameter %s failed [%s]",parameter.at(0).c_str(),e.what()),e);
  } catch (std::exception& e) {
    XCEPT_RAISE(gem::utils::exception::SOAPException,
                toolbox::toString("Parameter %s failed [%s]",parameter.at(0).c_str(),e.what()));
  } catch (...) {
    XCEPT_RAISE(gem::utils::exception::SOAPException,
                toolbox::toString("Parameter %s failed",parameter.at(0).c_str()));
  }
  return true;
}

std::pair<std::string,std::string> gem::utils::soap::GEMSOAPToolBox::extractCommandWithParameter(xoap::MessageReference const& msg)
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
  return std::make_pair(commandName,parameterValue);
}

bool gem::utils::soap::GEMSOAPToolBox::sendCommandWithParameter(std::string const& cmd, int const& parameter,
                                                                xdaq::ApplicationContext* appCxt,
                                                                xdaq::ApplicationDescriptor* srcDsc,
                                                                xdaq::ApplicationDescriptor* destDsc
                                                                )
  throw (gem::utils::exception::Exception)
{
  try {
    xoap::MessageReference msg = xoap::createMessage();
    
    xoap::SOAPEnvelope env = msg->getSOAPPart().getEnvelope();
    xoap::SOAPName soapcmd = env.createName(cmd,"xdaq", XDAQ_NS_URI);
    xoap::SOAPBodyElement paramValue = env.getBody().addBodyElement(soapcmd);
    paramValue.addTextNode(toolbox::toString("%d",parameter));
    
    xoap::MessageReference response = appCxt->postSOAP(msg,*srcDsc,*destDsc);
    std::string  tool;
    xoap::dumpTree(response->getSOAPPart().getEnvelope().getDOMNode(),tool);
    // need to catch failures somehow
    //LOG4CPLUS_INFO(logger, "sendParameter(" + cmd + ") received response: " + tool);
    
  } catch (xcept::Exception& e) {
    XCEPT_RETHROW(gem::utils::exception::SOAPException,
                  toolbox::toString("Sending parameter  %s (value %d) failed [%s]",
                                    cmd.c_str(), parameter, e.what()),e);
  } catch (std::exception& e) {
    XCEPT_RAISE(gem::utils::exception::SOAPException,
                toolbox::toString("Sending parameter  %s (value %d) failed [%s]",
                                  cmd.c_str(), parameter, e.what()));
  } catch (...) {
    XCEPT_RAISE(gem::utils::exception::SOAPException,
                toolbox::toString("Sending parameter  %s (value %d) failed",
                                  cmd.c_str(), parameter));
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
  throw (gem::utils::exception::Exception)
{
  try {
    xoap::MessageReference msg = xoap::createMessage(), answer;
  
    xoap::SOAPEnvelope env       = msg->getSOAPPart().getEnvelope();
    xoap::SOAPName     soapcmd   = env.createName("ParameterSet","xdaq",XDAQ_NS_URI);
    xoap::SOAPElement  container = env.getBody().addBodyElement(soapcmd);
    container.addNamespaceDeclaration("xsd","http://www.w3.org/2001/XMLSchema");
    container.addNamespaceDeclaration("xsi","http://www.w3.org/2001/XMLSchema-instance");
    container.addNamespaceDeclaration("soapenc","http://schemas.xmlsoap.org/soap/encoding/");
    xoap::SOAPName tname    = env.createName("type","xsi","http://www.w3.org/2001/XMLSchema-instance");
    std::string    appUrn   = "urn:xdaq-application:"+destDsc->getClassName();
    xoap::SOAPName pboxname = env.createName("Properties","props",appUrn);
    xoap::SOAPElement pbox = container.addChildElement(pboxname);
    pbox.addAttribute(tname,"soapenc:Struct");
    xoap::SOAPName    soapName = env.createName(parName,"props",appUrn);
    xoap::SOAPElement cs      = pbox.addChildElement(soapName);
    cs.addAttribute(tname,parType);
    cs.addTextNode(parValue);
    
    answer = appCxt->postSOAP(msg,*srcDsc,*destDsc);
  } catch (gem::utils::exception::Exception& e) {
    std::string errMsg = toolbox::toString("Send application parameter %s[%s,%s] failed [%s] (gem::utils::exception::Exception)",
                                           parName.c_str(),parType.c_str(),parValue.c_str(),e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException,errMsg,e);
  } catch (xcept::Exception& e) {
    std::string errMsg = toolbox::toString("Send application parameter %s[%s,%s] failed [%s] (xcept::Exception)",
                                           parName.c_str(),parType.c_str(),parValue.c_str(),e.what());
    XCEPT_RETHROW(gem::utils::exception::SOAPException,errMsg,e);
  } catch (std::exception& e) {
    std::string errMsg = toolbox::toString("Send application parameter %s[%s,%s] failed [%s] (std::exception)",
                                           parName.c_str(),parType.c_str(),parValue.c_str(),e.what());
    XCEPT_RAISE(gem::utils::exception::SOAPException,errMsg);
  } catch (...) {
    std::string errMsg = toolbox::toString("Send application parameter %s[%s,%s] failed (...)",
                                           parName.c_str(),parType.c_str(),parValue.c_str());
    XCEPT_RAISE(gem::utils::exception::SOAPException,errMsg);
  }
  return true;
}
