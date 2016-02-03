#ifndef gem_supervisor_tbutils_LatencyScan_h
#define gem_supervisor_tbutils_LatencyScan_h

#include "gem/supervisor/tbutils/GEMTBUtil.h"
#include "TStopwatch.h"


namespace gem {
  namespace supervisor {
    namespace tbutils {

      class LatencyScan : public GEMTBUtil
      {

	//friend class GEMTBUtil
	
      public:

	XDAQ_INSTANTIATOR();
	LatencyScan(xdaq::ApplicationStub * s)
	  throw (xdaq::exception::Exception);
	~LatencyScan();
	
	// HyperDAQ interface
	void webDefault(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	void webConfigure(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	void webStart(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	/*
	void webStop(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	void webInitialize(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	void webHalt(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	void webReset(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	void webResetCounters(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	void webSendFastCommands(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	*/

	//workloop functions
	bool run(       toolbox::task::WorkLoop* wl);
	bool readFIFO(  toolbox::task::WorkLoop* wl);

	// State transitions
	void configureAction(toolbox::Event::Reference e)
	  throw (toolbox::fsm::exception::Exception);
	void startAction(toolbox::Event::Reference e)
	  throw (toolbox::fsm::exception::Exception);
	void resetAction(toolbox::Event::Reference e)
	  throw (toolbox::fsm::exception::Exception);
	/*
	void initializeAction(toolbox::Event::Reference e)
	  throw (toolbox::fsm::exception::Exception);
	void stopAction(toolbox::Event::Reference e)
	  throw (toolbox::fsm::exception::Exception);
	void haltAction(toolbox::Event::Reference e)
	  throw (toolbox::fsm::exception::Exception);
	void noAction(toolbox::Event::Reference e)
	  throw (toolbox::fsm::exception::Exception);
	*/

	//web display helpers
	void scanParameters(xgi::Output* out)
	  throw (xgi::exception::Exception);
	void displayHistograms(xgi::Output* out)
	  throw (xgi::exception::Exception);
        void selectTrigSource(xgi::Output* out)
	    throw (xgi::exception::Exception);
	    
      class ConfigParams 
      {
      public:
	//void getFromFile(const std::string& fileName);
	void registerFields(xdata::Bag<ConfigParams> *bag);
	    
	xdata::UnsignedShort  stepSize;
	xdata::UnsignedShort  minLatency;
	xdata::UnsignedShort  maxLatency;
	xdata::UnsignedShort  nTriggers;
	    
	xdata::Integer  threshold;
      };
	  
    private:
      //ConfigParams confParams_;
      xdata::Bag<ConfigParams> scanParams_;

      //ESTAS NO ESTAN EN THRESHOLD.H
    
      //std::fstream* scanStream;
      //0xdeadbeef

      int minLatency_, maxLatency_, threshold_;
      uint8_t  currentLatency_;
      uint64_t stepSize_,eventsSeen_,channelSeen_;
      uint64_t totaltriggercounter_;
      protected:
	  
      };
    } //end namespace gem::supervisor::tbutils
  } //end namespace gem::supervisor
} //end namespace gem
#endif
