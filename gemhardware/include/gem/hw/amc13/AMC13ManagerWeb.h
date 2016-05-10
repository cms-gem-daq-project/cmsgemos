#ifndef GEM_HW_AMC13_AMC13MANAGERWEB_H
#define GEM_HW_AMC13_AMC13MANAGERWEB_H

#include <memory>

#include "gem/base/GEMWebApplication.h"
//#include "gem/hw/amc13/AMC13Manager.h"

namespace gem {
  namespace hw {
    namespace amc13 {

      class AMC13Manager;

      class AMC13ManagerWeb : public gem::base::GEMWebApplication
        {
          //friend class AMC13Monitor;
          //friend class AMC13Manager;

        public:
          AMC13ManagerWeb(AMC13Manager *amc13App);

          virtual ~AMC13ManagerWeb();

        protected:

          virtual void webDefault(  xgi::Input *in, xgi::Output *out )
            throw (xgi::exception::Exception);

          virtual void monitorPage(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);

          virtual void expertPage(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);

        private:
          size_t level;
          //AMC13Manager *amc13ManagerP_;
          //AMC13ManagerWeb(AMC13ManagerWeb const&);

        };

    }  // namespace gem::hw::amc13
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_AMC13_AMC13MANAGERWEB_H
