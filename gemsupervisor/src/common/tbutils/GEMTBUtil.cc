#include "gem/supervisor/tbutils/GEMTBUtil.h"
#include "gem/hw/vfat/HwVFAT2.h"

#include "gem/hw/glib/HwGLIB.h"
#include "gem/hw/optohybrid/HwOptoHybrid.h"
#include "gem/utils/GEMLogging.h"
#include "gem/utils/soap/GEMSOAPToolBox.h"

#include <algorithm>
#include <ctime>

#include <iomanip>
#include <iostream>
#include <ctime>
#include <sstream>
#include <cstdlib>

#include "cgicc/HTTPRedirectHeader.h"
#include "xdata/Vector.h"
#include <string>

#include "gem/supervisor/tbutils/VFAT2XMLParser.h"

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPConstants.h"
#include "xoap/SOAPBody.h"
#include "xoap/Method.h"
#include "xoap/AttachmentPart.h"
#include "xoap/domutils.h"

//XDAQ_INSTANTIATOR_IMPL(gem::supervisor::tbutils::GEMTBUtil)

void gem::supervisor::tbutils::GEMTBUtil::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{
  nTriggers = 1000U;

  //stablish the number of VFATs and the entry is
  for (int i = 0; i < 24; ++i) {
    deviceName.push_back("");
    deviceNum.push_back(-1);
  }

  //  triggerSource = 0x9;
  deviceChipID  = 0x0;

  triggersSeen = 0;
  triggercount = 0;
  ADCVoltage = 0;
  ADCurrent = 0;
  ohGTXLink    = 3;

  bag->addField("nTriggers",    &nTriggers);

  bag->addField("settingsFile", &settingsFile);

  bag->addField("deviceName",   &deviceName);
  bag->addField("deviceIP",     &deviceIP  );
  bag->addField("ohGTXLink",    &ohGTXLink );

  bag->addField("deviceNum",    &deviceNum   );
  bag->addField("deviceChipID", &deviceChipID);
  bag->addField("triggersSeen", &triggersSeen);
  bag->addField("triggercount", &triggercount);

  bag->addField("ADCVoltage",   &ADCVoltage);
  bag->addField("ADCurrent",    &ADCurrent);

  bag->addField("UseLocalTriggers",    &useLocalTriggers);
  bag->addField("LocalTriggerMode",    &localTriggerMode);
  bag->addField("LocalTriggerPeriod",  &localTriggerPeriod);
//  bag->addField("triggerSource",&triggerSource);
  bag->addField("slotFileName",  &slotFileName);

}

gem::supervisor::tbutils::GEMTBUtil::GEMTBUtil(xdaq::ApplicationStub *s)
  throw (xdaq::exception::Exception) :
  xdaq::WebApplication(s),
  m_gemLogger(this->getApplicationLogger()),
  fsmP_(0),
  wl_semaphore_(toolbox::BSem::FULL),
  hw_semaphore_(toolbox::BSem::FULL),

  initSig_ (0),
  confSig_ (0),
  startSig_(0),
  stopSig_ (0),
  haltSig_ (0),
  resetSig_(0),
  //  runSig_  (0),
  readout_mask(0x0),
  is_working_     (false),
  is_initialized_ (false),
  is_configured_  (false),
  is_running_     (false)
{
  // Detect when the setting of default parameters has been performed
  this->getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");

  getApplicationInfoSpace()->fireItemAvailable("confParams", &confParams_);
  getApplicationInfoSpace()->fireItemAvailable("ipAddr",     &ipAddr_);

  getApplicationInfoSpace()->fireItemValueRetrieve("confParams", &confParams_);
  getApplicationInfoSpace()->fireItemValueRetrieve("ipAddr",     &ipAddr_);

  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webDefault,      "Default"    );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webInitialize,   "Initialize" );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webConfigure,    "Configure"  );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webStart,        "Start"      );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webStop,         "Stop"       );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webHalt,         "Halt"       );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webReset,        "Reset"      );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webResetCounters,"ResetCounters");
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webSendFastCommands,"FastCommands");

  xoap::bind(this, &gem::supervisor::tbutils::GEMTBUtil::onInitialize,  "Initialize",  XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::GEMTBUtil::onConfigure,   "Configure",   XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::GEMTBUtil::onStart,       "Start",       XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::GEMTBUtil::onStop,        "Stop",        XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::GEMTBUtil::onHalt,        "Halt",        XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::GEMTBUtil::onReset,       "Reset",       XDAQ_NS_URI);

  initSig_  = toolbox::task::bind(this, &GEMTBUtil::initialize, "initialize");
  confSig_  = toolbox::task::bind(this, &GEMTBUtil::configure,  "configure" );
  startSig_ = toolbox::task::bind(this, &GEMTBUtil::start,      "start"     );
  stopSig_  = toolbox::task::bind(this, &GEMTBUtil::stop,       "stop"      );
  haltSig_  = toolbox::task::bind(this, &GEMTBUtil::halt,       "halt"      );
  resetSig_ = toolbox::task::bind(this, &GEMTBUtil::reset,      "reset"     );

  std::string className = getApplicationDescriptor()->getClassName();
  DEBUG("className " << className);
  className =
    className.substr(className.rfind("gem::supervisor::"),std::string::npos);
  DEBUG("className " << className);

  fsmP_ = new
    toolbox::fsm::AsynchronousFiniteStateMachine("GEMTButilFSM:" + className);


  fsmP_->addState('I', "Initial",     this, &gem::supervisor::tbutils::GEMTBUtil::stateChanged);
  fsmP_->addState('H', "Halted",      this, &gem::supervisor::tbutils::GEMTBUtil::stateChanged);
  fsmP_->addState('C', "Configured",  this, &gem::supervisor::tbutils::GEMTBUtil::stateChanged);
  fsmP_->addState('E', "Running",     this, &gem::supervisor::tbutils::GEMTBUtil::stateChanged);

  fsmP_->setStateName('F', "Error");
  fsmP_->setFailedStateTransitionAction(this,  &gem::supervisor::tbutils::GEMTBUtil::transitionFailed);
  fsmP_->setFailedStateTransitionChanged(this, &gem::supervisor::tbutils::GEMTBUtil::stateChanged);

  fsmP_->addStateTransition('I', 'H', "Initialize", this, &gem::supervisor::tbutils::GEMTBUtil::initializeAction);
  fsmP_->addStateTransition('H', 'C', "Configure",  this, &gem::supervisor::tbutils::GEMTBUtil::configureAction);
  fsmP_->addStateTransition('C', 'C', "Configure",  this, &gem::supervisor::tbutils::GEMTBUtil::configureAction);
  fsmP_->addStateTransition('C', 'E', "Start",      this, &gem::supervisor::tbutils::GEMTBUtil::startAction);
  fsmP_->addStateTransition('E', 'C', "Stop",       this, &gem::supervisor::tbutils::GEMTBUtil::stopAction);
  fsmP_->addStateTransition('C', 'H', "Halt",       this, &gem::supervisor::tbutils::GEMTBUtil::haltAction);
  fsmP_->addStateTransition('E', 'H', "Halt",       this, &gem::supervisor::tbutils::GEMTBUtil::haltAction);
  //fsmP_->addStateTransition('H', 'H', "Halt",       this, &gem::supervisor::tbutils::GEMTBUtil::haltAction);
  fsmP_->addStateTransition('C', 'I', "Reset",      this, &gem::supervisor::tbutils::GEMTBUtil::resetAction);
  fsmP_->addStateTransition('H', 'I', "Reset",      this, &gem::supervisor::tbutils::GEMTBUtil::resetAction);

  // Define invalid transitions, too, so that they can be ignored, or else FSM will be unhappy when one is fired.
  fsmP_->addStateTransition('E', 'E', "Configure", this, &gem::supervisor::tbutils::GEMTBUtil::noAction);
  fsmP_->addStateTransition('H', 'H', "Start"    , this, &gem::supervisor::tbutils::GEMTBUtil::noAction);
  fsmP_->addStateTransition('E', 'E', "Start"    , this, &gem::supervisor::tbutils::GEMTBUtil::noAction);
  fsmP_->addStateTransition('H', 'H', "Stop"     , this, &gem::supervisor::tbutils::GEMTBUtil::noAction);
  fsmP_->addStateTransition('C', 'C', "Stop"     , this, &gem::supervisor::tbutils::GEMTBUtil::noAction);
  fsmP_->addStateTransition('H', 'H', "Halt"     , this, &gem::supervisor::tbutils::GEMTBUtil::noAction);

  fsmP_->setInitialState('I');
  fsmP_->reset();

  /*
    wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:GEMTBUtil","waiting");
    wl_->activate();
  */

}

