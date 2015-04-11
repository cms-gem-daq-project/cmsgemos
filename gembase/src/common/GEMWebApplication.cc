// GEMWebApplication.cc

#include "gem/base/GEMWebApplication.h"
#include "gem/base/GEMFSMApplication.h"
#include "gem/base/GEMApplication.h"
#include "gem/base/GEMMonitor.h"
#include "gem/base/GEMFSM.h"

#include "xgi/framework/UIManager.h"
#include "xgi/Input.h"
#include "xgi/Method.h"
#include "xgi/Output.h"

#include "xcept/tools.h"


//gem::base::GEMWebApplication::GEMWebApplication(xdaq::Application *gemApp, bool hasFSM)
gem::base::GEMWebApplication::GEMWebApplication(gem::base::GEMFSMApplication* gemFSMApp)
  throw (xdaq::exception::Exception) :
  gemLogger_(gemFSMApp->getApplicationLogger()),
  gemMonitorP_(gemFSMApp->getMonitor()),
  gemFSMAppP_(gemFSMApp),
  gemAppP_(gemFSMApp)
{
}

gem::base::GEMWebApplication::GEMWebApplication(gem::base::GEMApplication* gemApp)
  throw (xdaq::exception::Exception) :
  gemLogger_(gemApp->getApplicationLogger()),
  gemMonitorP_(gemApp->getMonitor()),
  gemFSMAppP_(0),
  gemAppP_(gemApp)
{
}

gem::base::GEMWebApplication::~GEMWebApplication()
{
  if (gemMonitorP_!=NULL)
    delete gemMonitorP_;
  if (gemFSMAppP_!=NULL)
    delete gemFSMAppP_;
  if (gemAppP_!=NULL)
    delete gemAppP_;

  gemMonitorP_ = NULL;
  gemFSMAppP_  = NULL;
  gemAppP_     = NULL;
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

/*To be filled in with the monitor page code*/
void gem::base::GEMWebApplication::monitorPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("monitorPage");
  *out << "monitorPage</br>" << std::endl;
}

/*To be filled in with the expert page code*/
void gem::base::GEMWebApplication::expertPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("expertPage");
  *out << "expertPage</br>" << std::endl;
}

/** FSM callbacks */
/*To be filled in with the startup (enable) routine*/
void gem::base::GEMWebApplication::webInitialize(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("webInitialize");
  if (gemFSMAppP_)
    try {
      gemFSMAppP_->fireEvent("Initialize");
    }
    catch( toolbox::fsm::exception::Exception& e ){
      XCEPT_RETHROW( xgi::exception::Exception, "Initialize failed", e );
    }
  *out << "webInitialize</br>" << std::endl;
}

/*To be filled in with the startup (enable) routine*/
void gem::base::GEMWebApplication::webEnable(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("webEnable");
  if (gemFSMAppP_)
    try {
      gemFSMAppP_->fireEvent("Enable");
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
  INFO("webConfigure");
  if (gemFSMAppP_)
    try{
      gemFSMAppP_->fireEvent("Configure");
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
  INFO("webStart");
  if (gemFSMAppP_)
    try{
      gemFSMAppP_->fireEvent("Start");
    }
    catch( toolbox::fsm::exception::Exception& e ){
      XCEPT_RETHROW( xgi::exception::Exception, "Start failed", e );
    }
  *out << "webStart</br>" << std::endl;
}

void gem::base::GEMWebApplication::webPause(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("webPause");
  if (gemFSMAppP_)
    try{
      gemFSMAppP_->fireEvent("Pause");
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
  INFO("webResume");
  if (gemFSMAppP_)
    try{
      gemFSMAppP_->fireEvent("Resume");
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
  INFO("webStop");
  if (gemFSMAppP_)
    try{
      gemFSMAppP_->fireEvent("Stop");
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
  INFO("webHalt");
  if (gemFSMAppP_)
    try{
      gemFSMAppP_->fireEvent("Halt");
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
  INFO("webReset");
  if (gemFSMAppP_)
    try{
      gemFSMAppP_->fireEvent("Reset");
    }
    catch( toolbox::fsm::exception::Exception& e ){
      XCEPT_RETHROW( xgi::exception::Exception, "Reset failed", e );
    }
  *out << "webReset</br>" << std::endl;
}
// End of file
