// GEMOnlineDBManagerWeb.cc

#include "gem/onlinedb/GEMOnlineDBManagerWeb.h"

#include <memory>

#include "xcept/tools.h"

#include "gem/onlinedb/GEMOnlineDBManager.h"
#include "gem/onlinedb/exception/Exception.h"

gem::onlinedb::GEMOnlineDBManagerWeb::GEMOnlineDBManagerWeb(gem::onlinedb::GEMOnlineDBManager* glibApp) :
  gem::base::GEMWebApplication(glibApp)
{
  // default constructor
}

gem::onlinedb::GEMOnlineDBManagerWeb::~GEMOnlineDBManagerWeb()
{
  // default destructor
}

void gem::onlinedb::GEMOnlineDBManagerWeb::webDefault(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  *out << cgicc::script().set("type", "text/javascript")
    .set("src", "/gemdaq/gemonlinedb/html/scripts/gemonlinedb.js")
       << cgicc::script() << std::endl;

  GEMWebApplication::webDefault(in, out);
}

/*To be filled in with the monitor page code*/
void gem::onlinedb::GEMOnlineDBManagerWeb::monitorPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("GEMOnlineDBManagerWeb::monitorPage");

  *out << "    <div class=\"xdaq-tab-wrapper\">" << std::endl;
  *out << "    </div>" << std::endl;
}

/*To be filled in with the expert page code*/
void gem::onlinedb::GEMOnlineDBManagerWeb::expertPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("GEMOnlineDBManagerWeb::expertPage");
  // fill this page with the expert views for the GEMOnlineDBManager
  *out << "    <div class=\"xdaq-tab-wrapper\">" << std::endl;
  *out << "      <div class=\"xdaq-tab\" title=\"View selection page\"/>"  << std::endl;
  // selectViewPage(in, out);
  *out << "      </div>" << std::endl;
  *out << "      <div class=\"xdaq-tab\" title=\"XML Dump Page\"/>"  << std::endl;
  // dumpToXMLPage(in, out);
  *out << "      </div>" << std::endl;
  *out << "    </div>" << std::endl;
}

/*To be filled in with the application page code*/
void gem::onlinedb::GEMOnlineDBManagerWeb::applicationPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
}

void gem::onlinedb::GEMOnlineDBManagerWeb::jsonUpdate(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  CMSGEMOS_DEBUG("GEMOnlineDBManagerWeb::jsonUpdate");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");
  *out << " { " << std::endl;
  // fill JSON if needed
  *out << " } " << std::endl;
}

