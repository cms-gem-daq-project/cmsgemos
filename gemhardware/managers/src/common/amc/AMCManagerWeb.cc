// AMCManagerWeb.cc

#include <memory>

#include "xcept/tools.h"

#include "gem/hw/managers/amc/AMCManager.h"
#include "gem/hw/managers/amc/AMCManagerWeb.h"

#include "gem/hw/devices/exception/Exception.h"

gem::hw::amc::AMCManagerWeb::AMCManagerWeb(gem::hw::amc::AMCManager* amcApp) :
  gem::base::GEMWebApplication(amcApp)
{
  // default constructor
}

gem::hw::amc::AMCManagerWeb::~AMCManagerWeb()
{
  // default destructor
}

void gem::hw::amc::AMCManagerWeb::webDefault(xgi::Input* in, xgi::Output* out)
{
  if (p_gemFSMApp)
    CMSGEMOS_DEBUG("current state is" << dynamic_cast<gem::hw::amc::AMCManager*>(p_gemFSMApp)->getCurrentState());
  *out << cgicc::script().set("type", "text/javascript")
    .set("src", "/gemdaq/gemhardware/html/scripts/amc/amc.js")
       << cgicc::script() << std::endl;

  GEMWebApplication::webDefault(in, out);
}

/*To be filled in with the expert page code*/
void gem::hw::amc::AMCManagerWeb::expertPage(xgi::Input* in, xgi::Output* out)
{
  CMSGEMOS_DEBUG("AMCManagerWeb::expertPage");
  // fill this page with the expert views for the AMCManager
  *out << "    <div class=\"xdaq-tab-wrapper\">" << std::endl;
  *out << "      <div class=\"xdaq-tab\" title=\"Register dump page\"/>"  << std::endl;
  registerDumpPage(in, out);
  *out << "      </div>" << std::endl;
  *out << "    </div>" << std::endl;
}

/*To be filled in with the application page code*/
void gem::hw::amc::AMCManagerWeb::applicationPage(xgi::Input* in, xgi::Output* out)
{
  std::string cardURL = "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/cardPage";
  *out << "  <div class=\"xdaq-tab\" title=\"Card page\"/>"  << std::endl;
  cardPage(in, out);
  *out << "  </div>" << std::endl;
}

/*To be filled in with the card page code*/
void gem::hw::amc::AMCManagerWeb::registerDumpPage(xgi::Input* in, xgi::Output* out)
{
  CMSGEMOS_DEBUG("AMCManagerWeb::registerDumpPage");
  // dump registers for a given AMC and display
}

void gem::hw::amc::AMCManagerWeb::monitorPage(xgi::Input* in, xgi::Output* out)
{
  CMSGEMOS_DEBUG("AMCManagerWeb::monitorPage");
}

void gem::hw::amc::AMCManagerWeb::jsonUpdate(xgi::Input* in, xgi::Output* out)
{
  CMSGEMOS_DEBUG("AMCManagerWeb::jsonUpdate");
}
