#include "gem/base/GEMFSM.h"

#include "gem/base/GEMFSMApplication.h"

#include "gem/base/utils/GEMLogging.h"
#include "gem/base/utils/GEMSOAPToolBox.h"
#include "gem/base/utils/exception/Exception.h"

#include "toolbox/fsm/AsynchronousFiniteStateMachine.h"
#include "toolbox/fsm/InvalidInputEvent.h"
#include "toolbox/string.h"
#include "xercesc/dom/DOMNode.hpp"
#include "xercesc/dom/DOMNodeList.hpp"
#include "xercesc/util/XercesDefs.hpp"
#include "xcept/tools.h"

gem::base::GEMFSM::GEMFSM(GEMFSMApplication* const gemAppP
			  //,gem::base::utils::ApplicationStateInfoSpaceHandler* const appStateInfoSpaceHanderP
			  ) :
  //appStateInfoSpaceHandlerP_(appStateInfoSpaceHanderP),
  gemfsmP_(0),
  gemAppP_(gemAppP),
  gemLogger_(gemAppP->getApplicationLogger()),
  gemRCMSNotifier_(gemAppP_->getApplicationLogger(),
		   gemAppP_->getApplicationDescriptor(),
		   gemAppP_->getApplicationContext())
{
  DEBUG("GEMFSM ctor begin");
  
  // Create the underlying Finite State Machine itself.
  std::stringstream commandLoopName;
  uint32_t instanceNumber = gemAppP_->getApplicationDescriptor()->getInstance();
  commandLoopName << "gemFSMCommandLoop_" << instanceNumber;
  gemfsmP_ = new toolbox::fsm::AsynchronousFiniteStateMachine(commandLoopName.str());

  // A map to look up the names of the 'intermediate' state transitions.
  lookupMap_["Initializing"] = "Initialized";// Halted
  lookupMap_["Configuring"]  = "Configured" ;// Configured
  lookupMap_["Starting"]     = "Started"    ;// Running
  lookupMap_["Halting"]      = "Halted"     ;// Halted
  lookupMap_["Pausing"]      = "Paused"     ;// Paused
  lookupMap_["Resuming"]     = "Resumed"    ;// Running
  lookupMap_["Stopping"]     = "Stopped"    ;// Configured

  // Define all states and transitions.
  // intermediate states (states entered when a transition is requested
  gemfsmP_->addState('c', "Configuring",  this, &gem::base::GEMFSM::stateChanged);
  gemfsmP_->addState('i', "Initializing", this, &gem::base::GEMFSM::stateChanged);
  gemfsmP_->addState('h', "Halting",      this, &gem::base::GEMFSM::stateChanged);
  gemfsmP_->addState('e', "Starting",     this, &gem::base::GEMFSM::stateChanged);
  gemfsmP_->addState('p', "Pausing",      this, &gem::base::GEMFSM::stateChanged);
  gemfsmP_->addState('r', "Resuming",     this, &gem::base::GEMFSM::stateChanged);
  gemfsmP_->addState('s', "Stopping",     this, &gem::base::GEMFSM::stateChanged);
  
  //what's in a name: Halted vs. Ready, Running vs. Enabled, Stopped vs. Ready
  gemfsmP_->addState('I', "Initial",     this, &gem::base::GEMFSM::stateChanged);
  gemfsmP_->addState('H', "Halted",      this, &gem::base::GEMFSM::stateChanged);
  gemfsmP_->addState('C', "Configured",  this, &gem::base::GEMFSM::stateChanged);
  gemfsmP_->addState('E', "Running",     this, &gem::base::GEMFSM::stateChanged);
  gemfsmP_->addState('P', "Paused",      this, &gem::base::GEMFSM::stateChanged);

  // BUG BUG BUG
  // Remove duplicate state transitions below!
  // Probably remove the 'Stop' transition from 'Failed' to 'Stopped.'
  // BUG BUG BUG end

  // Initialize: I -> H., connect hardware, perform basic checks, (load firware?)
  gemfsmP_->addStateTransition('I', 'H', "Initialize", gemAppP_, &gem::base::GEMFSMApplication::initializeAction);

  // Configure: H/C/E/P -> C., configure hardware, set default parameters
  gemfsmP_->addStateTransition('H', 'C', "Configure", gemAppP_, &gem::base::GEMFSMApplication::configureAction);
  gemfsmP_->addStateTransition('C', 'C', "Configure", gemAppP_, &gem::base::GEMFSMApplication::configureAction);
  gemfsmP_->addStateTransition('E', 'C', "Configure", gemAppP_, &gem::base::GEMFSMApplication::configureAction);
  gemfsmP_->addStateTransition('P', 'C', "Configure", gemAppP_, &gem::base::GEMFSMApplication::configureAction);

  // Start: C -> E., enable links for data to flow from front ends to back ends
  gemfsmP_->addStateTransition('C', 'E', "Start", gemAppP_, &gem::base::GEMFSMApplication::startAction);
  
  // Pause: E -> P. pause data flow, links stay alive, TTC/TTS counters stay active
  gemfsmP_->addStateTransition('E', 'P', "Pause", gemAppP_, &gem::base::GEMFSMApplication::pauseAction);
  
  // Resume: P -> E., resume data flow
  gemfsmP_->addStateTransition('P', 'E', "Resume", gemAppP_, &gem::base::GEMFSMApplication::resumeAction);
  
  // Stop: C/E/P -> C., stop data flow, disable links
  gemfsmP_->addStateTransition('C', 'C', "Stop", gemAppP_, &gem::base::GEMFSMApplication::stopAction);
  gemfsmP_->addStateTransition('E', 'C', "Stop", gemAppP_, &gem::base::GEMFSMApplication::stopAction);
  gemfsmP_->addStateTransition('P', 'C', "Stop", gemAppP_, &gem::base::GEMFSMApplication::stopAction);
  
  // Halt: C/E/F/H/P/ -> H., reset hardware state to pre-configured state
  gemfsmP_->addStateTransition('C', 'H', "Halt", gemAppP_, &gem::base::GEMFSMApplication::haltAction);
  gemfsmP_->addStateTransition('E', 'H', "Halt", gemAppP_, &gem::base::GEMFSMApplication::haltAction);
  gemfsmP_->addStateTransition('F', 'H', "Halt", gemAppP_, &gem::base::GEMFSMApplication::haltAction);
  gemfsmP_->addStateTransition('H', 'H', "Halt", gemAppP_, &gem::base::GEMFSMApplication::haltAction);
  gemfsmP_->addStateTransition('P', 'H', "Halt", gemAppP_, &gem::base::GEMFSMApplication::haltAction);
  
  // Error: I/H/C/E/P -> F.
  /*
    gemfsmP_->addStateTransition('I', 'F', "Error", gemAppP_, &GEMFSMApplication::);
    gemfsmP_->addStateTransition('H', 'F', "Error", gemAppP_, &GEMFSMApplication::);
    gemfsmP_->addStateTransition('C', 'F', "Error", gemAppP_, &GEMFSMApplication::);
    gemfsmP_->addStateTransition('E', 'F', "Error", gemAppP_, &GEMFSMApplication::);
    gemfsmP_->addStateTransition('P', 'F', "Error", gemAppP_, &GEMFSMApplication::);
  */
  
  gemfsmP_->setStateName('F', "Error");
  gemfsmP_->setFailedStateTransitionAction(      gemAppP_, &gem::base::GEMFSMApplication::failAction);
  gemfsmP_->setFailedStateTransitionChanged(     this,     &gem::base::GEMFSM::stateChanged);
  gemfsmP_->setInvalidInputStateTransitionAction(this,     &gem::base::GEMFSM::invalidAction);
  
  // Start out with the FSM in its initial state: Initial.
  gemfsmP_->setInitialState('I');
  gemfsmP_->reset();

  // Find connection to RCMS.
  gemRCMSNotifier_.findRcmsStateListener();

}


