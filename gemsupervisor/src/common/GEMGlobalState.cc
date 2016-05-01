#include "gem/supervisor/GEMGlobalState.h"

#include "toolbox/task/TimerFactory.h"
#include "toolbox/task/Timer.h"

// #include "xoap/MessageFactory.h"
// #include "xoap/MessageFactory.h"

#include "gem/supervisor/GEMSupervisor.h"
#include "gem/base/GEMState.h"
#include "gem/utils/LockGuard.h"

gem::supervisor::GEMApplicationState::GEMApplicationState()
{
  state          = gem::base::STATE_NULL;
  progress       = 1.0;
  progressWeight = 1.0;
}

gem::supervisor::GEMGlobalState::GEMGlobalState(xdaq::ApplicationContext* context, GEMSupervisor* gemSupervisor) :
  // m_globalState(gem::base::STATE_UNINIT),
  // p_gemSupervisor(std::make_shared<GEMSupervisor>(gemSupervisor)),
  p_gemSupervisor(gemSupervisor),
  p_appContext(context),
  p_srcApp(gemSupervisor->getApplicationDescriptor()),
  m_globalState(gem::base::STATE_INITIAL),
  m_gemLogger(gemSupervisor->getApplicationLogger()),
  m_mutex(toolbox::BSem::FULL, true)
{
  // default constructor
}


gem::supervisor::GEMGlobalState::~GEMGlobalState()
{
  // delete p_timer;
}


void gem::supervisor::GEMGlobalState::addApplication(xdaq::ApplicationDescriptor* app)
{
  DEBUG("GEMGlobalState::addApplication");
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_mutex);

  m_states.insert(std::pair<xdaq::ApplicationDescriptor*, GEMApplicationState>(app, GEMApplicationState()));

  ApplicationMap::iterator i = m_states.find(app);
  std::string appURN = "urn:xdaq-application:"+app->getClassName();
  i->second.updateMsg = gem::utils::soap::GEMSOAPToolBox::createStateRequestMessage("app", appURN, true);
}

void gem::supervisor::GEMGlobalState::clear()
{
  DEBUG("GEMGlobalState::clear");
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_mutex);
}

void gem::supervisor::GEMGlobalState::update()
{
  DEBUG("GEMGlobalState::update");
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_mutex);
  for (auto i = m_states.begin(); i != m_states.end(); ++i) {
    updateApplication(i->first);
  }
  toolbox::fsm::State before = m_globalState;
  calculateGlobals();

  DEBUG("GEMGlobalState::update before=" << before << " after=" << m_globalState);
  if (before != m_globalState)
    p_gemSupervisor->globalStateChanged(before, m_globalState);
}

void gem::supervisor::GEMGlobalState::startTimer() {
  if (!p_timer) {
    // p_timer = std::make_shared<toolbox::task::Timer>(toolbox::task::TimerFactory::getInstance()->createTimer("GEMGlobalStateTimer"));
    p_timer = toolbox::task::TimerFactory::getInstance()->createTimer("GEMGlobalStateTimer");
    toolbox::TimeVal      start = toolbox::TimeVal::gettimeofday();
    toolbox::TimeInterval delta(5, 0);
    p_timer->activate();
    p_timer->scheduleAtFixedRate(start, this, delta, 0, "GEMGlobalStateUpdate");
  }
}

void gem::supervisor::GEMGlobalState::timeExpired(toolbox::task::TimerEvent& event)
{
  DEBUG("GEMGlobalState::timeExpired received event:" << event.type());
  update();
}


void gem::supervisor::GEMGlobalState::calculateGlobals()
{
  DEBUG("GEMGlobalState::calculateGlobalState");
  m_globalState = gem::base::STATE_NULL;
  m_globalStateMessage = "";

  for (auto appState = m_states.begin(); appState != m_states.end(); ++appState) {
    DEBUG("GEMGlobalState::calculateGlobalState:" << appState->first->getClassName().c_str() << ":"
          << appState->first->getInstance() << " has state message "
          << appState->second.stateMessage.c_str() << " and state:"
          << appState->second.state);
    if (appState->second.state == gem::base::STATE_FAILED) {
      m_globalStateMessage += toolbox::toString(" (%s:%d) : %s ", appState->first->getClassName().c_str(),
                                                appState->first->getInstance(),
                                                appState->second.stateMessage.c_str());
    }

    int pg = getStatePriority(m_globalState);
    int pa = getStatePriority(appState->second.state);
    if (pa < pg)
      m_globalState = appState->second.state;
  }
}


