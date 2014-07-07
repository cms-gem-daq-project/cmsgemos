#include "gemsupervisor/GemSupervisorWeb.h"
#include <sstream>
#include <boost/format.hpp>

XDAQ_INSTANTIATOR_IMPL(gemsupervisor::GemSupervisorWeb)

gemsupervisor::GemSupervisorWeb::GemSupervisorWeb(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception):
    xdaq::Application(s), xgi::framework::UIManager(this)
{
  xgi::framework::deferredbind(this, this, &GemSupervisorWeb::Default,      "Default");
  xgi::framework::deferredbind(this, this, &GemSupervisorWeb::setParameter, "setParameter");
  xgi::framework::deferredbind(this, this, &GemSupervisorWeb::Views,      "Views");
  xgi::framework::deferredbind(this, this, &GemSupervisorWeb::Read, "Read");
  xgi::framework::deferredbind(this, this, &GemSupervisorWeb::Write, "Write");

  myAction_         = "";
  myParameter_      = 0;
  testReg_          = 0;
  boardID_          = 0;
  systemID_         = 0;
  systemFirmwareID_ = 0;

  getApplicationInfoSpace()->fireItemAvailable("myParameter",      &myParameter_);
  getApplicationInfoSpace()->fireItemAvailable("testReg",          &testReg_);
  getApplicationInfoSpace()->fireItemAvailable("boardID",          &boardID_);
  getApplicationInfoSpace()->fireItemAvailable("systemID",         &systemID_);
  getApplicationInfoSpace()->fireItemAvailable("systemFirmwareID", &systemFirmwareID_);

  gemsupervisor::GemSupervisorWeb::initializeConnection();
}

void gemsupervisor::GemSupervisorWeb::initializeConnection() 
{
  char * val;
  val = std::getenv( "GLIBTEST" );
  std::string dirVal = "";
  if (val != NULL) {
    dirVal = val;
  }
  else {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"$GLIBTEST not set, exiting");
    std::cout<<"$GLIBTEST not set, exiting"<<std::endl;
    exit(1);
  }
  
  setLogLevelTo(uhal::Debug());  // Maximise uHAL logging
  //setLogLevelTo(uhal::Error());  // Minimise uHAL logging
  char connectionPath[128];
  try {
    sprintf(connectionPath,"file://%s/data/myconnections.xml;",dirVal.c_str());
    manager = new uhal::ConnectionManager( connectionPath );
    hw = new uhal::HwInterface(manager->getDevice("gemsupervisor.udp.0"));
  }
  catch (uhalException& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong initializing the connection: " << e.what());
    //throw exception(std::string("IPbus Creation Failure"));
  }
  
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong initializing the connection: " << e.what());
    std::cout << "Something went wrong initializing the connection: " << e.what() << std::endl;
  }
}


