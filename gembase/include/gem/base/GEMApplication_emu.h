#ifndef gem_base_GEMApplication_H
#define gem_base_GEMApplication_H

#include <cstdlib>
#include "string.h"
#include "limits.h"
#include <string>
#include <string>
#include <deque>
#include <map>

#include "xdaq/Application.h"
#include "xdaq/WebApplication.h"

#include "log4cplus/logger.h"

#include "toolbox/BSem.h"
#include "toolbox/task/WorkLoop.h"
#include "toolbox/fsm/FiniteStateMachine.h"
#include "toolbox/mem/MemoryPoolFactory.h"

#include "xdata/Bag.h"
#include "xdata/Vector.h"
#include "xdata/InfoSpace.h"
#include "xdata/ActionListener.h"

#include "xdata/Float.h"
#include "xdata/String.h"
#include "xdata/Boolean.h"
#include "xdata/Integer.h"
#include "xdata/Integer64.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger.h"

#include "xoap/MessageReference.h"

#include "i2o/utils/AddressMap.h"

#include "xgi/Method.h"
#include "xdaq2rc/RcmsStateNotifier.h"

namespace cgicc {
  BOOLEAN_ELEMENT(section,"section");
}

namespace gem {
  namespace base {
    
    class GEMApplication : public xdaq::Application, public xdata::ActionListener
      {
      public:
        XDAQ_INSTANTIATOR();
	
        GEMApplication(xdaq::ApplicationStub *stub)
          throw (xdaq::exception::Exception);
	
        toolbox::fsm::FiniteStateMachine* getFSM(){ return &fsm_; }
	
        //virtual void actionPerformed(xdata::Event&);
	
      protected:
        // SOAP interface
        xoap::MessageReference onEnable(     xoap::MessageReference message)
          throw (xoap::exception::Exception);
        xoap::MessageReference onConfigure(  xoap::MessageReference message)
          throw (xoap::exception::Exception);
        xoap::MessageReference onStart(      xoap::MessageReference message)
          throw (xoap::exception::Exception);
        xoap::MessageReference onPause(      xoap::MessageReference message)
          throw (xoap::exception::Exception);
        xoap::MessageReference onResume(     xoap::MessageReference message)
          throw (xoap::exception::Exception);
        xoap::MessageReference onStop(       xoap::MessageReference message)
          throw (xoap::exception::Exception);
        xoap::MessageReference onHalt(       xoap::MessageReference message)
          throw (xoap::exception::Exception);
        xoap::MessageReference onReset(      xoap::MessageReference message)
          throw (xoap::exception::Exception);
        xoap::MessageReference onRunSequence(xoap::MessageReference message)
          throw (xoap::exception::Exception);
        xoap::MessageReference reset(        xoap::MessageReference message)
          throw (xoap::exception::Exception);
        xoap::MessageReference fireEvent(    xoap::MessageReference message)
          throw (xoap::exception::Exception);
        xoap::MessageReference createReply(  xoap::MessageReference message)
          throw (xoap::exception::Exception);

        // work loop call-back functions
        bool enableAction(   toolbox::task::WorkLoop *wl);
        bool configureAction(toolbox::task::WorkLoop *wl);
        bool startAction(    toolbox::task::WorkLoop *wl);
        bool pauseAction(    toolbox::task::WorkLoop *wl);
        bool resumeAction(   toolbox::task::WorkLoop *wl);
        bool stopAction(     toolbox::task::WorkLoop *wl);
        bool haltAction(     toolbox::task::WorkLoop *wl);
        bool resetAction(    toolbox::task::WorkLoop *wl);

        //bool calibrationAction(toolbox::task::WorkLoop *wl);
        //bool calibrationSequencer(toolbox::task::WorkLoop *wl);
	
        //state transitions
        void enableAction(   toolbox::Event::Reference e)
          throw (toolbox::fsm::exception::Exception);
        void configureAction(toolbox::Event::Reference e)
          throw (toolbox::fsm::exception::Exception);
        void startAction(    toolbox::Event::Reference e)
          throw (toolbox::fsm::exception::Exception);
        void pauseAction(    toolbox::Event::Reference e)
          throw (toolbox::fsm::exception::Exception);
        void resumeAction(   toolbox::Event::Reference e)
          throw (toolbox::fsm::exception::Exception);
        void stopAction(     toolbox::Event::Reference e)
          throw (toolbox::fsm::exception::Exception);
        void haltAction(     toolbox::Event::Reference e)
          throw (toolbox::fsm::exception::Exception);
        void noAction(       toolbox::Event::Reference e) 
          throw (toolbox::fsm::exception::Exception);

        void resetAction()//toolbox::Event::Reference e)
          throw (toolbox::fsm::exception::Exception);

        void stateChanged(    toolbox::fsm::FiniteStateMachine &fsm)
          throw (toolbox::fsm::exception::Exception);
        void transitionFailed(toolbox::Event::Reference event)
          throw (toolbox::fsm::exception::Exception);

        void fireEvent(std::string event)
          throw (toolbox::fsm::exception::Exception);
	

      private:
        toolbox::fsm::FiniteStateMachine fsm_;

        xdata::String state_;
        xdata::String reasonForFailure_;

        /**** application properties ****/
        i2o::utils::AddressMap *i2oAddressMap_;
        toolbox::mem::MemoryPoolFactory *poolFactory_;
	
        xdata::InfoSpace *appInfoSpace_;
        xdaq::ApplicationDescriptor* appDescriptor_;
        xdaq::ApplicationContext *appContext_;
        xdaq::ApplicationGroup *appGroup_;
        std::string xmlClass_;
        unsigned long instance_;
        std::string urn_;
	
	
        log4cplus::Logger logger_;

        xdata::String          run_type_;
        xdata::UnsignedInteger run_number_;
        xdata::UnsignedInteger runSequenceNumber_;

        xdata::Integer64 nevents_;

        xdaq2rc::RcmsStateNotifier rcmsStateNotifier_;

        toolbox::BSem wl_semaphore_;

        toolbox::task::WorkLoop *wl_;

        toolbox::task::ActionSignature *enable_signature_, *configure_signature_;
        toolbox::task::ActionSignature *start_signature_,  *pause_signature_, *resume_signature_;
        toolbox::task::ActionSignature *stop_signature_,   *halt_signature_;

      };
    
  } // namespace gem::base
} // namespace gem

#endif
