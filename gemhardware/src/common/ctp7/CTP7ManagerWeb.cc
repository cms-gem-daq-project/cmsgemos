// CTP7ManagerWeb.cc

#include "xcept/tools.h"

#include "gem/hw/ctp7/CTP7ManagerWeb.h"
#include "gem/hw/ctp7/CTP7Manager.h"
#include "gem/hw/ctp7/CTP7Monitor.h"

#include "gem/hw/ctp7/exception/Exception.h"

gem::hw::ctp7::CTP7ManagerWeb::CTP7ManagerWeb(gem::hw::ctp7::CTP7Manager* ctp7App) :
  gem::base::GEMWebApplication(ctp7App)
{
  // default constructor
}

gem::hw::ctp7::CTP7ManagerWeb::~CTP7ManagerWeb()
{
  // default destructor
}

void gem::hw::ctp7::CTP7ManagerWeb::webDefault(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  if (p_gemFSMApp)
    CMSGEMOS_DEBUG("current state is" << dynamic_cast<gem::hw::ctp7::CTP7Manager*>(p_gemFSMApp)->getCurrentState());
  *out << cgicc::script().set("type", "text/javascript")
    .set("src", "/gemdaq/gemhardware/html/scripts/ctp7/ctp7.js")
       << cgicc::script() << std::endl;

  GEMWebApplication::webDefault(in, out);
}

/*To be filled in with the monitor page code*/
void gem::hw::ctp7::CTP7ManagerWeb::monitorPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CTP7ManagerWeb::monitorPage");
  *out << "    <div class=\"xdaq-tab-wrapper\">" << std::endl;
  *out << "      <div class=\"xdaq-tab\" title=\"DAQ Link Monitoring\" >"  << std::endl;
  // all monitored CTP7s in one page, or separate tabs?
  /* let's have a summary of major parameters for all managed CTP7s on this page,
     then have the card tab have the full information for every CTP7
   */
  buildCardSummaryTable(in, out);
  *out << "      </div>" << std::endl;

  *out << "    </div>" << std::endl;
}

/*To be filled in with the expert page code*/
void gem::hw::ctp7::CTP7ManagerWeb::expertPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CTP7ManagerWeb::expertPage");
  // fill this page with the expert views for the CTP7Manager
  *out << "    <div class=\"xdaq-tab-wrapper\">" << std::endl;
  *out << "      <div class=\"xdaq-tab\" title=\"Register dump page\"/>"  << std::endl;
  registerDumpPage(in, out);
  *out << "      </div>" << std::endl;
  *out << "      <div class=\"xdaq-tab\" title=\"Data FIFO dump page\"/>"  << std::endl;
  fifoDumpPage(in, out);
  *out << "      </div>" << std::endl;
  *out << "    </div>" << std::endl;
}

/*To be filled in with the application page code*/
void gem::hw::ctp7::CTP7ManagerWeb::applicationPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  std::string cardURL = "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/cardPage";
  *out << "  <div class=\"xdaq-tab\" title=\"Card page\"/>"  << std::endl;
  cardPage(in, out);
  *out << "  </div>" << std::endl;
}

/*To be filled in with the card page code*/
void gem::hw::ctp7::CTP7ManagerWeb::buildCardSummaryTable(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  *out << "      <table class=\"xdaq-table\">" << std::endl
       << cgicc::thead() << std::endl
       << cgicc::tr()    << std::endl // open
       << cgicc::th() << "Register" << cgicc::th() << std::endl;
  // loop over all managed CTP7s and put CTP7XX as the header
  for (unsigned int i = 0; i < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++i) {
    auto card = dynamic_cast<gem::hw::ctp7::CTP7Manager*>(p_gemFSMApp)->m_ctp7Monitors[i];
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

  // loop over CTP7s to be monitored
  for (unsigned int i = 0; i < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++i) {
    auto card = dynamic_cast<gem::hw::ctp7::CTP7Manager*>(p_gemFSMApp)->m_ctp7Monitors[i];
    if (card) {
      std::stringstream tdid;
      tdid << card->getDeviceID();
      *out << "            <td id=\"" << tdid << "\">" << std::endl
           << "            </td>"           << std::endl;
    }
  }
  *out << "          </tr>"    << std::endl;

  // close off the table
  *out << "        </tbody>" << std::endl
       << "      </table>" << std::endl;
}

/*To be filled in with the card page code*/
void gem::hw::ctp7::CTP7ManagerWeb::cardPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CTP7ManagerWeb::cardPage");
  // fill this page with the card views for the CTP7Manager
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  for (unsigned int i = 0; i < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++i) {
    auto card = dynamic_cast<gem::hw::ctp7::CTP7Manager*>(p_gemFSMApp)->m_ctp7Monitors[i];
    if (card) {
      *out << "<div class=\"xdaq-tab\" title=\"" << card->getDeviceID() << "\" >"  << std::endl;
      card->buildMonitorPage(out);
      *out << "</div>" << std::endl;
    }
  }
  *out << "</div>" << std::endl;
}

