#include "gem/supervisor/GEMSupervisorWeb.h"
#include "gem/supervisor/GEMSupervisor.h"
#include "gem/supervisor/GEMSupervisorMonitor.h"
#include "gem/base/GEMMonitor.h"
#include "xdata/InfoSpaceFactory.h"

#include "gem/base/utils/GEMInfoSpaceToolBox.h"

#include "gem/supervisor/exception/Exception.h"

#include "xcept/tools.h"


gem::supervisor::GEMSupervisorWeb::GEMSupervisorWeb(gem::supervisor::GEMSupervisor* gemSupervisorApp) :
  gem::base::GEMWebApplication(gemSupervisorApp)
{
  level = 5;
  //p_gemMonitor = dynamic_cast<gem::supervisor::GEMSupervisorMonitor*> gemSupervisorApp->getMonitor();
}

gem::supervisor::GEMSupervisorWeb::~GEMSupervisorWeb()
{
  //default destructor
  
}

void gem::supervisor::GEMSupervisorWeb::webDefault(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  if (p_gemFSMApp)
    DEBUG("current supervisor state is" << dynamic_cast<gem::supervisor::GEMSupervisor*>(p_gemFSMApp)->getCurrentState());
  *out << cgicc::script().set("type","text/javascript")
    .set("src","/gemdaq/gemsupervisor/html/scripts/gemsupervisor.js")
       << cgicc::script() << std::endl;
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  *out << "<div class=\"xdaq-tab\" title=\"GEM Supervisor Control Panel\" >"  << std::endl;
  controlPanel(in,out);
  
  *out << cgicc::br();
  
  // do this in an ajax way, so the page reload isn't necessary to get the state table update
  //displayManagedStateTable(in,out);
  dynamic_cast<gem::supervisor::GEMSupervisorMonitor*>(p_gemMonitor)->buildStateTable(out);
  *out << "</div>" << std::endl;

  *out << "<div class=\"xdaq-tab\" title=\"Monitoring page\"/>"  << std::endl;
  monitorPage(in,out);
  *out << "</div>" << std::endl;

  std::string expURL = "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/expertPage";
  *out << "<div class=\"xdaq-tab\" title=\"Expert page\"/>"  << std::endl;
  expertPage(in,out);
  *out << "</div>" << std::endl;
  *out << "</div>" << std::endl;

  std::string updateLink = "/" + p_gemApp->m_urn + "/jsonUpdate";
  *out << "<script type=\"text/javascript\">"            << std::endl
       << "    startUpdate( \"" << updateLink << "\" );" << std::endl
       << "</script>" << std::endl;
}

/*To be filled in with the monitor page code
 * need a way to have the content be dynamic with, e.g., AJAX
 */
void gem::supervisor::GEMSupervisorWeb::monitorPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("Rendering GEMSupervisorWeb monitorPage");
  DEBUG("current level is "      << level);
  if (level != 5) {
    try {
      cgicc::Cgicc cgi(in);
      int radio_i       = cgi["level"]->getIntegerValue();
      DEBUG("radio button value is " << radio_i);
      level = static_cast<size_t>(radio_i);
      DEBUG("setting GEMSupervisor display status info to " << level);
    } catch (const xgi::exception::Exception& e) {
      level = 2;
      WARN("Caught xgi::exception " << e.what());
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    } catch (const std::exception& e) {
      level = 2;
      WARN("Caught std::exception " << e.what());
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
  } else  {
    level = 2;
  }
  DEBUG("current level is "      << level);

  //form and control to set the display level of information
  std::string method = toolbox::toString("/%s/Default",p_gemFSMApp->getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::form().set("method","POST").set("action",method) << std::endl;
  
  *out << cgicc::section().set("style","display:inline-block;float:left") << std::endl
       << cgicc::fieldset().set("style","display:block;padding:5px;margin:5px;list-style-type:none;margin-bottom:5px;line-height:18px;padding:2px 5px;-webkit-border-radius:5px;-moz-border-radius:5px;border-radius:5px;border:medium outset #CCC;")
       << std::endl
       << cgicc::legend("GEMSupervisor")   << std::endl
       << cgicc::br()                      << std::endl
       << cgicc::div().set("align","left") << std::endl

       << cgicc::input().set("style","display:inline-block;margin-right:25px;margin-left:25px;float:center;-webkit-border-radius: 5px;-moz-border-radius: 5px;border-radius: 5px;border:medium outset #CCC;")
    .set("type","submit")
    .set("value","Set level")
    .set("name","setLevel") << std::endl

       << (level == 0 ?
           cgicc::input().set("type","radio").set("name","level").set("value","0").set("checked") :
           cgicc::input().set("type","radio").set("name","level").set("value","0"))
       << "version" << std::endl
       << (level == 1 ?
           cgicc::input().set("type","radio").set("name","level").set("value","1").set("checked") :
           cgicc::input().set("type","radio").set("name","level").set("value","1"))
       << "minimum" << std::endl
       << (level == 2 ?
           cgicc::input().set("type","radio").set("name","level").set("value","2").set("checked") :
           cgicc::input().set("type","radio").set("name","level").set("value","2"))
       << "default" << std::endl
       << (level == 3 ?
           cgicc::input().set("type","radio").set("name","level").set("value","3").set("checked") :
           cgicc::input().set("type","radio").set("name","level").set("value","3"))
       << "maximum" << std::endl

       << cgicc::br()     << std::endl
       << cgicc::div()    << std::endl
       << cgicc::span().set("style","display:block;float:left") << std::endl;

  *out << cgicc::span()     << std::endl
       << cgicc::fieldset() << std::endl
       << cgicc::section()  << std::endl
       << cgicc::form()     << std::endl;
}

/*To be filled in with the expert page code*/
void gem::supervisor::GEMSupervisorWeb::expertPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  DEBUG("rendering GEMSupervisorWeb expertPage");
  //should put all this style information into a common CSS sheet
  *out << cgicc::section().set("style","display:inline-block;float:left") << std::endl
       << cgicc::fieldset().set("style","style=\"display:block\";padding:5px;margin:5px;list-style-type:none;margin-bottom:5px;line-height:18px;padding:2px 5px;-webkit-border-radius:5px;-moz-border-radius:5px;border-radius:5px;border:medium outset #CCC;")
       << std::endl
       << cgicc::legend("GEM Supervisor expert page")    << std::endl
       << cgicc::br()       << std::endl
       << cgicc::span()     << std::endl
       << cgicc::fieldset() << std::endl
       << cgicc::section()  << std::endl;
}

