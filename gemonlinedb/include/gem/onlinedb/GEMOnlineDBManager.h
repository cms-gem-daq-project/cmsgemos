/** @file GEMOnlineDBManager.h */

#ifndef GEM_ONLINEDB_GEMONLINEDBMANAGER_H
#define GEM_ONLINEDB_GEMONLINEDBMANAGER_H

#include <string>
#include <vector>

#include "gem/base/GEMApplication.h"
// useful?
#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

#include "gem/onlinedb/GEMOnlineDBManagerWeb.h"
#include "gem/onlinedb/exception/Exception.h"

namespace gem {
  namespace onlinedb {

    class GEMOnlineDBManagerWeb;

    class GEMOnlineDBManager : public gem::base::GEMApplication
      {
        friend class GEMOnlineDBManagerWeb;

      public:
        XDAQ_INSTANTIATOR();

        GEMOnlineDBManager(xdaq::ApplicationStub* s);

        virtual ~GEMOnlineDBManager();

        virtual void init();

        virtual void actionPerformed(xdata::Event& event);

      protected:
      };
  }  // namespace gem::onlinedb
}  // namespace gem

#endif  // GEM_ONLINEDB_GEMONLINEDBMANAGER_H
