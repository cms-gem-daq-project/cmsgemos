// AMC13ManagerWeb.cc

#include "gem/hw/amc13/AMC13ManagerWeb.h"
#include "gem/hw/amc13/AMC13Manager.h"

#include "gem/hw/amc13/exception/Exception.h"

#include "xcept/tools.h"
#include "amc13/AMC13.hh"
#include "amc13/Status.hh"

gem::hw::amc13::AMC13ManagerWeb::AMC13ManagerWeb(gem::hw::amc13::AMC13Manager *amc13App) :
  gem::base::GEMWebApplication(amc13App)
{
  level = 2;
}

gem::hw::amc13::AMC13ManagerWeb::~AMC13ManagerWeb()
{
  //default destructor
}

void gem::hw::amc13::AMC13ManagerWeb::webDefault(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  if (p_gemFSMApp)
    CMSGEMOS_DEBUG("AMC13ManagerWeb::current state is" << dynamic_cast<gem::hw::amc13::AMC13Manager*>(p_gemFSMApp)->getCurrentState());

  *out << cgicc::script().set("type", "text/javascript")
    .set("src", "/gemdaq/gemhardware/html/scripts/amc13/amc13.js")
       << cgicc::script() << std::endl;

  GEMWebApplication::webDefault(in, out);
}

/*To be filled in with the monitor page code
 *right now it just prints out the status page that the AMC13 generates
 *in the future it will be nice to add other monitoring to a separate tab perhaps
 *need a way to have the content be dynamic with, e.g., AJAX
 */
void gem::hw::amc13::AMC13ManagerWeb::monitorPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("AMC13ManagerWeb::monitorPage");

  // process the form for the debug level, how do we set the default of 2 for the first load?
  *out << "    <div class=\"xdaq-tab-wrapper\">" << std::endl;
  *out << "      <div class=\"xdaq-tab\" title=\"AMC13 Status Page\" >"  << std::endl;
  CMSGEMOS_DEBUG("AMC13ManagerWeb::current level is "      << level);

  std::string method = toolbox::toString("/%s/setDisplayLevel", (p_gemApp->m_urn).c_str());
  *out << cgicc::form().set("method","POST")
    .set("name","amc13statusform")
    .set("id","amc13statusform")
    .set("action",method)
       << std::endl;

  *out << (level == 0 ?
           cgicc::input().set("type","radio").set("id","level0")
           .set("class","amc13statuslevel").set("name","amc13statuslevel").set("value","0").set("checked") :
           cgicc::input().set("type","radio").set("id","level0")
           .set("class","amc13statuslevel").set("name","amc13statuslevel").set("value","0"))
       << "version" << std::endl
       << (level == 1 ?
           cgicc::input().set("type","radio").set("id","level1")
           .set("class","amc13statuslevel").set("name","amc13statuslevel").set("value","1").set("checked") :
           cgicc::input().set("type","radio").set("id","level1")
           .set("class","amc13statuslevel").set("name","amc13statuslevel").set("value","1"))
       << "minimum" << std::endl
       << (level == 2 ?
           cgicc::input().set("type","radio").set("id","level2")
           .set("class","amc13statuslevel").set("name","amc13statuslevel").set("value","2").set("checked") :
           cgicc::input().set("type","radio").set("id","level2")
           .set("class","amc13statuslevel").set("name","amc13statuslevel").set("value","2"))
       << "default" << std::endl
       << (level == 3 ?
           cgicc::input().set("type","radio").set("id","level3")
           .set("class","amc13statuslevel").set("name","amc13statuslevel").set("value","3").set("checked") :
           cgicc::input().set("type","radio").set("id","level3")
           .set("class","amc13statuslevel").set("name","amc13statuslevel").set("value","3"))
       << "advanced" << std::endl
       << (level == 4 ?
           cgicc::input().set("type","radio").set("id","level4")
           .set("class","amc14statuslevel").set("name","amc14statuslevel").set("value","4").set("checked") :
           cgicc::input().set("type","radio").set("id","level4")
           .set("class","amc14statuslevel").set("name","amc14statuslevel").set("value","4"))
       << "maximum" << std::endl
       << (level == 99 ?
           cgicc::input().set("type","radio").set("id","level99")
           .set("class","amc13statuslevel").set("name","amc13statuslevel").set("value","99").set("checked") :
           cgicc::input().set("type","radio").set("id","level99")
           .set("class","amc13statuslevel").set("name","amc13statuslevel").set("value","99"))
       << "expert" << std::endl;

  *out << cgicc::form()
       << cgicc::br()
       << std::endl;

  *out << "        <span style=\"display:block;float:left\""
       << " name=\"amc13status\" id=\"amc13status\">"
       << std::endl;
  std::string currentState = dynamic_cast<gem::hw::amc13::AMC13Manager*>(p_gemFSMApp)->getCurrentState();
  if ( currentState != "Initial")
    try {
      if (dynamic_cast<gem::hw::amc13::AMC13Manager*>(p_gemFSMApp)->getAMC13Device()) {
        // amc13_status_ptr s(dynamic_cast<gem::hw::amc13::AMC13Manager*>(p_gemFSMApp)->getAMC13Device()->getStatus());
        ::amc13::Status* s(dynamic_cast<gem::hw::amc13::AMC13Manager*>(p_gemFSMApp)->getAMC13Device()->getStatus());
        s->SetHTML();
        s->Report(level,*out);
      } else {
        std::string msg = "Unable to obtain pointer to AMC13 device: " + currentState;
        CMSGEMOS_WARN("AMC13ManagerWeb:: " << msg);
        XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem, msg);
      }
    } catch (const gem::hw::amc13::exception::HardwareProblem& e) {
      CMSGEMOS_WARN("AMC13ManagerWeb::Unable to display the AMC13 status page: " << e.what());
    } catch (const std::exception& e) {
      CMSGEMOS_WARN("AMC13ManagerWeb::Unable to display the AMC13 status page: " << e.what());
    }

  *out << "        </span>" << std::endl;
  *out << "      </div>"    << std::endl;
  *out << "    </div>"      << std::endl;
}

