#ifndef gem_base_GEMFSMApplication_h
#define gem_base_GEMFSMApplication_h

#include "gem/base/GEMApplication.h"
#include "gem/base/GEMFSM.h"

namespace toolbox {
  namespace task{
    class WorkLoop;
    class ActionSignature;
  }
}

namespace gem {
  namespace base {
    class GEMFSM;
    class GEMApplication;
    class GEMWebApplication;

    class GEMFSMApplication : public GEMApplication
      {
      public:
	friend class GEMFSM;
	friend class GEMApplication;
	friend class GEMWebApplication;
	
	GEMFSMApplication(xdaq::ApplicationStub *stub)
	  throw (xdaq::exception::Exception);

	virtual ~GEMFSMApplication();

      protected:
	//xgi interface
	virtual void xgiInitialize(xgi::Input *in, xgi::Output *out );
	virtual void xgiEnable(    xgi::Input *in, xgi::Output *out );
	virtual void xgiConfigure( xgi::Input *in, xgi::Output *out );
	virtual void xgiStart(     xgi::Input *in, xgi::Output *out );
	virtual void xgiPause(     xgi::Input *in, xgi::Output *out );
	virtual void xgiResume(    xgi::Input *in, xgi::Output *out );
	virtual void xgiStop(      xgi::Input *in, xgi::Output *out );
	virtual void xgiHalt(      xgi::Input *in, xgi::Output *out );
 	virtual void xgiReset(     xgi::Input *in, xgi::Output *out );


	/*
	// SOAP interface
	virtual xoap::MessageReference onEnable(     xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference onConfigure(  xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference onStart(      xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference onPause(      xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference onResume(     xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference onStop(       xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference onHalt(       xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference onReset(      xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference onRunSequence(xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference reset(        xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference fireEvent(    xoap::MessageReference message)
	  throw (xoap::exception::Exception);
	virtual xoap::MessageReference createReply(  xoap::MessageReference message)
	  throw (xoap::exception::Exception);

	// work loop call-back functions
	virtual bool initializeAction(toolbox::task::WorkLoop *wl);
	virtual bool enableAction(    toolbox::task::WorkLoop *wl);
	virtual bool configureAction( toolbox::task::WorkLoop *wl);
	virtual bool startAction(     toolbox::task::WorkLoop *wl);
	virtual bool pauseAction(     toolbox::task::WorkLoop *wl);
	virtual bool resumeAction(    toolbox::task::WorkLoop *wl);
	virtual bool stopAction(      toolbox::task::WorkLoop *wl);
	virtual bool haltAction(      toolbox::task::WorkLoop *wl);
	virtual bool resetAction(     toolbox::task::WorkLoop *wl);
	//virtual bool noAction(        toolbox::task::WorkLoop *wl);
	virtual bool failAction(      toolbox::task::WorkLoop *wl);

	//bool calibrationAction(toolbox::task::WorkLoop *wl);
	//bool calibrationSequencer(toolbox::task::WorkLoop *wl);
	*/
	
	//state transitions
	virtual void initializeAction(toolbox::Event::Reference e) = 0;
	virtual void enableAction(    toolbox::Event::Reference e) = 0;
	virtual void configureAction( toolbox::Event::Reference e) = 0;
	virtual void startAction(     toolbox::Event::Reference e) = 0;
	virtual void pauseAction(     toolbox::Event::Reference e) = 0;
	virtual void resumeAction(    toolbox::Event::Reference e) = 0;
	virtual void stopAction(      toolbox::Event::Reference e) = 0;
	virtual void haltAction(      toolbox::Event::Reference e) = 0;
	virtual void noAction(        toolbox::Event::Reference e) = 0; 
	virtual void failAction(      toolbox::Event::Reference e) = 0; 
	
	virtual void resetAction()//toolbox::Event::Reference e)
	  throw (toolbox::fsm::exception::Exception) = 0;
	
	virtual void stateChanged(    toolbox::fsm::FiniteStateMachine &fsm)
	  throw (toolbox::fsm::exception::Exception) = 0;
	virtual void transitionFailed(toolbox::Event::Reference event)
	  throw (toolbox::fsm::exception::Exception) = 0;

	virtual void fireEvent(std::string event)
	  throw (toolbox::fsm::exception::Exception) = 0;
	
	virtual xoap::MessageReference changeState(xoap::MessageReference msg);
	
      private:
	GEMFSM gemfsm_;
	
	toolbox::BSem wl_semaphore_;

	toolbox::task::WorkLoop *wl_;

	toolbox::task::ActionSignature *enable_signature_, *configure_signature_;
	toolbox::task::ActionSignature *start_signature_,  *pause_signature_, *resume_signature_;
	toolbox::task::ActionSignature *stop_signature_,   *halt_signature_;
	
	xdata::InfoSpace* gemAppStateInfoSpaceP_;

      };
    
  } // namespace gem::base
} // namespace gem

#endif
