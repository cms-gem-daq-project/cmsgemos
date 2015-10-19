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
                  ::toolbox::toString("Command %s failed [%s]",cmd.c_str(),e.what()),e);
  } catch (std::exception& e) {
    XCEPT_RAISE(gem::utils::exception::SOAPException,
                ::toolbox::toString("Command %s failed [%s]",cmd.c_str(),e.what()));
  } catch (...) {
    XCEPT_RAISE(gem::utils::exception::SOAPException,
                ::toolbox::toString("Command %s failed",cmd.c_str()));
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
    
    //LOG4CPLUS_INFO(logger, "sendParameter(" + cmd + ") received response: " + tool);
    
  } catch (xcept::Exception& e) {
    XCEPT_RETHROW(gem::utils::exception::SOAPException,
                  ::toolbox::toString("Sending parameter  %s (value %d) failed [%s]",
                                      cmd.c_str(), parameter, e.what()),e);
  } catch (std::exception& e) {
    XCEPT_RAISE(gem::utils::exception::SOAPException,
                ::toolbox::toString("Sending parameter  %s (value %d) failed [%s]",
                                    cmd.c_str(), parameter, e.what()));
  } catch (...) {
    XCEPT_RAISE(gem::utils::exception::SOAPException,
                ::toolbox::toString("Sending parameter  %s (value %d) failed",
                                    cmd.c_str(), parameter));
  }
  return true;
}
