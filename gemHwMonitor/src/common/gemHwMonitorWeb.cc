#include "gem/hwMonitor/gemHwMonitorWeb.h"

XDAQ_INSTANTIATOR_IMPL(gem::hwMonitor::gemHwMonitorWeb)

gem::hwMonitor::gemHwMonitorWeb::gemHwMonitorWeb(xdaq::ApplicationStub * s)
    throw (xdaq::exception::Exception):
        xdaq::WebApplication(s)
{
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::Default, "Default");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::controlPanel, "Control Panel");
    xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::setConfFile,"setConfFile");
    gemHwMonitorBase_ = new gem::hwMonitor::gemHwMonitorBase();
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

        std::string metho = toolbox::toString("/%s/setConfFile",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","POST").set("action",metho) << std::endl ;
        *out << cgicc::input().set("type","text").set("name","xmlFilename").set("size","80").set("ENCTYPE","multipart/form-data").set("value",gemHwMonitorBase_->getXMLconfigFile()) << std::endl;
        *out << cgicc::input().set("type","submit").set("value","Set configuration file") << std::endl ;
        *out << cgicc::form() << std::endl ;

        std::string methodUpload = toolbox::toString("/%s/UploadConfFile",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","POST").set("enctype","multipart/form-data").set("action",methodUpload) << std::endl ;
        *out << cgicc::input().set("type","file").set("name","xmlFilenameUpload").set("size","80") << std::endl;
        *out << cgicc::input().set("type","submit").set("value","Submit") << std::endl ;
        *out << cgicc::form() << std::endl ;

        *out << cgicc::hr()<< std::endl;
        *out << cgicc::br();

        *out << cgicc::h1("Connected crates")<< std::endl;
	    this->showCratesAvailability(in,out);
        *out << cgicc::hr()<< std::endl;
        *out << cgicc::br();

        ///////////////////////////////////////////////
        //
        // Crate Utilities
        //
        ///////////////////////////////////////////////

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
        std::string b21 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","GET").set("action",b21) << std::endl ;
        *out << cgicc::input().set("type","submit").set("value","Check availability of crates") << std::endl ;
        *out << cgicc::form();
        *out << cgicc::td();

        *out << cgicc::td();
        std::string b22 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","GET").set("action",b22) << std::endl ;
        *out << cgicc::input().set("type","submit").set("value","Check availability of selected crate") << std::endl ;
        *out << cgicc::form();
        *out << cgicc::td();

        *out << cgicc::td();
        std::string b23 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","GET").set("action",b23) << std::endl ;
        *out << cgicc::input().set("type","submit").set("value","Crate Tests") << std::endl ;
        *out << cgicc::form();
        *out << cgicc::td();

        *out << cgicc::table();
        *out << cgicc::br();
        *out << cgicc::hr()<< std::endl;
        *out << cgicc::br();

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
    *out << "Dummy: will show crates from the CFG and their status" <<
        cgicc::br() << "The color of the button will show crate state (OK, ERROR, N/A)" << 
        cgicc::br() << "For the moment image upload doesn't work" << 
        cgicc::br() << std::endl;
    *out << cgicc::input().set("type","submit").set("value","Crate #1") << "    " <<
        cgicc::input().set("type","submit").set("value","Crate #2").set("background-color", "red") << "    " <<
        cgicc::input().set("type","submit").set("value","Crate #3") << std::endl ;
    *out << cgicc::br() << std::endl;
    *out << cgicc::img().set("src", "/gemdaq/gemHwMonitor/html/images/green.jpg") << "    " << cgicc::img().set("src", "./images/yellow.jpg") << "    " << cgicc::img().set("src", "./images/red.jpg") << std::endl;
        
}

gem::hwMonitor::gemHwMonitorWeb::setConfFile(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    cgicc::Cgicc cgi(in);
    std::string oldFile = settingsFile_;
    std::string newFile = cgi.getElement("xmlFilename")->getValue();
    struct stat buffer;
    if (stat(newFile.c_str(), &buffer) == 0) {
        settingsFile_ = newFile;
        parseXMLFile();
        if (fileError.size()) fileWarning = fileError;
        else {
            if (crateIds.size() == 0) {
                fileWarning = "Selected file doesn't have any crate information. Please select another configuration file.";
                settingsFile_ = oldFile;
            }
            else {
                fileWarning = "";
                settingsFile_ = newFile;
            }
        }
    }
    else {
        filewarning = "selected file doesn't exist. please select another configuration file.";
    }
    this->ControlPanel(in,out);
}
