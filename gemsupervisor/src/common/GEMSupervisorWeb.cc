#include "gem/supervisor/GEMSupervisorWeb.h"
#include <sstream>
#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::GEMSupervisorWeb)

gem::supervisor::GEMSupervisorWeb::GEMSupervisorWeb(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception):
  //gem::base::GEMApplication(s)
  xdaq::WebApplication(s)
//, xgi::framework::UIManager(this)
{
  xgi::framework::deferredbind(this, this, &GEMSupervisorWeb::Default,       "Default"       );
  xgi::framework::deferredbind(this, this, &GEMSupervisorWeb::setParameter,  "setParameter"  );
  xgi::framework::deferredbind(this, this, &GEMSupervisorWeb::writeVFATRegs, "writeVFATRegs" );
  //xgi::framework::deferredbind(this, this, &GEMSupervisorWeb::writeUserRegs, "writeUserRegs" );
  xgi::framework::deferredbind(this, this, &GEMSupervisorWeb::Views,         "Views"         );
  xgi::framework::deferredbind(this, this, &GEMSupervisorWeb::Read,          "Read"          );
  xgi::framework::deferredbind(this, this, &GEMSupervisorWeb::Write,         "Write"         );

  myAction_    = "";
  myParameter_ = 0;
  vfatSleep_   = 1.0;

  //read-only registers may not need this treatment
  testReg_          = 0;
  boardID_          = 0;
  systemID_         = 0;
  systemFirmwareID_ = 0;
  userFirmwareID_   = 0;
  
  xreg_ctrl_        = 0;
  xreg_ctrl2_       = 0;
  xreg_status_      = 0;
  xreg_status2_     = 0;
  xreg_ctrl_sram_   = 0;
  xreg_status_sram_ = 0;

  xreg_spi_txdata_  = 0;
  xreg_spi_command_ = 0;
  xreg_spi_rxdata_  = 0;

  xreg_i2c_settings_ = 0;
  xreg_i2c_command_  = 0;
  xreg_i2c_reply_    = 0;

  xreg_sfp_phase_mon_ctrl_  = 0;
  xreg_sfp_phase_mon_stats_ = 0;
  xreg_fmc_phase_mon_ctrl_  = 0;
  xreg_fmc_phase_mon_stats_ = 0;
  
  xreg_mac_info1_ = 0;
  xreg_mac_info2_ = 0;
  xreg_ip_info_   = 0;
  
  xreg_sram1_ = 0;
  xreg_sram2_ = 0;
  xreg_icap_  = 0;
  
  //xreg_users_.reserve(100);  xreg_users_.clear();

  getApplicationInfoSpace()->fireItemAvailable("myParameter",      &myParameter_      );
  getApplicationInfoSpace()->fireItemAvailable("vfatSleep",        &vfatSleep_        );
  getApplicationInfoSpace()->fireItemAvailable("testReg",          &testReg_          );
  getApplicationInfoSpace()->fireItemAvailable("boardID",          &boardID_          );
  getApplicationInfoSpace()->fireItemAvailable("systemID",         &systemID_         );
  getApplicationInfoSpace()->fireItemAvailable("systemFirmwareID", &systemFirmwareID_ );
  getApplicationInfoSpace()->fireItemAvailable("userFirmwareID",   &userFirmwareID_   );

  getApplicationInfoSpace()->fireItemAvailable("xreg_ctrl"        , &xreg_ctrl_);
  getApplicationInfoSpace()->fireItemAvailable("xreg_ctrl2"       , &xreg_ctrl2_);
  getApplicationInfoSpace()->fireItemAvailable("xreg_status_"     , &xreg_status_);
  getApplicationInfoSpace()->fireItemAvailable("xreg_status2_"    , &xreg_status2_);
  getApplicationInfoSpace()->fireItemAvailable("xreg_ctrl_sram_"  , &xreg_ctrl_sram_);
  getApplicationInfoSpace()->fireItemAvailable("xreg_status_sram_", &xreg_status_sram_);

  getApplicationInfoSpace()->fireItemAvailable("xreg_spi_txdata_" , &xreg_spi_txdata_);
  getApplicationInfoSpace()->fireItemAvailable("xreg_spi_command_", &xreg_spi_command_);
  getApplicationInfoSpace()->fireItemAvailable("xreg_spi_rxdata_" , &xreg_spi_rxdata_);

  getApplicationInfoSpace()->fireItemAvailable("xreg_i2c_settings_", &xreg_i2c_settings_);
  getApplicationInfoSpace()->fireItemAvailable("xreg_i2c_command_" , &xreg_i2c_command_);
  getApplicationInfoSpace()->fireItemAvailable("xreg_i2c_reply_"   , &xreg_i2c_reply_);

  getApplicationInfoSpace()->fireItemAvailable("xreg_sfp_phase_mon_ctrl_" , &xreg_sfp_phase_mon_ctrl_);
  getApplicationInfoSpace()->fireItemAvailable("xreg_sfp_phase_mon_stats_", &xreg_sfp_phase_mon_stats_);
  getApplicationInfoSpace()->fireItemAvailable("xreg_fmc_phase_mon_ctrl_" , &xreg_fmc_phase_mon_ctrl_);
  getApplicationInfoSpace()->fireItemAvailable("xreg_fmc_phase_mon_stats_", &xreg_fmc_phase_mon_stats_);
  						                     
  getApplicationInfoSpace()->fireItemAvailable("xreg_mac_info1_", &xreg_mac_info1_);
  getApplicationInfoSpace()->fireItemAvailable("xreg_mac_info2_", &xreg_mac_info2_);
  getApplicationInfoSpace()->fireItemAvailable("xreg_ip_info_"  , &xreg_ip_info_);
  						                     
  getApplicationInfoSpace()->fireItemAvailable("xreg_sram1_", &xreg_sram1_);
  getApplicationInfoSpace()->fireItemAvailable("xreg_sram2_", &xreg_sram2_);
  getApplicationInfoSpace()->fireItemAvailable("xreg_icap_" , &xreg_icap_);
  
  //std::vector<xdata::UnsignedInteger32>::const_iterator uiter = xreg_users_.begin();
  //unsigned int ureg = 0;
  ////for (; uiter != xreg_users_.end(); ++uiter,++ureg)  {
  //for (; ureg < 100; ++ureg)  {
  //  xreg_users_.push_back(0);
  //  getApplicationInfoSpace()->fireItemAvailable(boost::str(boost::format("xreg_user_%x")%ureg),
  //						 &(xreg_users_.at(ureg)));
  //}

  // Detect when the setting of default parameters has been performed
  this->getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");
  
  gem::supervisor::GEMSupervisorWeb::initializeConnection();
}

