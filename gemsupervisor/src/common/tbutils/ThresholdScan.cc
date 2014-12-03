#include "gem/supervisor/tbutils/ThresholdScan.h"
#include "gem/supervisor/tbutils/ThresholdEvent.h"
#include "gem/hw/vfat/HwVFAT2.h"

#include "TH1.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"

#include <algorithm>
#include <ctime>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "cgicc/HTTPRedirectHeader.h"

#include "gem/supervisor/tbutils/VFAT2XMLParser.h"

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::tbutils::ThresholdScan)

void gem::supervisor::tbutils::ThresholdScan::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{
  readoutDelay = 1U; //readout delay in milleseconds/microseconds?

  latency   = 128U;
  nTriggers = 2500U;
  minThresh = -25;
  maxThresh = 0;
  stepSize  = 1U;

  time_t now  = time(0);
  tm    *gmtm = gmtime(&now);
  char* utcTime = asctime(gmtm);
  std::string tmpFileName = "ThresholdScan_";
  tmpFileName.append(utcTime);
  tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
  tmpFileName.append(".dat");
  std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
  std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');
  //std::replace(tmpFileName.begin(), tmpFileName.end(), '\n', '_');

  outFileName  = tmpFileName;
  settingsFile = "${BUILD_HOME}/gemdaq-testing/gemhardware/xml/vfat/vfat_settings.xml";

  deviceName    = "";
  deviceNum     = -1;
  triggerSource = 0x2;
  deviceChipID  = 0x0;
  deviceVT1     = 0x0;
  deviceVT2     = 0x0;

  triggersSeen = 0;
  
  bag->addField("readoutDelay",   &readoutDelay);

  bag->addField("nTriggers",   &nTriggers);
  bag->addField("minThresh",   &minThresh);
  bag->addField("maxThresh",   &maxThresh);
  bag->addField("stepSize",    &stepSize );

  bag->addField("outFileName",   &outFileName );
  bag->addField("settingsFile",  &settingsFile);

  bag->addField("deviceName",   &deviceName  );
  bag->addField("deviceNum",    &deviceNum   );
  bag->addField("deviceChipID", &deviceChipID);
  bag->addField("deviceVT1",    &deviceVT1   );
  bag->addField("deviceVT2",    &deviceVT2   );
  bag->addField("triggersSeen", &triggersSeen);

}

gem::supervisor::tbutils::ThresholdScan::ThresholdScan(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception) :
  xdaq::WebApplication(s),
  fsmP_(0),
  wl_semaphore_(toolbox::BSem::FULL),
  hw_semaphore_(toolbox::BSem::FULL),
  initSig_ (0),
  confSig_ (0),
  startSig_(0),
  stopSig_ (0),
  haltSig_ (0),
  resetSig_(0),
  runSig_  (0),
  readSig_ (0),
  //deviceName_(""),
  //deviceChipID_(0x0),
  is_working_     (false),
  is_initialized_ (false),
  is_configured_  (false),
  is_running_     (false),
  vfatDevice_(0)
{
  
  // Detect when the setting of default parameters has been performed
  this->getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");

  getApplicationInfoSpace()->fireItemAvailable("confParams",   &confParams_);
  //getApplicationInfoSpace()->fireItemAvailable("deviceName",   &deviceName_);
  //getApplicationInfoSpace()->fireItemAvailable("deviceChipID", &deviceChipID_);
  //getApplicationInfoSpace()->fireItemAvailable("deviceVT1",    &deviceVT1_);
  //getApplicationInfoSpace()->fireItemAvailable("deviceVT2",    &deviceVT2_);
  //getApplicationInfoSpace()->fireItemAvailable("triggersSeen", &triggersSeen_);

  getApplicationInfoSpace()->fireItemValueRetrieve("confParams", &confParams_);

  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webDefault,      "Default"    );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webInitialize,   "Initialize" );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webConfigure,    "Configure"  );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webStart,        "Start"      );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webStop,         "Stop"       );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webHalt,         "Halt"       );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webReset,        "Reset"      );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webResetCounters,"ResetCounters");
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webSendFastCommands,"FastCommands");
  
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onInitialize,  "Initialize",  XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onConfigure,   "Configure",   XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onStart,       "Start",       XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onStop,        "Stop",        XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onHalt,        "Halt",        XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ThresholdScan::onReset,       "Reset",       XDAQ_NS_URI);
  
  initSig_  = toolbox::task::bind(this, &ThresholdScan::initialize, "initialize");
  confSig_  = toolbox::task::bind(this, &ThresholdScan::configure,  "configure" );
  startSig_ = toolbox::task::bind(this, &ThresholdScan::start,      "start"     );
  stopSig_  = toolbox::task::bind(this, &ThresholdScan::stop,       "stop"      );
  haltSig_  = toolbox::task::bind(this, &ThresholdScan::halt,       "halt"      );
  resetSig_ = toolbox::task::bind(this, &ThresholdScan::reset,      "reset"     );
  runSig_   = toolbox::task::bind(this, &ThresholdScan::run,        "run"       );
  readSig_  = toolbox::task::bind(this, &ThresholdScan::readFIFO,   "readFIFO"  );

  fsmP_ = new toolbox::fsm::AsynchronousFiniteStateMachine("GEMTestBeamThresholdScan");
  
  fsmP_->addState('I', "Initial",     this, &gem::supervisor::tbutils::ThresholdScan::stateChanged);
  fsmP_->addState('H', "Halted",      this, &gem::supervisor::tbutils::ThresholdScan::stateChanged);
  fsmP_->addState('C', "Configured",  this, &gem::supervisor::tbutils::ThresholdScan::stateChanged);
  fsmP_->addState('E', "Running",     this, &gem::supervisor::tbutils::ThresholdScan::stateChanged);
  
  fsmP_->setStateName('F', "Error");
  fsmP_->setFailedStateTransitionAction(this,  &gem::supervisor::tbutils::ThresholdScan::transitionFailed);
  fsmP_->setFailedStateTransitionChanged(this, &gem::supervisor::tbutils::ThresholdScan::stateChanged);
  
  fsmP_->addStateTransition('I', 'H', "Initialize", this, &gem::supervisor::tbutils::ThresholdScan::initializeAction);
  fsmP_->addStateTransition('H', 'C', "Configure",  this, &gem::supervisor::tbutils::ThresholdScan::configureAction);
  fsmP_->addStateTransition('C', 'C', "Configure",  this, &gem::supervisor::tbutils::ThresholdScan::configureAction);
  fsmP_->addStateTransition('C', 'E', "Start",      this, &gem::supervisor::tbutils::ThresholdScan::startAction);
  fsmP_->addStateTransition('E', 'C', "Stop",       this, &gem::supervisor::tbutils::ThresholdScan::stopAction);
  fsmP_->addStateTransition('C', 'H', "Halt",       this, &gem::supervisor::tbutils::ThresholdScan::haltAction);
  fsmP_->addStateTransition('E', 'H', "Halt",       this, &gem::supervisor::tbutils::ThresholdScan::haltAction);
  fsmP_->addStateTransition('H', 'H', "Halt",       this, &gem::supervisor::tbutils::ThresholdScan::haltAction);
  fsmP_->addStateTransition('C', 'I', "Reset",      this, &gem::supervisor::tbutils::ThresholdScan::resetAction);
  fsmP_->addStateTransition('H', 'I', "Reset",      this, &gem::supervisor::tbutils::ThresholdScan::resetAction);

  // Define invalid transitions, too, so that they can be ignored, or else FSM will be unhappy when one is fired.
  fsmP_->addStateTransition('E', 'E', "Configure", this, &gem::supervisor::tbutils::ThresholdScan::noAction);
  fsmP_->addStateTransition('H', 'H', "Start"    , this, &gem::supervisor::tbutils::ThresholdScan::noAction);
  fsmP_->addStateTransition('E', 'E', "Start"    , this, &gem::supervisor::tbutils::ThresholdScan::noAction);
  fsmP_->addStateTransition('H', 'H', "Stop"     , this, &gem::supervisor::tbutils::ThresholdScan::noAction);
  fsmP_->addStateTransition('C', 'C', "Stop"     , this, &gem::supervisor::tbutils::ThresholdScan::noAction);


  fsmP_->setInitialState('I');
  fsmP_->reset();

  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:ThresholdScan","waiting");
  wl_->activate();

}

