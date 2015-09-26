// OptoHybridManagerWeb.cc

#include "gem/hw/optohybrid/OptoHybridManagerWeb.h"
#include "gem/hw/optohybrid/OptoHybridManager.h"

#include "gem/hw/optohybrid/exception/Exception.h"

#include "xcept/tools.h"

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
    DEBUG("current state is" << dynamic_cast<gem::hw::optohybrid::OptoHybridManager*>(p_gemFSMApp)->getCurrentState());
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  *out << "<div class=\"xdaq-tab\" title=\"OptoHybridManager Control Panel\" >"  << std::endl;
  controlPanel(in,out);
  *out << "</div>" << std::endl;

  *out << "<div class=\"xdaq-tab\" title=\"Monitoring page\"/>"  << std::endl;
  monitorPage(in,out);
  *out << "</div>" << std::endl;

  std::string expURL = "/" + p_gemApp->getApplicationDescriptor()->getURN() + "/expertPage";
  *out << "<div class=\"xdaq-tab\" title=\"Expert page\"/>"  << std::endl;
  expertPage(in,out);
  *out << "</div>" << std::endl;
  *out << "</div>" << std::endl;
}

/*To be filled in with the monitor page code*/
void gem::hw::optohybrid::OptoHybridManagerWeb::monitorPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("monitorPage");
  //fill this page with the generic views for the OptoHybridManager
  //different tabs for certain functions
  *out << "monitorPage</br>" << std::endl;
}

/*To be filled in with the expert page code*/
void gem::hw::optohybrid::OptoHybridManagerWeb::expertPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("expertPage");
  //fill this page with the expert views for the OptoHybridManager
  *out << "expertPage</br>" << std::endl;
}

/*To be filled in with the card page code*/
void gem::hw::optohybrid::OptoHybridManagerWeb::cardPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("cardPage");
  //fill this page with the card views for the OptoHybridManager
  *out << "cardPage</br>" << std::endl;
}

