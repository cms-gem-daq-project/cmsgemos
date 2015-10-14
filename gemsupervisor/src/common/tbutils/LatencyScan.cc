#include "gem/supervisor/tbutils/LatencyScan.h"

#include "gem/readout/GEMDataAMCformat.h"
#include "gem/readout/GEMDataParker.h"

#include "gem/hw/vfat/HwVFAT2.h"
#include "gem/hw/glib/HwGLIB.h"
#include "gem/hw/optohybrid/HwOptoHybrid.h"

#include "gem/utils/GEMLogging.h"

#include "TH1D.h"
#include "TH2.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"
#include "TError.h"

#include <algorithm>
#include <iomanip>
#include <ctime>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "cgicc/HTTPRedirectHeader.h"
#include "gem/supervisor/tbutils/VFAT2XMLParser.h"
#include "TStopwatch.h"

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::tbutils::LatencyScan)

/*
  typedef gem::readout::GEMDataAMCformat::GEMData  AMCGEMData;
  typedef gem::readout::GEMDataAMCformat::GEBData  AMCGEBData;
  typedef gem::readout::GEMDataAMCformat::VFATData AMCVFATData;
*/



//
void gem::supervisor::tbutils::LatencyScan::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{

  minLatency    = 13U;
  maxLatency    = 17U;
  stepSize      = 1U;
  nTriggers = 10;
  threshold = -100;

  bag->addField("minLatency",  &minLatency);
  bag->addField("maxLatency",  &maxLatency);
  bag->addField("stepSize",    &stepSize );
  bag->addField("threshold",   &threshold );
  bag->addField("nTriggers",    &nTriggers   );

}

gem::supervisor::tbutils::LatencyScan::LatencyScan(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception) :
  gem::supervisor::tbutils::GEMTBUtil(s)
{

  getApplicationInfoSpace()->fireItemAvailable("scanParams", &scanParams_);
  getApplicationInfoSpace()->fireItemValueRetrieve("scanParams", &scanParams_);

  // HyperDAQ bindings
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webDefault,      "Default"    );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webConfigure,    "Configure"  );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webStart,        "Start"      );
  runSig_   = toolbox::task::bind(this, &LatencyScan::run,        "run"       );
  readSig_  = toolbox::task::bind(this, &LatencyScan::readFIFO,   "readFIFO"  );

  // Initiate and activate main workloop
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:LatencyScan","waiting");
  wl_->activate();

  currentLatency_ = 0;
  vfat_ = 0;
  event_ = 0;
  sumVFAT_ = 0;
  counter_ = {0,0,0};
  eventsSeen_ = 0;
  channelSeen_=0;
}

