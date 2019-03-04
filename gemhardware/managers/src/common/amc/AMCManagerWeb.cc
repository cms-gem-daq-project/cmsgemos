// GLIBManagerWeb.cc

#include "gem/hw/glib/GLIBManagerWeb.h"

#include <memory>

#include "xcept/tools.h"

#include "gem/hw/glib/GLIBManager.h"
#include "gem/hw/glib/GLIBMonitor.h"

#include "gem/hw/glib/exception/Exception.h"

gem::hw::glib::GLIBManagerWeb::GLIBManagerWeb(gem::hw::glib::GLIBManager* glibApp) :
  gem::base::GEMWebApplication(glibApp)
{
  // default constructor
}

gem::hw::glib::GLIBManagerWeb::~GLIBManagerWeb()
{
  // default destructor
}

void gem::hw::glib::GLIBManagerWeb::webDefault(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  if (p_gemFSMApp)
    CMSGEMOS_DEBUG("current state is" << dynamic_cast<gem::hw::glib::GLIBManager*>(p_gemFSMApp)->getCurrentState());
  *out << cgicc::script().set("type", "text/javascript")
    .set("src", "/gemdaq/gemhardware/html/scripts/glib/glib.js")
       << cgicc::script() << std::endl;

  GEMWebApplication::webDefault(in, out);
}

/*To be filled in with the monitor page code*/
void gem::hw::glib::GLIBManagerWeb::monitorPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("GLIBManagerWeb::monitorPage");

  *out << "    <div class=\"xdaq-tab-wrapper\">" << std::endl;
  *out << "      <div class=\"xdaq-tab\" title=\"DAQ Link Monitoring\" >"  << std::endl;
  // all monitored GLIBs in one page, or separate tabs?
  /* let's have a summary of major parameters for all managed GLIBs on this page,
     then have the card tab have the full information for every GLIB
   */
  buildCardSummaryTable(in, out);
  *out << "      </div>" << std::endl;
  *out << "    </div>" << std::endl;
}

/*To be filled in with the expert page code*/
void gem::hw::glib::GLIBManagerWeb::expertPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("GLIBManagerWeb::expertPage");
  // fill this page with the expert views for the GLIBManager
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
void gem::hw::glib::GLIBManagerWeb::applicationPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  std::string cardURL = "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/cardPage";
  *out << "  <div class=\"xdaq-tab\" title=\"Card page\"/>"  << std::endl;
  cardPage(in, out);
  *out << "  </div>" << std::endl;
}

/*To be filled in with the card page code*/
void gem::hw::glib::GLIBManagerWeb::buildCardSummaryTable(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  *out << "      <table class=\"xdaq-table\">" << std::endl
       << cgicc::thead() << std::endl
       << cgicc::tr()    << std::endl // open
       << cgicc::th() << "Register" << cgicc::th() << std::endl;
  // loop over all managed GLIBs and put GLIBXX as the header
  for (unsigned int i = 0; i < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++i) {
    auto card = dynamic_cast<gem::hw::glib::GLIBManager*>(p_gemFSMApp)->m_glibMonitors.at(i);
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

  // loop over GLIBs to be monitored
  for (unsigned int i = 0; i < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++i) {
    auto card = dynamic_cast<gem::hw::glib::GLIBManager*>(p_gemFSMApp)->m_glibMonitors.at(i);
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
void gem::hw::glib::GLIBManagerWeb::cardPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("GLIBManagerWeb::cardPage");
  // fill this page with the card views for the GLIBManager
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  for (unsigned int i = 0; i < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++i) {
    auto card = dynamic_cast<gem::hw::glib::GLIBManager*>(p_gemFSMApp)->m_glibMonitors.at(i);
    if (card) {
      *out << "<div class=\"xdaq-tab\" title=\"" << card->getDeviceID() << "\" >"  << std::endl;
      card->buildMonitorPage(out);
      *out << "</div>" << std::endl;
    }
  }
  *out << "</div>" << std::endl;
}

/*To be filled in with the card page code*/
void gem::hw::glib::GLIBManagerWeb::registerDumpPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("GLIBManagerWeb::registerDumpPage");
  // dump registers for a given GLIB and display
}

/*To be filled in with the card page code*/
void gem::hw::glib::GLIBManagerWeb::fifoDumpPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("GLIBManagerWeb::fifoDumpPage");
  // dump tracking fifo for given number of blocks
  //*out << cgicc::form() << std::endl;//.set("method","POST").set("action",);
  // input vs. button?
  // *out << cgicc::input().set("type","submit")       << std::endl;
  *out << cgicc::table().set("id","glibfifodumtable") << std::endl
       << cgicc::tr() << std::endl;

  *out << cgicc::td() << std::endl
       << cgicc::label("GLIB").set("for","glibID") << std::endl
       << cgicc::input().set("type","number").set("min","1").set("max","12").set("required value","1")
    .set("id","glibID").set("name","glibID")
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
    .set("id","dumpglibtrack")
    .set("onclick","dumpGLIBTrackingData(\'dumpGLIBFIFO\',\'/" + p_gemApp->m_urn + "\')")
       << std::endl << "Dump GLIB Tracking Data" << std::endl
       << cgicc::button() << std::endl
       << cgicc::td()     << std::endl;

  *out << cgicc::tr()    << std::endl
       << cgicc::table() << std::endl
       << cgicc::br()    << std::endl;

  *out << cgicc::textarea().set("cols","75").set("rows","50")
    .set("class","registerdumpbox").set("readonly")
    .set("name","glibtrackingdata").set("id","glibtrackingdata")
       << std::endl;
  *out << cgicc::textarea() << std::endl;
  //*out << cgicc::form()     << std::endl;
  *out << cgicc::br()       << std::endl;
}

void gem::hw::glib::GLIBManagerWeb::jsonUpdate(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("GLIBManagerWeb::jsonUpdate");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out << " { " << std::endl;
  for (unsigned int i = 0; i < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++i) {
    *out << "\"glib" << std::setw(2) << std::setfill('0') << (i+1) << "\"  : { " << std::endl;
    auto card = dynamic_cast<gem::hw::glib::GLIBManager*>(p_gemFSMApp)->m_glibMonitors.at(i);
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

void gem::hw::glib::GLIBManagerWeb::dumpGLIBFIFO(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("GLIBManagerWeb::dumpGLIBFIFO");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out << " { " << std::endl;
  for (unsigned int i = 0; i < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++i) {
    std::vector<uint32_t> dump = dynamic_cast<gem::hw::glib::GLIBManager*>(p_gemFSMApp)->dumpGLIBFIFO(i);
    *out << "\"glib" << std::setw(2) << std::setfill('0') << (i+1) << "FIFO\" : {" << std::endl;
    *out << "\"name\" : \"glibFIFODump\"," << std::endl
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
