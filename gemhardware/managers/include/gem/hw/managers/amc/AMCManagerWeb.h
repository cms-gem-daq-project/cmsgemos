/** @file GLIBManagerWeb.h */

#ifndef GEM_HW_GLIB_GLIBMANAGERWEB_H
#define GEM_HW_GLIB_GLIBMANAGERWEB_H

#include "gem/base/GEMWebApplication.h"

namespace gem {
  namespace hw {
    namespace glib {

      class GLIBManager;

      class GLIBManagerWeb : public gem::base::GEMWebApplication
        {
          //friend class GLIBMonitor;
          friend class GLIBManager;

        public:
          GLIBManagerWeb(GLIBManager *glibApp);

          virtual ~GLIBManagerWeb();

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

          void dumpGLIBFIFO(xgi::Input *in, xgi::Output *out);

        private:
          size_t activeCard;

          //GLIBManagerWeb(GLIBManagerWeb const&);

        };

    }  // namespace gem::glib
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_GLIB_GLIBMANAGERWEB_H
