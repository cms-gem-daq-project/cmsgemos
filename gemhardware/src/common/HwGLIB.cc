//General structure taken blatantly from tcds::utils::HwDeviceTCA/HwGLIB

#include "gem/hw/glib/HwGLIB.h"

#define DEBUG(MSG) LOG4CPLUS_DEBUG(logGEMHw_ , MSG)
#define INFO(MSG)  LOG4CPLUS_INFO(logGEMHw_  , MSG)
#define WARN(MSG)  LOG4CPLUS_WARN(logGEMHw_  , MSG)
#define ERROR(MSG) LOG4CPLUS_ERROR(logGEMHw_ , MSG)
#define FATAL(MSG) LOG4CPLUS_FATAL(logGEMHw_ , MSG)

gem::hw::glib::HwGLIB::HwGLIB(xdaq::Application* glibApp):
  gem::hw::GEMHwDevice::GEMHwDevice(glibApp)
  //logGLIB_(glibApp->getApplicationLogger()),
  //hwGLIB_(0),
  //monGLIB_(0)

{
  setDeviceID("GLIBHw");
  setDeviceBaseNode("");
  //gem::hw::glib::HwGLIB::initDevice();
}

gem::hw::glib::HwGLIB::~Device()
{
  releaseDevice();
}

void gem::hw::glib::HwGLIB::connectDevice()
{
  std::string const controlhubAddress = cfgInfoSpaceP_->getString("controlhubAddress");
  std::string const device1Address    = cfgInfoSpaceP_->getString("deviceAddress");
  uint32_t const    controlhubPort    = cfgInfoSpaceP_->getUInt32("controlhubPort");
  uint32_t const    ipbusPort         = cfgInfoSpaceP_->getUInt32("ipbusPort");
  
  std::stringstream tmpUri;
  if (controlhubAddress.size() > 0)
    {
      INFO("Using control hub at address '" << controlhubAddress
           << ", port number " << controlhubPort << "'.");
      tmpUri << "chtcp-"
             << getIPbusProtocolVersion()
             << "://"
             << controlhubAddress
             << ":"
             << controlhubPort
             << "?target="
             << deviceAddress
             << ":"
             << ipbusPort;
    }
  else
    {
      INFO("No control hub address specified -> "
           "continuing with a direct connection.");
      tmpUri << "ipbusudp-"
             << getIPbusProtocolVersion()
             << "://"
             << deviceAddress
             << ":"
             << ipbusPort;
    }
  std::string const uri = tmpUri.str();
  std::string const id = "HwGLIB";
  
}

void gem::hw::glib::HwGLIB::releaseDevice()
{

}

void gem::hw::glib::HwGLIB::enableDevice()
{

}

void gem::hw::glib::HwGLIB::disableDevice()
{

}

void gem::hw::glib::HwGLIB::pauseDevice()
{

}

void gem::hw::glib::HwGLIB::startDevice()
{

}

void gem::hw::glib::HwGLIB::stopDevice()
{

}

void gem::hw::glib::HwGLIB::resumeDevice()
{

}

void gem::hw::glib::HwGLIB::haltDevice()
{

}

void gem::hw::glib::HwGLIB::initDevice() 
{
  char * val;
  val = std::getenv( "GLIBTEST" );
  std::string dirVal = "";
  if (val != NULL) {
    dirVal = val;
  }
  else {
    std::cout<<"$GLIBTEST not set, exiting"<<std::endl;
    exit(1);
  }
  
  //setLogLevelTo(uhal::Debug());  // Maximise uHAL logging
  setLogLevelTo(uhal::Error());  // Minimise uHAL logging
  char connectionPath[128];
  try {
    sprintf(connectionPath,"file://%s/data/myconnections.xml;",dirVal.c_str());
    manageGLIBConnection = new uhal::ConnectionManager( connectionPath );
  }
  catch (const std::exception& e) {
    std::cout << "Something went wrong initializing the connection: " << e.what() << std::endl;
  }
}


void gem::hw::glib::HwGLIB::main()
{
  uhal::HwInterface hw=manager->getDevice ( "gemsupervisor.udp.0" );
  //*hw = manager->getDevice ( "gemsupervisor.udp.0" );

  // print out basic information
  std::cout << "current Value of myParameter_ = " << myParameter_ << std::endl;
  //std::cout << "System ID: " << formatSystemID(systemID_,0) << std::endl;
  //std::cout << "Board ID: "  << formatBoardID(boardID_,0)   << std::endl;
  //std::cout << "System firmware version: " << formatFW(firmwareID_,0) << std::endl;
  //std::cout << "System firmware date: "    << formatFW(firmwareID_,1) << std::endl;
  std::cout << "Current value of test register_ = " << testReg_       << std::endl;
  
  gem::hw::glib::HwGLIB::getTestReg();
  std::cout << "Current value of test register_ = " << testReg_       << std::endl;
  gem::hw::glib::HwGLIB::setTestReg(32);
  std::cout << "Current value of test register_ = " << testReg_       << std::endl;
}

void gem::hw::glib::HwGLIB::getTestReg()
						 //uhal::HwInterface &hw, uhal::ValWord< uint32_t> &mem)
{
  uhal::HwInterface hw=manager->getDevice ( "gemsupervisor.udp.0" );
  try {
    r_test = hw.getNode ( "test" ).read();
    hw.dispatch();
    testReg_ = r_test.value();
  }
  catch (const std::exception& e) {
    std::cout << "Something went wrong reading the test register: " << e.what() << std::endl;
  }
  
}
void gem::hw::glib::HwGLIB::setTestReg(uint32_t setVal)
						 //uhal::HwInterface &hw, uhal::ValWord< uint32_t> &mem)
{
  testReg_ = setVal;
  
  uhal::HwInterface hw=manager->getDevice ( "gemsupervisor.udp.0" );
  try {
    hw.getNode ( "test" ).write(testReg_);
    hw.dispatch();
  }
  catch (const std::exception& e) {
    std::cout << "Something went wrong writing the test register: " << e.what() << std::endl;
  }
  
  try {
    r_test = hw.getNode ( "test" ).read();
    hw.dispatch();
    testReg_ = r_test.value();
  }
  catch (const std::exception& e) {
    std::cout << "Something went wrong reading the test register: " << e.what() << std::endl;
  }
}
