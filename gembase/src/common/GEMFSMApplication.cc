/**
 * class: GEMFSMApplication
 * description: Generic GEM application with FSM interface
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date: 
 */

#include "gem/base/GEMFSMApplication.h"
#include "gem/base/GEMWebApplication.h"
#include "gem/base/GEMFSM.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"

#include "gem/utils/exception/Exception.h"
#include "gem/base/exception/Exception.h"
#include "gem/base/utils/exception/Exception.h"

#include "toolbox/fsm/AsynchronousFiniteStateMachine.h"
#include "toolbox/fsm/InvalidInputEvent.h"
#include "toolbox/fsm/FailedEvent.h"

#include "toolbox/task/WorkLoopFactory.h"
#include "toolbox/BSem.h"

#include "toolbox/string.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/NamespaceURI.h"
#include "xcept/Exception.h"

#include "xgi/framework/Method.h"
#include "xgi/framework/UIManager.h"

#include "xgi/Input.h"
#include "xgi/Method.h"
#include "xgi/Output.h"

#include "xoap/Method.h"


gem::base::GEMFSMApplication::GEMFSMApplication(xdaq::ApplicationStub* stub)
  throw (xdaq::exception::Exception) :
  GEMApplication(stub),
  m_gemfsm(this),//, &m_gemAppStateInfoSpace)
  m_progress(0.0),
  b_accept_web_commands(true),
  m_wl_semaphore(toolbox::BSem::FULL),
  m_db_semaphore(toolbox::BSem::FULL),
  m_cfg_semaphore(toolbox::BSem::FULL),
  m_web_semaphore(toolbox::BSem::FULL),
  m_infspc_semaphore(toolbox::BSem::FULL)
{
  INFO("GEMFSMApplication ctor begin");

  // These bindings expose the state machine to the hyperdaq world. The
  // xgi<Action> callback simply creates a SOAP message that then triggers the usual
  // state transition
  DEBUG("Creating xgi bindings...");
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiInitialize, "Initialize");
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiConfigure,  "Configure" );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiStart,      "Start"     );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiStop,       "Stop"      );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiPause,      "Pause"     );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiResume,     "Resume"    );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiHalt,       "Halt"      );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiReset,      "Reset"     );
  DEBUG("Created xgi bindings");

  // These bindings expose the state machine to the outside world. The
  // changeState() method simply forwards the calls to the GEMFSM
  // object.
  xoap::bind(this, &GEMFSMApplication::changeState, "Initialize", XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Configure",  XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Start",      XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Stop",       XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Pause",      XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Resume",     XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Halt",       XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Reset",      XDAQ_NS_URI);
  DEBUG("Created xoap bindings");

  //benefit or disadvantage to setting up the workloop signatures this way?
  //hcal has done a forwarding which may be a clever solution, but to what problem?
  initSig_   = toolbox::task::bind(this, &GEMFSMApplication::initialize, "initialize");
  confSig_   = toolbox::task::bind(this, &GEMFSMApplication::configure,  "configure" );
  startSig_  = toolbox::task::bind(this, &GEMFSMApplication::start,      "start"     );
  stopSig_   = toolbox::task::bind(this, &GEMFSMApplication::stop,       "stop"      );
  pauseSig_  = toolbox::task::bind(this, &GEMFSMApplication::pause,      "pause"     );
  resumeSig_ = toolbox::task::bind(this, &GEMFSMApplication::resume,     "resume"    );
  haltSig_   = toolbox::task::bind(this, &GEMFSMApplication::halt,       "halt"      );
  resetSig_  = toolbox::task::bind(this, &GEMFSMApplication::reset,      "reset"     );
  DEBUG("Created task bindings");

  std::stringstream tmpLoopName;
  uint32_t instanceNumber = this->getApplicationDescriptor()->getInstance();
  std::string className   = this->getApplicationDescriptor()->getClassName();
  DEBUG("Obtained instance and class names " << instanceNumber << ", " << className);

  //also want to get the name of the GEM FSM aplication to put it into this commandLoopName
  tmpLoopName << "urn:toolbox-task-workloop:"
              << className << ":" << instanceNumber;
  workLoopName = tmpLoopName.str();
  DEBUG("Created workloop name " << workLoopName);
    
  p_appInfoSpace->addListener(this, "urn:xdaq-event:setDefaultValues");
  //p_gemAppStateInfoSpace->addListener(this, "urn:xdaq-event:setDefaultValues");
  //p_gemAppStateInfoSpace->addItemRetrieveListener( "GEMFSMState", this);
  //p_gemAppStateInfoSpace->addItemChangedListener(  "GEMFSMState", this);
  //p_gemAppStateInfoSpace->addGroupRetrieveListener("GEMFSMState", this);
  //p_gemAppStateInfoSpace->addGroupChangedListener( "GEMFSMState", this);
  p_appInfoSpace->fireItemAvailable("application:state", p_gemAppStateInfoSpace );
  //p_gemAppStateInfoSpace->fireItemAvailable("State",&m_state);
  p_appInfoSpace->fireItemAvailable("State",&m_state);

  //appStateInfoSpace_.setFSMState(m_gemfsm.getCurrentStateName());

  INFO("GEMFSMApplication ctor end");
}

