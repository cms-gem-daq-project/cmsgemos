// GEMWebApplication.cc

#include "gem/base/GEMWebApplication.h"
#include "gem/base/GEMFSMApplication.h"
#include "gem/base/GEMApplication.h"
#include "gem/base/GEMMonitor.h"
#include "gem/base/GEMFSM.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"

#include "xgi/framework/UIManager.h"
#include "xgi/Input.h"
#include "xgi/Method.h"
#include "xgi/Output.h"

#include "xcept/tools.h"


//gem::base::GEMWebApplication::GEMWebApplication(xdaq::Application *gemApp, bool hasFSM)
gem::base::GEMWebApplication::GEMWebApplication(gem::base::GEMFSMApplication* gemFSMApp)
  throw (xdaq::exception::Exception) :
  m_gemLogger(gemFSMApp->getApplicationLogger()),
  p_gemMonitor(gemFSMApp->getMonitor()),
  p_gemFSMApp( gemFSMApp),
  p_gemApp(    gemFSMApp),
  b_is_working    (false),
  b_is_initialized(false),
  b_is_configured (false),
  b_is_running    (false),
  b_is_paused     (false)
{
  
}

gem::base::GEMWebApplication::GEMWebApplication(gem::base::GEMApplication* gemApp)
  throw (xdaq::exception::Exception) :
  m_gemLogger(gemApp->getApplicationLogger()),
  p_gemMonitor(gemApp->getMonitor()),
  p_gemFSMApp( NULL),
  p_gemApp(    gemApp),
  b_is_working    (false),
  b_is_initialized(false),
  b_is_configured (false),
  b_is_running    (false),
  b_is_paused     (false)
{
  
}

gem::base::GEMWebApplication::~GEMWebApplication()
{
  /*
    if (p_gemMonitor!=NULL)
    delete p_gemMonitor;
    if (p_gemFSMApp!=NULL)
    delete p_gemFSMApp;
    if (p_gemApp!=NULL)
    delete p_gemApp;
    
    p_gemMonitor = NULL;
    p_gemFSMApp  = NULL;
    p_gemApp     = NULL;
  */
}

void gem::base::GEMWebApplication::webRedirect(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  // std::string url = in->getenv("PATH_TRANSLATED");
  
  // cgicc::HTTPResponseHeader &header = out->getHTTPResponseHeader();
  
  // header.getStatusCode(303);
  // header.getReasonPhrase("See Other");
  // header.addHeader("Location",
  // 		   url.substr(0, url.find("/" + in->getenv("PATH_INFO"))));
  //change the status to halting and make sure the page displays this information
  std::string redURL = "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/Default";
  *out << "<meta http-equiv=\"refresh\" content=\"0;" << redURL << "\">" << std::endl;  
  //this->webDefault(in,out);
}

/*To be filled in with the monitor page code*/
void gem::base::GEMWebApplication::webDefault(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMWebApplication::webDefault");
  if (p_gemFSMApp)
    DEBUG("GEMWebApplication::current state is" << p_gemFSMApp->getCurrentState());
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;

  *out << "<div class=\"xdaq-tab\" title=\"Monitoring page\"/>"  << std::endl;
  monitorPage(in,out);
  *out << "</div>" << std::endl;

  *out << "<div class=\"xdaq-tab\" title=\"Expert page\"/>"  << std::endl;
  expertPage(in,out);
  *out << "</div>" << std::endl;

  *out << "</div>" << std::endl;
}

