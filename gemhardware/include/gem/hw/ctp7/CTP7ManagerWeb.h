/** @file CTP7ManagerWeb.h */

#ifndef GEM_HW_CTP7_CTP7MANAGERWEB_H
#define GEM_HW_CTP7_CTP7MANAGERWEB_H

#include <memory>

#include "gem/base/GEMWebApplication.h"

namespace gem {
  namespace hw {
    namespace ctp7 {

      class CTP7Manager;

      class CTP7ManagerWeb : public gem::base::GEMWebApplication
        {
          //friend class CTP7Monitor;
          friend class CTP7Manager;

        public:
          CTP7ManagerWeb(CTP7Manager *ctp7App);

          virtual ~CTP7ManagerWeb();

        protected:

          virtual void webDefault(  xgi::Input *in, xgi::Output *out )
            throw (xgi::exception::Exception);

          virtual void monitorPage(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);

          virtual void expertPage(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);

          virtual void applicationPage(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);

          virtual void jsonUpdate(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);

          void buildCardSummaryTable(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);

          void cardPage(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);

          void registerDumpPage(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);

          void fifoDumpPage(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);

          void dumpCTP7FIFO(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);

        private:
          size_t activeCard;

          //CTP7ManagerWeb(CTP7ManagerWeb const&);

        };

    }  // namespace gem::ctp7
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_CTP7_CTP7MANAGERWEB_H
