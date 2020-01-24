/** @file GEMOnlineDBManagerWeb.h */

#ifndef GEM_ONLINEDB_GEMONLINEDBMANAGERWEB_H
#define GEM_ONLINEDB_GEMONLINEDBMANAGERWEB_H

#include "gem/base/GEMWebApplication.h"

namespace gem {
  namespace onlinedb {

    class GEMOnlineDBManager;

    class GEMOnlineDBManagerWeb : public gem::base::GEMWebApplication
    {
      //friend class GLIBMonitor;
      friend class GEMOnlineDBManager;

    public:
      GEMOnlineDBManagerWeb(GEMOnlineDBManager *glibApp);

      virtual ~GEMOnlineDBManagerWeb();

    protected:

      virtual void webDefault(xgi::Input *in, xgi::Output *out);

      virtual void monitorPage(xgi::Input *in, xgi::Output *out);

      virtual void expertPage(xgi::Input *in, xgi::Output *out);

      virtual void applicationPage(xgi::Input *in, xgi::Output *out);

      virtual void jsonUpdate(xgi::Input *in, xgi::Output *out);

    };

  }  // namespace gem::onlinedb
}  // namespace gem

#endif  // GEM_ONLINEDB_GEMONLINEDBMANAGERWEB_H