gem::base::GEMFSM::~GEMFSM()
{
  if (gemfsmP_)
    delete gemfsmP_;
  gemfsmP_ = 0;
}


xoap::MessageReference gem::base::GEMFSM::changeState(xoap::MessageReference msg)
//throw (toolbox::fsm::exception::Exception)
{
  if (msg.isNull()) {
    XCEPT_RAISE(xoap::exception::Exception,"Null Message!");
  }
  
  std::string commandName = "undefined";
  try {
    commandName = gem::base::utils::GEMSOAPToolBox::extractFSMCommandName(msg);
  }
  catch(xoap::exception::Exception& err) {
    std::string msgBase =
      toolbox::toString("Unable to extract command from GEMFSM SOAP message");
    ERROR(toolbox::toString("%s: %s.",
			    msgBase.c_str(),
			    xcept::stdformat_exception_history(err).c_str()));
    XCEPT_DECLARE_NESTED(gem::base::utils::exception::SOAPTransitionProblem, top,
			 toolbox::toString("%s.", msgBase.c_str()), err);
    gemAppP_->notifyQualified("error", top);
    std::string faultString = toolbox::toString("%s failed", commandName.c_str());
    std::string faultCode   = "Client";
    std::string detail      = toolbox::toString("%s: %s.",
						msgBase.c_str(),
						err.message().c_str());
    std::string faultActor = gemAppP_->getFullURL();
    xoap::MessageReference reply =
      gem::base::utils::GEMSOAPToolBox::makeSoapFaultReply(faultString, faultCode, detail, faultActor);
    return reply;
  }
  
  DEBUG(toolbox::toString("GEMFSM::changeState() received command '%s'.",
                          commandName.c_str()));
  
  try {
    toolbox::Event::Reference event(new toolbox::Event(commandName, this));
    gemfsmP_->fireEvent(event);
  }
  catch(toolbox::fsm::exception::Exception& err) {
    std::string msgBase =
      toolbox::toString("Problem executing the GEMFSM '%s' command",
			commandName.c_str());
    ERROR(toolbox::toString("%s: %s.",
			    msgBase.c_str(),
			    xcept::stdformat_exception(err).c_str()));
    XCEPT_DECLARE_NESTED(gem::base::utils::exception::SOAPTransitionProblem, top,
			 toolbox::toString("%s.", msgBase.c_str()), err);
    gemAppP_->notifyQualified("error", top);
    std::string faultString = toolbox::toString("%s failed", commandName.c_str());
    std::string faultCode   = "Server";
    std::string detail      = toolbox::toString("%s: %s.",
						msgBase.c_str(),
						err.message().c_str());
    std::string faultActor = gemAppP_->getFullURL();
    xoap::MessageReference reply =
      gem::base::utils::GEMSOAPToolBox::makeSoapFaultReply(faultString, faultCode, detail, faultActor);
    return reply;
  }
  
  //best way?  tcds had questions about this part
  std::string newStateName = commandName + "Triggered";

  // Once we get here, the state transition has been triggered. Notify
  // the requestor of the new state.
  try {
    xoap::MessageReference reply =
      gem::base::utils::GEMSOAPToolBox::makeFsmSoapReply(commandName, newStateName);
    return reply;
  }
  catch(xcept::Exception& err) {
    std::string msgBase =
      toolbox::toString("Failed to create GEMFSM SOAP reply for command '%s'",
			commandName.c_str());
    ERROR(toolbox::toString("%s: %s.",
			    msgBase.c_str(),
			    xcept::stdformat_exception(err).c_str()));
    XCEPT_DECLARE_NESTED(gem::base::utils::exception::SoftwareProblem, top,
			 toolbox::toString("%s.", msgBase.c_str()), err);
    gemAppP_->notifyQualified("error", top);
    XCEPT_RETHROW(xoap::exception::Exception, msgBase, err);
  }
  
  //XCEPT_RAISE(xoap::exception::Exception,"command not found");              
}


