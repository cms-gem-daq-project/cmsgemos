#include "gemsupervisor/GEMSupervisor.h"

gemsupervisor::GEMSupervisor::GEMSupervisor()
{

  myParameter_ = 0;

  gemsupervisor::GEMSupervisor::initializeConnection();
}

void gemsupervisor::GEMSupervisor::initializeConnection() 
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
    manager = new uhal::ConnectionManager( connectionPath );
  }
  catch (const std::exception& e) {
    std::cout << "Something went wrong initializing the connection: " << e.what() << std::endl;
  }
}


void gemsupervisor::GEMSupervisor::main()
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
  
  gemsupervisor::GEMSupervisor::getTestReg();
  std::cout << "Current value of test register_ = " << testReg_       << std::endl;
  gemsupervisor::GEMSupervisor::setTestReg(32);
  std::cout << "Current value of test register_ = " << testReg_       << std::endl;
}

void gemsupervisor::GEMSupervisor::getTestReg()
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
void gemsupervisor::GEMSupervisor::setTestReg(uint32_t setVal)
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