void gem::supervisor::GEMSupervisorWeb::displayManagedStateTable(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  try {
    std::vector<xdaq::ApplicationDescriptor*> managedApps =
      dynamic_cast<gem::supervisor::GEMSupervisor*>(p_gemFSMApp)->getSupervisedAppDescriptors();
    *out << "<table class=\"xdaq-table\">" << std::endl
         << cgicc::thead() << std::endl
         << cgicc::tr()    << std::endl //open
         << cgicc::th()    << "Application Class (instance)" << cgicc::th() << std::endl
         << cgicc::th()    << "State" << cgicc::th() << std::endl
         << cgicc::tr()    << std::endl //close
         << cgicc::thead() << std::endl 
      
         << "<tbody>" << std::endl;
    
    for (auto managedApp = managedApps.begin(); managedApp != managedApps.end(); ++managedApp) {
      *out << "<tr>"  << std::endl
           << "<td>"  << std::endl
           << cgicc::h3() ;
      //<< dynamic_cast<gem::base::GEMFSMApplication*>(*managedApp)->getURN()
      std::string classname = (*managedApp)->getClassName();
      INFO("managed class name is " << classname);
      *out << classname;
      *out << "(";
      unsigned int instance = (*managedApp)->getInstance();
      INFO("managed class instance is " << instance);
      *out << instance;
      *out << ")"
           << cgicc::h3() << std::endl
           << "</td>"     << std::endl
           << "<td>"      << std::endl
           << cgicc::h3();
      
      INFO("trying to get the FSM class object for object " << std::hex << *managedApp << std::dec);
      std::string classstate
        = gem::base::utils::GEMInfoSpaceToolBox::getString(xdata::getInfoSpaceFactory()->get((*managedApp)->getURN()),"FSMState");
      *out << classstate;
      INFO("managed class FSM state is " << classstate);
      *out << cgicc::h3() << std::endl
           << "</td>"     << std::endl
           << "</tr>"     << std::endl;
    }
    *out << "</tbody>"  << std::endl
         << "</table>"  << std::endl;
  } catch (const xgi::exception::Exception& e) {
    INFO("Something went wrong displaying managed application state table(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception& e) {
    INFO("Something went wrong displaying managed application state table(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

void gem::supervisor::GEMSupervisorWeb::jsonUpdate(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  this->gem::base::GEMWebApplication::jsonUpdate(in,out);
  //out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  
/*
  *out << " { \n";
  auto monitor = p_gemFSMApp->p_gemMonitor;
  //if (p_gemMonitor) {
  if (monitor) {
    //p_gemMonitor->jsonUpdateItemSets(out);
    monitor->jsonUpdateItemSets(out);
  }
  *out << " } \n";
*/
}
