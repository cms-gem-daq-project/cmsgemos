#ifndef GEM_UTILS_SOAP_GEMSOAPTOOLBOX_H
#define GEM_UTILS_SOAP_GEMSOAPTOOLBOX_H

// using the SOAP toolbox defined in the TCDS code base with extra functionality from the EMU codebase
#include <string>
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

#include <gem/utils/exception/Exception.h>

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
          throw (gem::utils::exception::Exception);

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
          throw (gem::utils::exception::Exception);

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
          throw (gem::utils::exception::Exception);

         /**
         * @param parName Name of the parameter in the destination application info space
         * @param parType xsd type of the specified parameter
         * @param parValue parameter value (as string) to send in SOAP message
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
          throw (gem::utils::exception::Exception);

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

        static void sendAMC13Config(xdaq::ApplicationContext* appCxt,
                                    xdaq::ApplicationDescriptor* src,
                                    xdaq::ApplicationDescriptor* dest);

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