gem::supervisor::tbutils::GEMTBUtil::~GEMTBUtil()
{
  if (fsmP_)
    delete fsmP_;
  fsmP_ = 0;
}


void gem::supervisor::tbutils::GEMTBUtil::actionPerformed(xdata::Event& event)
{
  // This is called after all default configuration values have been
  // loaded (from the XDAQ configuration file).
  if (event.type() == "urn:xdaq-event:setDefaultValues") {
    std::stringstream ss;
    ss << "ipAddr_=[" << ipAddr_.toString() << "]" << std::endl;

    DEBUG(ss.str());
    confParams_.bag.deviceIP = ipAddr_;
  }
}

void gem::supervisor::tbutils::GEMTBUtil::fireEvent(const std::string& name)
{
  toolbox::Event::Reference event((new toolbox::Event(name, this)));
  fsmP_->fireEvent(event);
}

void gem::supervisor::tbutils::GEMTBUtil::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
{
  //keep_refresh_ = false;

  DEBUG("Current state is: [" << fsm.getStateName (fsm.getCurrentState()) << "]");
  std::string state_=fsm.getStateName (fsm.getCurrentState());

  DEBUG( "StateChanged: " << (std::string)state_);

}

void gem::supervisor::tbutils::GEMTBUtil::transitionFailed(toolbox::Event::Reference event)
{
  //keep_refresh_ = false;
  toolbox::fsm::FailedEvent &failed = dynamic_cast<toolbox::fsm::FailedEvent&>(*event);

  std::stringstream reason;
  reason << "<![CDATA["
         << std::endl
         << "Failure occurred when performing transition"
         << " from "        << failed.getFromState()
         << " to "          << failed.getToState()
         << ". Exception: " << xcept::stdformat_exception_history( failed.getException() )
         << std::endl
         << "]]>";

  ERROR(reason.str());
}

//Actions (defined in the base class, not in the derived class)
bool gem::supervisor::tbutils::GEMTBUtil::initialize(toolbox::task::WorkLoop* wl)
{
  fireEvent("Initialize");
  return false; //do once?
}

bool gem::supervisor::tbutils::GEMTBUtil::configure(toolbox::task::WorkLoop* wl)
{
  fireEvent("Configure");
  return false; //do once?
}

bool gem::supervisor::tbutils::GEMTBUtil::start(toolbox::task::WorkLoop* wl)
{
  fireEvent("Start");
  return false;
}

bool gem::supervisor::tbutils::GEMTBUtil::stop(toolbox::task::WorkLoop* wl)
{
  fireEvent("Stop");
  return false; //do once?
}

bool gem::supervisor::tbutils::GEMTBUtil::halt(toolbox::task::WorkLoop* wl)
{
  fireEvent("Halt");
  return false; //do once?
}

bool gem::supervisor::tbutils::GEMTBUtil::reset(toolbox::task::WorkLoop* wl)
{
  fireEvent("Reset");
  return false; //do once?
}


