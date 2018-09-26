/** @file GEMSOAPToolBox.h */

#ifndef GEM_UTILS_SOAP_GEMSOAPTOOLBOX_H
#define GEM_UTILS_SOAP_GEMSOAPTOOLBOX_H

// using the SOAP toolbox defined in the TCDS code base with extra functionality from the EMU codebase
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <xoap/MessageReference.h>
#include <xoap/MessageFactory.h>
#include <xoap/domutils.h>
#include <xoap/SOAPBody.h>
#include <xoap/SOAPEnvelope.h>
#include <xoap/SOAPPart.h>
#include <xoap/SOAPSerializer.h>

#include <xercesc/util/XercesDefs.hpp>

#include <xdaq/NamespaceURI.h>

#include <xdaq/ApplicationDescriptor.h>
#include <xdaq/ApplicationContext.h>

#include <xdata/Bag.h>
#include <xdata/Serializable.h>

#include <gem/utils/exception/Exception.h>
#include <gem/utils/GEMLogging.h>

namespace gem {
  namespace utils {
    namespace soap {

      class GEMSOAPToolBox
      {
      public:
        // methods copied from tcds soap helper
        static xoap::MessageReference makeSOAPReply(std::string const& command,
                                                    std::string const& response);
        static xoap::MessageReference makeSOAPFaultReply(std::string const& faultString,
                                                         std::string const& faultCode="Server",
                                                         std::string const& faultActor="",
                                                         std::string const& detail="");
        static xoap::MessageReference makeFSMSOAPReply(std::string const& event,
                                                       std::string const& state);

        static std::string extractFSMCommandName(xoap::MessageReference const& msg);

        /**
         * @param cmd command to send to the application
         * @param appCxt context in which the source/receiver applications are running
         * @param srcDsc source application descriptor
         * @param destDsc destination application descriptor
         * @param logger logger object
         * @returns true if successful/completed
         */
        static bool sendCommand(std::string const& cmd,
                                xdaq::ApplicationContext* appCxt,
                                xdaq::ApplicationDescriptor* srcDsc,
                                xdaq::ApplicationDescriptor* destDsc
                                // log4cplus::Logger* logger
                                )
          throw (gem::utils::exception::SOAPException);

        /**
         * @param cmd command to send to a TCDS application (which requires a special header)
         * @param appCxt context in which the source/receiver applications are running
         * @param srcDsc source application descriptor
         * @param destDsc destination application descriptor
         * @param logger logger object
         * @returns true if successful/completed
         */
        static bool sendTCDSCommand(std::string const& cmd,
                                    xdaq::ApplicationContext* appCxt,
                                    xdaq::ApplicationDescriptor* srcDsc,
                                    xdaq::ApplicationDescriptor* destDsc
                                    // log4cplus::Logger* logger
                                    )
          throw (gem::utils::exception::SOAPException);

        /**
         * @param parameter is a vector of strings, contaning the parameter name, value, and the xsd type for the SOAP transaction
         * @param appCxt context in which the source/receiver applications are running
         * @param srcDsc source application descriptor
         * @param destDsc destination application descriptor
         * @param logger logger object
         * @param param parameters to append to the SOAP message
         * @returns true if successful/completed
         */
        static bool sendParameter(std::vector<std::string> const& parameter,
                                  xdaq::ApplicationContext* appCxt,
                                  xdaq::ApplicationDescriptor* srcDsc,
                                  xdaq::ApplicationDescriptor* destDsc
                                  // log4cplus::Logger* logger,
                                  // std::string const& param
                                  )
          throw (gem::utils::exception::SOAPException);

        static std::pair<std::string,std::string> extractCommandWithParameter(xoap::MessageReference const& msg);

         /**
         * @param cmd command to send to the application
         * @param parameter parameter to send to the application
         * @param appCxt context in which the source/receiver applications are running
         * @param srcDsc source application descriptor
         * @param destDsc destination application descriptor
         * @returns true if successful/completed
         */
        static bool sendCommandWithParameter(std::string const& cmd, int const& parameter,
                                             xdaq::ApplicationContext* appCxt,
                                             xdaq::ApplicationDescriptor* srcDsc,
                                             xdaq::ApplicationDescriptor* destDsc
                                             )
          throw (gem::utils::exception::SOAPException);

