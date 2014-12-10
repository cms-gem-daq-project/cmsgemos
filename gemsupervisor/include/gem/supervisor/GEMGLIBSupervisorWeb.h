#ifndef gem_supervisor_GEMGLIBSupervisorWeb_h
#define gem_supervisor_GEMGLIBSupervisorWeb_h

#include "xdaq/Application.h"
#include "xdaq/WebApplication.h"

#include "xdata/Float.h"
#include "xdata/String.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"

#include "uhal/uhal.hpp"

#include "toolbox/fsm/FiniteStateMachine.h"
#include "toolbox/fsm/FailedEvent.h"
#include "toolbox/task/WorkLoop.h"
#include "toolbox/task/WorkLoopFactory.h"
#include "toolbox/Event.h"

#include "xdaq/NamespaceURI.h"
#include "xoap/Method.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPPart.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/SOAPSerializer.h"
#include "xoap/domutils.h"

#include "xoap/DOMParser.h"
#include "xoap/DOMParserFactory.h"
#include "xoap/domutils.h"
#include "xdata/soap/Serializer.h"

#include "xgi/framework/Method.h"
#include "cgicc/HTMLClasses.h"

#include <string>

typedef uhal::exception::exception uhalException;

namespace gem {
	namespace supervisor {

		class GEMGLIBSupervisorWeb: public xdaq::WebApplication
		{
			public:
				XDAQ_INSTANTIATOR();

				GEMGLIBSupervisorWeb(xdaq::ApplicationStub * s)
					throw (xdaq::exception::Exception);

				// SOAP interface
				xoap::MessageReference onConfigure(xoap::MessageReference message)
					throw (xoap::exception::Exception);
				xoap::MessageReference onStart(xoap::MessageReference message)
					throw (xoap::exception::Exception);
				xoap::MessageReference onStop(xoap::MessageReference message)
					throw (xoap::exception::Exception);
				xoap::MessageReference onHalt(xoap::MessageReference message)
					throw (xoap::exception::Exception);

				// HyperDAQ interface
				void webDefault(xgi::Input *in, xgi::Output *out)
					throw (xgi::exception::Exception);
				void webConfigure(xgi::Input *in, xgi::Output *out)
					throw (xgi::exception::Exception);
				void webStart(xgi::Input *in, xgi::Output *out)
					throw (xgi::exception::Exception);
				void webStop(xgi::Input *in, xgi::Output *out)
					throw (xgi::exception::Exception);
				void webHalt(xgi::Input *in, xgi::Output *out)
					throw (xgi::exception::Exception);
				void webRedirect(xgi::Input *in, xgi::Output *out)
					throw (xgi::exception::Exception);

				// work loop call-back functions
				bool configureAction(toolbox::task::WorkLoop *wl);
				bool stopAction(toolbox::task::WorkLoop *wl);
				bool haltAction(toolbox::task::WorkLoop *wl);
				bool startAction(toolbox::task::WorkLoop *wl);

				// State transitions
				void configureAction(toolbox::Event::Reference e)
					throw (toolbox::fsm::exception::Exception);
				void startAction(toolbox::Event::Reference e)
					throw (toolbox::fsm::exception::Exception);
				void stopAction(toolbox::Event::Reference e)
					throw (toolbox::fsm::exception::Exception);
				void haltAction(toolbox::Event::Reference e)
					throw (toolbox::fsm::exception::Exception);
				void noAction(toolbox::Event::Reference e)
					throw (toolbox::fsm::exception::Exception);

			private:
				toolbox::task::WorkLoop *wl_;
				toolbox::task::ActionSignature *configure_signature_, *stop_signature_, *halt_signature_, *start_signature_;
				toolbox::fsm::FiniteStateMachine fsm_;
				int counter_;
				bool is_running_;

				void fireEvent(std::string name);
				void stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
					throw (toolbox::fsm::exception::Exception);
				void transitionFailed(toolbox::Event::Reference event)
					throw (toolbox::fsm::exception::Exception);
		};
	}
}
#endif
