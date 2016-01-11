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
  DEBUG("webDefault");
  if (p_gemFSMApp)
    DEBUG("current state is" << p_gemFSMApp->getCurrentState());
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
  DEBUG("controlPanel");
  // maybe the control part should only be displayed if the application is not supervised?
  if (p_gemFSMApp) {
    *out << cgicc::script().set("type","text/javascript")
      .set("src","/gemdaq/gembase/html/scripts/gemfsmwebcontrol.js")
         << cgicc::script() << std::endl;
    
    try {
      cgicc::Cgicc cgi(in);
      INFO("GEMWebApplication::processing form: cgi.getElements().size() = " << cgi.getElements().size());
      for (auto dbg = cgi.getElements().begin(); dbg != cgi.getElements().end(); ++dbg ) {
        INFO("Found cgi element: " << dbg->getName());
      }

      std::string fsmcommand = "";
      cgicc::form_iterator iter = cgi.getElement("fsmcommand");
      //cgicc::form_iterator iter = cgi.getElement("fsmControl");
      if ( iter == cgi.getElements().end() ) {
        WARN("GEMWebApplication::did not find fsmcommand while processing form");
      } else {
        fsmcommand = **iter;
        INFO("fsmcommand = " << fsmcommand);
        // perform the action
        if (fsmcommand == "Initialize")
          webInitialize(in,out);
        else if (fsmcommand == "Configure")
          webConfigure(in,out);
        else if (fsmcommand == "Start")
          webStart(in,out);
        else if (fsmcommand == "Stop")
          webStop(in,out);
        else if (fsmcommand == "Pause")
          webPause(in,out);
        else if (fsmcommand == "Resume")
          webResume(in,out);
        else if (fsmcommand == "Halt")
          webHalt(in,out);
        else if (fsmcommand == "Reset")
          webReset(in,out);
      }
    } catch (const xgi::exception::Exception& e) {
      ERROR("Something went wrong processing web control panel form(xgi): " << e.what());
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    } catch (const std::exception& e) {
      ERROR("Something went wrong processing web control panel form(std): " << e.what());
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
    try {
      std::string state = dynamic_cast<gem::base::GEMFSMApplication*>(p_gemFSMApp)->getCurrentState();
      INFO("controlPanel:: current state " << state);
      // update the page refresh, should definitely move this functionality to AJAX/JS
      if (!b_is_working && !b_is_running) {
      } else if (b_is_working) {
        cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
        head.addHeader("Refresh","2");
      } else if (b_is_running) {
        cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
        head.addHeader("Refresh","30");
      }
      *out << cgicc::form().set("method","POST").set("id","fsmControl") << std::endl
           << cgicc::input().set("type", "hidden").set("id","fsmcommand").set("name","fsmcommand").set("value","") << std::endl
           << cgicc::form() << std::endl;
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

      // the state may be dynamically changing, so perhaps the whole control panel table should be updated using AJAX
      // how is the best way to do this?
      *out << "<tr class=\"hide\" id=\"initconf\">" << std::endl
           << "<td>" << std::endl
           << "<button class=\"hide\" id=\"init\" onclick=\"gemFSMWebCommand(\'Initialize\')\">Initialize</button>"
           << cgicc::br() << std::endl
           << "</td>" << std::endl
           << "<td>" << std::endl
           << "<button class=\"hide\" id=\"conf\" onclick=\"gemFSMWebCommand(\'Configure\')\">Configure</button>"
           << cgicc::br() << std::endl
           << "</td>" << std::endl
           << "</tr>" << std::endl;

      *out << "<tr class=\"hide\" id=\"startstop\">" << std::endl
           << "<td>" << std::endl
           << "<button class=\"hide\" id=\"start\" onclick=\"gemFSMWebCommand(\'Start\')\">Start</button>"
           << "<button class=\"hide\" id=\"stop\" onclick=\"gemFSMWebCommand(\'Stop\')\">Stop</button>"
           << cgicc::br() << std::endl
           << "</td>" << std::endl
           << "<td>" << std::endl
           << "<button class=\"hide\" id=\"pause\" onclick=\"gemFSMWebCommand(\'Pause\')\">Pause</button>"
           << "<button class=\"hide\" id=\"resume\" onclick=\"gemFSMWebCommand(\'Resume\')\">Resume</button>"
           << cgicc::br() << std::endl
           << "</td>" << std::endl
           << "</tr>" << std::endl;

      *out << "<tr class=\"hide\" id=\"haltreset\">" << std::endl
           << "<td>" << std::endl
           << "<button class=\"hide\" id=\"halt\" onclick=\"gemFSMWebCommand(\'Halt\')\">Halt</button>"
           << cgicc::br() << std::endl
           << "</td>" << std::endl
           << "<td>" << std::endl
           << "<button class=\"hide\" id=\"reset\" onclick=\"gemFSMWebCommand(\'Reset\')\">Reset</button>"
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
      ERROR("Something went wrong displaying web control panel(xgi): " << e.what());
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    } catch (const std::exception& e) {
      ERROR("Something went wrong displaying web control panel(std): " << e.what());
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }

    std::string updateLink = "/" + p_gemApp->m_urn + "/stateUpdate";
    *out << cgicc::script().set("type","text/javascript") << std::endl
         << "    updateStateTable( \"" << updateLink << "\" );" << std::endl
         << cgicc::script() << std::endl;
  }//only when the GEMFSM has been created
}

/*To be filled in with the monitor page code*/
void gem::base::GEMWebApplication::monitorPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("monitorPage");
  *out << "monitorPage</br>" << std::endl;
  webRedirect(in,out);
}

/*To be filled in with the expert page code*/
void gem::base::GEMWebApplication::expertPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("expertPage");
  *out << "expertPage</br>" << std::endl;
  webRedirect(in,out);
}

/*To be filled in with the json update code*/
/*
void gem::base::GEMWebApplication::jsonUpdate(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("jsonUpdate");
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
  DEBUG("webInitialize begin");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
    try {
      p_gemFSMApp->fireEvent("Initialize");
    } catch( toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Initialize failed", e );
    }
  }
  //DEBUG("webInitialize end");
  //webRedirect(in,out);
}

/*To be filled in with the startup (enable) routine*/
void gem::base::GEMWebApplication::webEnable(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("webEnable");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
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
  DEBUG("webConfigure");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
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
  DEBUG("webStart");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
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
  DEBUG("webPause");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
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
  DEBUG("webResume");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
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
  DEBUG("webStop");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
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
  DEBUG("webHalt");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
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
  DEBUG("webReset");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
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