         /**
         * @param cmd Name of the Command to send to the destination application
         * @param bag map of values (keyed by string) to send with the SOAP message
         * @param appCxt context in which the source/receiver applications are running
         * @param srcDsc source application descriptor
         * @param destDsc destination application descriptor
         * returns true if successful/completed
         */
        static bool sendCommandWithParameterBag(std::string const& cmd,
                                                std::unordered_map<std::string, xdata::Serializable*> const& bag,
                                                xdaq::ApplicationContext* appCxt,
                                                xdaq::ApplicationDescriptor* srcDsc,
                                                xdaq::ApplicationDescriptor* destDsc
                                                )
          throw (gem::utils::exception::SOAPException);

         /**
         * @param parName Name of the parameter in the destination application info space
         * @param parType xsd type of the specified parameter
         * @param parValue parameter value (as string) to send with the SOAP message
         * @param appCxt context in which the source/receiver applications are running
         * @param srcDsc source application descriptor
         * @param destDsc destination application descriptor
         * returns true if successful/completed
         */
        static bool sendApplicationParameter(std::string const& parName,
                                             std::string const& parType,
                                             std::string const& parValue,
                                             xdaq::ApplicationContext* appCxt,
                                             xdaq::ApplicationDescriptor* srcDsc,
                                             xdaq::ApplicationDescriptor* destDsc
                                             )
          throw (gem::utils::exception::SOAPException);

         /**
         * @param bagName Name of the parameter bag in the destination application info space
         * @param bag bag of parameter values to send with the SOAP message
         * @param appCxt context in which the source/receiver applications are running
         * @param srcDsc source application descriptor
         * @param destDsc destination application descriptor
         * returns true if successful/completed
         */
        template <typename T>
          static bool sendApplicationParameterBag(std::string const& bagName,
                                                  xdata::Bag<T> const& bag,
                                                  xdaq::ApplicationContext* appCxt,
                                                  xdaq::ApplicationDescriptor* srcDsc,
                                                  xdaq::ApplicationDescriptor* destDsc
                                                  )
          throw (gem::utils::exception::SOAPException) {
          log4cplus::Logger m_gemLogger(log4cplus::Logger::getInstance("GEMSOAPToolBoxLogger"));
          try {
            xoap::MessageReference msg = xoap::createMessage(), answer;

            xoap::SOAPEnvelope env       = msg->getSOAPPart().getEnvelope();
            xoap::SOAPName     soapcmd   = env.createName("ParameterSet", "xdaq", XDAQ_NS_URI);
            xoap::SOAPElement  container = env.getBody().addBodyElement(soapcmd);
            container.addNamespaceDeclaration("xsd", "http://www.w3.org/2001/XMLSchema");
            container.addNamespaceDeclaration("xsi", "http://www.w3.org/2001/XMLSchema-instance");
            container.addNamespaceDeclaration("soapenc", "http://schemas.xmlsoap.org/soap/encoding/");
            xoap::SOAPName    tname    = env.createName("type", "xsi", "http://www.w3.org/2001/XMLSchema-instance");
            std::string       appUrn   = "urn:xdaq-application:"+destDsc->getClassName();
            xoap::SOAPName    pboxname = env.createName("Properties", "props", appUrn);
            xoap::SOAPElement pbox     = container.addChildElement(pboxname);
            pbox.addAttribute(tname, "soapenc:Struct");

            xoap::SOAPName    soapBagName = env.createName(bagName, "props", appUrn);
            xoap::SOAPElement csbag       = pbox.addChildElement(soapBagName);
            csbag.addAttribute(tname, "soapenc:Struct");
            for (auto b = bag.begin(); b != bag.end(); ++b) {
              xdata::Serializable* s = bag.getField((*b).first);
              xoap::SOAPName    soapName = env.createName(b->first, "props", appUrn);
              xoap::SOAPElement cs       = csbag.addChildElement(soapName);
              cs.addAttribute(tname, getXSDType(*s));
              cs.addTextNode(s->toString());
            }
            std::string tool;
            xoap::dumpTree(msg->getSOAPPart().getEnvelope().getDOMNode(),tool);
            DEBUG("GEMSOAPToolBox::sendApplicationParameterBag: " << tool);
            answer = appCxt->postSOAP(msg, *srcDsc, *destDsc);
          } catch (gem::utils::exception::Exception& e) {
            std::string errMsg = toolbox::toString("Send application parameter bag %s failed [%s] (gem::utils::exception::Exception)",
                                                   bagName.c_str(), e.what());
            XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
          } catch (xcept::Exception& e) {
            std::string errMsg = toolbox::toString("Send application parameter bag %s failed [%s] (xcept::Exception)",
                                                   bagName.c_str(), e.what());
            XCEPT_RETHROW(gem::utils::exception::SOAPException, errMsg, e);
          } catch (std::exception& e) {
            std::string errMsg = toolbox::toString("Send application parameter bag %s failed [%s] (std::exception)",
                                                   bagName.c_str(), e.what());
            XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
          } catch (...) {
            std::string errMsg = toolbox::toString("Send application parameter bag %s failed (...)",
                                                   bagName.c_str());
            XCEPT_RAISE(gem::utils::exception::SOAPException, errMsg);
          }
          return true;
        }