void gem::supervisor::GEMGlobalState::updateApplication(xdaq::ApplicationDescriptor* app)
{
  ApplicationMap::iterator i   = m_states.find(app);
  xoap::MessageReference   msg = xoap::createMessage(i->second.updateMsg), answer;
  std::string nstag = "gemapp";
  std::stringstream debugstream;
  msg->writeTo(debugstream);
  try {
    answer = p_appContext->postSOAP(msg, *p_srcApp, *app);
  } catch (xoap::exception::Exception& e) {
    ERROR("GEMGlobalState::updateApplication caught exception communicating with " << app->getClassName() << ":" << app->getInstance()
          << ". Applcation probably crashed, setting state to FAILED"
          << " (xoap::exception::Exception)" << e.what());
    INFO("GEMGlobalState::updateApplication tried sending SOAP [" << debugstream << "]");
    i->second.state        = gem::base::STATE_FAILED;
    i->second.stateMessage = "Communication failure, assuming state is FAILED, may mean application/executive crash.";
    return;
  } catch (xdaq::exception::Exception& e) {
    ERROR("GEMGlobalState::updateApplication caught exception communicating with " << app->getClassName() << ":" << app->getInstance()
          << ". Applcation probably crashed, setting state to FAILED"
          << " (xdaq::exception::Exception)" << e.what());
    INFO("GEMGlobalState::updateApplication tried sending SOAP [" << debugstream << "]");
    i->second.state        = gem::base::STATE_FAILED;
    i->second.stateMessage = "Communication failure, assuming state is FAILED, may mean application/executive crash.";
    return;
  } catch (xcept::Exception& e) {
    ERROR("GEMGlobalState::updateApplication caught exception communicating with " << app->getClassName() << ":" << app->getInstance()
          << ". Applcation probably crashed, setting state to FAILED"
          << " (xcept::Exception)" << e.what());
    INFO("GEMGlobalState::updateApplication tried sending SOAP [" << debugstream << "]");
    i->second.state        = gem::base::STATE_FAILED;
    i->second.stateMessage = "Communication failure, assuming state is FAILED, may mean application/executive crash.";
    return;
  } catch (std::exception& e) {
    ERROR("GEMGlobalState::updateApplication caught exception communicating with " << app->getClassName() << ":" << app->getInstance()
          << ". Applcation probably crashed, setting state to FAILED"
          << " (std::exception)" << e.what());
    INFO("GEMGlobalState::updateApplication tried sending SOAP [" << debugstream << "]");
    i->second.state        = gem::base::STATE_FAILED;
    i->second.stateMessage = "Communication failure, assuming state is FAILED, may mean application/executive crash.";
    return;
  } catch (...) {
    ERROR("GEMGlobalState::updateApplication caught exception communicating with " << app->getClassName() << ":" << app->getInstance()
          << ". Applcation probably crashed, setting state to FAILED");
    INFO("GEMGlobalState::updateApplication tried sending SOAP [" << debugstream << "]");
    i->second.state        = gem::base::STATE_FAILED;
    i->second.stateMessage = "Communication failure, assuming state is FAILED, may mean application/executive crash.";
    return;
  }

  // parse answer here
  std::string    appUrn = "urn:xdaq-application:" + app->getClassName();
  xoap::SOAPName stateReply("StateName", nstag, appUrn);

  xoap::SOAPElement props = answer->getSOAPPart().getEnvelope().getBody().getChildElements()[0].getChildElements()[0];
  std::vector<xoap::SOAPElement> basic = props.getChildElements(stateReply);
  if (basic.size() == 1) {
    std::string stateString = basic[0].getValue();
    if (!strcasecmp(stateString.c_str(), "Uninitialized"))
      i->second.state = gem::base::STATE_UNINIT;
    else if (!strcasecmp(stateString.c_str(), "Halted"))
      i->second.state = gem::base::STATE_HALTED;
    else if (!strcasecmp(stateString.c_str(), "Cold-Init"))
      i->second.state = gem::base::STATE_COLD;
    else if (!strcasecmp(stateString.c_str(), "Initial"))
      i->second.state = gem::base::STATE_INITIAL;
    else if (!strcasecmp(stateString.c_str(), "Configured"))
      i->second.state = gem::base::STATE_CONFIGURED;
    else if (!strcasecmp(stateString.c_str(), "Active"))
      i->second.state = gem::base::STATE_RUNNING;
    else if (!strcasecmp(stateString.c_str(), "Enabled"))
      i->second.state = gem::base::STATE_RUNNING;
    else if (!strcasecmp(stateString.c_str(), "RUNNING"))
      i->second.state = gem::base::STATE_RUNNING;
    else if (!strcasecmp(stateString.c_str(), "Paused"))
      i->second.state = gem::base::STATE_PAUSED;
    else if (!strcasecmp(stateString.c_str(), "Suspended"))
      i->second.state = gem::base::STATE_PAUSED;

    else if (!strcasecmp(stateString.c_str(), "Initializing"))
      i->second.state = gem::base::STATE_INITIALIZING;
    else if (!strcasecmp(stateString.c_str(), "Configuring"))
      i->second.state = gem::base::STATE_CONFIGURING;
    else if (!strcasecmp(stateString.c_str(), "Halting"))
      i->second.state = gem::base::STATE_HALTING;
    else if (!strcasecmp(stateString.c_str(), "Pausing"))
      i->second.state = gem::base::STATE_PAUSING;
    else if (!strcasecmp(stateString.c_str(), "Stopping"))
      i->second.state = gem::base::STATE_STOPPING;
    else if (!strcasecmp(stateString.c_str(), "Starting"))
      i->second.state = gem::base::STATE_STARTING;
    else if (!strcasecmp(stateString.c_str(), "Resuming"))
      i->second.state = gem::base::STATE_RESUMING;
    else if (!strcasecmp(stateString.c_str(), "Resetting"))
      i->second.state = gem::base::STATE_RESETTING;
    else if (!strcasecmp(stateString.c_str(), "Fixing"))
      i->second.state = gem::base::STATE_FIXING;
    else if (!strcasecmp(stateString.c_str(), "failed"))
      i->second.state = gem::base::STATE_FAILED;
    else
      WARN("GEMGlobalState::updateApplication " << app->getClassName() << ":" << static_cast<int>(app->getInstance())
           << " " << stateString);
  } else {
    std::string toolInput;
    xoap::dumpTree(msg->getSOAPPart().getEnvelope().getDOMNode(), toolInput);
    std::string  tool;
    xoap::dumpTree(answer->getSOAPPart().getEnvelope().getDOMNode(), tool);

    if (answer->getSOAPPart().getEnvelope().getBody().hasFault()) {
      ERROR("SOAP fault getting state: " << std::endl << "SOAP request:" << std::endl << toolInput);
      ERROR("SOAP fault getting state: " << std::endl << "SOAP reply:" << std::endl
            << answer->getSOAPPart().getEnvelope().getBody().getFault().getFaultString()
            << std::endl << tool);
    }
    DEBUG("GEMGlobalState::updateApplication " << app->getClassName() << ":" << static_cast<int>(app->getInstance())
          << std::endl << static_cast<int>(basic.size())
          << std::endl << tool);
  }
}

