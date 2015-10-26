#include "gem/supervisor/tbutils/ThresholdScan.h"
#include "gem/supervisor/tbutils/ThresholdEvent.h"

#include "gem/readout/GEMDataParker.h"
#include "gem/readout/GEMDataAMCformat.h"

#include "gem/hw/vfat/HwVFAT2.h"
#include "gem/hw/glib/HwGLIB.h"
#include "gem/hw/optohybrid/HwOptoHybrid.h"

#include "gem/utils/GEMLogging.h"

#include "TH1.h"
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

#include <iostream>
#include "xdata/Vector.h"
#include <string>

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::tbutils::ThresholdScan)

bool First = true, Last = false;

TH1F* RTime = NULL;
TCanvas *can = NULL;

void gem::supervisor::tbutils::ThresholdScan::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{
  latency      = 12U;
  minThresh    = -80;
  maxThresh    = 20;
  stepSize     = 5U;
  currentHisto = 0U;

  deviceVT1    = 0x0;
  deviceVT2    = 0x0;

  bag->addField("minThresh",   &minThresh);
  bag->addField("maxThresh",   &maxThresh);
  bag->addField("stepSize",    &stepSize );
  bag->addField("currentHisto",&currentHisto);
  bag->addField("deviceVT1",   &deviceVT1   );
  bag->addField("deviceVT2",   &deviceVT2   );

}

gem::supervisor::tbutils::ThresholdScan::ThresholdScan(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception) :
  gem::supervisor::tbutils::GEMTBUtil(s)
{

  getApplicationInfoSpace()->fireItemAvailable("scanParams", &scanParams_);
  getApplicationInfoSpace()->fireItemValueRetrieve("scanParams", &scanParams_);

  // HyperDAQ bindings
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webDefault,      "Default"    );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webConfigure,    "Configure"  );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ThresholdScan::webStart,        "Start"      );
  runSig_   = toolbox::task::bind(this, &ThresholdScan::run,        "run"       );
  readSig_  = toolbox::task::bind(this, &ThresholdScan::readFIFO,   "readFIFO"  );

  // Initiate and activate main workloop  
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:ThresholdScan","waiting");
  wl_->activate();

}

gem::supervisor::tbutils::ThresholdScan::~ThresholdScan()
{
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:ThresholdScan","waiting");
  //should we check to see if it's running and try to stop?
  wl_->cancel();
  wl_ = 0;
  
  if (histo) delete histo;
  histo = 0;

  for (int hi = 0; hi < 128; ++hi) {
    if (histos[hi]) delete histos[hi];
    histos[hi] = 0;
  }

  if (outputCanvas) delete outputCanvas;
  outputCanvas = 0;

}

