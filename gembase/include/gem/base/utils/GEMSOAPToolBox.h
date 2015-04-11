#ifndef gem_base_utils_GEMSOAPToolBox_h
#define gem_base_utils_GEMSOAPToolBox_h

//using the SOAP toolbox defined in the TCDS code base with extra functionality from the EMU codebase
#include <string>

#include "xoap/MessageReference.h"
#include "xercesc/util/XercesDefs.hpp"

namespace gem {
  namespace base {
    namespace utils {
      
      class GEMSOAPToolBox
      {
      public:
	//methods copied from tcds soap helper
	static xoap::MessageReference makeSoapReply(std::string const& command,
						    std::string const& response);
	static xoap::MessageReference makeSoapFaultReply(std::string const& faultString,
							 std::string const& faultCode="Server",
							 std::string const& faultActor="",
							 std::string const& detail="");
	static xoap::MessageReference makeFSMSoapReply(std::string const& event,
						       std::string const& state);
	
	static std::string extractFSMCommandName(xoap::MessageReference const& msg);
	
	//methods copied from emu/soap/toolbox
	/*
	xoap::MessageReference createMessage( const gem::base::utils::soap::QualifiedName &command, 
					      const gem::base::utils::soap::Parameters &parameters = gem::base::utils::soap::Parameters::none,
					      const gem::base::utils::soap::Attributes &attributes = gem::base::utils::soap::Attributes::none,
					      const vector<gem::base::utils::soap::Attachment> &attachments = gem::base::utils::soap::Attachment::none );
	
	void addAttachments(           xoap::MessageReference message,
				       const vector<gem::base::utils::soap::Attachment> &attachments );
	void addAttributes(            xoap::MessageReference message,
				       xoap::SOAPElement* element,
				       const gem::base::utils::soap::Attributes &attributes );
	void includeParameters(        xoap::MessageReference message,
				       xoap::SOAPElement* parent,
				       gem::base::utils::soap::Parameters &parameters );
	void includeParameters(        xoap::MessageReference message,
				       xoap::SOAPElement* parent,
				       const gem::base::utils::soap::Parameters &parameters );
	void extractCommandAttributes( xoap::MessageReference message,
				       gem::base::utils::soap::Attributes &attributes );
	void extractParameters(        xoap::MessageReference message,
				       gem::base::utils::soap::Parameters &parameters );
	void setResponseTimeout(       xoap::MessageReference message,
				       const uint64_t timeoutInSec );
	*/
      private:
	
      protected:
	GEMSOAPToolBox();
      };
      
    }//end namespace gem::base::util

  }//end namespace gem::base

}//end namespace gem

#endif 