/*To be filled in with the control page code (only for FSM derived classes?*/
void gem::base::GEMWebApplication::controlPanel(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMWebApplication::controlPanel");
  // maybe the control part should only be displayed if the application is not supervised?
  if (p_gemFSMApp) {
    *out << cgicc::script().set("type","text/javascript")
      .set("src","/gemdaq/gembase/html/scripts/gemfsmwebcontrol.js")
         << cgicc::script() << std::endl;
    
    std::string updateLink = "/" + p_gemApp->m_urn + "/stateUpdate";
    *out << cgicc::script().set("type","text/javascript") << std::endl
         << "    updateStateTable( \"" << updateLink << "\" );" << std::endl
         << cgicc::script() << std::endl;

    try {
      std::string state = dynamic_cast<gem::base::GEMFSMApplication*>(p_gemFSMApp)->getCurrentState();
      DEBUG("GEMWebApplication::controlPanel:: current state " << state);

      *out << "<table class=\"xdaq-table\">" << std::endl
           << cgicc::thead() << std::endl
           << cgicc::tr()    << std::endl //open
           << cgicc::th()    << "Control" << cgicc::th() << std::endl
           << cgicc::th()    << "State"   << cgicc::th() << std::endl
           << cgicc::tr()    << std::endl //close
           << cgicc::thead() << std::endl 
      
           << "<tbody>" << std::endl
           << "<tr>"    << std::endl
           << "<td>"    << std::endl;
    
      *out << "<table class=\"xdaq-table\">" << std::endl;
      // Buttons                 | Initial             | Halted               | Configured           | Running | Paused |
      // Initialize Configure    | Configure invisible | Initialize invisible | Initialize invisible | All invisible | All invisible |
      // Start Stop Pause Resume | All invisible | All invisible        | Start visible        | Stop/Pause visible | Stop/Resume visible |
      // Halt Reset              | All invisible | All visible          | All visible          | All visible          | All visible          |

      *out << "<tr class=\"hide\" id=\"initconf\">" << std::endl
           << "<td>" << std::endl
           << "<button class=\"hide\" id=\"init\" onclick=\"gemFSMWebCommand(\'Initialize\',\'/"
           << p_gemApp->m_urn << "\')\">Initialize</button>"
           << cgicc::br() << std::endl
           << "</td>" << std::endl
           << "<td>" << std::endl
           << "<button class=\"hide\" id=\"conf\" onclick=\"gemFSMWebCommand(\'Configure\',\'/"
           << p_gemApp->m_urn << "\')\">Configure</button>"
           << cgicc::br() << std::endl
           << "</td>" << std::endl
           << "</tr>" << std::endl;

      *out << "<tr class=\"hide\" id=\"startstop\">" << std::endl
           << "<td>" << std::endl
           << "<button class=\"hide\" id=\"start\" onclick=\"gemFSMWebCommand(\'Start\',\'/"
           << p_gemApp->m_urn << "\')\">Start</button>"
           << "<button class=\"hide\" id=\"stop\" onclick=\"gemFSMWebCommand(\'Stop\',\'/"
           << p_gemApp->m_urn << "\')\">Stop</button>"
           << cgicc::br() << std::endl
           << "</td>" << std::endl
           << "<td>" << std::endl
           << "<button class=\"hide\" id=\"pause\" onclick=\"gemFSMWebCommand(\'Pause\',\'/"
           << p_gemApp->m_urn << "\')\">Pause</button>"
           << "<button class=\"hide\" id=\"resume\" onclick=\"gemFSMWebCommand(\'Resume\',\'/"
           << p_gemApp->m_urn << "\')\">Resume</button>"
           << cgicc::br() << std::endl
           << "</td>" << std::endl
           << "</tr>" << std::endl;

      *out << "<tr class=\"hide\" id=\"haltreset\">" << std::endl
           << "<td>" << std::endl
           << "<button class=\"hide\" id=\"halt\" onclick=\"gemFSMWebCommand(\'Halt\',\'/"
           << p_gemApp->m_urn << "\')\">Halt</button>"
           << cgicc::br() << std::endl
           << "</td>" << std::endl
           << "<td>" << std::endl
           << "<button class=\"hide\" id=\"reset\" onclick=\"gemFSMWebCommand(\'Reset\',\'/"
           << p_gemApp->m_urn << "\')\">Reset</button>"
           << cgicc::br() << std::endl
           << "</td>" << std::endl
           << "</tr>" << std::endl;

      *out << "</table>" << std::endl
           << "</br>"  << std::endl
           << "Last command was: "          << std::endl
           << "<div id=\"fsmdebug\"></div>" << std::endl
           << "</td>"  << std::endl
           << "<td>"  << std::endl
           << cgicc::h3().set("id","fsmState") 
        //change the colour to red if failed maybe
           << dynamic_cast<gem::base::GEMFSMApplication*>(p_gemFSMApp)->getCurrentState()
           << cgicc::h3() << std::endl
           << "</td>"     << std::endl
           << "</tr>"     << std::endl
           << "</tbody>"  << std::endl
           << "</table>"  << std::endl;
    } catch (const xgi::exception::Exception& e) {
      ERROR("GEMWebApplication::Something went wrong displaying web control panel(xgi): " << e.what());
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    } catch (const std::exception& e) {
      ERROR("GEMWebApplication::Something went wrong displaying web control panel(std): " << e.what());
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
  }//only when the GEMFSM has been created
}

/*To be filled in with the monitor page code*/
void gem::base::GEMWebApplication::monitorPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMWebApplication::monitorPage");
  *out << "monitorPage</br>" << std::endl;
  webRedirect(in,out);
}

/*To be filled in with the expert page code*/
void gem::base::GEMWebApplication::expertPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMWebApplication::expertPage");
  *out << "expertPage</br>" << std::endl;
  webRedirect(in,out);
}

/*To be filled in with the json update code*/
/*
void gem::base::GEMWebApplication::jsonUpdate(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMWebApplication::jsonUpdate");
}
*/
void gem::base::GEMWebApplication::jsonStateUpdate(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out << " {" << std::endl;
  *out << "   \"name\":\"fsmState\"" << ",\"value\": \"" 
       << dynamic_cast<gem::base::GEMFSMApplication*>(p_gemFSMApp)->getCurrentState()
       << "\"" << std::endl;
  *out << " }" << std::endl;
}

void gem::base::GEMWebApplication::jsonUpdate(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out << " { \n";
  auto monitor = p_gemFSMApp->p_gemMonitor;
  //if (p_gemMonitor) {
  if (monitor) {
    //p_gemMonitor->jsonUpdateItemSets(out);
    monitor->jsonUpdateItemSets(out);
  }
  *out << " } \n";
}

