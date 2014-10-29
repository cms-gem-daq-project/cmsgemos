// GEMApplication.cc

#include "gem/base/GEMApplication.h"

#include "xdaq/NamespaceURI.h"  // XDAQ_NS_URI
#include "xoap/Method.h"
#include "xoap/MessageFactory.h"  // createMessage()
#include "xoap/SOAPPart.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/SOAPSerializer.h"
#include "xoap/domutils.h"  // XMLCh2String()

#include "toolbox/fsm/FailedEvent.h"
#include "toolbox/task/WorkLoopFactory.h" // getWorkLoopFactory()
#include "xcept/tools.h"

#include "xoap/DOMParser.h"
#include "xoap/DOMParserFactory.h"
#include "xoap/domutils.h"
#include "xdata/soap/Serializer.h"

#include "xcept/tools.h"
#include "xdaq2rc/RcmsStateNotifier.h"
#include "toolbox/fsm/FailedEvent.h"

#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationDescriptorImpl.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/exception/Exception.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/XceptSerializer.h"
#include "xdata/Float.h" 
#include "xdata/Double.h" 
#include "xdata/Boolean.h"

XDAQ_INSTANTIATOR_IMPL(gem::base::GEMApplication)

gem::base::GEMApplication::GEMApplication(xdaq::ApplicationStub *stub)
  throw (xdaq::exception::Exception) :
  xdaq::Application(stub),
  logger_(Logger::getInstance("gem::base::GEMApplication")),
  run_type_("Monitor"), run_number_(1), runSequenceNumber_(0),
  nevents_(-1),
  rcmsStateNotifier_(getApplicationLogger(), getApplicationDescriptor(), getApplicationContext()),
  wl_semaphore_(toolbox::BSem::EMPTY)
{

  try {
    i2oAddressMap_ = i2o::utils::getAddressMap();
    poolFactory_   = toolbox::mem::getMemoryPoolFactory();
    appInfoSpace_  = getApplicationInfoSpace();
    appDescriptor_ = getApplicationDescriptor();
    appContext_    = getApplicationContext();
    appGroup_      = appContext_->getDefaultZone()->getApplicationGroup("default");
    xmlClass_      = appDescriptor_->getClassName();
    instance_      = appDescriptor_->getInstance();
    urn_           = appDescriptor_->getURN();
    }
  catch(xcept::Exception e) {
    XCEPT_RETHROW(xdaq::exception::Exception, "Failed to get GEM application information", e);
  }
  //xoap::deferredbind(this, this, &gem::base::GEMApplication::fireEvent, "Enable",    XDAQ_NS_URI);
  //xoap::deferredbind(this, this, &gem::base::GEMApplication::fireEvent, "Configure", XDAQ_NS_URI);
  //xoap::deferredbind(this, this, &gem::base::GEMApplication::fireEvent, "Start",     XDAQ_NS_URI);
  //xoap::deferredbind(this, this, &gem::base::GEMApplication::fireEvent, "Pause",     XDAQ_NS_URI);
  //xoap::deferredbind(this, this, &gem::base::GEMApplication::fireEvent, "Resume",    XDAQ_NS_URI);
  //xoap::deferredbind(this, this, &gem::base::GEMApplication::fireEvent, "Stop",      XDAQ_NS_URI);
  //xoap::deferredbind(this, this, &gem::base::GEMApplication::fireEvent, "Halt",      XDAQ_NS_URI);

  //what is the better way to do the binding? through an on<Function> call or through the fireEvent call?
  //where did i first see the fireEvent method???
  xoap::deferredbind(this, this, &gem::base::GEMApplication::onEnable,    "Enable",    XDAQ_NS_URI);
  xoap::deferredbind(this, this, &gem::base::GEMApplication::onConfigure, "Configure", XDAQ_NS_URI);
  xoap::deferredbind(this, this, &gem::base::GEMApplication::onStart,     "Start",     XDAQ_NS_URI);
  xoap::deferredbind(this, this, &gem::base::GEMApplication::onPause,     "Pause",     XDAQ_NS_URI);
  xoap::deferredbind(this, this, &gem::base::GEMApplication::onResume,    "Resume",    XDAQ_NS_URI);
  xoap::deferredbind(this, this, &gem::base::GEMApplication::onStop,      "Stop",      XDAQ_NS_URI);
  xoap::deferredbind(this, this, &gem::base::GEMApplication::onHalt,      "Halt",      XDAQ_NS_URI);

  xoap::deferredbind(this, this, &gem::base::GEMApplication::reset, "Reset",     XDAQ_NS_URI);

  //maybe replace GEMApplication with a more specific urn based name, in the case of multiple
  //GEMApplication instances?
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("GEMApplication", "waiting");
  wl_->activate();

  //define the signatures for the workloops
  enable_signature_      = toolbox::task::bind(this, &gem::base::GEMApplication::enableAction,         "enableAction");
  configure_signature_   = toolbox::task::bind(this, &gem::base::GEMApplication::configureAction,      "configureAction");
  start_signature_       = toolbox::task::bind(this, &gem::base::GEMApplication::startAction,          "startAction");
  pause_signature_       = toolbox::task::bind(this, &gem::base::GEMApplication::pauseAction,          "pauseAction");
  resume_signature_      = toolbox::task::bind(this, &gem::base::GEMApplication::resumeAction,         "resumeAction");
  stop_signature_        = toolbox::task::bind(this, &gem::base::GEMApplication::stopAction,           "stopAction");
  halt_signature_        = toolbox::task::bind(this, &gem::base::GEMApplication::haltAction,           "haltAction");

  //calibration_signature_ = toolbox::task::bind(this, &gem::base::GEMApplication::calibrationAction,    "calibrationAction");
  //sequencer_signature_   = toolbox::task::bind(this, &gem::base::GEMApplication::calibrationSequencer, "calibrationSequencer");

  //Define the posible states of the FSM
  fsm_.addState('H', "Halted",     this, &gem::base::GEMApplication::stateChanged);
  fsm_.addState('C', "Configured", this, &gem::base::GEMApplication::stateChanged);
  fsm_.addState('E', "Enabled",    this, &gem::base::GEMApplication::stateChanged);
  fsm_.addState('P', "Paused",     this, &gem::base::GEMApplication::stateChanged);
  fsm_.addState('R', "Running",    this, &gem::base::GEMApplication::stateChanged);

  fsm_.setStateName('F', "Failed");//,     this, &gem::base::GEMApplication::stateChanged);
  fsm_.setFailedStateTransitionAction( this, &gem::base::GEMApplication::transitionFailed);
  fsm_.setFailedStateTransitionChanged(this, &gem::base::GEMApplication::stateChanged);

  //Enable actions: from states Failed and Halted to Enabled via command Enable
  fsm_.addStateTransition('F', 'E',  "Enable", this, &gem::base::GEMApplication::enableAction);
  fsm_.addStateTransition('H', 'E',  "Enable", this, &gem::base::GEMApplication::enableAction);

  //Configure actions: from states Failed, Enabled, and Configured to Configured via command Configure
  fsm_.addStateTransition('F', 'C',  "Configure", this, &gem::base::GEMApplication::configureAction);
  fsm_.addStateTransition('E', 'C',  "Configure", this, &gem::base::GEMApplication::configureAction);
  fsm_.addStateTransition('C', 'C',  "Configure", this, &gem::base::GEMApplication::configureAction);

  /****** may be internal to the FSM and unnecessary to modify
  //Reset actions: from states Failed, Configured, Paused, and Running to Enabled via command Reset
  fsm_.addStateTransition('F', 'E',  "Reset",     this, &gem::base::GEMApplication::resetAction);
  fsm_.addStateTransition('C', 'E',  "Reset",     this, &gem::base::GEMApplication::resetAction);
  fsm_.addStateTransition('P', 'E',  "Reset",     this, &gem::base::GEMApplication::resetAction);
  //fsm_.addStateTransition('R', 'E',  "Reset",     this, &gem::base::GEMApplication::resetAction);
  ******/

  //Stop actions: from states Configured, Running, and Paused to Configured via command Stop
  fsm_.addStateTransition('C', 'C',  "Stop",      this, &gem::base::GEMApplication::stopAction);
  fsm_.addStateTransition('R', 'C',  "Stop",      this, &gem::base::GEMApplication::stopAction);
  fsm_.addStateTransition('P', 'C',  "Stop",      this, &gem::base::GEMApplication::stopAction);

  //Start/resume actions: from states Configured and Paused to Running via commands Start and Resume
  fsm_.addStateTransition('C', 'R',  "Start",     this, &gem::base::GEMApplication::startAction);
  fsm_.addStateTransition('P', 'R',  "Resume",    this, &gem::base::GEMApplication::resumeAction);

  //Pause action: from state Running to Paused via command Pause
  fsm_.addStateTransition('R', 'P',  "Pause",     this, &gem::base::GEMApplication::pauseAction);
  
  //Halt actions: from states Enabled, Configured, Running, Paused, and Halted to Halted via command Halt
  fsm_.addStateTransition('E', 'H',  "Halt",      this, &gem::base::GEMApplication::haltAction);
  fsm_.addStateTransition('C', 'H',  "Halt",      this, &gem::base::GEMApplication::haltAction);
  fsm_.addStateTransition('R', 'H',  "Halt",      this, &gem::base::GEMApplication::haltAction);
  fsm_.addStateTransition('P', 'H',  "Halt",      this, &gem::base::GEMApplication::haltAction);
  fsm_.addStateTransition('H', 'H',  "Halt",      this, &gem::base::GEMApplication::haltAction);

  //Set initial state to Halted
  fsm_.setInitialState('H');
  fsm_.reset();

  state_ = fsm_.getStateName(fsm_.getCurrentState());
  getApplicationInfoSpace()->fireItemAvailable("State",     &state_);
  getApplicationInfoSpace()->fireItemAvailable("stateName", &state_);

  reasonForFailure_ = "";

  getApplicationInfoSpace()->fireItemAvailable("reasonForFailure", &reasonForFailure_);
  
  //Get the RCMS state listener
  rcmsStateNotifier_.findRcmsStateListener();
  getApplicationInfoSpace()->fireItemAvailable("rcmsStateListener",      rcmsStateNotifier_.getRcmsStateListenerParameter());
  getApplicationInfoSpace()->fireItemAvailable("foundRcmsStateListener", rcmsStateNotifier_.getFoundRcmsStateListenerParameter());
    
  LOG4CPLUS_INFO(getApplicationLogger(), "gem::base::GEMApplication constructed");
}