void gem::supervisor::GEMSupervisorWeb::actionPerformed (xdata::Event& event)
{
  if ( event.type() == "urn:xdaq-event:setDefaultValues" )
    {
      std::stringstream ss;
      ss << "myParameter=[" << myParameter_ << "]" << std::endl;
      ss.str(std::string());
      ss << "vfatSleep=[" << vfatSleep_ << "]" << std::endl;
      
      /*
      for ( std::vector<xdata::UnsignedInteger>::size_t i = 0;  i != myVector_.size() ; i++ )
	{
	  ss << "myVector=[" << i << "]=[" << myVector_[i] << "]"      << std::endl;
	}
      */
      LOG4CPLUS_INFO(this->getApplicationLogger(), ss.str());
    }
}

void gem::supervisor::GEMSupervisorWeb::initializeConnection() 
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
    hw = new uhal::HwInterface(manager->getDevice("gemsupervisor.controlhub.0"));
  }
  catch (uhalException& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong initializing the connection: " << e.what());
    //throw exception(std::string("IPbus Creation Failure"));
  }
  
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong initializing the connection: " << e.what());
    std::cout << "Something went wrong initializing the connection: " << e.what() << std::endl;
  }
  
  vfatNodes = hw->getNodes("user_regs.vfats.CMS_hybrid_J8.*");
  vfatNodes.erase( std::remove(vfatNodes.begin(),
			       vfatNodes.end(),
			       "user_regs.vfats.CMS_hybrid_J8"),
		   vfatNodes.end() );

  //vfatNodes.erase("user_regs.vfats.CMS_hybrid_J8");
  std::vector<std::string>::const_iterator node  = vfatNodes.begin();
  std::cout << "getNodes(): ";
  std::copy(vfatNodes.begin(),vfatNodes.end(),std::ostream_iterator<std::string>(std::cout,", "));
  for (; node != vfatNodes.end(); ++node) {
    xreg_vfats_[*node] = 0;
    getApplicationInfoSpace()->fireItemAvailable(boost::str(boost::format("xreg_vfat_%x")%(*node)),
						 &(xreg_vfats_[*node]));
    
    //uint32_t r_value = (vfatDevice->hw->read(*node));
    //r_vfats[*node]  = r_value;
    //vfatRegs[*node] = r_value&0x000000ff;
  }
  
  this->getTestReg();
  
}