/*To be filled in with the expert page code*/
void gem::hw::amc13::AMC13ManagerWeb::expertPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("AMC13ManagerWeb::expertPage");
}

void gem::hw::amc13::AMC13ManagerWeb::applicationPage(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("AMC13ManagerWeb::applicationPage");
}

void gem::hw::amc13::AMC13ManagerWeb::jsonUpdate(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("AMC13ManagerWeb::jsonUpdate");
  updateStatus(out);
}

void gem::hw::amc13::AMC13ManagerWeb::setDisplayLevel(xgi::Input *in)
  throw (xgi::exception::Exception)
{
  // parse the form for the debug level
  try {
    cgicc::Cgicc cgi(in);
    int radio_i       = cgi["amc13statuslevel"]->getIntegerValue();
    CMSGEMOS_DEBUG("AMC13ManagerWeb::radio button value is " << radio_i);
    level = static_cast<size_t>(radio_i);
    CMSGEMOS_DEBUG("AMC13ManagerWeb::setting AMC13 display status info to " << level);
  } catch (const xgi::exception::Exception& e) {
    level = 2;
    CMSGEMOS_WARN("AMC13ManagerWeb::Caught xgi::exception " << e.what());
    // XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception& e) {
    level = 2;
    CMSGEMOS_WARN("AMC13ManagerWeb::Caught std::exception " << e.what());
    // XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (...) {
    level = 2;
    CMSGEMOS_WARN("AMC13ManagerWeb::Caught unknown exception");
    // XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

void gem::hw::amc13::AMC13ManagerWeb::updateStatus(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "plain/text");
  std::string currentState = dynamic_cast<gem::hw::amc13::AMC13Manager*>(p_gemFSMApp)->getCurrentState();
  if ( currentState != "Initial")
    try {
      if (dynamic_cast<gem::hw::amc13::AMC13Manager*>(p_gemFSMApp)->getAMC13Device()) {
        // amc13_status_ptr s(dynamic_cast<gem::hw::amc13::AMC13Manager*>(p_gemFSMApp)->getAMC13Device()->getStatus());
        ::amc13::Status* s(dynamic_cast<gem::hw::amc13::AMC13Manager*>(p_gemFSMApp)->getAMC13Device()->getStatus());
        s->SetHTML();
        s->Report(level,*out);
      } else {
        std::string msg = "Unable to obtain pointer to AMC13 device: " + currentState;
        CMSGEMOS_WARN("AMC13ManagerWeb:: " << msg);
        XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem, msg);
      }
    } catch (const gem::hw::amc13::exception::HardwareProblem& e) {
      CMSGEMOS_WARN("AMC13ManagerWeb::Unable to display the AMC13 status page: " << e.what());
      *out << "AMC13ManagerWeb::Unable to display the AMC13 status page: " << e.what();
    } catch (const std::exception& e) {
      CMSGEMOS_WARN("AMC13ManagerWeb::Unable to display the AMC13 status page: " << e.what());
      *out << "AMC13ManagerWeb::Unable to display the AMC13 status page: " << e.what();
    }
}
