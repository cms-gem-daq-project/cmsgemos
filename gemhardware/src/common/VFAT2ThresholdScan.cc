#include "gem/hw/vfat/VFAT2ThresholdScan.h"
#include "gem/hw/vfat/HwVFAT2.h"

XDAQ_INSTANTIATOR_IMPL(gem::hw::vfat::VFAT2ThresholdScan)

gem::hw::vfat::VFAT2ThresholdScan::VFAT2ThresholdScan(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception):
xdaq::WebApplication(s)
{
  xgi::framework::deferredbind(this, this, &VFAT2ThresholdScan::Default,       "Default"     );
  xgi::framework::deferredbind(this, this, &VFAT2ThresholdScan::RegisterView,  "RegisterView");
  xgi::framework::deferredbind(this, this, &VFAT2ThresholdScan::ControlPanel,  "ControlPanel");
  xgi::framework::deferredbind(this, this, &VFAT2ThresholdScan::ExpertView,    "ExpertView"  );
  xgi::framework::deferredbind(this, this, &VFAT2ThresholdScan::Peek,          "Peek"        ); 
  xgi::framework::deferredbind(this, this, &VFAT2ThresholdScan::controlVFAT2,  "controlVFAT2");
  device_ = "CMS_hybrid_J8";
  settingsFile_ = "";

  // Detect when the setting of default parameters has been performed
  this->getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");

  getApplicationInfoSpace()->fireItemAvailable("device", &device_);
  getApplicationInfoSpace()->fireItemAvailable("settingsFile", &settingsFile_);

  getApplicationInfoSpace()->fireItemValueRetrieve("device", &device_);
  getApplicationInfoSpace()->fireItemValueRetrieve("settingsFile", &settingsFile_);

}

gem::hw::vfat::VFAT2ThresholdScan::~VFAT2ThresholdScan()
{
  vfatDevice->releaseDevice();
}

void gem::hw::vfat::VFAT2ThresholdScan::actionPerformed(xdata::Event& event)
{
  // This is called after all default configuration values have been
  // loaded (from the XDAQ configuration file).
  if (event.type() == "urn:xdaq-event:setDefaultValues") {
      std::stringstream ss;
      ss << "device_=[" << device_.toString() << "]" << std::endl;
      ss << "settingsFile_=[" << settingsFile_.toString() << "]" << std::endl;
      LOG4CPLUS_DEBUG(getApplicationLogger(), "VFAT2ThresholdScan::actionPerformed() Default configuration values have been loaded");
      LOG4CPLUS_INFO(this->getApplicationLogger(), ss.str());
      //LOG4CPLUS_DEBUG(getApplicationLogger(), "VFAT2ThresholdScan::actionPerformed()   --> starting monitoring");
      //monitorP_->startMonitoring();
    }
  //Initialize the HW device, should have picked up the device string from the xml file by now
  LOG4CPLUS_INFO(this->getApplicationLogger(),"VFAT2ThresholdScan::VFAT2ThresholdScan::4 device_ = " << device_.toString() << std::endl);
  vfatDevice = new HwVFAT2(this, device_.toString());
  vfatDevice->connectDevice();
  setLogLevelTo(uhal::Error());  // Maximise uHAL logging
  LOG4CPLUS_INFO(this->getApplicationLogger(),"VFAT2ThresholdScan::VFAT2ThresholdScan::5 device_ = " << device_.toString() << std::endl);

  //initialize the vfatParameters struct
  //readVFAT2Registers(vfatParams);
  vfatDevice->getAllSettings();

  LOG4CPLUS_INFO(this->getApplicationLogger(),"vfatParams:" << std::endl 
    << vfatDevice->getVFAT2Params() << std::endl);

  //readVFAT2Registers();
  vfatParams_ = vfatDevice->getVFAT2Params();

  scanParams_.thresholdLatency   = 128U;
  scanParams_.thresholdMin       = -255;
  scanParams_.thresholdMax       = 0;
  scanParams_.thresholdStep      = 1U;
  scanParams_.thresholdNTriggers = 10000U;
  scanParams_.thresholdVTH1      = vfatParams_.vThresh1;
  scanParams_.thresholdVTH2      = vfatParams_.vThresh2;

  LOG4CPLUS_INFO(this->getApplicationLogger(),"VFAT2ThresholdScan::VFAT2ThresholdScan::6 device_ = " << device_.toString() << std::endl);

}

