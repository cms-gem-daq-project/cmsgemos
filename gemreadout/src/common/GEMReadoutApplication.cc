/**
 * class: GEMReadoutApplication
 * description: Generic GEM application to handle readout
 *              structure borrowed from HCAL
 * author: J. Sturdy
 * date: 
 */
#include "toolbox/mem/Pool.h"
#include "toolbox/mem/MemoryPoolFactory.h"
#include "toolbox/mem/CommittedHeapAllocator.h"

#include "gem/readout/GEMReadoutApplication.h"
//#include "gem/base/GEMWebApplication.h"

const int gem::readout::GEMReadoutApplication::I2O_READOUT_NOTIFY=0x84;
const int gem::readout::GEMReadoutApplication::I2O_READOUT_CONFIRM=0x85;

/*
  namespace gem {
  namespace readout {
    class GEMReadoutApplicationTask : public toolbox::Task {
    public:
      ReadoutApplicationTask(GEMReadoutApplication* app) : toolbox::Task("GEMReadoutApplicationTask")
      {
        p_readoutApp=std::make_shared<GEMReadoutApplication>(app);
      }
      virtual int svc() { return p_readoutApp->readoutTask(); }
    private:
      std::shared_ptr<GEMReadoutApplication> p_readoutApp;
    };
  }
}
*/
gem::readout::GEMReadoutApplication::GEMReadoutSettings::GEMReadoutSettings() {
  runType        = "";
  fileName       = "";
  outputType     = "Bin";
  outputLocation = "";
}

void gem::readout::GEMReadoutApplication::GEMReadoutSettings::registerFields(xdata::Bag<gem::readout::GEMReadoutApplication::GEMReadoutSettings>* bag) {
  bag->addField("runType",        &runType);
  bag->addField("fileName",       &fileName);
  bag->addField("outputType",     &outputType);
  bag->addField("outputLocation", &outputLocation);
}


gem::readout::GEMReadoutApplication::GEMReadoutApplication(xdaq::ApplicationStub* stub)
  throw (xdaq::exception::Exception) :
  gem::base::GEMFSMApplication(stub),
  m_outFileName(""),
  m_connectionFile("ConnectionFile"),
  m_deviceName("ReadoutDevice"),
  m_eventsReadout(0),
  m_usecPerEvent(0.0),
  m_usecUsed(0.0)
{
  DEBUG("GEMReadoutApplication ctor begin");
  //i2o::bind(this,&ReadoutApplication::onReadoutNotify,I2O_READOUT_NOTIFY,XDAQ_ORGANIZATION_ID);
  //xoap::bind(this,&ReadoutApplication::getReadoutCredits,"GetReadoutCredits","urn:GEMReadoutApplication-soap:1");
  p_appInfoSpace->fireItemAvailable("ReadoutSettings",&m_readoutSettings);
  p_appInfoSpace->fireItemAvailable("DeviceName",     &m_deviceName);
  p_appInfoSpace->fireItemAvailable("ConnectionFile", &m_connectionFile);
  p_appInfoSpace->fireItemAvailable("EventsReadout",  &m_eventsReadout);
  p_appInfoSpace->fireItemAvailable("uSecPerEvent",   &m_usecPerEvent);

  p_appInfoSpace->addItemRetrieveListener("ReadoutSettings", this);
  p_appInfoSpace->addItemRetrieveListener("DeviceName",      this);
  p_appInfoSpace->addItemRetrieveListener("ConnectionFile",  this);
  p_appInfoSpace->addItemRetrieveListener("EventsReadout",   this);
  p_appInfoSpace->addItemRetrieveListener("uSecPerEvent",    this);
  
  p_appInfoSpace->addItemChangedListener( "ReadoutSettings", this);
  p_appInfoSpace->addItemChangedListener( "DeviceName",      this);
  p_appInfoSpace->addItemChangedListener( "ConnectionFile",  this);
  p_appInfoSpace->addItemChangedListener( "EventsReadout",   this);
  p_appInfoSpace->addItemChangedListener( "uSecPerEvent",    this);

  ////initialize the GLIB application objects
  //DEBUG("Connecting to the GLIBReadoutWeb interface");
  //p_gemWebInterface = new gem::hw::glib::GLIBManagerWeb(this);
  ////p_gemMonitor      = new gem::hw::glib::GLIBHwMonitor(this);
  //DEBUG("done");
  //
  ////set up the info hwCfgInfoSpace 
  //init();
  DEBUG("GEMReadoutApplication::GEMReadoutApplication() "      << std::endl
        << " m_deviceName:"     << m_deviceName.toString()     << std::endl
        << " m_connectionFile:" << m_connectionFile.toString() << std::endl
        << " m_eventsReadout:"  << m_eventsReadout.toString()  << std::endl
        );
  DEBUG("GEMReadoutApplication ctor end");
}