void gemsupervisor::GemSupervisorWeb::Views(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  *out << "<div class=\"xdaq-tab-wrapper\">"             << std::endl;

  *out << "  <div class=\"xdaq-tab\" title=\"GLIB Registers\" >"     << std::endl;
  *out << "    <table class=\"xdaq-table\" caption=\"GLIB Registers\" >"     << std::endl;
  *out << "      <thead>" << std::endl;
  *out << "        <tr>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Register name</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Read value</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Value to write</th>" << std::endl;
  *out << "        </tr>" << std::endl;
  *out << "      </thead>" << std::endl;
  *out << "      </br>" << std::endl;

  *out << "      <tbody>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Board_ID</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_boardid.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>First value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>System_ID</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_sysid.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Second value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Firmware_ID</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_fwid.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Third value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Test</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_test.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fourth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Ctrl</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_ctrl.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Ctrl2</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_ctrl2.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Status</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_status.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Status2</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_status2.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Ctrl_SRAM</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_ctrl_sram.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Status_SRAM</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_status_sram.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>SPI_txdata</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_spi_txdata.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>SPI_command</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_spi_command.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>SPI_rxdata</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_spi_rxdata.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>I2C_settings</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_i2c_settings.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>I2C_command</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_i2c_command.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>I2C_reply</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_i2c_reply.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>SFP_phase_mon_stats</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_sfp_phase_mon_stats.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>SFP_phase_mon_ctrl</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_sfp_phase_mon_ctrl.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>FMC_phase_mon_stats</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_fmc_phase_mon_stats.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>FMC_phase_mon_ctrl</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_fmc_phase_mon_ctrl.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;

  *out << "      <tr>" << std::endl;
  *out << "        <td>mac_info1</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_mac_info1.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>mac_info2</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_mac_info2.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  /*
  *out << "      <tr>" << std::endl;
  *out << "        <td>ip_info</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_ip_info.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  */
  *out << "      <tr>" << std::endl;
  *out << "        <td>SRAM1</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_sram1.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;

  *out << "      <tr>" << std::endl;
  *out << "        <td>SRAM2</td>" << std::endl;
  *out << "        <td>0x" << std::hex << r_sram2.value() << std::dec << "</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "      </tr>" << std::endl;

  //*out << "      <tr>" << std::endl;
  //*out << "        <td>icap</td>" << std::endl;
  //*out << "        <td>0x" << std::hex << r_icap.value() << std::dec << "</td>" << std::endl;
  //*out << "        <td>Fifth value 3</td>" << std::endl;
  //*out << "      </tr>" << std::endl;
  *out << "      </br>" << std::endl;
  *out << "      </tbody>" << std::endl;
  *out << "    </table>" << std::endl;
  *out << "    </br>" << std::endl;
  *out << "  </div>" << std::endl;

  *out << "  <div class=\"xdaq-tab\" title=\"GLIB User Registers\" >"     << std::endl;
  *out << "    <table class=\"xdaq-table\" caption=\"GLIB User Registers\" >"     << std::endl;
  *out << "      <thead>" << std::endl;
  *out << "        <tr>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Register name</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Read value</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Value to write</th>" << std::endl;
  *out << "        </tr>" << std::endl;
  *out << "      </thead>" << std::endl;
  *out << "      </br>" << std::endl;

  *out << "      <tbody>" << std::endl;
  unsigned int uregoffset = 0x40010000;
  for (unsigned int ureg = 0; ureg < 16; ++ureg) {
    *out << "      <tr>" << std::endl;
    *out << "        <td>User register 0x" << std::hex << uregoffset+ureg <<std::dec << "</td>" << std::endl;
    *out << "        <td>First value 2</td>" << std::endl;
    *out << "        <td>First value 3</td>" << std::endl;
    *out << "      </tr>" << std::endl;
  }
  *out << "      </br>" << std::endl;
  *out << "      </tbody>" << std::endl;
  *out << "    </table>" << std::endl;
  *out << "    </br>" << std::endl;
  *out << "  </div>" << std::endl;

  *out << "  <div class=\"xdaq-tab\" title=\"Expert\" >"   << std::endl;
  *out << "  Content of the expert view" << std::endl;
  *out << "    <table class=\"xdaq-table-striped\" caption=\"Striped\" >"     << std::endl;
  *out << "      <thead>" << std::endl;
  *out << "        <tr>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 1</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 2</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 3</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 4</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 5</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 6</th>" << std::endl;
  *out << "        </tr>" << std::endl;
  *out << "      </thead>" << std::endl;
  *out << "      </br>" << std::endl;

  *out << "      <tbody>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>First value 1</td>" << std::endl;
  *out << "        <td>First value 2</td>" << std::endl;
  *out << "        <td>First value 3</td>" << std::endl;
  *out << "        <td>First value 4</td>" << std::endl;
  *out << "        <td>First value 5</td>" << std::endl;
  *out << "        <td>First value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Second value 1</td>" << std::endl;
  *out << "        <td>Second value 2</td>" << std::endl;
  *out << "        <td>Second value 3</td>" << std::endl;
  *out << "        <td>Second value 4</td>" << std::endl;
  *out << "        <td>Second value 5</td>" << std::endl;
  *out << "        <td>Second value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Third value 1</td>" << std::endl;
  *out << "        <td>Third value 2</td>" << std::endl;
  *out << "        <td>Third value 3</td>" << std::endl;
  *out << "        <td>Third value 4</td>" << std::endl;
  *out << "        <td>Third value 5</td>" << std::endl;
  *out << "        <td>Third value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Fourth value 1</td>" << std::endl;
  *out << "        <td>Fourth value 2</td>" << std::endl;
  *out << "        <td>Fourth value 3</td>" << std::endl;
  *out << "        <td>Fourth value 4</td>" << std::endl;
  *out << "        <td>Fourth value 5</td>" << std::endl;
  *out << "        <td>Fourth value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Fifth value 1</td>" << std::endl;
  *out << "        <td>Fifth value 2</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "        <td>Fifth value 4</td>" << std::endl;
  *out << "        <td>Fifth value 5</td>" << std::endl;
  *out << "        <td>Fifth value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      </br>" << std::endl;
  *out << "      </tbody>" << std::endl;
  *out << "    </table>" << std::endl;
  *out << "    </br>" << std::endl;
  *out << "  </br>" << std::endl;
  *out << "  </div>" << std::endl;

  *out << "  <div class=\"xdaq-tab\" title=\"Vertical\" >" << std::endl;
  *out << "  Content of the settings view" << std::endl;
  *out << "    <table class=\"xdaq-table-vertical\" caption=\"Vertical\" >"     << std::endl;
  *out << "      <thead>" << std::endl;
  *out << "        <tr>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 1</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 2</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 3</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 4</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 5</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 6</th>" << std::endl;
  *out << "        </tr>" << std::endl;
  *out << "      </thead>" << std::endl;
  *out << "      </br>" << std::endl;

  *out << "      <tbody>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>First value 1</td>" << std::endl;
  *out << "        <td>First value 2</td>" << std::endl;
  *out << "        <td>First value 3</td>" << std::endl;
  *out << "        <td>First value 4</td>" << std::endl;
  *out << "        <td>First value 5</td>" << std::endl;
  *out << "        <td>First value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Second value 1</td>" << std::endl;
  *out << "        <td>Second value 2</td>" << std::endl;
  *out << "        <td>Second value 3</td>" << std::endl;
  *out << "        <td>Second value 4</td>" << std::endl;
  *out << "        <td>Second value 5</td>" << std::endl;
  *out << "        <td>Second value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Third value 1</td>" << std::endl;
  *out << "        <td>Third value 2</td>" << std::endl;
  *out << "        <td>Third value 3</td>" << std::endl;
  *out << "        <td>Third value 4</td>" << std::endl;
  *out << "        <td>Third value 5</td>" << std::endl;
  *out << "        <td>Third value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Fourth value 1</td>" << std::endl;
  *out << "        <td>Fourth value 2</td>" << std::endl;
  *out << "        <td>Fourth value 3</td>" << std::endl;
  *out << "        <td>Fourth value 4</td>" << std::endl;
  *out << "        <td>Fourth value 5</td>" << std::endl;
  *out << "        <td>Fourth value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Fifth value 1</td>" << std::endl;
  *out << "        <td>Fifth value 2</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "        <td>Fifth value 4</td>" << std::endl;
  *out << "        <td>Fifth value 5</td>" << std::endl;
  *out << "        <td>Fifth value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      </br>" << std::endl;
  *out << "      </tbody>" << std::endl;
  *out << "    </table>" << std::endl;
  *out << "    </br>" << std::endl;
  *out << "  </br>" << std::endl;
  *out << "  </div>" << std::endl;

  *out << "  <div class=\"xdaq-tab\" title=\"Nohover\" >"   << std::endl;
  *out << "  Content of the events view" << std::endl;
  *out << "    <table class=\"xdaq-table-nohover\" caption=\"Nohover\" >"     << std::endl;
  *out << "      <thead>" << std::endl;
  *out << "        <tr>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 1</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 2</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 3</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 4</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 5</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 6</th>" << std::endl;
  *out << "        </tr>" << std::endl;
  *out << "      </thead>" << std::endl;
  *out << "      </br>" << std::endl;

  *out << "      <tbody>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>First value 1</td>" << std::endl;
  *out << "        <td>First value 2</td>" << std::endl;
  *out << "        <td>First value 3</td>" << std::endl;
  *out << "        <td>First value 4</td>" << std::endl;
  *out << "        <td>First value 5</td>" << std::endl;
  *out << "        <td>First value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Second value 1</td>" << std::endl;
  *out << "        <td>Second value 2</td>" << std::endl;
  *out << "        <td>Second value 3</td>" << std::endl;
  *out << "        <td>Second value 4</td>" << std::endl;
  *out << "        <td>Second value 5</td>" << std::endl;
  *out << "        <td>Second value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Third value 1</td>" << std::endl;
  *out << "        <td>Third value 2</td>" << std::endl;
  *out << "        <td>Third value 3</td>" << std::endl;
  *out << "        <td>Third value 4</td>" << std::endl;
  *out << "        <td>Third value 5</td>" << std::endl;
  *out << "        <td>Third value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Fourth value 1</td>" << std::endl;
  *out << "        <td>Fourth value 2</td>" << std::endl;
  *out << "        <td>Fourth value 3</td>" << std::endl;
  *out << "        <td>Fourth value 4</td>" << std::endl;
  *out << "        <td>Fourth value 5</td>" << std::endl;
  *out << "        <td>Fourth value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Fifth value 1</td>" << std::endl;
  *out << "        <td>Fifth value 2</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "        <td>Fifth value 4</td>" << std::endl;
  *out << "        <td>Fifth value 5</td>" << std::endl;
  *out << "        <td>Fifth value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      </br>" << std::endl;
  *out << "      </tbody>" << std::endl;
  *out << "    </table>" << std::endl;
  *out << "    </br>" << std::endl;
  *out << "  </br>" << std::endl;
  *out << "  </div>" << std::endl;

  *out << "  <div class=\"xdaq-tab\" title=\"Matrix\" >"   << std::endl;
  *out << "  Content of the events view" << std::endl;
  *out << "    <table class=\"xdaq-table-matrix\" caption=\"Matrix\" >"     << std::endl;
  *out << "      <thead>" << std::endl;
  *out << "        <tr>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 1</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 2</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 3</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 4</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 5</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Column 6</th>" << std::endl;
  *out << "        </tr>" << std::endl;
  *out << "      </thead>" << std::endl;
  *out << "      </br>" << std::endl;

  *out << "      <tbody>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>First value 1</td>" << std::endl;
  *out << "        <td>First value 2</td>" << std::endl;
  *out << "        <td>First value 3</td>" << std::endl;
  *out << "        <td>First value 4</td>" << std::endl;
  *out << "        <td>First value 5</td>" << std::endl;
  *out << "        <td>First value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Second value 1</td>" << std::endl;
  *out << "        <td>Second value 2</td>" << std::endl;
  *out << "        <td>Second value 3</td>" << std::endl;
  *out << "        <td>Second value 4</td>" << std::endl;
  *out << "        <td>Second value 5</td>" << std::endl;
  *out << "        <td>Second value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Third value 1</td>" << std::endl;
  *out << "        <td>Third value 2</td>" << std::endl;
  *out << "        <td>Third value 3</td>" << std::endl;
  *out << "        <td>Third value 4</td>" << std::endl;
  *out << "        <td>Third value 5</td>" << std::endl;
  *out << "        <td>Third value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Fourth value 1</td>" << std::endl;
  *out << "        <td>Fourth value 2</td>" << std::endl;
  *out << "        <td>Fourth value 3</td>" << std::endl;
  *out << "        <td>Fourth value 4</td>" << std::endl;
  *out << "        <td>Fourth value 5</td>" << std::endl;
  *out << "        <td>Fourth value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      <tr>" << std::endl;
  *out << "        <td>Fifth value 1</td>" << std::endl;
  *out << "        <td>Fifth value 2</td>" << std::endl;
  *out << "        <td>Fifth value 3</td>" << std::endl;
  *out << "        <td>Fifth value 4</td>" << std::endl;
  *out << "        <td>Fifth value 5</td>" << std::endl;
  *out << "        <td>Fifth value 6</td>" << std::endl;
  *out << "      </tr>" << std::endl;
  *out << "      </br>" << std::endl;
  *out << "      </tbody>" << std::endl;
  *out << "    </table>" << std::endl;
  *out << "    </br>" << std::endl;
  *out << "  </br>" << std::endl;
  *out << "  </div>" << std::endl;

  *out << "  </br>" << std::endl;
  *out << "</div>" << std::endl;
}

