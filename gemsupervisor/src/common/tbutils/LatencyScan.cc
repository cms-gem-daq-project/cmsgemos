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
#include <queue>


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


  hw_semaphore_.take();//oh sendL1ACalpulse

  //send L1A and Calpulse
  if((unsigned)confParams_.bag.triggerSource == (unsigned)0x1){
  optohybridDevice_->sendL1ACal(1,15);  //from T1 generator
  sleep(1);
  }
  if((unsigned)confParams_.bag.triggerSource == (unsigned)0x2){
  optohybridDevice_->sendCalPulse(1);  //from T1 generator
  sleep(1);
  }
  
  //count triggers and Calpulses coming from TTC
  confParams_.bag.triggersSeen =  optohybridDevice_->getL1ACount(0x1);
  CalPulseCount_[0] = optohybridDevice_->getCalPulseCount(0x1); 

  LOG4CPLUS_INFO(getApplicationLogger(),"***sent trigger *** "
		 << "trigger count " << confParams_.bag.triggersSeen
		 << " CalpulseCount " << CalPulseCount_[0]);

  hw_semaphore_.give();//end sendL1ACalpulse

  // if triggersSeen < N triggers
  if ((uint64_t)(confParams_.bag.triggersSeen) < (uint64_t)(confParams_.bag.nTriggers)) {

    hw_semaphore_.take();//glib buffer depth
    
    // GLIB data buffer validation                                                 
    uint32_t fifoDepth[3];
    fifoDepth[0] = glibDevice_->getFIFOOccupancy(readout_mask);

    // Get the size of GLIB data buffer       
    uint32_t bufferDepth;
    bufferDepth  = glibDevice_->getFIFOOccupancy(readout_mask); 

    hw_semaphore_.give();// end glib buffer depth 

    if (bufferDepth < 1) {
      hw_semaphore_.take();//oh trigger counter update

      //trigger seen and CalCounters  updated
      confParams_.bag.triggersSeen =  optohybridDevice_->getL1ACount(0x0);// L1A from GLIB
      CalPulseCount_[0] = optohybridDevice_->getCalPulseCount(0); //from GLIB
 
      hw_semaphore_.give();//oh trigger counter update
      wl_semaphore_.give();
      return true;
    } 
    else{ // buffer depth is less than 1

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
    if(confParams_.bag.triggerSource.value_==0x1){
      optohybridDevice_->stopT1Generator(true);
    } else { 
      optohybridDevice_->setTrigSource(0x1);       
    }
    sleep(1);

    for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	    
      (*chip)->setRunMode(0);

      hw_semaphore_.give(); //end vfat log
      wl_semaphore_.give();
	    
      wl_->submit(readSig_);    
	    
      histolatency->Fill((int)currentLatency_, eventsSeen_);
      //histolatency->Fill((int)currentLatency_, channelSeen_);
	    
      std::string imgName = "${XDAQ_DOCUMENT_ROOT}/gemdaq/gemsupervisor/html/images/tbutils/latencyscan/"
	+confParams_.bag.deviceName.toString()+"_Latency_scan.png";
	    
      outputCanvas->cd();
      histolatency->Draw("ep0");
      outputCanvas->Update();
      outputCanvas->SaveAs(TString(imgName));
    }// end for  
    hw_semaphore_.take();// vfat setrun0
    wl_semaphore_.take();
	  
 // flush FIFO, how to disable a specific, misbehaving, chip
  INFO("Flushing the FIFOs, readout_mask 0x" <<std::hex << (int)readout_mask << std::dec);
  DEBUG("Flushing FIFO" << readout_mask << " (depth " << glibDevice_->getFIFOOccupancy(readout_mask));
  glibDevice_->flushFIFO(readout_mask);
  while (glibDevice_->hasTrackingData(readout_mask)) {
    glibDevice_->flushFIFO(readout_mask);
    std::vector<uint32_t> dumping = glibDevice_->getTrackingData(readout_mask,
                                                                 glibDevice_->getFIFOVFATBlockOccupancy(readout_mask));
  }
  // once more for luck
  glibDevice_->flushFIFO(readout_mask);
  
    //reset counters
    optohybridDevice_->resetL1ACount(0x1);
    optohybridDevice_->resetResyncCount();
    optohybridDevice_->resetBC0Count();
    optohybridDevice_->resetCalPulseCount(0x1);
    optohybridDevice_->sendResync();     
    optohybridDevice_->sendBC0();          

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
    
      if(confParams_.bag.triggerSource.value_==0x0){
	optohybridDevice_->setTrigSource(0x0);//from AMC13   
      }else if(confParams_.bag.triggerSource.value_==0x1){
	optohybridDevice_->setTrigSource(0x1);//from T1   
      }else if(confParams_.bag.triggerSource.value_==0x2){
	optohybridDevice_->setTrigSource(0x2);//from sbits   
      }else if(confParams_.bag.triggerSource.value_==0x3){
	optohybridDevice_->setTrigSource(0x3);//from Ext_LEMO   
      }
    
      CalPulseCount_[0] = 0;	    
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

  gem::readout::GEMDataAMCformat::VFATData vfat;
  
  int ievent=0;

  wl_semaphore_.take();
  hw_semaphore_.take();//glib getFIFO

  // Get the size of GLIB data buffer       
  uint32_t bufferDepth;
  uint32_t TrigReg;

    bufferDepth  = glibDevice_->getFIFOOccupancy(readout_mask); 
  
  LOG4CPLUS_INFO(getApplicationLogger()," readFIFO bufferDepth " << std::hex << bufferDepth << std::dec );
  LOG4CPLUS_INFO(getApplicationLogger(), " CurLaten " << (int)currentLatency_ 
		 << " TrigSeen " << confParams_.bag.triggersSeen 
		 << " CalPulses " << CalPulseCount_[0] 
		 << " eventsSeen " << eventsSeen_
		 << "channelSeen " << channelSeen_ ); 

  //grab events from the fifo
  while (bufferDepth) {
    
    std::vector<uint32_t> data;
    std::queue<uint32_t> data_fifo;     

    // read trigger data 
    std::vector<uint32_t> dumping = glibDevice_->getTrackingData(readout_mask,
                                                                 glibDevice_->getFIFOVFATBlockOccupancy(readout_mask));

      data = glibDevice_->getTrackingData(0x0,glibDevice_->getFIFOOccupancy(readout_mask));
      TrigReg = glibDevice_->readTriggerFIFO(readout_mask);

    //if no data has been found, shouldn't be the case, but still, need to not process any further:
    if (data.size() == 0) {
        bufferDepth = glibDevice_->getFIFOOccupancy(readout_mask);
      continue;
    }
    
    for (auto iword = data.begin(); iword != data.end(); ++iword){
      data_fifo.push(*iword);
    }
     
    //make sure we are aligned
    uint16_t b1010, b1100, b1110;
    uint16_t bcn, evn, crc, chipid;
    uint8_t  flags;
    uint32_t BX;
    uint32_t data10, data11, data20, data21, data30, data31, data40, data41;
    uint64_t data1, data2, data3, data4, msData, lsData;

    //    uint32_t datafront = 0;
    for(int j=1; j<8;j++){
    uint32_t datafront = data_fifo.front();
      if ( j == 1 ){
	b1010   = ((0xf0000000 & datafront) >> 28 );
	bcn     = ((0x0fff0000 & datafront) >> 16 );
	b1100   = ((0x0000f000 & datafront) >> 12 );
	evn     = (uint8_t)((0x00000ff0 & datafront) >>  4 );
	flags   = (0x0000000f & datafront);
      } else if ( j == 2 ){
	b1110   = ((0xf0000000 & datafront) >> 28 );
	chipid  = ((0x0fff0000 & datafront) >> 16 );
	data10   = (uint16_t)((0x0000ffff & datafront) << 16 );
      }
      if (!(((b1010 == 0xa) && (b1100==0xc)))){
	LOG4CPLUS_INFO(getApplicationLogger(),"VFAT Data Package is misAligned");
	bool misAligned_ = true;
	while ((misAligned_) && (data_fifo.size()>6)){
	  
	  data_fifo.pop();
	  datafront = data_fifo.front();
	  b1010   = ((0xf0000000 & datafront) >> 28 );
	  bcn     = ((0x0fff0000 & datafront) >> 16 );
	  b1100   = ((0x0000f000 & datafront) >> 12 );
	  evn     = (uint8_t)((0x00000ff0 & datafront) >>  4 );
	  flags   = (0x0000000f & datafront);
	  if ((b1010 == 0xa && b1100 == 0xc)) { misAligned_ = false;}
	}//end while misaligned
      }// end if it is misaligned
            
      if ( j == 3 ){
	data11   = (uint16_t)((0xffff0000 & datafront) >> 16 );
	data20   = (uint16_t)((0x0000ffff & datafront) << 16 );
      } else if ( j == 4 ){
	data40   = (uint16_t)((0x0000ffff & datafront) << 16 );
	data31   = (uint16_t)((0xffff0000 & datafront) >> 16 );
      } else if ( j == 5 ){
	data21   = (uint16_t)((0xffff0000 & datafront) >> 16 );
	data30   = (uint16_t)((0x0000ffff & datafront) << 16 );
      } else if ( j == 6 ){
	data41   = (uint16_t)((0xffff0000 & datafront) >> 16 );
	crc = (uint16_t)(0x0000ffff & datafront);
      } else if ( j == 7 ){
	BX      = (uint32_t)datafront;
      }

    
    data_fifo.pop();
    
    }// end for words

    data1  = data10 | data11;
    data2  = data20 | data21;
    data3  = data30 | data31;
    data4  = data40 | data41;

    
    lsData = (data3 << 32) | (data4);
    msData = (data1 << 32) | (data2);

    
    vfat.BC     = ( b1010 << 12 ) | (bcn);                // 1010     | bcn:12
    vfat.EC     = ( b1100 << 12 ) | (evn << 4) | (flags); // 1100     | EC:8      | Flag:4 (zero?)
    vfat.ChipID = ( b1110 << 12 ) | (chipid);             // 1110     | ChipID:12
    vfat.lsData = lsData;                                 // lsData:64
    vfat.msData = msData;                                 // msData:64
    vfat.BXfrOH = BX;                                     // BXfrOH:32
    vfat.crc    = crc;                                    // crc:16
   
    //print data package    
    gem::readout::GEMDataAMCformat::printVFATdataBits(ievent, vfat);
    
    if((lsData>>63) && 0x1){
      ++channelSeen_;
    }

    //only count events if there is a hit in the data packet
    //without this, it will just count the reveived data packets, which is not quite right
    if (lsData || msData){
      ++eventsSeen_;
    }    

      bufferDepth = glibDevice_->getFIFOOccupancy(readout_mask);
  }//end while buffer
  
  
  LOG4CPLUS_INFO(getApplicationLogger(), "END READING*****CurLaten " << (int)currentLatency_ << 
		 " TrigSeen " << confParams_.bag.triggersSeen << " CalPulses " << CalPulseCount_[0] << 
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

	 << cgicc::label("MinLatency").set("for","MinLatency") << std::endl
	 << cgicc::input().set("id","MinLatency").set("name","MinLatency")
      .set("type","number").set("min","0").set("max","255")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.minLatency)))
	 << std::endl
	 << cgicc::label("Set Threshold").set("for","Threshold") << std::endl
	 << cgicc::input().set("id","Threshold").set("name","Threshold")
      .set("type","number").set("min","0").set("max","255")
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

      selectOptohybridDevice(out);
      selectMultipleVFAT(out);
      selectTrigSource(out);
      scanParameters(out);


      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Initialize hardware acces.")
	.set("value", "Initialize") << std::endl;

      *out << cgicc::form() << std::endl;
    }

    else if (!is_configured_) {
      //this will allow the parameters to be set to the chip and scan routine

      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Configure") << std::endl;

      selectOptohybridDevice(out);
      selectMultipleVFAT(out);
      selectTrigSource(out);
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
      selectTrigSource(out);
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
      selectTrigSource(out);
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

    element = cgi.getElement("Threshold");
    if (element != cgi.getElements().end())
      scanParams_.bag.threshold  = element->getIntegerValue();

    cgicc::form_iterator trgsrciterator = cgi.getElement("SetTrigSrc");
    if (strcmp((**trgsrciterator).c_str(),"Calpulse+L1A") == 0) {
      confParams_.bag.triggerSource = 0x1;
      optohybridDevice_->setTrigSource(0x1); 
      INFO("Fake Latency Scan sending Calpulses+L1As. TrigSource : " << confParams_.bag.triggerSource);
    }//if calpulse+l1a
    else if (strcmp((**trgsrciterator).c_str(),"sBits_looping_back") == 0) {
      confParams_.bag.triggerSource = 0x3;
      optohybridDevice_->setTrigSource(0x3);
      INFO("Sending Calpulses and sbits come back from the OH. TrigSource : " << confParams_.bag.triggerSource );
    }//if sbits looping back
    else if (strcmp((**trgsrciterator).c_str(),"Ext_Trigger_AMC13") == 0) {
      confParams_.bag.triggerSource = 0x0;
      optohybridDevice_->setTrigSource(0x0);
      INFO("Real signals and the trigger comes from the AMC13. TrigSource : " << confParams_.bag.triggerSource );
    }//if external
    else if (strcmp((**trgsrciterator).c_str(),"Ext_LEMO_Cable") == 0) {
      confParams_.bag.triggerSource = 0x2;
      optohybridDevice_->setTrigSource(0x2);
      INFO("Real signals and the trigger comes from the LEMO Cable. TrigSource : " << confParams_.bag.triggerSource );
    }//if external

  }//end try
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

    confParams_.bag.deviceChipID = (*chip)->getChipID();
    (*chip)->setDeviceIPAddress(confParams_.bag.deviceIP);
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

  (*chip)->setVThreshold1(scanParams_.bag.threshold);//50
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
  INFO("Flushing the FIFOs, readout_mask 0x" <<std::hex << (int)readout_mask << std::dec);
  DEBUG("Flushing FIFO" << readout_mask << " (depth " << glibDevice_->getFIFOOccupancy(readout_mask));
  glibDevice_->flushFIFO(readout_mask);
  while (glibDevice_->hasTrackingData(readout_mask)) {
    glibDevice_->flushFIFO(readout_mask);
    std::vector<uint32_t> dumping = glibDevice_->getTrackingData(readout_mask,
                                                                 glibDevice_->getFIFOVFATBlockOccupancy(readout_mask));
  }
  // once more for luck
  glibDevice_->flushFIFO(readout_mask);

  //reset counters
  optohybridDevice_->resetL1ACount(0x1);
  optohybridDevice_->resetCalPulseCount(0x1);
  optohybridDevice_->resetResyncCount();
  optohybridDevice_->resetBC0Count();
  optohybridDevice_->sendResync();      
  optohybridDevice_->sendBC0();          
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
  if(confParams_.bag.triggerSource.value_==0x0){
    optohybridDevice_->setTrigSource(0x0);//from AMC13   
  }else if(confParams_.bag.triggerSource.value_==0x1){
    optohybridDevice_->setTrigSource(0x1);//from T1   
  }else if(confParams_.bag.triggerSource.value_==0x2){
    optohybridDevice_->setTrigSource(0x2);//from sbits   
  }else if(confParams_.bag.triggerSource.value_==0x3){
    optohybridDevice_->setTrigSource(0x3);//from Ext_LEMO   
  }

  //set clock source
  //optohybridDevice_->setVFATClock(1,1,0x0);    
  //optohybridDevice_->setCDCEClock(1,1,0x0); 

  //reset counters
  optohybridDevice_->resetL1ACount(0x1);
  optohybridDevice_->resetResyncCount();
  optohybridDevice_->resetBC0Count();
  optohybridDevice_->resetCalPulseCount(0x1);
  optohybridDevice_->sendResync();     
  optohybridDevice_->sendBC0();          

  hw_semaphore_.give();//end oh reset counters
  hw_semaphore_.take();//glib flush fifo

  //flush fifo
  INFO("Flushing the FIFOs, readout_mask 0x" <<std::hex << (int)readout_mask << std::dec);
  DEBUG("Flushing FIFO" << readout_mask << " (depth " << glibDevice_->getFIFOOccupancy(readout_mask));
  glibDevice_->flushFIFO(readout_mask);
  while (glibDevice_->hasTrackingData(readout_mask)) {
    glibDevice_->flushFIFO(readout_mask);
    std::vector<uint32_t> dumping = glibDevice_->getTrackingData(readout_mask,
                                                                 glibDevice_->getFIFOVFATBlockOccupancy(readout_mask));
  }
  // once more for luck
  glibDevice_->flushFIFO(readout_mask);
  
  optohybridDevice_->sendResync();      
  optohybridDevice_->sendBC0();          

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
    scanParams_.bag.threshold  = 50U;
    //    confParams_.bag.deviceName   = "";
    confParams_.bag.deviceChipID = 0x0;

    is_working_     = false;
  }
}