void gem::supervisor::GEMSupervisorWeb::Views(xgi::Input * in, xgi::Output * out )
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

  *out << "  <div class=\"xdaq-tab\" title=\"VFAT Registers\" >"     << std::endl;
  
  std::string method = toolbox::toString("/%s/writeVFATRegs",getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::fieldset().set("style","font-size: 10pt; font-family: arial;") << std::endl;
  *out << cgicc::legend("Read/Write VFAT registers") << cgicc::p() << std::endl;
  *out << cgicc::form().set("method","GET").set("action", method) << "</br>" << std::endl;
  
  *out << "    <table class=\"xdaq-table\" caption=\"VFAT Registers\" >"     << std::endl;
  *out << "      <thead>" << std::endl;
  *out << "        <tr>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Register name</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">IPBus Read value</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">VFAT Read value</th>" << std::endl;
  *out << "          <th class=\"xdaq-sortable\">Value to write</th>" << std::endl;
  //*out << "          <th class=\"xdaq-sortable\">Execute</th>" << std::endl;
  *out << "        </tr>" << std::endl;
  *out << "      </thead>" << std::endl;
  *out << "      </br>" << std::endl;
  
  *out << "      <tbody>" << std::endl;
  
  std::vector<std::string>::const_iterator node  = vfatNodes.begin();
  for (; node != vfatNodes.end(); ++node) {
    *out << "      <tr>"     << std::endl;
    *out << "        <td>"   << *node << "</td>"  << std::endl;
    *out << "        <td>0x" << std::setfill('0') << std::setw(8)
      //<< std::hex << r_vfats[*node].value() 
	 << std::hex << xreg_vfats_[*node]
	 << std::dec << "</td>"                   << std::endl;
    *out << "        <td>0x" << std::setfill('0') << std::setw(2)
	 << std::hex << (xreg_vfats_[*node] &0x000000ff)
	 << std::dec << "</td>"                   << std::endl;
    if ( !(hw->getNode(*node).getPermission() == uhal::defs::READ) )
      *out << "        <td>" << cgicc::input().set("type","text"
						   ).set("name",(*node)
							 ).set("value", boost::str(boost::format("0x%02x")%(xreg_vfats_[*node]&0x000000ff))
							       ).set("size","10").set("maxlength","32");
    else
      *out << "        <td>" << cgicc::input().set("type","text"
						   ).set("name",(*node)
							 ).set("value", boost::str(boost::format("0x%02x")%(xreg_vfats_[*node]&0x000000ff))
							       ).set("size","10").set("maxlength","32").set("readonly");
    *out << "        </td>" << std::endl;
    *out << "      </tr>" << std::endl;
  }

  *out << "      </br>" << std::endl;
  *out << "      </tbody>" << std::endl;
  *out << cgicc::input().set("type","submit").set("value","Write") << std::endl;
  *out << cgicc::input().set("type","reset").set("value","Clear") << std::endl;
  *out << "    </table>" << std::endl;
  *out << "    </br>" << std::endl;
  *out << "  </div>" << std::endl;
  /****************************/

  //*out << "  <div class=\"xdaq-tab\" title=\"GLIB User Registers\" >"     << std::endl;
  //
  //std::string method = toolbox::toString("/%s/writeUserRegs",getApplicationDescriptor()->getURN().c_str());
  //*out << cgicc::fieldset().set("style","font-size: 10pt; font-family: arial;") << std::endl;
  //*out << cgicc::legend("Read/Write VFAT user registers") << cgicc::p() << std::endl;
  //*out << cgicc::form().set("method","GET").set("action", method) << "</br>" << std::endl;
  //
  //*out << "    <table class=\"xdaq-table\" caption=\"GLIB User Registers\" >"     << std::endl;
  //*out << "      <thead>" << std::endl;
  //*out << "        <tr>" << std::endl;
  //*out << "          <th class=\"xdaq-sortable\">Register name</th>" << std::endl;
  //*out << "          <th class=\"xdaq-sortable\">Read value</th>" << std::endl;
  //*out << "          <th class=\"xdaq-sortable\">Value to write</th>" << std::endl;
  ////*out << "          <th class=\"xdaq-sortable\">Execute</th>" << std::endl;
  //*out << "        </tr>" << std::endl;
  //*out << "      </thead>" << std::endl;
  //*out << "      </br>" << std::endl;
  //
  //*out << "      <tbody>" << std::endl;
  //unsigned int uregoffset = 0x40010000;
  //boost::format uregform("ureg0x%x");
  //for (unsigned int ureg = 0; ureg < 16; ++ureg) {
  //  *out << "      <tr>" << std::endl;
  //  *out << "        <td>User register 0x" << std::hex << uregoffset+(ureg<<0x8) << std::dec << "</td>" << std::endl;
  //  *out << "        <td>0x" << std::setfill('0') << std::setw(8) << std::hex << r_users.at(ureg).value() << std::dec << "</td>" << std::endl;
  //  *out << "        <td>" << cgicc::input().set("type","text"
  //						 ).set("name",boost::str(uregform%ureg)
  //						       ).set("value", boost::str(boost::format("0x%08x")%xreg_users_.at(ureg))
  //							     ).set("size","10").set("maxlength","32")
  //	 << "        </td>" << std::endl;
  //  *out << "      </tr>" << std::endl;
  //}
  //*out << "      </br>" << std::endl;
  //*out << "      </tbody>" << std::endl;
  //*out << cgicc::input().set("type","submit").set("value","Write") << std::endl;
  //*out << cgicc::input().set("type","reset").set("value","Clear") << std::endl;
  //*out << "    </table>" << std::endl;
  //*out << "    </br>" << std::endl;
  //*out << "  </div>" << std::endl;

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

void gem::supervisor::GEMSupervisorWeb::Default(xgi::Input * in, xgi::Output * out )
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
  *out << cgicc::legend("Set the parameters") << cgicc::p() << std::endl;
  *out << cgicc::form().set("method","GET").set("action", method) << "</br>" << std::endl;
  
  *out << "myParameter:" << cgicc::input().set("type","number"
					       ).set("name","myParam"
						     ).set("value", boost::str(boost::format("0x%08x")%myParameter_)
							   ).set("size","10").set("maxlength","32") << "</br>" << std::endl;
  *out << std::endl;

  *out << "vfatSleep:" << cgicc::input().set("type","number"
					     ).set("name","vfatSleep"
						   ).set("value", boost::str(boost::format("%f")%vfatSleep_)
							 ).set("size","10").set("maxlength","32") << "</br>" << std::endl;
  *out << std::endl;

  *out << "<br>testReg:" << cgicc::input().set("type","number"
					       ).set("name","testReg"
						     ).set("value", boost::str(boost::format("0x%08x")%testReg_)
							   ).set("size","10").set("maxlength","32") << "</br>" << std::endl;
  *out << "</br>" << std::endl;

  *out << "<br>SystemID:" << cgicc::input().set("type","number"
						).set("name","systemID"
						      ).set("value", boost::str(boost::format("0x%08x")%systemID_)
							    ).set("size","10").set("maxlength","32").set("readonly") << "</br>" << std::endl;
  *out << "</br>" << std::endl;

  *out << "<br>BoardID:" << cgicc::input().set("type","number"
					       ).set("name","boardID"
						     ).set("value", boost::str(boost::format("0x%08x")%boardID_)
							   ).set("size","10").set("maxlength","32").set("readonly") << "</br>" << std::endl;
  *out << "</br>" << std::endl;

  *out << "<br>SystemFW:" << cgicc::input().set("type","number"
						).set("name","systemFirmwareID"
						      ).set("value", boost::str(boost::format("0x%08x")%systemFirmwareID_)
							    ).set("size","10").set("maxlength","32").set("readonly") << "</br>" << std::endl;
  *out << "</br>" << std::endl;
  
  *out << cgicc::input().set("type","submit").set("value","Write").set("name","option") << std::endl;
  *out << cgicc::input().set("type","submit").set("value","Read").set("name","option") << std::endl;
  *out << cgicc::input().set("type","reset").set("value","Clear") << std::endl;
  *out << cgicc::form() << "</br>" << std::endl;
  *out << cgicc::fieldset();
  
  // print out basic information
  *out << "Current value of myParameter_ = "   << myParameter_                  << "</br>" << std::endl;
  *out << "Current value of vfatSleep_ =   "   << vfatSleep_                    << "</br>" << std::endl;
  *out << "System ID:               "          << formatSystemID(systemID_)     << "</br>" << std::endl;
  *out << "Board ID:                "          << formatBoardID(boardID_)       << "</br>" << std::endl;
  *out << "System firmware version: "          << formatFW(systemFirmwareID_,0) << "</br>" << std::endl;
  *out << "System firmware date:    "          << formatFW(systemFirmwareID_,1) << "</br>" << std::endl;
  *out << "System IP address:       "          << getIPAddress() << "</br>" << std::endl;
  //*out << "System MAC address:       "         << getMACAddress()<< "</br>" << std::endl;
  *out << "Current value of test register_ = " << testReg_ << "("
       << registerToChar(testReg_) << ")"  << "</br>" << std::endl;
}

