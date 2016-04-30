// GLIBManagerWeb.cc

#include "xcept/tools.h"

#include "gem/hw/glib/GLIBManagerWeb.h"
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
    DEBUG("current state is" << dynamic_cast<gem::hw::glib::GLIBManager*>(p_gemFSMApp)->getCurrentState());
  *out << cgicc::script().set("type", "text/javascript")
    .set("src", "/gemdaq/gemhardware/html/scripts/glib/glib.js")
       << cgicc::script() << std::endl;
  
  GEMWebApplication::webDefault(in, out);
}

/*To be filled in with the monitor page code*/
void gem::hw::glib::GLIBManagerWeb::monitorPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  INFO("monitorPage");
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  *out << "<div class=\"xdaq-tab\" title=\"DAQ Link Monitoring\" >"  << std::endl;
  // all monitored GLIBs in one page, or separate tabs?
  /* let's have a summary of major parameters for all managed GLIBs on this page,
     then have the card tab have the full information for every GLIB
   */
  buildCardSummaryTable(in, out);
  *out << "</div>" << std::endl;
  
  std::string cardURL = "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/cardPage";
  *out << "<div class=\"xdaq-tab\" title=\"Card page\"/>"  << std::endl;
  cardPage(in, out);
  *out << "</div>" << std::endl;
  
  *out << "</div>" << std::endl;
}

/*To be filled in with the expert page code*/
void gem::hw::glib::GLIBManagerWeb::expertPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  INFO("expertPage");
  // fill this page with the expert views for the GLIBManager
  *out << "expertPage</br>" << std::endl;
}

/*To be filled in with the card page code*/
void gem::hw::glib::GLIBManagerWeb::buildCardSummaryTable(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  *out << "<table class=\"xdaq-table\">" << std::endl
       << cgicc::thead() << std::endl
       << cgicc::tr()    << std::endl // open
       << cgicc::th() << "Register" << cgicc::th() << std::endl;
  // loop over all managed GLIBs and put GLIBXX as the header
  for (unsigned int i = 0; i < gem::base::GEMFSMApplication::MAX_AMCS_PER_CRATE; ++i) {
    auto card = dynamic_cast<gem::hw::glib::GLIBManager*>(p_gemFSMApp)->m_glibMonitors[i];
    if (card) {
      *out << cgicc::th() << card->getDeviceID() << cgicc::th() << std::endl;
    }
  }
  *out << cgicc::tr()    << std::endl // close
       << cgicc::thead() << std::endl 
       << "<tbody>" << std::endl;

  // loop over values to be monitored
  // L1A, CalPulse, Resync, BC0, Link Errors, Link FIFO status, Event builder status
  *out << "<tr>"         << std::endl
       << "<td>"         << std::endl
    //<< registerName   << std::endl
       << "</td>"        << std::endl;
  
  // loop over GLIBs to be monitored
  for (unsigned int i = 0; i < gem::base::GEMFSMApplication::MAX_AMCS_PER_CRATE; ++i) {
    auto card = dynamic_cast<gem::hw::glib::GLIBManager*>(p_gemFSMApp)->m_glibMonitors[i];
    if (card) {
      std::stringstream tdid;
      tdid << card->getDeviceID();
      *out << "<td id=\"" << tdid << "\">" << std::endl
           << "</td>"           << std::endl;
    }
  }
  *out << "</tr>"    << std::endl;
  
  // close off the table
  *out << "</tbody>" << std::endl
       << "</table>" << std::endl;
}

/*To be filled in with the card page code*/
void gem::hw::glib::GLIBManagerWeb::cardPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  INFO("cardPage");
  // fill this page with the card views for the GLIBManager
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  for (unsigned int i = 0; i < gem::base::GEMFSMApplication::MAX_AMCS_PER_CRATE; ++i) {
    auto card = dynamic_cast<gem::hw::glib::GLIBManager*>(p_gemFSMApp)->m_glibMonitors[i];
    if (card) {
      *out << "<div class=\"xdaq-tab\" title=\"" << card->getDeviceID() << "\" >"  << std::endl;
      card->buildMonitorPage(out);
      *out << "</div>" << std::endl;
    }
  }
  *out << "</div>" << std::endl;
}

void gem::hw::glib::GLIBManagerWeb::jsonUpdate(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  DEBUG("GLIBManagerWeb::jsonUpdate");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out << " { " << std::endl;
  for (unsigned int i = 0; i < gem::base::GEMFSMApplication::MAX_AMCS_PER_CRATE; ++i) {
    *out << "\"glib" << std::setw(2) << std::setfill('0') << (i+1) << "\"  : { " << std::endl;
    auto card = dynamic_cast<gem::hw::glib::GLIBManager*>(p_gemFSMApp)->m_glibMonitors[i];
    if (card) {
      card->jsonUpdateItemSets(out);
    }
    // can't have a trailing comma for the last entry...
    if (i == (gem::base::GEMFSMApplication::MAX_AMCS_PER_CRATE-1))
      *out << " }" << std::endl;
    else
      *out << " }," << std::endl;
  }
  *out << " } " << std::endl;
}
