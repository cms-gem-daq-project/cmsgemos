#include "gem/hwMon/gemHWmonitorWeb.h"

XDAQ_INSTANTIATOR_IMPL(gem::hwMon::gemHWmonitorWeb)

gem::hwMon::gemHWmonitorWeb::gemHWmonitorWeb(xdaq::ApplicationStub * s)
    throw (xdaq::exception::Exception):
        xdaq::WebApplication(s)
{
    xgi::framework::deferredbind(this, this, &gemHWmonitorWeb::createHeader, "Header");
    xgi::framework::deferredbind(this, this, &gemHWmonitorWeb::controlPanel, "ControlPanel");

    //this->controlPanel(in, out);

    /*
    xgi::framework::deferredbind(this, this, &GEMController::controlVFAT2,  "controlVFAT2");
    xgi::framework::deferredbind(this, this, &GEMController::CrateSelection,"CrateSelection"); 
    xgi::framework::deferredbind(this, this, &GEMController::setConfFile,"setConfFile"); 
    xgi::framework::deferredbind(this, this, &GEMController::MCHStatus,"MCHStatus"); 
    xgi::framework::deferredbind(this, this, &GEMController::MCHUtils,"MCHUtils"); 
    xgi::framework::deferredbind(this, this, &GEMController::AMCStatus,"AMCStatus"); 
    xgi::framework::deferredbind(this, this, &GEMController::AMCUtils,"AMCUtils"); 
    xgi::framework::deferredbind(this, this, &GEMController::GLIBStatus,"GLIBStatus"); 
    xgi::framework::deferredbind(this, this, &GEMController::GLIBUtils,"GLIBUtils"); 
    xgi::framework::deferredbind(this, this, &GEMController::OHStatus,"OHStatus"); 
    xgi::framework::deferredbind(this, this, &GEMController::OHUtils,"OHUtils"); 
    xgi::framework::deferredbind(this, this, &GEMController::VFAT2Manager,"VFAT2Manager"); 
    */

    /* No need to check if the settings has been performed
    *  There should be connecter to the device
    */
    //device_ = "CMS_hybrid_J8";
    //settingsFile_ = "";

    //// Detect when the setting of default parameters has been performed
    //this->getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");

    //getApplicationInfoSpace()->fireItemAvailable("device", &device_);
    //getApplicationInfoSpace()->fireItemAvailable("settingsFile", &settingsFile_);

    //getApplicationInfoSpace()->fireItemValueRetrieve("device", &device_);
    //getApplicationInfoSpace()->fireItemValueRetrieve("settingsFile", &settingsFile_);

}

