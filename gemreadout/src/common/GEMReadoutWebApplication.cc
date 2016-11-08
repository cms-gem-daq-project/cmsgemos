// GEMReadoutWebApplication.cc

#include "gem/readout/GEMReadoutWebApplication.h"

#include <memory>

#include "xcept/tools.h"

#include "gem/readout/GEMReadoutApplication.h"

#include "gem/readout/exception/Exception.h"

gem::readout::GEMReadoutWebApplication::GEMReadoutWebApplication(gem::readout::GEMReadoutApplication* readoutApp) :
  gem::base::GEMWebApplication(readoutApp)
{
  // default constructor
}

gem::readout::GEMReadoutWebApplication::~GEMReadoutWebApplication()
{
  // default destructor
}

void gem::readout::GEMReadoutWebApplication::webDefault(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  if (p_gemFSMApp)
    DEBUG("current state is" << dynamic_cast<gem::readout::GEMReadoutApplication*>(p_gemFSMApp)->getCurrentState());
  *out << cgicc::script().set("type", "text/javascript")
    .set("src", "/gemdaq/gemreadout/html/scripts/readout.js")
       << cgicc::script() << std::endl;

  GEMWebApplication::webDefault(in, out);
}

/*To be filled in with the monitor page code*/
void gem::readout::GEMReadoutWebApplication::monitorPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMReadoutWebApplication::monitorPage");
}

/*To be filled in with the expert page code*/
void gem::readout::GEMReadoutWebApplication::expertPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMReadoutWebApplication::expertPage");
}

/*To be filled in with the application page code*/
void gem::readout::GEMReadoutWebApplication::applicationPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMReadoutWebApplication::applicationPage");
  *out << "  <div class=\"xdaq-tab\" title=\"Application page\"/>"  << std::endl;
  *out << "  </div>" << std::endl;
}

void gem::readout::GEMReadoutWebApplication::jsonUpdate(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  DEBUG("GEMReadoutWebApplication::jsonUpdate");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out << " { " << std::endl;
  *out << " } " << std::endl;
}
