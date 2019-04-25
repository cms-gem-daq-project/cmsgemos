// GEMOnlineDBManagerWeb.cc

#include "gem/onlinedb/GEMOnlineDBManagerWeb.h"

#include <memory>

#include <jansson.h>

#include "xcept/tools.h"

#include "gem/onlinedb/ConfigurationManager.h"
#include "gem/onlinedb/GEMOnlineDBManager.h"
#include "gem/onlinedb/exception/Exception.h"

namespace /* anonymous */
{
    /// @brief Turns Jansson error codes into @c xgi::exception::Exception.
    void janssonCheck(int retcode)
    {
        if (retcode != 0) {
            XCEPT_RAISE(xgi::exception::Exception,
                        "Call to Jansson function failed");
        }
    }
    /**
     * @brief Checks that the provided @c json_t is not null.
     * @returns The provided pointer if it isn't null.
     * @throws xgi::exception::Exception if the pointer is null.
     */
    json_t *janssonCheckPtr(json_t *json)
    {
        if (json == nullptr) {
            XCEPT_RAISE(xgi::exception::Exception,
                        "Jansson could not allocate memory");
        }
        return json;
    }
} // anonymous namespace

gem::onlinedb::GEMOnlineDBManagerWeb::GEMOnlineDBManagerWeb(gem::onlinedb::GEMOnlineDBManager* dbApp) :
  gem::base::GEMWebApplication(dbApp)
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

void gem::onlinedb::GEMOnlineDBManagerWeb::monitorPage(xgi::Input* in, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  try {
    CMSGEMOS_DEBUG("GEMOnlineDBManagerWeb::monitorPage");

    *out <<  cgicc::div().set("class", "xdaq-tab-wrapper");

    auto lock = ConfigurationManager::makeReadLock();
    if (!lock.timed_lock(boost::posix_time::seconds(1))) {
      *out << "<div class=\"xdaq-error\">"
           << "Failed to acquire read lock"
           << "</div>";
    } else {
      auto &manager = ConfigurationManager::getManager(lock);

      *out << cgicc::h2("Sources");
      *out << cgicc::h3()
           << "Topology ("
           << cgicc::span().set("id", "topology-source-mode")
           << cgicc::span()
           << ")"
           << cgicc::h3();
      *out << "The topology of the system was loaded from the following sources:";
      *out << cgicc::ul().set("id", "topology-source-details") << cgicc::ul();
      *out << cgicc::h3()
           << "Objects ("
           << cgicc::span().set("id", "objects-source-mode")
           << cgicc::span()
           << ")"
           << cgicc::h3();
      *out << "Configuration objects were loaded from the following sources:";
      *out << cgicc::ul().set("id", "objects-source-details") << cgicc::ul();

      auto configStats = manager.getStatistics();

      *out << cgicc::h2("Statistics");
      *out << cgicc::p(
          "The table below counts how many configuration objects of each type "
          "are loaded.");
      *out << cgicc::table().set("class", "xdaq-table-vertical");
      *out << cgicc::tr()
           << cgicc::th("AMC 13")
           << cgicc::td().set("id", "amc13-count") << cgicc::td()
           << cgicc::tr();
      *out << cgicc::tr()
           << cgicc::th("AMC")
           << cgicc::td().set("id", "amc-count") << cgicc::td()
           << cgicc::tr();
      *out << cgicc::tr()
           << cgicc::th("OH")
           << cgicc::td().set("id", "oh-count") << cgicc::td()
           << cgicc::tr();
      *out << cgicc::tr()
           << cgicc::th("VFAT")
           << cgicc::td().set("id", "vfat-count") << cgicc::td()
           << cgicc::tr();
      *out << cgicc::table();
    }

    *out << cgicc::div();

  } catch (xcept::Exception &e) {
    XCEPT_RETHROW(xgi::exception::Exception, e.message(), e);
  } catch (const std::exception &e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
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
  // We use jsonUpdate for the basic statistics that are always visible in the
  // first tab. Additional information is retrieved from other sources.

  CMSGEMOS_DEBUG("GEMOnlineDBManagerWeb::jsonUpdate");
  out->getHTTPResponseHeader().addHeader("Content-Type", "application/json");

  json_auto_t *answer = janssonCheckPtr(json_object());

  auto lock = ConfigurationManager::makeReadLock();
  if (!lock.timed_lock(boost::posix_time::seconds(1))) {
    // Error
    janssonCheck(json_object_set_new(
      answer,
      "error",
      janssonCheckPtr(json_string("Could not acquire read lock in one second."))));
  } else {
    // Fill JSON
    auto &manager = ConfigurationManager::getManager(lock);

    {
      // Topology
      json_auto_t *topo = janssonCheckPtr(json_object());
      const char * const mode =
        manager.getTopologySource() == ConfigurationManager::Source::DB ? "Database" : "XML";
      janssonCheck(
        json_object_set_new(topo, "mode", janssonCheckPtr(json_string(mode))));

      json_auto_t *array = janssonCheckPtr(json_array());
      for (const auto &source : manager.getTopologySourceDetails()) {
        janssonCheck(json_array_append_new(array, json_string(source.data())));
      }
      janssonCheck(json_object_set(topo, "sources", array));

      janssonCheck(json_object_set(answer, "topology", topo));
    }

    {
      // Objects
      json_auto_t *obj = janssonCheckPtr(json_object());

      const char * const mode =
        manager.getObjectSource() == ConfigurationManager::Source::DB ? "Database" : "XML";
      janssonCheck(
        json_object_set_new(obj, "mode", janssonCheckPtr(json_string(mode))));

      json_auto_t *array = janssonCheckPtr(json_array());
      for (const auto &source : manager.getObjectSourceDetails()) {
        janssonCheck(json_array_append_new(array, json_string(source.data())));
      }
      janssonCheck(json_object_set(obj, "sources", array));

      janssonCheck(json_object_set(answer, "objects", obj));
    }

    {
      // Statistics
      json_auto_t *statsJson = janssonCheckPtr(json_object());
      auto stats = manager.getStatistics();

      janssonCheck(json_object_set_new(
        statsJson,
        "amc13-count",
        janssonCheckPtr(json_integer(stats.amc13Count))));
      janssonCheck(json_object_set_new(
        statsJson,
        "amc-count",
        janssonCheckPtr(json_integer(stats.amcCount))));
      janssonCheck(json_object_set_new(
        statsJson,
        "oh-count",
        janssonCheckPtr(json_integer(stats.ohCount))));
      janssonCheck(json_object_set_new(
        statsJson,
        "vfat-count",
        janssonCheckPtr(json_integer(stats.vfatCount))));
      janssonCheck(json_object_set(answer, "statistics", statsJson));
    }
  }

  // Write to stream
  char *encoded = json_dumps(answer, 0);
  *out << encoded;
  free(encoded);
}