void gem::supervisor::GEMSupervisorWeb::setParameter(xgi::Input * in, xgi::Output * out)
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

void gem::supervisor::GEMSupervisorWeb::Read(xgi::Input * in, xgi::Output * out )
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

void gem::supervisor::GEMSupervisorWeb::Write(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(),"Writing parameter values");
  try
    {
      cgicc::Cgicc cgi(in);
      std::stringstream stream;
      stream << cgi["vfatSleep"]->getValue();
      float sleepVal;
      stream >> sleepVal;
      vfatSleep_ = sleepVal;
      
      stream.str(std::string());
      stream << cgi["myParam"]->getValue();
      uint writeVal;
      stream >> std::hex >> writeVal >> std::dec;
      myParameter_ = writeVal;
      //myParameter_ = cgi["myParam"]->getIntegerValue();
      this->setTestReg(myParameter_);
      this->Default(in,out);
    }
  catch (const std::exception & e)
    {
      XCEPT_RAISE(xgi::exception::Exception, e.what());
    }
}

void gem::supervisor::GEMSupervisorWeb::writeVFATRegs(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  LOG4CPLUS_INFO(this->getApplicationLogger(),"Writing values to vfat registers");
  
  try {
    cgicc::Cgicc cgi(in);
    /*
      cgicc::const_form_iterator cgiter = cgi.getElements().begin();
      LOG4CPLUS_INFO(this->getApplicationLogger(),"looping over form values");
      for (;cgiter != cgi.getElements().end(); ++cgiter) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"form name " << cgiter->getName());
      LOG4CPLUS_INFO(this->getApplicationLogger(),"form value " << cgiter->getValue());
      LOG4CPLUS_INFO(this->getApplicationLogger(),"form value " << cgiter->getIntegerValue());
      }
    */
    std::vector<std::string>::const_iterator node  = vfatNodes.begin();
    for (; node != vfatNodes.end(); ++node) {
      std::stringstream stream;
      stream << cgi[*node]->getValue();
      uint writeVal;
      stream >> std::hex >> writeVal >> std::dec;
      LOG4CPLUS_INFO(this->getApplicationLogger(),"filling register " << *node);
      LOG4CPLUS_INFO(this->getApplicationLogger(),"sees string as "   << cgi[*node]->getValue());
      LOG4CPLUS_INFO(this->getApplicationLogger(),"sees integer as "  << cgi[*node]->getIntegerValue());
      LOG4CPLUS_INFO(this->getApplicationLogger(),"with value "       << writeVal);
      if ( !(hw->getNode(*node).getPermission() == uhal::defs::READ) )
	hw->getNode(*node ).write(writeVal);
    }
    hw->dispatch();
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong writing the vfat register: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
    
  try {
    cgicc::Cgicc cgi(in);
    std::vector<std::string>::const_iterator node  = vfatNodes.begin();
    for (; node != vfatNodes.end(); ++node) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"Read executed on vfat register " << (*node)
		     << ".  Sleep value is " << vfatSleep_);
      r_vfats[*node] = hw->getNode(*node).read();
      hw->dispatch();
      sleep(vfatSleep_);
      LOG4CPLUS_INFO(this->getApplicationLogger(),"Read executed on vfat_response register"
		     << ".  Sleep value is " << vfatSleep_);
      r_vfats[*node] = hw->getNode("user_regs.vfats.vfat_response").read();
      hw->dispatch();
      sleep(vfatSleep_);
    }

    node  = vfatNodes.begin();
    for (; node != vfatNodes.end(); ++node) {
      std::stringstream stream;
      stream << cgi[*node]->getValue();
      uint writeVal;
      stream >> std::hex >> writeVal >> std::dec;
      if (writeVal != ((r_vfats[*node].value())&0x000000ff) ) {
	boost::format logmessage("Read back value 0x%02x does not match set value 0x%02x on register %s");
	LOG4CPLUS_INFO(this->getApplicationLogger(),boost::str(logmessage % r_vfats[*node].value() % writeVal % (*node)));
      }
      xreg_vfats_[*node] = r_vfats[*node].value();
    }
    this->Views(in,out);

  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong reading the vfat register: " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

//void gem::supervisor::GEMSupervisorWeb::writeUserRegs(xgi::Input * in, xgi::Output * out )
//  throw (xgi::exception::Exception)
//{
//  LOG4CPLUS_INFO(this->getApplicationLogger(),"Writing values to user registers");
//  
//  boost::format uregform("vfat.ureg0x%x");
//  boost::format writeform("ureg0x%x");
//  
//  try {
//    cgicc::Cgicc cgi(in);
//    /*
//    cgicc::const_form_iterator cgiter = cgi.getElements().begin();
//    LOG4CPLUS_INFO(this->getApplicationLogger(),"looping over form values");
//    for (;cgiter != cgi.getElements().end(); ++cgiter) {
//      LOG4CPLUS_INFO(this->getApplicationLogger(),"form name " << cgiter->getName());
//      LOG4CPLUS_INFO(this->getApplicationLogger(),"form value " << cgiter->getValue());
//      LOG4CPLUS_INFO(this->getApplicationLogger(),"form value " << cgiter->getIntegerValue());
//    }
//    */
//    for (unsigned int ureg = 0; ureg < 16; ++ureg) {
//      //int writeVal = atoi(cgi[boost::str(writeform%ureg)]->getValue().c_str());
//      std::stringstream stream;
//      stream << cgi[boost::str(writeform%ureg)]->getValue();
//      uint writeVal;
//      stream >> std::hex >> writeVal >> std::dec;
//      LOG4CPLUS_INFO(this->getApplicationLogger(),"filling register " << boost::str(writeform % ureg));
//      LOG4CPLUS_INFO(this->getApplicationLogger(),"sees string as "   << cgi[boost::str(writeform % ureg)]->getValue());
//      LOG4CPLUS_INFO(this->getApplicationLogger(),"sees integer as "  << cgi[boost::str(writeform % ureg)]->getIntegerValue());
//      LOG4CPLUS_INFO(this->getApplicationLogger(),"with value "       << writeVal);
//      hw->getNode ( boost::str(uregform%ureg) ).write(writeVal);
//    }
//    hw->dispatch();
//  }
//  catch (const std::exception& e) {
//    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong writing the user register: " << e.what());
//    XCEPT_RAISE(xgi::exception::Exception, e.what());
//  }
//    
//  try {
//    cgicc::Cgicc cgi(in);
//    for (unsigned int ureg = 0; ureg < 16; ++ureg)
//      r_users.at(ureg) = hw->getNode ( boost::str(uregform%ureg) ).read();
//    hw->dispatch();
//    for (unsigned int ureg = 0; ureg < 16; ++ureg) {
//      std::stringstream stream;
//      stream << cgi[boost::str(writeform%ureg)]->getValue();
//      uint writeVal;
//      stream >> std::hex >> writeVal >> std::dec;
//      //uint32_t writeVal = cgi[boost::str(writeform%ureg)]->getIntegerValue();
//      if (writeVal != r_users.at(ureg).value()) {
//	boost::format logmessage("Read back value 0x%08x does not match set value 0x%08x on register %s");
//	LOG4CPLUS_INFO(this->getApplicationLogger(),boost::str(logmessage % r_users.at(ureg).value() % writeVal % boost::str(uregform%ureg)));
//      }
//      xreg_users_.at(ureg) = r_users.at(ureg).value();
//    }
//    this->Views(in,out);
//
//  }
//  catch (const std::exception& e) {
//    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong reading the user register: " << e.what());
//    XCEPT_RAISE(xgi::exception::Exception, e.what());
//  }
//}

  void gem::supervisor::GEMSupervisorWeb::getTestReg()
{
  //uhal::HwInterface hw=manager->getDevice ( "gemsupervisor.udp.0" );
  try {
    
    LOG4CPLUS_INFO(this->getApplicationLogger(),"reading the GLIB registers");
    
    r_sysid   = hw->getNode ( "glib_regs.sysregs.system_id"   ).read();
    r_boardid = hw->getNode ( "glib_regs.sysregs.board_id"    ).read();
    r_fwid    = hw->getNode ( "glib_regs.sysregs.firmware_id" ).read();
    r_test    = hw->getNode ( "glib_regs.test"                ).read();

    r_ctrl    = hw->getNode ( "glib_regs.ctrl"                ).read();
    r_ctrl2   = hw->getNode ( "glib_regs.ctrl_2"              ).read();

    r_status    = hw->getNode ( "glib_regs.status"            ).read();
    r_status2   = hw->getNode ( "glib_regs.status_2"          ).read();
    
    r_ctrl_sram    = hw->getNode ( "glib_regs.ctrl_sram"      ).read();
    r_status_sram  = hw->getNode ( "glib_regs.status_sram"    ).read();
    
    r_spi_txdata  = hw->getNode ( "glib_regs.spi_txdata"  ).read();
    r_spi_command = hw->getNode ( "glib_regs.spi_command" ).read();
    r_spi_rxdata  = hw->getNode ( "glib_regs.spi_rxdata"  ).read();
    
    r_i2c_settings = hw->getNode ( "glib_regs.i2c_settings" ).read();
    r_i2c_command  = hw->getNode ( "glib_regs.i2c_command"  ).read();
    r_i2c_reply    = hw->getNode ( "glib_regs.i2c_reply"    ).read();
    
    r_sfp_phase_mon_ctrl  = hw->getNode ( "glib_regs.sfp_phase_mon_ctrl"  ).read();
    r_sfp_phase_mon_stats = hw->getNode ( "glib_regs.sfp_phase_mon_stats" ).read();
    
    r_fmc_phase_mon_ctrl  = hw->getNode ( "glib_regs.fmc1_phase_mon_ctrl"  ).read();
    r_fmc_phase_mon_stats = hw->getNode ( "glib_regs.fmc1_phase_mon_stats" ).read();
    
    r_mac_info1 = hw->getNode ( "glib_regs.mac_info1" ).read();
    r_mac_info2 = hw->getNode ( "glib_regs.mac_info2" ).read();
    r_ip_info   = hw->getNode ( "glib_regs.ip_info"   ).read();
    
    r_sram1 = hw->getNode ( "glib_regs.sram1" ).read();
    r_sram2 = hw->getNode ( "glib_regs.sram2" ).read();
    //r_icap  = hw->getNode ( "glib_regs.icap"  ).read();
    
    //boost::format uregform("vfat.ureg0x%x");
    //for (unsigned int ureg = 0; ureg < 16; ++ureg) {
    //  r_users.push_back(hw->getNode ( boost::str(uregform%ureg) ).read());
    //}

    hw->dispatch();
    LOG4CPLUS_INFO(this->getApplicationLogger(),"done reading the GLIB registers");
  }

  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong reading the registers: " << e.what());
    std::cout << "Something went wrong reading the registers: " << e.what() << std::endl;
  }

  LOG4CPLUS_INFO(this->getApplicationLogger(),"reading the VFAT registers");
  int count = 0;
  std::vector<std::string>::const_iterator node  = vfatNodes.begin();
  for (; node != vfatNodes.end(); ++node) {
    ++count;
    try {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"Read executed on vfat register " << (*node)
		     << ".  Sleep value is " << vfatSleep_);
      r_vfats[*node] = hw->getNode(*node).read();
      hw->dispatch();
      sleep(vfatSleep_);
      LOG4CPLUS_INFO(this->getApplicationLogger(),"Read executed on vfat_response register"
		     << ".  Sleep value is " << vfatSleep_);
      r_vfats[*node] = hw->getNode("user_regs.vfats.vfat_response").read();
      hw->dispatch();
      sleep(vfatSleep_);
    }
    catch (uhal::exception::exception const& e) {
      LOG4CPLUS_INFO(this->getApplicationLogger(), "uhal::exception: " << count << "/" 
		     << vfatNodes.size() << ") Something went wrong reading the VFAT register "
		     << (*node) << ": " << e.what());
      std::cout << "uhal::exception: " << count << "/"
		<< vfatNodes.size() << ") Something went wrong reading the VFAT register "
		<< (*node) << ": " << e.what() << std::endl;
    }
    catch (std::exception const& e) {
      LOG4CPLUS_INFO(this->getApplicationLogger(), "std::exception: " << count << "/"
		     << vfatNodes.size() << ") Something went wrong reading the VFAT register "
		     << (*node) << ": " << e.what());
      std::cout << "std::exception: " << count << "/"
		<< vfatNodes.size() << ") Something went wrong reading the VFAT register "
		<< (*node) << ": " << e.what() << std::endl;
    }
  }
  LOG4CPLUS_INFO(this->getApplicationLogger(),"done reading the VFAT registers");
  
  LOG4CPLUS_INFO(this->getApplicationLogger(),"mac_info1: 0x" << std::hex << r_mac_info1.value() << std::dec);
  LOG4CPLUS_INFO(this->getApplicationLogger(),"mac_info2: 0x" << std::hex << r_mac_info2.value() << std::dec);
  LOG4CPLUS_INFO(this->getApplicationLogger(),"ip_info: 0x"   << std::hex << r_ip_info.value()   << std::dec);

  systemID_         = r_sysid.value();
  boardID_          = r_boardid.value();
  systemFirmwareID_ = r_fwid.value();
  testReg_          = r_test.value();

  xreg_ctrl_        = r_ctrl.value();                 
  xreg_ctrl2_       = r_ctrl2.value();                
  xreg_status_      = r_status.value();               
  xreg_status2_     = r_status2.value();              
  xreg_ctrl_sram_   = r_ctrl_sram.value();            
  xreg_status_sram_ = r_status_sram.value();          

  xreg_spi_txdata_  = r_spi_txdata.value();           
  xreg_spi_command_ = r_spi_command.value();          
  xreg_spi_rxdata_  = r_spi_rxdata.value();           

  xreg_i2c_settings_ = r_i2c_settings.value();         
  xreg_i2c_command_  = r_i2c_command.value();          
  xreg_i2c_reply_    = r_i2c_reply.value();            

  xreg_sfp_phase_mon_ctrl_  = r_sfp_phase_mon_ctrl.value();   
  xreg_sfp_phase_mon_stats_ = r_sfp_phase_mon_stats.value();  
  xreg_fmc_phase_mon_ctrl_  = r_fmc_phase_mon_ctrl.value();   
  xreg_fmc_phase_mon_stats_ = r_fmc_phase_mon_stats.value();  
  				                           
  xreg_mac_info1_ = r_mac_info1.value();            
  xreg_mac_info2_ = r_mac_info2.value();            
  xreg_ip_info_   = r_ip_info.value();              
  				                           
  xreg_sram1_ = r_sram1.value();
  xreg_sram2_ = r_sram2.value();
  //xreg_icap_  = r_icap.value();
  
  //LOG4CPLUS_INFO(this->getApplicationLogger(),"xreg_users_.size:" << xreg_users_.size());
  //for (unsigned int ureg = 0; ureg < 16; ++ureg)
  //  xreg_users_.at(ureg) = r_users.at(ureg).value();
  node  = vfatNodes.begin();
  for (; node != vfatNodes.end(); ++node) {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(),"storing r_vfats[" << *node << "].value() in xreg_vfats_[" << *node << "]\n");
    std::cout << "storing r_vfats[" << *node << "].value() in xreg_vfats_[" << *node << "]" << std::endl;
    xreg_vfats_[*node] = r_vfats[*node].value();
  }
}