// SOAP interface (defined in the base class, not in the derived class)
xoap::MessageReference gem::supervisor::tbutils::GEMTBUtil::onInitialize(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  is_working_ = true;

  wl_->submit(initSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::GEMTBUtil::onConfigure(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  is_working_ = true;

  wl_->submit(confSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::GEMTBUtil::onStart(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  is_working_ = true;

  wl_->submit(startSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::GEMTBUtil::onStop(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  is_working_ = true;

  wl_->submit(stopSig_);
  return message;
}


xoap::MessageReference gem::supervisor::tbutils::GEMTBUtil::onHalt(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  is_working_ = true;

  wl_->submit(haltSig_);

  return message;
}

xoap::MessageReference gem::supervisor::tbutils::GEMTBUtil::onReset(xoap::MessageReference message)
  throw (xoap::exception::Exception)
{
  is_working_ = true;

  wl_->submit(resetSig_);

  return message;
}

void gem::supervisor::tbutils::GEMTBUtil::showCounterLayout(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    if (is_initialized_) {

      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/ResetCounters")
           << std::endl;

      hw_semaphore_.take();

      *out << "<table class=\"xdaq-table\">" << std::endl
	//<< cgicc::caption("Counters")     << std::endl
	   << cgicc::thead() << std::endl
	   << cgicc::tr()    << std::endl //open
	   << cgicc::th()    << "L1A"      << cgicc::th() << std::endl
	   << cgicc::th()    << "CalPulse" << cgicc::th() << std::endl
	   << cgicc::th()    << "Other"    << cgicc::th() << std::endl
	   << cgicc::tr()    << std::endl //close
	   << cgicc::thead() << std::endl

	   << cgicc::tbody() << std::endl;

      *out << "<tr>" << std::endl
	   << "<td>" << std::endl
	   << "<table class=\"xdaq-table\">" << std::endl
	   << cgicc::thead() << std::endl
	   << "<tr>" << std::endl
	   << cgicc::th()    << "Source" << cgicc::th() << std::endl
	   << cgicc::th()    << "Value"  << cgicc::th() << std::endl
	   << cgicc::th()    << "Reset"  << cgicc::th() << std::endl
	   << "</tr>" << std::endl //close
	   << cgicc::thead() << std::endl //close

	   << "<tbody>" << std::endl
	   << "<tr>" << std::endl
	   << cgicc::td()    << "TTC_on_GLIB"    << cgicc::td() << std::endl
	   << cgicc::td()    << optohybridDevice_->getL1ACount(0x0) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstL1ATTC")
	.set("name","RstL1ATTC")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "T1_in_Firmware"    << cgicc::td() << std::endl
	   << cgicc::td()    <<  optohybridDevice_->getL1ACount(0x1) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstL1AT1")
	.set("name","RstL1AT1")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "External"     << cgicc::td() << std::endl
	   << cgicc::td()    << optohybridDevice_->getL1ACount(0x2) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstL1AExt")
	.set("name","RstL1AExt")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Loopback_sBits"       << cgicc::td() << std::endl
	   << cgicc::td()    << optohybridDevice_->getL1ACount(0x3) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstL1Asbits")
	.set("name","RstL1Asbits")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Sent_along_GEB"     << cgicc::td() << std::endl
	   << cgicc::td()    << optohybridDevice_->getL1ACount(0x4) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstL1AGEB")
	.set("name","RstL1AGEB")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "</tbody>" << std::endl
	   << "</table>"     << std::endl
	   << "</td>" << std::endl;

      *out << "<td>" << std::endl
	   << "<table class=\"xdaq-table\">" << std::endl
	   << cgicc::thead() << std::endl
	   << "<tr>" << std::endl
	   << cgicc::th()    << "Source" << cgicc::th() << std::endl
	   << cgicc::th()    << "Value"  << cgicc::th() << std::endl
	   << cgicc::th()    << "Reset"  << cgicc::th() << std::endl
	   << "</tr>" << std::endl
	   << cgicc::thead() << std::endl

	   << "<tbody>" << std::endl
	   << "<tr>" << std::endl
	//	   << "<tr>" << std::endl
	   << cgicc::td()    << "TTC_on_GLIB"  << cgicc::td() << std::endl
	   << cgicc::td()    << optohybridDevice_->getCalPulseCount(0x0)  << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstCalPulseTTC")
	.set("name","RstCalPulseTTC")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl

	   << cgicc::td()    << "T1_in_Firmware"  << cgicc::td() << std::endl
	   << cgicc::td()    << optohybridDevice_->getCalPulseCount(0x1)  << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstCalPulseT1")
	.set("name","RstCalPulseT1")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl

	   << cgicc::td()    << "External"  << cgicc::td() << std::endl
	   << cgicc::td()    << optohybridDevice_->getCalPulseCount(0x2)  << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstCalPulseExt")
	.set("name","RstCalPulseExt")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Loopback_sBits"     << cgicc::td() << std::endl
	   << cgicc::td()    << optohybridDevice_->getCalPulseCount(0x3) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstCalPulseSbit")
	.set("name","RstCalPulseSbit")
	   << cgicc::td() << std::endl
	   << "</tr>"     << std::endl

	   << "<tr>" << std::endl

	   << cgicc::td()    << "Sent_along_GEB"  << cgicc::td() << std::endl
	   << cgicc::td()    << optohybridDevice_->getCalPulseCount(0x4)  << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstCalPulseGEB")
	.set("name","RstCalPulseGEB")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "</tbody>"  << std::endl
	   << "</table>"  << std::endl
	   << "</td>"     << std::endl;

      *out << "<td>" << std::endl
	   << "<table class=\"xdaq-table\">" << std::endl
	   << cgicc::thead() << std::endl
	   << "<tr>" << std::endl
	   << cgicc::th()    << "Source" << cgicc::th() << std::endl
	   << cgicc::th()    << "Value"  << cgicc::th() << std::endl
	   << cgicc::th()    << "Reset"  << cgicc::th() << std::endl
	   << "</tr>" << std::endl
	   << cgicc::thead() << std::endl

	   << "<tbody>" << std::endl
	   << "<tr>" << std::endl
	   << cgicc::td()    << "Resync"    << cgicc::td() << std::endl
	   << cgicc::td()    << optohybridDevice_->getResyncCount() << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstResync")
	.set("name","RstResync")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "BC0"       << cgicc::td() << std::endl
	   << cgicc::td()    << optohybridDevice_->getBC0Count() << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstBC0")
	.set("name","RstBC0")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()  << "BXCount"   << cgicc::td() << std::endl
	   << cgicc::td()  << optohybridDevice_->getBXCountCount() << cgicc::td() << std::endl
	   << cgicc::td()  << "" << cgicc::td() << std::endl
	   << "</tr>"      << std::endl
	   << "</tbody>"   << std::endl
	   << "</table>"   << std::endl
	   << "</td>"      << std::endl
	   << "</tr>"      << std::endl
	   << cgicc::tbody() << std::endl
	   << "</table>"   << std::endl;

      hw_semaphore_.give();

      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Reset counters.")
	.set("value", "ResetCounters") << std::endl;

      *out << cgicc::form() << std::endl;

    }
  } catch (const xgi::exception::Exception& e) {
    ERROR("Something went wrong displaying showCounterLayout(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception& e) {
    ERROR("Something went wrong displaying showCounterLayout(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
} //end showCounterLayout


void gem::supervisor::tbutils::GEMTBUtil::showBufferLayout(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    if (is_initialized_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/FastCommands")
           << std::endl;

      *out << cgicc::br() << std::endl;
      *out << cgicc::input().set("class","button").set("type","submit")
	.set("value","FlushFIFO").set("name","SendFastCommand")
	   << std::endl;

      *out << cgicc::input().set("class","button").set("type","submit")
	.set("value","SendTestPackets").set("name","SendFastCommand")
	   << std::endl;

      *out << cgicc::form() << std::endl
	   << cgicc::br()   << std::endl;
    }
  } catch (const xgi::exception::Exception& e) {
    ERROR("Something went wrong displaying showBufferLayout(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception& e) {
    ERROR("Something went wrong displaying showBufferLayout(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
} //end showBufferLayout


void gem::supervisor::tbutils::GEMTBUtil::fastCommandLayout(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    if (is_initialized_) {

      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/FastCommands")
           << std::endl;

      *out << cgicc::table().set("class","xdaq-table") << std::endl
	   << cgicc::thead() << std::endl
	   << cgicc::tr()    << std::endl //open
	   << cgicc::th()    << "L1A"          << cgicc::th() << std::endl
	   << cgicc::th()    << "CalPulse"     << cgicc::th() << std::endl
	   << cgicc::th()    << "Resync"       << cgicc::th() << std::endl
	   << cgicc::th()    << "BC0"          << cgicc::th() << std::endl
	   << cgicc::th()    << "L1A+CalPulse" << cgicc::th() << std::endl
	   << cgicc::tr()    << std::endl //close
	   << cgicc::thead() << std::endl

	   << cgicc::tbody() << std::endl;

      *out << cgicc::tr()  << std::endl;
      *out << cgicc::td()  << cgicc::input().set("class","button").set("type","submit")
	.set("value","Send L1A").set("name","SendFastCommand")
	   << cgicc::td()  << std::endl;
      *out << cgicc::td()  << cgicc::input().set("class","button").set("type","submit")
	.set("value","Send CalPulse").set("name","SendFastCommand")
	   << cgicc::td()  << std::endl;
      *out << cgicc::td()  << cgicc::input().set("class","button").set("type","submit")
	.set("value","Send Resync").set("name","SendFastCommand")
	   << cgicc::td()  << std::endl;
      *out << cgicc::td()  << cgicc::input().set("class","button").set("type","submit")
	.set("value","Send BC0").set("name","SendFastCommand")
	   << cgicc::td()  << std::endl;
      *out << cgicc::td()  << cgicc::input().set("class","button").set("type","submit")
	.set("value","Send L1A+CalPulse").set("name","SendFastCommand")
	   << cgicc::br()  << std::endl
	   << cgicc::input().set("id","CalPulseDelay").set("name","CalPulseDelay")
	.set("type","number").set("min","0").set("max","255")
	.set("value","4")
	   << cgicc::td()  << std::endl;

      *out << cgicc::tr()    << std::endl
	   << cgicc::tbody() << std::endl
	   << cgicc::table() << std::endl;

      //trigger setup
      /*
      *out << cgicc::table().set("class","xdaq-table") << std::endl
	   << cgicc::thead() << std::endl
	   << cgicc::tr()    << std::endl //open
	   << cgicc::th()    << "Trigger Source Select" << cgicc::th() << std::endl
	   << cgicc::th()    << "SBit to TDC Select"    << cgicc::th() << std::endl
	   << cgicc::tr()    << std::endl //close
	   << cgicc::thead() << std::endl

	   << cgicc::tbody() << std::endl;

      *out << cgicc::tr() << std::endl;
      *out << cgicc::td() << std::endl
	   << cgicc::input().set("type","radio").set("name","trgSrc")
	.set("id","GLIBsrc").set("value","TTC_GLIB_trsSrc")
	.set((unsigned)confParams_.bag.triggerSource == (unsigned)0x0 ? "checked" : "")

	   << cgicc::label("TTC_GLIB_trsSrc").set("for","GLIBSrc") << std::endl
	   << cgicc::br()
	   << cgicc::input().set("type","radio").set("name","trgSrc")
	.set("id","T1_Src").set("value","T1_trgSrc")
	.set((unsigned)confParams_.bag.triggerSource == (unsigned)0x1 ? "checked" : "")
	   << cgicc::label("T1_trgSrc").set("for","T1_Src") << std::endl
	   << cgicc::br()
	   << cgicc::input().set("type","radio").set("name","trgSrc")
	.set("id","Ext").set("value","Ext_trgSrc")
	.set((unsigned)confParams_.bag.triggerSource == (unsigned)0x2 ? "checked" : "")
	   << cgicc::label("Ext_trgSrc").set("for","Ext") << std::endl
	   << cgicc::br()
	   << cgicc::input().set("type","radio").set("name","trgSrc").set("checked")
	.set("id","sBitSrc").set("value","sbits_trgSrc")
	.set((unsigned)confParams_.bag.triggerSource == (unsigned)0x3 ? "checked" : "")
	   << cgicc::label("sbits_trgSrc").set("for","sBitSrc") << std::endl
	   << cgicc::br()
	   << cgicc::input().set("type","radio").set("name","trgSrc")
	.set("id","Total").set("value","Total_trgSrc")
	.set((unsigned)confParams_.bag.triggerSource == (unsigned)0x4 ? "checked" : "")
	   << cgicc::label("Total_trgSrc").set("for","Total") << std::endl
	   << cgicc::br()
	   << cgicc::input().set("class","button").set("type","submit")
	.set("value","SetTriggerSource").set("name","SendFastCommand")
	   << cgicc::td() << std::endl;
      */
      std::string isReadonly = "";
      if (is_running_ || is_configured_)
	isReadonly = "readonly";

      *out << cgicc::td() << std::endl
	   << cgicc::label("SBitSelect").set("for","SBitSelect") << std::endl
	   << cgicc::input().set("class","vfatBiasInput").set("id","SBitSelect" ).set("name","SBitSelect")
	.set("type","number").set("min","0").set("max","5")
	.set("value",confParams_.bag.deviceNum.toString())
	.set(isReadonly)
	   << cgicc::input().set("class","button").set("type","submit")
	.set("value","SBitSelect").set("name","SendFastCommand")
	   << cgicc::td() << std::endl;

      *out << cgicc::tr()    << std::endl
	   << cgicc::tbody() << std::endl
	   << cgicc::table() << std::endl
	   << cgicc::form()  << std::endl;
    }
  } catch (const xgi::exception::Exception& e) {
    ERROR("Something went wrong displaying fastCommandLayout(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception& e) {
    ERROR("Something went wrong displaying fastCommandLayout(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

void gem::supervisor::tbutils::GEMTBUtil::redirect(xgi::Input *in, xgi::Output* out) {
  //change the status to halting and make sure the page displays this information
  std::string redURL = "/" + getApplicationDescriptor()->getURN() + "/Default";
  *out << "<meta http-equiv=\"refresh\" content=\"0;" << redURL << "\">" << std::endl;
  this->webDefault(in,out);
}

// HyperDAQ interface
void gem::supervisor::tbutils::GEMTBUtil::webDefault(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    ////update the page refresh
    if (!is_working_ && !is_running_) {
    }
    else if (is_working_) {
      cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
      head.addHeader("Refresh","2");
    }
    else if (is_running_) {
      cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
      head.addHeader("Refresh","30");
    }

    //generate the control buttons and display the ones that can be touched depending on the run mode
    *out << "<div class=\"xdaq-tab-wrapper\">"            << std::endl;
    *out << "<div class=\"xdaq-tab\" title=\"Control\">"  << std::endl;


    *out << "<table class=\"xdaq-table\">" << std::endl
	 << cgicc::thead() << std::endl
	 << cgicc::tr()    << std::endl //open
	 << cgicc::th()    << "Control" << cgicc::th() << std::endl
	 << cgicc::th()    << "Buffer"  << cgicc::th() << std::endl
	 << cgicc::tr()    << std::endl //close
	 << cgicc::thead() << std::endl

	 << "<tbody>" << std::endl
	 << "<tr>"    << std::endl
	 << "<td>"    << std::endl;

    if (!is_initialized_) {
      //have a menu for selecting the VFAT
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Initialize") << std::endl;

      selectMultipleVFAT(out);
      scanParameters(out);

      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Initialize hardware acces.")
	.set("value", "Initialize") << std::endl;

      *out << cgicc::form() << std::endl;
    }

    else if (!is_configured_) {
      //this will allow the parameters to be set to the chip and scan routine

      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Configure") << std::endl;

      selectMultipleVFAT(out);
      scanParameters(out);

      *out << cgicc::input().set("type","text").set("name","xmlFilename").set("size","80")
        .set("ENCTYPE","multipart/form-data").set("readonly")
        .set("value",confParams_.bag.settingsFile.toString()) << std::endl;

      *out << cgicc::br() << std::endl;
      *out << cgicc::input().set("type", "submit")
        .set("name", "command").set("title", "Configure threshold scan.")
        .set("value", "Configure") << std::endl;
      *out << cgicc::form()        << std::endl;
    }

    else if (!is_running_) {
      //hardware is initialized and configured, we can start the run
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Start") << std::endl;

      selectMultipleVFAT(out);
      scanParameters(out);

      *out << cgicc::input().set("type", "submit")
        .set("name", "command").set("title", "Start threshold scan.")
        .set("value", "Start") << std::endl;
      *out << cgicc::form()    << std::endl;
    }

    else if (is_running_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Stop") << std::endl;

      selectMultipleVFAT(out);
      scanParameters(out);

      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Stop threshold scan.")
	.set("value", "Stop") << std::endl;
      *out << cgicc::form()   << std::endl;
    }

    *out << cgicc::comment() << "end the main commands, now putting the halt/reset commands" << cgicc::comment() << cgicc::br() << std::endl;
    *out << cgicc::span()  << std::endl
	 << "<table>" << std::endl
	 << "<tr>"    << std::endl
	 << "<td>"    << std::endl;

    //always should have a halt command
    *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Halt") << std::endl;

    *out << cgicc::input().set("type", "submit")
      .set("name", "command").set("title", "Halt threshold scan.")
      .set("value", "Halt") << std::endl;
    *out << cgicc::form() << std::endl
	 << "</td>" << std::endl;

    *out << "<td>"  << std::endl;

    if (!is_running_) {
      //comand that will take the system to initial and allow to change the hw device
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Reset") << std::endl;
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Reset device.")
	.set("value", "Reset") << std::endl;
      *out << cgicc::form() << std::endl;
    }
    *out << "</td>"    << std::endl
	 << "</tr>"    << std::endl
	 << "</table>" << std::endl
	 << cgicc::br() << std::endl
	 << cgicc::span()  << std::endl;

    *out << "</td>" << std::endl;

    *out << "<td>" << std::endl;
    if (is_initialized_)
      showBufferLayout(out);
    *out << "</td>"    << std::endl
	 << "</tr>"    << std::endl
	 << "</tbody>" << std::endl
	 << "</table>" << cgicc::br() << std::endl;

    *out << "</div>" << std::endl; //close control

    *out << "<div class=\"xdaq-tab\" title=\"Counters\">"  << std::endl;//open countera
    if (is_initialized_)
      showCounterLayout(out);
    *out << "</div>" << std::endl;//close counters

    *out << "<div class=\"xdaq-tab\" title=\"Fast Commands/Trigger Setup\">"  << std::endl;//open fast commands
    if (is_initialized_)
      fastCommandLayout(out);
    *out << "</div>" << std::endl; //close fast commands

    *out << "</div>" << std::endl;    //</div> //close the new div xdaq-tab

    *out << cgicc::br() << cgicc::br() << std::endl;

    //*out << "<div class=\"xdaq-tab\" title=\"Status\">"  << std::endl
    //*out << cgicc::div().set("class","xdaq-tab").set("title","Status")   << std::endl
    *out << "<table class=\"xdaq-table\">" << std::endl
	 << cgicc::thead() << std::endl
	 << cgicc::tr()    << std::endl //open
	 << cgicc::th()    << "Program" << cgicc::th() << std::endl
	 << cgicc::th()    << "System"  << cgicc::th() << std::endl
	 << cgicc::tr()    << std::endl //close
	 << cgicc::thead() << std::endl

	 << "<tbody>" << std::endl
	 << "<tr>"    << std::endl
	 << "<td>"    << std::endl;

    *out << "<table class=\"xdaq-table\">" << std::endl
	 << cgicc::thead() << std::endl
	 << cgicc::tr()    << std::endl //open
	 << cgicc::th()    << "Status" << cgicc::th() << std::endl
	 << cgicc::th()    << "Value"  << cgicc::th() << std::endl
	 << cgicc::tr()    << std::endl //close
	 << cgicc::thead() << std::endl

	 << "<tbody>" << std::endl

	 << "<tr>" << std::endl
	 << "<td>" << "is_working_" << "</td>"
	 << "<td>" << is_working_   << "</td>"
	 << "</tr>"   << std::endl

	 << "<tr>" << std::endl
	 << "<td>" << "is_initialized_" << "</td>"
	 << "<td>" << is_initialized_   << "</td>"
	 << "</tr>"       << std::endl

	 << "<tr>" << std::endl
	 << "<td>" << "is_configured_" << "</td>"
	 << "<td>" << is_configured_   << "</td>"
	 << "</tr>"      << std::endl

	 << "<tr>" << std::endl
	 << "<td>" << "is_running_" << "</td>"
	 << "<td>" << is_running_   << "</td>"
	 << "</tr>"   << std::endl

	 << "</tbody>" << std::endl
	 << "</table>" << cgicc::br() << std::endl
	 << "</td>"    << std::endl;

    *out  << "<td>"     << std::endl
	  << "<table class=\"xdaq-table\">" << std::endl
	  << cgicc::thead() << std::endl
	  << cgicc::tr()    << std::endl //open
	  << cgicc::th()    << "Device"     << cgicc::th() << std::endl
	  << cgicc::th()    << "Connected"  << cgicc::th() << std::endl
	  << cgicc::tr()    << std::endl //close
	  << cgicc::thead() << std::endl
	  << "<tbody>" << std::endl;

    *out << "</tbody>" << std::endl
	 << "</table>" << std::endl
	 << "</td>"    << std::endl
	 << "</tr>"    << std::endl
	 << "</tbody>" << std::endl
	 << "</table>" << std::endl;

    *out << cgicc::script().set("type","text/javascript")
      .set("src","http://ajax.googleapis.com/ajax/libs/jquery/1/jquery.min.js")
	 << cgicc::script() << std::endl;
    *out << cgicc::script().set("type","text/javascript")
      .set("src","http://ajax.googleapis.com/ajax/libs/jqueryui/1/jquery-ui.min.js")
	 << cgicc::script() << std::endl;
  } catch (const xgi::exception::Exception& e) {
    ERROR("Something went wrong displaying GEMTBUtil control panel(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception& e) {
    ERROR("Something went wrong displaying GEMTBUtil control panel(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}


void gem::supervisor::tbutils::GEMTBUtil::webInitialize(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    cgicc::Cgicc cgi(in);
    std::vector<cgicc::FormEntry> vfat2FormEntries = cgi.getElements();
    TRACE( "debugging form entries");
    std::vector<cgicc::FormEntry>::const_iterator myiter = vfat2FormEntries.begin();

    //OH Devices
    cgicc::form_iterator oh = cgi.getElement("SetOH");
    if (strcmp((**oh).c_str(),"OH_0") == 0) {
      confParams_.bag.ohGTXLink.value_= 0;
      DEBUG("OH_0 has been selected " << confParams_.bag.ohGTXLink);
    } else if (strcmp((**oh).c_str(),"OH_1") == 0) {
      confParams_.bag.ohGTXLink.value_= 1;
      DEBUG("OH_1 has been selected " << confParams_.bag.ohGTXLink);
    }

    m_vfatMask = 0x0;
    for(int i = 0; i < 24; ++i) {
      std::stringstream currentChipID;
      currentChipID << "VFAT" << i;

      std::stringstream form;
      form << "VFATDevice" << i;

      std::string tmpDeviceName = confParams_.bag.deviceName[i].toString();
      cgicc::const_form_iterator name = cgi.getElement(form.str());
      if (name != cgi.getElements().end()) {
        DEBUG( "found form element::" << form.str());
        DEBUG( "has value::" << name->getValue());
	tmpDeviceName = name->getValue();
	confParams_.bag.deviceName[i] = tmpDeviceName;
	DEBUG( "Web_deviceName::"             << confParams_.bag.deviceName[i].toString());
        m_vfatMask |= (0x1<<i);
      }

      int tmpDeviceNum = -1;
      tmpDeviceName.erase(0,4);
      tmpDeviceNum = atoi(tmpDeviceName.c_str());

      readout_mask = confParams_.bag.ohGTXLink;

    }//end for

    m_vfatMask = ~m_vfatMask;
    //change the status to initializing and make sure the page displays this information
  } catch (const xgi::exception::Exception & e) {
    ERROR("Something went wrong: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception & e) {
    ERROR("Something went wrong: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }

  wl_->submit(initSig_);

  redirect(in,out);
}


void gem::supervisor::tbutils::GEMTBUtil::webConfigure(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{

  wl_->submit(confSig_);

  redirect(in,out);
}


void gem::supervisor::tbutils::GEMTBUtil::webStart(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{

  wl_->submit(startSig_);

  redirect(in,out);
}

//no need to redefine in the derived class
void gem::supervisor::tbutils::GEMTBUtil::webStop(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  wl_->submit(stopSig_);

  redirect(in,out);
}


//no need to redefine in the derived class
void gem::supervisor::tbutils::GEMTBUtil::webHalt(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  wl_->submit(haltSig_);

  redirect(in,out);
}


//no need to redefine in the derived class
void gem::supervisor::tbutils::GEMTBUtil::webReset(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  wl_->submit(resetSig_);

  redirect(in,out);
}


//no need to redefine in the derived class
void gem::supervisor::tbutils::GEMTBUtil::webResetCounters(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    cgicc::Cgicc cgi(in);
    std::vector<cgicc::FormEntry> resetCounters = cgi.getElements();
    DEBUG( "resetting counters entries");

    hw_semaphore_.take();

    DEBUG("GEMTBUtil::webResetCounters Reseting counters");

    if (cgi.queryCheckbox("RstL1ATTC") )
      optohybridDevice_->resetL1ACount(0x0);

    if (cgi.queryCheckbox("RstL1AT1") )
      optohybridDevice_->resetL1ACount(0x1);

    if (cgi.queryCheckbox("RstL1AExt") )
      optohybridDevice_->resetL1ACount(0x2);

    if (cgi.queryCheckbox("RstL1Asbits") )
      optohybridDevice_->resetL1ACount(0x3);

    if (cgi.queryCheckbox("RstL1AGEB") )
      optohybridDevice_->resetL1ACount(0x4);

    if (cgi.queryCheckbox("RstCalPulseTTC") )
      optohybridDevice_->resetCalPulseCount(0x0);

    if (cgi.queryCheckbox("RstCalPulseT1") )
      optohybridDevice_->resetCalPulseCount(0x1);

    if (cgi.queryCheckbox("RstCalPulseExt") )
      optohybridDevice_->resetCalPulseCount(0x2);

    if (cgi.queryCheckbox("RstCalPulseSbit") )
      optohybridDevice_->resetCalPulseCount(0x3);

    if (cgi.queryCheckbox("RstCalPulseGEB") )
      optohybridDevice_->resetCalPulseCount(0x4);

    if (cgi.queryCheckbox("RstResync") )
      optohybridDevice_->resetResyncCount();

    if (cgi.queryCheckbox("RstBC0") )
      optohybridDevice_->resetBC0Count();

    hw_semaphore_.give();

  } catch (const xgi::exception::Exception & e) {
    ERROR("Something went wrong: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception & e) {
    ERROR("Something went wrong: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }

  redirect(in,out);
}


//no need to redefine in the derived class
void gem::supervisor::tbutils::GEMTBUtil::webSendFastCommands(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    cgicc::Cgicc cgi(in);
    std::vector<cgicc::FormEntry> resetCounters = cgi.getElements();
    DEBUG( "resetting counters entries");

    std::string fastCommand = cgi["SendFastCommand"]->getValue();

    if (strcmp(fastCommand.c_str(),"FlushFIFO") == 0) {
      DEBUG("FlushFIFO button pressed");
      hw_semaphore_.take();
      for (int i = 0; i < 2; ++i){
	glibDevice_->flushFIFO(i);
      }
      hw_semaphore_.give();
    }

    if (strcmp(fastCommand.c_str(),"SendTestPackets") == 0) {
      DEBUG("SendTestPackets button pressed");
      hw_semaphore_.take();
      if (!is_running_) {
	for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip){
	  (*chip)->setRunMode(0x1);
	  (*chip)->sendTestPattern(0x1);
	  (*chip)->sendTestPattern(0x0);
	}
      }
      if (!is_running_) {
	for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
          (*chip)->setRunMode(0x0);
	}
      }
      hw_semaphore_.give();

    }

    else if (strcmp(fastCommand.c_str(),"Send L1A+CalPulse") == 0) {
      DEBUG("Send L1A+CalPulse button pressed");
      cgicc::const_form_iterator element = cgi.getElement("CalPulseDelay");
      uint8_t delay = 4;
      if (element != cgi.getElements().end())
	delay = element->getIntegerValue();
      hw_semaphore_.take();
      optohybridDevice_->sendResync();
      for (unsigned int com = 0; com < 15; ++com)
	optohybridDevice_->sendL1ACal(10, delay,1);
	hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send L1A") == 0) {
      DEBUG("Send L1A button pressed");
      hw_semaphore_.take();
      optohybridDevice_->sendL1A(10,1);
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send CalPulse") == 0) {
      DEBUG("Send CalPulse button pressed");
      hw_semaphore_.take();
      optohybridDevice_->sendCalPulse(0x1,1);
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send Resync") == 0) {
      DEBUG("Send Resync button pressed");
      hw_semaphore_.take();
      optohybridDevice_->sendResync();
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send BC0") == 0) {
      DEBUG("Send BC0 button pressed");
      hw_semaphore_.take();
      optohybridDevice_->sendBC0();
      hw_semaphore_.give();
    }
    /*
    else if (strcmp(fastCommand.c_str(),"SetTriggerSource") == 0) {
      DEBUG("SetTriggerSource button pressed");
      hw_semaphore_.take();
      cgicc::form_iterator fi = cgi.getElement("trgSrc");
      if( !fi->isEmpty() && fi != (*cgi).end()) {
	if (strcmp((**fi).c_str(),"TTC_GLIB_trsSrc") == 0) {
	  confParams_.bag.triggerSource = 0x0;
	  optohybridDevice_->setTrigSource(0x0);
	}
	else if (strcmp((**fi).c_str(),"T1_trgSrc") == 0) {
	  confParams_.bag.triggerSource = 0x1;
	  optohybridDevice_->setTrigSource(0x1);
	}
	else if (strcmp((**fi).c_str(),"Ext_trgSrc") == 0) {
	  confParams_.bag.triggerSource = 0x2;
	  optohybridDevice_->setTrigSource(0x2);
	}
	else if (strcmp((**fi).c_str(),"sbits_trgSrc") == 0) {
	  confParams_.bag.triggerSource = 0x3;
	  optohybridDevice_->setTrigSource(0x3);
	}
      }
      hw_semaphore_.give();
    }*/

  } catch (const xgi::exception::Exception & e) {
    ERROR("Something went wrong: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception & e) {
    ERROR("Something went wrong: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }

  redirect(in,out);
}


// State transitions
//is initialize different than halt? they come from different positions but put the software/hardware in the same state 'halted'
void gem::supervisor::tbutils::GEMTBUtil::initializeAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  is_working_ = true;
  setLogLevelTo(uhal::Debug());  // Set uHAL logging level Debug (most) to Error (least)

  hw_semaphore_.take();

  gem::utils::soap::GEMSOAPToolBox::sendCommand("Initialize",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Manager", 0));  // this should not be hard coded
  gem::utils::soap::GEMSOAPToolBox::sendCommand("Initialize",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::glib::GLIBManager", 0));  // this should not be hard coded
  gem::utils::soap::GEMSOAPToolBox::sendCommand("Initialize",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Readout", 0));  // this should not be hard coded


  std::stringstream tmpURI;
  tmpURI << "chtcp-2.0://localhost:10203?target=" << confParams_.bag.deviceIP.toString() << ":50001";

  glibDevice_ = glib_shared_ptr(new gem::hw::glib::HwGLIB("HwGLIB", tmpURI.str(),
                                                          "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));

  /*    optohybridDevice_ = optohybrid_shared_ptr(new gem::hw::optohybrid::HwOptoHybrid("HwOptoHybrid0", tmpURI.str(),
	"file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));*/

  std::string ohDeviceName = toolbox::toString("HwOptoHybrid%d",confParams_.bag.ohGTXLink.value_);
  optohybridDevice_ = optohybrid_shared_ptr(new gem::hw::optohybrid::HwOptoHybrid(ohDeviceName, tmpURI.str(),
                                                                                  "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));

  if (glibDevice_->isHwConnected()) {
    DEBUG("GLIB device connected");
    glibDevice_->writeReg("GLIB.TTC.CONTROL.INHIBIT_L1A",0x1);
    disableTriggers();
    if (optohybridDevice_->isHwConnected()) {
      DEBUG("OptoHybrid device connected");

      optohybridDevice_->setVFATMask(m_vfatMask);

      for(int i=0;i<24;++i){
	//  int i=0;
	std::stringstream currentChipID;
	currentChipID << "VFAT" << i;

	std::string vfat;
	vfat=currentChipID.str();
	currentChipID.str("");

	vfat_shared_ptr tmpVFATDevice(new gem::hw::vfat::HwVFAT2(vfat, tmpURI.str(),
                                                                 "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));

	if (tmpVFATDevice->isHwConnected()) {
          tmpVFATDevice->setDeviceBaseNode(toolbox::toString("GLIB.OptoHybrid_%d.OptoHybrid.GEB.VFATS.%s",
                                                             confParams_.bag.ohGTXLink.value_,
                                                             vfat.c_str()));

	  tmpVFATDevice->setDeviceIPAddress(confParams_.bag.deviceIP);
	  tmpVFATDevice->setRunMode(0);
	  VFATdeviceConnected.push_back(tmpVFATDevice);

	  std::string VfatName = confParams_.bag.deviceName[i].toString();
	  if (VfatName != "") {
	    readout_mask = confParams_.bag.ohGTXLink;

	    DEBUG(" webInitialize : DeviceName " << VfatName );
	    DEBUG(" webInitialize : readout_mask 0x"  << std::hex << (int)readout_mask << std::dec );

	    confParams_.bag.deviceChipID = tmpVFATDevice->getChipID();
	    DEBUG(" CHIPID   :: " << confParams_.bag.deviceChipID);
	    // need to put all chips in sleep mode to start off
	    vfatDevice_.push_back(tmpVFATDevice);
	  }//end if VfatName
	}//end for

	for (auto chip = VFATdeviceConnected.begin(); chip != VFATdeviceConnected.end(); ++chip) {
	  if ((*chip)->isHwConnected()) {
	    (*chip)->setRunMode(0);
	    DEBUG( "vfatDevice Conected::" << (*chip)->getSlot());
	  }
	}// end for

	for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	  DEBUG( "vfatDevice selected::" << (*chip)->getSlot());
	}
      }// end for
   }//end if OH connected
    else {
      DEBUG("OptoHybrid device not connected, breaking out");
      is_configured_  = false;
      is_working_     = false;
      hw_semaphore_.give();
      return;
    }

  }// end if glib connected
  else {
    DEBUG("GLIB device not connected, breaking out");
    is_configured_  = false;
    is_working_     = false;
    hw_semaphore_.give();
    return;
  }

  is_initialized_ = true;
  hw_semaphore_.give();

  //sleep(5);
  is_working_     = false;
}

void gem::supervisor::tbutils::GEMTBUtil::configureAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  is_working_ = true;

  setLogLevelTo(uhal::Debug());  // Set uHAL logging level Debug (most) to Error (least)

  hw_semaphore_.take();
  is_initialized_ = true;

  std::stringstream ss;
  auto num = confParams_.bag.deviceNum.begin();
  for (auto chip = confParams_.bag.deviceName.begin();
       chip != confParams_.bag.deviceName.end(); ++chip, ++num) {
    ss << "Device name: " << chip->toString() << std::endl;
  }
  DEBUG(ss.str());

  hw_semaphore_.give();

  is_working_     = false;

}


void gem::supervisor::tbutils::GEMTBUtil::startAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  is_working_ = true;

  //start scan routine
  wl_->submit(runSig_);

  is_working_ = false;
}


void gem::supervisor::tbutils::GEMTBUtil::stopAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  is_working_ = true;

  glibDevice_->writeReg("GLIB.TTC.CONTROL.INHIBIT_L1A",0x1);
  disableTriggers();

  if (is_running_) {
    hw_semaphore_.take();
    for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
      (*chip)->setRunMode(0);
    }

    hw_semaphore_.give();
    is_running_ = false;
  }

  wl_->submit(stopSig_);

  gem::utils::soap::GEMSOAPToolBox::sendCommand("Stop",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Manager", 0));  // this should not be hard coded
  gem::utils::soap::GEMSOAPToolBox::sendCommand("Stop",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::glib::GLIBManager", 0));  // this should not be hard coded
  gem::utils::soap::GEMSOAPToolBox::sendCommand("Stop",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Readout", 0));  // this should not be hard coded


  sleep(0.001);

  is_working_ = false;
}


void gem::supervisor::tbutils::GEMTBUtil::haltAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  is_working_    = true;
  is_configured_ = false;
  is_running_    = false;

  if (is_running_) {
    hw_semaphore_.take();
    /*int islot=0;
      for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip, ++islot) {
      (*chip)->setRunMode(0x0);
      }
    */
    hw_semaphore_.give();
  }
  is_running_ = false;

  is_configured_ = false;

  gem::utils::soap::GEMSOAPToolBox::sendCommand("Halt",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Manager", 0));  // this should not be hard coded
  gem::utils::soap::GEMSOAPToolBox::sendCommand("Halt",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::glib::GLIBManager", 0));  // this should not be hard coded
  gem::utils::soap::GEMSOAPToolBox::sendCommand("Halt",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Readout", 0));  // this should not be hard coded


  wl_->submit(haltSig_);

  //sleep(5);
  sleep(0.001);
  is_working_    = false;
}


void gem::supervisor::tbutils::GEMTBUtil::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  is_working_ = true;

  is_initialized_ = false;
  is_configured_  = false;
  is_running_     = false;

  gem::utils::soap::GEMSOAPToolBox::sendCommand("Reset",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Manager", 0));  // this should not be hard coded
  gem::utils::soap::GEMSOAPToolBox::sendCommand("Reset",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::glib::GLIBManager", 0));  // this should not be hard coded
  gem::utils::soap::GEMSOAPToolBox::sendCommand("Reset",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Readout", 0));  // this should not be hard coded


  hw_semaphore_.take();
  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip)
    (*chip)->setRunMode(0x0);

  for (int i = 0; i < 24; ++i)
    confParams_.bag.deviceName[i] = ""; // ensure that the selected chips are reset

  confParams_.bag.ohGTXLink = 0; // reset this to 0

  //sleep(2);
  hw_semaphore_.give();

  //reset parameters to defaults, allow to select new device
  confParams_.bag.nTriggers = 2U;

  //  confParams_.bag.deviceName   = "";
  confParams_.bag.deviceChipID = 0x0;
  confParams_.bag.triggersSeen = 0;
  confParams_.bag.triggercount = 0;
  //  confParams_.bag.triggerSource = 0x9;

  wl_->submit(resetSig_);

  //sleep(5);
  sleep(0.001);
  is_working_     = false;
}


void gem::supervisor::tbutils::GEMTBUtil::noAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  is_working_ = false;
  //hw_semaphore_.take();
  ////vfatDevice_->setRunMode(0);
  //hw_semaphore_.give();
}

void gem::supervisor::tbutils::GEMTBUtil::selectMultipleVFAT(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    bool isDisabled = false;
    if (is_running_ || is_configured_ || is_initialized_)
      isDisabled = true;

    const int nChips = 24;
    *out << cgicc::table();
    *out << cgicc::tr();


    for (int i = 0; i < nChips; ++i) {
      std::stringstream currentChipID;
      currentChipID << "VFAT" << i;

      std::stringstream form;
      form << "VFATDevice" << i;

      std::string label = "primary";
      cgicc::input vfatselection;
      *out << cgicc::td() << std::endl;

      *out << "<span class=\"label label-primary\">" << currentChipID.str() << "</span>" << std::endl;

      if (isDisabled) {
        vfatselection.set("type","checkbox").set("name",form.str()).set("disabled","disabled");
      } else {
        vfatselection.set("type","checkbox").set("name",form.str());
      }
      *out << ((confParams_.bag.deviceName[i].toString().compare(currentChipID.str())) == 0 ?
	       vfatselection.set("checked","checked").set("multiple","multiple") :
	       vfatselection.set("value",currentChipID.str())) << std::endl;

      *out << cgicc::td() << std::endl;
      if( i == 7 || i == 15) {
	*out << cgicc::tr() << std::endl  // close
	     << cgicc::tr() << std::endl; // open
      } // end if

    }// end for nChips

    *out << cgicc::tr()    << std::endl;
    *out << cgicc::table() << std::endl;
  } catch (const xgi::exception::Exception& e) {
    ERROR("Something went wrong displaying VFATS(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception& e) {
    ERROR("Something went wrong displaying VFATS(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

void gem::supervisor::tbutils::GEMTBUtil::selectOptohybridDevice(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    bool isDisabled = false;
    if (is_running_ || is_configured_ || is_initialized_)
      isDisabled = true;

    // cgicc::input OHselection;
    *out   << "<table>"     << std::endl
	   << "<tr>"   << std::endl //open
	   << "<td>" << "OH GTX Link " << "</td>" << std::endl
	   << "</tr>"     << std::endl //close

	   << "<tr>" << std::endl //open
	   << "<td>" << std::endl; //open
    if (isDisabled)
      *out << cgicc::select().set("name","SetOH").set("disabled","disabled")
	   << cgicc::option("OH_0").set("value","OH_0")
	   << cgicc::option("OH_1").set("value","OH_1")
	   << cgicc::select().set("disabled","disabled") << std::endl
	   << "</td>"    << std::endl
	   << "</tr>"    << std::endl
	   << "</table>" << std::endl;
    else
      *out << cgicc::select().set("name","SetOH") << std::endl
	   << cgicc::option("OH_0").set("value","OH_0")
	   << cgicc::option("OH_1").set("value","OH_1")
	   << cgicc::select()<< std::endl
	   << "</td>"    << std::endl
	   << "</tr>"    << std::endl
	   << "</table>" << std::endl;

    /*      *out << "<tr><td class=\"title\"> Select Latency Scan: </td>"
	    << "<td class=\"form\">"*/

  } catch (const xgi::exception::Exception& e) {
    ERROR("Something went wrong setting the trigger source): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception& e) {
    ERROR("Something went wrong setting the trigger source): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}// end void selectoptohybrid

void gem::supervisor::tbutils::GEMTBUtil::NTriggersAMC13()
  throw (xgi::exception::Exception)
{
  DEBUG("-----------start SOAP message modify paramteres AMC13------ ");

  xdaq::ApplicationDescriptor *d = getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Manager", 0);  // this should not be hard coded
  xdaq::ApplicationDescriptor *o = this->getApplicationDescriptor();
  std::string    appUrn   = "urn:xdaq-application:"+d->getClassName();

  xoap::MessageReference  msg = xoap::createMessage();
  xoap::SOAPPart         soap = msg->getSOAPPart();
  xoap::SOAPEnvelope envelope = soap.getEnvelope();
  xoap::SOAPName   parameterset = envelope.createName("ParameterSet","xdaq",XDAQ_NS_URI);
  xoap::SOAPElement   container = envelope.getBody().addBodyElement(parameterset);
  container.addNamespaceDeclaration("xsd","http://www.w3.org/2001/XMLSchema");
  container.addNamespaceDeclaration("xsi","http://www.w3.org/2001/XMLSchema-instance");
  //  container.addNamespaceDeclaration("parameterset","http://schemas.xmlsoap.org/soap/encoding/");
  xoap::SOAPName    tname_param = envelope.createName("type","xsi","http://www.w3.org/2001/XMLSchema-instance");
  xoap::SOAPName pboxname_param = envelope.createName("Properties","props",appUrn);
  xoap::SOAPElement  pbox_param = container.addChildElement(pboxname_param);
  pbox_param.addAttribute(tname_param,"soapenc:Struct");

  xoap::SOAPName pboxname_amc13config = envelope.createName("amc13ConfigParams","props",appUrn);
  xoap::SOAPElement  pbox_amc13config = pbox_param.addChildElement(pboxname_amc13config);
  pbox_amc13config.addAttribute(tname_param,"soapenc:Struct");

  xoap::SOAPName soapName_l1A = envelope.createName("L1Aburst","props",appUrn);
  xoap::SOAPElement    cs_l1A = pbox_amc13config.addChildElement(soapName_l1A);
  cs_l1A.addAttribute(tname_param,"xsd:unsignedInt");
  cs_l1A.addTextNode(confParams_.bag.nTriggers.toString());


  std::string tool;
  xoap::dumpTree(msg->getSOAPPart().getEnvelope().getDOMNode(),tool);
  DEBUG("msg: " << tool);

  try {
    DEBUG("trying to send parameters");
    xoap::MessageReference reply = getApplicationContext()->postSOAP(msg, *o,  *d);
    std::string tool;
    xoap::dumpTree(reply->getSOAPPart().getEnvelope().getDOMNode(),tool);
    DEBUG("reply: " << tool);
  } catch (xoap::exception::Exception& e) {
    ERROR("------------------Fail  AMC13 configuring parameters message " << e.what());
    XCEPT_RETHROW (xoap::exception::Exception, "Cannot send message", e);
  } catch (xdaq::exception::Exception& e) {
    ERROR("------------------Fail  AMC13 configuring parameters message " << e.what());
    XCEPT_RETHROW (xoap::exception::Exception, "Cannot send message", e);
  } catch (std::exception& e) {
    ERROR("------------------Fail  AMC13 configuring parameters message " << e.what());
    //XCEPT_RETHROW (xoap::exception::Exception, "Cannot send message", e);
  } catch (...) {
    ERROR("------------------Fail  AMC13 configuring parameters message ");
    XCEPT_RAISE (xoap::exception::Exception, "Cannot send message");
  }
  DEBUG("-----------The message to AMC13 configuring parameters has been sent------------");
}

void gem::supervisor::tbutils::GEMTBUtil::enableTriggers()
  throw (xgi::exception::Exception)
{
  //  is_working_ = true;
  gem::utils::soap::GEMSOAPToolBox::sendCommand("enableTriggers",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Manager", 0));  // this should not be hard coded
}

void gem::supervisor::tbutils::GEMTBUtil::disableTriggers()
  throw (xgi::exception::Exception)
{
  //  is_working_ = true;
  gem::utils::soap::GEMSOAPToolBox::sendCommand("disableTriggers",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Manager", 0));  // this should not be hard coded
}

void gem::supervisor::tbutils::GEMTBUtil::sendTriggers()
  throw (xgi::exception::Exception)
{
  //  is_working_ = true;
  gem::utils::soap::GEMSOAPToolBox::sendCommand("sendtriggerburst",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Manager", 0));  // this should not be hard coded
}

void gem::supervisor::tbutils::GEMTBUtil::AMC13TriggerSetup()
  throw (xgi::exception::Exception)
{
  DEBUG("-----------start SOAP message modify paramteres AMC13------ ");

  xdaq::ApplicationDescriptor *d = getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Manager", 0);  // this should not be hard coded
  xdaq::ApplicationDescriptor *o = this->getApplicationDescriptor();
  std::string             appUrn = "urn:xdaq-application:"+d->getClassName();

  xoap::MessageReference  msg = xoap::createMessage();
  xoap::SOAPPart         soap = msg->getSOAPPart();
  xoap::SOAPEnvelope envelope = soap.getEnvelope();

  // create the ParameterSet message part
  xoap::SOAPName   parameterset = envelope.createName("ParameterSet","xdaq",XDAQ_NS_URI);
  xoap::SOAPElement   container = envelope.getBody().addBodyElement(parameterset);
  container.addNamespaceDeclaration("xsd",    "http://www.w3.org/2001/XMLSchema");
  container.addNamespaceDeclaration("xsi",    "http://www.w3.org/2001/XMLSchema-instance");
  container.addNamespaceDeclaration("soapenc","http://schemas.xmlsoap.org/soap/encoding/");

  // Create the Properties element
  xoap::SOAPName      xsi_type = envelope.createName("type",     "xsi",    "http://www.w3.org/2001/XMLSchema-instance");
  xoap::SOAPName   soapenc_arr = envelope.createName("arrayType","soapenc","http://schemas.xmlsoap.org/soap/encoding/");
  xoap::SOAPName   soapenc_pos = envelope.createName("position", "soapenc","http://schemas.xmlsoap.org/soap/encoding/");
  xoap::SOAPName     pbox_name = envelope.createName("Properties","props", appUrn);
  xoap::SOAPElement pbox_param = container.addChildElement(pbox_name);
  pbox_param.addAttribute(xsi_type,"soapenc:Struct");

  // Create the amc13ConfigParams element
  xoap::SOAPName    amc13config_name  = envelope.createName("amc13ConfigParams","props",appUrn);
  xoap::SOAPElement amc13config_param = pbox_param.addChildElement(amc13config_name);
  amc13config_param.addAttribute(xsi_type,"soapenc:Struct");

  // Create the LocalTriggerConfig element
  xoap::SOAPName     tc_name = envelope.createName("LocalTriggerConfig","props",appUrn);
  xoap::SOAPElement tc_param = amc13config_param.addChildElement(tc_name);
  tc_param.addAttribute(xsi_type,"soapenc:Struct");

  xoap::SOAPName  uselocall1A_name = envelope.createName("EnableLocalL1A","props",appUrn);
  xoap::SOAPElement tc_uselocall1A = tc_param.addChildElement(uselocall1A_name);
  tc_uselocall1A.addAttribute(xsi_type,"xsd:boolean");
  tc_uselocall1A.addTextNode(confParams_.bag.useLocalTriggers.toString());

  xoap::SOAPName  l1Amode_name = envelope.createName("L1Amode","props",appUrn);
  xoap::SOAPElement tc_l1Amode = tc_param.addChildElement(l1Amode_name);
  tc_l1Amode.addAttribute(xsi_type,"xsd:integer");
  tc_l1Amode.addTextNode(confParams_.bag.localTriggerPeriod.toString());

  xoap::SOAPName  l1Anumber_name = envelope.createName("L1Aburst","props",appUrn);
  xoap::SOAPElement tc_l1Anumber = tc_param.addChildElement(l1Anumber_name);
  tc_l1Anumber.addAttribute(xsi_type,"xsd:unsignedInt");
  tc_l1Anumber.addTextNode(confParams_.bag.nTriggers.toString());

  xoap::SOAPName  l1Aperiod_name = envelope.createName("InternalPeriodicPeriod","props",appUrn);
  xoap::SOAPElement tc_l1Aperiod = tc_param.addChildElement(l1Aperiod_name);
  tc_l1Aperiod.addAttribute(xsi_type,"xsd:unsignedInt");
  tc_l1Aperiod.addTextNode(confParams_.bag.localTriggerPeriod.toString());

  xoap::SOAPName  enableTrigCont_name = envelope.createName("startL1ATricont","props",appUrn);
  xoap::SOAPElement tc_enableTrigCont = tc_param.addChildElement(enableTrigCont_name);
  tc_enableTrigCont.addAttribute(xsi_type,"xsd:boolean");
  tc_enableTrigCont.addTextNode(confParams_.bag.EnableTrigCont.toString());

  // Create the BGOConfig element
  xoap::SOAPName     bgoarray_name = envelope.createName("BGOConfig","props", appUrn);
  xoap::SOAPElement bgoarray_param = amc13config_param.addChildElement(bgoarray_name);
  bgoarray_param.addAttribute(xsi_type,   "soapenc:Array");
  bgoarray_param.addAttribute(soapenc_arr,"xsd:ur-type[4]");
  
  // Create the BGOInfo element
  xoap::SOAPName     bc_name = envelope.createName("BGOInfo","props", appUrn);
  xoap::SOAPElement bc_param = bgoarray_param.addChildElement(bc_name);
  bc_param.addAttribute(xsi_type,"soapenc:Struct");
  bc_param.addAttribute(soapenc_pos,"0");

  xoap::SOAPName  bgoChan_name = envelope.createName("BGOChannel","props",appUrn);
  xoap::SOAPElement bc_bgoChan = bc_param.addChildElement(bgoChan_name);
  bc_bgoChan.addAttribute(xsi_type,"xsd:integer");
  bc_bgoChan.addTextNode("0");
  
  xoap::SOAPName  bgoCmd_name = envelope.createName("BGOcmd","props",appUrn);
  xoap::SOAPElement bc_bgoCmd = bc_param.addChildElement(bgoCmd_name);
  bc_bgoCmd.addAttribute(xsi_type,"xsd:unsignedInt");
  bc_bgoCmd.addTextNode("20");

  xoap::SOAPName  bgoBX_name = envelope.createName("BGObx","props",appUrn);
  xoap::SOAPElement bc_bgoBX = bc_param.addChildElement(bgoBX_name);
  bc_bgoBX.addAttribute(xsi_type,"xsd:unsignedInt");
  bc_bgoBX.addTextNode("1");

  xoap::SOAPName  bgoRepeat_name = envelope.createName("BGOrepeat","props",appUrn);
  xoap::SOAPElement bc_bgoRepeat = bc_param.addChildElement(bgoRepeat_name);
  bc_bgoRepeat.addAttribute(xsi_type,"xsd:boolean");
  bc_bgoRepeat.addTextNode("true");

  xoap::SOAPName  bgoPrescale_name = envelope.createName("BGOprescale","props",appUrn);
  xoap::SOAPElement bc_bgoPrescale = bc_param.addChildElement(bgoPrescale_name);
  bc_bgoPrescale.addAttribute(xsi_type,"xsd:unsignedInt");
  bc_bgoPrescale.addTextNode("1");

  std::string tool;
  xoap::dumpTree(msg->getSOAPPart().getEnvelope().getDOMNode(),tool);
  DEBUG("msg: " << tool);

  try {
    DEBUG("trying to send parameters");
    xoap::MessageReference reply = getApplicationContext()->postSOAP(msg, *o,  *d);
    std::string tool;
    xoap::dumpTree(reply->getSOAPPart().getEnvelope().getDOMNode(),tool);
    DEBUG("reply: " << tool);
  } catch (xoap::exception::Exception& e) {
    ERROR("------------------Fail  AMC13 configuring parameters message " << e.what());
    XCEPT_RETHROW (xoap::exception::Exception, "Cannot send message", e);
  } catch (xdaq::exception::Exception& e) {
    ERROR("------------------Fail  AMC13 configuring parameters message " << e.what());
    XCEPT_RETHROW (xoap::exception::Exception, "Cannot send message", e);
  } catch (std::exception& e) {
    ERROR("------------------Fail  AMC13 configuring parameters message " << e.what());
    //XCEPT_RETHROW (xoap::exception::Exception, "Cannot send message", e);
  } catch (...) {
    ERROR("------------------Fail  AMC13 configuring parameters message ");
    XCEPT_RAISE (xoap::exception::Exception, "Cannot send message");
  }
  DEBUG("-----------The message to AMC13 configuring parameters has been sent------------");
}
