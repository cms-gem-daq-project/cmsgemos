/** @file AMCManagerWeb.h */

#ifndef GEM_HW_AMC_AMCMANAGERWEB_H
#define GEM_HW_AMC_AMCMANAGERWEB_H

#include "gem/base/GEMWebApplication.h"

namespace gem {
  namespace hw {
    namespace amc {

      class AMCManager;

      class AMCManagerWeb : public gem::base::GEMWebApplication
        {
          //friend class AMCMonitor;
          friend class AMCManager;

        public:
          AMCManagerWeb(AMCManager *amcApp);

          virtual ~AMCManagerWeb();

        protected:

          virtual void webDefault(  xgi::Input *in, xgi::Output *out);

          virtual void monitorPage(xgi::Input *in, xgi::Output *out);

          virtual void expertPage(xgi::Input *in, xgi::Output *out);

          virtual void applicationPage(xgi::Input *in, xgi::Output *out);

          virtual void jsonUpdate(xgi::Input *in, xgi::Output *out);

          void buildCardSummaryTable(xgi::Input *in, xgi::Output *out);

          void cardPage(xgi::Input *in, xgi::Output *out);

          void registerDumpPage(xgi::Input *in, xgi::Output *out);

          void fifoDumpPage(xgi::Input *in, xgi::Output *out);

          void dumpAMCFIFO(xgi::Input *in, xgi::Output *out);

        private:
          size_t activeCard;

          //AMCManagerWeb(AMCManagerWeb const&);

        };

    }  // namespace gem::amc
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_AMC_AMCMANAGERWEB_H
