#ifndef gem_base_GEMReadoutApplication_h
#define gem_base_GEMReadoutApplication_h

#include "gem/base/GEMApplication.h"
#include "gem/base/GEMFSMApplication.h"

#include <toolbox/SyncQueue.h>
#include <i2o/i2o.h>
#include <toolbox/Task.h>
#include <toolbox/mem/Pool.h>

#include <xoap/MessageReference.h>
#include <xoap/Method.h>

#include <xdata/String.h>
#include <string>
#include <queue>

#include <gem/utils/GEMLogging.h>
#include <gem/utils/Lock.h>
#include <gem/utils/LockGuard.h>

namespace gem {
  namespace base {
    
    class GEMReadoutApplication : public gem::base::GEMFSMApplication
      {
      public:
        const int I2O_READOUT_NOTIFY=0x84;
        const int I2O_READOUT_CONFIRM=0x85;
	
        XDAQ_INSTANTIATOR();

        GEMReadoutApplication(xdaq::ApplicationStub *stub);
        
        virtual ~GEMReadoutApplication();
	
      protected:

        // inspired by HCAL readout application
        virtual int readout(unsigned int expected, unsigned int* eventNumbers,
                            std::vector< ::toolbox::mem::Reference* >& data) = 0;

        virtual void init();

        virtual void actionPerformed(xdata::Event& event);

        //state transitions, how many of these are necessary for the readout applications (removed pause/resume)?
        virtual void initializeAction() throw (gem::hw::glib::exception::Exception);
        virtual void configureAction()  throw (gem::hw::glib::exception::Exception);
        virtual void startAction()      throw (gem::hw::glib::exception::Exception);
        virtual void stopAction()       throw (gem::hw::glib::exception::Exception);
        virtual void haltAction()       throw (gem::hw::glib::exception::Exception);
        virtual void resetAction()      throw (gem::hw::glib::exception::Exception);
	
        virtual void failAction(toolbox::Event::Reference e)
          throw (toolbox::fsm::exception::Exception); 
	
        virtual void resetAction(toolbox::Event::Reference e)
          throw (toolbox::fsm::exception::Exception);
        
      private:
        class GEMReadoutSettings {
        public:
          GEMReadoutSettings();
          void registerFields(xdata::Bag<GEMReadoutApplication::GEMReadoutSettings>* bag);
          
          //configuration parameters
          xdata::String runType;
          xdata::String fileName;
          xdata::String outputLocation;
        }; 
      };
  } // namespace gem::base
} // namespace gem

#endif