// State transitions
bool gem::supervisor::tbutils::ThresholdScan::run(toolbox::task::WorkLoop* wl)
{
  wl_semaphore_.take();
  if (!is_running_) {
    wl_semaphore_.give();
    wl_->submit(readSig_);
    LOG4CPLUS_INFO(getApplicationLogger()," ******IT IS NOT RUNNIG ***** ");
    return false;
  }

  //send triggers
  hw_semaphore_.take();
  for (size_t trig = 0; trig < 500; ++trig) optohybridDevice_->sendL1A(0x1);  // Sent from T1 generator

  //count triggers
  confParams_.bag.triggersSeen =  optohybridDevice_->getL1ACount(0x1);// Sent from T1 generator

  LOG4CPLUS_INFO(getApplicationLogger(),
		 " ABC TriggersSeen " << confParams_.bag.triggersSeen);

  hw_semaphore_.give();
  
  // if triggersSeen < N triggers
  if ((uint64_t)(confParams_.bag.triggersSeen) < (uint64_t)(confParams_.bag.nTriggers)) {
    
    hw_semaphore_.take();
    
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

    hw_semaphore_.give();
    
    if (bufferDepth < 1) {
      hw_semaphore_.take();
      
      //trigger seen and CalCounters  updated
      confParams_.bag.triggersSeen =  optohybridDevice_->getL1ACount(0x0);// L1A from GLIB
      CalPulseCount_[0] = optohybridDevice_->getCalPulseCount(0); //from GLIB
      
      hw_semaphore_.give();
      wl_semaphore_.give();
      return true;
    }
    else { // buffer depth is less than 1

      wl_semaphore_.give();
      for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	wl_->submit(readSig_);
      }
      
      return true;
    }//end else buffer<1
    //wl_semaphore_.give();
  }//end if triggerSeen < nTrigger
  else {
    
    hw_semaphore_.take();
    
    for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {

	  (*chip)->setRunMode(0);
	  
	  hw_semaphore_.give();
	  wl_semaphore_.give();

	  wl_->submit(readSig_);
	  
    }//end for

	  wl_semaphore_.take();
	  hw_semaphore_.take();
	  
  //flush fifo
  for (int i = 0; i < 2; ++i){
    if ((readout_mask >> i)&0x1) {
      glibDevice_->flushFIFO(i);
      while (glibDevice_->hasTrackingData(i)){
	glibDevice_->flushFIFO(i);
	INFO( " has data" << i << " (depth " << glibDevice_->getFIFOOccupancy(i));
	//get trackindata has another entry 
	std::vector<uint32_t> dumping = glibDevice_->getTrackingData(i, glibDevice_->getFIFOOccupancy(i));
      }
      glibDevice_->flushFIFO(i);
    }
  }

    //reset counters
    optohybridDevice_->resetL1ACount(0x1);
    optohybridDevice_->resetResyncCount();
    optohybridDevice_->resetBC0Count();
    optohybridDevice_->resetCalPulseCount(0x1);
    optohybridDevice_->sendResync();     
    optohybridDevice_->sendBC0();          
	  
    hw_semaphore_.give();
	  
    LOG4CPLUS_INFO(getApplicationLogger()," ABC Scan point TriggersSeen " 
		   << confParams_.bag.triggersSeen );
    
    if ( (unsigned)scanParams_.bag.deviceVT1 == (unsigned)0x0 ) {
      wl_semaphore_.give();
      wl_->submit(stopSig_);
      return false;
      
    }//end if deviceVT1=0
    else if ( (scanParams_.bag.deviceVT2-scanParams_.bag.deviceVT1) <= scanParams_.bag.maxThresh ) {    
      //if VT2 - VT1 <= maxThreshold
      
      hw_semaphore_.take();
      
      LOG4CPLUS_INFO(getApplicationLogger()," ABC run: VT1= " 
		     << scanParams_.bag.deviceVT1 << " VT2-VT1= " << scanParams_.bag.deviceVT2-scanParams_.bag.deviceVT1 
		     << " bag.maxThresh= " << scanParams_.bag.maxThresh 
		     << " abs(VT2-VT1) " << abs(scanParams_.bag.deviceVT2-scanParams_.bag.deviceVT1) );

      hw_semaphore_.give();
      //how to ensure that the VT1 never goes negative
      hw_semaphore_.take();
      //if VT1 > stepSize      
      if (scanParams_.bag.deviceVT1 > scanParams_.bag.stepSize) {

	for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	  (*chip)->setVThreshold1(scanParams_.bag.deviceVT1 - scanParams_.bag.stepSize);
	}
      } else { //end if VT1 > stepsize, begin else

	for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	  (*chip)->setVThreshold1(0);
	}
      }// end else VT1 <stepsize
      
      for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
	scanParams_.bag.deviceVT1    = (*chip)->getVThreshold1();
	scanParams_.bag.deviceVT2    = (*chip)->getVThreshold2();
      }	
      
      confParams_.bag.triggersSeen = 0;

      //reset counters
      optohybridDevice_->resetL1ACount(0x1);
      optohybridDevice_->resetResyncCount();
      optohybridDevice_->resetBC0Count();
      optohybridDevice_->resetCalPulseCount(0x1);
      optohybridDevice_->sendResync();     
      optohybridDevice_->sendBC0();          
      
      for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {	
	(*chip)->setRunMode(1);
      }

      hw_semaphore_.give();
      wl_semaphore_.give();	
      return true;	
    }//else if VT2-VT1 < maxthreshold 
    else {
      hw_semaphore_.take();
      wl_->submit(stopSig_);      
      hw_semaphore_.give();
      wl_semaphore_.give();
	   
      return true; 
    }//end else
    
  }//end else triggerseen < N triggers
  
  return false; 
}//end run