void gemsupervisor::GemSupervisorWeb::Default(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  //uhal::HwInterface hw=manager->getDevice ( "gemsupervisor.udp.0" );
  //*hw = manager->getDevice ( "gemsupervisor.udp.0" );
  /*
   *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
   *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
   *out << cgicc::title("GEM Web Supervisor") << std::endl;
   *out << cgicc::a("Visit the XDAQ Web site").set("href","http://xdaq.web.cern.ch") << std::endl;
   */
  
  // set parameter callback
  std::string method = toolbox::toString("/%s/setParameter",getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::fieldset().set("style","font-size: 10pt; font-family: arial;") << std::endl;
  *out << cgicc::legend("Set the value of myParameter") << cgicc::p() << std::endl;
  *out << cgicc::form().set("method","GET").set("action", method) << "</br>" << std::endl;
  
  *out << "myParameter:" << cgicc::input().set("type","text"
					       ).set("name","value"
						     ).set("value", boost::str(boost::format("0x%X")%myParameter_)
							   ).set("size","10").set("maxlength","32") << "</br>" << std::endl;
  *out << std::endl;

  *out << "<br>testReg:" << cgicc::input().set("type","text"
					       ).set("name","testReg"
						     ).set("value", boost::str(boost::format("0x%X")%testReg_)
							   ).set("size","10").set("maxlength","32") << "</br>" << std::endl;
  *out << "</br>" << std::endl;

  *out << "<br>SystemID:" << cgicc::input().set("type","text"
						).set("name","systemID"
						      ).set("value", boost::str(boost::format("0x%X")%systemID_)
							    ).set("size","10").set("maxlength","32").set("readonly") << "</br>" << std::endl;
  *out << "</br>" << std::endl;

  *out << "<br>BoardID:" << cgicc::input().set("type","text"
					       ).set("name","boardID"
						     ).set("value", boost::str(boost::format("0x%X")%boardID_)
							   ).set("size","10").set("maxlength","32").set("readonly") << "</br>" << std::endl;
  *out << "</br>" << std::endl;

  *out << "<br>SystemFW:" << cgicc::input().set("type","text"
						).set("name","systemFirmwareID"
						      ).set("value", boost::str(boost::format("0x%X")%systemFirmwareID_)
							    ).set("size","10").set("maxlength","32").set("readonly") << "</br>" << std::endl;
  *out << "</br>" << std::endl;
  
  *out << cgicc::input().set("type","submit").set("value","Write").set("name","option") << std::endl;
  *out << cgicc::input().set("type","submit").set("value","Read").set("name","option") << std::endl;
  *out << cgicc::input().set("type","reset").set("value","Clear") << std::endl;
  *out << cgicc::form() << "</br>" << std::endl;
  *out << cgicc::fieldset();
  
  // print out basic information
  *out << "Current value of myParameter_ = "   << myParameter_                  << "</br>" << std::endl;
  *out << "System ID:               "          << formatSystemID(systemID_)     << "</br>" << std::endl;
  *out << "Board ID:                "          << formatBoardID(boardID_)       << "</br>" << std::endl;
  *out << "System firmware version: "          << formatFW(systemFirmwareID_,0) << "</br>" << std::endl;
  *out << "System firmware date:    "          << formatFW(systemFirmwareID_,1) << "</br>" << std::endl;
  *out << "System IP address:       "          << getIPAddress() << "</br>" << std::endl;
  //*out << "System MAC address:       "         << getMACAddress()<< "</br>" << std::endl;
  *out << "Current value of test register_ = " << testReg_ << "("
       << registerToChar(testReg_) << ")"  << "</br>" << std::endl;
}

void gemsupervisor::GemSupervisorWeb::setParameter(xgi::Input * in, xgi::Output * out)
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

      myAction_    = cgi["option"]->getValue();
      if (myAction_ == "Read")
      	this->Read(in,out);
      else if (myAction_ == "Write")
      	this->Write(in,out);
      else
	this->Default(in,out);
    }
  catch (const std::exception & e)
    {
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
}

