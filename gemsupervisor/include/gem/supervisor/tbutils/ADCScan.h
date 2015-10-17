#ifndef gem_supervisor_tbutils_ADCScan_h
#define gem_supervisor_tbutils_ADCScan_h

#include <map>
#include <string>

#include "xdaq/WebApplication.h"
#include "xgi/Method.h"

#include "xgi/framework/Method.h"
#include "cgicc/HTMLClasses.h"

#include "xdaq/NamespaceURI.h"
#include "xoap/Method.h"
#include "xoap/MessageFactory.h"

#include "log4cplus/logger.h"

#include "toolbox/Event.h"
#include "toolbox/fsm/AsynchronousFiniteStateMachine.h"
#include "toolbox/fsm/FailedEvent.h"
#include "toolbox/task/WorkLoopFactory.h"
#include "toolbox/BSem.h"
#include "toolbox/lang/Class.h"

#include "xdaq2rc/RcmsStateNotifier.h"
#include "xoap/MessageReference.h"
#include "xcept/Exception.h"
#include "xcept/tools.h"

#include "xdata/UnsignedInteger64.h"
#include "xdata/UnsignedInteger.h"
#include "xdata/UnsignedShort.h"
#include "xdata/Integer.h"

class TH1F;
class TH2F;
class TFile;
class TCanvas;

namespace toolbox {
  namespace fsm {
    class AsynchronousFiniteStateMachine;
  }
}

namespace cgicc {
  BOOLEAN_ELEMENT(section,"section");
}

namespace gem {
  
  namespace hw {
    namespace vfat {
      class HwVFAT2;
    }
  }

  typedef std::shared_ptr<hw::vfat::HwVFAT2 > vfat_shared_ptr;
  
  namespace supervisor {
    namespace tbutils {

      class ADCScan : public xdaq::WebApplication, public xdata::ActionListener
        {
	  
        public:
          XDAQ_INSTANTIATOR();
          ADCScan(xdaq::ApplicationStub * s)
            throw (xdaq::exception::Exception);
          ~ADCScan();

          xoap::MessageReference changeState(xoap::MessageReference msg);
          void stateChanged(toolbox::fsm::FiniteStateMachine& fsm);
          void transitionFailed(toolbox::Event::Reference event);
          void fireEvent(const std::string& name);
	  
          // SOAP interface
          xoap::MessageReference onInitialize(xoap::MessageReference message)
            throw (xoap::exception::Exception);
          xoap::MessageReference onConfigure(xoap::MessageReference message)
            throw (xoap::exception::Exception);
          xoap::MessageReference onStart(xoap::MessageReference message)
            throw (xoap::exception::Exception);
          xoap::MessageReference onStop(xoap::MessageReference message)
            throw (xoap::exception::Exception);
          xoap::MessageReference onHalt(xoap::MessageReference message)
            throw (xoap::exception::Exception);
          xoap::MessageReference onReset(xoap::MessageReference message)
            throw (xoap::exception::Exception);

          // HyperDAQ interface
          void webDefault(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);
          void webInitialize(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);
          void webConfigure(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);
          void webStart(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);
          void webStop(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);
          void webHalt(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);
          void webReset(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);


          //workloop functions
          bool initialize(toolbox::task::WorkLoop* wl);
          bool configure( toolbox::task::WorkLoop* wl);
          bool start(     toolbox::task::WorkLoop* wl);
          bool stop(      toolbox::task::WorkLoop* wl);
          bool halt(      toolbox::task::WorkLoop* wl);
          bool reset(     toolbox::task::WorkLoop* wl);
          bool run(       toolbox::task::WorkLoop* wl);

          // State transitions
          void initializeAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);
          void configureAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);
          void startAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);
          void stopAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);
          void haltAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);
          void resetAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);
          void noAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);
	  
	  
          //web display helpers
          void selectVFAT(xgi::Output* out)
            throw (xgi::exception::Exception);
          void scanParameters(xgi::Output* out)
            throw (xgi::exception::Exception);
          void displayHistograms(xgi::Output* out)
            throw (xgi::exception::Exception);
          void redirect(xgi::Input* in, xgi::Output* out);
	  
          //action performed callback
          void actionPerformed(xdata::Event& event);

          class ConfigParams 
          {
          public:
            //void getFromFile(const std::string& fileName);
            void registerFields(xdata::Bag<ConfigParams> *bag);

            xdata::UnsignedShort  stepSize;
            xdata::UnsignedShort  minDACValue;
            xdata::UnsignedShort  maxDACValue;
	    
            xdata::String        outFileName;
            xdata::String        settingsFile;

            xdata::String        dacToScan;
            xdata::String        deviceName;
            xdata::String        deviceIP;
            xdata::Integer       deviceNum;
            xdata::UnsignedShort deviceChipID;

            xdata::UnsignedInteger nSamples;
          };

        private:
          toolbox::fsm::AsynchronousFiniteStateMachine* fsmP_;

          toolbox::task::WorkLoop *wl_;
          toolbox::BSem wl_semaphore_;

          toolbox::BSem hw_semaphore_;
	  
          toolbox::task::ActionSignature* initSig_;
          toolbox::task::ActionSignature* confSig_;
          toolbox::task::ActionSignature* startSig_;
          toolbox::task::ActionSignature* stopSig_;
          toolbox::task::ActionSignature* haltSig_;
          toolbox::task::ActionSignature* resetSig_;
          toolbox::task::ActionSignature* runSig_;

          //ConfigParams confParams_;
          xdata::Bag<ConfigParams> confParams_;
          xdata::String ipAddr_;
	  
          FILE* outputFile;
          //std::fstream* scanStream;
          //0xdeadbeef

          int minDACValue_, maxDACValue_;
          uint8_t  curDACRegValue;
          uint32_t curDACValue;
          uint64_t stepSize_, samplesTaken_;
          bool is_working_, is_initialized_, is_configured_, is_running_;
	  vfat_shared_ptr vfatDevice_;
	  
          //dac register mapping
          //dacMap[regName] = <ADC to read, DAC Mode>
          std::map<std::string,std::pair<std::string, std::string> > dacMap;
	  
          TH2F* histo;
          TCanvas* outputCanvas;
        protected:
	  
	  
        };

    } //end namespace gem::supervisor::tbutils
  } //end namespace gem::supervisor
} //end namespace gem
#endif
