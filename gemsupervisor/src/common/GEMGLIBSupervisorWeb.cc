#include "gem/supervisor/GEMGLIBSupervisorWeb.h"
#include "gem/supervisor/tbutils/ThresholdEvent.h"
#include "gem/hw/vfat/HwVFAT2.h"

#include <ctime>
#include <sstream>
#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::GEMGLIBSupervisorWeb)

void gem::supervisor::GEMGLIBSupervisorWeb::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{
    latency   = 128U;

    outFileName  = "";

    deviceIP      = "192.168.0.115";
    deviceName    = (xdata::String)"VFAT13";
    deviceNum     = -1;
    triggerSource = 0x2; 
    deviceChipID  = 0x0; 
    deviceVT1     = 0x0; 
    deviceVT2     = 0x0; 

    bag->addField("latency",       &latency );
    bag->addField("outFileName",   &outFileName );

    bag->addField("deviceName",    &deviceName  );
    bag->addField("deviceIP",      &deviceIP    );
    bag->addField("deviceNum",     &deviceNum   );
    bag->addField("triggerSource", &triggerSource );
    bag->addField("deviceChipID",  &deviceChipID);
    bag->addField("deviceVT1",     &deviceVT1   );
    bag->addField("deviceVT2",     &deviceVT2   );
}

// Main constructor
gem::supervisor::GEMGLIBSupervisorWeb::GEMGLIBSupervisorWeb(xdaq::ApplicationStub * s)
    throw (xdaq::exception::Exception):
        xdaq::WebApplication(s),
        wl_semaphore_(toolbox::BSem::FULL),
        hw_semaphore_(toolbox::BSem::FULL),
        is_working_ (false),
        is_initialized_ (false),
        is_configured_ (false),
        is_running_ (false)
{
    // HyperDAQ bindings
    xgi::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::webDefault,     "Default");
    xgi::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::webConfigure,   "Configure");
    xgi::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::webStart,       "Start");
    xgi::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::webStop,        "Stop");
    xgi::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::webHalt,        "Halt");
    xgi::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::webTrigger,     "Trigger");

    // SOAP bindings
    xoap::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::onConfigure,   "Configure",   XDAQ_NS_URI);
    xoap::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::onStart,       "Start",       XDAQ_NS_URI);
    xoap::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::onStop,        "Stop",        XDAQ_NS_URI);
    xoap::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::onHalt,        "Halt",        XDAQ_NS_URI);

    // Initiate and activate main workloop
    wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("GEMGLIBSupervisorWebWorkLoop", "waiting");
    wl_->activate();

    // Workloop bindings
    configure_signature_   = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::configureAction, "configureAction");
    start_signature_       = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::startAction,     "startAction");
    stop_signature_        = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::stopAction,      "stopAction");
    halt_signature_        = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction,      "haltAction");
    run_signature_         = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::runAction,       "runAction");
    read_signature_        = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::readAction,      "readAction");

    // Define FSM states
    fsm_.addState('H', "Halted",     this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);
    fsm_.addState('C', "Configured", this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);
    fsm_.addState('R', "Running",    this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);

    // Define error FSM state
    fsm_.setStateName('F', "Error");
    fsm_.setFailedStateTransitionAction(this, &gem::supervisor::GEMGLIBSupervisorWeb::transitionFailed);
    fsm_.setFailedStateTransitionChanged(this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);

    // Define allowed FSM state transitions
    fsm_.addStateTransition('H', 'C', "Configure", this, &gem::supervisor::GEMGLIBSupervisorWeb::configureAction);
    fsm_.addStateTransition('H', 'H', "Halt",      this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction);
    fsm_.addStateTransition('C', 'C', "Configure", this, &gem::supervisor::GEMGLIBSupervisorWeb::configureAction);
    fsm_.addStateTransition('C', 'R', "Start",     this, &gem::supervisor::GEMGLIBSupervisorWeb::startAction);
    fsm_.addStateTransition('C', 'H', "Halt",      this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction);
    fsm_.addStateTransition('R', 'C', "Stop",      this, &gem::supervisor::GEMGLIBSupervisorWeb::stopAction);
    fsm_.addStateTransition('R', 'H', "Halt",      this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction);

    // Define forbidden FSM state transitions
    fsm_.addStateTransition('H', 'H', "Start"    , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);
    fsm_.addStateTransition('H', 'H', "Stop"     , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);
    fsm_.addStateTransition('C', 'C', "Stop"     , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);
    fsm_.addStateTransition('R', 'R', "Configure", this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);
    fsm_.addStateTransition('R', 'R', "Start"    , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);

    // Set initial FSM state and reset FSM
    fsm_.setInitialState('H');
    fsm_.reset();

    counter_ = 0;
}

