#ifndef gem_supervisor_tbutils_ThresholdScan_h
#define gem_supervisor_tbutils_ThresholdScan_h

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
  
  namespace supervisor {
    namespace tbutils {

      class ThresholdScan : public xdaq::WebApplication, public xdata::ActionListener
	{
	  
	public:
	  XDAQ_INSTANTIATOR();
	  ThresholdScan(xdaq::ApplicationStub * s)
	    throw (xdaq::exception::Exception);
	  ~ThresholdScan();

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
	  void webResetCounters(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  void webSendFastCommands(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);


	  bool initialize(toolbox::task::WorkLoop* wl);
	  bool configure( toolbox::task::WorkLoop* wl);
	  bool start(     toolbox::task::WorkLoop* wl);
	  bool stop(      toolbox::task::WorkLoop* wl);
	  bool halt(      toolbox::task::WorkLoop* wl);
	  bool reset(     toolbox::task::WorkLoop* wl);
	  bool run(       toolbox::task::WorkLoop* wl);
	  bool readFIFO(  toolbox::task::WorkLoop* wl);

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
	  void showCounterLayout(xgi::Output* out)
	    throw (xgi::exception::Exception);
	  void fastCommandLayout(xgi::Output* out)
	    throw (xgi::exception::Exception);
	  void showBufferLayout(xgi::Output* out)
	    throw (xgi::exception::Exception);
	  void displayHistograms(xgi::Output* out)
	    throw (xgi::exception::Exception);
	  void redirect(xgi::Input* in, xgi::Output* out);
	  
	  void actionPerformed(xdata::Event& event);

	  class ConfigParams 
	  {
	  public:
	    //void getFromFile(const std::string& fileName);
	    void registerFields(xdata::Bag<ConfigParams> *bag);
	    
	    xdata::UnsignedInteger readoutDelay;

	    xdata::UnsignedInteger latency;
	    xdata::UnsignedInteger nTriggers;
	    xdata::UnsignedInteger stepSize;

	    xdata::Integer minThresh;
	    xdata::Integer maxThresh;
	    
	    xdata::String        deviceName;
	    xdata::Integer       deviceNum;
	    xdata::UnsignedShort deviceChipID;
	    xdata::UnsignedShort deviceVT1;
	    xdata::UnsignedShort deviceVT2;
	    xdata::UnsignedInteger64 triggersSeen;
	  };
	  /*
	  class DeviceParams 
	  {
	  public:
	    //void getFromFile(const std::string& fileName);
	    void registerFields(xdata::Bag<DeviceParams> *bag);
	    
	    xdata::UnsignedInteger latency;
	    xdata::UnsignedShort deviceVT1;
	    xdata::UnsignedShort deviceVT2;
	    xdata::UnsignedInteger64 triggersSeen;
	    xdata::UnsignedInteger64 bufferDepth;
	    
	    
	  };
	  */
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
	  toolbox::task::ActionSignature* readSig_;

	  //ConfigParams confParams_;
	  xdata::Bag<ConfigParams> confParams_;


	  int minThresh_, maxThresh_;
	  uint64_t nTriggers_, stepSize_, latency_;
	  bool is_working_, is_initialized_, is_configured_, is_running_;
	  gem::hw::vfat::HwVFAT2* vfatDevice_;
	  
	  TH1F* histos[128];
	  TCanvas* outputCanvas;
	protected:
	  
	  
	};

    } //end namespace gem::supervisor::tbutils
  } //end namespace gem::supervisor
} //end namespace gem
#endif