/** FSM callbacks */
/*To be filled in with the startup (enable) routine*/
void gem::base::GEMWebApplication::webInitialize(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMWebApplication::webInitialize begin");
  if (p_gemFSMApp) {
    DEBUG("GEMWebApplication::p_gemFSMApp non-zero");
    try {
      p_gemFSMApp->fireEvent("Initialize");
    } catch( toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Initialize failed", e );
    }
  }
  //DEBUG("GEMWebApplication::webInitialize end");
  //webRedirect(in,out);
}

/*To be filled in with the startup (enable) routine*/
void gem::base::GEMWebApplication::webEnable(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMWebApplication::webEnable");
  if (p_gemFSMApp) {
    DEBUG("GEMWebApplication::p_gemFSMApp non-zero");
    try {
      p_gemFSMApp->fireEvent("Enable");
    } catch( toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Enable failed", e );
    }
  }
  //webRedirect(in,out);
}

/*To be filled in with the configure routine*/
void gem::base::GEMWebApplication::webConfigure(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMWebApplication::webConfigure");
  if (p_gemFSMApp) {
    DEBUG("GEMWebApplication::p_gemFSMApp non-zero");
    try{
      p_gemFSMApp->fireEvent("Configure");
    } catch( toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Configure failed", e );
    }
  }
  //webRedirect(in,out);
}

/*To be filled in with the start routine*/
void gem::base::GEMWebApplication::webStart(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMWebApplication::webStart");
  if (p_gemFSMApp) {
    DEBUG("GEMWebApplication::p_gemFSMApp non-zero");
    try{
      p_gemFSMApp->fireEvent("Start");
    } catch( toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Start failed", e );
    }
  }
  //webRedirect(in,out);
}

void gem::base::GEMWebApplication::webPause(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMWebApplication::webPause");
  if (p_gemFSMApp) {
    DEBUG("GEMWebApplication::p_gemFSMApp non-zero");
    try{
      p_gemFSMApp->fireEvent("Pause");
    } catch( toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Pause failed", e );
    }
  }
  //webRedirect(in,out);
}

/*To be filled in with the resume routine*/
void gem::base::GEMWebApplication::webResume(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMWebApplication::webResume");
  if (p_gemFSMApp) {
    DEBUG("GEMWebApplication::p_gemFSMApp non-zero");
    try{
      p_gemFSMApp->fireEvent("Resume");
    } catch( toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Resume failed", e );
    }
  }
  //webRedirect(in,out);
}

/*To be filled in with the stop routine*/
void gem::base::GEMWebApplication::webStop(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMWebApplication::webStop");
  if (p_gemFSMApp) {
    DEBUG("GEMWebApplication::p_gemFSMApp non-zero");
    try{
      p_gemFSMApp->fireEvent("Stop");
    } catch( toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Stop failed", e );
    }
  }
  //webRedirect(in,out);
}

/*To be filled in with the halt routine*/
void gem::base::GEMWebApplication::webHalt(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMWebApplication::webHalt");
  if (p_gemFSMApp) {
    DEBUG("GEMWebApplication::p_gemFSMApp non-zero");
    try{
      p_gemFSMApp->fireEvent("Halt");
    } catch( toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Halt failed", e );
    }
  }
  //webRedirect(in,out);
}

/*To be filled in with the reset routine*/
void gem::base::GEMWebApplication::webReset(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMWebApplication::webReset");
  if (p_gemFSMApp) {
    DEBUG("GEMWebApplication::p_gemFSMApp non-zero");
    try{
      p_gemFSMApp->fireEvent("Reset");
    } catch( toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Reset failed", e );
    }
  }
  //webRedirect(in,out);
}

void gem::base::GEMWebApplication::buildCfgWebpage()
{
}

/** some generic static functions for web use, copied from ferol::WebServer */
std::string gem::base::GEMWebApplication::jsonEscape(std::string const& orig)
{   
  std::string::const_iterator it = orig.begin();
  std::string res;
  
  for ( it = orig.begin(); it != orig.end(); it++ ) {
    if ( ((*it) == '"') || ((*it) == '\\') ) {
      res.append( 1, '\\' );
      res.append( 1, *it );
    } else if(  ((*it) == '\n') ) {
      res.append("; ");
    } else if(  ((*it) == '/') ) {
      res.append("\\/");
    } else {
      res.append(1,*it);
    }
  }
  return res;
}

std::string gem::base::GEMWebApplication::htmlEscape(std::string const& orig)
{
  std::string::const_iterator it = orig.begin();
  std::string res;

  for ( it = orig.begin(); it != orig.end(); it++ ) {
    if ( (*it) == '"' ) {
      res.append( "&quot;" );
    } else {
      res.append(1,*it);
    }
  }
  
  size_t pos = 0;
  while ( (pos = res.find( "<br>", 0 )) != std::string::npos ) {
    res.replace( pos, 4, "\n" );
    pos = 0;
  }
  return res;
}