void gemsupervisor::GemSupervisorWeb::Read(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(),"Reading parameters");
  try
    {
      cgicc::Cgicc cgi(in);
      this->getTestReg();      
      this->Default(in,out);
    }
  catch (const std::exception & e)
    {
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
}

void gemsupervisor::GemSupervisorWeb::Write(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(),"Writing value to myParameter_");
  try
    {
      cgicc::Cgicc cgi(in);
      myParameter_ = cgi["value"]->getIntegerValue();
      this->setTestReg(myParameter_);
      this->Default(in,out);
    }
  catch (const std::exception & e)
    {
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
}

void gemsupervisor::GemSupervisorWeb::getTestReg()
{
  //uhal::HwInterface hw=manager->getDevice ( "gemsupervisor.udp.0" );
  try {
    r_sysid   = hw->getNode ( "sysregs.system_id"   ).read();
    r_boardid = hw->getNode ( "sysregs.board_id"    ).read();
    r_fwid    = hw->getNode ( "sysregs.firmware_id" ).read();
    r_test    = hw->getNode ( "test"                ).read();

    r_ctrl    = hw->getNode ( "ctrl"                ).read();
    r_ctrl2   = hw->getNode ( "ctrl_2"              ).read();

    r_status    = hw->getNode ( "status"            ).read();
    r_status2   = hw->getNode ( "status_2"          ).read();
    
    r_ctrl_sram    = hw->getNode ( "ctrl_sram"      ).read();
    r_status_sram  = hw->getNode ( "status_sram"    ).read();
    
    r_spi_txdata  = hw->getNode ( "spi_txdata"  ).read();
    r_spi_command = hw->getNode ( "spi_command" ).read();
    r_spi_rxdata  = hw->getNode ( "spi_rxdata"  ).read();
    
    r_i2c_settings = hw->getNode ( "i2c_settings" ).read();
    r_i2c_command  = hw->getNode ( "i2c_command"  ).read();
    r_i2c_reply    = hw->getNode ( "i2c_reply"    ).read();
    
    r_sfp_phase_mon_ctrl  = hw->getNode ( "sfp_phase_mon_ctrl"  ).read();
    r_sfp_phase_mon_stats = hw->getNode ( "sfp_phase_mon_stats" ).read();
    
    r_fmc_phase_mon_ctrl  = hw->getNode ( "fmc1_phase_mon_ctrl"  ).read();
    r_fmc_phase_mon_stats = hw->getNode ( "fmc1_phase_mon_stats" ).read();
    
    r_mac_info1 = hw->getNode ( "mac_info1" ).read();
    r_mac_info2 = hw->getNode ( "mac_info2" ).read();
    r_ip_info   = hw->getNode ( "ip_info"   ).read();
    
    r_sram1 = hw->getNode ( "sram1" ).read();
    r_sram2 = hw->getNode ( "sram2" ).read();
    //r_icap  = hw->getNode ( "icap"  ).read();

    //unsigned int uregoffset = 0x40010000;
    //for (unsigned int ureg = 0; ureg < 16; ++ureg) {
    //  r_users.push_back(hw->getNode ( "test" ).read());
    //}
    hw->dispatch();
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong reading the registers: " << e.what());
    std::cout << "Something went wrong reading the registers: " << e.what() << std::endl;
  }

  LOG4CPLUS_INFO(this->getApplicationLogger(),"mac_info1: 0x" << std::hex << r_mac_info1.value() << std::dec);
  LOG4CPLUS_INFO(this->getApplicationLogger(),"mac_info2: 0x" << std::hex << r_mac_info2.value() << std::dec);
  LOG4CPLUS_INFO(this->getApplicationLogger(),"ip_info: 0x"   << std::hex << r_ip_info.value()   << std::dec);

  systemID_         = r_sysid.value();
  boardID_          = r_boardid.value();
  systemFirmwareID_ = r_fwid.value();
  testReg_          = r_test.value();
}

