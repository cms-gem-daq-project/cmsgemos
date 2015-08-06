#include "gem/supervisor/GEMTestBeamSupervisorWeb.h"
#include <sstream>
#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::GEMTestBeamSupervisorWeb)

gem::supervisor::GEMTestBeamSupervisorWeb::GEMTestBeamSupervisorWeb(xdaq::ApplicationStub * s)
throw (xdaq::exception::Exception):
//gem::base::GEMApplication(s)
xdaq::WebApplication(s)
//, xgi::framework::UIManager(this)
{
  xgi::framework::deferredbind(this, this, &GEMTestBeamSupervisorWeb::controlPanel,      "controlPanel");
  xgi::framework::deferredbind(this, this, &GEMTestBeamSupervisorWeb::registerView,      "registerView");
  xgi::framework::deferredbind(this, this, &GEMTestBeamSupervisorWeb::monitoringWebPage, "monitoring");
  xgi::framework::deferredbind(this, this, &GEMTestBeamSupervisorWeb::monitoringWebPage, "Default");
  xgi::framework::deferredbind(this, this, &GEMTestBeamSupervisorWeb::thresholdScan,     "thresholdScan");
  xgi::framework::deferredbind(this, this, &GEMTestBeamSupervisorWeb::latencyScan,       "latencyScan");
  xgi::framework::deferredbind(this, this, &GEMTestBeamSupervisorWeb::daqWeb,            "daqWeb");

  this->getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");
  
  gem::supervisor::GEMTestBeamSupervisorWeb::initializeConnection();
}

void gem::supervisor::GEMTestBeamSupervisorWeb::actionPerformed (xdata::Event& event)
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


void gem::supervisor::GEMTestBeamSupervisorWeb::controlPanel(xgi::Input * in, xgi::Output * out )
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

void gem::supervisor::GEMTestBeamSupervisorWeb::Default(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  this->controlPanel(in,out);
}

void gem::supervisor::GEMTestBeamSupervisorWeb::registerView(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  this->controlPanel(in,out);
}

void gem::supervisor::GEMTestBeamSupervisorWeb::monitoringWebPage(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  this->controlPanel(in,out);
}

void gem::supervisor::GEMTestBeamSupervisorWeb::thresholdScan(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  this->controlPanel(in,out);
}

void gem::supervisor::GEMTestBeamSupervisorWeb::latencyScan(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  this->controlPanel(in,out);
}

void gem::supervisor::GEMTestBeamSupervisorWeb::daqWeb(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception)
{
  this->controlPanel(in,out);
}
