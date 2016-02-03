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
  //*out << "<div class=\"xdaq-tab\" title=\"GEM Supervisor Control Panel\" >"  << std::endl;
  
  // maybe the control part should only be displayed if the application is not supervised?
  if (p_gemFSMApp) {
    try {
      std::string state = dynamic_cast<gem::base::GEMFSMApplication*>(p_gemFSMApp)->getCurrentState();
      DEBUG("controlPanel:: current state " << state);
      ////update the page refresh 
      if (!b_is_working && !b_is_running) {
      } else if (b_is_working) {
        cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
        head.addHeader("Refresh","2");
      } else if (b_is_running) {
        cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
        head.addHeader("Refresh","30");
      }

      *out << "<table class=\"xdaq-table\">" << std::endl
           << cgicc::thead() << std::endl
           << cgicc::tr()    << std::endl //open
           << cgicc::th()    << "Control" << cgicc::th() << std::endl
           << cgicc::th()    << "State" << cgicc::th() << std::endl
           << cgicc::tr()    << std::endl //close
           << cgicc::thead() << std::endl 
      
           << "<tbody>" << std::endl
           << "<tr>"    << std::endl
           << "<td>"    << std::endl;
    
      *out << "<table class=\"xdaq-table\">" << std::endl;

      if (state == "Initial") {
        //send the initialize command
        *out << "<tr>" << std::endl << "<td>"    << std::endl
             << cgicc::form().set("method","POST").set("action", "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/Initialize") << std::endl;
        *out << cgicc::input().set("type", "submit")
          .set("name", "command").set("title", "Initialize GEM system.")
          .set("value", "Initialize") << std::endl;
        *out << cgicc::form() << std::endl
             << "</td>"       << std::endl
             << "</tr>"       << std::endl;
      } else {
        if (state == "Halted") {
          //this will allow the parameters to be set to the chip and scan routine
          *out << "<tr>" << std::endl << "<td colspan=\"2\">"    << std::endl
               << cgicc::form().set("method","POST").set("action", "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/Configure") << std::endl;
          *out << cgicc::input().set("type", "submit")
            .set("name", "command").set("title", "Configure FSM")
            .set("value", "Configure") << std::endl;
          *out << cgicc::form()        << std::endl
               << "</td>" << std::endl
               << "</tr>" << std::endl;
        } else if (state == "Configured") {
          //this will allow the parameters to be set to the chip and scan routine
          *out << "<tr>" << std::endl << "<td>"    << std::endl
               << cgicc::form().set("method","POST").set("action", "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/Configure") << std::endl;
          *out << cgicc::input().set("type", "submit")
            .set("name", "command").set("title", "Configure FSM")
            .set("value", "Configure") << std::endl;
          *out << cgicc::form()        << std::endl
               << "</td>" << std::endl;
          
          *out << "<td>"  << std::endl;
          *out << cgicc::form().set("method","POST").set("action", "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/Start") << std::endl;
          *out << cgicc::input().set("type", "submit")
            .set("name", "command").set("title", "Start FSM")
            .set("value", "Start") << std::endl;
          *out << cgicc::form()    << std::endl
               << "</td>" << std::endl
               << "</tr>" << std::endl;
        } else if (state == "Running") {
          *out << "<tr>" << std::endl << "<td>"    << std::endl
               << cgicc::form().set("method","POST").set("action", "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/Stop") << std::endl;
          *out << cgicc::input().set("type", "submit")
            .set("name", "command").set("title", "Stop FSM")
            .set("value", "Stop") << std::endl;
          *out << cgicc::form()   << std::endl
               << "</td>" << std::endl;
          
          *out << "<td>"  << std::endl;
          *out << cgicc::form().set("method","POST").set("action", "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/Pause") << std::endl;
          *out << cgicc::input().set("type", "submit")
            .set("name", "command").set("title", "Pause FSM")
            .set("value", "Pause") << std::endl;
          *out << cgicc::form()   << std::endl
               << "</td>" << std::endl
               << "</tr>" << std::endl;
        } else if (state == "Paused") {
          *out << "<tr>" << std::endl << "<td>"    << std::endl
               << cgicc::form().set("method","POST").set("action", "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/Stop") << std::endl;
          *out << cgicc::input().set("type", "submit")
            .set("name", "command").set("title", "Stop FSM")
            .set("value", "Stop") << std::endl;
          *out << cgicc::form()   << std::endl
               << "</td>" << std::endl;
          
          *out << "<td>"  << std::endl;
          *out << cgicc::form().set("method","POST").set("action", "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/Resume") << std::endl;
          *out << cgicc::input().set("type", "submit")
            .set("name", "command").set("title", "Resume FSM")
            .set("value", "Resume") << std::endl;
          *out << cgicc::form()   << std::endl
               << "</td>" << std::endl
               << "</tr>" << std::endl;
        }
        
        if (state == "Halted" ||
            state == "Configured" ||
            state == "Running" ||
            state == "Paused") {
          *out << cgicc::comment() << "end the main commands, now putting the halt/reset commands which should be possible all the time"
               << cgicc::comment() << cgicc::br() << std::endl;
          *out << "<tr>"    << std::endl
               << "<td>"    << std::endl;
          //always should have a halt/reset command?
          *out << cgicc::form().set("method","POST").set("action", "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/Halt") << std::endl;
          *out << cgicc::input().set("type", "submit")
            .set("name", "command").set("title", "Halt GEM system FSM.")
            .set("value", "Halt") << std::endl;
          *out << cgicc::form() << std::endl
               << "</td>" << std::endl;
          
          *out << "<td>"  << std::endl;
          *out << cgicc::form().set("method","POST").set("action", "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/Reset") << std::endl;
          *out << cgicc::input().set("type", "submit")
            .set("name", "command").set("title", "Reset GEM FSM.")
            .set("value", "Reset") << std::endl;
          *out << cgicc::form() << std::endl
               << "</td>" << std::endl
               << "</tr>" << std::endl;
        } else if (state == "Failed" || state == "Error") {
          *out << cgicc::form().set("method","POST").set("action", "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/Reset") << std::endl;
          *out << cgicc::input().set("type", "submit")
            .set("name", "command").set("title", "Reset GEM FSM.")
            .set("value", "Reset") << std::endl;
          *out << cgicc::form() << std::endl
               << "</td>" << std::endl
               << "</tr>" << std::endl;
        }
      }//end check on Initial vs Other
      *out << "</table>" << std::endl
           << "</td>"  << std::endl
           << "<td>"  << std::endl
           << cgicc::h3() 
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
    // try {
    //   p_gemFSMApp->fireEvent("Initialize");
    // } catch( toolbox::fsm::exception::Exception& e ) {
    //   XCEPT_RETHROW( xgi::exception::Exception, "Initialize failed", e );
    //}
  }
  DEBUG("webInitialize end");
  webRedirect(in,out);
}