gem::supervisor::tbutils::ThresholdScan::~ThresholdScan()
  
{
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:ThresholdScan","waiting");
  //should we check to see if it's running and try to stop?
  wl_->cancel();
  wl_ = 0;
  
  for (int hi = 0; hi < 128; ++hi) {
    if (histos[hi])
      delete histos[hi];
    histos[hi] = 0;
  }

    if (outputCanvas)
      delete outputCanvas;
    outputCanvas = 0;

  //if (scanStream) {
  //  if (scanStream->is_open())
  //    scanStream->close();
  //  delete scanStream;
  //}
  //scanStream = 0;

  if (fsmP_)
    delete fsmP_;
  fsmP_ = 0;
  
}


void gem::supervisor::tbutils::ThresholdScan::actionPerformed(xdata::Event& event)
{
  // This is called after all default configuration values have been
  // loaded (from the XDAQ configuration file).

}

void gem::supervisor::tbutils::ThresholdScan::fireEvent(const std::string& name)
{
  toolbox::Event::Reference event((new toolbox::Event(name, this)));  
  fsmP_->fireEvent(event);
}

void gem::supervisor::tbutils::ThresholdScan::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
{
  //keep_refresh_ = false;
  
  LOG4CPLUS_INFO(getApplicationLogger(),"Current state is: [" << fsm.getStateName (fsm.getCurrentState()) << "]");
  std::string state_=fsm.getStateName (fsm.getCurrentState());
  
  LOG4CPLUS_INFO(getApplicationLogger(), "StateChanged: " << (std::string)state_);
  
}

void gem::supervisor::tbutils::ThresholdScan::transitionFailed(toolbox::Event::Reference event)
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
  
  LOG4CPLUS_ERROR(getApplicationLogger(), reason.str());
}



//Actions
bool gem::supervisor::tbutils::ThresholdScan::initialize(toolbox::task::WorkLoop* wl)
{
  fireEvent("Initialize");
  return false; //do once?
}

bool gem::supervisor::tbutils::ThresholdScan::configure(toolbox::task::WorkLoop* wl)
{
  fireEvent("Configure");
  return false; //do once?
}

bool gem::supervisor::tbutils::ThresholdScan::start(toolbox::task::WorkLoop* wl)
{
  fireEvent("Start");
  return false;
}

bool gem::supervisor::tbutils::ThresholdScan::stop(toolbox::task::WorkLoop* wl)
{
  fireEvent("Stop");
  return false; //do once?
}

bool gem::supervisor::tbutils::ThresholdScan::halt(toolbox::task::WorkLoop* wl)
{
  fireEvent("Halt");
  return false; //do once?
}

bool gem::supervisor::tbutils::ThresholdScan::reset(toolbox::task::WorkLoop* wl)
{
  fireEvent("Reset");
  return false; //do once?
}

bool gem::supervisor::tbutils::ThresholdScan::run(toolbox::task::WorkLoop* wl)
{

  wl_semaphore_.take();
  if (!is_running_) {
    //hw_semaphore_.take();
    //vfatDevice_->setRunMode(1);
    //hw_semaphore_.give();
    //if stop action has killed the run, take the final readout
    wl_semaphore_.give();
    wl_->submit(readSig_);
    return false;
  }
  
  if ((uint64_t)(confParams_.bag.triggersSeen) < (uint64_t)(confParams_.bag.nTriggers)) {
    hw_semaphore_.take();
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    LOG4CPLUS_INFO(getApplicationLogger(),"Not enough triggers, run mode 0x" << std::hex << (unsigned)vfatDevice_->getRunMode() << std::dec);
    
    vfatDevice_->setDeviceBaseNode("GLIB");
    uint32_t bufferDepth = vfatDevice_->readReg("LINK1.TRK_FIFO.DEPTH");
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    hw_semaphore_.give();
    if (bufferDepth < 10) {
      //update triggersSeen
      sleep(1);
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS");
      confParams_.bag.triggersSeen = vfatDevice_->readReg("L1A.Total");
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      LOG4CPLUS_INFO(getApplicationLogger(),"Not enough entries in the buffer, run mode 0x" << std::hex << (unsigned)vfatDevice_->getRunMode() << std::dec);
      hw_semaphore_.give();
      wl_semaphore_.give();
      return true;
    }
    else {
      //maybe don't do the readout as a workloop?
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      LOG4CPLUS_INFO(getApplicationLogger(),"Buffer full, reading out, run mode 0x" << std::hex << (unsigned)vfatDevice_->getRunMode() << std::dec);
      hw_semaphore_.give();
      wl_semaphore_.give();
      wl_->submit(readSig_);
      return true;
    }
    //wl_semaphore_.give();
  }
  else {
    hw_semaphore_.take();
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    LOG4CPLUS_INFO(getApplicationLogger(),"Enough triggers, reading out, run mode 0x" << std::hex << (unsigned)vfatDevice_->getRunMode() << std::dec);
    hw_semaphore_.give();
    wl_semaphore_.give();
    wl_->submit(readSig_);
    wl_semaphore_.take();
    hw_semaphore_.take();
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    vfatDevice_->setRunMode(0);
    vfatDevice_->setDeviceBaseNode("GLIB.LINK1");
    vfatDevice_->writeReg("TRK_FIFO.FLUSH",0x1);
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    hw_semaphore_.give();
    
    if ((unsigned)confParams_.bag.deviceVT1 == (unsigned)0x0) {
      //wl_semaphore_.take();
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      LOG4CPLUS_INFO(getApplicationLogger(),"VT1 is 0, reading out, run mode 0x" << std::hex << (unsigned)vfatDevice_->getRunMode() << std::dec);
      hw_semaphore_.give();
      wl_semaphore_.give();
      wl_->submit(stopSig_);
      return false;
    }
    else if ((confParams_.bag.deviceVT2-confParams_.bag.deviceVT1) <= confParams_.bag.maxThresh)  {
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      LOG4CPLUS_INFO(getApplicationLogger(),"VT2-VT1 is less than the max threshold, run mode 0x" << std::hex << (unsigned)vfatDevice_->getRunMode() << std::dec);
      hw_semaphore_.give();
      //how to ensure that the VT1 never goes negative
      hw_semaphore_.take();
      if (confParams_.bag.deviceVT1 > confParams_.bag.stepSize)
	vfatDevice_->setVThreshold1(confParams_.bag.deviceVT1 - confParams_.bag.stepSize);
      else
	vfatDevice_->setVThreshold1(0);

      confParams_.bag.deviceVT1    = vfatDevice_->getVThreshold1();
      confParams_.bag.triggersSeen = 0;
      vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS.RESETS");
      vfatDevice_->writeReg("L1A.Total",0x1);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      vfatDevice_->setRunMode(1);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      LOG4CPLUS_INFO(getApplicationLogger(),"Resubmitting the run workloop, run mode 0x" << std::hex << (unsigned)vfatDevice_->getRunMode() << std::dec);
      hw_semaphore_.give();
      wl_semaphore_.give();	
      return true;	
    }
    else {
      //wl_semaphore_.take();
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      LOG4CPLUS_INFO(getApplicationLogger(),"reached max threshold, stopping out, run mode 0x" << std::hex << (unsigned)vfatDevice_->getRunMode() << std::dec);
      hw_semaphore_.give();
      wl_semaphore_.give();
      wl_->submit(stopSig_);
      return false;
    }
  }
  /*
  else {
    wl_semaphore_.give();
    wl_->submit(readSig_);
    wl_semaphore_.take();
    wl_semaphore_.give();
    wl_->submit(stopSig_);
    return false;
  }
  */
}

