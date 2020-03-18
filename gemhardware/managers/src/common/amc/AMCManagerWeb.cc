// AMCManagerWeb.cc

#include <memory>

#include "xcept/tools.h"

#include "gem/hw/managers/amc/AMCManager.h"
#include "gem/hw/managers/amc/AMCManagerWeb.h"
#include "gem/hw/managers/amc/AMCMonitor.h"

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

/*To be filled in with the monitor page code*/
void gem::hw::amc::AMCManagerWeb::monitorPage(xgi::Input* in, xgi::Output* out)
{
  CMSGEMOS_DEBUG("AMCManagerWeb::monitorPage");

  *out << "    <div class=\"xdaq-tab-wrapper\">" << std::endl;
  *out << "      <div class=\"xdaq-tab\" title=\"DAQ Link Monitoring\" >"  << std::endl;
  // all monitored AMCs in one page, or separate tabs?
  /* let's have a summary of major parameters for all managed AMCs on this page,
     then have the card tab have the full information for every AMC
   */
  buildCardSummaryTable(in, out);
  *out << "      </div>" << std::endl;
  *out << "    </div>" << std::endl;
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
void gem::hw::amc::AMCManagerWeb::buildCardSummaryTable(xgi::Input* in, xgi::Output* out)
{
  *out << "      <table class=\"xdaq-table\">" << std::endl
       << cgicc::thead() << std::endl
       << cgicc::tr()    << std::endl // open
       << cgicc::th() << "Register" << cgicc::th() << std::endl;
  // loop over all managed AMCs and put AMCXX as the header
  for (unsigned int i = 0; i < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++i) {
    auto card = dynamic_cast<gem::hw::amc::AMCManager*>(p_gemFSMApp)->m_amcMonitors.at(i);
    if (card) {
      *out << cgicc::th() << card->getDeviceID() << cgicc::th() << std::endl;
    }
  }
  *out << cgicc::tr()    << std::endl // close
       << cgicc::thead() << std::endl
       << "        <tbody>" << std::endl;

  // loop over values to be monitored
  // L1A, CalPulse, Resync, BC0, Link Errors, Link FIFO status, Event builder status
  *out << "          <tr>"         << std::endl
       << "            <td>"         << std::endl
    //<< registerName   << std::endl
       << "            </td>"        << std::endl;

  // loop over AMCs to be monitored
  for (unsigned int i = 0; i < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++i) {
    auto card = dynamic_cast<gem::hw::amc::AMCManager*>(p_gemFSMApp)->m_amcMonitors.at(i);
    if (card) {
      *out << "            <td id=\"" << card->getDeviceID() << "\">" << std::endl
           << "            </td>"           << std::endl;
    }
  }
  *out << "          </tr>"    << std::endl;

  // close off the table
  *out << "        </tbody>" << std::endl
       << "      </table>" << std::endl;
}

/*To be filled in with the card page code*/
void gem::hw::amc::AMCManagerWeb::cardPage(xgi::Input* in, xgi::Output* out)
{
  CMSGEMOS_DEBUG("AMCManagerWeb::cardPage");
  // fill this page with the card views for the AMCManager
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  for (unsigned int i = 0; i < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++i) {
    auto card = dynamic_cast<gem::hw::amc::AMCManager*>(p_gemFSMApp)->m_amcMonitors.at(i);
    if (card) {
      *out << "<div class=\"xdaq-tab\" title=\"" << card->getDeviceID() << "\" >"  << std::endl;
      card->buildMonitorPage(out);
      *out << "</div>" << std::endl;
    }
  }
  *out << "</div>" << std::endl;
}

/*To be filled in with the card page code*/
void gem::hw::amc::AMCManagerWeb::registerDumpPage(xgi::Input* in, xgi::Output* out)
{
  CMSGEMOS_DEBUG("AMCManagerWeb::registerDumpPage");
  // dump registers for a given AMC and display
}

void gem::hw::amc::AMCManagerWeb::jsonUpdate(xgi::Input* in, xgi::Output* out)
{
  CMSGEMOS_DEBUG("AMCManagerWeb::jsonUpdate");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out << " { " << std::endl;
  for (unsigned int i = 0; i < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++i) {
    *out << "\"amc" << std::setw(2) << std::setfill('0') << (i+1) << "\"  : { " << std::endl;
    auto card = dynamic_cast<gem::hw::amc::AMCManager*>(p_gemFSMApp)->m_amcMonitors.at(i);
    if (card) {
      card->jsonUpdateItemSets(out);
    }
    // can't have a trailing comma for the last entry...
    if (i == (gem::base::GEMApplication::MAX_AMCS_PER_CRATE-1))
      *out << " }" << std::endl;
    else
      *out << " }," << std::endl;
  }
  *out << " } " << std::endl;
}