/*To be filled in with the card page code*/
void gem::hw::ctp7::CTP7ManagerWeb::registerDumpPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CTP7ManagerWeb::registerDumpPage");
  // dump registers for a given CTP7 and display
}

/*To be filled in with the card page code*/
void gem::hw::ctp7::CTP7ManagerWeb::fifoDumpPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CTP7ManagerWeb::fifoDumpPage");
  // dump tracking fifo for given number of blocks
  //*out << cgicc::form() << std::endl;//.set("method","POST").set("action",);
  // input vs. button?
  // *out << cgicc::input().set("type","submit")       << std::endl;
  *out << cgicc::table().set("id","ctp7fifodumtable") << std::endl
       << cgicc::tr() << std::endl;

  *out << cgicc::td() << std::endl
       << cgicc::label("CTP7").set("for","ctp7ID") << std::endl
       << cgicc::input().set("type","number").set("min","1").set("max","12").set("required value","1")
    .set("id","ctp7ID").set("name","ctp7ID")
       << std::endl
       << cgicc::td() << std::endl;

  *out << cgicc::td() << std::endl
       << cgicc::label("GTX").set("for","gtxlink") << std::endl
       << cgicc::input().set("type","number").set("min","0").set("max","1").set("required value","0")
    .set("id","gtxlink").set("name","gtxlink")
       << std::endl
       << cgicc::td() << std::endl;

  *out << cgicc::td() << std::endl
       << cgicc::button().set("type","submit")
    .set("id","dumpctp7track")
    .set("onclick","dumpCTP7TrackingData(\'dumpCTP7FIFO\',\'/" + p_gemApp->m_urn + "\')")
       << std::endl << "Dump CTP7 Tracking Data" << std::endl
       << cgicc::button() << std::endl
       << cgicc::td()     << std::endl;

  *out << cgicc::tr()    << std::endl
       << cgicc::table() << std::endl
       << cgicc::br()    << std::endl;

  *out << cgicc::textarea().set("cols","75").set("rows","50")
    .set("class","registerdumpbox").set("readonly")
    .set("name","ctp7trackingdata").set("id","ctp7trackingdata")
       << std::endl;
  *out << cgicc::textarea() << std::endl;
  //*out << cgicc::form()     << std::endl;
  *out << cgicc::br()       << std::endl;
}

void gem::hw::ctp7::CTP7ManagerWeb::jsonUpdate(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CTP7ManagerWeb::jsonUpdate");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out << " { " << std::endl;
  for (unsigned int i = 0; i < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++i) {
    *out << "\"ctp7" << std::setw(2) << std::setfill('0') << (i+1) << "\"  : { " << std::endl;
    auto card = dynamic_cast<gem::hw::ctp7::CTP7Manager*>(p_gemFSMApp)->m_ctp7Monitors[i];
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

void gem::hw::ctp7::CTP7ManagerWeb::dumpCTP7FIFO(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("CTP7ManagerWeb::dumpCTP7FIFO");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out << " { " << std::endl;
  for (unsigned int i = 0; i < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++i) {
    std::vector<uint32_t> dump = dynamic_cast<gem::hw::ctp7::CTP7Manager*>(p_gemFSMApp)->dumpCTP7FIFO(i);
    *out << "\"ctp7" << std::setw(2) << std::setfill('0') << (i+1) << "FIFO\" : {" << std::endl;
    *out << "\"name\" : \"ctp7FIFODump\"," << std::endl
         << "\"value\" : [ " << std::endl << "\"";
    for (auto word = dump.begin(); word != dump.end(); ++word) {
      *out << "0x" << std::hex << std::setw(8) << std::setfill('0') << *word << std::dec;
      if (!(std::distance(word, dump.end()) == 1))
        *out << "\\n";
    }
    *out << "\"" << std::endl;
    *out << "]" << std::endl;
    // can't have a trailing comma for the last entry...
    if (i == (gem::base::GEMApplication::MAX_AMCS_PER_CRATE-1))
      *out << " }" << std::endl;
    else
      *out << " }," << std::endl;
  }
  *out << " } " << std::endl;
}