// This is the callback used for setting parameters.                                                                                                                                                                                                                              
/******
 **no need to add this for the base GEMApplication class**
void gem::base::GEMApplication::actionPerformed(xdata::Event& event)
{
  // This is called after all default configuration values have been
  // loaded (from the XDAQ configuration file).
  if (event.type() == "urn:xdaq-event:setDefaultValues")
    {
      LOG4CPLUS_DEBUG(getApplicationLogger(), "GEMApplication::actionPerformed() Default configuration values have been loaded");
      //LOG4CPLUS_DEBUG(getApplicationLogger(), "GEMApplication::actionPerformed()   --> starting monitoring");
      //monitorP_->startMonitoring();
    }
}
******/

void gem::base::GEMApplication::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
  throw (toolbox::fsm::exception::Exception)
{
  state_ = fsm_.getStateName(fsm_.getCurrentState());
  LOG4CPLUS_DEBUG(getApplicationLogger(), "gem::base::GEM StateChanged: " << (std::string)state_);
}

void gem::base::GEMApplication::transitionFailed(toolbox::Event::Reference event)
  throw (toolbox::fsm::exception::Exception)
{
  toolbox::fsm::FailedEvent &fevent_ = dynamic_cast<toolbox::fsm::FailedEvent &>(*event);
  
  std::stringstream reason;
  reason << "<![CDATA[" 
	 << std::endl
	 << "Failure occurred when performing transition"
	 << " from "        << fevent_.getFromState()
	 << " to "          << fevent_.getToState()
    //<< ". Exception: " << xcept::stdformat_exception_history( fevent_.getException() )
	 << ". Exception: " << fevent_.getException().what()
	 << std::endl
	 << "]]>";
  
  reasonForFailure_ = reason.str();
  
  LOG4CPLUS_ERROR(getApplicationLogger(), reason.str());
}

