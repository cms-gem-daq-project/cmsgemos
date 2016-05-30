#include "gem/supervisor/tbutils/LatencyScan.h"

#include "gem/hw/vfat/HwVFAT2.h"
#include "gem/hw/glib/HwGLIB.h"
#include "gem/hw/optohybrid/HwOptoHybrid.h"

#include "gem/utils/GEMLogging.h"
#include "gem/utils/soap/GEMSOAPToolBox.h"

#include <algorithm>
#include <iomanip>
#include <ctime>
#include <queue>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "cgicc/HTTPRedirectHeader.h"
#include "gem/supervisor/tbutils/VFAT2XMLParser.h"

#include <iostream>
#include "xdata/Vector.h"
#include <string>

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPConstants.h"
#include "xoap/SOAPBody.h"
#include "xoap/Method.h"
#include "xoap/AttachmentPart.h"
#include "xoap/domutils.h"


XDAQ_INSTANTIATOR_IMPL(gem::supervisor::tbutils::LatencyScan)

//
void gem::supervisor::tbutils::LatencyScan::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{

  minLatency    = 13U;
  maxLatency    = 17U;
  stepSize      = 1U;
  nTriggers     = 10;
  deviceVT1     = 25;
  deviceVT2     = 0;
  MSPulseLength = 3;
  VCal          = 100;

  bag->addField("minLatency",    &minLatency   );
  bag->addField("maxLatency",    &maxLatency   );
  bag->addField("stepSize",      &stepSize     );
  bag->addField("VT2",           &deviceVT1    );
  bag->addField("VT1",           &deviceVT2    );
  bag->addField("VCal",          &VCal         );
  bag->addField("MSPulseLength", &MSPulseLength);
  bag->addField("nTriggers",     &nTriggers    );

}

gem::supervisor::tbutils::LatencyScan::LatencyScan(xdaq::ApplicationStub * s)  throw (xdaq::exception::Exception) :
  gem::supervisor::tbutils::GEMTBUtil(s)
{

  getApplicationInfoSpace()->fireItemAvailable("scanParams", &scanParams_);
  getApplicationInfoSpace()->fireItemValueRetrieve("scanParams", &scanParams_);

  // HyperDAQ bindings
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webDefault,      "Default"    );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webConfigure,    "Configure"  );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webStart,        "Start"      );
  runSig_   = toolbox::task::bind(this, &LatencyScan::run,        "run"       );

  // Initiate and activate main workloop
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:LatencyScan","waiting");
  wl_->activate();

  currentLatency_ = 0;

  scanpoint_ = true; // never initialized
  /*
  confParams_.bag.useLocalTriggers   = false;
  confParams_.bag.localTriggerMode   = 0;
  confParams_.bag.localTriggerPeriod = 1;
  */

  confParams_.bag.useLocalTriggers   = false;
  confParams_.bag.localTriggerMode   = 0; // per orbit
  confParams_.bag.EnableTrigCont     = false;
  confParams_.bag.localTriggerPeriod = 1;


  disableTriggers();
}

