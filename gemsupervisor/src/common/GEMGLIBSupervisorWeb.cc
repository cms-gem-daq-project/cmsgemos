#include "gem/supervisor/GEMGLIBSupervisorWeb.h"
#include <sstream>
#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::GEMGLIBSupervisorWeb)

gem::supervisor::GEMGLIBSupervisorWeb::GEMGLIBSupervisorWeb(xdaq::ApplicationStub * s)
	throw (xdaq::exception::Exception):
		xdaq::WebApplication(s)
{
	xgi::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::webDefault,     "Default");
	xgi::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::webConfigure,   "Configure");
	xgi::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::webStart,       "Start");
	xgi::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::webStop,        "Stop");
	xgi::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::webHalt,        "Halt");

	xoap::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::onConfigure,   "Configure",   XDAQ_NS_URI);
	xoap::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::onStart,       "Start",       XDAQ_NS_URI);
	xoap::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::onStop,        "Stop",        XDAQ_NS_URI);
	xoap::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::onHalt,        "Halt",        XDAQ_NS_URI);

	wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("GEMGLIBSupervisorWebWorkLoop", "waiting");
	wl_->activate();

	configure_signature_   = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::configureAction,      "configureAction");
	start_signature_       = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::startAction,          "startAction");
	stop_signature_        = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::stopAction,           "stopAction");
	halt_signature_        = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction,           "haltAction");

	fsm_.addState('H', "Halted",     this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);
	fsm_.addState('C', "Configured", this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);
	fsm_.addState('R', "Running",    this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);

	fsm_.setStateName('F', "Error");
	fsm_.setFailedStateTransitionAction(this, &gem::supervisor::GEMGLIBSupervisorWeb::transitionFailed);
	fsm_.setFailedStateTransitionChanged(this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);

	fsm_.addStateTransition('H', 'C', "Configure", this, &gem::supervisor::GEMGLIBSupervisorWeb::configureAction);
	fsm_.addStateTransition('H', 'H', "Halt",      this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction);
	fsm_.addStateTransition('C', 'C', "Configure", this, &gem::supervisor::GEMGLIBSupervisorWeb::configureAction);
	fsm_.addStateTransition('C', 'R', "Start",     this, &gem::supervisor::GEMGLIBSupervisorWeb::startAction);
	fsm_.addStateTransition('C', 'H', "Halt",      this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction);
	fsm_.addStateTransition('R', 'C', "Stop",      this, &gem::supervisor::GEMGLIBSupervisorWeb::stopAction);
	fsm_.addStateTransition('R', 'H', "Halt",      this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction);

	fsm_.addStateTransition('H', 'H', "Start"    , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);
	fsm_.addStateTransition('H', 'H', "Stop"     , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);
	fsm_.addStateTransition('C', 'C', "Stop"     , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);
	fsm_.addStateTransition('R', 'R', "Configure", this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);
	fsm_.addStateTransition('R', 'R', "Start"    , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);

	fsm_.setInitialState('H');
	fsm_.reset();

	counter_ = 0;
}

// SOAP interface
	xoap::MessageReference gem::supervisor::GEMGLIBSupervisorWeb::onConfigure(xoap::MessageReference message)
throw (xoap::exception::Exception)
{
	wl_->submit(configure_signature_);
	return message;
}

	xoap::MessageReference gem::supervisor::GEMGLIBSupervisorWeb::onStart(xoap::MessageReference message)
throw (xoap::exception::Exception)
{
	wl_->submit(start_signature_);
	return message;
}

	xoap::MessageReference gem::supervisor::GEMGLIBSupervisorWeb::onStop(xoap::MessageReference message)
throw (xoap::exception::Exception)
{
	wl_->submit(stop_signature_);
	return message;
}

	xoap::MessageReference gem::supervisor::GEMGLIBSupervisorWeb::onHalt(xoap::MessageReference message)
throw (xoap::exception::Exception)
{
	wl_->submit(halt_signature_);
	return message;
}