xoap::MessageReference gem::base::GEMApplication::reset(xoap::MessageReference msgRef)
  throw (xoap::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(), "New state before reset is: "
		  << fsm_.getStateName (fsm_.getCurrentState()) );
  fsm_.reset();
  state_ = fsm_.getStateName(fsm_.getCurrentState());

  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope  envelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName  responseName = envelope.createName("ResetResponse", "xdaq", XDAQ_NS_URI);

  (void) envelope.getBody().addBodyElement ( responseName );

  LOG4CPLUS_INFO (getApplicationLogger(), "New state after reset is: "
		  << fsm_.getStateName (fsm_.getCurrentState()));
  return reply;
}

void gem::base::GEMApplication::fireEvent(std::string name)
  throw (toolbox::fsm::exception::Exception)
{
  toolbox::Event::Reference event((new toolbox::Event(name, this)));
  
  fsm_.fireEvent(event);
}

xoap::MessageReference gem::base::GEMApplication::fireEvent(xoap::MessageReference msgRef)
  throw (xoap::exception::Exception)
{

  xoap::SOAPPart     part = msgRef->getSOAPPart();
  xoap::SOAPEnvelope env  = part.getEnvelope();
  xoap::SOAPBody     body = env.getBody();

  DOMNode*     node = body.getDOMNode();
  DOMNodeList* bodyList = node->getChildNodes();

  for (unsigned int i = 0; i < bodyList->getLength(); i++)
    {
      DOMNode* command = bodyList->item(i);
      if (command->getNodeType() == DOMNode::ELEMENT_NODE)
	{
	  std::string commandName = xoap::XMLCh2String (command->getLocalName());
	  try
	    {
	      toolbox::Event::Reference e(new toolbox::Event(commandName,this));
	      fsm_.fireEvent(e);
	    }
	  catch (toolbox::fsm::exception::Exception & e)
	    {
	      XCEPT_RETHROW(xoap::exception::Exception, "invalid command", e);
	    }
	  xoap::MessageReference reply = xoap::createMessage();
	  xoap::SOAPEnvelope  envelope = reply->getSOAPPart().getEnvelope();
	  xoap::SOAPName  responseName = envelope.createName( commandName + "Response", "xdaq", XDAQ_NS_URI);
	  (void) envelope.getBody().addBodyElement ( responseName );
	  return reply;
	}
    }
  XCEPT_RAISE(xoap::exception::Exception,"command not found");
}