void gemsupervisor::GemSupervisorWeb::setTestReg(uint32_t myValue)
{
  //uhal::HwInterface hw=manager->getDevice ( "gemsupervisor.udp.0" );
  try {
    hw->getNode ( "test" ).write(myValue);
    hw->dispatch();
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong writing the test register: " << e.what());
  }
  
  try {
    r_test = hw->getNode ( "test" ).read();
    hw->dispatch();
    if (myValue != r_test.value())
      LOG4CPLUS_INFO(this->getApplicationLogger(),"Read back value does not match set value");
    testReg_ = r_test.value();
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong reading the test register: " << e.what());
  }
}

std::string gemsupervisor::GemSupervisorWeb::registerToChar(xdata::UnsignedInteger32 registerValue)
{
  std::string regToChars;
  char fourth = char(registerValue&0xff);
  char third  = char((registerValue>>8)&0xff);
  char second = char((registerValue>>16)&0xff);
  char first  = char((registerValue>>24)&0xff);
  
  regToChars += first;
  regToChars += second;
  regToChars += third;
  regToChars += fourth;
  return regToChars;
}

std::string gemsupervisor::GemSupervisorWeb::getIPAddress()
{
  uint32_t ipInfo;
  try {
    r_ip_info = hw->getNode ( "ip_info" ).read();
    hw->dispatch();
    ipInfo    = r_ip_info.value();
  }
  catch (uhalException& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong reading ip_info register: " << e.what());
    return "N/A";
  }
  
  uint32_t fourth = ipInfo&0xff;
  uint32_t third  = (ipInfo>>8)&0xff;
  uint32_t second = (ipInfo>>16)&0xff;
  uint32_t first  = (ipInfo>>24)&0xff;

  LOG4CPLUS_DEBUG(this->getApplicationLogger(),"ip_info: 0x" << std::hex << ipInfo << std::dec );
  LOG4CPLUS_DEBUG(this->getApplicationLogger(),"ip_addr: "   << boost::str(boost::format("%d.%d.%d.%d") % first % second % third % fourth) );
  
  return boost::str(boost::format("%d.%d.%d.%d") % first % second % third % fourth);
}