gem::base::GEMFSMApplication::~GEMFSMApplication()
{
  INFO("GEMFSMApplication dtor begin");
  INFO("GEMFSMApplication dtor end");
}

/**hyperdaq callbacks*/
void gem::base::GEMFSMApplication::xgiInitialize(xgi::Input* in, xgi::Output* out)
{
  INFO("xgiInitialize begin");
  if (b_accept_web_commands) {
    try {
      INFO("xgiInitialize::Sending SOAP command to application");
      gem::utils::soap::GEMSOAPToolBox::sendCommand("Initialize",p_appContext,p_appDescriptor,p_appDescriptor);
    } catch (toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Initialize failed", e );
    }
  } //is it OK to then call webInitialize?
  INFO("xgiInitialize end");
  p_gemWebInterface->webInitialize(in,out);
}

void gem::base::GEMFSMApplication::xgiConfigure(xgi::Input* in, xgi::Output* out)
{
  if (b_accept_web_commands) {
    try {
      DEBUG("Sending SOAP command to application");
      gem::utils::soap::GEMSOAPToolBox::sendCommand("Configure",p_appContext,p_appDescriptor,p_appDescriptor);
    } catch (toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Configure failed", e );
    }
  }
  p_gemWebInterface->webConfigure(in,out);
}

void gem::base::GEMFSMApplication::xgiStart(xgi::Input* in, xgi::Output* out)
{
  if (b_accept_web_commands) {
    try {
      DEBUG("Sending SOAP command to application");
      gem::utils::soap::GEMSOAPToolBox::sendCommand("Start",p_appContext,p_appDescriptor,p_appDescriptor);
    } catch (toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Start failed", e );
    }
  }
  p_gemWebInterface->webStart(in,out);
}

void gem::base::GEMFSMApplication::xgiStop(xgi::Input* in, xgi::Output* out)
{
  if (b_accept_web_commands) {
    try {
      DEBUG("Sending SOAP command to application");
      gem::utils::soap::GEMSOAPToolBox::sendCommand("Stop",p_appContext,p_appDescriptor,p_appDescriptor);
    } catch (toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Stop failed", e );
    }
  }
  p_gemWebInterface->webStop(in,out);
}

void gem::base::GEMFSMApplication::xgiPause(xgi::Input* in, xgi::Output* out)
{
  if (b_accept_web_commands) {
    try {
      DEBUG("Sending SOAP command to application");
      gem::utils::soap::GEMSOAPToolBox::sendCommand("Pause",p_appContext,p_appDescriptor,p_appDescriptor);
    } catch (toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Pause failed", e );
    }
  }
  p_gemWebInterface->webPause(in,out);
}

void gem::base::GEMFSMApplication::xgiResume(xgi::Input* in, xgi::Output* out)
{
  if (b_accept_web_commands) {
    try {
      DEBUG("Sending SOAP command to application");
      gem::utils::soap::GEMSOAPToolBox::sendCommand("Resume",p_appContext,p_appDescriptor,p_appDescriptor);
    } catch (toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Resume failed", e );
    }
  }
  p_gemWebInterface->webResume(in,out);
}

void gem::base::GEMFSMApplication::xgiHalt(xgi::Input* in, xgi::Output* out)
{
  if (b_accept_web_commands) {
    try {
      DEBUG("Sending SOAP command to application");
      gem::utils::soap::GEMSOAPToolBox::sendCommand("Halt",p_appContext,p_appDescriptor,p_appDescriptor);
    } catch (toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Halt failed", e );
    }
  }
  p_gemWebInterface->webHalt(in,out);
}

void gem::base::GEMFSMApplication::xgiReset(xgi::Input* in, xgi::Output* out)
{
  if (b_accept_web_commands) {
    try {
      DEBUG("Sending SOAP command to application");
      gem::utils::soap::GEMSOAPToolBox::sendCommand("Reset",p_appContext,p_appDescriptor,p_appDescriptor);
    } catch (toolbox::fsm::exception::Exception& e ) {
      XCEPT_RETHROW( xgi::exception::Exception, "Reset failed", e );
    }
  }  
  p_gemWebInterface->webReset(in,out);
}