        /**
         * @brief Creates a SOAP message requesting informtion about an application FSM state
         * @param nstag Namespace tag to append to the parameter request
         * @param appURN URN of the application to send the request to
         * @param isGEMApp whether to query additional parameters that are only in GEM applications
         * returns xoap::MessageReference to calling application
         */
        static xoap::MessageReference createStateRequestMessage(std::string const& nstag,
                                                                std::string const& appURN,
                                                                bool const& isGEMApp);

        /**
         * @brief Returns the FSM state from the given application
         * @param appCxt context in which the source/receiver applications are running
         * @param srcDsc source application descriptor
         * @param destDsc destination application descriptor
         * returns xoap::MessageReference to calling application
         */
        static std::string getApplicationState(xdaq::ApplicationContext*    appCxt,
                                               xdaq::ApplicationDescriptor* srcDsc,
                                               xdaq::ApplicationDescriptor* destDsc);

        /**
         * @brief Sends a bag of configuration parameters via SOAP to an AMC13
         * @param appCxt context in which the source/receiver applications are running
         * @param srcDsc source application descriptor
         * @param destDsc destination application descriptor
         * returns xoap::MessageReference to calling application
         */
        static void sendAMC13Config(xdaq::ApplicationContext* appCxt,
                                    xdaq::ApplicationDescriptor* src,
                                    xdaq::ApplicationDescriptor* dest);

        /**
         * @brief Obtain the correct XSD type identifier for a given xdata::Serializable object
         * @param item is an xdata::Serializable object
         * returns std::string corresponding to the xsd:<type> for creating a proper SOAP message
         */
        static std::string getXSDType(xdata::Serializable const& item);

        // methods copied from emu/soap/toolbox
        /*
          xoap::MessageReference createMessage( const gem::utils::soap::QualifiedName &command,
          const gem::utils::soap::Parameters &parameters = gem::utils::soap::Parameters::none,
          const gem::utils::soap::Attributes &attributes = gem::utils::soap::Attributes::none,
          const vector<gem::utils::soap::Attachment> &attachments = gem::utils::soap::Attachment::none );

          void addAttachments(           xoap::MessageReference message,
          const vector<gem::utils::soap::Attachment> &attachments );
          void addAttributes(            xoap::MessageReference message,
          xoap::SOAPElement* element,
          const gem::utils::soap::Attributes &attributes );
          void includeParameters(        xoap::MessageReference message,
          xoap::SOAPElement* parent,
          gem::utils::soap::Parameters &parameters );
          void includeParameters(        xoap::MessageReference message,
          xoap::SOAPElement* parent,
          const gem::utils::soap::Parameters &parameters );
          void extractCommandAttributes( xoap::MessageReference message,
          gem::utils::soap::Attributes &attributes );
          void extractParameters(        xoap::MessageReference message,
          gem::utils::soap::Parameters &parameters );
          void setResponseTimeout(       xoap::MessageReference message,
          const uint64_t timeoutInSec );
        */
      private:

      protected:
        GEMSOAPToolBox();
      };

    }  // namespace gem::utils::soap
  }  // namespace gem::utils
}  // namespace gem

#endif  // GEM_UTILS_SOAP_GEMSOAPTOOLBOX_H