void gem::supervisor::GEMSupervisorWeb::setTestReg(uint32_t myValue)
{
  //uhal::HwInterface hw=manager->getDevice ( "gemsupervisor.udp.0" );
  try {
    hw->getNode ( "glib_regs.test" ).write(myValue);
    hw->dispatch();
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong writing the test register: " << e.what());
  }
  
  try {
    r_test = hw->getNode ( "glib_regs.test" ).read();
    hw->dispatch();
    if (myValue != r_test.value())
      LOG4CPLUS_INFO(this->getApplicationLogger(),"Read back value does not match set value");
    testReg_ = r_test.value();
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong reading the test register: " << e.what());
  }
}

std::string gem::supervisor::GEMSupervisorWeb::registerToChar(xdata::UnsignedInteger32 registerValue)
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

std::string gem::supervisor::GEMSupervisorWeb::getIPAddress()
{
  uint32_t ipInfo;
  try {
    r_ip_info = hw->getNode ( "glib_regs.ip_info" ).read();
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

  LOG4CPLUS_INFO(this->getApplicationLogger(),"ip_info: 0x" << std::hex << ipInfo << std::dec );
  LOG4CPLUS_INFO(this->getApplicationLogger(),"ip_addr: "   << boost::str(boost::format("%d.%d.%d.%d") % first % second % third % fourth) );
  
  return boost::str(boost::format("%d.%d.%d.%d") % first % second % third % fourth);
}
 
std::string gem::supervisor::GEMSupervisorWeb::formatSystemID(xdata::UnsignedInteger32 myValue)
{
  return registerToChar(myValue);
}
 
std::string gem::supervisor::GEMSupervisorWeb::formatBoardID(xdata::UnsignedInteger32 myValue)
{
  return registerToChar(myValue);
}

std::string gem::supervisor::GEMSupervisorWeb::formatFW(xdata::UnsignedInteger32 myValue, int type)
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