/**state transitions*/
void gem::base::GEMFSMApplication::transitionDriver(toolbox::Event::Reference event)
  throw (toolbox::fsm::exception::Exception)
{
  //set a transition message to ""
  INFO("GEMFSMApplication::transitionDriver(" << event->type() << ")");
  try {
    if (event->type() == "Initialize" || event->type() == "Configure" || event->type() == "Start"  ||
        event->type() == "Stop"       || event->type() == "Pause"     || event->type() == "Resume" || 
        event->type() == "Halt"       || event->type() == "Reset" ) {
      INFO("transitionDriver::submitting workloopDriver(" << event->type() << ")");
      workloopDriver(event->type());
      //does this preclude the future "success" message at the end of the catch block?
      return;
    } else if (event->type() == "IsInitial" || event->type() == "IsConfigured" ||
               event->type() == "IsRunning" || event->type() == "IsPaused"     ||
               event->type() == "IsHalted") {
      //report success
      INFO("Recieved confirmation that state changed to " << event->type());
    } else if (event->type()=="Fail" || event->type()=="fail") {
      //do nothing for the fail action
      INFO("Recieved fail event type");
    } else {
      INFO("Unknown transition command");
      XCEPT_RAISE(toolbox::fsm::exception::Exception,"Unknown transition command");
    }
  } catch (gem::utils::exception::Exception& ex) {
    INFO("Caught gem::utils::exception::Exception");
    fireEvent("Fail");
    //set a transition message to ex.what()
    XCEPT_RETHROW(toolbox::fsm::exception::Exception,"State Transition Failed",ex);
  } /*catch (std::exception& ex) {
    fireEvent("Fail");
    //set a transition message to ex.what()
    XCEPT_RETHROW(toolbox::fsm::exception::Exception,"State Transition Failed",ex);
  } catch (...) {
    fireEvent("Fail");
    //set a transition message to ex.what()
    XCEPT_RETHROW(toolbox::fsm::exception::Exception,"State Transition Failed","...");
    }*/
  //set a transition message to "Success"
}

void gem::base::GEMFSMApplication::workloopDriver(std::string const& command)
  throw (toolbox::task::exception::Exception)
{
  INFO("workloopDriver begin");
  try {
    toolbox::task::WorkLoopFactory* wlf  = toolbox::task::WorkLoopFactory::getInstance();
    INFO("Trying to access the workloop with name " << workLoopName);
    toolbox::task::WorkLoop*        loop = wlf->getWorkLoop(workLoopName,"waiting");
    if (!loop->isActive()) loop->activate();
    INFO("Workloop should now be active");

    if      (command=="Initialize") loop->submit(initSig_  );
    else if (command=="Configure")  loop->submit(confSig_  );
    else if (command=="Start")      loop->submit(startSig_ );
    else if (command=="Stop")       loop->submit(stopSig_  );
    else if (command=="Pause")      loop->submit(pauseSig_ );
    else if (command=="Resume")     loop->submit(resumeSig_);
    else if (command=="Halt")       loop->submit(haltSig_  );
    else if (command=="Reset")      loop->submit(resetSig_ );
    INFO("Workloop should now be submitted");
  } catch (toolbox::task::exception::Exception& e) {
    XCEPT_RETHROW(gem::utils::exception::Exception,"Workloop failure",e);
  }
  INFO("workloopDriver end");
}

void gem::base::GEMFSMApplication::resetAction(toolbox::Event::Reference event)
  throw (toolbox::fsm::exception::Exception)
{
  INFO("GEMFSMApplication::resetAction(" << event->type() << ")");
  //should only enter this function on reciept of a "Reset" event
  //should probably do much more than this...
  INFO("Firing 'IsInitial' into the FSM");
  fireEvent("IsInitial");
}

/*	
        void gem::base::GEMFSMApplication::failAction(toolbox::Event::Reference event)
        throw (toolbox::fsm::exception::Exception)
        {
        }
*/	

void gem::base::GEMFSMApplication::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
  throw (toolbox::fsm::exception::Exception)
{
  INFO("GEMFSMApplication::stateChanged");
}

void gem::base::GEMFSMApplication::transitionFailed(toolbox::Event::Reference event)
  throw (toolbox::fsm::exception::Exception)
{
  INFO("GEMFSMApplication::transitionFailed(" <<event->type() << ")");
}

