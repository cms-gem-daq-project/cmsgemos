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
        static const int I2O_READOUT_NOTIFY;
        static const int I2O_READOUT_CONFIRM;
	
        //XDAQ_INSTANTIATOR();
        
        GEMReadoutApplication(xdaq::ApplicationStub *stub);
        
        virtual ~GEMReadoutApplication();
	
        //virtual void init();

        virtual void actionPerformed(xdata::Event& event);
        
        /** Recieve a message to update the scan parameters
         * @param   message
         * message should contain 
         *  - the scan type
         *  - the value(s) to store in the RunParams bits in the data format
         * @returns xoap::MessageReference
         xoap::MessageReference updateScanParameters(xoap::MessageReference message) throw (xoap::exception::Exception);
        */
        
      protected:
        
        // inspired by HCAL readout application
        /*
        virtual int readout(unsigned int expected, unsigned int* eventNumbers,
                            std::vector< ::toolbox::mem::Reference* >& data) = 0;
        */
        //state transitions, how many of these are necessary for the readout applications?
        // use dummy functions on unnecessary ones, as they are pure virtual in GEMFSMApplication
        virtual void initializeAction() /*throw (gem::base::exception::Exception)*/;
        virtual void configureAction()  /*throw (gem::base::exception::Exception)*/;
        virtual void startAction()      /*throw (gem::base::exception::Exception)*/;
        virtual void pauseAction()      /*throw (gem::base::exception::Exception)*/;
        virtual void resumeAction()     /*throw (gem::base::exception::Exception)*/;
        virtual void stopAction()       /*throw (gem::base::exception::Exception)*/;
        virtual void haltAction()       /*throw (gem::base::exception::Exception)*/;
        virtual void resetAction()      /*throw (gem::base::exception::Exception)*/;
	
        virtual void failAction(toolbox::Event::Reference e)
          throw (toolbox::fsm::exception::Exception); 
	
        virtual void resetAction(toolbox::Event::Reference e)
          throw (toolbox::fsm::exception::Exception);
        
        
        class GEMReadoutSettings {
        public:
          GEMReadoutSettings();
          void registerFields(xdata::Bag<GEMReadoutApplication::GEMReadoutSettings>* bag);
          
          //configuration parameters
          xdata::String runType;
          xdata::String fileName;
          xdata::String outputType;
          xdata::String outputLocation;
        }; 
        
        xdata::Bag<GEMReadoutSettings> m_readoutSettings;        
        xdata::String                  m_connectionFile;
        xdata::String                  m_deviceName;

      private:
        
      };
  } // namespace gem::base
} // namespace gem

#endif
