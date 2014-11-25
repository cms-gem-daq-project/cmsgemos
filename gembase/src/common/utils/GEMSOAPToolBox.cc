#include "gem/base/utils/GEMSOAPToolBox.h"

#include "toolbox/string.h"
#include "xcept/Exception.h"
#include "xdaq/NamespaceURI.h"

#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/dom/DOMNode.hpp"

#include "xoap/domutils.h"
#include "xoap/DOMParser.h"
#include "xoap/exception/Exception.h"
#include "xoap/Method.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPBody.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPPart.h"

xoap::MessageReference
gem::base::utils::GEMSOAPToolBox::makeSoapReply(std::string const& command,
						std::string const& answer)
{
  xoap::MessageReference reply        = xoap::createMessage();
  xoap::SOAPEnvelope     envelope     = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName         responseName = envelope.createName(command, "xdaq", XDAQ_NS_URI);
  xoap::SOAPElement      bodyElement  = envelope.getBody().addBodyElement(responseName);
  bodyElement.addTextNode(answer);
  return reply;
}

xoap::MessageReference
gem::base::utils::GEMSOAPToolBox::makeSoapFaultReply(std::string const& faultString,
						     std::string const& faultCode,
						     std::string const& faultActor,
						     std::string const& detail)
{
  xoap::MessageReference  reply       = xoap::createMessage();
  xoap::SOAPEnvelope      envelope    = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName          faultName   = envelope.createName("Fault", "xdaq", XDAQ_NS_URI);
  xoap::SOAPElement       bodyElement = envelope.getBody().addBodyElement(faultName);

  xoap::SOAPName    faultStringName    = envelope.createName("faultstring", "xdaq", XDAQ_NS_URI);
  xoap::SOAPElement faultStringElement = bodyElement.addChildElement(faultStringName);
  faultStringElement.addTextNode(faultString);

  xoap::SOAPName    faultCodeName    = envelope.createName("faultcode", "xdaq", XDAQ_NS_URI);
  xoap::SOAPElement faultCodeElement = bodyElement.addChildElement(faultCodeName);
  faultCodeElement.addTextNode(faultCode);

  if (detail.size() > 0)
    {
      xoap::SOAPName    detailName    = envelope.createName("detail", "xdaq", XDAQ_NS_URI);
      xoap::SOAPElement detailElement = bodyElement.addChildElement(detailName);
      detailElement.addTextNode(detail);
    }

  if (faultActor.size() > 0)
    {
      xoap::SOAPName    faultActorName    = envelope.createName("faultactor", "xdaq", XDAQ_NS_URI);
      xoap::SOAPElement faultActorElement = bodyElement.addChildElement(faultActorName);
      faultActorElement.addTextNode(faultActor);
    }
  
  return reply;
}

xoap::MessageReference
gem::base::utils::GEMSOAPToolBox::makeFsmSoapReply(std::string const& event,
						   std::string const& state)
{
  xoap::MessageReference reply         = xoap::createMessage();
  xoap::SOAPEnvelope     envelope      = reply->getSOAPPart().getEnvelope();
  xoap::SOAPBody         body          = envelope.getBody();
  std::string            replyString   = event + "Response";
  xoap::SOAPName         replyName     = envelope.createName(replyString, "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement  replyElement  = body.addBodyElement(replyName);
  xoap::SOAPName         stateName     = envelope.createName("state", "xdaq", XDAQ_NS_URI);
  xoap::SOAPElement      stateElement  = replyElement.addChildElement(stateName);
  xoap::SOAPName         attributeName = envelope.createName("stateName", "xdaq", XDAQ_NS_URI);
  stateElement.addAttribute(attributeName, state);
  return reply;
}

std::string
gem::base::utils::GEMSOAPToolBox::extractFSMCommandName(xoap::MessageReference const& msg)
{
  XERCES_CPP_NAMESPACE::DOMNodeList* bodyList = extractBodyNodes(msg);
  
  // The body should contain a single node with the name of the FSM
  // command to execute.
  if (bodyList->getLength() != 1)
    {
      XCEPT_RAISE(xoap::exception::Exception,
                  toolbox::toString("Expected exactly one element "
                                    "in FSM command SOAP message, "
                                    "but found %d.", bodyList->getLength()));
    }
  XERCES_CPP_NAMESPACE::DOMNode* commandNode = bodyList->item(0);
  std::string commandName = xoap::XMLCh2String(commandNode->getLocalName());
  
  return commandName;
}

XERCES_CPP_NAMESPACE::DOMNodeList*
gem::base::utils::GEMSOAPToolBox::extractBodyNodes(xoap::MessageReference const& msg)
{
  return msg->getSOAPPart().getEnvelope().getBody().getDOMNode()->getChildNodes();
}
