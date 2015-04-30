/**
 * class: GEMFSMApplication
 * description: Generic GEM application with FSM interface
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: 
 * date: 
 */

#include "gem/base/GEMFSMApplication.h"
#include "gem/base/GEMWebApplication.h"

#include "gem/utils/exception/Exception.h"
#include "gem/base/exception/Exception.h"
#include "gem/base/utils/exception/Exception.h"

#include "toolbox/fsm/FailedEvent.h"
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
  gemfsm_(this),//, &gemAppStateInfoSpace_)
  wl_semaphore_(toolbox::BSem::FULL)
{
  DEBUG("GEMFSMApplication ctor begin");

  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiInitialize, "Initialize" );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiEnable,     "Enable"     );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiConfigure,  "Configure"  );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiStart,      "Start"      );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiStop,       "Stop"       );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiPause,      "Pause"      );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiResume,     "Resume"     );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiHalt,       "Halt"       );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiReset,      "Reset"      );

  // These bindings expose the state machine to the outside world. The
  // changeState() method simply forwards the calls to the GEMFSM
  // object.
  xoap::bind(this, &GEMFSMApplication::changeState, "Initialize", XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Enable",     XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Configure",  XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Start",      XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Stop",       XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Pause",      XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Resume",     XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Halt",       XDAQ_NS_URI);

  //appStateInfoSpace_.setFSMState(gemfsm_.getCurrentStateName());

  DEBUG("GEMFSMApplication ctor end");
}

gem::base::GEMFSMApplication::~GEMFSMApplication()
{
  DEBUG("GEMFSMApplication dtor begin");
  DEBUG("GEMFSMApplication dtor end");
}

/**hyperdaq callbacks*/
void gem::base::GEMFSMApplication::xgiInitialize(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webInitialize(in,out);
}

void gem::base::GEMFSMApplication::xgiEnable(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webEnable(in,out);
}

void gem::base::GEMFSMApplication::xgiConfigure(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webConfigure(in,out);
}

void gem::base::GEMFSMApplication::xgiStart(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webStart(in,out);
}

void gem::base::GEMFSMApplication::xgiStop(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webStop(in,out);
}

void gem::base::GEMFSMApplication::xgiPause(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webPause(in,out);
}

void gem::base::GEMFSMApplication::xgiResume(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webResume(in,out);
}

void gem::base::GEMFSMApplication::xgiHalt(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webHalt(in,out);
}

void gem::base::GEMFSMApplication::xgiReset(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webReset(in,out);
}

