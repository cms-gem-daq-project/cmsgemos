#ifndef gem_supervisor_tbutils_GEMTBUtil_h
#define gem_supervisor_tbutils_GEMTBUtil_h


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
#include "xdata/Vector.h"

#include "TStopwatch.h"

class TH1D;
class TH1F;
class TFile;
class TCanvas;

class MyTime;

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
    class GEMHwDevice;
    namespace vfat {
      class HwVFAT2;
    }
    namespace optohybrid {
      class HwOptoHybrid;
    }
    namespace glib {
      class HwGLIB;
    }
  }
  namespace readout {
    struct VFATData;
  }
  namespace readout {
    class GEMDataParker;
  }

  typedef std::shared_ptr<hw::vfat::HwVFAT2 > vfat_shared_ptr;
  typedef std::shared_ptr<hw::glib::HwGLIB >  glib_shared_ptr;
  typedef std::shared_ptr<hw::optohybrid::HwOptoHybrid > optohybrid_shared_ptr;

  namespace supervisor {
    namespace tbutils {
      
      class GEMTBUtil : public xdaq::WebApplication, public xdata::ActionListener
	{
	  
	public:
	  XDAQ_INSTANTIATOR();
	  GEMTBUtil(xdaq::ApplicationStub * s)
	    throw (xdaq::exception::Exception);
	  virtual ~GEMTBUtil();

	  // virtual xoap::MessageReference changeState(xoap::MessageReference msg);
	  virtual void stateChanged(toolbox::fsm::FiniteStateMachine& fsm);
	  virtual void transitionFailed(toolbox::Event::Reference event);
	  virtual void fireEvent(const std::string& name);
	  
	  // SOAP interface
	  virtual xoap::MessageReference onInitialize(xoap::MessageReference message)
	    throw (xoap::exception::Exception);
	  virtual xoap::MessageReference onConfigure(xoap::MessageReference message)
	    throw (xoap::exception::Exception);
	  virtual xoap::MessageReference onStart(xoap::MessageReference message)
	    throw (xoap::exception::Exception);
	  virtual xoap::MessageReference onStop(xoap::MessageReference message)
	    throw (xoap::exception::Exception);
	  virtual xoap::MessageReference onHalt(xoap::MessageReference message)
	    throw (xoap::exception::Exception);
	  virtual xoap::MessageReference onReset(xoap::MessageReference message)
	    throw (xoap::exception::Exception);

	  // HyperDAQ interface
	  virtual void webDefault(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  virtual void webInitialize(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  virtual void webConfigure(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  virtual void webStart(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  virtual void webStop(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  virtual void webHalt(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  virtual void webReset(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  virtual void webResetCounters(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  virtual void webSendFastCommands(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);

	  //workloop functions
	  virtual bool initialize(toolbox::task::WorkLoop* wl);
	  virtual bool configure( toolbox::task::WorkLoop* wl);
	  virtual bool start(     toolbox::task::WorkLoop* wl);
	  virtual bool stop(      toolbox::task::WorkLoop* wl);
	  virtual bool halt(      toolbox::task::WorkLoop* wl);
	  virtual bool reset(     toolbox::task::WorkLoop* wl);
	  virtual bool run(       toolbox::task::WorkLoop* wl)=0;
	  virtual bool readFIFO(  toolbox::task::WorkLoop* wl)=0;

	  // State transitions
	  virtual void initializeAction(toolbox::Event::Reference e)
	    throw (toolbox::fsm::exception::Exception);
	  virtual void configureAction(toolbox::Event::Reference e)
	    throw (toolbox::fsm::exception::Exception)=0;
	  virtual void startAction(toolbox::Event::Reference e)
	    throw (toolbox::fsm::exception::Exception)=0;
	  virtual void stopAction(toolbox::Event::Reference e)
	    throw (toolbox::fsm::exception::Exception);
	  virtual void haltAction(toolbox::Event::Reference e)
	    throw (toolbox::fsm::exception::Exception);
	  virtual void resetAction(toolbox::Event::Reference e)
	    throw (toolbox::fsm::exception::Exception);
	  virtual void noAction(toolbox::Event::Reference e)
	    throw (toolbox::fsm::exception::Exception);
	  
	  //web display helpers
	  //	  virtual void selectVFAT(xgi::Output* out)
	  //  throw (xgi::exception::Exception);
	  virtual void selectMultipleVFAT(xgi::Output* out)
	    throw (xgi::exception::Exception);
	  virtual void scanParameters(xgi::Output* out)
	    throw (xgi::exception::Exception)=0;
	  virtual void showCounterLayout(xgi::Output* out)
	    throw (xgi::exception::Exception);
	  virtual void fastCommandLayout(xgi::Output* out)
	    throw (xgi::exception::Exception);
	  virtual void showBufferLayout(xgi::Output* out)
	    throw (xgi::exception::Exception);
	  virtual void displayHistograms(xgi::Output* out)
	    throw (xgi::exception::Exception)=0;
	  virtual void redirect(xgi::Input* in, xgi::Output* out);
	  
	  //action performed callback
	  virtual void actionPerformed(xdata::Event& event);

	  class ConfigParams 
	  {
	  public:
	    //void getFromFile(const std::string& fileName);
	    virtual void registerFields(xdata::Bag<ConfigParams> *bag);
	    
	    xdata::UnsignedInteger readoutDelay;

	    xdata::UnsignedInteger nTriggers;

	    xdata::String        outFileName;
	    xdata::String        settingsFile;

	    xdata::Vector<xdata::String>  deviceName;
	    xdata::Vector<xdata::Integer> deviceNum;
	    xdata::String        deviceIP;
	    xdata::UnsignedShort triggerSource;
	    xdata::UnsignedShort deviceChipID;
	    xdata::UnsignedInteger64 triggersSeen;
	    xdata::Integer       ADCVoltage;
	    xdata::Integer       ADCurrent;

	    xdata::UnsignedShort deviceVT1;
	    xdata::UnsignedShort deviceVT2;
	    
	  };
	  
	protected:

	  log4cplus::Logger m_gemLogger;

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
	  uint8_t readout_mask;
	  xdata::Bag<ConfigParams> confParams_;
	  xdata::String ipAddr_;
	  
	  FILE* outputFile;
	  //std::fstream* scanStream;
	  //0xdeadbeef

	  uint64_t nTriggers_;
	  bool is_working_, is_initialized_, is_configured_, is_running_;

	  //readout application should be running elsewhere, not tied to supervisor                                                                           
        glib_shared_ptr glibDevice_;
        optohybrid_shared_ptr optohybridDevice_;
        std::vector<vfat_shared_ptr> vfatDevice_;

        std::shared_ptr<gem::readout::GEMDataParker> gemDataParker;

	  // Counter
	  int counter_[3];
	  
	  // VFAT Blocks Counter
	  int vfat_;

	  // Events Counter     
	  int event_;
	  
	  // VFATs counter per event
	  int sumVFAT_;
	  
	  // CalPulse counting
	  uint32_t CalPulseCount_[3];
	  
	  TH1D* histolatency;
	  TH1F* histo;
	  TH1F* histos[128];
	  TCanvas* outputCanvas;

          TStopwatch timer;

	  xdata::Bag<ConfigParams> scanParams_;
	  uint64_t eventsSeen_,channelSeen_;
	  uint8_t  currentLatency_;


	protected:

	};

    } //end namespace gem::supervisor::tbutils
  } //end namespace gem::supervisor
} //end namespace gem
#endif