/*To be filled in with the startup (enable) routine*/
void gem::base::GEMWebApplication::webEnable(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("webEnable");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
    //try {
    //  p_gemFSMApp->fireEvent("Enable");
    //} catch( toolbox::fsm::exception::Exception& e ) {
    //  XCEPT_RETHROW( xgi::exception::Exception, "Enable failed", e );
    //}
  }
  webRedirect(in,out);
}

/*To be filled in with the configure routine*/
void gem::base::GEMWebApplication::webConfigure(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("webConfigure");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
    //try{
    //  p_gemFSMApp->fireEvent("Configure");
    //} catch( toolbox::fsm::exception::Exception& e ) {
    //  XCEPT_RETHROW( xgi::exception::Exception, "Configure failed", e );
    //}
  }
  webRedirect(in,out);
}

/*To be filled in with the start routine*/
void gem::base::GEMWebApplication::webStart(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("webStart");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
    //try{
    //  p_gemFSMApp->fireEvent("Start");
    //} catch( toolbox::fsm::exception::Exception& e ) {
    //  XCEPT_RETHROW( xgi::exception::Exception, "Start failed", e );
    //}
  }
  webRedirect(in,out);
}

void gem::base::GEMWebApplication::webPause(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("webPause");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
    //try{
    //  p_gemFSMApp->fireEvent("Pause");
    //} catch( toolbox::fsm::exception::Exception& e ) {
    //  XCEPT_RETHROW( xgi::exception::Exception, "Pause failed", e );
    //}
  }
  webRedirect(in,out);
}

/*To be filled in with the resume routine*/
void gem::base::GEMWebApplication::webResume(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("webResume");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
    //try{
    //  p_gemFSMApp->fireEvent("Resume");
    //} catch( toolbox::fsm::exception::Exception& e ) {
    //  XCEPT_RETHROW( xgi::exception::Exception, "Resume failed", e );
    //}
  }
  webRedirect(in,out);
}

/*To be filled in with the stop routine*/
void gem::base::GEMWebApplication::webStop(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("webStop");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
    //try{
    //  p_gemFSMApp->fireEvent("Stop");
    //} catch( toolbox::fsm::exception::Exception& e ) {
    //  XCEPT_RETHROW( xgi::exception::Exception, "Stop failed", e );
    //}
  }
  webRedirect(in,out);
}

/*To be filled in with the halt routine*/
void gem::base::GEMWebApplication::webHalt(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("webHalt");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
    //try{
    //  p_gemFSMApp->fireEvent("Halt");
    //} catch( toolbox::fsm::exception::Exception& e ) {
    //  XCEPT_RETHROW( xgi::exception::Exception, "Halt failed", e );
    //}
  }
  webRedirect(in,out);
}

/*To be filled in with the reset routine*/
void gem::base::GEMWebApplication::webReset(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("webReset");
  if (p_gemFSMApp) {
    DEBUG("p_gemFSMApp non-zero");
    //try{
    //  p_gemFSMApp->fireEvent("Reset");
    //} catch( toolbox::fsm::exception::Exception& e ) {
    //  XCEPT_RETHROW( xgi::exception::Exception, "Reset failed", e );
    //}
  }
  webRedirect(in,out);
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