void gem::hwMon::gemHWmonitorWeb::createHeader(xgi::Output *out ) {

    *out << "<!doctype html>" << std::endl
        << "<html xmlns=\"http://www.w3.org/1999/xhtml\">" << std::endl
        << "<head>" << std::endl
        << "    <meta charset=\"UTF-8\" />" << std::endl
        << "    <meta name=\"viewport\" content=\"width = device-width\">" << std::endl
        << "    <title>XDAQ HyperDAQ</title>" << std::endl
        << "    <link rel=\"shortcut icon\" href=\"/xdaq/images/xdaq.ico\">" << std::endl
        << "    <link rel=\"apple-touch-icon\" href=\"/xdaq/images/xdaq.ico\"/>" << std::endl
        << "    <link href=\"/hyperdaq/html/css/xdaq-normalize.css\" rel=\"stylesheet\" />" << std::endl
        << "    <link href=\"/hyperdaq/html/css/xdaq-buttons.css\" rel=\"stylesheet\" />" << std::endl
        << "    <link href=\"/hyperdaq/html/css/xdaq-generics.css\" rel=\"stylesheet\" />" << std::endl
        << "    <link href=\"/hyperdaq/html/css/xdaq-layout.css\" rel=\"stylesheet\" />" << std::endl
        << "    <link href=\"/hyperdaq/html/css/xdaq-fonts.css\" rel=\"stylesheet\" />" << std::endl
        << "    <link href=\"/hyperdaq/html/css/xdaq-forms.css\" rel=\"stylesheet\" />" << std::endl
        << "    <link href=\"/hyperdaq/html/css/xdaq-tables.css\" rel=\"stylesheet\" />" << std::endl
        << "    <link href=\"/hyperdaq/html/css/xdaq-trees.css\" rel=\"stylesheet\" />" << std::endl
        << "    <link href=\"/hyperdaq/html/css/xdaq-tooltips.css\" rel=\"stylesheet\" />" << std::endl
        << "    <link href=\"/hyperdaq/html/css/xdaq-tabs.css\" rel=\"stylesheet\" />" << std::endl
        << "    <link href=\"/hyperdaq/html/css/xdaq-console.css\" rel=\"stylesheet\" />" << std::endl
        << "    <link href=\"/hyperdaq/html/css/xdaq-hyperdaq-app-widgets.css\" rel=\"stylesheet\" />" << std::endl
        << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/jquery-2.1.0.js\"></script>" << std::endl
        //<< "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/jquery-2.1.0.min.js\"></script>" << std::endl
        << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/xdaq-utils.js\"></script>" << std::endl
        << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/xdaq-main-layout.js\"></script>" << std::endl
        << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/xdaq-tabs.js\"></script>" << std::endl
        << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/xdaq-treetable.js\"></script>" << std::endl
        << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/xdaq-tree-utils.js\"></script>" << std::endl
        << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/xdaq-console.js\"></script>" << std::endl
        << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/xdaq-sort.js\"></script>" << std::endl
        << "    <script type=\"text/javascript\" src=\"/hyperdaq/html/js/xdaq-window-load.js\"></script>" << std::endl

        << "<link rel=\"stylesheet\" type=\"text/css\" href=\"gemdaq/gemhardware/html/css/vfat//vfatchannelregister.css\"/>" << std::endl
        << "<link rel=\"stylesheet\" type=\"text/css\" href=\"gemdaq/gemhardware/html/css/vfat//vfatcommands.css\"/>"        << std::endl
        << "<link rel=\"stylesheet\" type=\"text/css\" href=\"gemdaq/gemhardware/html/css/vfat//vfatcontrolregisters.css\"/>"      << std::endl
        << "<link rel=\"stylesheet\" type=\"text/css\" href=\"gemdaq/gemhardware/html/css/vfat//vfatglobalsettings.css\"/>"      << std::endl
        << "<link rel=\"stylesheet\" type=\"text/css\" href=\"gemdaq/gemhardware/html/css/vfat//vfatmanager.css\"/>"<< std::endl;

    *out << "<script>" << std::endl;
    *out << "window.onload = function()" << std::endl
        << "    alert( \"welcome\" );"  << std::endl
        << "}"                          << std::endl
        << "</script>"     << std::endl << std::endl;

    *out << "<script>" << std::endl;
    *out << "$( document ).ready(function() {"           << std::endl
        << "    $( \"a\" ).click(function( event ) {"   << std::endl
        << "        alert( \"Thanks for visiting!\" );" << std::endl
        << "    });"                                    << std::endl
        << "});"                                        << std::endl;
    *out << "</script>"  << std::endl << std::endl;
    *out << "</head>" << std::endl;
}
void gem::hwMon::gemHWmonitorWeb::controlPanel(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    try {

        //need to grab the page header and modify it here somehow...
        //gem::hw::vfat::GEMController::GEMControllerPanelWeb::createHeader(out);
        //*out << "  <body>" << std::endl;
        //cgicc::HTTPResponseHeader& head = out->getHTTPResponseHeader();
        //head->addHeader();

        //head = out["head"]
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
        *out << settingsFile_.toString() << cgicc::span() << std::endl ;
        if (fileWarning.size()) {
            *out << cgicc::br()<< std::endl;
            *out << cgicc::span().set("style","color:red");
            *out << cgicc::b(cgicc::i(fileWarning)) << cgicc::span() << std::endl ;
        }
        *out << cgicc::br()<< std::endl;

        std::string metho = toolbox::toString("/%s/setConfFile",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","POST").set("action",metho) << std::endl ;
        *out << cgicc::input().set("type","text").set("name","xmlFilename").set("size","80").set("ENCTYPE","multipart/form-data").set("value",settingsFile_.toString()) << std::endl;
        *out << cgicc::input().set("type","submit").set("value","Set configuration file") << std::endl ;
        *out << cgicc::form() << std::endl ;

        std::string methodUpload = toolbox::toString("/%s/UploadConfFile",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","POST").set("enctype","multipart/form-data").set("action",methodUpload) << std::endl ;
        *out << cgicc::input().set("type","file").set("name","xmlFilenameUpload").set("size","80") << std::endl;
        *out << cgicc::input().set("type","submit").set("value","Submit") << std::endl ;
        *out << cgicc::form() << std::endl ;

        *out << cgicc::hr()<< std::endl;
        *out << cgicc::br();

        ///////////////////////////////////////////////
        //
        // System Utilities
        //
        ///////////////////////////////////////////////

        *out << cgicc::h1("System Utilities")<< std::endl;

        *out << cgicc::table().set("border","0");

        *out << cgicc::td();
        std::string b11 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","GET").set("action",b11) << std::endl ;
        *out << cgicc::input().set("type","submit").set("value","Crates Power-up Init") << std::endl ;
        *out << cgicc::form();
        *out << cgicc::td();

        *out << cgicc::td();
        std::string b12 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","GET").set("action",b12) << std::endl ;
        *out << cgicc::input().set("type","submit").set("value","Configuration Validation") << std::endl ;
        *out << cgicc::form();
        *out << cgicc::td();

        *out << cgicc::td();
        std::string b13 = toolbox::toString("/%s/Dummy",getApplicationDescriptor()->getURN().c_str());
        *out << cgicc::form().set("method","GET").set("action",b13) << std::endl ;
        *out << cgicc::input().set("type","submit").set("value","Firmware Manager") << std::endl ;
        *out << cgicc::form();
        *out << cgicc::td();

        *out << cgicc::table();
        *out << cgicc::br();
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
        *out << currentCrateId << cgicc::span() << std::endl ;
        //
        *out << cgicc::br();

        // Begin select crate
        // Config listbox
        *out << cgicc::form().set("action",
                "/" + getApplicationDescriptor()->getURN() + "/CrateSelection") << std::endl;

        int n_keys = crateIds.size();

        *out << "Choose crate: " << std::endl;
        *out << cgicc::select().set("name", "chosenCrateId") << std::endl;

        int selected_index = currentCrate;
        std::string CrateName;
        for (int i = 0; i < n_keys; ++i) {
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
        }

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
        if (MCHIds.size()) {
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
        else *out << "There are no MCH boards" << cgicc::br();
        *out << cgicc::br();

        *out << cgicc::h2("AMC13 Boards");
        if (AMCIds.size()) {
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
        else *out << "There are no AMC13 boards" << cgicc::br();
        *out << cgicc::br();

        *out << cgicc::h2("GLIB Boards");
        if (GLIBIds.size()) {
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
        else *out << "There are no GLIB boards" << cgicc::br();
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