void gem::base::GEMFSMApplication::fireEvent(std::string event)
  throw (toolbox::fsm::exception::Exception)
{
  INFO("GEMFSMApplication::fireEvent(" << event << ")");
  try {
    toolbox::Event::Reference e(new toolbox::Event(event,this));
    m_gemfsm.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    XCEPT_RETHROW(::xoap::exception::Exception, "invalid command", e);
  }
}

 
/**SOAP callback*/
// This simply forwards the message to the GEMFSM object, since it is
// technically not possible to bind directly to anything but an
// xdaq::Application.
xoap::MessageReference gem::base::GEMFSMApplication::changeState(xoap::MessageReference msg)
{
  INFO("GEMFSMApplication::changeState");
  return m_gemfsm.changeState(msg);
}

/** workloop driven transitions*/
bool gem::base::GEMFSMApplication::initialize(toolbox::task::WorkLoop *wl)
{
  m_wl_semaphore.take();
  INFO("initialize called, current state: " << m_gemfsm.getCurrentState());
  while ((m_gemfsm.getCurrentState()) != m_gemfsm.getStateName(STATE_INITIALIZING)) { // deal with possible race condition
    INFO("not in " << STATE_INITIALIZING << " sleeping (" << m_gemfsm.getCurrentState() << ")");
    usleep(100);
  }
  INFO("initialize called, current state: " << m_gemfsm.getCurrentState());
  
  p_gemWebInterface->buildCfgWebpage(); // Set up the basic config web page from the GEMWebApplication

  try {
    m_progress = 0.0;
    INFO("Calling initializeAction");
    this->initializeAction();
    INFO("Finished initializeAction");
    p_gemWebInterface->buildCfgWebpage(); // complete, so re render the config web page
    m_progress = 1.0;
  } catch (gem::utils::exception::Exception const& ex) {
    ERROR("Error in initialize gem::utils::exception " << ex.what());
    toolbox::Event::Reference e(new toolbox::Event("Fail",this));
    fireEvent("Fail");
    m_wl_semaphore.give();
    return false;
  } catch (toolbox::net::exception::MalformedURN const& ex) {
    ERROR("Error in initialize, malformed URN " << ex.what());
    toolbox::Event::Reference e(new toolbox::Event("Fail",this));
    fireEvent("Fail");
    m_wl_semaphore.give();
    return false;
  } catch (std::exception const& ex) {
    ERROR("Error in initialize, std::exception " << ex.what());
    toolbox::Event::Reference e(new toolbox::Event("Fail",this));
    fireEvent("Fail");
    m_wl_semaphore.give();
    return false;
  }

  INFO("Firing 'IsHalted' into the FSM");
  fireEvent("IsHalted");
  m_wl_semaphore.give();
  return false;
}

bool gem::base::GEMFSMApplication::configure( toolbox::task::WorkLoop *wl)
{
  INFO("configure called, current state: " << m_gemfsm.getCurrentState());
  while ((m_gemfsm.getCurrentState()) != m_gemfsm.getStateName(STATE_CONFIGURING)) { // deal with possible race condition
    INFO("not in " << STATE_CONFIGURING << " sleeping (" << m_gemfsm.getCurrentState() << ")");
    usleep(100);
  }
  INFO("configure called, current state: " << m_gemfsm.getCurrentState());

  try {
    m_progress = 0.0;
    configureAction();
    p_gemWebInterface->buildCfgWebpage(); // complete, so re render the config web page
    m_progress = 1.0;
  } catch (gem::utils::exception::Exception& ex) {
    fireEvent("Fail");
    return false;
  }
  
  fireEvent("IsConfigured");
  return false;
}

bool gem::base::GEMFSMApplication::start(toolbox::task::WorkLoop *wl)
{
  INFO("start called, current state: " << m_gemfsm.getCurrentState());
  while ((m_gemfsm.getCurrentState()) != m_gemfsm.getStateName(STATE_STARTING)) { // deal with possible race condition
    usleep(100);
  }
  INFO("start called, current state: " << m_gemfsm.getCurrentState());

  try {
    m_progress = 0.0;
    startAction();
    p_gemWebInterface->buildCfgWebpage(); // complete, so re render the config web page
    m_progress = 1.0;
  } catch (gem::utils::exception::Exception& ex) {
    fireEvent("Fail");
    return false;
  }

  fireEvent("IsRunning");
  return false;
}