gem::supervisor::tbutils::LatencyScan::~LatencyScan()
{
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:LatencyScan","waiting");
  //should we check to see if it's running and try to stop?
  wl_->cancel();
  wl_ = 0;

  if (histolatency) delete histolatency;
  histolatency = 0;

  if (outputCanvas) delete outputCanvas;
  outputCanvas = 0;

}
bool gem::supervisor::tbutils::LatencyScan::run(toolbox::task::WorkLoop* wl)
{
  wl_semaphore_.take();
  if (!is_running_) {
    wl_semaphore_.give();
    wl_->submit(readSig_);
    LOG4CPLUS_INFO(getApplicationLogger()," ******IT IS NOT RUNNIG ***** ");
    return false;
  }

  //send L1A and Calpulse
  hw_semaphore_.take();//oh sendL1ACalpulse
  optohybridDevice_->SendL1ACal(1,15);   
  sleep(1);

  //count triggers
  confParams_.bag.triggersSeen =  optohybridDevice_->GetL1ACount(0x3);// total L1A
  CalPulseCount_[0] = optohybridDevice_->GetCalPulseCount(0); //internal
  CalPulseCount_[1] = optohybridDevice_->GetCalPulseCount(1); //delayed
  CalPulseCount_[2] = optohybridDevice_->GetCalPulseCount(2); //total

  LOG4CPLUS_INFO(getApplicationLogger()," ******sent trigger ***** " << "trigger count" << confParams_.bag.triggersSeen);

  hw_semaphore_.give();//end sendL1ACalpulse

  // if triggersSeen < N triggers
  if ((uint64_t)(confParams_.bag.triggersSeen) < (uint64_t)(confParams_.bag.nTriggers)) {

    hw_semaphore_.take();//glib buffer depth
    
    // GLIB data buffer validation                                                 
    uint32_t fifoDepth[3];
    
    if (readout_mask&0x1)
      fifoDepth[0] = glibDevice_->getFIFOOccupancy(0x0);
    if (readout_mask&0x2)
      fifoDepth[1] = glibDevice_->getFIFOOccupancy(0x1);
    if (readout_mask&0x4)
      fifoDepth[2] = glibDevice_->getFIFOOccupancy(0x2);
     
    // Get the size of GLIB data buffer       
    uint32_t bufferDepth;
    if (readout_mask&0x1) {
      bufferDepth  = glibDevice_->getFIFOOccupancy(0x0); 
    }
    if (readout_mask&0x2) {
      bufferDepth = glibDevice_->getFIFOOccupancy(0x1);     
    }
    if (readout_mask&0x4) {
      bufferDepth = glibDevice_->getFIFOOccupancy(0x2);     
    }

    hw_semaphore_.give();// end glib buffer depth 

    if (bufferDepth < 1) {
      hw_semaphore_.take();//oh trigger counter update

      //trigger seen and CalCounters  updated
      confParams_.bag.triggersSeen =  optohybridDevice_->GetL1ACount(0x3);// total L1A
      CalPulseCount_[0] = optohybridDevice_->GetCalPulseCount(0); //internal
      CalPulseCount_[1] = optohybridDevice_->GetCalPulseCount(1); //delayed
      CalPulseCount_[2] = optohybridDevice_->GetCalPulseCount(2); //total

      hw_semaphore_.give();//oh trigger counter update
      wl_semaphore_.give();
      return true;
    } 
    else{ // buffer depth is less tha 1

      wl_semaphore_.give();
      for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	wl_->submit(readSig_);
      }
	    
      return true;
    } // end else buffer < 10
    //wl_semaphore_.give();
  }// end triggerSeen < N triggers
  else { 

    hw_semaphore_.take(); //vfat log 
    //disable triggers
    optohybridDevice_->setTrigSource(0x1);   
    //SB glibDevice_->setTrigSource(0x0);   
    sleep(1);

    for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	    
      (*chip)->setRunMode(0);

      hw_semaphore_.give(); //end vfat log
      wl_semaphore_.give();
	    
      wl_->submit(readSig_);    
	    
      //histolatency->Fill((int)currentLatency_, eventsSeen_);
      histolatency->Fill((int)currentLatency_, channelSeen_);
	    
      std::string imgName = "${XDAQ_DOCUMENT_ROOT}/gemdaq/gemsupervisor/html/images/tbutils/latencyscan/"
	+confParams_.bag.deviceName.toString()+"_Latency_scan.png";
	    
      outputCanvas->cd();
      histolatency->Draw("ep0");
      outputCanvas->Update();
      outputCanvas->SaveAs(TString(imgName));
    }
    hw_semaphore_.take();// vfat setrun0
    wl_semaphore_.take();
	  
    // flush fifo
    for (int i = 0; i < 2; ++i){
      if (readout_mask >> i) {
	glibDevice_->flushFIFO(i);
	while (glibDevice_->hasTrackingData(i))
	  std::vector<uint32_t> dumping = glibDevice_->getTrackingData(i);
	glibDevice_->flushFIFO(i);
      }      
    }

    //reset counters
    optohybridDevice_->ResetL1ACount(0x4);
    //optohybridDevice_->ResetResyncCount();
    //optohybridDevice_->ResetBC0Count();
    optohybridDevice_->ResetCalPulseCount(0x3);
    optohybridDevice_->SendResync();     
    
    hw_semaphore_.give(); //end glib flush fifo

    INFO(" Scan point TriggersSeen " << confParams_.bag.triggersSeen );

    //if max Latency - current Latency >= stepsize
    if (scanParams_.bag.maxLatency - currentLatency_ >= scanParams_.bag.stepSize) {

      hw_semaphore_.take();// vfat get latency

      INFO("we've seen enough triggers, changing the latency");
      INFO( "CurLaten " << (int)currentLatency_ 
	    << " TrigSeen " << confParams_.bag.triggersSeen 
	    << " CalPulses " << CalPulseCount_[2] 
	    << " eventsSeen " << eventsSeen_ 
	    << "channelseen " << channelSeen_ ); 
      
      if ((currentLatency_ + scanParams_.bag.stepSize) < 0xFF) {

	for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	  (*chip)->setLatency(currentLatency_ + scanParams_.bag.stepSize);
	}
      } else  { 

	for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	  (*chip)->setLatency(0xFF);
	}	 
      }//end else

      for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	currentLatency_ = (*chip)->getLatency();
	(*chip)->setRunMode(1);      
      }
      optohybridDevice_->setTrigSource(0x2);
      //SB glibDevice_->setTrigSource(0x2);
	    
      confParams_.bag.triggersSeen =  0;
      eventsSeen_   = 0;  
      channelSeen_ = 0;
 
      hw_semaphore_.give(); // end vfat
      wl_semaphore_.give();	

      return true;	
    } // end if maxLat - curreLat > step
    else {

      wl_semaphore_.give();
      wl_->submit(stopSig_);
      return false; 
	    
    }//end else
	  
  }//end else triggerseen < N triggers

  return false;
}//end run



