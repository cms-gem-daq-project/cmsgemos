// OptoHybridManagerWeb.cc

#include "gem/hw/optohybrid/OptoHybridManagerWeb.h"

#include <memory>

#include "xcept/tools.h"

#include "gem/hw/optohybrid/OptoHybridManager.h"
#include "gem/hw/optohybrid/OptoHybridMonitor.h"

#include "gem/hw/optohybrid/exception/Exception.h"

gem::hw::optohybrid::OptoHybridManagerWeb::OptoHybridManagerWeb(gem::hw::optohybrid::OptoHybridManager* optohybridApp) :
  gem::base::GEMWebApplication(optohybridApp)
{

}

gem::hw::optohybrid::OptoHybridManagerWeb::~OptoHybridManagerWeb()
{
  //default destructor
}

void gem::hw::optohybrid::OptoHybridManagerWeb::webDefault(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  if (p_gemFSMApp)
    CMSGEMOS_DEBUG("current state is" << dynamic_cast<gem::hw::optohybrid::OptoHybridManager*>(p_gemFSMApp)->getCurrentState());
  *out << cgicc::style().set("type", "text/css")
    .set("src", "/gemdaq/gemhardware/html/css/optohybrid/optohybrid.css")
       << cgicc::style() << std::endl;

  *out << cgicc::script().set("type", "text/javascript")
    .set("src", "/gemdaq/gemhardware/html/scripts/optohybrid/optohybrid.js")
       << cgicc::script() << std::endl;

  GEMWebApplication::webDefault(in, out);
}

/*To be filled in with the monitor page code*/
void gem::hw::optohybrid::OptoHybridManagerWeb::monitorPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_INFO("OptoHybridManagerWeb::monitorPage");
  //fill this page with the generic views for the OptoHybridManager
  //different tabs for certain functions
  *out << "    <div class=\"xdaq-tab-wrapper\">" << std::endl;
  *out << "monitorPage</br>" << std::endl;
  *out << "    </div>" << std::endl;
}

/*To be filled in with the expert page code*/
void gem::hw::optohybrid::OptoHybridManagerWeb::expertPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_INFO("OptoHybridManagerWeb::expertPage");
  //fill this page with the expert views for the OptoHybridManager
  *out << "expertPage</br>" << std::endl;
}

/*To be filled in with the application page code*/
void gem::hw::optohybrid::OptoHybridManagerWeb::applicationPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_INFO("OptoHybridManagerWeb::applicationPage");
  std::string cardURL = "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/boardPage";
  *out << "  <div class=\"xdaq-tab\" title=\"Board page\"/>"  << std::endl;
  boardPage(in, out);
  *out << "  </div>" << std::endl;
}

/*To be filled in with the card page code*/
void gem::hw::optohybrid::OptoHybridManagerWeb::boardPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_INFO("OptoHybridManagerWeb::boardPage");
  // fill this page with the card views for the OptoHybridManager
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  for (unsigned int i = 0; i < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++i) {
    for (unsigned int j = 0; j < gem::base::GEMApplication::MAX_OPTOHYBRIDS_PER_AMC; ++j) {
      auto card = dynamic_cast<gem::hw::optohybrid::OptoHybridManager*>(p_gemFSMApp)->m_optohybridMonitors.at(i).at(j);
      if (card) {
        *out << "<div class=\"xdaq-tab\" title=\"" << card->getDeviceID() << "\" >"  << std::endl;
        card->buildMonitorPage(out);
        *out << "</div>" << std::endl;
      }
    }
  }
  *out << "</div>" << std::endl;
}

void gem::hw::optohybrid::OptoHybridManagerWeb::jsonUpdate(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("OptoHybridManagerWeb::jsonUpdate");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out << " { " << std::endl;
  for (unsigned int i = 0; i < gem::base::GEMApplication::MAX_AMCS_PER_CRATE; ++i) {
    for (unsigned int j = 0; j < gem::base::GEMApplication::MAX_OPTOHYBRIDS_PER_AMC; ++j) {
      *out << "\"amcslot"   << std::setw(2) << std::setfill('0') << (i+1)
           << ".optohybrid" << std::setw(2) << std::setfill('0') << (j)
           << "\"  : { "    << std::endl;
      auto card = dynamic_cast<gem::hw::optohybrid::OptoHybridManager*>(p_gemFSMApp)->m_optohybridMonitors.at(i).at(j);
      if (card) {
        card->jsonUpdateItemSets(out);
      }
      // can't have a trailing comma for the last entry...
      if (i == (gem::base::GEMApplication::MAX_AMCS_PER_CRATE-1) &&
          j == (gem::base::GEMApplication::MAX_OPTOHYBRIDS_PER_AMC-1))
        *out << " }" << std::endl;
      else
        *out << " }," << std::endl;
    }
  }
  *out << " } " << std::endl;
}