void gem::supervisor::tbutils::LatencyScan::selectTrigSource(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    bool isDisabled = false;
    if (is_running_ || is_configured_ || is_initialized_)
      isDisabled = true;

    cgicc::input triggersourceselection;
    /*    if (isDisabled)
      triggersourceselection.set("disabled","disabled");
      else*/
      *out   << "<table>"     << std::endl
	     << "<tr>"   << std::endl
	     << "<td>" << "Select kind of Latency Scan: " << "</td>" << std::endl	 
	     << "</tr>"     << std::endl
	     << "<tr>" << std::endl
	     << "<td>" << std::endl
	     << cgicc::select().set("name","SetTrigSrc") << std::endl
	     << cgicc::option("Calpulse+L1A").set("value","Calpulse+L1A")
	     << cgicc::option("sBits_looping_back").set("value","sBits_looping_back")  
	     << cgicc::option("Ext_Trigger_AMC13").set("value","Ext_Trigger_AMC13")  
	     << cgicc::option("Ext_LEMO_Cable").set("value","Ext_LEMO_Cable") << std::endl
	     << cgicc::select()<< std::endl
	     << "</td>"    << std::endl
	     << "</tr>"    << std::endl
	     << "</table>" << std::endl;
    /*      *out << "<tr><td class=\"title\"> Select Latency Scan: </td>"
	      << "<td class=\"form\">"*/

}//end try
catch (const xgi::exception::Exception& e) {
  INFO("Something went wrong setting the trigger source): " << e.what());
  XCEPT_RAISE(xgi::exception::Exception, e.what());
 }
 catch (const std::exception& e) {
   INFO("Something went wrong setting the trigger source): " << e.what());
   XCEPT_RAISE(xgi::exception::Exception, e.what());
 }

}// end void selectTrigSource