bool gem::supervisor::tbutils::LatencyScan::readFIFO(toolbox::task::WorkLoop* wl)    
{
  
  // gem::readout::GEMDataAMCformat::VFATData vfat;

  gem::readout::GEMDataAMCformat::VFATData vfat;
  
  int ievent=0;

  wl_semaphore_.take();
  hw_semaphore_.take();//glib getFIFO

  // Get the size of GLIB data buffer       
  uint32_t bufferDepth;

  if (readout_mask&0x1){ 
    bufferDepth  = glibDevice_->getFIFOOccupancy(0x0); 
  }
  if (readout_mask&0x2){ 
    bufferDepth = glibDevice_->getFIFOOccupancy(0x1);     
  }
  if (readout_mask&0x4){
    bufferDepth = glibDevice_->getFIFOOccupancy(0x2);     
  }
  
  LOG4CPLUS_INFO(getApplicationLogger()," readFIFO bufferDepth " << std::hex << bufferDepth << std::dec );
  LOG4CPLUS_INFO(getApplicationLogger(), " CurLaten " << (int)currentLatency_ 
		 << " TrigSeen " << confParams_.bag.triggersSeen 
		 << " CalPulses " << CalPulseCount_[2] 
		 << " eventsSeen " << eventsSeen_
		 << "channelSeen " << channelSeen_ ); 

  //grab events from the fifo
  bool isFirst = true;
  uint32_t bxNum, bxExp;

  while (bufferDepth) {
    
    std::vector<uint32_t> data;
     
    data.reserve(7);
    uint32_t TrigReg, bxNumTr;
    uint8_t sBit;
    
    // read trigger data 
    if (readout_mask&0x1 && glibDevice_->hasTrackingData(0x0)) {
      data = glibDevice_->getTrackingData(0x0);
      TrigReg = glibDevice_->readTriggerFIFO(0x0);
    }
    
    if (readout_mask&0x2 && glibDevice_->hasTrackingData(0x1)) {
      data = glibDevice_->getTrackingData(0x1);
      TrigReg = glibDevice_->readTriggerFIFO(0x1);
    }
    
    if (readout_mask&0x4 && glibDevice_->hasTrackingData(0x2)) {
      data = glibDevice_->getTrackingData(0x2);
      TrigReg = glibDevice_->readTriggerFIFO(0x2);
    }

    //if no data has been found, shouldn't be the case, but still, need to not process any further:
    if (data.size() == 0) {
      if (readout_mask&0x1)
        bufferDepth = glibDevice_->getFIFOOccupancy(0x0);
      if (readout_mask&0x2)
        bufferDepth = glibDevice_->getFIFOOccupancy(0x1);
      if (readout_mask&0x4)
        bufferDepth = glibDevice_->getFIFOOccupancy(0x2);  
      continue;
    }
    
    bxNumTr = TrigReg >> 6;
    sBit = TrigReg & 0x0000003F;
     
    //make sure we are aligned
     
    //if (!checkHeaders(data)) 
    uint16_t b1010, b1100, b1110;
    b1010 = ((data.at(5) & 0xF0000000)>>28);
    b1100 = ((data.at(5) & 0x0000F000)>>12);
    b1110 = ((data.at(4) & 0xF0000000)>>28);
    
    bxNum = data.at(6);

    uint16_t bcn, evn, crc, chipid;
    uint64_t msData, lsData;
    uint8_t  flags;
    //    double   delVT;
    
    if (isFirst)
      bxExp = bxNum;
     
    if (bxNum == bxExp)
      isFirst = false;
     
    bxNum  = data.at(6);
    bcn    = (0x0fff0000 & data.at(5)) >> 16;
    evn    = (0x00000ff0 & data.at(5)) >> 4;
    chipid = (0x0fff0000 & data.at(4)) >> 16;
    flags  = (0x0000000f & data.at(5));
    crc    = (0x0000ffff & data.at(0));
    
    uint64_t data1  = ((0x0000ffff & data.at(4)) << 16) | ((0xffff0000 & data.at(3)) >> 16);
    uint64_t data2  = ((0x0000ffff & data.at(3)) << 16) | ((0xffff0000 & data.at(2)) >> 16);
    uint64_t data3  = ((0x0000ffff & data.at(2)) << 16) | ((0xffff0000 & data.at(1)) >> 16);
    uint64_t data4  = ((0x0000ffff & data.at(1)) << 16) | ((0xffff0000 & data.at(0)) >> 16);
     
    lsData = (data3 << 32) | (data4);
    msData = (data1 << 32) | (data2);


      vfat.BC     = ( b1010 << 12 ) | (bcn);                // 1010     | bcn:12
      vfat.EC     = ( b1100 << 12 ) | (evn << 4) | (flags); // 1100     | EC:8      | Flag:4 (zero?)
      vfat.ChipID = ( b1110 << 12 ) | (chipid);             // 1110     | ChipID:12
      vfat.lsData = lsData;                                 // lsData:64
      vfat.msData = msData;                                 // msData:64
      vfat.crc    = crc;                                    // crc:16

    gem::readout::GEMDataAMCformat::printVFATdataBits(ievent, vfat);


    if (!(((b1010 == 0xa) && (b1100==0xc) && (b1110==0xe)))){
      LOG4CPLUS_INFO(getApplicationLogger(),"VFAT headers do not match expectation");
      if (readout_mask&0x1)
	bufferDepth  = glibDevice_->getFIFOOccupancy(0x0);
      if (readout_mask&0x2)
	bufferDepth += glibDevice_->getFIFOOccupancy(0x1);
      if (readout_mask&0x4)
	bufferDepth += glibDevice_->getFIFOOccupancy(0x2);
      
      //  TrigReg = glibDevice_->readTriggerFIFO(0x2);
      continue;
    }
     
    if (readout_mask&0x1)
      bufferDepth = glibDevice_->getFIFOOccupancy(0x0);
    if (readout_mask&0x2)
      bufferDepth = glibDevice_->getFIFOOccupancy(0x1);
    if (readout_mask&0x4)
      bufferDepth = glibDevice_->getFIFOOccupancy(0x2);  
   
    //    vfatDevice_->readVFAT2Channel(1);
    
    
    if((lsData>>63) && 0x1){
      ++channelSeen_;
    }

    //only count events if there is a hit in the data packet
    //without this, it will just count the reveived data packets, which is not quite right
    if (lsData || msData){
      ++eventsSeen_;
    }
  }
  
  
  LOG4CPLUS_INFO(getApplicationLogger(), "END READING*****CurLaten " << (int)currentLatency_ << 
		 " TrigSeen " << confParams_.bag.triggersSeen << " CalPulses " << CalPulseCount_[2] << 
		 " eventsSeen " << eventsSeen_ << " channelSeen " << channelSeen_ ); 

  hw_semaphore_.give();
  wl_semaphore_.give();

  return false;
}