// SOAP interface
    xoap::MessageReference gem::supervisor::GEMGLIBSupervisorWeb::onConfigure(xoap::MessageReference message)
throw (xoap::exception::Exception)
{
    is_working_ = true;

    wl_->submit(configure_signature_);
    return message;
}

    xoap::MessageReference gem::supervisor::GEMGLIBSupervisorWeb::onStart(xoap::MessageReference message)
throw (xoap::exception::Exception)
{
    is_working_ = true;

    wl_->submit(start_signature_);
    return message;
}

    xoap::MessageReference gem::supervisor::GEMGLIBSupervisorWeb::onStop(xoap::MessageReference message)
throw (xoap::exception::Exception)
{
    is_working_ = true;

    wl_->submit(stop_signature_);
    return message;
}

    xoap::MessageReference gem::supervisor::GEMGLIBSupervisorWeb::onHalt(xoap::MessageReference message)
throw (xoap::exception::Exception)
{
    is_working_ = true;

    wl_->submit(halt_signature_);
    return message;
}

// HyperDAQ interface
    void gem::supervisor::GEMGLIBSupervisorWeb::webDefault(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    // Define how often main web interface refreshes
    cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
    head.addHeader("Refresh","2");

    // If we are in "Running" state, check if GLIB has any data available
    if (is_running_) wl_->submit(run_signature_);

    // Page title
    *out << cgicc::h1("GEM DAQ Supervisor")<< std::endl;

    // Choose DAQ type: Spy or Global
    *out << "DAQ type: " << cgicc::select().set("name", "runtype");
    *out << cgicc::option().set("value", "Spy").set("selected","") << "Spy" << cgicc::option();
    *out << cgicc::option().set("value", "Global") << "Global" << cgicc::option();
    *out << cgicc::select() << endl;
    *out << cgicc::input().set("type", "submit").set("name", "command").set("title", "Set DAQ type").set("value", "Set DAQ type") << cgicc::br() << cgicc::br();

    // Show current state, counter, output filename
    *out << "Current state: " << fsm_.getStateName(fsm_.getCurrentState()) << cgicc::br();
    *out << "Current counter: " << counter_ << " events dumped to disk"  << cgicc::br();
    *out << "Output filename: " << confParams_.bag.outFileName.toString() << cgicc::br();

    // Table with action buttons
    *out << cgicc::table().set("border","0");

    // Row with action buttons
    *out << cgicc::tr();

    // Configure button
    *out << cgicc::td();
    std::string configureButton = toolbox::toString("/%s/Configure",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",configureButton) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Configure") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();

    // Start button
    *out << cgicc::td();
    std::string startButton = toolbox::toString("/%s/Start",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",startButton) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Start") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();

    // Stop button
    *out << cgicc::td();
    std::string stopButton = toolbox::toString("/%s/Stop",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",stopButton) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Stop") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();

    // Halt button
    *out << cgicc::td();
    std::string haltButton = toolbox::toString("/%s/Halt",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",haltButton) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Halt") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();

    // Send L1A signal
    *out << cgicc::td();
    std::string triggerButton = toolbox::toString("/%s/Trigger",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",triggerButton) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Send L1A") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();

    // Finish row with action buttons
    *out << cgicc::tr();

    // Finish table with action buttons
    *out << cgicc::table();
}

    void gem::supervisor::GEMGLIBSupervisorWeb::webConfigure(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    // Derive device number from device name
    std::string tmpDeviceName = confParams_.bag.deviceName.toString();
    int tmpDeviceNum = -1;
    tmpDeviceName.erase(0,4);
    tmpDeviceNum = atoi(tmpDeviceName.c_str());
    tmpDeviceNum -= 8;
    confParams_.bag.deviceNum = tmpDeviceNum;

    // Initiate configure workloop
    wl_->submit(configure_signature_);

    // Go back to main web interface
    this->webRedirect(in, out);
}

    void gem::supervisor::GEMGLIBSupervisorWeb::webStart(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    // Initiate start workloop
    wl_->submit(start_signature_);
    
    // Go back to main web interface
    this->webRedirect(in, out);
}

    void gem::supervisor::GEMGLIBSupervisorWeb::webStop(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    // Initiate stop workloop
    wl_->submit(stop_signature_);

    // Go back to main web interface
    this->webRedirect(in, out);
}

    void gem::supervisor::GEMGLIBSupervisorWeb::webHalt(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    // Initiate halt workloop
    wl_->submit(halt_signature_);

    // Go back to main web interface
    this->webRedirect(in, out);
}

    void gem::supervisor::GEMGLIBSupervisorWeb::webTrigger(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    // Send L1A signal
    hw_semaphore_.take();
    vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
    //for (unsigned int com = 0; com < 15; ++com) vfatDevice_->writeReg("Send.L1ACalPulse",1);
    vfatDevice_->writeReg("Send.L1A",0x1);
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    hw_semaphore_.give();

    // Go back to main web interface
    this->webRedirect(in, out);
}

    void gem::supervisor::GEMGLIBSupervisorWeb::webRedirect(xgi::Input *in, xgi::Output* out) 