// HyperDAQ interface
	void gem::supervisor::GEMGLIBSupervisorWeb::webDefault(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
	if (is_running_) head.addHeader("Refresh","1");

	*out << cgicc::h1("GEM GLIB Supervisor")<< std::endl;
	*out << "Current state: " << fsm_.getStateName(fsm_.getCurrentState()) << cgicc::br();
	*out << "Current counter: " << counter_ << cgicc::br();
	*out << "is_running_: " << is_running_ << cgicc::br();

	if (is_running_) counter_++;

	*out << cgicc::table().set("border","0");

	*out << cgicc::tr();

	*out << cgicc::td();
	std::string configureButton = toolbox::toString("/%s/Configure",getApplicationDescriptor()->getURN().c_str());
	*out << cgicc::form().set("method","GET").set("action",configureButton) << std::endl ;
	*out << cgicc::input().set("type","submit").set("value","Configure") << std::endl ;
	*out << cgicc::form();
	*out << cgicc::td();

	*out << cgicc::td();
	std::string startButton = toolbox::toString("/%s/Start",getApplicationDescriptor()->getURN().c_str());
	*out << cgicc::form().set("method","GET").set("action",startButton) << std::endl ;
	*out << cgicc::input().set("type","submit").set("value","Start") << std::endl ;
	*out << cgicc::form();
	*out << cgicc::td();

	*out << cgicc::td();
	std::string stopButton = toolbox::toString("/%s/Stop",getApplicationDescriptor()->getURN().c_str());
	*out << cgicc::form().set("method","GET").set("action",stopButton) << std::endl ;
	*out << cgicc::input().set("type","submit").set("value","Stop") << std::endl ;
	*out << cgicc::form();
	*out << cgicc::td();

	*out << cgicc::td();
	std::string haltButton = toolbox::toString("/%s/Halt",getApplicationDescriptor()->getURN().c_str());
	*out << cgicc::form().set("method","GET").set("action",haltButton) << std::endl ;
	*out << cgicc::input().set("type","submit").set("value","Halt") << std::endl ;
	*out << cgicc::form();
	*out << cgicc::td();

	*out << cgicc::tr();

	*out << cgicc::table();
}

	void gem::supervisor::GEMGLIBSupervisorWeb::webConfigure(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	wl_->submit(configure_signature_);
	this->webRedirect(in, out);
}

	void gem::supervisor::GEMGLIBSupervisorWeb::webStart(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	wl_->submit(start_signature_);
	this->webRedirect(in, out);
}

	void gem::supervisor::GEMGLIBSupervisorWeb::webStop(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	wl_->submit(stop_signature_);
	this->webRedirect(in, out);
}

	void gem::supervisor::GEMGLIBSupervisorWeb::webHalt(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
	wl_->submit(halt_signature_);
	this->webRedirect(in, out);
}

	void gem::supervisor::GEMGLIBSupervisorWeb::webRedirect(xgi::Input *in, xgi::Output* out) 
throw (xgi::exception::Exception)
{
	std::string url = "/" + getApplicationDescriptor()->getURN() + "/Default";
	*out << "<meta http-equiv=\"refresh\" content=\"0;" << url << "\">" << std::endl;
	this->webDefault(in,out);
}

// work loop call-back functions
bool gem::supervisor::GEMGLIBSupervisorWeb::configureAction(toolbox::task::WorkLoop *wl)
{
	fireEvent("Configure");
	return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::startAction(toolbox::task::WorkLoop *wl)
{
	fireEvent("Start");
	return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::stopAction(toolbox::task::WorkLoop *wl)
{
	fireEvent("Stop");
	return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::haltAction(toolbox::task::WorkLoop *wl)
{
	fireEvent("Halt");
	return false;
}

// State transitions
	void gem::supervisor::GEMGLIBSupervisorWeb::configureAction(toolbox::Event::Reference evt)
throw (toolbox::fsm::exception::Exception)
{
	is_running_ = false;
	counter_ = 0;
}

	void gem::supervisor::GEMGLIBSupervisorWeb::startAction(toolbox::Event::Reference evt)
throw (toolbox::fsm::exception::Exception)
{
	is_running_ = true;
}

	void gem::supervisor::GEMGLIBSupervisorWeb::stopAction(toolbox::Event::Reference evt)
throw (toolbox::fsm::exception::Exception)
{
	is_running_ = false;
}

	void gem::supervisor::GEMGLIBSupervisorWeb::haltAction(toolbox::Event::Reference evt)
throw (toolbox::fsm::exception::Exception)
{
	is_running_ = false;
	counter_ = 0;
}

	void gem::supervisor::GEMGLIBSupervisorWeb::noAction(toolbox::Event::Reference evt)
throw (toolbox::fsm::exception::Exception)
{
}

void gem::supervisor::GEMGLIBSupervisorWeb::fireEvent(std::string name)
{
	toolbox::Event::Reference event(new toolbox::Event(name, this));
	fsm_.fireEvent(event);
}

	void gem::supervisor::GEMGLIBSupervisorWeb::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
throw (toolbox::fsm::exception::Exception)
{
}

	void gem::supervisor::GEMGLIBSupervisorWeb::transitionFailed(toolbox::Event::Reference event)
throw (toolbox::fsm::exception::Exception)
{
}
