// GEMWebApplication.cc

#include "gem/base/GEMWebApplication.h"

#include "xdaq/NamespaceURI.h"  // XDAQ_NS_URI
#include "xoap/Method.h"
#include "xoap/MessageFactory.h"  // createMessage()
#include "xoap/SOAPPart.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/SOAPSerializer.h"
#include "xoap/domutils.h"  // XMLCh2String()

#include "toolbox/fsm/FailedEvent.h"
#include "toolbox/task/WorkLoopFactory.h" // getWorkLoopFactory()
#include "xcept/tools.h"

#include "xoap/DOMParser.h"
#include "xoap/DOMParserFactory.h"
#include "xoap/domutils.h"
#include "xdata/soap/Serializer.h"

#include "xcept/tools.h"
#include "xdaq2rc/RcmsStateNotifier.h"
#include "toolbox/fsm/FailedEvent.h"

#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationDescriptorImpl.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/exception/Exception.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/XceptSerializer.h"
#include "xdata/Float.h" 
#include "xdata/Double.h" 
#include "xdata/Boolean.h"

//XDAQ_INSTANTIATOR_IMPL(gem::base::GEMWebApplication)

gem::base::GEMWebApplication::GEMWebApplication(xdaq::ApplicationStub *stub)
  throw (xdaq::exception::Exception) :
  gem::base::GEMApplication(stub),
  //xgi::framework::UIManager(this)
  xdaq::WebApplication(stub),
  gemWebLogger_(gemLogger_)
{
  xgi::framework::deferredbind(this, this, &gem::base::GEMWebApplication::Default,      "Default"  );
  xgi::framework::deferredbind(this, this, &gem::base::GEMWebApplication::webEnable,    "Enable"   );
  xgi::framework::deferredbind(this, this, &gem::base::GEMWebApplication::webConfigure, "Configure");
  xgi::framework::deferredbind(this, this, &gem::base::GEMWebApplication::webStart,     "Start"    );
  xgi::framework::deferredbind(this, this, &gem::base::GEMWebApplication::webPause,     "Pause"    );
  xgi::framework::deferredbind(this, this, &gem::base::GEMWebApplication::webResume,    "Resume"   );
  xgi::framework::deferredbind(this, this, &gem::base::GEMWebApplication::webStop,      "Stop"     );
  xgi::framework::deferredbind(this, this, &gem::base::GEMWebApplication::webHalt,      "Halt"     );
  xgi::framework::deferredbind(this, this, &gem::base::GEMWebApplication::webReset,     "Reset"    );

  LOG4CPLUS_INFO(gemWebLogger_, "GEMWebApplication");
}



void gem::base::GEMWebApplication::webRedirect(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  std::string url = in->getenv("PATH_TRANSLATED");
  
  cgicc::HTTPResponseHeader &header = out->getHTTPResponseHeader();
  
  header.getStatusCode(303);
  header.getReasonPhrase("See Other");
  header.addHeader("Location",
		   url.substr(0, url.find("/" + in->getenv("PATH_INFO"))));
}

/*To be filled in with the startup (enable) routine*/
void gem::base::GEMWebApplication::Default(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  LOG4CPLUS_INFO (gemWebLogger_, "Default");
  *out << "Default</br>" << std::endl;
}

/*To be filled in with the startup (enable) routine*/
void gem::base::GEMWebApplication::webEnable(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  LOG4CPLUS_INFO (gemWebLogger_, "webEnable");
  try{
    fireEvent("Enable");
  }
  catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Enable failed", e );
  }
  *out << "webEnable</br>" << std::endl;
}

/*To be filled in with the configure routine*/
void gem::base::GEMWebApplication::webConfigure(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  LOG4CPLUS_INFO (gemWebLogger_, "webConfigure");
  try{
    fireEvent("Configure");
  }
  catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Configure failed", e );
  }
  *out << "webConfigure</br>" << std::endl;
}

/*To be filled in with the start routine*/
void gem::base::GEMWebApplication::webStart(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  LOG4CPLUS_INFO (gemWebLogger_, "webStart");
  try{
    fireEvent("Start");
  }
  catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Start failed", e );
  }
  *out << "webStart</br>" << std::endl;
}

void gem::base::GEMWebApplication::webPause(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  LOG4CPLUS_INFO (gemWebLogger_, "webPause");
  try{
    fireEvent("Pause");
  }
  catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Pause failed", e );
  }
  *out << "webPause</br>" << std::endl;
}

/*To be filled in with the resume routine*/
void gem::base::GEMWebApplication::webResume(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  LOG4CPLUS_INFO (gemWebLogger_, "webResume");
  try{
    fireEvent("Resume");
  }
  catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Resume failed", e );
  }
  *out << "webResume</br>" << std::endl;
}

/*To be filled in with the stop routine*/
void gem::base::GEMWebApplication::webStop(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  LOG4CPLUS_INFO (gemWebLogger_, "webStop");
  try{
    fireEvent("Stop");
  }
  catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Stop failed", e );
  }
  *out << "webStop</br>" << std::endl;
}

/*To be filled in with the halt routine*/
void gem::base::GEMWebApplication::webHalt(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  LOG4CPLUS_INFO (gemWebLogger_, "webHalt");
  try{
    fireEvent("Halt");
  }
  catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Halt failed", e );
  }
  *out << "webHalt</br>" << std::endl;
}

/*To be filled in with the reset routine*/
void gem::base::GEMWebApplication::webReset(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  LOG4CPLUS_INFO (gemWebLogger_, "webReset");
  try{
    fireEvent("Reset");
  }
  catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Reset failed", e );
  }
  *out << "webReset</br>" << std::endl;
}

// End of file
