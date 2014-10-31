#include "gem/base/GEMFSMApplication.h"

#include "toolbox/fsm/FailedEvent.h"
#include "toolbox/string.h"
#include "xcept/Exception.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/NamespaceURI.h"
#include "xgi/Input.h"
#include "xgi/Method.h"
#include "xgi/Output.h"
#include "xoap/Method.h"

//#include "tcds/utils/exception/Exception.h"
//#include "tcds/utils/ConfigurationInfoSpaceHandler.h"
//#include "tcds/utils/HwDeviceTCA.h"
//#include "tcds/utils/LogMacros.h"
//#include "tcds/utils/Monitor.h"
//#include "tcds/utils/WebServer.h"
//#include "tcds/utils/Utils.h"

gem::base::GEMFSMApplication::GEMFSMApplication(xdaq::ApplicationStub* stub) :
  GEMApplication(stub),
  gemfsm_(this, &appStateInfoSpace_)
{
  DEBUG("GEMFSMApplication ctor begin");

  // These bindings expose the state machine to the outside world. The
  // changeState() method simply forwards the calls to the GlibFSM
  // object.
  xoap::bind(this, &GEMFSMApplication::changeState, "Initialize", XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Configure",  XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Start",      XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Halt",       XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Pause",      XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Resume",     XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Stop",       XDAQ_NS_URI);

  appStateInfoSpace_.setFSMState(gemfsm_getCurrentStateName());

  DEBUG("GEMFSMApplication ctor end");
}

gem::base::GEMFSMApplication::~GEMFSMApplication()
{
  DEBUG("GEMFSMApplication dtor begin");
  DEBUG("GEMFSMApplication dtor end");
}

void gem::base::GEMFSMApplication::InitializeAction(toolbox::Event::Reference event)
{
  INFO("GEMFSMApplication::InitializeAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();

  try
    {
      gemhwP_->hwConnect();
    }
  catch (gem::base::utils::exception::Exception& err)
    {
      std::string msgBase = "Could not connect to the hardware";
      std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
      FATAL(msg);
      XCEPT_DECLARE_NESTED(gem::base::utils::exception::HardwareProblem, top, msg, err);
      notifyQualified("fatal", top);
      gemfsm_gotoFailed(top);
    }

  appStateInfoSpace_.setBool("hw_connected", true, true);
}

void
gem::base::GEMFSMApplication::ConfigureAction(toolbox::Event::Reference event)
  try
    {
      hwP_->hwConfigure();
    }
  catch (gem::base::utils::exception::Exception& err)
    {
      std::string msgBase = "Could not configure the hardware";
      std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
      FATAL(msg);
      XCEPT_DECLARE_NESTED(gem::base::utils::exception::HardwareProblem, top, msg, err);
      notifyQualified("fatal", top);
      gemfsm_gotoFailed(top);
    }

  // BUG BUG BUG
  // Slow things down a bit while developing. Easier to follow the flow...
  ::sleep(2);
  // BUG BUG BUG end

  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  INFO("GEMFSMApplication::ConfigureAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void
gem::base::GEMFSMApplication::EnableAction(toolbox::Event::Reference event)
{
  INFO("EnableAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();

  try
    {
      hwP_->hwEnable();
    }
  catch (gem::base::utils::exception::Exception& err)
    {
      std::string msgBase = "Could not enable the hardware";
      std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
      FATAL(msg);
      XCEPT_DECLARE_NESTED(gem::base::utils::exception::HardwareProblem, top, msg, err);
      notifyQualified("fatal", top);
      gemfsm_gotoFailed(top);
    }

  // BUG BUG BUG
  // Slow things down a bit while developing. Easier to follow the flow...
  ::sleep(2);
  // BUG BUG BUG end
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  INFO("EnableAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void
gem::base::GEMFSMApplication::FailAction(toolbox::Event::Reference event)
{
  INFO("FailAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();
  if (hwP_ != 0)
    {
      // BUG BUG BUG
      // This is a mess (and may need a lock, actually).
      hwP_->hwRelease();
      // BUG BUG BUG end
    }
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  INFO("FailAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void
gem::base::GEMFSMApplication::HaltAction(toolbox::Event::Reference event)
{
  INFO("HaltAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();

  try
    {
      hwP_->hwHalt();
    }
  catch (gem::base::utils::exception::Exception& err)
    {
      std::string msgBase = "Could not halt the hardware";
      std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
      FATAL(msg);
      XCEPT_DECLARE_NESTED(gem::base::utils::exception::HardwareProblem, top, msg, err);
      notifyQualified("fatal", top);
      gemfsm_gotoFailed(top);
    }

  try
    {
      hwP_->hwRelease();
    }
  catch (gem::base::utils::exception::Exception& err)
    {
      std::string msgBase = "Could not release the hardware";
      std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
      FATAL(msg);
      XCEPT_DECLARE_NESTED(gem::base::utils::exception::HardwareProblem, top, msg, err);
      notifyQualified("fatal", top);
      gemfsm_gotoFailed(top);
    }

  appStateInfoSpace_.setBool("hw_connected", false, true);

  // BUG BUG BUG
  // Slow things down a bit while developing. Easier to follow the flow...
  ::sleep(2);
  // BUG BUG BUG end
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  INFO("HaltAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void
gem::base::GEMFSMApplication::PauseAction(toolbox::Event::Reference event)
{
  INFO("PauseAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();

  try
    {
      hwP_->hwPause();
    }
  catch (gem::base::utils::exception::Exception& err)
    {
      std::string msgBase = "Could not pause the hardware";
      std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
      FATAL(msg);
      XCEPT_DECLARE_NESTED(gem::base::utils::exception::HardwareProblem, top, msg, err);
      notifyQualified("fatal", top);
      gemfsm_gotoFailed(top);
    }

  // BUG BUG BUG
  // Slow things down a bit while developing. Easier to follow the flow...
  ::sleep(2);
  // BUG BUG BUG end
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  INFO("PauseAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void
gem::base::GEMFSMApplication::ResumeAction(toolbox::Event::Reference event)
{
  INFO("ResumeAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();

  try
    {
      hwP_->hwResume();
    }
  catch (gem::base::utils::exception::Exception& err)
    {
      std::string msgBase = "Could not resume the hardware";
      std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
      FATAL(msg);
      XCEPT_DECLARE_NESTED(gem::base::utils::exception::HardwareProblem, top, msg, err);
      notifyQualified("fatal", top);
      gemfsm_gotoFailed(top);
    }

  // BUG BUG BUG
  // Slow things down a bit while developing. Easier to follow the flow...
  ::sleep(2);
  // BUG BUG BUG end
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  INFO("ResumeAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void
gem::base::GEMFSMApplication::StopAction(toolbox::Event::Reference event)
{
  INFO("StopAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();
  this->HaltAction(event);
  this->ConfigureAction(event);
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  INFO("StopAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

// This simply forwards the message to the FSM object, since it is
// technically not possible to bind directly to anything but an
// xdaq::Application.
xoap::MessageReference
gem::base::GEMFSMApplication::changeState(xoap::MessageReference msg)
{
  return gemfsm_changeState(msg);
}
