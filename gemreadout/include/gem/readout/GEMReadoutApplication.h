#ifndef GEM_READOUT_GEMREADOUTAPPLICATION_H
#define GEM_READOUT_GEMREADOUTAPPLICATION_H

#include <string>
#include <queue>

#include "i2o/i2o.h"

#include "toolbox/Task.h"
#include "toolbox/mem/Pool.h"
#include "toolbox/SyncQueue.h"

#include "xoap/MessageReference.h"
#include "xoap/Method.h"

#include "gem/base/GEMFSMApplication.h"

#include "gem/utils/GEMLogging.h"
#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

namespace gem {
  namespace readout {

    class GEMReadoutTask;

    class GEMReadoutApplication : public gem::base::GEMFSMApplication
      {
      public:
        static const int I2O_READOUT_NOTIFY;
        static const int I2O_READOUT_CONFIRM;

        struct ReadoutCommands {
          enum EReadoutCommands {
            CMD_STOP   = 1,
            CMD_START  = 2,
            CMD_PAUSE  = 3,
            CMD_RESUME = 4,
            CMD_EXIT   = 5
          } ReadoutCommands;
        };

        GEMReadoutApplication(xdaq::ApplicationStub *stub)
          throw (xdaq::exception::Exception);

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

        int readoutTask();

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

        virtual int readout(unsigned int expected, unsigned int* eventNumbers, std::vector< ::toolbox::mem::Reference* >& data) = 0;

        std::string m_outFileName;
        std::shared_ptr<toolbox::Task> m_task;
        toolbox::mem::Pool*            m_pool;
        toolbox::SyncQueue<int>        m_cmdQueue;

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

        xdata::Integer64 m_eventsReadout;
        xdata::Double    m_usecPerEvent;

        double m_usecUsed;

      private:

      };

    class GEMReadoutTask : public toolbox::Task {
    public:
    GEMReadoutTask(GEMReadoutApplication* app) : toolbox::Task("GEMReadoutTask")
        {
          //p_readoutApp = std::make_shared<GEMReadoutApplication>(app);
          p_readoutApp = app;
        }
      virtual int svc() { return p_readoutApp->readoutTask(); }
    private:
      //std::shared_ptr<GEMReadoutApplication> p_readoutApp;
      GEMReadoutApplication* p_readoutApp;
    };

  }  // namespace gem::readout
}  // namespace gem

#endif  //GEM_READOUT_GEMREADOUTAPPLICATION_H