xoap::MessageReference gem::base::GEMApplication::onEnable(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  wl_->submit(enable_signature_);
  return createReply(message);
}

xoap::MessageReference gem::base::GEMApplication::onConfigure(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  run_number_ = 1;
  nevents_ = -1;
  
  wl_->submit(configure_signature_);
  return createReply(message);
}

xoap::MessageReference gem::base::GEMApplication::onStart(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  wl_->submit(start_signature_);
  return createReply(message);
}

xoap::MessageReference gem::base::GEMApplication::onPause(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  wl_->submit(pause_signature_);
  return createReply(message);
}

xoap::MessageReference gem::base::GEMApplication::onResume(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  wl_->submit(resume_signature_);
  return createReply(message);
}

xoap::MessageReference gem::base::GEMApplication::onStop(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  wl_->submit(stop_signature_);
  return createReply(message);
}

xoap::MessageReference gem::base::GEMApplication::onHalt(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  wl_->submit(halt_signature_);
  return createReply(message);
}

xoap::MessageReference gem::base::GEMApplication::onReset(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  resetAction();
  return onHalt(message);
}



/*To be filled in with the startup (enable) routine*/
void gem::base::GEMApplication::enableAction (toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(), e->type());
}

bool gem::base::GEMApplication::enableAction(toolbox::task::WorkLoop *wl)
{
  fireEvent("Enable");
  return false;
}