throw (xgi::exception::Exception)
{
    // Redirect to main web interface
    std::string url = "/" + getApplicationDescriptor()->getURN() + "/Default";
    *out << "<meta http-equiv=\"refresh\" content=\"0;" << url << "\">" << std::endl;

    this->webDefault(in,out);
}

// work loop call-back functions
bool gem::supervisor::GEMGLIBSupervisorWeb::configureAction(toolbox::task::WorkLoop *wl)
{
    // fire "Configure" event to FSM
    fireEvent("Configure");
    return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::startAction(toolbox::task::WorkLoop *wl)
{
    // fire "Start" event to FSM
    fireEvent("Start");
    return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::stopAction(toolbox::task::WorkLoop *wl)
{
    // Fire "Stop" event to FSM
    fireEvent("Stop");
    return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::haltAction(toolbox::task::WorkLoop *wl)
{
    // Fire "Halt" event to FSM
    fireEvent("Halt");
    return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::runAction(toolbox::task::WorkLoop *wl)
{
    wl_semaphore_.take();
    hw_semaphore_.take();

    // Get the size of GLIB data buffer
    vfatDevice_->setDeviceBaseNode("GLIB");
    uint32_t bufferDepth = vfatDevice_->readReg("LINK1.TRK_FIFO.DEPTH");

    wl_semaphore_.give();
    hw_semaphore_.give();

    LOG4CPLUS_INFO(this->getApplicationLogger(),"bufferDepth = " << bufferDepth << std::endl);

    // If GLIB data buffer has non-zero size, initiate read workloop
    if (bufferDepth) {
        wl_->submit(read_signature_);
    }

    return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::readAction(toolbox::task::WorkLoop *wl)
{
    wl_semaphore_.take();
    hw_semaphore_.take();

    // Book event variables
    tbutils::ChannelData ch;
    tbutils::VFATEvent ev;
    int event=0;

    // Output filename
    std::string tmpFileName = confParams_.bag.outFileName.toString();
    LOG4CPLUS_INFO(getApplicationLogger(),"file " << tmpFileName << " opened to write from FIFO ");

    // GLIB data buffer validation
    boost::format linkForm("LINK%d");
    uint32_t fifoDepth[3];
    vfatDevice_->setDeviceBaseNode("GLIB");
    fifoDepth[0] = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");
    fifoDepth[1] = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");
    fifoDepth[2] = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRK_FIFO.DEPTH");

    int bufferDepth = 0;
    if (fifoDepth[0] != fifoDepth[1] ||
            fifoDepth[0] != fifoDepth[2] ||
            fifoDepth[1] != fifoDepth[2]) {
        LOG4CPLUS_INFO(getApplicationLogger(), "tracking data fifos had different depths:: "
                << fifoDepth[0] << ","
                << fifoDepth[0] << ","
                << fifoDepth[0]);
        bufferDepth = std::min(fifoDepth[0],std::min(fifoDepth[1],fifoDepth[2]));
    }
    bufferDepth = fifoDepth[1];

    bool isFirst = true;
    uint32_t bxNum, bxExp;

    // For each event in GLIB data buffer
    while (bufferDepth) {
        std::vector<uint32_t> data;
        vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.TRK_DATA.COL1");

        LOG4CPLUS_DEBUG(getApplicationLogger(),"Trying to read register "<<vfatDevice_->getDeviceBaseNode()<<".DATA_RDY");
        if (vfatDevice_->readReg("DATA_RDY")) {
            LOG4CPLUS_DEBUG(getApplicationLogger(),"Trying to read the block at "<<vfatDevice_->getDeviceBaseNode()<<".DATA");
            for (int word = 0; word < 7; ++word) {
                std::stringstream ss9;
                ss9 << "DATA." << word;
                data.push_back(vfatDevice_->readReg(ss9.str()));
            }
        }

        uint32_t TrigReg, bxNumTr;
        uint8_t SBit;

        // read trigger data
        vfatDevice_->setDeviceBaseNode("GLIB");
        //TrigReg = vfatDevice_->readReg(boost::str(linkForm%(link))+".TRG_DATA.DATA");
        TrigReg = vfatDevice_->readReg("TRG_DATA.DATA");
        bxNumTr = TrigReg >> 6;
        SBit = TrigReg & 0x0000003F;

        if (!(
                    (((data.at(5)&0xF0000000)>>28)==0xa) &&
                    (((data.at(5)&0x0000F000)>>12)==0xc) &&
                    (((data.at(4)&0xF0000000)>>28)==0xe)
             )) {
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

        // dump event to disk
	keepEvent(tmpFileName, event, ev, ch);
        counter_++;
	
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

        vfatDevice_->setDeviceBaseNode("GLIB");
        bufferDepth = vfatDevice_->readReg("LINK1.TRK_FIFO.DEPTH");
    }

    hw_semaphore_.give();
    wl_semaphore_.give();

    return false;
}

    // State transitions
    void gem::supervisor::GEMGLIBSupervisorWeb::configureAction(toolbox::Event::Reference evt)
        throw (toolbox::fsm::exception::Exception)
        {
            is_working_ = true;

            counter_ = 0;

            hw_semaphore_.take();

            // Define device
            vfatDevice_ = new gem::hw::vfat::HwVFAT2(this, "VFAT9");

            vfatDevice_->setAddressTableFileName("testbeam_registers.xml");
            vfatDevice_->setDeviceIPAddress(confParams_.bag.deviceIP);
            vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
            vfatDevice_->connectDevice();
            vfatDevice_->readVFAT2Counters();
            vfatDevice_->setRunMode(0);
            confParams_.bag.deviceChipID = vfatDevice_->getChipID();

            latency_   = confParams_.bag.latency;

            // Set VFAT2 registers
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

            vfatDevice_->setVThreshold1(2);
            confParams_.bag.deviceVT1 = vfatDevice_->getVThreshold1();
            vfatDevice_->setVThreshold2(0);
            confParams_.bag.deviceVT2 = vfatDevice_->getVThreshold2();
            confParams_.bag.latency = vfatDevice_->getLatency();

            // Create a new output file
            time_t now  = time(0);
            tm    *gmtm = gmtime(&now);
            char* utcTime = asctime(gmtm);
            std::string tmpFileName = "GEM_DAQ_";
            tmpFileName.append(utcTime);
            tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
            tmpFileName.append(".dat");
            std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
            std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');

            confParams_.bag.outFileName = tmpFileName;

            std::fstream scanStream(tmpFileName.c_str(), std::ios::app | std::ios::binary);

            //start readout
            scanStream.close();

            hw_semaphore_.give();

            is_configured_  = true;
            is_working_     = false;    

        }

    void gem::supervisor::GEMGLIBSupervisorWeb::startAction(toolbox::Event::Reference evt)
        throw (toolbox::fsm::exception::Exception)
        {
            is_working_ = true;

            is_running_ = true;
            hw_semaphore_.take();

            //set clock source
            vfatDevice_->setDeviceBaseNode("OptoHybrid.CLOCKING");
            vfatDevice_->writeReg("VFAT.SOURCE",  0x0);
            vfatDevice_->writeReg("CDCE.SOURCE",  0x0);

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

            is_working_ = false;
        }

    void gem::supervisor::GEMGLIBSupervisorWeb::stopAction(toolbox::Event::Reference evt)
        throw (toolbox::fsm::exception::Exception)
        {
            is_running_ = false;
        }

    void gem::supervisor::GEMGLIBSupervisorWeb::haltAction(toolbox::Event::Reference evt)
        throw (toolbox::fsm::exception::Exception)
        {
            is_running_ = false;
            counter_ = 0;
        }

    void gem::supervisor::GEMGLIBSupervisorWeb::noAction(toolbox::Event::Reference evt)
        throw (toolbox::fsm::exception::Exception)
        {
        }

    void gem::supervisor::GEMGLIBSupervisorWeb::fireEvent(std::string name)
    {
        toolbox::Event::Reference event(new toolbox::Event(name, this));
        fsm_.fireEvent(event);
    }

    void gem::supervisor::GEMGLIBSupervisorWeb::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
        throw (toolbox::fsm::exception::Exception)
        {
        }

    void gem::supervisor::GEMGLIBSupervisorWeb::transitionFailed(toolbox::Event::Reference event)
        throw (toolbox::fsm::exception::Exception)
        {
        }