//might be better done not as a workloop?
bool gem::supervisor::tbutils::ThresholdScan::readFIFO(toolbox::task::WorkLoop* wl)
{
  //VFATEvent vfat;
   
  gem::readout::GEMDataAMCformat::VFATData vfat;

  int ievent=0;

  wl_semaphore_.take();
  hw_semaphore_.take();

  //  std::string tmpFileName = confParams_.bag.outFileName.toString();

  // Get the size of GLIB data buffer       
  uint32_t bufferDepth;
  uint32_t TrigReg;

  if (readout_mask&0x1){ 
    bufferDepth  = glibDevice_->getFIFOOccupancy(0x0); 
  }
  if (readout_mask&0x2){ 
    bufferDepth = glibDevice_->getFIFOOccupancy(0x1);     
  }
  if (readout_mask&0x4){
    bufferDepth = glibDevice_->getFIFOOccupancy(0x2);     
  }
  
  //grab events from the fifo
  while (bufferDepth) {

    std::vector<uint32_t> data;
    std::queue<uint32_t> data_fifo;     

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
    double   delVT;

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
    }

    data1  = data10 | data11;
    data2  = data20 | data21;
    data3  = data30 | data31;
    data4  = data40 | data41;

    
    lsData = (data3 << 32) | (data4);
    msData = (data1 << 32) | (data2);

    delVT = (scanParams_.bag.deviceVT2-scanParams_.bag.deviceVT1);
    
    vfat.BC     = ( b1010 << 12 ) | (bcn);                // 1010     | bcn:12
    vfat.EC     = ( b1100 << 12 ) | (evn << 4) | (flags); // 1100     | EC:8      | Flag:4 (zero?)
    vfat.ChipID = ( b1110 << 12 ) | (chipid);             // 1110     | ChipID:12
    vfat.lsData = lsData;                                 // lsData:64
    vfat.msData = msData;                                 // msData:64
    vfat.BXfrOH = BX;                                     // BXfrOH:32
    vfat.crc    = crc;                                    // crc:16

    //print data package        
    gem::readout::GEMDataAMCformat::printVFATdataBits(ievent, vfat);

    if (!(((b1010 == 0xa) && (b1100==0xc) && (b1110==0xe)))){
      LOG4CPLUS_INFO(getApplicationLogger(),"VFAT headers do not match expectation");
      if (readout_mask&0x1)
	bufferDepth  = glibDevice_->getFIFOOccupancy(0x0);
      if (readout_mask&0x2)
	bufferDepth += glibDevice_->getFIFOOccupancy(0x1);
      if (readout_mask&0x4)
	bufferDepth += glibDevice_->getFIFOOccupancy(0x2);
      continue;
    }
     
    if (readout_mask&0x1)
      bufferDepth = glibDevice_->getFIFOOccupancy(0x0);
    if (readout_mask&0x2)
      bufferDepth = glibDevice_->getFIFOOccupancy(0x1);
    if (readout_mask&0x4)
      bufferDepth = glibDevice_->getFIFOOccupancy(0x2);  

    //Maybe add another histogramt that is a combined all channels histogram
    histo->Fill(delVT,(lsData||msData));

    //I think it would be nice to time this...
    for (int chan = 0; chan < 128; ++chan) {
      if (chan < 64)
	histos[chan]->Fill(delVT,((lsData>>chan))&0x1);
      else
	histos[chan]->Fill(delVT,((msData>>(chan-64)))&0x1);
    }
   
  } // end while buffer

  hw_semaphore_.give();

  std::string imgRoot = "${XDAQ_DOCUMENT_ROOT}/gemdaq/gemsupervisor/html/images/tbutils/tscan/";
  std::stringstream ss;
  ss << "chanthresh0.png";
  std::string imgName = ss.str();
  outputCanvas->cd();
  histo->Draw("ep0l");
  outputCanvas->Update();
  outputCanvas->SaveAs(TString(imgRoot+imgName));

  for (int chan = 0; chan < 128; ++chan) {
    imgRoot = "${XDAQ_DOCUMENT_ROOT}/gemdaq/gemsupervisor/html/images/tbutils/tscan/";
    ss.clear();
    ss.str(std::string());
    ss << "chanthresh" << (chan+1) << ".png";
    imgName = ss.str();
    outputCanvas->cd();
    histos[chan]->Draw("ep0l");
    outputCanvas->Update();
    outputCanvas->SaveAs(TString(imgRoot+imgName));
  }

  wl_semaphore_.give();

  return false;
}