std::string gem::base::GEMFSM::getCurrentState() const
{
  return gemfsmP_->getStateName(gemfsmP_->getCurrentState());
}


void gem::base::GEMFSM::notifyRCMS(toolbox::fsm::FiniteStateMachine &fsm, std::string const msg)
  throw (toolbox::fsm::exception::Exception)
{
  // Notify RCMS of a state change.
  // NOTE: Should only be used for state _changes_.

  //toolbox::fsm::State currentState = fsm.getCurrentState();
  //std::string stateName            = fsm.getStateName(currentState);
  std::string stateName = fsm.getStateName(fsm.getCurrentState());
  DEBUG("notifyRCMS() called with msg = " << msg);
  try
    {
      gemRCMSNotifier_.stateChanged(stateName, msg);
    }
  catch(xcept::Exception& err)
    {
      ERROR("Failed to notify RCMS of state change: "
            << xcept::stdformat_exception_history(err));
      XCEPT_DECLARE_NESTED(gem::base::utils::exception::RCMSNotificationError, top,
                           "Failed to notify RCMS of state change.", err);
      gemAppP_->notifyQualified("error", top);
    }
}


void gem::base::GEMFSM::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
  throw (toolbox::fsm::exception::Exception)
{
  state_=fsm.getStateName(fsm.getCurrentState());
  //appStateInfoSpaceHandlerP_->setFSMState(state_);
  DEBUG("Current state is: [" << state_ << "]");
  // Send notification to Run Control
  try {
    DEBUG("Notifying Run Control of state change.");
    gemRCMSNotifier_.stateChanged((std::string)state_,"");
  }
  catch(xcept::Exception &e) {
    ERROR("Failed to notify Run Control of state change."
	  << xcept::stdformat_exception_history(e));
    stringstream ss2;
    ss2 << "Failed to notify Run Control of state change.";
    XCEPT_DECLARE_NESTED( gem::base::exception::Exception, eObj, ss2.str(), e );
    this->notifyQualified( "error", eObj );
  }
}


void gem::base::GEMFSM::invalidAction(toolbox::Event::Reference event)
//throw (toolbox::fsm::exception::Exception)
{
  /* what's the point of this action?  
   * should we go to failed or try to ensure no action is taken and the initial state is preserved?
   */
  toolbox::fsm::InvalidInputEvent& invalidInputEvent = dynamic_cast<toolbox::fsm::InvalidInputEvent&>(*event);
  std::string initialState   = gemfsmP_->getStateName(invalidInputEvent.fromState());
  std::string requestedState = invalidInputEvent.getInput();
  
  std::string message = toolbox::toString("An invalid state transition has been received:"
					  "requested transition to '%s' from '%s'.",
					  requestedState.c_str(), initialState.c_str());
  ERROR(message);
  gotoFailed(message);
}