/**soap callbacks*/
/**
void gem::base::GEMFSMApplication::initializeAction(toolbox::Event::Reference event)
{
  INFO("GEMFSMApplication::initializeAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();

  try {
    //gemhwP_->connectDevice();
    std::string msgBase = "Could not configure the hardware";
  }
  catch (gem::hw::exception::Exception& err) {
    std::string msgBase = "Could not connect to the hardware";
    std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
    FATAL(msg);
    XCEPT_DECLARE_NESTED(gem::hw::exception::HardwareProblem, top, msg, err);
    notifyQualified("fatal", top);
    gemfsm_.gotoFailed(top);
  }
  
  //appStateInfoSpace_.setBool("gem_hw_connected", true, true);
}

void gem::base::GEMFSMApplication::configureAction(toolbox::Event::Reference event)
{
  INFO("ConfigureAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();
  try {
    //gemhwP_->configureDevice();
    std::string msgBase = "Could not configure the hardware";
  }
  catch (gem::hw::exception::Exception& err) {
    std::string msgBase = "Could not configure the hardware";
    std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
    FATAL(msg);
    XCEPT_DECLARE_NESTED(gem::hw::exception::HardwareProblem, top, msg, err);
    notifyQualified("fatal", top);
    gemfsm_.gotoFailed(top);
  }
  
  // BUG BUG BUG
  // Slow things down a bit while developing. Easier to follow the flow...
  ::sleep(2);
  // BUG BUG BUG end
  
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  //INFO("GEMFSMApplication::ConfigureAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void gem::base::GEMFSMApplication::enableAction(toolbox::Event::Reference event)
{
  INFO("EnableAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();

  try {
    //gemhwP_->enableDevice();
    std::string msgBase = "Could not configure the hardware";
  }
  catch (gem::hw::exception::Exception& err) {
    std::string msgBase = "Could not enable the hardware";
    std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
    FATAL(msg);
    XCEPT_DECLARE_NESTED(gem::hw::exception::HardwareProblem, top, msg, err);
    notifyQualified("fatal", top);
    gemfsm_.gotoFailed(top);
  }
  
  // BUG BUG BUG
  // Slow things down a bit while developing. Easier to follow the flow...
  ::sleep(2);
  // BUG BUG BUG end
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  //INFO("EnableAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void
gem::base::GEMFSMApplication::failAction(toolbox::Event::Reference event)
{
  INFO("FailAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();
//  if (gemhwP_ != 0)
//    {
//      // BUG BUG BUG
//      // This is a mess (and may need a lock, actually).
//      //gemhwP_->releaseDevice();
//      // BUG BUG BUG end
//    }
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  //INFO("FailAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void
gem::base::GEMFSMApplication::haltAction(toolbox::Event::Reference event)
{
  INFO("HaltAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();

  try {
    //gemhwP->haltDevice();
    std::string msgBase = "Could not configure the hardware";
  }
  catch (gem::hw::exception::Exception& err) {
    std::string msgBase = "Could not halt the hardware";
    std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
    FATAL(msg);
    XCEPT_DECLARE_NESTED(gem::hw::exception::HardwareProblem, top, msg, err);
    notifyQualified("fatal", top);
    gemfsm_.gotoFailed(top);
  }
  
  try {
    //gemhwP_->releaseDevice();
    std::string msgBase = "Could not configure the hardware";
  }
  catch (gem::hw::exception::Exception& err) {
    std::string msgBase = "Could not release the hardware";
    std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
    FATAL(msg);
    XCEPT_DECLARE_NESTED(gem::hw::exception::HardwareProblem, top, msg, err);
    notifyQualified("fatal", top);
    gemfsm_.gotoFailed(top);
  }

  //appStateInfoSpace_.setBool("gem_hw_connected", false, true);

  // BUG BUG BUG
  // Slow things down a bit while developing. Easier to follow the flow...
  ::sleep(2);
  // BUG BUG BUG end
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  //INFO("HaltAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void
gem::base::GEMFSMApplication::pauseAction(toolbox::Event::Reference event)
{
  INFO("PauseAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();

  try {
    //gemhwP_->pauseDevice();
    std::string msgBase = "Could not configure the hardware";
  }
  catch (gem::hw::exception::Exception& err) {
    std::string msgBase = "Could not pause the hardware";
    std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
    FATAL(msg);
    XCEPT_DECLARE_NESTED(gem::hw::exception::HardwareProblem, top, msg, err);
    notifyQualified("fatal", top);
    gemfsm_.gotoFailed(top);
  }

  // BUG BUG BUG
  // Slow things down a bit while developing. Easier to follow the flow...
  ::sleep(2);
  // BUG BUG BUG end
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  //INFO("PauseAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void
gem::base::GEMFSMApplication::resumeAction(toolbox::Event::Reference event)
{
  INFO("ResumeAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();

  try {
    //gemhwP_->resumeDevice();
    std::string msgBase = "Could not configure the hardware";
  }
  catch (gem::hw::exception::Exception& err) {
    std::string msgBase = "Could not resume the hardware";
    std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
    FATAL(msg);
    XCEPT_DECLARE_NESTED(gem::hw::exception::HardwareProblem, top, msg, err);
    notifyQualified("fatal", top);
    gemfsm_.gotoFailed(top);
  }
  
  // BUG BUG BUG
  // Slow things down a bit while developing. Easier to follow the flow...
  ::sleep(2);
  // BUG BUG BUG end
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  //INFO("ResumeAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void
gem::base::GEMFSMApplication::stopAction(toolbox::Event::Reference event)
{
  INFO("StopAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();
  this->haltAction(event);
  this->configureAction(event);
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  //INFO("StopAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}
*/ 
 
// This simply forwards the message to the FSM object, since it is
// technically not possible to bind directly to anything but an
// xdaq::Application.
xoap::MessageReference
gem::base::GEMFSMApplication::changeState(xoap::MessageReference msg)
{
  return gemfsm_.changeState(msg);
}
