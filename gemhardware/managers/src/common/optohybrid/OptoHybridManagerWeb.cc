// OptoHybridManagerWeb.cc

#include "gem/hw/managers/optohybrid/OptoHybridManagerWeb.h"

#include <memory>
#include <iomanip>

#include "xcept/tools.h"

#include "gem/hw/managers/optohybrid/OptoHybridManager.h"

#include "gem/hw/managers/optohybrid/exception/Exception.h"

gem::hw::optohybrid::OptoHybridManagerWeb::OptoHybridManagerWeb(gem::hw::optohybrid::OptoHybridManager* optohybridApp) :
  gem::base::GEMWebApplication(optohybridApp)
{

}

gem::hw::optohybrid::OptoHybridManagerWeb::~OptoHybridManagerWeb()
{
  //default destructor
}

void gem::hw::optohybrid::OptoHybridManagerWeb::webDefault(xgi::Input * in, xgi::Output * out)
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

/*To be filled in with the expert page code*/
void gem::hw::optohybrid::OptoHybridManagerWeb::expertPage(xgi::Input * in, xgi::Output * out)
{
  CMSGEMOS_INFO("OptoHybridManagerWeb::expertPage");
  //fill this page with the expert views for the OptoHybridManager
  *out << "expertPage</br>" << std::endl;
}

/*To be filled in with the application page code*/
void gem::hw::optohybrid::OptoHybridManagerWeb::applicationPage(xgi::Input* in, xgi::Output* out)
{
  CMSGEMOS_INFO("OptoHybridManagerWeb::applicationPage");
  std::string cardURL = "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/boardPage";
  *out << "  <div class=\"xdaq-tab\" title=\"Board page\"/>"  << std::endl;
  boardPage(in, out);
  *out << "  </div>" << std::endl;
}

void gem::hw::optohybrid::OptoHybridManagerWeb::monitorPage(xgi::Input * in, xgi::Output * out)
{
  CMSGEMOS_DEBUG("OptoHybridManagerWeb::monitorPage");
}

void gem::hw::optohybrid::OptoHybridManagerWeb::jsonUpdate(xgi::Input * in, xgi::Output * out)
{
  CMSGEMOS_DEBUG("OptoHybridManagerWeb::jsonUpdate");
}


