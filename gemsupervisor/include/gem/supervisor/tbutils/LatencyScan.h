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

	//SOAP MEssage	
	//	virtual xoap::MessageReference testmns(xoap::MessageReference message)
	//  throw (xoap::exception::Exception);

	void sendMessage(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	
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
        void selectTrigSource(xgi::Output* out)
	  throw (xgi::exception::Exception);
	    
      class ConfigParams 
      {
      public:
	//void getFromFile(const std::string& fileName);
	void registerFields(xdata::Bag<ConfigParams> *bag);
	    
	xdata::String          slotFileName;

	xdata::UnsignedShort  stepSize;
	xdata::UnsignedShort  minLatency;
	xdata::UnsignedShort  maxLatency;
	xdata::UnsignedShort  nTriggers;
	//xdata::UnsignedShort  triggerSource;
	    
	xdata::Integer  threshold;
	xdata::Integer  deviceVT1;
	xdata::Integer  deviceVT2;
	xdata::Integer  VCal;
	xdata::Integer  MSPulseLength;
      };
	  
    private:
      //ConfigParams m_confParams;
      xdata::Bag<ConfigParams> m_scanParams;

      //ESTAS NO ESTAN EN THRESHOLD.H
    
      //std::fstream* scanStream;
      //0xdeadbeef

      int m_minLatency, m_maxLatency, m_threshold, m_mspl, m_vCal;
      // duplicated from GEMTBUtil? uint8_t  m_currentLatency;
      uint64_t m_stepSize;// duplicated from GEMTBUtil?, m_eventsSeen, m_channelSeen;
      uint64_t m_totaltriggercounter;
      protected:
	  
      };
    } //end namespace gem::supervisor::tbutils
  } //end namespace gem::supervisor
} //end namespace gem
#endif