/*To be filled in with the configure routine*/
void gem::base::GEMApplication::configureAction (toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(), e->type());
}

bool gem::base::GEMApplication::configureAction(toolbox::task::WorkLoop *wl)
{
  fireEvent("Configure");
  return false;
}

/*To be filled in with the start routine*/
void gem::base::GEMApplication::startAction (toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(), e->type());
}

bool gem::base::GEMApplication::startAction(toolbox::task::WorkLoop *wl)
{
  fireEvent("Start");
  return false;
}


void gem::base::GEMApplication::pauseAction (toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(), e->type());
  // a failure is forced here
  XCEPT_RAISE(toolbox::fsm::exception::Exception,"error in pause");
}

bool gem::base::GEMApplication::pauseAction(toolbox::task::WorkLoop *wl)
{
  fireEvent("Pause");
  return false;
}

/*To be filled in with the resume routine*/
void gem::base::GEMApplication::resumeAction (toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(), e->type());
}

bool gem::base::GEMApplication::resumeAction(toolbox::task::WorkLoop *wl)
{
  fireEvent("Resume");
  return false;
}

/*To be filled in with the stop routine*/
void gem::base::GEMApplication::stopAction (toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(), e->type());
}

bool gem::base::GEMApplication::stopAction(toolbox::task::WorkLoop *wl)
{
  fireEvent("Stop");
  return false;
}

///*To be filled in with the reset routine*/
//void gem::base::GEMApplication::resetAction (toolbox::Event::Reference e)
//  throw (toolbox::fsm::exception::Exception)
//{
//  LOG4CPLUS_INFO (getApplicationLogger(), e->type());
//}

/*To be filled in with the halt routine*/
void gem::base::GEMApplication::haltAction (toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(), e->type());
}

bool gem::base::GEMApplication::haltAction(toolbox::task::WorkLoop *wl)
{
  fireEvent("Halt");
  
  return false;
}

void gem::base::GEMApplication::resetAction()
  throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_DEBUG(logger_, "reset(begin)");
  
  fsm_.reset();
  state_ = fsm_.getStateName(fsm_.getCurrentState());
  
  LOG4CPLUS_DEBUG(logger_, "reset(end)");
}

bool gem::base::GEMApplication::resetAction(toolbox::task::WorkLoop *wl)
{
  fireEvent("Reset");
  
  return false;
}

xoap::MessageReference gem::base::GEMApplication::createReply(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  std::string command = "";
  
  DOMNodeList *elements =
    message->getSOAPPart().getEnvelope().getBody().getDOMNode()->getChildNodes();
  
  for (unsigned int i = 0; i < elements->getLength(); i++) {
    DOMNode *e = elements->item(i);
    if (e->getNodeType() == DOMNode::ELEMENT_NODE) {
      command = xoap::XMLCh2String(e->getLocalName());
      break;
    }
  }
  
  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope envelope  = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName  = envelope.createName(
						     command + "Response", "xdaq", XDAQ_NS_URI);
  envelope.getBody().addBodyElement(responseName);
  
  return reply;
}
// End of file
