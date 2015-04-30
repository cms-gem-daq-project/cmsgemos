// AMC13ManagerWeb.cc

#include "gem/hw/amc13/AMC13ManagerWeb.h"
#include "gem/hw/amc13/AMC13Manager.h"

#include "xcept/tools.h"
#include "amc13/AMC13.hh"
#include "amc13/Status.hh"

gem::hw::amc13::AMC13ManagerWeb::AMC13ManagerWeb(gem::hw::amc13::AMC13Manager* amc13App) :
  gem::base::GEMWebApplication(amc13App)
{
  //amc13ManagerP_ = amc13App;
}

//is
gem::hw::amc13::AMC13ManagerWeb::~AMC13ManagerWeb()
{

}

/*To be filled in with the monitor page code*/
void gem::hw::amc13::AMC13ManagerWeb::monitorPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("monitorPage");
  //fill this page with the generic views for the AMC13Manager
  //different tabs for certain functions?
  size_t level = 2;

  //pre process the form for the debug level
  try {
    cgicc::Cgicc cgi(in);
    int radio_i       = cgi["level"]->getIntegerValue();
    LOG4CPLUS_DEBUG(gemFSMAppP_->getApplicationLogger(),"current level is "      << level);
    LOG4CPLUS_DEBUG(gemFSMAppP_->getApplicationLogger(),"radio button value is " << radio_i);
    level = static_cast<size_t>(radio_i);
    LOG4CPLUS_DEBUG(gemFSMAppP_->getApplicationLogger(),"setting AMC13 status info to " << level);
  } catch (const xgi::exception::Exception& e) {
    level = 2;
    LOG4CPLUS_WARN(gemFSMAppP_->getApplicationLogger(),"Caught xgi::exception " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception& e) {
    level = 2;
    LOG4CPLUS_WARN(gemFSMAppP_->getApplicationLogger(),"Caught std::exception " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  
  //form and control to set the level of information
  std::string method = toolbox::toString("/%s/monitorView",gemFSMAppP_->getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::form().set("method","POST").set("action",method) << std::endl;
  
  *out << cgicc::section().set("style","display:inline-block;float:left")       << std::endl
       << cgicc::fieldset().set("style","display:block;padding:5px;margin:5px;list-style-type:none;margin-bottom:5px;line-height:18px;padding:2px 5px;-webkit-border-radius:5px;-moz-border-radius:5px;border-radius:5px;border:medium outset #CCC;")
       << std::endl
       << cgicc::legend("AMC13 Status")                                         << std::endl
       << cgicc::br()                                                           << std::endl
       << cgicc::div().set("align","center")                                    << std::endl

       << (level == 0 ?
	   cgicc::input().set("type","radio").set("name","level").set("value","0").set("checked") :
	   cgicc::input().set("type","radio").set("name","level").set("value","0"))
       << "version" << std::endl
       << (level == 1 ?
	   cgicc::input().set("type","radio").set("name","level").set("value","1").set("checked") :
	   cgicc::input().set("type","radio").set("name","level").set("value","1"))
       << "minimum" << std::endl
       << (level == 2 ?
	   cgicc::input().set("type","radio").set("name","level").set("value","2").set("checked") :
	   cgicc::input().set("type","radio").set("name","level").set("value","2"))
       << "default" << std::endl
       << (level == 3 ?
	   cgicc::input().set("type","radio").set("name","level").set("value","3").set("checked") :
	   cgicc::input().set("type","radio").set("name","level").set("value","3"))
       << "maximum" << std::endl
       << cgicc::input().set("style","display:inline-block;margin-right:25px;margin-left:25px;float:center;-webkit-border-radius: 5px;-moz-border-radius: 5px;border-radius: 5px;border:medium outset #CCC;")
    .set("type","submit")
    .set("value","Set level")
    .set("name","setLevel") << std::endl
       << cgicc::br()       << std::endl
       << cgicc::div()      << std::endl
       << cgicc::form()   << std::endl
       << cgicc::span().set("style","display:block;float:left") << std::endl;
  
  //just print out the status paget that the AMC13 generates

  //::amc13::Status *s = amc13ManagerP_->getHTMLStatus();
  ::amc13::Status *s = dynamic_cast<gem::hw::amc13::AMC13Manager*>(gemFSMAppP_)->getAMC13Device()->getStatus();
  s->SetHTML();
  s->Report(level,*out);
  *out << cgicc::span()     << std::endl;
  *out << cgicc::fieldset() << std::endl;
  *out << cgicc::section()  << std::endl;
  
}

/*To be filled in with the expert page code*/
void gem::hw::amc13::AMC13ManagerWeb::expertPage(xgi::Input * in, xgi::Output * out)
  throw (xgi::exception::Exception)
{
  INFO("expertPage");
  //fill this page with the expert views for the AMC13Manager
  *out << "expertPage</br>" << std::endl;
}