std::string gemsupervisor::GemSupervisorWeb::formatSystemID(xdata::UnsignedInteger32 myValue)
{
  return registerToChar(myValue);
}

std::string gemsupervisor::GemSupervisorWeb::formatBoardID(xdata::UnsignedInteger32 myValue)
{
  return registerToChar(myValue);
}

std::string gemsupervisor::GemSupervisorWeb::formatFW(xdata::UnsignedInteger32 myValue, int type)
{
  std::string fwVer = "";
  std::string fwDate = "";
  std::stringstream ss;
  std::string s;

  //the firmware register is broken into 6 chunks, 

  int fwMaj = (myValue>>28)&0x0f;
  int fwMin = (myValue>>24)&0x0f;
  int fwBui = (myValue>>16)&0xff;

  int fwYY = (myValue>>9)&0x7f;
  int fwMM = (myValue>>5)&0x0f;
  int fwDD = myValue&0x1f;
  
  
  if (type == 0)
    {
      //return the firmware version
      ss << fwMaj << "." << fwMin << "." << fwBui;
      ss >> s;
      fwVer = s;
      return fwVer;
    }
  else if (type == 1)
    {
      //return the firmware date
      ss << fwDD << "." << fwMM << "." << 2000+fwYY;
      ss >> s;
      fwDate = s;
      return fwDate;
    }
  else
    return "unsupported type";
}