void gem::hw::vfat::VFAT2ThresholdScan::readVFAT2Registers(gem::hw::vfat::VFAT2ControlParams& params)
{
  //boost::format vfatregform("user_regs.vfats.%1%.%2%");
  boost::format vfatregform("user_regs.vfats.%1%.%2%");
  
  //try the hardware connection
  try {
    uhal::HwInterface hw = vfatDevice->getVFAT2HwInterface();
    std::string deviceBaseNode = vfatDevice->getDeviceBaseNode();
    //this conflicts with setting the base node in the HwVFAT device, need to reset
    //vfatDevice->setDeviceBaseNode("");
    //this doesn't fix the problem as HwVFATw or GEMHwDevice needs to be smarter
    //better to strip away the search string from these node names
    //std::vector<std::string> nodes_ = hw.getNodes(deviceBaseNode+".*");
    nodes_ = hw.getNodes(deviceBaseNode+".*");
    nodes_.erase( std::remove(nodes_.begin(),
			     nodes_.end(),
			     deviceBaseNode),
		 nodes_.end() );
    
    std::vector<std::string>::iterator cleannode  = nodes_.begin();
    LOG4CPLUS_INFO(this->getApplicationLogger(),"before removal::getNodes(): " << std::endl
		   << "nodes_.size() = " << nodes_.size() << std::endl);
    std::copy(nodes_.begin(),nodes_.end(),std::ostream_iterator<std::string>(std::cout,", "));
    
    for (; cleannode != nodes_.end(); ++cleannode) {
      cleannode->erase(cleannode->find(deviceBaseNode+"."),
		       std::string(deviceBaseNode+".").length());
    }
    
    std::vector<std::string>::const_iterator node = nodes_.begin();
    LOG4CPLUS_INFO(this->getApplicationLogger(),"after removal::getNodes(): " << std::endl
		   << "nodes_.size() = " << nodes_.size() << std::endl);
    std::copy(nodes_.begin(),nodes_.end(),std::ostream_iterator<std::string>(std::cout,", "));
    
    node = nodes_.begin();
    LOG4CPLUS_INFO(this->getApplicationLogger(), "attempting to read the vfat registers");
    LOG4CPLUS_INFO(this->getApplicationLogger(),"start reading::getNodes(): " << std::endl
		   << "nodes_.size() = " << nodes_.size() << std::endl);
    
    for (; node != nodes_.end(); ++node) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"reading::getNodes(): " << std::endl
		     << "nodes_.size() = " << nodes_.size() << std::endl);
      LOG4CPLUS_INFO(this->getApplicationLogger(), "attempting to read register " << (*node));
      vfatFullRegs_[*node] = vfatDevice->readReg(*node);
      vfatRegs_[*node]     = vfatDevice->readVFATReg(*node);
      std::cout << *node << "  0x" << std::hex << vfatFullRegs_[*node] << std::dec << std::endl;
      std::cout << *node << "  0x" << std::hex << static_cast<unsigned>(vfatRegs_[*node])     << std::dec << std::endl;
    }
    LOG4CPLUS_INFO(this->getApplicationLogger(),"done reading::getNodes(): " << std::endl
		   << "nodes_.size() = " << nodes_.size() << std::endl);
    
    LOG4CPLUS_INFO(getApplicationLogger(), "VFAT2ThresholdScan::readVFAT2Registers() --> read in current parameters");
    //std::string key = boost::str(vfatregform % device_.toString() % "Latency");
    //key = "Latency";
    params.latency = static_cast<unsigned>(vfatRegs_["Latency"]);
    
    params.iPreampIn   = static_cast<unsigned>(vfatRegs_["IPreampIn"]);
    params.iPreampFeed = static_cast<unsigned>(vfatRegs_["IPreampFeed"]);
    params.iPreampOut  = static_cast<unsigned>(vfatRegs_["IPreampOut"]);
    params.iShaper     = static_cast<unsigned>(vfatRegs_["IShaper"]);
    params.iShaperFeed = static_cast<unsigned>(vfatRegs_["IShaperFeed"]);
    params.iComp       = static_cast<unsigned>(vfatRegs_["IComp"]);
    
    params.vCal     = static_cast<unsigned>(vfatRegs_["VCal"]);
    params.vThresh1 = static_cast<unsigned>(vfatRegs_["VThreshold1"]);
    params.vThresh2 = static_cast<unsigned>(vfatRegs_["VThreshold2"]);
    params.calPhase = static_cast<unsigned>(vfatRegs_["CalPhase"]);

    //counters
    
    //uint16_t myChipID;
    //myChipID = (vfatRegs_["ChipID1"]<<8)|vfatRegs_["ChipID0"];
    params.chipID = (vfatRegs_["ChipID1"]<<8)|vfatRegs_["ChipID0"];
    std::cout << "ChipID0 = 0x" << std::hex << static_cast<unsigned>(vfatRegs_["ChipID0"]) << std::dec << std::endl;
    std::cout << "ChipID1 = 0x" << std::hex << static_cast<unsigned>(vfatRegs_["ChipID1"]) << std::dec << std::endl;
    std::cout << "ChipID = 0x" << std::hex << params.chipID << std::dec << std::endl;
    
    //uint8_t myUpset;
    params.upsetCounter = static_cast<unsigned>(vfatRegs_["UpsetReg"]);
    
    //uint32_t myCounter;
    params.hitCounter = (vfatRegs_["HitCount2"]<<16)|
      (vfatRegs_["HitCount1"]<<8)|
      vfatRegs_["HitCount0"];
    
    //uint8_t control0, control1, control2, control3;
    
    LOG4CPLUS_INFO(getApplicationLogger(), "VFAT2ThresholdScan::readVFAT2Registers() --> read in control registers");
    params.control0 = static_cast<unsigned>(vfatRegs_["ContReg0"]);
    params.control1 = static_cast<unsigned>(vfatRegs_["ContReg1"]);
    params.control2 = static_cast<unsigned>(vfatRegs_["ContReg2"]);
    params.control3 = static_cast<unsigned>(vfatRegs_["ContReg3"]);
    
    params.runMode   = (VFAT2RunMode  )((params.control0&VFAT2ContRegBitMasks::RUNMODE )>>VFAT2ContRegBitShifts::RUNMODE );
    params.trigMode  = (VFAT2TrigMode )((params.control0&VFAT2ContRegBitMasks::TRIGMODE)>>VFAT2ContRegBitShifts::TRIGMODE);
    params.msPol     = (VFAT2MSPol    )((params.control0&VFAT2ContRegBitMasks::MSPOL   )>>VFAT2ContRegBitShifts::MSPOL   );
    params.calPol    = (VFAT2CalPol   )((params.control0&VFAT2ContRegBitMasks::CALPOL  )>>VFAT2ContRegBitShifts::CALPOL  );
    params.calibMode = (VFAT2CalibMode)((params.control0&VFAT2ContRegBitMasks::CALMODE )>>VFAT2ContRegBitShifts::CALMODE );
    
    params.dacMode   = (VFAT2DACMode  )((params.control1&VFAT2ContRegBitMasks::DACMODE  )>>VFAT2ContRegBitShifts::DACMODE  );
    params.probeMode = (VFAT2ProbeMode)((params.control1&VFAT2ContRegBitMasks::PROBEMODE)>>VFAT2ContRegBitShifts::PROBEMODE);
    params.lvdsMode  = (VFAT2LVDSMode )((params.control1&VFAT2ContRegBitMasks::LVDSMODE )>>VFAT2ContRegBitShifts::LVDSMODE );
    params.reHitCT   = (VFAT2ReHitCT  )((params.control1&VFAT2ContRegBitMasks::REHITCT  )>>VFAT2ContRegBitShifts::REHITCT  );
    
    params.hitCountMode = (VFAT2HitCountMode )((params.control2&VFAT2ContRegBitMasks::HITCOUNTMODE )>>VFAT2ContRegBitShifts::HITCOUNTMODE );
    params.msPulseLen   = (VFAT2MSPulseLength)((params.control2&VFAT2ContRegBitMasks::MSPULSELENGTH)>>VFAT2ContRegBitShifts::MSPULSELENGTH);
    params.digInSel     = (VFAT2DigInSel     )((params.control2&VFAT2ContRegBitMasks::DIGINSEL     )>>VFAT2ContRegBitShifts::DIGINSEL     );
    
    params.trimDACRange    = (VFAT2TrimDACRange )((params.control3&VFAT2ContRegBitMasks::TRIMDACRANGE)>>VFAT2ContRegBitShifts::TRIMDACRANGE);
    params.padBandGap      = (VFAT2PadBandgap   )((params.control3&VFAT2ContRegBitMasks::PADBANDGAP  )>>VFAT2ContRegBitShifts::PADBANDGAP  );
    params.sendTestPattern = (VFAT2DFTestPattern)((params.control3&VFAT2ContRegBitMasks::DFTESTMODE  )>>VFAT2ContRegBitShifts::DFTESTMODE  );
    
    LOG4CPLUS_INFO(getApplicationLogger(), "VFAT2ThresholdScan::readVFAT2Registers() --> read in current parameters");
    LOG4CPLUS_INFO(getApplicationLogger(), "VFAT2ThresholdScan::readVFAT2Registers()::Control Register 0");
    LOG4CPLUS_INFO(getApplicationLogger(), boost::str(boost::format("0x%02x <%s:%s:%s:%s:%s>")
						      % static_cast<unsigned>(params.control0)
						      % gem::hw::vfat::RunModeToString.at(        params.runMode  )
						      % gem::hw::vfat::TriggerModeToString.at(    params.trigMode )
						      % gem::hw::vfat::MSPolarityToString.at(     params.msPol    )
						      % gem::hw::vfat::CalPolarityToString.at(    params.calPol   )
						      % gem::hw::vfat::CalibrationModeToString.at(params.calibMode)));
    
    LOG4CPLUS_INFO(getApplicationLogger(), "VFAT2ThresholdScan::readVFAT2Registers()::Control Register 1");
    LOG4CPLUS_INFO(getApplicationLogger(), boost::str(boost::format("0x%02x <%s:%s:%s:%s>")
						      % static_cast<unsigned>(params.control1)
						      % gem::hw::vfat::DACModeToString.at(      params.dacMode   )
						      % gem::hw::vfat::ProbeModeToString.at(    params.probeMode )
						      % gem::hw::vfat::LVDSPowerSaveToString.at(params.lvdsMode  )
						      % gem::hw::vfat::ReHitCTToString.at(      params.reHitCT   )));
    
    LOG4CPLUS_INFO(getApplicationLogger(), "VFAT2ThresholdScan::readVFAT2Registers()::Control Register 2");
    LOG4CPLUS_INFO(getApplicationLogger(), boost::str(boost::format("0x%02x <%s:%s:%s>")
						      %static_cast<unsigned>(params.control2)
						      % gem::hw::vfat::HitCountModeToString.at( params.hitCountMode)
						      % gem::hw::vfat::MSPulseLengthToString.at(params.msPulseLen  )
						      % gem::hw::vfat::DigInSelToString.at(     params.digInSel    )));
    
    LOG4CPLUS_INFO(getApplicationLogger(), "VFAT2ThresholdScan::readVFAT2Registers()::Control Register 3");
    LOG4CPLUS_INFO(getApplicationLogger(), boost::str(boost::format("0x%02x <%s:%s:%s>")
						      % static_cast<unsigned>(params.control3)
						      % gem::hw::vfat::TrimDACRangeToString.at( params.trimDACRange    )
						      % gem::hw::vfat::PbBGToString.at(         params.padBandGap      )
						      % gem::hw::vfat::DFTestPatternToString.at(params.sendTestPattern )));
  }
  catch (uhal::exception::exception const& err) {
    std::string msg =
      toolbox::toString("unable to access VFAT2 hardware %s",(device_.toString()).c_str());
    //remove fatals//LOG4CPLUS_FATAL(this->getApplicationLogger(),msg);
    XCEPT_RAISE(gem::hw::vfat::exception::VFATHwProblem, msg);
  }
  catch (std::exception const& err) {
    std::string msg =
      toolbox::toString("unable to access VFAT2 hardware %s",(device_.toString()).c_str());
    //remove fatals//LOG4CPLUS_FATAL(this->getApplicationLogger(),msg);
    XCEPT_RAISE(gem::hw::vfat::exception::VFATHwProblem, msg);
  }
  LOG4CPLUS_INFO(this->getApplicationLogger(),"ending constructor::getNodes(): " << std::endl
		 << "nodes_.size() = " << nodes_.size() << std::endl);
}