gem::readout::GEMReadoutApplication::~GEMReadoutApplication()
{
  
}

void gem::readout::GEMReadoutApplication::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("GEMReadoutApplication::actionPerformed() setDefaultValues" << 
          "Default configuration values have been loaded from xml profile");
    importConfigurationParameters();
    importMonitoringParameters();
    //p_gemMonitor->startMonitoring();
  }
  // update monitoring variables
  DEBUG("GEMReadoutApplication::actionPerformed() " << event.type() << std::endl
        << " m_connectionFile:" << m_connectionFile.toString()      << std::endl
        << " m_deviceName:"     << m_deviceName.toString()          << std::endl
        << " m_eventsReadout:"  << m_eventsReadout.toString()       << std::endl
        );
  // update monitoring variables
  gem::base::GEMFSMApplication::actionPerformed(event);  
}


void gem::readout::GEMReadoutApplication::initializeAction()
  /*throw (gem::readout::exception::Exception)*/
{
  DEBUG("gem::readout::GEMReadoutApplication::initializeAction begin");
  if (!m_task) {
    m_task = std::make_shared<gem::readout::GEMReadoutTask>(this);
    m_task->activate();
  } else {
    m_cmdQueue.push(ReadoutCommands::CMD_STOP);
  }

  /*
  // create a pool
  if (!m_pool) {
    char poolname[128];
    snprintf(poolname,128,"GEMReadoutPool-%s-%d",getApplicationDescriptor()->getClassName().c_str(),(int)getApplicationDescriptor()->getInstance());
    try {
      // 4k events at the average size
      toolbox::mem::CommittedHeapAllocator* alloc = new toolbox::mem::CommittedHeapAllocator(4096*4096);
      toolbox::net::URN urn("toolbox-mem-pool",poolname);
      m_pool = toolbox::mem::getMemoryPoolFactory()->createPool(urn,alloc);
    } catch (xcept::Exception& e) {
      XCEPT_RETHROW(gem::base::exception::Exception,"Unable to create readout memory pool",e);
    }
  }
  */
  m_eventsReadout.value_ = 0;
  m_usecPerEvent.value_  = 0;
  m_usecUsed = 0;
}

void gem::readout::GEMReadoutApplication::configureAction()
  /*throw (gem::readout::exception::Exception)*/
{
  DEBUG("gem::readout::GEMReadoutApplication::configureAction begin");
}

void gem::readout::GEMReadoutApplication::startAction()
  /*throw (gem::readout::exception::Exception)*/
{
  DEBUG("gem::readout::GEMReadoutApplication::startAction begin");
  // build output filename
  
  // Times for output files
  time_t now  = time(0);
  tm    *gmtm = gmtime(&now);
  char* utcTime = asctime(gmtm);

  std::string date_and_time = "";
  date_and_time.append(utcTime);
  date_and_time.erase(std::remove(date_and_time.begin(), date_and_time.end(), '\n'), date_and_time.end());
  std::replace(date_and_time.begin(), date_and_time.end(), ' ', '_' );
  std::replace(date_and_time.begin(), date_and_time.end(), ':', '-');
  
  m_readoutSettings.bag.fileName = toolbox::toString("%s/%s_%s_%s.dat",
                                                     m_readoutSettings.bag.outputLocation.toString().c_str(),
                                                     m_readoutSettings.bag.runType.toString().c_str(),
                                                     m_runNumber.toString().c_str(),
                                                     date_and_time.c_str()
                                                     );
  
  m_outFileName  = m_readoutSettings.bag.fileName.toString();

  m_cmdQueue.push(ReadoutCommands::CMD_START);
}

