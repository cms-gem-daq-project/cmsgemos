#include "gem/hwMonitor/gemHwMonitorWeb.h"

XDAQ_INSTANTIATOR_IMPL(gem::hwMonitor::gemHwMonitorWeb)

gem::hwMonitor::gemHwMonitorWeb::gemHwMonitorWeb(xdaq::ApplicationStub * s)
    throw (xdaq::exception::Exception):
        xdaq::WebApplication(s)
{
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::Default, "Default");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::controlPanel, "Control Panel");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::setConfFile,"setConfFile");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::getCratesConfiguration,"getCratesConfiguration");
    gemHwMonitorBase_ = new gem::hwMonitor::gemHwMonitorBase();
    crateCfgAvailable_ = false;
}

void gem::hwMonitor::gemHwMonitorWeb::Default(xgi::Input * in, xgi::Output * out )
    throw (xgi::exception::Exception)
{
    this->controlPanel(in,out);
}
/* Generates the main page interface. Allows to choose the configuration file, then shows the availability of crates corresponding to this configuration.
 * Allows to launch the test utility to check the crates state.
 * */
void gem::hwMonitor::gemHwMonitorWeb::controlPanel(xgi::Input * in, xgi::Output * out )
    throw (xgi::exception::Exception)
{
    try {

         *out << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatmanager.css\"/>"          << std::endl
            << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatcontrolregisters.css\"/>" << std::endl
            << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatglobalsettings.css\"/>"   << std::endl
            << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatchannelregister.css\"/>"  << std::endl
            << "    <link rel=\"stylesheet\" href=\"/gemdaq/gemhardware/html/css/vfat/vfatcommands.css\"/>"         << std::endl;

        ///////////////////////////////////////////////
        //
        // GEM System Configuration
        //
        ///////////////////////////////////////////////

        *out << cgicc::h1("GEM System Configuration")<< std::endl;
        //
        *out << cgicc::span().set("style","color:blue");
        *out << cgicc::b(cgicc::i("Current configuration file: ")) ;
        *out << gemHwMonitorBase_->getXMLconfigFile() << cgicc::span() << std::endl ;
        *out << cgicc::br()<< std::endl;
        *out << cgicc::br()<< std::endl;

        std::string methodText = toolbox::toString("/%s/setConfFile",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","POST").set("action",methodText) << std::endl ;
        *out << cgicc::input().set("type","text").set("name","xmlFilename").set("size","80").set("ENCTYPE","multipart/form-data").set("value",gemHwMonitorBase_->getXMLconfigFile()) << std::endl;
        *out << cgicc::input().set("type","submit").set("value","Set configuration file") << std::endl ;
        *out << cgicc::form() << std::endl ;

        std::string methodUpload = toolbox::toString("/%s/uploadConfFile",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","POST").set("enctype","multipart/form-data").set("action",methodUpload) << std::endl ;
        *out << cgicc::input().set("type","file").set("name","xmlFilenameUpload").set("size","80") << std::endl;
        *out << cgicc::input().set("type","submit").set("value","Submit") << std::endl ;
        *out << cgicc::form() << std::endl ;

        *out << cgicc::hr()<< std::endl;
        *out << cgicc::br();

        *out << cgicc::h1("Crates configuration")<< std::endl;
	    this->showCratesAvailability(in,out);
        *out << cgicc::hr()<< std::endl;
        *out << cgicc::br();

	    this->showCrateUtilities(in,out);

        ///////////////////////////////////////////////
        //
        // Crate Utilities
        //
        ///////////////////////////////////////////////

        ///////////////////////////////////////////////
        //
        // Crate Configuration
        //
        ///////////////////////////////////////////////

        *out << cgicc::h1("Current Crate Configuration");

        *out << cgicc::h2("MCH Boards");
        /*if (MCHIds.size()) {
            for (unsigned int i = 0; i < MCHIds.size(); i++) {
                *out << "MCH Board: " << MCHIds[i] << cgicc::br();

                *out << cgicc::table().set("border","0");

                *out << cgicc::td();
                std::string f1 = toolbox::toString("/%s/MCHStatus",getApplicationDescriptor()->getURN().c_str());
                *out << cgicc::form().set("method","GET").set("action",f1) << std::endl ;
                *out << cgicc::input().set("type","submit").set("value","MCH Status") << std::endl ;
                *out << cgicc::form();
                *out << cgicc::td();

                *out << cgicc::td();
                std::string f2 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
                *out << cgicc::form().set("method","GET").set("action",f2) << std::endl ;
                *out << cgicc::input().set("type","submit").set("value","MCH Tests") << std::endl ;
                *out << cgicc::form();
                *out << cgicc::td();

                *out << cgicc::td();
                std::string f3 = toolbox::toString("/%s/MCHUtils",getApplicationDescriptor()->getURN().c_str());
                *out << cgicc::form().set("method","GET").set("action",f3) << std::endl ;
                *out << cgicc::input().set("type","submit").set("value","MCH Utils") << std::endl ;
                *out << cgicc::form();
                *out << cgicc::td();

                *out << cgicc::table();
            }
        }
        else */
	*out << "There are no MCH boards" << cgicc::br();
        *out << cgicc::br();

        *out << cgicc::h2("AMC13 Boards");
        /*if (AMCIds.size()) {
            for (unsigned int i = 0; i < AMCIds.size(); i++) {
                *out << "AMC13 Board: " << AMCIds[i] << cgicc::br();

                *out << cgicc::table().set("border","0");

                *out << cgicc::td();
                std::string f1 = toolbox::toString("/%s/AMCStatus",getApplicationDescriptor()->getURN().c_str());
                *out << cgicc::form().set("method","GET").set("action",f1) << std::endl ;
                *out << cgicc::input().set("type","submit").set("value","AMC13 Status") << std::endl ;
                *out << cgicc::form();
                *out << cgicc::td();

                *out << cgicc::td();
                std::string f2 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
                *out << cgicc::form().set("method","GET").set("action",f2) << std::endl ;
                *out << cgicc::input().set("type","submit").set("value","AMC13 Tests") << std::endl ;
                *out << cgicc::form();
                *out << cgicc::td();

                *out << cgicc::td();
                std::string f3 = toolbox::toString("/%s/AMCUtils",getApplicationDescriptor()->getURN().c_str());
                *out << cgicc::form().set("method","GET").set("action",f3) << std::endl ;
                *out << cgicc::input().set("type","submit").set("value","AMC13 Utils") << std::endl ;
                *out << cgicc::form();
                *out << cgicc::td();

                *out << cgicc::table();
            }
        }
        else*/ *out << "There are no AMC13 boards" << cgicc::br();
        *out << cgicc::br();

        *out << cgicc::h2("GLIB Boards");
        /*if (GLIBIds.size()) {
            for (unsigned int i = 0; i < GLIBIds.size(); i++) {
                *out << "GLIB Board: " << GLIBIds[i] << cgicc::br();

                *out << cgicc::table().set("border","0");

                *out << cgicc::td();
                std::string f1 = toolbox::toString("/%s/GLIBStatus",getApplicationDescriptor()->getURN().c_str());
                *out << cgicc::form().set("method","GET").set("action",f1) << std::endl ;
                *out << cgicc::input().set("type","submit").set("value","GLIB Status") << std::endl ;
                *out << cgicc::form();
                *out << cgicc::td();

                *out << cgicc::td();
                std::string f2 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
                *out << cgicc::form().set("method","GET").set("action",f2) << std::endl ;
                *out << cgicc::input().set("type","submit").set("value","GLIB Tests") << std::endl ;
                *out << cgicc::form();
                *out << cgicc::td();

                *out << cgicc::td();
                std::string f3 = toolbox::toString("/%s/GLIBUtils",getApplicationDescriptor()->getURN().c_str());
                *out << cgicc::form().set("method","GET").set("action",f3) << std::endl ;
                *out << cgicc::input().set("type","submit").set("value","GLIB Utils") << std::endl ;
                *out << cgicc::form();
                *out << cgicc::td();

                *out << cgicc::table();
            }
        }
        else */*out << "There are no GLIB boards" << cgicc::br();
        *out << cgicc::br();

        std::string method = toolbox::toString("/%s/controlVFAT2",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","POST")
            .set("action",method)
            << std::endl;
        *out << cgicc::script().set("type","text/javascript")
            .set("src","/gemdaq/gemhardware/html/scripts/toggleVFATCheckboxes.js")
            << cgicc::script() << cgicc::br()
            << std::endl;
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
void gem::hwMonitor::gemHwMonitorWeb::showCratesAvailability(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    std::string methodGetCratesCfg = toolbox::toString("/%s/getCratesConfiguration", getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","POST").set("action", methodGetCratesCfg) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Get crates configuration") << "    " << std::endl;
    *out << cgicc::form() << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Test all crates") << 
        cgicc::br() << std::endl;
    // If crates config is not available yet prompt to get it
    if (!crateCfgAvailable_) {
        *out << "Crate configuration isn't available. Please, get the crates configuration" << cgicc::br() <<std::endl;
    } else {
        *out << "Number of crates = " << nCrates_ << cgicc::br() << std::endl;
    }
/*    *out << "Dummy: will show crates from the CFG and their status" <<
        cgicc::br() << "The color of the button will show crate state (OK, ERROR, N/A)" << 
        cgicc::br() << "&nbsp" << 
        cgicc::br() << std::endl;
    *out << cgicc::input().set("type","submit").set("value","Crate #1") << "    " <<
        cgicc::input().set("type","submit").set("value","Crate #2") << "    " <<
        cgicc::input().set("type","submit").set("value","Crate #3") << "    " << std::endl;
    *out << cgicc::br() << std::endl;
    *out << cgicc::img().set("src", "/gemdaq/gemHwMonitor/html/images/green.jpg").set("width","88") << 
    "    " << cgicc::img().set("src", "/gemdaq/gemHwMonitor/html/images/yellow.jpg").set("width","88") << 
    "    " << cgicc::img().set("src", "/gemdaq/gemHwMonitor/html/images/red.jpg").set("width","88") << std::endl;
*/      
}

void gem::hwMonitor::gemHwMonitorWeb::showCrateUtilities(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    *out << cgicc::h1("Crate Utilities")<< std::endl;
    //
    *out << cgicc::span().set("style","color:blue");
    *out << cgicc::b(cgicc::i("Current crate : ")) ;
    *out << gemHwMonitorBase_->getCurrentCrate() << cgicc::span() << std::endl ;
    //
    *out << cgicc::br();
    
    // Begin select crate
    // Config listbox
    *out << cgicc::form().set("action",
            "/" + getApplicationDescriptor()->getURN() + "/CrateSelection") << std::endl;
    
    //int n_keys = crateIds.size();
    
    *out << "Choose crate: " << std::endl;
    *out << cgicc::select().set("name", "chosenCrateId") << std::endl;
    
    //int selected_index = currentCrate;
    std::string CrateName;
    /*for (int i = 0; i < n_keys; ++i) {
        CrateName = crateIds[i];
        if (i == selected_index) {
            *out << cgicc::option()
                .set("value", CrateName)
                .set("selected", "");
        } else {
            *out << cgicc::option()
                .set("value", CrateName);
        }
        *out << CrateName << cgicc::option() << std::endl;
    }*/
    
    *out << cgicc::select() << std::endl;
    
    *out << cgicc::input().set("type", "submit")
        .set("name", "command")
        .set("value", "Select crate") << std::endl;
    *out << cgicc::form() << std::endl;
    //End select crate
    
    *out << cgicc::br();
    *out << cgicc::table().set("border","0");
    
    *out << cgicc::td();
    std::string b22 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",b22) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Check availability of selected crate") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();
    
    *out << cgicc::td();
    std::string b23 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",b23) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Test selected crate") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();
    
    *out << cgicc::table();
    *out << cgicc::br();
    *out << cgicc::hr()<< std::endl;
    *out << cgicc::br();
}
void gem::hwMonitor::gemHwMonitorWeb::setConfFile(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    cgicc::Cgicc cgi(in);
    std::string newFile = cgi.getElement("xmlFilename")->getValue();
    struct stat buffer;
    if (stat(newFile.c_str(), &buffer) == 0) {
        gemHwMonitorBase_->setXMLconfigFile(newFile);
    }
    else {
        //filewarning = "selected file doesn't exist. please select another configuration file.";
        XCEPT_RAISE(xgi::exception::Exception, "File not found");
    }
    this->controlPanel(in,out);
}

void gem::hwMonitor::gemHwMonitorWeb::uploadConfFile(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    cgicc::Cgicc cgi(in);
    std::string newFile = cgi.getElement("xmlFilenameUpload")->getValue();
    struct stat buffer;
    if (stat(newFile.c_str(), &buffer) == 0) {
        gemHwMonitorBase_->setXMLconfigFile(newFile);
    }
    else {
        //filewarning = "selected file doesn't exist. please select another configuration file.";
        XCEPT_RAISE(xgi::exception::Exception, "File not found");
    }
    this->controlPanel(in,out);
}
void gem::hwMonitor::gemHwMonitorWeb::getCratesConfiguration(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    gemHwMonitorBase_->initParser();
    gemHwMonitorBase_->getSystemConfiguration();
    crateCfgAvailable_ = true;
    nCrates_ = gemHwMonitorBase_->getNumberOfCrates();
    this->controlPanel(in,out);
}