// static functions
std::string gem::supervisor::GEMGlobalState::getStateName(toolbox::fsm::State state) {
  switch (state) {
  case (gem::base::STATE_UNINIT):
    return "Uninitialized";
    break;
  case (gem::base::STATE_COLD):
    return "Cold-Init";
    break;
  case (gem::base::STATE_INITIAL):
    return "Initial";
    break;
  case (gem::base::STATE_HALTED):
    return "Halted";
    break;
  case (gem::base::STATE_CONFIGURED):
    return "Configured";
    break;
  case (gem::base::STATE_RUNNING):
    return "Running";
    break;
  case (gem::base::STATE_PAUSED):
    return "Pause";
    break;
  case (gem::base::STATE_FAILED):
    return "Failed";
    break;
  case (gem::base::STATE_INITIALIZING):
    return "Initializing";
    break;
  case (gem::base::STATE_CONFIGURING):
    return "Configuring";
    break;
  case (gem::base::STATE_HALTING):
    return "Halting";
    break;
  case (gem::base::STATE_PAUSING):
    return "Pausing";
    break;
  case (gem::base::STATE_STOPPING):
    return "Stopping";
    break;
  case (gem::base::STATE_STARTING):
    return "Starting";
    break;
  case (gem::base::STATE_RESUMING):
    return "Resuming";
    break;
  case (gem::base::STATE_RESETTING):
    return "Resetting";
    break;
  case (gem::base::STATE_FIXING):
    return "Fixing";
    break;
  default:
    return toolbox::toString("Unknown : %c (%d)", state, static_cast<int>(state));
  }
}

int gem::supervisor::GEMGlobalState::getStatePriority(toolbox::fsm::State state)
{
  static const toolbox::fsm::State statePriority[] = {
    gem::base::STATE_RESETTING,
    gem::base::STATE_FAILED,
    gem::base::STATE_COLD,
    gem::base::STATE_UNINIT,
    gem::base::STATE_INITIALIZING,
    gem::base::STATE_HALTING,
    gem::base::STATE_CONFIGURING,
    gem::base::STATE_STARTING,
    gem::base::STATE_STOPPING,
    gem::base::STATE_PAUSING,
    gem::base::STATE_RESUMING,
    gem::base::STATE_INITIAL,
    gem::base::STATE_HALTED,
    gem::base::STATE_PAUSED,
    gem::base::STATE_FIXING,
    gem::base::STATE_CONFIGURED,
    gem::base::STATE_RUNNING,
    gem::base::STATE_NULL
  };
  int i;
  for (i = 0; statePriority[i] != state && statePriority[i] != gem::base::STATE_NULL; ++i)
    {}
  return i;
};