gem::supervisor::tbutils::LatencyScan::~LatencyScan()
{
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:LatencyScan","waiting");
  //should we check to see if it's running and try to stop?
  wl_->cancel();
  wl_ = 0;
}
bool gem::supervisor::tbutils::LatencyScan::run(toolbox::task::WorkLoop* wl)
{
  wl_semaphore_.take(); // take workloop
  if (!is_running_) {
    //uint32_t bufferDepth = 0;
    hw_semaphore_.take();
    //bufferDepth = glibDevice_->getFIFOVFATBlockOccupancy(readout_mask);
    hw_semaphore_.give();
    TRACE(" ******IT IS NOT RUNNIG ***** ");
    wl_semaphore_.give(); // give work loop if it is not running
    return false;
  }

  hw_semaphore_.take();//take hw to set the trigger source, send L1A+Cal pulses,

  optohybridDevice_->setTrigSource(0x0);// trigger sources
  //count triggers and Calpulses coming from TTC

  if (scanpoint_) {
    enableTriggers();
    glibDevice_->writeReg("GLIB.TTC.CONTROL.INHIBIT_L1A",0x0);
  }

  confParams_.bag.triggersSeen =  optohybridDevice_->getL1ACount(0x0);
  CalPulseCount_[0] = optohybridDevice_->getCalPulseCount(0x0);

  TRACE("ABC TriggersSeen " << confParams_.bag.triggersSeen << " Calpulse " << optohybridDevice_->getCalPulseCount(0x0));

  hw_semaphore_.give();//give hw to set the trigger source, send L1A+Cal pulses,

  // if triggersSeen < N triggers
  if ((uint64_t)(confParams_.bag.triggersSeen) < (uint64_t)(confParams_.bag.nTriggers)) {
    hw_semaphore_.take();//take hw. glib buffer depth

    scanpoint_=false;

    hw_semaphore_.give();//give hw. glib buffer depth
    wl_semaphore_.give();//give workloop to read
    return true;
  }// end triggerSeen < N triggers
  else {
    disableTriggers();
    glibDevice_->writeReg("GLIB.TTC.CONTROL.INHIBIT_L1A",0x1);

    int counter = 3;
    while (counter > 0) {
      confParams_.bag.triggersSeen = optohybridDevice_->getL1ACount(0x0);
      TRACE(" ABC Scan point TriggersSeen "
           << confParams_.bag.triggersSeen
           << " Calpulse " << optohybridDevice_->getCalPulseCount(0x0)
           << " counter = " << counter);
      sleep(1);
      --counter;
    }
    //uint32_t bufferDepth = 0;
    //bufferDepth = glibDevice_->getFIFOVFATBlockOccupancy(readout_mask);

    // have to be sure that no more triggers are coming here!!!
    sleep(0.005);
    // and if no more triggers are coming, then this step *should* be unnecessary
    // also better to do a broadcast write
    for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
      (*chip)->setRunMode(0);
    }// end for

    scanpoint_=true;

    //reset counters
    optohybridDevice_->resetL1ACount(0x0);
    /*    optohybridDevice_->resetResyncCount();
	  optohybridDevice_->resetBC0Count();
	  optohybridDevice_->resetCalPulseCount(0x0);
	  optohybridDevice_->sendResync();
	  optohybridDevice_->sendBC0();
    */

    confParams_.bag.triggersSeen = optohybridDevice_->getL1ACount(0x0);
    TRACE("ABC Scan point TriggersSeen "
         << confParams_.bag.triggersSeen << " Calpulse " << optohybridDevice_->getCalPulseCount(0x0));

    hw_semaphore_.give(); // give hw to reset counters

    //if max Latency - current Latency >= stepsize
    if (scanParams_.bag.maxLatency - currentLatency_ >= scanParams_.bag.stepSize) {

      hw_semaphore_.take();// vfat set latency

      TRACE("ABC run: Latency= "
           << (int)currentLatency_ << " VT1= "
           << scanParams_.bag.deviceVT1 << " VT2= "
           << scanParams_.bag.deviceVT2
           << " abs(VT2-VT1) "
           << abs(scanParams_.bag.deviceVT2-scanParams_.bag.deviceVT1) );

      if ((currentLatency_ + scanParams_.bag.stepSize) < 0xFF) {
	optohybridDevice_->broadcastWrite("Latency",currentLatency_ + scanParams_.bag.stepSize,0x0,false);
      } else  {
	optohybridDevice_->broadcastWrite("Latency",0xFF,0x0,false);
      }//end else

      sleep(0.01);

      //uint32_t bufferDepth = 0;
      //bufferDepth = glibDevice_->getFIFOVFATBlockOccupancy(readout_mask);

      for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	currentLatency_ = (*chip)->getLatency();
	scanParams_.bag.deviceVT1 = (*chip)->getVThreshold1();
	scanParams_.bag.deviceVT2 = (*chip)->getVThreshold2();
      }
      while (!(glibDevice_->readReg(glibDevice_->getDeviceBaseNode(),
                                    toolbox::toString("DAQ.GTX%d.STATUS.EVENT_FIFO_IS_EMPTY",
                                                      confParams_.bag.ohGTXLink.value_))))
	TRACE("waiting for FIFO is empty: "
	      << glibDevice_->readReg(glibDevice_->getDeviceBaseNode(),
                                      toolbox::toString("DAQ.GTX%d.STATUS.EVENT_FIFO_IS_EMPTY",
                                                        confParams_.bag.ohGTXLink.value_))
	      );

      glibDevice_->setDAQLinkRunParameter(1,currentLatency_);

      for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	(*chip)->setRunMode(1);
      }

      //setting counters  = 0
      optohybridDevice_->resetL1ACount(0x0);
      confParams_.bag.triggersSeen = 0;
      CalPulseCount_[0] = 0;

      hw_semaphore_.give(); // give hw vfat set latency
      wl_semaphore_.give(); // end of workloop

      return true;
    } // end if maxLat - curreLat >= step
    else {
      hw_semaphore_.take(); // take hw to stop workloop
      scanpoint_=false; //
      disableTriggers();
      glibDevice_->writeReg("GLIB.TTC.CONTROL.INHIBIT_L1A",0x1);
      wl_->submit(stopSig_);
      hw_semaphore_.give(); // give hw to stop workloop
      wl_semaphore_.give(); // end of workloop
      return true;
    }//end else

  }//end else triggerseen < N triggers

  return false;
}//end run

