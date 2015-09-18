#ifndef gem_hw_glib_GLIBManager_h
#define gem_hw_glib_GLIBManager_h

#include "uhal/uhal.hpp"

//#include "xgi/framework/Method.h"
//#include "cgicc/HTMLClasses.h"

#include "gem/base/GEMFSMApplication.h"
//#include "gem/hw/glib/GLIBSettings.h"

#include "gem/hw/glib/exception/Exception.h"

namespace gem {
  namespace base {
  }
  
  namespace hw {
    namespace glib {
      class HwGLIB;
      class GLIBManagerWeb;

      class GLIBManager : public gem::base::GEMFSMApplication
        {
	  
          friend class GLIBManagerWeb;

        public:
          XDAQ_INSTANTIATOR();
	  
          GLIBManager(xdaq::ApplicationStub* s);

          virtual ~GLIBManager();
	  
        protected:
	  
          virtual void preInit() throw (gem::base::exception::Exception);
          virtual void init()    throw (gem::base::exception::Exception);
          virtual void enable()  throw (gem::base::exception::Exception);
          virtual void disable() throw (gem::base::exception::Exception);
	  
          virtual void actionPerformed(xdata::Event& event);
	  
          //state transitions
          virtual void initializeAction() throw (gem::hw::glib::exception::Exception);
          virtual void configureAction()  throw (gem::hw::glib::exception::Exception);
          virtual void startAction()      throw (gem::hw::glib::exception::Exception);
          virtual void pauseAction()      throw (gem::hw::glib::exception::Exception);
          virtual void resumeAction()     throw (gem::hw::glib::exception::Exception);
          virtual void stopAction()       throw (gem::hw::glib::exception::Exception);
          virtual void haltAction()       throw (gem::hw::glib::exception::Exception);
          virtual void resetAction()      throw (gem::hw::glib::exception::Exception);
          //virtual void noAction()         throw (gem::hw::glib::exception::Exception); 
	
          virtual void failAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception); 
	
          virtual void resetAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);
	
	  
        private:
	  uint16_t parseAMCEnableList(std::string const&);
	  bool     isValidSlotNumber( std::string const&);

          uint16_t m_amcEnableMask;

          class GLIBInfo {
	    
          public:
            GLIBInfo();
            void registerFields(xdata::Bag<GLIBManager::GLIBInfo>* bag);
            xdata::Boolean present;
            xdata::Integer crateID;
            xdata::Integer slotID;
          };
	  
          mutable gem::utils::Lock m_deviceLock;//[MAX_AMCS_PER_CRATE];
	  
          HwGLIB* m_glibs[MAX_AMCS_PER_CRATE];
          xdata::Bag<GLIBInfo> m_glibInfo[MAX_AMCS_PER_CRATE];
          xdata::String        m_amcSlots;
        }; //end class GLIBManager
      
    }//end namespace gem::hw::glib
    
  }//end namespace gem::hw
  
}//end namespace gem

#endif