void gem::hw::vfat::VFAT2ThresholdScan::Default(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  this->ControlPanel(in,out);
  //this->RegisterView(in,out);
}

void gem::hw::vfat::VFAT2ThresholdScan::ControlPanel(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  
  try {
    //need to grab the page header and modify it here somehow...
    //gem::hw::vfat::VFAT2ThresholdScan::createHeader(out);
    //*out << "  <body>" << std::endl;
    //cgicc::HTTPResponseHeader& head = out->getHTTPResponseHeader();
    //head->addHeader();

    //head = out["head"]
    *out << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatmanager.css\"/>"          << std::endl
	 << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatcontrolregisters.css\"/>" << std::endl
	 << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatglobalsettings.css\"/>"   << std::endl
	 << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatchannelregister.css\"/>"  << std::endl
	 << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatcommands.css\"/>"         << std::endl;

    
    LOG4CPLUS_INFO(this->getApplicationLogger(),"building the control panel vfatParams_:" << vfatParams_);
    LOG4CPLUS_INFO(this->getApplicationLogger(),"vfatDevice->getVFAT2Params():" << vfatDevice->getVFAT2Params());
    std::string method = toolbox::toString("/%s/controlVFAT2",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","POST")
                         .set("action",method)
                         //.set("action","")
                         //.set("onsubmit","return false;")
	 << std::endl;
    /*
      cgicc::section *leftSide = new cgicc::section();
      leftSide->set("style","display:inline-block;float:left");
      cgicc::fieldset *vfatParametersFieldset = new cgicc::fieldset();
      vfatParametersFieldset->set("class","vfatParameters");
      vfatParametersFieldset->add(cgicc::legend("VFAT2 Parameters"));
      vfatParametersFieldset->add(cgicc::span().set("style","display:block;float:left"));
      leftSide->add(vfatParametersFieldset);
      *out << *leftSide << std::endl;
      */
    *out << cgicc::section().set("style","display:inline-block;float:left")       << std::endl
	 << cgicc::fieldset().set("class","vfatParameters")       << std::endl
    	 << cgicc::legend("VFAT2 Parameters")                     << std::endl
    	 << cgicc::span().set("style","display:block;float:left") << std::endl;

    LOG4CPLUS_INFO(this->getApplicationLogger(),"building the VFATInfoLayout");
    gem::hw::vfat::VFAT2ThresholdScan::VFAT2ThresholdScanWeb::createVFATInfoLayout(out, vfatParams_);
    *out << cgicc::span()     << std::endl;

    *out << std::endl;
    *out << cgicc::span().set("style","display:block;float:right") << std::endl;
    LOG4CPLUS_INFO(this->getApplicationLogger(),"building the CounterLayout");
    gem::hw::vfat::VFAT2ThresholdScan::VFAT2ThresholdScanWeb::createCounterLayout(out, scanParams_);

    *out << cgicc::span()     << std::endl;
    *out << cgicc::comment() << "ending the VFAT2 Parameters fieldset" << cgicc::comment() << std::endl
	 << cgicc::fieldset() << std::endl;
    *out << std::endl;

    LOG4CPLUS_INFO(this->getApplicationLogger(),"building the CommandLayout");
    gem::hw::vfat::VFAT2ThresholdScan::VFAT2ThresholdScanWeb::createCommandLayout(out, vfatParams_);
    
    *out << cgicc::section() << std::endl
	 << "Bad headers:: " << (vfatDevice->ipBusErrs).badHeader_     << cgicc::br() << std::endl
	 << "Read errors:: " << (vfatDevice->ipBusErrs).readError_     << cgicc::br() << std::endl
	 << "Timeouts   :: " << (vfatDevice->ipBusErrs).timeouts_      << cgicc::br() << std::endl
         //SB << "CH errors  :: " << (vfatDevice->ipBusErrs).controlHubErr_ << cgicc::br() << std::endl
	 << cgicc::section() << std::endl;
    
    *out << cgicc::form() << cgicc::br() << std::endl;
    *out << cgicc::script().set("type","text/javascript")
                           .set("src","http://ajax.googleapis.com/ajax/libs/jquery/1/jquery.min.js")
	 << cgicc::script() << std::endl;
    *out << cgicc::script().set("type","text/javascript")
                           .set("src","http://ajax.googleapis.com/ajax/libs/jqueryui/1/jquery-ui.min.js")
	 << cgicc::script() << std::endl;

    *out << cgicc::script().set("type","text/javascript")
                           .set("src","/gemdaq/gemhardware/html/scripts/toggleVFATCheckboxes.js")
	 << cgicc::script() << cgicc::br()
	 << std::endl;

    /*
    **load here all the scripts to jquery/ajax the forms/values/files**
    *out << "    <script type=\"text/javascript\">" << std::endl
	 << "      $(document).ready(function()"    << std::endl
	 << "        {" << std::endl
	 << "          alert(\"JQuery is working! :D\");" << std::endl
	 << "        });"   << std::endl
	 << "    </script>" << std::endl;
    *out << std::endl; 
    *out << std::endl; 
    *out << "    <script type=\"text/javascript\">" << std::endl
	 << "    $(document).ready( function () {"  << std::endl
	 << "    	var registerSetBoxes = [$(\"#CR0Set\"),$(\"#CR1Set\"),$(\"#CR2Set\"),$(\"#CR3Set\"),"       << std::endl
	 << "    				$(\"#SetIPreampIn\"),$(\"#SetIPreampFeed\"),$(\"#SetIPreampOut\")," << std::endl
	 << "    				$(\"#SetIShaper\"),$(\"#SetIShaperFeed\"),"                         << std::endl
	 << "    				$(\"#SetIComp\"),"   << std::endl
	 << "    				$(\"#SetLatency\")," << std::endl
	 << "    				$(\"#SetVCal\"),"    << std::endl
	 << "    				$(\"#SetVThreshold1\"),$(\"#SetVThreshold2\")," << std::endl
	 << "    				$(\"#SetCalPhase\")" << std::endl
	 << "    				];"                  << std::endl
	 << "    	var nBoxes = registerSetBoxes.length;"       << std::endl
	 << "    	$(\"#SelectAll\").click(function(event) {"   << std::endl
	 << "                   alert(\"Select all checkboxes\");"   << std::endl
	 << "    		for (var i=0; i<nBoxes; ++i ) {"     << std::endl
	 << "    		    registerSetBoxes[i].get(0).checked = true;" << std::endl
	 << "    		};" << std::endl
	 << "    	    });"    << std::endl
	 << "    	$(\"#SelectNone\").click(function(event) {"  << std::endl
	 << "                   alert(\"Deselect all checkboxes\");" << std::endl
	 << "    		for (var i=0; i<nBoxes; ++i ) {"     << std::endl
	 << "    		    registerSetBoxes[i].get(0).checked = false;" << std::endl
	 << "    		};" << std::endl
	 << "    	    });"    << std::endl
	 << "        });"           << std::endl
	 << "    </script>"         << std::endl;
    *out << std::endl; 
    */
    // *out << "  </body>" << std::endl;
    // *out << "</html>" << std::endl;
    
  }
  
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying ControlPanel xgi: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying the ControlPanel: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

void gem::hw::vfat::VFAT2ThresholdScan::ExpertView(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
}


void gem::hw::vfat::VFAT2ThresholdScan::RegisterView(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  //std::string method = toolbox::toString("/%s/writeUserRegs",getApplicationDescriptor()->getURN().c_str());
  
  //try the cgicc display
  try {
    *out << "<div class=\"xdaq-tab-wrapper\">"                      << std::endl;
    
    *out << "  <div class=\"xdaq-tab\" title=\"VFAT2 Registers\" >" << std::endl;
    
    *out << cgicc::fieldset().set("style","font-size: 10pt; font-family: arial;") << std::endl;
    //*out << cgicc::legend(        "Read/Write VFAT user registers"              ) << cgicc::p() << std::endl;
    //*out << cgicc::form().set(    "method","GET").set("action", method)           << "<br />" << std::endl;
    *out << "    <table class=\"xdaq-table\" caption=\"VFAT2 Registers\" >"       << std::endl;
    *out << "      <thead>" << std::endl;
    *out << "        <tr>"  << std::endl;
    *out << "          <th class=\"xdaq-sortable\">VFAT2 Register</th>"  << std::endl;
    *out << "          <th class=\"xdaq-sortable\">Read IPBus data</th>"      << std::endl;
    *out << "          <th class=\"xdaq-sortable\">Read VFAT data</th>"      << std::endl;
    //*out << "          <th class=\"xdaq-sortable\">Value to write</th>"  << std::endl;
    //*out << "          <th class=\"xdaq-sortable\">Execute</th>"         << std::endl;
    *out << "        </tr>"  << std::endl;
    *out << "      </thead>" << std::endl;
    *out << "      <br />"    << std::endl;
    
    //set up the rest by way of a loop
    *out << "      <tbody>"             << std::endl;
    
    std::vector<std::string>::const_iterator node  = nodes_.begin();
    LOG4CPLUS_INFO(this->getApplicationLogger(), "nodes_.size() = " << nodes_.size());
    for (; node != nodes_.end(); ++node) {
      *out << "      <tr>"     << std::endl;
      *out << "        <td>"   << *node << "</td>"  << std::endl;
      *out << "        <td>0x" << std::setfill('0') << std::setw(8)
	   << std::hex << vfatFullRegs_[*node]
	   << std::dec << "</td>"                   << std::endl;
      *out << "        <td>0x" << std::setfill('0') << std::setw(2)
	   << std::hex << vfatRegs_[*node]
	   << std::dec << "</td>"                   << std::endl;
      //*out << "        <td>" << cgicc::input().set("type","text"
      //						   ).set("name",boost::str(uregform%ureg)
      //							 ).set("value", boost::str(boost::format("0x%08x")%xreg_users_.at(ureg))
      //							       ).set("size","10").set("maxlength","32")
      //     << "        </td>" << std::endl;
      *out << "      </tr>" << std::endl;
    }
    
    
    *out << "      <br />"    << std::endl;
    *out << "      </tbody>" << std::endl;
    *out << "    </table>"   << std::endl;
    *out << "    <br />"      << std::endl;
    *out << "  </div>"       << std::endl;
    *out << "</div>"         << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying xgi: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying the registers: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

void gem::hw::vfat::VFAT2ThresholdScan::Peek(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  
}

void gem::hw::vfat::VFAT2ThresholdScan::controlVFAT2(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  try
    {
      cgicc::Cgicc cgi(in);
      std::vector<cgicc::FormEntry> myElements = cgi.getElements();
      
      //for (unsigned int formiter = 0; formiter < myElements.size(); ++formiter) {
      //	LOG4CPLUS_INFO(this->getApplicationLogger(),"form element name: "  + myElements.at(formiter).getName() );
      //	LOG4CPLUS_INFO(this->getApplicationLogger(),"form element value: " + myElements.at(formiter).getValue());
      //}
      // read a list of the checked checkboxes
      std::vector<std::pair<std::string,uint8_t> > regValsToSet;
      getCheckedRegisters(cgi, regValsToSet);

      //now process the form based on the button clicked
      performAction(cgi, regValsToSet);
      this->Default(in,out);
    }
  catch (const xgi::exception::Exception & e)
    {
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
  catch (const std::exception & e)
    {
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
}

void gem::hw::vfat::VFAT2ThresholdScan::getCheckedRegisters(cgicc::Cgicc cgi, std::vector<std::pair<std::string,uint8_t> > &regValsToSet)
  throw (xgi::exception::Exception)
{
  /****
   ***get a list of the following registers to set***
   
   *Address table register - checkbox name - corresponding form inputs to grab

   ContReg0 - CR0Set - CalMode:CalPolarity:MSPolarity:TriggerMode:RunMode
   ContReg1 - CR1Set - ReHitCT:LVDSPowerSave:ProbeMode:DACMode
   ContReg2 - CR2Set - DigInSel:MSPulseLength:HitCountMode
   ContReg3 - CR3Set - DFTest:PbBG:TrimDACRange
   
   IPreampIn   - SetIPreampIn    - IPreampIn   
   IPreampFeed - SetIPreampFeed  - IPreampFeed 
   IPreampOut  - SetIPreampOut   - IPreampOut  
   IShaper     - SetIShaper      - IShaper     
   IShpaerFeed - SetIShpaerFeed  - IShpaerFeed 
   IComp       - SetIComp        - IComp       

   Latency     - SetLatency      - Latency     
   VCal        - SetVCal         - VCal        
   VThreshold1 - SetVThreshold1  - VThreshold1 
   VThreshold2 - SetVThreshold2  - VThreshold2 
   
   CalPhase - SetCalPhase - CalPhase

   //if all selected, pass the params struct to the setter?
   
   ***Readonly***
   ChipID0
   ChipID1

   UpsetReg

   HitCount0
   HitCount1
   HitCount2
   **/

  //std::vector<std::pair<std::string,uint8_t> > regValsToSet;
  std::pair<std::string,uint8_t> regPair;
  LOG4CPLUS_INFO(this->getApplicationLogger(), "getCheckedRegisters::Getting list of checked registers and values to read/write");
  try
    {
      std::vector<cgicc::FormEntry> vfat2FormEntries = cgi.getElements();
 
        uint8_t oldThresholdLatency = vfatDevice->getLatency();
	uint8_t newThresholdLatency = cgi["ThresholdLatency"     ]->getIntegerValue();
        LOG4CPLUS_INFO(getApplicationLogger(), "Setting the ThresholdLatency to :"<<std::dec<<static_cast<int>(oldThresholdLatency)<<std::dec);
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the ThresholdLatency to :"<<std::dec<<static_cast<int>(newThresholdLatency)<<std::dec);
	vfatDevice->setLatency(newThresholdLatency);

	int newInitialThreshold = cgi["InitialThreshold"     ]->getIntegerValue();
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the InitialThreshold to :"<<std::dec<<static_cast<int>(newInitialThreshold)<<std::dec);
	
	int newFinalThreshold = cgi["FinalThreshold"     ]->getIntegerValue();
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the FinalThreshold to :"<<std::dec<<static_cast<int>(newFinalThreshold)<<std::dec);
	
	int newThresholdStep = cgi["ThresholdStep"     ]->getIntegerValue();
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the ThresholdStep to :"<<std::dec<<static_cast<int>(newThresholdStep)<<std::dec);
	
	uint8_t newThresholdVTH1 = cgi["ThresholdVTH1"     ]->getIntegerValue();
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the ThresholdVTH1 to :"<<std::dec<<static_cast<int>(newThresholdVTH1)<<std::dec);
	vfatDevice->setVThreshold1(newThresholdVTH1);
	
	uint8_t newThresholdVTH2 = cgi["ThresholdVTH2"     ]->getIntegerValue();
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the ThresholdVTH2 to :"<<std::dec<<static_cast<int>(newThresholdVTH2)<<std::dec);
	vfatDevice->setVThreshold2(newThresholdVTH2);
	
	uint8_t newMinTriggersThreshold = cgi["MinTriggersThreshold"     ]->getIntegerValue();
	LOG4CPLUS_INFO(getApplicationLogger(), "Setting the MinTriggersThreshold to :"<<std::dec<<static_cast<int>(newMinTriggersThreshold)<<std::dec);

    }

  /*
  catch (const boost::exception & e)
    {
      XCEPT_RAISE(boost::exception, e);
    }
  */
  catch (const xgi::exception::Exception & e)
    {
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
  catch (const std::exception & e)
    {
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }  
  LOG4CPLUS_INFO(this->getApplicationLogger(), "getCheckedRegisters::list obtained");
  //return regValsToSet;
}

void gem::hw::vfat::VFAT2ThresholdScan::performAction(cgicc::Cgicc cgi, std::vector<std::pair<std::string,uint8_t> > regValsToSet)
  throw (xgi::exception::Exception)
{
  /** possible actions
   *Read Counters (name=ReadCounters
   *
   *Get selected channel (name=GetChannel)
   *Set selected channel (name=SetChannel)
   *Set all channels (name=SetAllChannels)
   *
   *Read all(selected?) registers (name=Read)
   *Write selected registers      (name=Write)
   *Compare written values of selected registers      (name=Compare)
   ** change the font colour if the written value
   ** does not equal the read back value?
   *
   *Load settings from XML file (server or upload?)
   *Write current settings to XML file (server or download?)
   **/
  bool makeComparison = cgi.queryCheckbox("Compare");

  if (makeComparison) {
    /**
     *should we just compare what we want to write with what is currently there,
     *or should we write, and compare that what we wrote matches what we said to write?
     */
  }

  LOG4CPLUS_INFO(this->getApplicationLogger(), "performAction::reading options");
  std::string controlOption = "";
  std::string channelOption = "";

  controlOption = cgi["VFAT2ControlOption"]->getValue();
  LOG4CPLUS_INFO(this->getApplicationLogger(),"performAction::Control option " << controlOption);

  /* SB
  LOG4CPLUS_INFO(this->getApplicationLogger(),"performAction::Channel option " << channelOption);
  */

  if (strcmp(controlOption.c_str(),"Read counters") == 0) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Read counters button pressed");
    vfatDevice->readVFAT2Counters();
  }

  else if (strcmp(controlOption.c_str(),"Set Channel 0/1") == 0) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Set channel 1 button pressed");

    if (cgi.queryCheckbox("Cal0") )
      vfatDevice->sendCalPulseToChannel(0);
    else
      vfatDevice->sendCalPulseToChannel(0,false);
    if (cgi.queryCheckbox("Cal1") )
      vfatDevice->sendCalPulseToChannel(1);
    else
      vfatDevice->sendCalPulseToChannel(1,false);
    if (cgi.queryCheckbox("Ch1Mask") )
      vfatDevice->maskChannel(1);
    else
      vfatDevice->maskChannel(1,false);
    if (cgi.queryCheckbox("SetCh1TrimDAC")) 
      vfatDevice->setChannelTrimDAC(1,cgi["Ch1TrimDAC"]->getIntegerValue());
    
    vfatDevice->readVFAT2Channel(1);
    vfatParams_ = vfatDevice->getVFAT2Params();
    scanParams_.thresholdLatency = vfatParams_.latency;
    scanParams_.thresholdVTH1    = vfatParams_.vThresh1;
    scanParams_.thresholdVTH2    = vfatParams_.vThresh2;

    LOG4CPLUS_INFO(this->getApplicationLogger(),"set channel 0/1 - 0x"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[0].fullChannelReg)<<std::dec<<"::<"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[0].calPulse0     )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[0].calPulse      )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[0].mask          )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[0].trimDAC       )<<std::dec<<">"
		   << std::endl);
  }
  
  else if (strcmp(controlOption.c_str(),"Get This Channel") == 0) {
    //shouldn't need to query the device here, just read the properties from the 
    //stored values, and ensure the web page displays that channel
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Get channel button pressed");

    /* SB
    uint8_t chan = cgi["ChanSel"]->getIntegerValue();
    uint8_t chanSettings = vfatDevice->getChannelSettings(chan);
    */

    vfatParams_ = vfatDevice->getVFAT2Params();
    scanParams_.thresholdLatency = vfatParams_.latency;
    scanParams_.thresholdVTH1    = vfatParams_.vThresh1;
    scanParams_.thresholdVTH2    = vfatParams_.vThresh2;

    /* SB
    vfatParams_.activeChannel = chan;
    vfatDevice->setActiveChannelWeb(chan);
    */
  }

  else if (strcmp(controlOption.c_str(),"Set This Channel") == 0) {
    //shouldn't need protection here
    uint8_t chan = cgi["ChanSel"]->getIntegerValue();
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Set this channel button pressed for channel "
		   << (unsigned)chan);
    //vfatDevice->getVFAT2Params().channels[chan-1].calPulse  = (cgi.queryCheckbox("ChCal") );
    //vfatDevice->getVFAT2Params().channels[chan-1].mask      = (cgi.queryCheckbox("ChMask") );

    if (cgi.queryCheckbox("ChCal") )
      vfatDevice->sendCalPulseToChannel(chan);
    else
      vfatDevice->sendCalPulseToChannel(chan,false);
    if (cgi.queryCheckbox("ChMask") )
      vfatDevice->maskChannel(chan);
    else
      vfatDevice->maskChannel(chan,false);
    if (cgi.queryCheckbox("SetTrimDAC")) 
      //vfatDevice->getVFAT2Params().channels[chan-1].trimDAC = cgi["TrimDAC"]->getIntegerValue();
      vfatDevice->setChannelTrimDAC(chan,cgi["TrimDAC"]->getIntegerValue());
    
    //vfatDevice->getVFAT2Params().activeChannel = chan;
    //vfatDevice->readVFAT2Channel(vfatDevice->getVFAT2Params(),chan);

    /* SB
    vfatDevice->readVFAT2Channel(chan);
    vfatParams_ = vfatDevice->getVFAT2Params();
    vfatParams_.activeChannel = chan;
    LOG4CPLUS_INFO(this->getApplicationLogger(),
		   "chan = "<< (unsigned)chan << "; activeChannel = " <<(unsigned)vfatParams_.activeChannel << std::endl);
    LOG4CPLUS_INFO(this->getApplicationLogger(),"set this channel " << (unsigned)chan << " - 0x"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].fullChannelReg)<<std::dec<<"::<"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].calPulse0     )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].calPulse      )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].mask          )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].trimDAC       )<<std::dec<<">"
		   << std::endl);
    */
  }
  
  else if (strcmp(controlOption.c_str(),"Set All Channels") == 0) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Set all channels button pressed");
    //apply provided settings to all channels (2-128 or 1-128?)
    //SB  uint8_t chan = cgi["ChanSel"]->getIntegerValue();
    int min_chan = 2;
    bool setMasked(false), setCalPulse(false);
    if (cgi.queryCheckbox("ChCal") )
      setCalPulse = true;
    for (int chan = min_chan; chan < 129; ++chan) {
      if (chan == 1)
	vfatDevice->sendCalPulseToChannel(chan-1,setCalPulse);
      vfatDevice->sendCalPulseToChannel(chan,setCalPulse); }
    if (cgi.queryCheckbox("ChMask") )
      setMasked = true;
    for (int chan = min_chan; chan < 129; ++chan)
      vfatDevice->maskChannel(chan,setMasked);
    if (cgi.queryCheckbox("SetTrimDAC")) 
      for (int chan = min_chan; chan < 129; ++chan)
	vfatDevice->setChannelTrimDAC(chan,cgi["TrimDAC"]->getIntegerValue());
    
    //vfatDevice->readVFAT2Channels(vfatDevice->getVFAT2Params());
    vfatDevice->readVFAT2Channels();
    vfatParams_ = vfatDevice->getVFAT2Params();
    scanParams_.thresholdLatency = vfatParams_.latency;
    scanParams_.thresholdVTH1  = vfatParams_.vThresh1;
    scanParams_.thresholdVTH2  = vfatParams_.vThresh2;

    /* SB
    vfatParams_.activeChannel  = chan;
    LOG4CPLUS_INFO(this->getApplicationLogger(),"set all channels " << (unsigned)chan << " - 0x"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].fullChannelReg)<<std::dec<<"::<"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].calPulse0     )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].calPulse      )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].mask          )<<std::dec<<":"
		   <<std::hex<<static_cast<unsigned>(vfatParams_.channels[chan-1].trimDAC       )<<std::dec<<">"
		   << std::endl);
    */
  }

  else if (strcmp(controlOption.c_str(),"Read VFAT") == 0) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Read VFAT button pressed with following registers");

    std::vector<std::pair<std::string,uint8_t> >::const_iterator curReg = regValsToSet.begin();
    for (; curReg != regValsToSet.end(); ++curReg) {
      //std::string msg = toolbox::toString((curReg->first).c_str());
      LOG4CPLUS_INFO(this->getApplicationLogger(),curReg->first);
    }
    vfatDevice->readVFATRegs(regValsToSet);
    //readVFAT2Registers(vfatDevice->getVFAT2Params());
    vfatDevice->getAllSettings();
    vfatParams_ = vfatDevice->getVFAT2Params();
    scanParams_.thresholdLatency = vfatParams_.latency;
    scanParams_.thresholdVTH1    = vfatParams_.vThresh1;
    scanParams_.thresholdVTH2    = vfatParams_.vThresh2;
  }
  else if (strcmp(controlOption.c_str(),"Write VFAT") == 0) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Write VFAT button pressed with following registers");
    
    std::vector<std::pair<std::string,uint8_t> >::const_iterator curReg = regValsToSet.begin();
    for (; curReg != regValsToSet.end(); ++curReg) {
      std::string msg =
	toolbox::toString("%s - 0x%02x",(curReg->first).c_str(),static_cast<unsigned>(curReg->second));
      LOG4CPLUS_INFO(this->getApplicationLogger(),msg);
  }
    
    //do a single transaction
    //vfatDevice->writeVFATRegs(regValsToSet);
    
    curReg = regValsToSet.begin();
    for (; curReg != regValsToSet.end(); ++curReg)
      vfatDevice->writeVFATReg(curReg->first,curReg->second);

    /* SB
    vfatDevice->getAllSettings();
    vfatParams_ = vfatDevice->getVFAT2Params();
    */
  }
  //vfatParams_ = vfatDevice->getVFAT2Params();
}