bool gem::base::GEMFSMApplication::pause(toolbox::task::WorkLoop *wl)
{
  INFO("pause called, current state: " << m_gemfsm.getCurrentState());
  while ((m_gemfsm.getCurrentState()) != m_gemfsm.getStateName(STATE_PAUSING)) { // deal with possible race condition
    usleep(100);
  }
  INFO("pause called, current state: " << m_gemfsm.getCurrentState());

  try {
    m_progress = 0.0;
    pauseAction();
    p_gemWebInterface->buildCfgWebpage(); // complete, so re render the config web page
    m_progress = 1.0;
  } catch (gem::utils::exception::Exception& ex) {
    fireEvent("Fail");
    return false;
  }

  fireEvent("IsPaused");
  return false;
}

bool gem::base::GEMFSMApplication::resume(toolbox::task::WorkLoop *wl)
{
  INFO("resume called, current state: " << m_gemfsm.getCurrentState());
  while ((m_gemfsm.getCurrentState()) != m_gemfsm.getStateName(STATE_RESUMING)) { // deal with possible race condition
    usleep(100);
  }
  INFO("resume called, current state: " << m_gemfsm.getCurrentState());

  try {
    m_progress = 0.0;
    resumeAction();
    p_gemWebInterface->buildCfgWebpage(); // complete, so re render the config web page
    m_progress = 1.0;
  } catch (gem::utils::exception::Exception& ex) {
    fireEvent("Fail");
    return false;
  }

  fireEvent("IsRunning");
  return false;
}

bool gem::base::GEMFSMApplication::stop(toolbox::task::WorkLoop *wl)
{
  INFO("stop called, current state: " << m_gemfsm.getCurrentState());
  while ((m_gemfsm.getCurrentState()) != m_gemfsm.getStateName(STATE_STOPPING)) { // deal with possible race condition
    usleep(100);
  }
  INFO("stop called, current state: " << m_gemfsm.getCurrentState());

  try {
    m_progress = 0.0;
    stopAction();
    p_gemWebInterface->buildCfgWebpage(); // complete, so re render the config web page
    m_progress = 1.0;
  } catch (gem::utils::exception::Exception& ex) {
    fireEvent("Fail");
    return false;
  }

  fireEvent("IsConfigured");
  return false;
}

bool gem::base::GEMFSMApplication::halt(toolbox::task::WorkLoop *wl)
{
  INFO("halt called, current state: " << m_gemfsm.getCurrentState());
  while ((m_gemfsm.getCurrentState()) != m_gemfsm.getStateName(STATE_HALTING)) { // deal with possible race condition
    usleep(100);
  }
  INFO("halt called, current state: " << m_gemfsm.getCurrentState());

  try {
    m_progress = 0.0;
    haltAction();
    p_gemWebInterface->buildCfgWebpage(); // complete, so re render the config web page
    m_progress = 1.0;
  } catch (gem::utils::exception::Exception& ex) {
    fireEvent("Fail");
    return false;
  }

  fireEvent("IsHalted");
  return false;
}

bool gem::base::GEMFSMApplication::reset(toolbox::task::WorkLoop *wl)
{
  INFO("reset called, current state: " << m_gemfsm.getCurrentState());
  while ((m_gemfsm.getCurrentState()) != m_gemfsm.getStateName(STATE_RESETTING)) { // deal with possible race condition
    usleep(100);
  }
  INFO("reset called, current state: " << m_gemfsm.getCurrentState());

  try {
    m_progress = 0.0;
    resetAction();
    p_gemWebInterface->buildCfgWebpage(); // complete, so re render the config web page
    m_progress = 1.0;
  } catch (gem::utils::exception::Exception& ex) {
    fireEvent("Fail");
    return false;
  }
  
  fireEvent("IsInitial");
  //maybe do a m_gemfsm.reset()?
  return false;
}

/*
  bool gem::base::GEMFSMApplication::noAction(toolbox::task::WorkLoop *wl)
  {
  return false
  }

  bool gem::base::GEMFSMApplication::fail(toolbox::task::WorkLoop *wl)
  {
  return false;
  }
*/

/** transition details*/
/*
  void gem::base::GEMFSMApplication::initializeAction()
  {
  DEBUG(std::string("gem::base::GEMFSMApplication::initializeAction Initializing"));
  }

  void gem::base::GEMFSMApplication::configureAction() {}
  void gem::base::GEMFSMApplication::startAction()     {}
  void gem::base::GEMFSMApplication::pauseAction()     {}
  void gem::base::GEMFSMApplication::resumeAction()    {}
  void gem::base::GEMFSMApplication::stopAction()      {}
  void gem::base::GEMFSMApplication::haltAction()      {}
  //void gem::base::GEMFSMApplication::resetAction()     {}
  //void gem::base::GEMFSMApplication::noAction()        {} 
  //void gem::base::GEMFSMApplication::failAction()      {} 	
  */