void gem::supervisor::tbutils::LatencyScan::scanParameters(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    *out << cgicc::span()   << std::endl //open span

	 << cgicc::label("MinLatency").set("for","MinLatency") << std::endl
	 << cgicc::input().set("id","MinLatency").set("name","MinLatency")
      .set("type","number").set("min","0").set("max","255")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.minLatency)))
	 << std::endl

	 << cgicc::label("MaxLatency").set("for","MaxLatency") << std::endl
	 << cgicc::input().set("id","MaxLatency").set("name","MaxLatency")
      .set("type","number").set("min","0").set("max","255")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.maxLatency)))
	 << std::endl

	 << cgicc::label("Latency Step").set("for","LatencyStep") << std::endl
	 << cgicc::input().set("id","LatencyStep").set("name","LatencyStep")
      .set("type","number").set("min","1").set("max","255")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.stepSize)))
	 << std::endl

	 << cgicc::br() << std::endl

	 << cgicc::label("Current Latency").set("for","CurrentLatency") << std::endl
	 << cgicc::input().set("id","CurrentLatency").set("name","CurrentLatency")
      .set("type","text").set("readonly")
      .set("value",boost::str(boost::format("%d")%((unsigned)currentLatency_)))
	 << std::endl

	 << cgicc::br() << std::endl

	 << cgicc::label("Triggers to take").set("for","TriggersToTake") << std::endl
	 << cgicc::input().set("id","TriggersToTake").set("name","TriggersToTake")
      .set("type","number").set("min","0")
      .set("value",boost::str(boost::format("%d")%(confParams_.bag.nTriggers)))
	 << std::endl

	 << cgicc::label("Triggers taken").set("for","TriggersSeen") << std::endl
	 << cgicc::input().set("id","TriggersSeen").set("name","TriggersSeen")
      .set("type","text").set("readonly")
      .set("value",boost::str(boost::format("%d")%(confParams_.bag.triggersSeen)))
      //.set("value",boost::str(boost::format("%d")%(triggersSeen_)))
	 << std::endl

	 << cgicc::br() << std::endl

	 << cgicc::label("Set Threshold (VT2-VT1):  VT1 ").set("for","VT1") << std::endl
	 << cgicc::input().set("id","VT1").set("name","VT1")
      .set("type","number").set("min","0").set("max","255")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.deviceVT1)))
	 << std::endl

	 << cgicc::label("VT2").set("for","VT2") << std::endl
	 << cgicc::input().set("id","VT2").set("name","VT2")
      .set("type","number").set("min","0").set("max","255")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.deviceVT2)))
	 << std::endl

	 << cgicc::br()  // << std::endl

	 << cgicc::label("VCal").set("for","VCal") << std::endl
	 << cgicc::input().set("id","VCal").set("name","VCal")
      .set("type","number").set("min","0").set("max","200")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.VCal)))
	 << std::endl

      	 << cgicc::label("MSPulseLength").set("for","MSPulseLength") << std::endl
	 << cgicc::input().set("id","MSPulseLength").set("name","MSPulseLength")
      .set("type","number").set("min","0").set("max","3")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.MSPulseLength)))
	 << std::endl
	 << cgicc::br()
	 << cgicc::span() << std::endl; //end span
  } catch (const xgi::exception::Exception& e) {
    ERROR("Something went wrong displaying VFATS(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception& e) {
    ERROR("Something went wrong displaying VFATS(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

//
void gem::supervisor::tbutils::LatencyScan::webDefault(xgi::Input *in, xgi::Output *out)
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
      head.addHeader("Refresh","5");
    }

    //generate the control buttons and display the ones that can be touched depending on the run mode
    *out << "<div class=\"xdaq-tab-wrapper\">"            << std::endl; //open all tabs
    *out << "<div class=\"xdaq-tab\" title=\"Control\">"  << std::endl; // open control tab

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
      //have a menu for selecting the OH and VFAT
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Initialize") << std::endl;

      selectOptohybridDevice(out);
      selectMultipleVFAT(out);
      scanParameters(out);

      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Initialize hardware acces.")
	.set("value", "Initialize") << std::endl;

      *out << cgicc::form() << std::endl;
    }

    else if (!is_configured_) {
      //this will allow the parameters to be set to the chip and scan routine and the trigger source
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Configure") << std::endl;

      selectOptohybridDevice(out);
      selectMultipleVFAT(out);
      //      selectTrigSource(out);
      scanParameters(out);

      *out << cgicc::input().set("type","text").set("name","xmlFilename").set("size","80")
	.set("ENCTYPE","multipart/form-data").set("readonly")
	.set("value",confParams_.bag.settingsFile.toString()) << std::endl;

      *out << cgicc::br() << std::endl;
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Configure scan.")
	.set("value", "Configure") << std::endl;
      *out << cgicc::form()        << std::endl;
    }

    else if (!is_running_) {

      //hardware is initialized and configured, we can start the run
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Start") << std::endl;

      selectOptohybridDevice(out);
      selectMultipleVFAT(out);
      //      selectTrigSource(out);
      scanParameters(out);

      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Start scan.")
	.set("value", "Start") << std::endl;
      *out << cgicc::form()    << std::endl;
    }

    else if (is_running_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Stop") << std::endl;

      selectOptohybridDevice(out);
      selectMultipleVFAT(out);
      //      selectTrigSource(out);
      scanParameters(out);

      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Stop scan.")
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
      .set("name", "command").set("title", "Halt scan.")
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

    *out << "</div>" << std::endl;//close control

    *out << "<div class=\"xdaq-tab\" title=\"Counters\">"  << std::endl; // open counters
    if (is_initialized_)
      showCounterLayout(out);
    *out << "</div>" << std::endl;//close counters

    *out << "<div class=\"xdaq-tab\" title=\"Fast Commands/Trigger Setup\">"  << std::endl;//open fast commands
    if (is_initialized_)
      fastCommandLayout(out);
    *out << "</div>" << std::endl; //close fastcommands

    *out << "</div>" << std::endl;//close total tabs

    *out << cgicc::br() << cgicc::br() << std::endl;

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
    //<< "</div>"   << std::endl;

    *out << cgicc::script().set("type","text/javascript")
      .set("src","http://ajax.googleapis.com/ajax/libs/jquery/1/jquery.min.js")
	 << cgicc::script() << std::endl;
    *out << cgicc::script().set("type","text/javascript")
      .set("src","http://ajax.googleapis.com/ajax/libs/jqueryui/1/jquery-ui.min.js")
	 << cgicc::script() << std::endl;

  } catch (const xgi::exception::Exception& e) {
    ERROR("Something went wrong displaying LatencyScan control panel(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception& e) {
    ERROR("Something went wrong displaying LatencyScan control panel(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

//
void gem::supervisor::tbutils::LatencyScan::webConfigure(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {

  try {
    cgicc::Cgicc cgi(in);

    //aysen's xml parser
    confParams_.bag.settingsFile = cgi.getElement("xmlFilename")->getValue();

    cgicc::const_form_iterator element  = cgi.getElement("MinLatency");
    if (element != cgi.getElements().end())
      scanParams_.bag.minLatency = element->getIntegerValue();

    element = cgi.getElement("MaxLatency");
    if (element != cgi.getElements().end())
      scanParams_.bag.maxLatency = element->getIntegerValue();

    element = cgi.getElement("LatencyStep");
    if (element != cgi.getElements().end())
      scanParams_.bag.stepSize  = element->getIntegerValue();

    element = cgi.getElement("TriggersToTake");
    if (element != cgi.getElements().end())
      confParams_.bag.nTriggers  = element->getIntegerValue();

    element = cgi.getElement("VCal");
    if (element != cgi.getElements().end())
      scanParams_.bag.VCal  = element->getIntegerValue();

    element = cgi.getElement("MSPulseLength");
    if (element != cgi.getElements().end())
      scanParams_.bag.MSPulseLength  = element->getIntegerValue();

    element = cgi.getElement("VT1");
    if (element != cgi.getElements().end())
      scanParams_.bag.deviceVT1  = element->getIntegerValue();

    element = cgi.getElement("VT2");
    if (element != cgi.getElements().end())
      scanParams_.bag.deviceVT2  = element->getIntegerValue();


  } catch (const xgi::exception::Exception & e) {
    ERROR("Something went wrong: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception & e) {
    ERROR("Something went wrong: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }

  // Initiate configure workloop
  wl_->submit(confSig_);

  // Go back to main web interface
  redirect(in,out);
}

//
void gem::supervisor::tbutils::LatencyScan::webStart(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  // Initiate start workloop
  wl_->submit(startSig_);
  // Go back to main web interface
  redirect(in,out);
}

void gem::supervisor::tbutils::LatencyScan::configureAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;

  stepSize_   = scanParams_.bag.stepSize;
  minLatency_ = scanParams_.bag.minLatency;
  maxLatency_ = scanParams_.bag.maxLatency;

  gem::utils::soap::GEMSOAPToolBox::sendCommand("Configure",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Manager", 0));  // this should not be hard coded
  gem::utils::soap::GEMSOAPToolBox::sendCommand("Configure",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::glib::GLIBManager", 0));  // this should not be hard coded
  gem::utils::soap::GEMSOAPToolBox::sendCommand("Configure",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Readout", 0));  // this should not be hard coded

  AMC13TriggerSetup();

  confParams_.bag.triggercount = 0;

  hw_semaphore_.take();
  DEBUG( "attempting to configure device");

  //make sure device is not running
  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {

    confParams_.bag.deviceChipID = (*chip)->getChipID();
    (*chip)->setDeviceIPAddress(confParams_.bag.deviceIP);
    (*chip)->setRunMode(0);

    TRACE("loading default settings");
    //default settings for the frontend
    (*chip)->setTriggerMode(    0x3); //set to S1 to S8
    (*chip)->setCalibrationMode(0x0); //set to normal
    (*chip)->setMSPolarity(     0x1); //negative
    (*chip)->setCalPolarity(    0x1); //negative

    (*chip)->setProbeMode(        0x0);
    (*chip)->setLVDSMode(         0x0);
    (*chip)->setHitCountCycleTime(0x0); //maximum number of bits

    (*chip)->setHitCountMode( 0x0);
    (*chip)->setMSPulseLength(scanParams_.bag.MSPulseLength);
    (*chip)->setInputPadMode( 0x0);
    (*chip)->setTrimDACRange( 0x0);
    (*chip)->setBandgapPad(   0x0);
    (*chip)->sendTestPattern( 0x0);

    (*chip)->setVCal(scanParams_.bag.VCal);
    for (int chan = 0; chan < 129; ++chan)
      if (chan == 10 || chan == 60 || chan == 100)
        (*chip)->enableCalPulseToChannel(chan, true);
      else
        (*chip)->enableCalPulseToChannel(chan, false);

    (*chip)->setIPreampIn(  168);
    (*chip)->setIPreampFeed(150);
    (*chip)->setIPreampOut(  80);
    (*chip)->setIShaper(    150);
    (*chip)->setIShaperFeed(100);
    (*chip)->setIComp(       75);

    (*chip)->setVThreshold1(scanParams_.bag.deviceVT1);
    (*chip)->setVThreshold2(scanParams_.bag.deviceVT2);

    TRACE( "setting DAC mode to normal");
    (*chip)->setDACMode(gem::hw::vfat::StringToDACMode.at("OFF"));

    TRACE( "setting starting latency value");
    (*chip)->setLatency(    scanParams_.bag.minLatency);

    TRACE( "reading back current latency value");
    currentLatency_ = (*chip)->getLatency();

    TRACE( "Threshold " << scanParams_.bag.deviceVT1);

    TRACE( "VCal " << scanParams_.bag.VCal);

    TRACE( "device configured");
    is_configured_ = true;
  }

  //flush fifo
  TRACE("Flushing the FIFOs, readout_mask 0x" <<std::hex << (int)readout_mask << std::dec);
  TRACE("Flushing FIFO" << readout_mask << " (depth " << glibDevice_->getFIFOOccupancy(readout_mask));
  glibDevice_->flushFIFO(readout_mask);
  while (glibDevice_->hasTrackingData(readout_mask)) {
    glibDevice_->flushFIFO(readout_mask);
    std::vector<uint32_t> dumping = glibDevice_->getTrackingData(readout_mask,
                                                                 glibDevice_->getFIFOVFATBlockOccupancy(readout_mask));
  }
  // once more for luck
  glibDevice_->flushFIFO(readout_mask);

  glibDevice_->setDAQLinkRunParameter(1,currentLatency_);

  //reset counters
  optohybridDevice_->resetL1ACount(0x0);
  optohybridDevice_->resetCalPulseCount(0x0);
  optohybridDevice_->resetResyncCount();
  optohybridDevice_->resetBC0Count();
  optohybridDevice_->sendResync();
  optohybridDevice_->sendBC0();
  //  vfatDevice_->setRunMode(1);

  hw_semaphore_.give();

  DEBUG( "configure routine completed");

  is_working_    = false;
}

//
void gem::supervisor::tbutils::LatencyScan::startAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  //  wl_semaphore_.take();
  is_working_ = true;

  gem::utils::soap::GEMSOAPToolBox::sendCommand("Start",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Manager", 0));  // this should not be hard coded
  gem::utils::soap::GEMSOAPToolBox::sendCommand("Start",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::glib::GLIBManager", 0));  // this should not be hard coded
  gem::utils::soap::GEMSOAPToolBox::sendCommand("Start",
                                                getApplicationContext(),this->getApplicationDescriptor(),
                                                getApplicationContext()->getDefaultZone()->getApplicationDescriptor("gem::hw::amc13::AMC13Readout", 0));  // this should not be hard coded


  // enableTriggers();
  sleep(1);

  //AppHeader ah;
  threshold_     = scanParams_.bag.deviceVT2 -scanParams_.bag.deviceVT1;
  VCal           = scanParams_.bag.VCal;
  MSPulseLength  = scanParams_.bag.MSPulseLength;
  nTriggers_  = confParams_.bag.nTriggers;
  stepSize_   = scanParams_.bag.stepSize;
  minLatency_ = scanParams_.bag.minLatency;
  maxLatency_ = scanParams_.bag.maxLatency;

  //char data[128/8]
  is_running_ = true;
  hw_semaphore_.take();//oh reset counters

  //flush fifo
  TRACE("Flushing the FIFOs, readout_mask 0x" <<std::hex << (int)readout_mask << std::dec);
  TRACE("Flushing FIFO" << readout_mask << " (depth " << glibDevice_->getFIFOOccupancy(readout_mask));
  glibDevice_->flushFIFO(readout_mask);
  while (glibDevice_->hasTrackingData(readout_mask)) {
    glibDevice_->flushFIFO(readout_mask);
    std::vector<uint32_t> dumping = glibDevice_->getTrackingData(readout_mask,
                                                                 glibDevice_->getFIFOVFATBlockOccupancy(readout_mask));
  }
  // once more for luck
  glibDevice_->flushFIFO(readout_mask);

  //send Resync
  optohybridDevice_->sendResync();
  optohybridDevice_->sendBC0();

  glibDevice_->setDAQLinkRunType(1);
  glibDevice_->setDAQLinkRunParameter(1,currentLatency_);
  glibDevice_->setDAQLinkRunParameter(2,scanParams_.bag.deviceVT1);
  glibDevice_->setDAQLinkRunParameter(3,scanParams_.bag.deviceVT2);

  // set selected VFATs
  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
    (*chip)->setRunMode(1);
  }

  //reset counters
  optohybridDevice_->resetL1ACount(0x0);
  optohybridDevice_->resetResyncCount();
  optohybridDevice_->resetBC0Count();
  optohybridDevice_->resetCalPulseCount(0x0);
  optohybridDevice_->sendResync();
  optohybridDevice_->sendBC0();

  scanpoint_=true;

  wl_->submit(runSig_);

  hw_semaphore_.give();//end vfat

  is_working_ = false;
}
//
void gem::supervisor::tbutils::LatencyScan::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  gem::supervisor::tbutils::GEMTBUtil::resetAction(e);
  is_working_ = true;

  confParams_.bag.nTriggers       = 10U;
  scanParams_.bag.minLatency      = 0U;
  scanParams_.bag.maxLatency      = 25U;
  scanParams_.bag.stepSize        = 1U;
  scanParams_.bag.deviceVT1       = 25U;
  scanParams_.bag.deviceVT2       = 0U;
  scanParams_.bag.VCal            = 100;
  scanParams_.bag.MSPulseLength   = 3;

  is_working_     = false;
}