void gem::readout::GEMReadoutApplication::pauseAction()
  /*throw (gem::readout::exception::Exception)*/
{
  DEBUG("gem::readout::GEMReadoutApplication::pauseAction begin");
  m_cmdQueue.push(ReadoutCommands::CMD_PAUSE);
}

void gem::readout::GEMReadoutApplication::resumeAction()
  /*throw (gem::readout::exception::Exception)*/
{
  DEBUG("gem::readout::GEMReadoutApplication::resumeAction begin");
  m_cmdQueue.push(ReadoutCommands::CMD_RESUME);
}

void gem::readout::GEMReadoutApplication::stopAction()
  /*throw (gem::readout::exception::Exception)*/
{
  DEBUG("gem::readout::GEMReadoutApplication::stopAction begin");
  m_cmdQueue.push(ReadoutCommands::CMD_STOP);
}

void gem::readout::GEMReadoutApplication::haltAction()
  /*throw (gem::readout::exception::Exception)*/
{
  DEBUG("gem::readout::GEMReadoutApplication::haltAction begin");
  if (m_task) 
    m_cmdQueue.push(ReadoutCommands::CMD_STOP);
}

void gem::readout::GEMReadoutApplication::resetAction()
  /*throw (gem::readout::exception::Exception)*/
{
  DEBUG("gem::readout::GEMReadoutApplication::resetAction begin");
}

void gem::readout::GEMReadoutApplication::failAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  // close open file pointers
}

void gem::readout::GEMReadoutApplication::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception)
{
  // close open file pointers
}

int gem::readout::GEMReadoutApplication::readoutTask()
{
  bool isRunning(false), isDone(false);
  int nevtsRead(0);
  // may at some point want to actually pass the memory
  std::vector<toolbox::mem::Reference* > data;
  
  while (!isDone) {
    if (!isRunning || m_cmdQueue.size() > 0) {
      int cmd = m_cmdQueue.pop();
      switch(cmd) {
      case(ReadoutCommands::CMD_PAUSE) :
        isRunning = false;
        break;
      case(ReadoutCommands::CMD_STOP) :
        isRunning = false;
        break;
      case(ReadoutCommands::CMD_START) :
        isRunning = true;
        break;
      case(ReadoutCommands::CMD_RESUME) :
        isRunning = true;
        break;
      case(ReadoutCommands::CMD_EXIT) :
        isDone    = true;
        isRunning = false;
        break;
      }
    }
    if (isRunning) {
      data.clear();
      struct timeval start,stop;
      
      gettimeofday(&start,0);
      nevtsRead = 0;
      try {
        nevtsRead = readout(0,0,data);
      } catch (gem::base::exception::Exception& e) {
        ERROR(xcept::stdformat_exception_history(e));
      }
      
      DEBUG("GEMReadoutApplication::readoutTask read " << nevtsRead << " events");
      /*
      for (int i = 0; i < nevtsRead; i++) {
        DEBUG("GEMReadoutApplication::readoutTask releaseing data[" << i << "]");
        data[i]->release();
      }
      */
      if (nevtsRead > 0) {
        DEBUG("GEMReadoutApplication::readoutTask read " << nevtsRead << " events");
        gettimeofday(&stop,0);
        m_eventsReadout.value_ = m_eventsReadout.value_ + nevtsRead;
        double deltaU=(stop.tv_sec-start.tv_sec)*1e6+(stop.tv_usec-start.tv_usec);
        m_usecUsed += deltaU;
        m_usecPerEvent.value_ = m_usecUsed/(m_eventsReadout.value_);
      }
    }
  }
  return 0;
}
