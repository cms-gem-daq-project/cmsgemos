#ifndef gem_hw_glib_GLIBManagerWeb_h
#define gem_hw_glib_GLIBManagerWeb_h

#include <memory>

#include "gem/base/GEMWebApplication.h"

namespace gem {
  namespace hw {
    namespace glib {
      
      class GLIBManager;

      class GLIBManagerWeb : public gem::base::GEMWebApplication
        {
          //friend class GLIBMonitor;
          //friend class GLIBManager;

        public:
          GLIBManagerWeb(GLIBManager *glibApp);
	  
          virtual ~GLIBManagerWeb();

        protected:

          virtual void webDefault(  xgi::Input *in, xgi::Output *out )
            throw (xgi::exception::Exception);

          virtual void monitorPage(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);
          
          virtual void expertPage(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);
          
          virtual void jsonUpdate(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);
          
          void buildCardSummaryTable(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);
          
          void cardPage(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);
          
        private:
          size_t activeCard;
	  
          //GLIBManagerWeb(GLIBManagerWeb const&);
	  
        };

    } // namespace gem::glib
  } // namespace gem::hw
} // namespace gem

#endif