//might be better done not as a workloop?
bool gem::supervisor::tbutils::ThresholdScan::readFIFO(toolbox::task::WorkLoop* wl)
{
  wl_semaphore_.take();
  hw_semaphore_.take();

  ChannelData ch;
  VFATEvent ev;
  int event=0;

  std::string tmpFileName = confParams_.bag.outFileName.toString();

  //maybe not even necessary?
  //vfatDevice_->setRunMode(0);
  sleep(5);
  //read the fifo (x3 times fifo depth), add headers, write to disk, save disk
  boost::format linkForm("LINK%d");
  //should all links have the same fifo depth? if not is this an error?

  uint32_t fifoDepth[3];
  //set proper base address
  vfatDevice_->setDeviceBaseNode("GLIB");
  fifoDepth[0] = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");
  fifoDepth[1] = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");
  fifoDepth[2] = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");
  
  //check that the fifos are all the same size?
  int bufferDepth = 0;
  if (fifoDepth[0] != fifoDepth[1] || 
      fifoDepth[0] != fifoDepth[2] || 
      fifoDepth[1] != fifoDepth[2]) {
    LOG4CPLUS_INFO(getApplicationLogger(), "tracking data fifos had different depths:: "
		   << fifoDepth[0] << ","
		   << fifoDepth[0] << ","
		   << fifoDepth[0]);
    //use the minimum
    bufferDepth = std::min(fifoDepth[0],std::min(fifoDepth[1],fifoDepth[2]));
  }
  //right now only have FIFO on LINK1
  bufferDepth = fifoDepth[1];
  
  //grab events from the fifo
  bool isFirst = true;
  uint32_t bxNum, bxExp;

  while (bufferDepth) {
    //for ( int fiEvt = 0; fiEvt < bufferDepth; ++fiEvt) {
    //create the event header (run number, event number, anything else?
    
    std::vector<uint32_t> data;
    //readInWords(data);
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.TRK_DATA.COL1");
    
    LOG4CPLUS_INFO(getApplicationLogger(),"Trying to read register "<<vfatDevice_->getDeviceBaseNode()<<".DATA_RDY");
    if (vfatDevice_->readReg("DATA_RDY")) {
      LOG4CPLUS_INFO(getApplicationLogger(),"Trying to read the block at "<<vfatDevice_->getDeviceBaseNode()<<".DATA");
      //data = vfatDevice_->readBlock("OptoHybrid.GEB.TRK_DATA.COL1.DATA");
      //data = vfatDevice_->readBlock("OptoHybrid.GEB.TRK_DATA.COL1.DATA",7);
      for (int word = 0; word < 7; ++word) {
	std::stringstream ss9;
	ss9 << "DATA." << word;
	data.push_back(vfatDevice_->readReg(ss9.str()));
      }
    }

    uint32_t TrigReg, bxNumTr;
    uint8_t SBit;

    //set proper base address
    vfatDevice_->setDeviceBaseNode("GLIB");
    TrigReg = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRK_DATA.DATA");
    bxNumTr = TrigReg >> 6;
    SBit = TrigReg & 0x0000003F;

    //make sure we are aligned

    //if (!checkHeaders(data)) 
    if (!( 
	  (((data.at(5)&0xF0000000)>>28)==0xa) && 
	  (((data.at(5)&0x0000F000)>>12)==0xc) && 
	  (((data.at(4)&0xF0000000)>>28)==0xe)
	   )) {
      //--bufferDepth; 
      LOG4CPLUS_INFO(getApplicationLogger(),"VFAT headers do not match expectation");
      vfatDevice_->setDeviceBaseNode("GLIB");
      bufferDepth = vfatDevice_->readReg("LINK1.TRK_FIFO.DEPTH");
      continue;
    }
    
    bxNum = data.at(6);
    
    uint16_t bcn, evn, crc, chipid;
    uint64_t msData, lsData;
    uint8_t  flags;
    
    if (isFirst)
      bxExp = bxNum;
    
    if (bxNum == bxExp)
      isFirst = false;
    
    bxNum  = data.at(6);
    bcn    = (0x0fff0000 & data.at(5)) >> 16;
    evn    = (0x00000ff0 & data.at(5)) >> 4;
    chipid = (0x0fff0000 & data.at(4)) >> 16;
    flags  = (0x0000000f & data.at(5));

    uint64_t data1  = ((0x0000ffff & data.at(4)) << 16) | ((0xffff0000 & data.at(3)) >> 16);
    uint64_t data2  = ((0x0000ffff & data.at(3)) << 16) | ((0xffff0000 & data.at(2)) >> 16);
    uint64_t data3  = ((0x0000ffff & data.at(2)) << 16) | ((0xffff0000 & data.at(1)) >> 16);
    uint64_t data4  = ((0x0000ffff & data.at(1)) << 16) | ((0xffff0000 & data.at(0)) >> 16);

    lsData = (data3 << 32) | (data4);
    msData = (data1 << 32) | (data2);

    crc    = 0x0000ffff & data.at(0);

    ch.lsdata = lsData;
    ch.msdata = msData;

    ev.BC = ((data.at(5)&0xF0000000)>>28) << 12; // 1010
    ev.BC = (ev.BC | bcn);
    ev.EC = ((data.at(5)&0x0000F000)>>12) << 12; // 1100
    ev.EC = (ev.EC | evn) << 4;
    ev.EC = (ev.EC | flags);
    ev.bxExp = bxExp;
    ev.bxNum = bxNum << 6;
    ev.bxNum = (ev.bxNum | SBit);
    ev.ChipID = ((data.at(4)&0xF0000000)>>28) << 12; // 1110
    ev.ChipID = (ev.ChipID | chipid);
    ev.crc = crc;

    /*
    gemev.header = 0x0;
    gemev.vfats.push_back (ev);
    gemev.trailer = 0x0;
    */

    keepEvent(tmpFileName, event, ev, ch);

    LOG4CPLUS_INFO(getApplicationLogger(),
		   "Received tracking data word:" << std::endl
		   << "bxn     :: 0x" << std::setfill('0') << std::setw(4) << std::hex << bxNum  << std::dec << std::endl
		   << "bcn     :: 0x" << std::setfill('0') << std::setw(4) << std::hex << bcn    << std::dec << std::endl
		   << "evn     :: 0x" << std::setfill('0') << std::setw(4) << std::hex << evn    << std::dec << std::endl
		   << "flags   :: 0x" << std::setfill('0') << std::setw(2) << std::hex << (unsigned)flags  << std::dec << std::endl
		   << "chipid  :: 0x" << std::setfill('0') << std::setw(4) << std::hex << chipid << std::dec << std::endl
		   << "<127:0> :: 0x" << std::setfill('0') << std::setw(8) << std::hex << msData << 
		   std::dec << std::setfill('0') << std::setw(8) << std::hex << lsData << std::dec << std::endl
		   << "<127:64>:: 0x" << std::setfill('0') << std::setw(8) << std::hex << msData << std::dec << std::endl
		   << "<63:0>  :: 0x" << std::setfill('0') << std::setw(8) << std::hex << lsData << std::dec << std::endl
		   << "crc     :: 0x" << std::setfill('0') << std::setw(4) << std::hex << crc    << std::dec << std::endl
		   );
    //while (bxNum == bxExp) {
    
    for (int chan = 0; chan < 128; ++chan) {
      if (chan < 64)
	histos[chan]->Fill(confParams_.bag.deviceVT2-confParams_.bag.deviceVT1,((lsData>>chan))&0x1);
      else
	histos[chan]->Fill(confParams_.bag.deviceVT2-confParams_.bag.deviceVT1,((msData>>(chan-64)))&0x1);
    }
    //createCommonEventHeader();
    //loop over the links
    //for (int link = 0; link < 3; ++link) {
    //  fifoDepth[link] = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");
    //  std::stringstream regname;
    //  regname << "tracking_data.COL" << link << ".DATA_RDY";
    //  vfatDevice_->readReg(regname.str());
    //  //block read the 6 words (are the words from each vfat read in sequentially?
    //  
    //  //how do we put all the data from a single trigger into a single event?
    //  //block read
    //  //vfatPacket = vfatDevice_->readReg("tracking_data.COL" << link << ".DATA");
    //}
    //createCommonEventTrailer();
    vfatDevice_->setDeviceBaseNode("GLIB");
    bufferDepth = vfatDevice_->readReg("LINK1.TRK_FIFO.DEPTH");
  }
  //header should have event number...
  //return to running
  //}

  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  hw_semaphore_.give();

  for (int chan = 0; chan < 128; ++chan) {
    std::string imgRoot = "${XDAQ_DOCUMENT_ROOT}/gemdaq/gemsupervisor/html/images/tbutils/tscan/";
    std::stringstream ss;
    ss << "chanthresh" << chan << ".png";
    std::string imgName = ss.str();
    outputCanvas->cd();
    histos[chan]->Draw("histfill");
    outputCanvas->Update();
    outputCanvas->SaveAs(TString(imgRoot+imgName));
  }

  wl_semaphore_.give();
  return false;
}

