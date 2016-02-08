/**
 * class: GEMFSM
 * description: Generic FSM interface for GEM applications and devices
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: 
 * date: 
 */

#include "gem/base/GEMFSMApplication.h"
#include "gem/base/GEMFSM.h"
#include "gem/base/utils/GEMInfoSpaceToolBox.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"
#include "gem/utils/exception/Exception.h"

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
  //p_appStateInfoSpaceHandler(appStateInfoSpaceHanderP),
  p_gemfsm(0),
  p_gemApp(gemAppP),
  m_gemLogger(gemAppP->getApplicationLogger()),
  m_gemRCMSNotifier(p_gemApp->getApplicationLogger(),
                    p_gemApp->getApplicationDescriptor(),
                    p_gemApp->getApplicationContext())
{
  DEBUG("GEMFSM::ctor begin");
  
  // Create the underlying Finite State Machine itself.
  std::stringstream commandLoopName;
  uint32_t instanceNumber = p_gemApp->getApplicationDescriptor()->getInstance();
  std::string className   = p_gemApp->getApplicationDescriptor()->getClassName();

  //also want to get the name of the GEM FSM aplication to put it into this commandLoopName
  commandLoopName << "urn:toolbox-task-workloop:gemFSMCommandLoop:"
                  << className << ":" << instanceNumber;
  p_gemfsm = new toolbox::fsm::AsynchronousFiniteStateMachine(commandLoopName.str());

  // A map to look up the names of the 'intermediate' state transitions.
  //TCDS does things this way, is it the right way for GEMs?
  lookupMap_["Initializing"] = "Halted"     ;// Halted
  lookupMap_["Configuring"]  = "Configured" ;// Configured
  lookupMap_["Halting"]      = "Halted"     ;// Halted
  lookupMap_["Starting"]     = "Running"    ;// Running
  lookupMap_["Pausing"]      = "Paused"     ;// Paused
  lookupMap_["Resuming"]     = "Running"    ;// Running
  lookupMap_["Stopping"]     = "Configured" ;// Configured
  lookupMap_["Resetting"]    = "Initial"    ;// Resetting
  lookupMap_["Reset"]        = "Initial"    ;// Resetting

  // Define all states and transitions.
  /* intermediate states (states entered when a transition is requested*/
  p_gemfsm->addState(STATE_INITIALIZING, "Initializing", this, &gem::base::GEMFSM::stateChanged);
  p_gemfsm->addState(STATE_HALTING,      "Halting",      this, &gem::base::GEMFSM::stateChanged);
  p_gemfsm->addState(STATE_CONFIGURING,  "Configuring",  this, &gem::base::GEMFSM::stateChanged);
  p_gemfsm->addState(STATE_STARTING,     "Starting",     this, &gem::base::GEMFSM::stateChanged);
  p_gemfsm->addState(STATE_PAUSING,      "Pausing",      this, &gem::base::GEMFSM::stateChanged);
  p_gemfsm->addState(STATE_RESUMING,     "Resuming",     this, &gem::base::GEMFSM::stateChanged);
  p_gemfsm->addState(STATE_STOPPING,     "Stopping",     this, &gem::base::GEMFSM::stateChanged);
  p_gemfsm->addState(STATE_RESETTING,    "Resetting",    this, &gem::base::GEMFSM::stateChanged);

  /*terminal states*/
  p_gemfsm->addState(STATE_INITIAL,    "Initial",     this, &gem::base::GEMFSM::stateChanged);
  p_gemfsm->addState(STATE_HALTED,     "Halted",      this, &gem::base::GEMFSM::stateChanged);
  p_gemfsm->addState(STATE_CONFIGURED, "Configured",  this, &gem::base::GEMFSM::stateChanged);
  p_gemfsm->addState(STATE_RUNNING,    "Running",     this, &gem::base::GEMFSM::stateChanged);
  p_gemfsm->addState(STATE_PAUSED,     "Paused",      this, &gem::base::GEMFSM::stateChanged);

  /*State transitions*/
  // Initialize: I -> h., connect hardware, perform basic checks, (load firware?)
  p_gemfsm->addStateTransition(STATE_INITIAL, STATE_INITIALIZING, "Initialize", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);

  // Configure: H/C/E/P -> c., configure hardware, set default parameters, probably don't allow from running
  p_gemfsm->addStateTransition(STATE_HALTED,     STATE_CONFIGURING, "Configure", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_CONFIGURED, STATE_CONFIGURING, "Configure", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  /*p_gemfsm->addStateTransition(STATE_RUNNING,    STATE_CONFIGURING, "Configure", p_gemApp,
    &gem::base::GEMFSMApplication::transitionDriver);*/
  p_gemfsm->addStateTransition(STATE_PAUSED,     STATE_CONFIGURING, "Configure", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);

  // Start: C -> e., enable links for data to flow from front ends to back ends
  p_gemfsm->addStateTransition(STATE_CONFIGURED, STATE_STARTING, "Start", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  
  // Pause: E -> p. pause data flow, links stay alive, TTC/TTS counters stay active
  p_gemfsm->addStateTransition(STATE_RUNNING, STATE_PAUSING, "Pause", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  
  // Resume: P -> e., resume data flow
  p_gemfsm->addStateTransition(STATE_PAUSED, STATE_RESUMING, "Resume", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  
  // Stop: C/E/P -> s., stop data flow, disable links
  p_gemfsm->addStateTransition(STATE_CONFIGURED, STATE_STOPPING, "Stop", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_RUNNING,    STATE_STOPPING, "Stop", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_PAUSED,     STATE_STOPPING, "Stop", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  
  // Halt: C/E/F/H/P/ -> h., halt hardware state to pre-configured state
  p_gemfsm->addStateTransition(STATE_CONFIGURED, STATE_HALTING, "Halt", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_RUNNING,    STATE_HALTING, "Halt", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_FAILED,     STATE_HALTING, "Halt", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_HALTED,     STATE_HALTING, "Halt", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_PAUSED,     STATE_HALTING, "Halt", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  
  // Reset the state machine: I/H/C/E/P/F -> t.
  /*benefit of using dedicated resetAction vs transitionDriver and a workloop that
    will implement derived class specific features?*/
  p_gemfsm->addStateTransition(STATE_INITIAL,    STATE_RESETTING, "Reset", p_gemApp,
                               &GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_HALTED,     STATE_RESETTING, "Reset", p_gemApp,
                               &GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_CONFIGURED, STATE_RESETTING, "Reset", p_gemApp,
                               &GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_RUNNING,    STATE_RESETTING, "Reset", p_gemApp,
                               &GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_PAUSED,     STATE_RESETTING, "Reset", p_gemApp,
                               &GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_FAILED,     STATE_RESETTING, "Reset", p_gemApp,
                               &GEMFSMApplication::transitionDriver);


  // intermediate to terminal transitions:
  // i/h -> H
  p_gemfsm->addStateTransition(STATE_INITIALIZING, STATE_HALTED,     "IsHalted",     p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_HALTING,      STATE_HALTED,     "IsHalted",     p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  // c/s -> C
  p_gemfsm->addStateTransition(STATE_CONFIGURING,  STATE_CONFIGURED, "IsConfigured", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_STOPPING,     STATE_CONFIGURED, "IsConfigured", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  // e/r -> E
  p_gemfsm->addStateTransition(STATE_STARTING,     STATE_RUNNING,    "IsRunning",    p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_RESUMING,     STATE_RUNNING,    "IsRunning",    p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  // p -> P
  p_gemfsm->addStateTransition(STATE_PAUSING,      STATE_PAUSED,     "IsPaused",     p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  // t -> I
  p_gemfsm->addStateTransition(STATE_RESETTING,    STATE_INITIAL,    "IsInitial",    p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  
  // Invalid transitions that should result in no action taken
  /*
    p_gemfsm->addStateTransition(STATE_RESETTING,    STATE_INITIAL,    "IsInitial",    p_gemApp,
    &gem::base::GEMFSMApplication::noAction);
  */
  // State transition failures: i/c/h/p/s/e/r/t -> F., go to failed
  p_gemfsm->addStateTransition(STATE_INITIALIZING, STATE_FAILED, "Fail", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_CONFIGURING,  STATE_FAILED, "Fail", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_HALTING,      STATE_FAILED, "Fail", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_PAUSING,      STATE_FAILED, "Fail", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_STOPPING,     STATE_FAILED, "Fail", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_STARTING,     STATE_FAILED, "Fail", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_RESUMING,     STATE_FAILED, "Fail", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->addStateTransition(STATE_RESETTING,    STATE_FAILED, "Fail", p_gemApp,
                               &gem::base::GEMFSMApplication::transitionDriver);
  

  p_gemfsm->setStateName(STATE_FAILED, "Error");
  p_gemfsm->setFailedStateTransitionAction(      p_gemApp, &gem::base::GEMFSMApplication::transitionDriver);
  p_gemfsm->setFailedStateTransitionChanged(     this,     &gem::base::GEMFSM::stateChanged);
  p_gemfsm->setInvalidInputStateTransitionAction(this,     &gem::base::GEMFSM::invalidAction);
  
  // Start out with the FSM in its initial state: Initial.
  p_gemfsm->setInitialState(STATE_INITIAL);
  p_gemfsm->reset();
  
  m_gemFSMState = p_gemfsm->getStateName(p_gemfsm->getCurrentState());
  DEBUG("GEMFSM::GEMFSM current state is " << m_gemFSMState.toString());

  p_gemApp->getAppISToolBox()->createString("FSMState",        m_gemFSMState.toString(), utils::GEMInfoSpaceToolBox::PROCESS);
  p_gemApp->getAppISToolBox()->createString("ReasonForFailure","",                       utils::GEMInfoSpaceToolBox::PROCESS);
  
  //p_gemApp->getApplicationInfoSpace()->fireItemAvailable("FSMState",        &m_gemFSMState);
  //p_gemApp->getApplicationInfoSpace()->fireItemAvailable("ReasonForFailure",&m_reasonForFailure);
  p_gemApp->getApplicationInfoSpace()->fireItemValueRetrieve("FSMState");
  p_gemApp->getApplicationInfoSpace()->fireItemValueRetrieve("ReasonForFailure");

  // // Find connection to RCMS.
  //   p_gemApp->getApplicationInfoSpace()->fireItemAvailable("rcmsStateListener",      
  // 							 m_gemRCMSNotifier.getRcmsStateListenerParameter());
  //   p_gemApp->getApplicationInfoSpace()->fireItemAvailable("foundRcmsStateListener", 
  // 							 m_gemRCMSNotifier.getFoundRcmsStateListenerParameter());

  //   m_gemRCMSNotifier.findRcmsStateListener();

  //   m_gemRCMSNotifier.subscribeToChangesInRcmsStateListener(p_gemApp->getApplicationInfoSpace()); 
}


gem::base::GEMFSM::~GEMFSM()
{
  if (p_gemfsm)
    delete p_gemfsm;
  p_gemfsm = 0;
}


void gem::base::GEMFSM::fireEvent(toolbox::Event::Reference const &event)
{
  INFO("GEMFSM::fireEvent(" << event->type() << ")");
  try {
    p_gemfsm->fireEvent(event);
  } catch (::toolbox::fsm::exception::Exception & ex) {
    XCEPT_RETHROW(::xoap::exception::Exception, "invalid command", ex);
  }
};
	
xoap::MessageReference gem::base::GEMFSM::changeState(xoap::MessageReference msg)
//throw (toolbox::fsm::exception::Exception)
{
  INFO("GEMFSM::changeState()");
  if (msg.isNull()) {
    XCEPT_RAISE(xoap::exception::Exception,"Null message received!");
  }
  
  std::string commandName = "undefined";
  try {
    commandName = gem::utils::soap::GEMSOAPToolBox::extractFSMCommandName(msg);
    INFO("GEMFSM::received command " << commandName);
  } catch(xoap::exception::Exception& err) {
    std::string msgBase = toolbox::toString("Unable to extract command from GEMFSM SOAP message");
    ERROR(toolbox::toString("%s: %s.", msgBase.c_str(), xcept::stdformat_exception_history(err).c_str()));
    XCEPT_DECLARE_NESTED(gem::base::utils::exception::SOAPTransitionProblem, top,
                         toolbox::toString("%s.", msgBase.c_str()), err);
    p_gemApp->notifyQualified("error", top);
    std::string faultString = toolbox::toString("%s failed", commandName.c_str());
    std::string faultCode   = "Client";
    std::string detail      = toolbox::toString("%s: %s.",
                                                msgBase.c_str(),
                                                err.message().c_str());
    std::string faultActor = p_gemApp->getFullURL();
    xoap::MessageReference reply =
      gem::utils::soap::GEMSOAPToolBox::makeSOAPFaultReply(faultString, faultCode, detail, faultActor);
    return reply;
  }
  
  DEBUG("GEMFSM::changeState() received command '" <<  commandName.c_str() << "'.");

  try {
    toolbox::Event::Reference event(new toolbox::Event(commandName, this));
    INFO("Firing GEMFSM for event " << commandName);
    INFO("initial state is: " << p_gemfsm->getStateName(p_gemfsm->getCurrentState()));
    p_gemfsm->fireEvent(event);
    INFO("new state is: " << p_gemfsm->getStateName(p_gemfsm->getCurrentState()));
  } catch(toolbox::fsm::exception::Exception& err) {
    std::string msgBase = toolbox::toString("Problem executing the GEMFSM '%s' command",commandName.c_str());
    ERROR(toolbox::toString("%s: %s.", msgBase.c_str(), xcept::stdformat_exception(err).c_str()));
    XCEPT_DECLARE_NESTED(gem::base::utils::exception::SOAPTransitionProblem, top,
                         toolbox::toString("%s.", msgBase.c_str()), err);
    p_gemApp->notifyQualified("error", top);
    std::string faultString = toolbox::toString("%s failed", commandName.c_str());
    std::string faultCode   = "Server";
    std::string detail      = toolbox::toString("%s: %s.", msgBase.c_str(), err.message().c_str());
    std::string faultActor = p_gemApp->getFullURL();

    //should we go to failed here?
    gotoFailedAsynchronously(err);
    /*xoap::MessageReference reply =
      gem::utils::soap::GEMSOAPToolBox::makeSOAPFaultReply(faultString, faultCode, detail, faultActor);
      return reply;*/
    return gem::utils::soap::GEMSOAPToolBox::makeSOAPFaultReply(faultString, faultCode, detail, faultActor);
  }
  
  ////best way?  tcds had questions about this part
  //std::string newStateName = commandName + "Triggered";

  //the HCAL way
  std::string newStateName = p_gemfsm->getStateName(p_gemfsm->getCurrentState());
  // Once we get here, the state transition has been triggered. Notify
  // the requestor of the new state.
  try {
    INFO("changeState::sending command " << commandName << " newStateName " << newStateName);
    /*xoap::MessageReference reply = 
      gem::utils::soap::GEMSOAPToolBox::makeFSMSOAPReply(commandName, newStateName);
      return reply;*/
    return
      gem::utils::soap::GEMSOAPToolBox::makeFSMSOAPReply(commandName, 
                                                         p_gemfsm->getStateName(p_gemfsm->getCurrentState()));
    //newStateName);
  } catch(xcept::Exception& err) {
    std::string msgBase = toolbox::toString("Failed to create GEMFSM SOAP reply for command '%s'",
                                            commandName.c_str());
    ERROR(toolbox::toString("%s: %s.", msgBase.c_str(), xcept::stdformat_exception(err).c_str()));
    XCEPT_DECLARE_NESTED(gem::base::utils::exception::SoftwareProblem,
                         top, toolbox::toString("%s.",msgBase.c_str()), err);
    p_gemApp->notifyQualified("error", top);

    gotoFailedAsynchronously(err);
    XCEPT_RETHROW(xoap::exception::Exception, msgBase, err);
  }
  
  XCEPT_RAISE(xoap::exception::Exception,"command not found");
}


std::string gem::base::GEMFSM::getCurrentState() const
{
  INFO("GEMFSM::getCurrentState()");
  return p_gemfsm->getStateName(p_gemfsm->getCurrentState());
}

std::string gem::base::GEMFSM::getStateName(toolbox::fsm::State const& state) const
{
  INFO("GEMFSM::getStateName()");
  return p_gemfsm->getStateName(state);
}

void gem::base::GEMFSM::notifyRCMS(toolbox::fsm::FiniteStateMachine &fsm, std::string const msg)
  throw (toolbox::fsm::exception::Exception)
{
  INFO("GEMFSM::notifyRCMS()");
  // Notify RCMS of a state change.
  // NOTE: Should only be used for state _changes_.

  //toolbox::fsm::State currentState = fsm.getCurrentState();
  //std::string stateName            = fsm.getStateName(currentState);
  std::string stateName = fsm.getStateName(fsm.getCurrentState());
  DEBUG("notifyRCMS() called with msg = " << msg);
  try {
    m_gemRCMSNotifier.stateChanged(stateName, msg);
  } catch(xcept::Exception& err) {
    ERROR("GEMFSM::Failed to notify RCMS of state change: "
          << xcept::stdformat_exception_history(err));
    XCEPT_DECLARE_NESTED(gem::base::utils::exception::RCMSNotificationError, top,
                         "Failed to notify RCMS of state change.", err);
    p_gemApp->notifyQualified("error", top);
  }
}


void gem::base::GEMFSM::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
  throw (toolbox::fsm::exception::Exception)
{
  INFO("GEMFSM::stateChanged() begin");
  m_gemFSMState = fsm.getStateName(fsm.getCurrentState());
  //p_appStateInfoSpaceHandler->setFSMState(state_);
  p_gemApp->getAppISToolBox()->setString("FSMState",m_gemFSMState.toString());
  INFO("GEMFSM::stateChanged:Current state is: [" << m_gemFSMState.toString() << "]");
  /* TCDS way
  // Send notification to Run Control
  notifyRCMS(fsm, "Normal state change.");

  std::map<std::string, std::string>::const_iterator iter = lookupMap_.find(state_);
  if (iter != lookupMap_.end()) {
    std::string commandName = iter->second;
    INFO("GEMFSM::stateChanged:DEBUG JGH '" << state_
          << "' is an intermediate state --> forwarding to '"
          << commandName << "'");
    
    try {
      toolbox::Event::Reference event(new toolbox::Event(commandName, this));
      p_gemfsm->fireEvent(event);
    } catch(toolbox::fsm::exception::Exception& err) {
      std::string msgBase =
        toolbox::toString("Problem executing the GEMFSM '%s' command",
                          commandName.c_str());
      ERROR(toolbox::toString("%s: %s.",
                              msgBase.c_str(),
                              xcept::stdformat_exception(err).c_str()));
      XCEPT_DECLARE_NESTED(gem::base::utils::exception::TransitionProblem, top,
                           toolbox::toString("%s.", msgBase.c_str()), err);
      p_gemApp->notifyQualified("error", top);
    }
  }
  else {
    INFO("GEMFSM::stateChanged:DEBUG JGH '" << state_ << "' is not an intermediate state");
    }
  */
  INFO("GEMFSM::stateChanged:stateChanged() end");

}


void gem::base::GEMFSM::invalidAction(toolbox::Event::Reference event)
//throw (toolbox::fsm::exception::Exception)
{
  /* what's the point of this action?  
   * should we go to failed or try to ensure no action is taken and the initial state is preserved?
   */
  INFO("GEMFSM::invalidAction(" << event->type() << ")");
  toolbox::fsm::InvalidInputEvent& invalidInputEvent = dynamic_cast<toolbox::fsm::InvalidInputEvent&>(*event);
  std::string initialState   = p_gemfsm->getStateName(invalidInputEvent.getFromState());
  std::string requestedState = invalidInputEvent.getInput();
  
  std::string message = toolbox::toString("An invalid state transition has been received:"
                                          " requested transition to '%s' from '%s'.",
                                          requestedState.c_str(), initialState.c_str());
  ERROR("GEMFSM::" << message);
  gotoFailed(message);
}

void gem::base::GEMFSM::gotoFailed(std::string const reason)
{
  INFO("GEMFSM::gotoFailed(std::string)");
  //p_appStateInfoSpaceHandler->setFSMState("Failed", reason);
  ERROR("GEMFSM::Going to 'Failed' state. Reason: '" << reason << "'.");
  XCEPT_RAISE(toolbox::fsm::exception::Exception, reason);
}

void gem::base::GEMFSM::gotoFailed(xcept::Exception& err)
{
  INFO("GEMFSM::gotoFailed(xcept::Exception&)");
  std::string reason = err.message();
  gotoFailed(reason);
}

void gem::base::GEMFSM::gotoFailedAsynchronously(xcept::Exception& err)
{
  INFO("GEMFSM::gotoFailedAsynchronously(xcept::Exception&)");
  std::string reason = err.message();
  //p_appStateInfoSpaceHandler->setFSMState("Failed", reason);
  ERROR("GEMFSM::Going to 'Failed' state. Reason: " << reason);
  try {
    toolbox::Event::Reference event(new toolbox::Event("Fail", this));
    p_gemfsm->fireEvent(event);
  } catch(xcept::Exception& error) {
    std::string msg = "Cannot initiate asynchronous 'Fail' transition.";
    FATAL(msg);
    XCEPT_DECLARE_NESTED(gem::utils::exception::SoftwareProblem, top, msg, error);
    p_gemApp->notifyQualified("fatal", top);
  }
}
