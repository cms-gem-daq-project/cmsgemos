// GLIBManagerWeb.cc

#include "gem/hw/glib/GLIBManagerWeb.h"
#include "gem/hw/glib/GLIBManager.h"

#include "gem/hw/glib/exception/Exception.h"

#include "xcept/tools.h"

gem::hw::glib::GLIBManagerWeb::GLIBManagerWeb(gem::hw::glib::GLIBManager* glibApp) :
  gem::base::GEMWebApplication(glibApp)
{
  
}

gem::hw::glib::GLIBManagerWeb::~GLIBManagerWeb()
{
  //default destructor
}

void gem::hw::glib::GLIBManagerWeb::webDefault(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  if (p_gemFSMApp)
    DEBUG("current state is" << dynamic_cast<gem::hw::glib::GLIBManager*>(p_gemFSMApp)->getCurrentState());
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  *out << "<div class=\"xdaq-tab\" title=\"GLIBManager Control Panel\" >"  << std::endl;
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
void gem::hw::glib::GLIBManagerWeb::monitorPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("monitorPage");
  //fill this page with the generic views for the GLIBManager
  //different tabs for certain functions
  *out << "monitorPage</br>" << std::endl;
}

/*To be filled in with the expert page code*/
void gem::hw::glib::GLIBManagerWeb::expertPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("expertPage");
  //fill this page with the expert views for the GLIBManager
  *out << "expertPage</br>" << std::endl;
}

/*To be filled in with the card page code*/
void gem::hw::glib::GLIBManagerWeb::cardPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("cardPage");
  //fill this page with the card views for the GLIBManager
  *out << "cardPage</br>" << std::endl;
}