void gem::supervisor::tbutils::LatencyScan::scanParameters(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    *out << cgicc::span()   << std::endl
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
	 << cgicc::br() << std::endl

	 << cgicc::label("Current Latency").set("for","CurrentLatency") << std::endl
	 << cgicc::input().set("id","CurrentLatency").set("name","CurrentLatency")
      .set("type","text").set("readonly")
      .set("value",boost::str(boost::format("%d")%((unsigned)currentLatency_)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("Latency Step").set("for","LatencyStep") << std::endl
	 << cgicc::input().set("id","LatencyStep").set("name","LatencyStep")
      .set("type","number").set("min","1").set("max","255")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.stepSize)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("Triggers to take").set("for","TriggersToTake") << std::endl
	 << cgicc::input().set("id","TriggersToTake").set("name","TriggersToTake")
      .set("type","number").set("min","0")
      .set("value",boost::str(boost::format("%d")%(confParams_.bag.nTriggers)))
	 << cgicc::br() << std::endl
	 << cgicc::label("Triggers taken").set("for","TriggersSeen") << std::endl
	 << cgicc::input().set("id","TriggersSeen").set("name","TriggersSeen")
      .set("type","text").set("readonly")
      .set("value",boost::str(boost::format("%d")%(confParams_.bag.triggersSeen)))
      //.set("value",boost::str(boost::format("%d")%(triggersSeen_)))
	 << std::endl
	 << cgicc::label("Events with hits").set("for","EventsSeen") << std::endl
	 << cgicc::input().set("id","EventsSeen").set("name","EventsSeen")
      .set("type","text").set("readonly")
      .set("value",boost::str(boost::format("%d")%(eventsSeen_)))
	 << cgicc::br()   << std::endl

	 << cgicc::span() << std::endl;
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



//
void gem::supervisor::tbutils::LatencyScan::displayHistograms(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    *out << cgicc::img().set("src","/gemdaq/gemsupervisor/html/images/tbutils/latencyscan/"+
			     confParams_.bag.deviceName.toString()+"_Latency_scan.png")
      .set("id","vfatChannelHisto")
	 << cgicc::br()  << std::endl;
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
	.set("name", "command").set("title", "Configure scan.")
	.set("value", "Configure") << std::endl;
      *out << cgicc::form()        << std::endl;
    }

    else if (!is_running_) {
      //hardware is initialized and configured, we can start the run
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Start") << std::endl;

      selectMultipleVFAT(out);
      scanParameters(out);

      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Start scan.")
	.set("value", "Start") << std::endl;
      *out << cgicc::form()    << std::endl;
    }

    else if (is_running_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Stop") << std::endl;

      selectMultipleVFAT(out);
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
    *out << "<div class=\"xdaq-tab\" title=\"Scan histogram\">"  << std::endl;
    displayHistograms(out);

    *out << "</div>" << std::endl;
    *out << "</div>" << std::endl;

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
    *out << cgicc::script().set("type","text/javascript")
      .set("src","/gemdaq/gemsupervisor/html/scripts/tbutils/changeImage.js")
	 << cgicc::script() << std::endl;

  }//end try
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying LatencyScan control panel(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying LatencyScan control panel(std): " << e.what());
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
  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
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

  vfat_ = 0;
  event_ = 0;
  sumVFAT_ = 0;
  counter_ = {0,0,0};
  eventsSeen_ = 0;
  channelSeen_ = 0;

  hw_semaphore_.take();
  LOG4CPLUS_INFO(getApplicationLogger(), "attempting to configure device");

  //make sure device is not running

  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {

  (*chip)->setRunMode(0);

  LOG4CPLUS_INFO(getApplicationLogger(),"loading default settings");
  //default settings for the frontend
  (*chip)->setTriggerMode(    0x3); //set to S1 to S8
  (*chip)->setCalibrationMode(0x0); //set to normal
  (*chip)->setMSPolarity(     0x1); //negative
  (*chip)->setCalPolarity(    0x1); //negative

  (*chip)->setProbeMode(        0x0);
  (*chip)->setLVDSMode(         0x0);
  (*chip)->setHitCountCycleTime(0x0); //maximum number of bits

  (*chip)->setHitCountMode( 0x0);
  (*chip)->setMSPulseLength(0x3);
  (*chip)->setInputPadMode( 0x0);
  (*chip)->setTrimDACRange( 0x0);
  (*chip)->setBandgapPad(   0x0);
  (*chip)->sendTestPattern( 0x0);

  (*chip)->setVCal(100);
  (*chip)->setIPreampIn(  168);
  (*chip)->setIPreampFeed(150);
  (*chip)->setIPreampOut(  80);
  (*chip)->setIShaper(    150);
  (*chip)->setIShaperFeed(100);
  (*chip)->setIComp(       75);//120

  (*chip)->setVThreshold1( 50);//50
  (*chip)->setVThreshold2(  0);



  LOG4CPLUS_INFO(getApplicationLogger(), "setting DAC mode to normal");
  (*chip)->setDACMode(gem::hw::vfat::StringToDACMode.at("OFF"));

  LOG4CPLUS_INFO(getApplicationLogger(), "setting starting latency value");
  (*chip)->setLatency(    scanParams_.bag.minLatency);

  LOG4CPLUS_INFO(getApplicationLogger(), "reading back current latency value");
  currentLatency_ = (*chip)->getLatency();

  LOG4CPLUS_INFO(getApplicationLogger(), "CurrentLatency" << currentLatency_);

  LOG4CPLUS_INFO(getApplicationLogger(), "device configured");
  is_configured_ = true;



  }

  //enable channel 
  /*  vfatDevice_->enableCalPulseToChannel(10,true);
      vfatDevice_->enableCalPulseToChannel(11,true);
      vfatDevice_->enableCalPulseToChannel(12,true);*/

  //flush fifo
  for (int i = 0; i < 2; ++i){
    if (readout_mask >> i) {
      glibDevice_->flushFIFO(i);
      while (glibDevice_->hasTrackingData(i))
	std::vector<uint32_t> dumping = glibDevice_->getTrackingData(i);
      glibDevice_->flushFIFO(i);
    }
  }

  //reset counters
  optohybridDevice_->ResetL1ACount(0x4);
  optohybridDevice_->ResetResyncCount();
  optohybridDevice_->ResetBC0Count();
  optohybridDevice_->ResetCalPulseCount(0x3);
  optohybridDevice_->SendResync();      
  //  vfatDevice_->setRunMode(1);      

  hw_semaphore_.give();

  if (histolatency) delete histolatency;
  histolatency = 0;

  LOG4CPLUS_INFO(getApplicationLogger(), "setting up histogram");
  int minVal = scanParams_.bag.minLatency;
  int maxVal = scanParams_.bag.maxLatency;
  int nBins = (maxVal - minVal +1)/(scanParams_.bag.stepSize);
  histolatency = new TH1D("LatencyScan", "Latency scan", nBins, minVal-0.5, maxVal+0.5);

  outputCanvas = new TCanvas("outputCanvas","outputCanvas",600,800);

  LOG4CPLUS_INFO(getApplicationLogger(), "configure routine completed");


  is_working_    = false;

}		 

//
void gem::supervisor::tbutils::LatencyScan::startAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  wl_semaphore_.take();
  is_working_ = true;

  time_t now = time(0);
  tm *gmtm = gmtime(&now);
  char* utcTime = asctime(gmtm);

  std::string tmpFileName = "LatencyScan_";
  tmpFileName.append(utcTime);
  tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
  tmpFileName.append(".dat");
  std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
  std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');

  confParams_.bag.outFileName = tmpFileName;

  // Book GEM Data Parker                                                                 
  //SB gemDataParker = new gem::readout::GEMDataParker(*glibDevice_, tmpFileName, tmpFileName);

  LOG4CPLUS_INFO(getApplicationLogger(),"Creating file " << confParams_.bag.outFileName.toString());
  //std::fstream scanStream(confParams_.bag.outFileName.c_str(),
  std::fstream scanStream(tmpFileName.c_str(),  std::ios::app | std::ios::binary);

  if (scanStream.is_open()){
    LOG4CPLUS_DEBUG(getApplicationLogger(),"file " << confParams_.bag.outFileName.toString() << "opened");
  }
  //write some global run information header

  //char data[128/8]
  is_running_ = true;
  hw_semaphore_.take();//oh reset counters

  //set trigger source
  optohybridDevice_->setTrigSource(0x2);   
  //SB glibDevice_->setTrigSource(0x2);   

  //set clock source
  optohybridDevice_->SetVFATClock(1,1,0x0);    
  optohybridDevice_->SetCDCEClock(1,1,0x0); 
  //send resync
  optohybridDevice_->SendResync();
  //reset counters
  optohybridDevice_->ResetL1ACount(0x4);
  optohybridDevice_->ResetResyncCount();
  optohybridDevice_->ResetBC0Count();
  optohybridDevice_->ResetCalPulseCount(0x3);


  hw_semaphore_.give();//end oh reset counters
  hw_semaphore_.take();//glib flush fifo

  //flush FIFO                                                                            
  for (int i = 0; i < 2; ++i){
    if (readout_mask >> i) {
      glibDevice_->flushFIFO(i);
      while (glibDevice_->hasTrackingData(i))
	std::vector<uint32_t> dumping = glibDevice_->getTrackingData(i);
      glibDevice_->flushFIFO(i);
    }
  }

  //enable the channels 56-64 
  /*  vfatDevice_->enableCalPulseToChannel(10,true);
      vfatDevice_->enableCalPulseToChannel(11,true);
      vfatDevice_->enableCalPulseToChannel(12,true);*/

  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
    (*chip)->setRunMode(1);
  }


hw_semaphore_.give();//end vfat

  //start readout
  scanStream.close();

  if (histolatency) {
    delete histolatency;
    histolatency = 0;
  }
  int minVal = scanParams_.bag.minLatency;
  int maxVal = scanParams_.bag.maxLatency;
  int nBins = (maxVal - minVal +1)/(scanParams_.bag.stepSize);
  histolatency = new TH1D("LatencyScan", "Latency scan", nBins, minVal-0.5, maxVal+0.5);


  //felipe+1


  wl_->submit(runSig_);
  is_working_ = false;
  wl_semaphore_.give();

}

//
void gem::supervisor::tbutils::LatencyScan::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  gem::supervisor::tbutils::GEMTBUtil::resetAction(e);
  {

    confParams_.bag.nTriggers  = 100U;
    scanParams_.bag.minLatency = 0U;
    scanParams_.bag.maxLatency = 25U;
    scanParams_.bag.stepSize   = 1U;
    scanParams_.bag.threshold = -100U;
    //    confParams_.bag.deviceName   = "";
    confParams_.bag.deviceChipID = 0x0;

    is_working_     = false;
  }
}