// SOAP interface
xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onInitialize(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(initSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onConfigure(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(confSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onStart(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(startSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onStop(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(stopSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onHalt(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(haltSig_);

  return message;
}

xoap::MessageReference gem::supervisor::tbutils::ThresholdScan::onReset(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(resetSig_);

  return message;
}

void gem::supervisor::tbutils::ThresholdScan::selectVFAT(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    std::string isDisabled = "";
    if (is_running_ || is_configured_ || is_initialized_)
      isDisabled = "disabled";
    
    LOG4CPLUS_INFO(getApplicationLogger(),"selected device is: "<<confParams_.bag.deviceName.toString());
    *out << cgicc::span() << std::endl
	 << "<table>"     << std::endl
	 << "<tr>"   << std::endl
	 << "<td>" << "Selected VFAT:" << "</td>" << std::endl
	 << "<td>" << "ChipID:"        << "</td>" << std::endl
	 << "</tr>"     << std::endl

	 << "<tr>" << std::endl
	 << "<td>" << std::endl
	 << cgicc::select().set("id","VFATDevice").set("name","VFATDevice")     << std::endl
	 << ((confParams_.bag.deviceName.toString().compare("VFAT8")) == 0 ?
	     (cgicc::option("VFAT8").set(isDisabled).set("value","VFAT8").set("selected")) :
	     (cgicc::option("VFAT8").set(isDisabled).set("value","VFAT8"))) << std::endl

	 << ((confParams_.bag.deviceName.toString().compare("VFAT9")) == 0 ?
	     (cgicc::option("VFAT9").set(isDisabled).set("value","VFAT9").set("selected")) :
	     (cgicc::option("VFAT9").set(isDisabled).set("value","VFAT9"))) << std::endl

	 << ((confParams_.bag.deviceName.toString().compare("VFAT10")) == 0 ?
	     (cgicc::option("VFAT10").set(isDisabled).set("value","VFAT10").set("selected")) :
	     (cgicc::option("VFAT10").set(isDisabled).set("value","VFAT10"))) << std::endl

	 << ((confParams_.bag.deviceName.toString().compare("VFAT11")) == 0 ?
	     (cgicc::option("VFAT11").set(isDisabled).set("value","VFAT11").set("selected")) :
	     (cgicc::option("VFAT11").set(isDisabled).set("value","VFAT11"))) << std::endl

	 << ((confParams_.bag.deviceName.toString().compare("VFAT12")) == 0 ?
	     (cgicc::option("VFAT12").set(isDisabled).set("value","VFAT12").set("selected")) :
	     (cgicc::option("VFAT12").set(isDisabled).set("value","VFAT12"))) << std::endl

	 << ((confParams_.bag.deviceName.toString().compare("VFAT13")) == 0 ?
	     (cgicc::option("VFAT13").set(isDisabled).set("value","VFAT13").set("selected")) :
	     (cgicc::option("VFAT13").set(isDisabled).set("value","VFAT13"))) << std::endl
	 << cgicc::select()<< std::endl
	 << "</td>" << std::endl
      
	 << "<td>" << std::endl
	 << cgicc::input().set("type","text").set("id","ChipID")
                          .set("name","ChipID").set("readonly")
                          .set("value",boost::str(boost::format("0x%04x")%(confParams_.bag.deviceChipID)))
	 << std::endl
	 << "</td>"    << std::endl
	 << "</tr>"    << std::endl
	 << "</table>" << std::endl
	 << cgicc::span()  << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying VFATS(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying VFATS(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}


void gem::supervisor::tbutils::ThresholdScan::scanParameters(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    *out << cgicc::span()   << std::endl
	 << cgicc::label("Latency").set("for","Latency") << std::endl
	 << cgicc::input().set("id","Latency").set("name","Latency")
                          .set("type","number").set("min","0").set("max","255")
                          .set("value",boost::str(boost::format("%d")%static_cast<unsigned>(confParams_.bag.latency)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("MinThreshold").set("for","MinThreshold") << std::endl
	 << cgicc::input().set("id","MinThreshold").set("name","MinThreshold")
                          .set("type","number").set("min","-255").set("max","255")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.minThresh)))
	 << std::endl

	 << cgicc::label("MaxThreshold").set("for","MaxThreshold") << std::endl
	 << cgicc::input().set("id","MaxThreshold").set("name","MaxThreshold")
                          .set("type","number").set("min","-255").set("max","255")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.maxThresh)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("VStep").set("for","VStep") << std::endl
	 << cgicc::input().set("id","VStep").set("name","VStep")
                          .set("type","number").set("min","1").set("max","255")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.stepSize)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("VT1").set("for","VT1") << std::endl
	 << cgicc::input().set("id","VT1").set("name","VT1").set("readonly")
                          .set("value",boost::str(boost::format("%d")%static_cast<unsigned>(confParams_.bag.deviceVT1)))
	 << std::endl

	 << cgicc::label("VT2").set("for","VT2") << std::endl
	 << cgicc::input().set("id","VT2").set("name","VT2").set("readonly")
                          .set("value",boost::str(boost::format("%d")%static_cast<unsigned>(confParams_.bag.deviceVT2)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("NTrigsStep").set("for","NTrigsStep") << std::endl
	 << cgicc::input().set("id","NTrigsStep").set("name","NTrigsStep")
                          .set("type","number").set("min","0")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.nTriggers)))
	 << cgicc::br() << std::endl
	 << cgicc::label("NTrigsSeen").set("for","NTrigsSeen") << std::endl
	 << cgicc::input().set("id","NTrigsSeen").set("name","NTrigsSeen")
                          .set("type","number").set("min","0").set("readonly")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.triggersSeen)))
	 << cgicc::br() << std::endl

	 << cgicc::span()   << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying VFATS(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying VFATS(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

void gem::supervisor::tbutils::ThresholdScan::showCounterLayout(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    if (is_initialized_ && vfatDevice_) {

      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/ResetCounters") << std::endl;
      
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS");
      //*out << cgicc::div().set("class","xdaq-tab").set("title","Counters")   << std::endl
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
	   << cgicc::td()    << "External"    << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("L1A.External") << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstL1AExt")
	                                      .set("name","RstL1AExt")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Internal"    << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("L1A.Internal") << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstL1AInt")
	                                      .set("name","RstL1AInt")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Delayed"     << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("L1A.Delayed" ) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstL1ADel")
	                                      .set("name","RstL1ADel")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Total"       << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("L1A.Total"   ) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstL1ATot")
	                                      .set("name","RstL1ATot")
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
	   << cgicc::td()    << "External"  << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("CalPulse.External") << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstCalPulseExt")
	                                      .set("name","RstCalPulseExt")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Internal"  << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("CalPulse.Internal") << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstCalPulseInt")
	                                      .set("name","RstCalPulseInt")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Total"     << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("CalPulse.Total"   ) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstCalPulseTot")
	                                      .set("name","RstCalPulseTot")
	   << cgicc::td() << std::endl
	   << "</tr>"     << std::endl
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
	   << cgicc::td()    << vfatDevice_->readReg("Resync" ) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstResync")
	                                      .set("name","RstResync")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "BC0"       << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg("BC0"    ) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstBC0")
	                                      .set("name","RstBC0")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()  << "BXCount"   << cgicc::td() << std::endl
	   << cgicc::td()  << vfatDevice_->readReg("BXCount") << cgicc::td() << std::endl
	   << cgicc::td()  << "" << cgicc::td() << std::endl
	   << "</tr>"      << std::endl
	   << "</tbody>"   << std::endl
	   << "</table>"   << std::endl
	   << "</td>"      << std::endl
	   << "</tr>"      << std::endl
	   << cgicc::tbody() << std::endl
	   << "</table>"   << std::endl;

      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();

      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Reset counters.")
	.set("value", "ResetCounters") << std::endl;

      *out << cgicc::form() << std::endl;
      
    }
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying showCounterLayout(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying showCounterLayout(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  hw_semaphore_.take();
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  hw_semaphore_.give();
} //end showCounterLayout


void gem::supervisor::tbutils::ThresholdScan::showBufferLayout(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    if (is_initialized_ && vfatDevice_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/FastCommands") << std::endl;
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("GLIB.LINK1");
      *out << cgicc::label("FIFOOcc").set("for","FIFOOcc") << std::endl
	   << cgicc::input().set("id","FIFOOcc").set("name","FIFOOcc").set("type","text")
	                    .set("value",boost::str( boost::format("%d")%(
									 vfatDevice_->readReg("TRK_FIFO.DEPTH")
									 ))) << std::endl;

      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();

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
  }
  
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying showBufferLayout(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying showBufferLayout(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  hw_semaphore_.take();
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  hw_semaphore_.give();
} //end showBufferLayout


void gem::supervisor::tbutils::ThresholdScan::fastCommandLayout(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    if (is_initialized_ && vfatDevice_) {

      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/FastCommands") << std::endl;
      
      //hw_semaphore_.take();
      //vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS");
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
                            .set("value","1")
	   << cgicc::td()  << std::endl;

      *out << cgicc::tr()    << std::endl
	   << cgicc::tbody() << std::endl
	   << cgicc::table() << std::endl;
	
	//trigger setup
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
                            .set("id","GLIBsrc").set("value","GLIB")
	                    .set((unsigned)confParams_.bag.triggerSource == (unsigned)0x0 ? "checked" : "")

	   << cgicc::label("GLIB").set("for","GLIBSrc") << std::endl
	   << cgicc::br()
	   << cgicc::input().set("type","radio").set("name","trgSrc")
	                    .set("id","ExtSrc").set("value","Ext")
                            .set((unsigned)confParams_.bag.triggerSource == (unsigned)0x1 ? "checked" : "")
	   << cgicc::label("Ext (LEMO)").set("for","ExtSrc") << std::endl
	   << cgicc::br()
	   << cgicc::input().set("type","radio").set("name","trgSrc").set("checked")
                            .set("id","BothSrc").set("value","Both")
                            .set((unsigned)confParams_.bag.triggerSource == (unsigned)0x2 ? "checked" : "")
	   << cgicc::label("Both").set("for","BothSrc") << std::endl
	   << cgicc::br()
	   << cgicc::input().set("class","button").set("type","submit")
	                    .set("value","SetTriggerSource").set("name","SendFastCommand")
	   << cgicc::td() << std::endl;
      
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
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying fastCommandLayout(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying fastCommandLayout(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  hw_semaphore_.take();
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  hw_semaphore_.give();
}


void gem::supervisor::tbutils::ThresholdScan::displayHistograms(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    *out << cgicc::form().set("method","POST").set("action", "") << std::endl;
    
    *out << cgicc::table().set("class","xdaq-table") << std::endl
	 << cgicc::thead() << std::endl
	 << cgicc::tr()    << std::endl //open
	 << cgicc::th()    << "Select Channel" << cgicc::th() << std::endl
	 << cgicc::th()    << "Histogram"      << cgicc::th() << std::endl
	 << cgicc::tr()    << std::endl //close
	 << cgicc::thead() << std::endl 
      
	 << cgicc::tbody() << std::endl;
    
    *out << cgicc::tr()  << std::endl;
    *out << cgicc::td()
	 << cgicc::label("Channel").set("for","ChannelHist") << std::endl
	 << cgicc::input().set("id","ChannelHist").set("name","ChannelHist")
                          .set("type","number").set("min","0").set("max","127")
                          .set("value","1") << std::endl
	 << cgicc::br() << std::endl;
    *out << cgicc::input().set("class","button").set("type","button")
                          .set("value","SelectChannel").set("name","DisplayHistogram")
                          .set("onClick","changeImage(this.form)");
    *out << cgicc::td() << std::endl;

    *out << cgicc::td()  << std::endl
	 << cgicc::img().set("src","/gemdaq/gemsupervisor/html/images/tbutils/tscan/chanthresh1.png")
                        .set("id","vfatChannelHisto")
	 << cgicc::td()  << std::endl;
    *out << cgicc::tr()    << std::endl
	 << cgicc::tbody() << std::endl
	 << cgicc::table() << std::endl;
    *out << cgicc::form() << cgicc::br() << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying displayHistograms(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying displayHistograms(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

void gem::supervisor::tbutils::ThresholdScan::redirect(xgi::Input *in, xgi::Output* out) {
  //change the status to halting and make sure the page displays this information
  std::string redURL = "/" + getApplicationDescriptor()->getURN() + "/Default";
  //cgicc::Cgicc cgi_in(in);
  //cgicc::Cgicc cgi_out(out);
  //cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
  //cgicc::HTTPRedirectHeader &head = out->getHTTPRedirectHeader();
  //head.addHeader("<meta http-equiv=\"refresh\" content=\"0;" + redURL + "\">");
  *out << "<meta http-equiv=\"refresh\" content=\"0;" << redURL << "\">" << std::endl;
  this->webDefault(in,out);
}

// HyperDAQ interface
void gem::supervisor::tbutils::ThresholdScan::webDefault(xgi::Input *in, xgi::Output *out)
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

      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Initialize hardware acces.")
	.set("value", "Initialize") << std::endl;

      *out << cgicc::form() << std::endl;
    }
    
    else if (!is_configured_) {
      //this will allow the parameters to be set to the chip and scan routine

      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Configure") << std::endl;
      
      selectVFAT(out);
      scanParameters(out);
      
      //adding aysen's xml parser
      //std::string setConfFile = toolbox::toString("/%s/setConfFile",getApplicationDescriptor()->getURN().c_str());
      //*out << cgicc::form().set("method","POST").set("action",setConfFile) << std::endl ;
      
      *out << cgicc::input().set("type","text").set("name","xmlFilename").set("size","80")
 	                    .set("ENCTYPE","multipart/form-data").set("readonly")
                            .set("value",confParams_.bag.settingsFile.toString()) << std::endl;
      //*out << cgicc::input().set("type","submit").set("value","Set configuration file") << std::endl ;
      //*out << cgicc::form() << std::endl ;
      
      *out << cgicc::br() << std::endl;
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Configure threshold scan.")
	.set("value", "Configure") << std::endl;
      *out << cgicc::form()        << std::endl;
    }
    
    else if (!is_running_) {
      //hardware is initialized and configured, we can start the run
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Start") << std::endl;
      
      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Start threshold scan.")
	.set("value", "Start") << std::endl;
      *out << cgicc::form()    << std::endl;
    }
    
    else if (is_running_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Stop") << std::endl;
      
      selectVFAT(out);
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
    
    *out << "</div>" << std::endl;
    
    *out << "<div class=\"xdaq-tab\" title=\"Counters\">"  << std::endl;
    if (is_initialized_)
      showCounterLayout(out);
    *out << "</div>" << std::endl;

    *out << "<div class=\"xdaq-tab\" title=\"Fast Commands/Trigger Setup\">"  << std::endl;
    if (is_initialized_)
      fastCommandLayout(out);
    *out << "</div>" << std::endl;

    //place new div class=xdaq-tab here to hold the histograms
    /*
      display a single histogram and have a form that selects which channel you want to display
      use the file name of the histogram that is saved in readFIFO
    */
    *out << "<div class=\"xdaq-tab\" title=\"Channel histograms\">"  << std::endl;
    displayHistograms(out);
    
    *out << "</div>" << std::endl;
    *out << "</div>" << std::endl;
    //</div> //close the new div xdaq-tab

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
	 //<< "<tr>"    << std::endl
	 //<< "<td>" << "Status:"   << "</td>"
	 //<< "<td>" << "Value:"    << "</td>"
	 //<< "</tr>" << std::endl
      
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
	 //<< "<tr>"    << std::endl
	 //<< "<td>" << "Status:"   << "</td>"
	 //<< "<td>" << "Value:"    << "</td>"
	 //<< "</tr>" << std::endl
      
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
    
    if (is_initialized_ && vfatDevice_) {
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("TEST");
      *out << "<tr>" << std::endl
	   << "<td>" << "GLIB" << "</td>"
	   << "<td>" << vfatDevice_->readReg("GLIB") << "</td>"
	   << "</tr>"   << std::endl
	
	   << "<tr>" << std::endl
	   << "<td>" << "OptoHybrid" << "</td>"
	   << "<td>" << vfatDevice_->readReg("OptoHybrid") << "</td>"
	   << "</tr>"       << std::endl
	
	   << "<tr>" << std::endl
	   << "<td>" << "VFATs" << "</td>"
	   << "<td>" << vfatDevice_->readReg("VFATs") << "</td>"
	   << "</tr>"      << std::endl;
      
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }
    
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
    *out << cgicc::script().set("type","text/javascript")
                           .set("src","/gemdaq/gemsupervisor/html/scripts/tbutils/changeImage.js")
	 << cgicc::script() << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying ThresholdScan control panel(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying ThresholdScan control panel(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}


void gem::supervisor::tbutils::ThresholdScan::webInitialize(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  
  try {
    cgicc::Cgicc cgi(in);
    std::vector<cgicc::FormEntry> vfat2FormEntries = cgi.getElements();
    LOG4CPLUS_INFO(getApplicationLogger(), "debugging form entries");
    std::vector<cgicc::FormEntry>::const_iterator myiter = vfat2FormEntries.begin();
    
    //for (; myiter != vfat2FormEntries.end(); ++myiter ) {
    //  LOG4CPLUS_INFO(getApplicationLogger(), "form entry::" myiter->getName());
    //}

    
    std::string tmpDeviceName = "";
    cgicc::const_form_iterator name = cgi.getElement("VFATDevice");
    if (name != cgi.getElements().end())
      tmpDeviceName = name->getValue();

    //std::string tmpDeviceName = cgi["VFATDevice"]->getValue();
    LOG4CPLUS_INFO(getApplicationLogger(), "deviceName_::"             << confParams_.bag.deviceName.toString());
    LOG4CPLUS_INFO(getApplicationLogger(), "setting deviceName_ to ::" << tmpDeviceName);
    confParams_.bag.deviceName = tmpDeviceName;
    LOG4CPLUS_INFO(getApplicationLogger(), "deviceName_::"             << confParams_.bag.deviceName.toString());
    
    int tmpDeviceNum = -1;
    tmpDeviceName.erase(0,4);
    tmpDeviceNum = atoi(tmpDeviceName.c_str());
    tmpDeviceNum -= 8;
    LOG4CPLUS_INFO(getApplicationLogger(), "deviceNum_::"             << confParams_.bag.deviceNum.toString());
    LOG4CPLUS_INFO(getApplicationLogger(), "setting deviceNum_ to ::" << tmpDeviceNum);
    confParams_.bag.deviceNum = tmpDeviceNum;
    LOG4CPLUS_INFO(getApplicationLogger(), "deviceNum_::"             << confParams_.bag.deviceNum.toString());
    
    //change the status to initializing and make sure the page displays this information
  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  
  wl_->submit(initSig_);

  redirect(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webConfigure(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {

  try {
    cgicc::Cgicc cgi(in);
    
    //aysen's xml parser
    confParams_.bag.settingsFile = cgi.getElement("xmlFilename")->getValue();
    
    cgicc::const_form_iterator element = cgi.getElement("Latency");
    if (element != cgi.getElements().end())
      confParams_.bag.latency   = element->getIntegerValue();
    element = cgi.getElement("Latency");
    if (element != cgi.getElements().end())
      confParams_.bag.nTriggers = element->getIntegerValue();

    element = cgi.getElement("MinThreshold");
    if (element != cgi.getElements().end())
      confParams_.bag.minThresh = element->getIntegerValue();
    
    element = cgi.getElement("MaxThreshold");
    if (element != cgi.getElements().end())
      confParams_.bag.maxThresh = element->getIntegerValue();

    element = cgi.getElement("VStep");
    if (element != cgi.getElements().end())
      confParams_.bag.stepSize  = element->getIntegerValue();
        
    element = cgi.getElement("NTrigsStep");
    if (element != cgi.getElements().end())
      confParams_.bag.nTriggers  = element->getIntegerValue();
  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  
  wl_->submit(confSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webStart(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(startSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webStop(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(stopSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webHalt(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(haltSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webReset(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(resetSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webResetCounters(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  
  try {
    cgicc::Cgicc cgi(in);
    std::vector<cgicc::FormEntry> resetCounters = cgi.getElements();
    LOG4CPLUS_INFO(getApplicationLogger(), "resetting counters entries");
    
    hw_semaphore_.take();
    vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS.RESETS");

    if (cgi.queryCheckbox("RstL1AExt") ) 
      vfatDevice_->writeReg("L1A.External",0x1);
    if (cgi.queryCheckbox("RstL1AInt") ) 
      vfatDevice_->writeReg("L1A.Internal",0x1);
    if (cgi.queryCheckbox("RstL1ADel") ) 
      vfatDevice_->writeReg("L1A.Delayed",0x1);
    if (cgi.queryCheckbox("RstL1ATot") ) 
      vfatDevice_->writeReg("L1A.Total",0x1);

    if (cgi.queryCheckbox("RstCalPulseExt") ) 
      vfatDevice_->writeReg("CalPulse.External",0x1);
    if (cgi.queryCheckbox("RstCalPulseInt") ) 
      vfatDevice_->writeReg("CalPulse.Internal",0x1);
    if (cgi.queryCheckbox("RstCalPulseTot") ) 
      vfatDevice_->writeReg("CalPulse.Total",0x1);

    if (cgi.queryCheckbox("RstResync") ) 
      vfatDevice_->writeReg("Resync",0x1);
    if (cgi.queryCheckbox("RstBC0") ) 
      vfatDevice_->writeReg("BC0",0x1);

    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    hw_semaphore_.give();
  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }

  hw_semaphore_.take();
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  hw_semaphore_.give();
  redirect(in,out);
}


void gem::supervisor::tbutils::ThresholdScan::webSendFastCommands(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  
  try {
    cgicc::Cgicc cgi(in);
    std::vector<cgicc::FormEntry> resetCounters = cgi.getElements();
    LOG4CPLUS_INFO(getApplicationLogger(), "resetting counters entries");
    
    std::string fastCommand = cgi["SendFastCommand"]->getValue();
    
    if (strcmp(fastCommand.c_str(),"FlushFIFO") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"FlushFIFO button pressed");
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("GLIB.LINK1");
      vfatDevice_->writeReg("TRK_FIFO.FLUSH",0x1);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }

    if (strcmp(fastCommand.c_str(),"SendTestPackets") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"SendTestPackets button pressed");
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      if (!is_running_) 
	vfatDevice_->setRunMode(0x1);
      vfatDevice_->sendTestPattern(0x1);
      sleep(1);
      vfatDevice_->sendTestPattern(0x0);
      if (!is_running_) 
	vfatDevice_->setRunMode(0x0);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send L1A+CalPulse") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"Send L1A+CalPulse button pressed");
      cgicc::const_form_iterator element = cgi.getElement("CalPulseDelay");
      uint8_t delay;
      if (element != cgi.getElements().end())
	delay = element->getIntegerValue();
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
      for (unsigned int com = 0; com < 15; ++com)
	vfatDevice_->writeReg("Send.L1ACalPulse",delay);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send L1A") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"Send L1A button pressed");
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
      vfatDevice_->writeReg("Send.L1A",0x1);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send CalPulse") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"Send CalPulse button pressed");
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
      vfatDevice_->writeReg("Send.CalPulse",0x1);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send Resync") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"Send Resync button pressed");
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
      vfatDevice_->writeReg("Send.Resync",0x1);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send BC0") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"Send BC0 button pressed");
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
      vfatDevice_->writeReg("Send.BC0",0x1);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"SetTriggerSource") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"SetTriggerSource button pressed");
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.TRIGGER");
      
      cgicc::form_iterator fi = cgi.getElement("trgSrc");
      if( !fi->isEmpty() && fi != (*cgi).end()) {  
	if (strcmp((**fi).c_str(),"GLIB") == 0) {
	  confParams_.bag.triggerSource = 0x0;
	  vfatDevice_->writeReg("SOURCE",0x0);
	}
	else if (strcmp((**fi).c_str(),"Ext") == 0) {
	  confParams_.bag.triggerSource = 0x1;
	  vfatDevice_->writeReg("SOURCE",0x1);
	}
	else if (strcmp((**fi).c_str(),"Both") == 0) {
	  confParams_.bag.triggerSource = 0x2;
	  vfatDevice_->writeReg("SOURCE",0x2);
	}
      }
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }
    
    else if (strcmp(fastCommand.c_str(),"SBitSelect") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"SBitSelect button pressed");
      uint32_t value = cgi["SBitSelect"]->getIntegerValue();
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.TRIGGER");
      vfatDevice_->writeReg("TDC_SBits",value);
      vfatDevice_->setDeviceBaseNode("GLIB");
      vfatDevice_->writeReg("TDC_SBits",value);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }
    
    hw_semaphore_.take();
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    hw_semaphore_.give();
  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }

  hw_semaphore_.take();
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  hw_semaphore_.give();
  redirect(in,out);
}


// State transitions
//is initialize different than halt? they come from different positions but put the software/hardware in the same state 'halted'
void gem::supervisor::tbutils::ThresholdScan::initializeAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  //Need to know which device to connnect to here...
  //dropdown list from the web interface?
  //deviceName_ = "CMS_hybrid_J44";
  //here the connection to the device should be made
  setLogLevelTo(uhal::Debug());  // Set uHAL logging level Debug (most) to Error (least)
  hw_semaphore_.take();
  vfatDevice_ = new gem::hw::vfat::HwVFAT2(this, confParams_.bag.deviceName.toString());
  
  //vfatDevice_->setAddressTableFileName("allregsnonfram.xml");
  //vfatDevice_->setDeviceBaseNode("user_regs.vfats."+confParams_.bag.deviceName.toString());
  vfatDevice_->setAddressTableFileName("testbeam_registers.xml");
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  //sleep(1);
  vfatDevice_->connectDevice();
  
  //read in default parameters from an xml file?
  //vfatDevice_->setRegisters(xmlFile);
  vfatDevice_->readVFAT2Counters();
  vfatDevice_->setRunMode(0);
  confParams_.bag.deviceChipID = vfatDevice_->getChipID();
  is_initialized_ = true;
  hw_semaphore_.give();

  //sleep(5);
  is_working_     = false;
}


void gem::supervisor::tbutils::ThresholdScan::configureAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  latency_   = confParams_.bag.latency;
  nTriggers_ = confParams_.bag.nTriggers;
  stepSize_  = confParams_.bag.stepSize;
  minThresh_ = confParams_.bag.minThresh;
  maxThresh_ = confParams_.bag.maxThresh;
  
  hw_semaphore_.take();
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  //make sure device is not running
  vfatDevice_->setRunMode(0);

  /****unimplemented at the moment
  if ((confParams_.bag.settingsFile.toString()).rfind("xml") != std::string::npos) {
    LOG4CPLUS_INFO(getApplicationLogger(),"loading settings from XML file");
    gem::supervisor::tbutils::VFAT2XMLParser::VFAT2XMLParser theParser(confParams_.bag.settingsFile.toString(),
								       vfatDevice_);
    theParser.parseXMLFile();
  }
  */
  
  //else {
    LOG4CPLUS_INFO(getApplicationLogger(),"loading default settings");
    //default settings for the frontend
    vfatDevice_->setTriggerMode(    0x3); //set to S1 to S8
    vfatDevice_->setCalibrationMode(0x0); //set to normal
    vfatDevice_->setMSPolarity(     0x1); //negative
    vfatDevice_->setCalPolarity(    0x1); //negative
    
    vfatDevice_->setProbeMode(        0x0);
    vfatDevice_->setLVDSMode(         0x0);
    vfatDevice_->setDACMode(          0x0);
    vfatDevice_->setHitCountCycleTime(0x0); //maximum number of bits
    
    vfatDevice_->setHitCountMode( 0x0);
    vfatDevice_->setMSPulseLength(0x3);
    vfatDevice_->setInputPadMode( 0x0);
    vfatDevice_->setTrimDACRange( 0x0);
    vfatDevice_->setBandgapPad(   0x0);
    vfatDevice_->sendTestPattern( 0x0);
    
    
    vfatDevice_->setIPreampIn(  168);
    vfatDevice_->setIPreampFeed(150);
    vfatDevice_->setIPreampOut(  80);
    vfatDevice_->setIShaper(    150);
    vfatDevice_->setIShaperFeed(100);
    vfatDevice_->setIComp(      120);

    vfatDevice_->setLatency(latency_);
    //}
  
  vfatDevice_->setVThreshold1(0-minThresh_);
  confParams_.bag.deviceVT1 = vfatDevice_->getVThreshold1();
  vfatDevice_->setVThreshold2(0);
  confParams_.bag.latency = vfatDevice_->getLatency();
  is_configured_ = true;
  hw_semaphore_.give();

  for (int hi = 0; hi < 128; ++hi) {
    if (histos[hi]) {
      histos[hi]->Delete();
      delete histos[hi];
      histos[hi] = 0;
    }
    TString histName  = "channel"+hi;
    TString histTitle = "Threshold scan for channel "+hi;
    int minTh = confParams_.bag.minThresh;
    int maxTh = confParams_.bag.maxThresh;
    int nBins = (maxTh - minTh +1)/(confParams_.bag.stepSize);
    //((max-min)+1)/stepSize+1
    histos[hi] = new TH1F(histName, histTitle, nBins, minTh-0.5, maxTh+0.5);
  }
  outputCanvas = new TCanvas("outputCanvas","outputCanvas",600,800);
  
  is_working_    = false;
}


void gem::supervisor::tbutils::ThresholdScan::startAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
  
  is_working_ = true;

  time_t now = time(0);
  // convert now to string form
  //char* dt = ctime(&now);
  
  tm *gmtm = gmtime(&now);
  char* utcTime = asctime(gmtm);

  std::string tmpFileName = "ThresholdScan_";
  tmpFileName.append(utcTime);
  tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
  tmpFileName.append(".dat");
  std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
  std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');
  //std::replace(tmpFileName.begin(), tmpFileName.end(), '\n', '_');

  confParams_.bag.outFileName = tmpFileName;

  LOG4CPLUS_INFO(getApplicationLogger(),"Creating file " << confParams_.bag.outFileName.toString());
  //std::fstream scanStream(confParams_.bag.outFileName.c_str(),
  std::fstream scanStream(tmpFileName.c_str(),
			  std::ios::app | std::ios::binary);
  if (scanStream.is_open())
    LOG4CPLUS_INFO(getApplicationLogger(),"file " << confParams_.bag.outFileName.toString() << "opened");

  //write some global run information header
  
  //char data[128/8]
  is_running_ = true;
  hw_semaphore_.take();

  //set clock source
  vfatDevice_->setDeviceBaseNode("OptoHybrid.CLOCKING");
  vfatDevice_->writeReg("VFAT.SOURCE",  0x0);
  //vfatDevice_->writeReg("VFAT.FALLBACK",0x1);
  vfatDevice_->writeReg("CDCE.SOURCE",  0x0);
  //vfatDevice_->writeReg("CDCE.FALLBACK",0x1);
  
  //send resync
  vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
  vfatDevice_->writeReg("Send.Resync",0x1);

  //reset counters
  vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS");
  vfatDevice_->writeReg("RESETS.L1A.External",0x1);
  vfatDevice_->writeReg("RESETS.L1A.Internal",0x1);
  vfatDevice_->writeReg("RESETS.L1A.Delayed", 0x1);
  vfatDevice_->writeReg("RESETS.L1A.Total",   0x1);

  vfatDevice_->writeReg("RESETS.CalPulse.External",0x1);
  vfatDevice_->writeReg("RESETS.CalPulse.Internal",0x1);
  vfatDevice_->writeReg("RESETS.CalPulse.Total",   0x1);

  vfatDevice_->writeReg("RESETS.Resync",0x1);
  vfatDevice_->writeReg("RESETS.BC0",   0x1);
  
  //flush FIFO
  vfatDevice_->setDeviceBaseNode("GLIB.LINK1");
  vfatDevice_->writeReg("TRK_FIFO.FLUSH", 0x1);
  
  //set trigger source
  vfatDevice_->setDeviceBaseNode("OptoHybrid.TRIGGER");
  vfatDevice_->writeReg("SOURCE",   0x2);
  vfatDevice_->writeReg("TDC_SBits",(unsigned)confParams_.bag.deviceNum);
  
  vfatDevice_->setDeviceBaseNode("GLIB");
  vfatDevice_->writeReg("TDC_SBits",(unsigned)confParams_.bag.deviceNum);
  
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  vfatDevice_->setRunMode(1);
  hw_semaphore_.give();

  //start readout

  scanStream.close();
  //start scan routine
  wl_->submit(runSig_);
  
  is_working_ = false;
}


void gem::supervisor::tbutils::ThresholdScan::stopAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  if (is_running_) {
    hw_semaphore_.take();
    vfatDevice_->setRunMode(0);
    hw_semaphore_.give();
    is_running_ = false;
  }
  
  for (int hi = 0; hi < 128; ++hi) {
    delete histos[hi];
    histos[hi] = 0;
  }
  //if (scanStream->is_open())
  //  LOG4CPLUS_INFO(getApplicationLogger(),"Closling file");
  //scanStream->close();
  //delete scanStream;
  //scanStream = 0;
  
  //wl_->submit(stopSig_);
  //wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:ThresholdScan","waiting");
  //wl_->cancel();
  
  is_working_ = false;
}


void gem::supervisor::tbutils::ThresholdScan::haltAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;

  is_configured_ = false;
  is_running_    = false;

  for (int hi = 0; hi < 128; ++hi) {
    if (histos[hi])
      delete histos[hi];
    histos[hi] = 0;
  }

  hw_semaphore_.take();
  vfatDevice_->setRunMode(0);
  hw_semaphore_.give();
  
  //wl_->submit(haltSig_);
  
  //sleep(5);
  is_working_    = false;
}


void gem::supervisor::tbutils::ThresholdScan::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;

  is_initialized_ = false;
  is_configured_  = false;
  is_running_     = false;

  hw_semaphore_.take();
  vfatDevice_->setRunMode(0);

  if (vfatDevice_->isGEMHwDeviceConnected())
    vfatDevice_->releaseDevice();
  
  if (vfatDevice_)
    delete vfatDevice_;
  
  vfatDevice_ = 0;
  sleep(2);
  hw_semaphore_.give();

  confParams_.bag.latency   = 128U;
  confParams_.bag.nTriggers = 2500U;
  confParams_.bag.minThresh = -25;
  confParams_.bag.maxThresh = 0;
  confParams_.bag.stepSize  = 1U;

  confParams_.bag.deviceName   = "";
  confParams_.bag.deviceChipID = 0x0;
  confParams_.bag.deviceVT1    = 0x0;
  confParams_.bag.deviceVT2    = 0x0;
  confParams_.bag.triggersSeen = 0;
  
  //wl_->submit(resetSig_);
  
  //sleep(5);
  is_working_     = false;
}


void gem::supervisor::tbutils::ThresholdScan::noAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = false;
  hw_semaphore_.take();
  //vfatDevice_->setRunMode(0);
  hw_semaphore_.give();
}