void gem::supervisor::tbutils::ThresholdScan::scanParameters(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    std::string isReadonly = "";
    if (is_running_ || is_configured_)
      isReadonly = "readonly";
    *out << cgicc::span()   << std::endl
	 << cgicc::label("Latency").set("for","Latency") << std::endl
	 << cgicc::input().set("id","Latency").set(is_running_?"readonly":"").set("name","Latency")
      .set("type","number").set("min","0").set("max","255")
      .set("value",boost::str(boost::format("%d")%static_cast<unsigned>(scanParams_.bag.latency)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("MinThreshold").set("for","MinThreshold") << std::endl
	 << cgicc::input().set("id","MinThreshold").set(is_running_?"readonly":"").set("name","MinThreshold")
      .set("type","number").set("min","-255").set("max","255")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.minThresh)))
	 << std::endl

	 << cgicc::label("MaxThreshold").set("for","MaxThreshold") << std::endl
	 << cgicc::input().set("id","MaxThreshold").set(is_running_?"readonly":"").set("name","MaxThreshold")
      .set("type","number").set("min","-255").set("max","255")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.maxThresh)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("VStep").set("for","VStep") << std::endl
	 << cgicc::input().set("id","VStep").set(is_running_?"readonly":"").set("name","VStep")
      .set("type","number").set("min","1").set("max","255")
      .set("value",boost::str(boost::format("%d")%(scanParams_.bag.stepSize)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("VT1").set("for","VT1") << std::endl
	 << cgicc::input().set("id","VT1").set("name","VT1").set("readonly")
      .set("value",boost::str(boost::format("%d")%static_cast<unsigned>(scanParams_.bag.deviceVT1)))
	 << std::endl

	 << cgicc::label("VT2").set("for","VT2") << std::endl
	 << cgicc::input().set("id","VT2").set("name","VT2").set("readonly")
      .set("value",boost::str(boost::format("%d")%static_cast<unsigned>(scanParams_.bag.deviceVT2)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("NTrigsStep").set("for","NTrigsStep") << std::endl
	 << cgicc::input().set("id","NTrigsStep").set(is_running_?"readonly":"").set("name","NTrigsStep")
      .set("type","number").set("min","0")
      .set("value",boost::str(boost::format("%d")%(confParams_.bag.nTriggers)))
	 << cgicc::br() << std::endl
	 << cgicc::label("NTrigsSeen").set("for","NTrigsSeen") << std::endl
	 << cgicc::input().set("id","NTrigsSeen").set("name","NTrigsSeen")
      .set("type","number").set("min","0").set("readonly")
      .set("value",boost::str(boost::format("%d")%(confParams_.bag.triggersSeen)))
	 << cgicc::br() << std::endl
	 << cgicc::label("ADCVoltage").set("for","ADCVoltage") << std::endl
	 << cgicc::input().set("id","ADCVoltage").set("name","ADCVoltage")
      .set("type","number").set("min","0").set("readonly")
      .set("value",boost::str(boost::format("%d")%(confParams_.bag.ADCVoltage)))
	 << cgicc::label("ADCurrent").set("for","ADCurrent") << std::endl
	 << cgicc::input().set("id","ADCurrent").set("name","ADCurrent")
      .set("type","number").set("min","0").set("readonly")
      .set("value",boost::str(boost::format("%d")%(confParams_.bag.ADCurrent)))
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
      .set("type","number").set("min","0").set("max","128")
      .set("value",scanParams_.bag.currentHisto.toString())
	 << std::endl
	 << cgicc::br() << std::endl;
    *out << cgicc::input().set("class","button").set("type","button")
      .set("value","SelectChannel").set("name","DisplayHistogram")
      .set("onClick","changeImage(this.form)");
    *out << cgicc::td() << std::endl;

    *out << cgicc::td()  << std::endl
	 << cgicc::img().set("src","/gemdaq/gemsupervisor/html/images/tbutils/tscan/chanthresh"+scanParams_.bag.currentHisto.toString()+".png")
      .set("id","vfatChannelHisto")
	 << cgicc::td()    << std::endl;
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

// HyperDAQ interface
void gem::supervisor::tbutils::ThresholdScan::webDefault(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  //LOG4CPLUS_INFO(this->getApplicationLogger(),"gem::supervisor::tbutils::ThresholdScan::webDefaul");
  try {
    ////update the page refresh 
    if (!is_working_ && !is_running_) {
    }
    else if (is_working_) {
      cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
      head.addHeader("Refresh","60");
    }
    else if (is_running_) {
      cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
      head.addHeader("Refresh","20");
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

      selectOptohybridDevice(out);
      selectMultipleVFAT(out);      
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Start threshold scan.")
	.set("value", "Start") << std::endl;
      *out << cgicc::form()    << std::endl;
    }
    
    else if (is_running_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Stop") << std::endl;
      
      selectOptohybridDevice(out);
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


void gem::supervisor::tbutils::ThresholdScan::webConfigure(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {

  try {
    cgicc::Cgicc cgi(in);
    
    //aysen's xml parser
    confParams_.bag.settingsFile = cgi.getElement("xmlFilename")->getValue();
    
    cgicc::const_form_iterator element = cgi.getElement("Latency");
    if (element != cgi.getElements().end())
      scanParams_.bag.latency   = element->getIntegerValue();

    element = cgi.getElement("MinThreshold");
    if (element != cgi.getElements().end())
      scanParams_.bag.minThresh = element->getIntegerValue();
    
    element = cgi.getElement("MaxThreshold");
    if (element != cgi.getElements().end())
      scanParams_.bag.maxThresh = element->getIntegerValue();

    element = cgi.getElement("VStep");
    if (element != cgi.getElements().end())
      scanParams_.bag.stepSize  = element->getIntegerValue();
        
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

  try {
    cgicc::Cgicc cgi(in);
    
    cgicc::const_form_iterator element = cgi.getElement("Latency");
    if (element != cgi.getElements().end())
      scanParams_.bag.latency   = element->getIntegerValue();

    element = cgi.getElement("MinThreshold");
    if (element != cgi.getElements().end())
      scanParams_.bag.minThresh = element->getIntegerValue();
    
    element = cgi.getElement("MaxThreshold");
    if (element != cgi.getElements().end())
      scanParams_.bag.maxThresh = element->getIntegerValue();

    element = cgi.getElement("VStep");
    if (element != cgi.getElements().end())
      scanParams_.bag.stepSize  = element->getIntegerValue();
        
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
  
  wl_->submit(startSig_);
  
  redirect(in,out);
}

// State transitions
void gem::supervisor::tbutils::ThresholdScan::configureAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;

  latency_   = scanParams_.bag.latency;
  nTriggers_ = confParams_.bag.nTriggers;
  stepSize_  = scanParams_.bag.stepSize;
  minThresh_ = scanParams_.bag.minThresh;
  maxThresh_ = scanParams_.bag.maxThresh;
  
  hw_semaphore_.take();

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
    (*chip)->setDACMode(          0x0);
    (*chip)->setHitCountCycleTime(0x0); //maximum number of bits
    
    (*chip)->setHitCountMode( 0x0);
    (*chip)->setMSPulseLength(0x3);
    (*chip)->setInputPadMode( 0x0);
    (*chip)->setTrimDACRange( 0x0);
    (*chip)->setBandgapPad(   0x0);
    (*chip)->sendTestPattern( 0x0);
        
    (*chip)->setIPreampIn(  168);
    (*chip)->setIPreampFeed(150);
    (*chip)->setIPreampOut(  80);
    (*chip)->setIShaper(    150);
    (*chip)->setIShaperFeed(100);
    (*chip)->setIComp(      120);

    (*chip)->setLatency(latency_);
    //}
  
    (*chip)->setVThreshold1(maxThresh_-minThresh_);
    (*chip)->setVThreshold2(std::max(0,maxThresh_));
    scanParams_.bag.deviceVT1 = (*chip)->getVThreshold1();
    scanParams_.bag.deviceVT2 = (*chip)->getVThreshold2();

    scanParams_.bag.latency = (*chip)->getLatency();

  }

  is_configured_ = true;
  hw_semaphore_.give();

  if (histo) {
    delete histo;
    histo = 0;
  }
  std::stringstream histName, histTitle;
  histName  << "allchannels";
  histTitle << "Threshold scan for all channels";

  int minTh = scanParams_.bag.minThresh;
  int maxTh = scanParams_.bag.maxThresh;
  int nBins = ((maxTh - minTh) + 1)/(scanParams_.bag.stepSize);

  LOG4CPLUS_DEBUG(getApplicationLogger(),"histogram name and title: " << histName.str() 
		  << ", " << histTitle.str()
		  << "(" << nBins << " bins)");
  histo = new TH1F(histName.str().c_str(), histTitle.str().c_str(), nBins, minTh-0.5, maxTh+0.5);
  
  for (unsigned int hi = 0; hi < 128; ++hi) {
    if (histos[hi]) {
      delete histos[hi];
      histos[hi] = 0;
    }
    
    histName.clear();
    histName.str(std::string());
    histTitle.clear();
    histTitle.str(std::string());

    histName  << "channel"<<(hi+1);
    histTitle << "Threshold scan for channel "<<(hi+1);
    LOG4CPLUS_DEBUG(getApplicationLogger(),"histogram name and title: " << histName.str() 
		    << ", " << histTitle.str()
		    << "(" << nBins << " bins)");
    histos[hi] = new TH1F(histName.str().c_str(), histTitle.str().c_str(), nBins, minTh-0.5, maxTh+0.5);
  }
  outputCanvas = new TCanvas("outputCanvas","outputCanvas",600,800);

  is_working_    = false;
}


void gem::supervisor::tbutils::ThresholdScan::startAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
  
  is_working_ = true;

  //AppHeader ah;

  latency_   = scanParams_.bag.latency;
  nTriggers_ = confParams_.bag.nTriggers;
  stepSize_  = scanParams_.bag.stepSize;
  minThresh_ = scanParams_.bag.minThresh;
  maxThresh_ = scanParams_.bag.maxThresh;

  time_t now = time(0);
  tm *gmtm = gmtime(&now);
  char* utcTime = asctime(gmtm);

  std::string tmpFileName = "ThresholdScan_";
  tmpFileName.append(utcTime);
  tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
  tmpFileName.append(".dat");
  std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
  std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');

  confParams_.bag.outFileName = tmpFileName;

  LOG4CPLUS_INFO(getApplicationLogger(),"Creating file " << confParams_.bag.outFileName.toString());

  std::ofstream scanStream(tmpFileName.c_str(), std::ios::app | std::ios::binary);

  if (scanStream.is_open()){
    LOG4CPLUS_INFO(getApplicationLogger(),"::startAction " 
		   << "file " << confParams_.bag.outFileName.toString() << " opened");
  }

  // Setup Scan file, information header
  tmpFileName = "ScanSetup_";
  tmpFileName.append(utcTime);
  tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
  tmpFileName.append(".txt");
  std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
  std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');
  confParams_.bag.outFileName = tmpFileName;

  LOG4CPLUS_DEBUG(getApplicationLogger(),"::startAction " 
		  << "Created ScanSetup file " << tmpFileName );

  std::ofstream scanSetup(tmpFileName.c_str(), std::ios::app );
  if (scanSetup.is_open()){
    LOG4CPLUS_INFO(getApplicationLogger(),"::startAction " 
		   << "file " << tmpFileName << " opened and closed");

    scanSetup << "\n The Time & Date : " << utcTime << std::endl;
    scanSetup << " ChipID        0x" << std::hex << confParams_.bag.deviceChipID << std::dec << std::endl;
    scanSetup << " Latency       " << latency_ << std::endl;
    scanSetup << " nTriggers     " << nTriggers_  << std::endl;
    scanSetup << " stepSize      " << stepSize_ << std::endl;
    scanSetup << " minThresh     " << minThresh_ << std::endl;
    scanSetup << " maxThresh     " << maxThresh_ << std::endl;
  }
  scanSetup.close();
  
  //char data[128/8]
  is_running_ = true;
  hw_semaphore_.take();

  //reset counters
  optohybridDevice_->resetL1ACount(0x1);
  optohybridDevice_->resetResyncCount();
  optohybridDevice_->resetBC0Count();
  optohybridDevice_->resetCalPulseCount(0x1);
  optohybridDevice_->sendResync();      
  optohybridDevice_->sendBC0();          

  
  //set trigger source
  optohybridDevice_->setTrigSource(0x4);// All trigger sources   

  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
    (*chip)->setVThreshold1(maxThresh_-minThresh_);
    (*chip)->setVThreshold2(std::max(0,maxThresh_));
    scanParams_.bag.deviceVT1 = (*chip)->getVThreshold1();
    scanParams_.bag.deviceVT2 = (*chip)->getVThreshold2();
    
    scanParams_.bag.latency = (*chip)->getLatency();
  }

  //start readout
  scanStream.close();

  if (histo) {
    delete histo;
    histo = 0;
  }
  std::stringstream histName, histTitle;
  histName  << "allchannels";
  histTitle << "Threshold scan for all channels";
  int minTh = scanParams_.bag.minThresh;
  int maxTh = scanParams_.bag.maxThresh;
  int nBins = ((maxTh - minTh) + 1)/(scanParams_.bag.stepSize);

  /*
  //write Applicatie  header
  ah.minTh = minTh;
  ah.maxTh = maxTh;
  ah.stepSize = scanParams_.bag.stepSize;
  keepAppHeader(tmpFileName, ah);
  */

  histo = new TH1F(histName.str().c_str(), histTitle.str().c_str(), nBins, minTh-0.5, maxTh+0.5);
  
  for (unsigned int hi = 0; hi < 128; ++hi) {
    LOG4CPLUS_INFO(getApplicationLogger(),"histos[" << hi << "] = 0x" << std::hex << histos[hi] << std::dec);
    if (histos[hi]) {
      delete histos[hi];
      histos[hi] = 0;
    }
    
    histName.clear();
    histName.str(std::string());
    histTitle.clear();
    histTitle.str(std::string());

    histName  << "channel"<<(hi+1);
    histTitle << "Threshold scan for channel "<<(hi+1);
    histos[hi] = new TH1F(histName.str().c_str(), histTitle.str().c_str(), nBins, minTh-0.5, maxTh+0.5);
  }

  //flush fifo
  for (int i = 0; i < 2; ++i){
    if ((readout_mask >> i)&0x1) {
      glibDevice_->flushFIFO(i);
      while (glibDevice_->hasTrackingData(i)){
	glibDevice_->flushFIFO(i);
	INFO( " has data" << i << " (depth " << glibDevice_->getFIFOOccupancy(i));
	std::vector<uint32_t> dumping = glibDevice_->getTrackingData(i, glibDevice_->getFIFOOccupancy(i));
      }
      
    }
  }

  optohybridDevice_->sendResync();      
  optohybridDevice_->sendBC0();          

  for (auto chip = vfatDevice_.begin(); chip != vfatDevice_.end(); ++chip) {
    (*chip)->setRunMode(1);
  }

  hw_semaphore_.give();
  //start scan routine
  wl_->submit(runSig_);
  
  is_working_ = false;
}


void gem::supervisor::tbutils::ThresholdScan::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  gem::supervisor::tbutils::GEMTBUtil::resetAction(e);
  
  scanParams_.bag.latency   = 12U;
  scanParams_.bag.minThresh = -80;
  scanParams_.bag.maxThresh = 20;
  scanParams_.bag.stepSize  = 5U;
  scanParams_.bag.deviceVT1 = 0x0;
  scanParams_.bag.deviceVT2 = 0x0;
  
  is_working_     = false;
}

