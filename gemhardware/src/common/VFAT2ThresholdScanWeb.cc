#include "gem/hw/vfat/VFAT2ThresholdScan.h"

void gem::hw::vfat::VFAT2ThresholdScan::VFAT2ThresholdScanWeb::createHeader(xgi::Output *out ) {

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

//building the control view
void gem::hw::vfat::VFAT2ThresholdScan::VFAT2ThresholdScanWeb::createVFATInfoLayout(xgi::Output *out, const gem::hw::vfat::VFAT2ControlParams& params) {
  //Report the GEB information (slot, fibre, etc.,)
  *out << cgicc::label("ChipID").set("for","ChipID") << std::endl
       << cgicc::input().set("type","text").set("id","ChipID")
                        .set("name","ChipID").set("readonly")
                        .set("value",boost::str(boost::format("0x%04x")%(params.chipID)))
       << std::endl
       << "          <fieldset class=\"vfatPosition\">" << std::endl
       << cgicc::legend("VFAT2 Position") << std::endl
       << "            <div>" << std::endl
       << cgicc::label("GLIB").set("for","GLIB") << std::endl
       << cgicc::input().set("id","GLIB").set("type","text")
                        .set("name","GLIB").set("readonly")
       << std::endl
       << cgicc::label("Fibre").set("for","GLIBFibre") << std::endl
       << cgicc::input().set("id","GLIBFibre").set("type","text")
                        .set("name","GLIBFibre").set("readonly")
       << std::endl
       << "            </div>" << std::endl
       << cgicc::comment() << "ending the VFAT2 Position fieldset" << cgicc::comment() << std::endl
       << "          </fieldset>" << std::endl;
}

//building the counter view
void gem::hw::vfat::VFAT2ThresholdScan::VFAT2ThresholdScanWeb::createCounterLayout(xgi::Output *out, const gem::hw::vfat::VFAT2ThresholdScanParams& params) {
  *out << std::endl << cgicc::br() << std::endl << std::endl << cgicc::br() << std::endl << std::endl << cgicc::br() << std::endl
       << std::endl << cgicc::br() << std::endl << std::endl << cgicc::br() << std::endl << std::endl << cgicc::br() << std::endl
       << std::endl
       << "<fieldset class=\"vfatCounters\">" << std::endl
       << cgicc::legend("Threshold Scan") << std::endl
       << std::endl
       << cgicc::label("Latency Value").set("for","ThresholdLatency") << std::endl
       << cgicc::input().set("class","vfatBiasInput")
                        .set("type","number").set("min","0").set("max","255")
                        .set("id","ThresholdLatency").set("name","ThresholdLatency")
                        .set("value",boost::str(boost::format("%d")%((unsigned)params.thresholdLatency)))
       << std::endl << cgicc::br() << std::endl
       << cgicc::label("Initial Threshold").set("for","InitialThreshold") << std::endl
       << cgicc::input().set("class","vfatBiasInput")
                        .set("type","number").set("min","-255").set("max","255")
                        .set("id","InitialThreshold").set("name","InitialThreshold")
                        .set("value",boost::str(boost::format("%d")%(params.thresholdMin)))
       << std::endl << cgicc::br() << std::endl
       << std::endl
       << cgicc::label("Final Threshold").set("for","FinalThreshold") << std::endl
       << cgicc::input().set("class","vfatBiasInput")
                        .set("type","number").set("min","-255").set("max","255")
                        .set("id","FinalThreshold").set("name","FinalThreshold")
                        .set("value",boost::str(boost::format("%d")%(params.thresholdMax)))
       << std::endl<< cgicc::br() << std::endl 
       << std::endl
       << cgicc::label("Threshold Step").set("for","ThresholdStep") << std::endl
       << cgicc::input().set("class","vfatBiasInput")
                        .set("type","number").set("min","-255").set("max","255")
                        .set("id","ThresholdStep").set("name","ThresholdStep")
                        .set("value",boost::str(boost::format("%d")%(params.thresholdStep)))
       << std::endl<< cgicc::br() << std::endl
       << std::endl
       << cgicc::label("VTH1").set("for","ThresholdVTH1") << std::endl
       << cgicc::input().set("class","vfatBiasInput")
                        .set("type","number").set("value","0").set("readonly")
                        .set("id","ThresholdVTH1").set("name","ThresholdVTH1")
                        .set("value",boost::str(boost::format("%d")%((unsigned)params.thresholdVTH1)))
       << std::endl
       << cgicc::label("VTH2").set("for","ThresholdVTH2") << std::endl
       << cgicc::input().set("class","vfatBiasInput")
                        .set("type","number").set("value","0").set("readonly")
                        .set("id","ThresholdVTH2").set("name","ThresholdVTH2")
                        .set("value",boost::str(boost::format("%d")%((unsigned)params.thresholdVTH2)))
       << std::endl<< cgicc::br() << std::endl
       << std::endl
       << cgicc::label("Minimum Number of Triggers per Threshold").set("for","MinTriggersThreshold") << std::endl
       << cgicc::input().set("class","vfatBiasInput")
                        .set("type","number").set("min","0")
                        .set("id","MinTriggersThreshold").set("name","MinTriggersThreshold")
                        .set("value",boost::str(boost::format("%d")%(params.thresholdNTriggers)))
       << std::endl<< cgicc::br() << std::endl
       << std::endl
       << cgicc::label("Number of Triggers Collected per Threshold").set("for","TriggersCollectedThreshold") << std::endl
       << cgicc::input().set("class","vfatBiasInput")
                        .set("type","number").set("readonly")
                        .set("id","TriggersCollectedThreshold").set("name","TriggersCollectedThreshold")
                        .set("value","0")
       << std::endl
       << cgicc::comment() << "ending the Counters fieldset" <<cgicc::comment() << std::endl
       << "</fieldset>" << std::endl;  
}

//building the command view
void gem::hw::vfat::VFAT2ThresholdScan::VFAT2ThresholdScanWeb::createCommandLayout(xgi::Output *out, const gem::hw::vfat::VFAT2ControlParams& params) {

  *out << cgicc::section().set("class","vfatCommands") << std::endl
       << cgicc::h3() << "Commands" << cgicc::h3()
       << std::endl << std::endl
       << cgicc::input().set("class","vfatButtonInput")//.set("style","width:auto")
                        .set("type","submit").set("value","Scan for connected VFATs")
                        .set("name","ScanForVFATS")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::input().set("class","vfatButtonInput")//.set("style","width:auto")
                        .set("type","submit").set("value","Initialize VFAT")
                        .set("name","InitializeVFAT")
       << std::endl
       << cgicc::br() << std::endl;

  *out << cgicc::fieldset().set("class","selectedChipControl") << std::endl
       << cgicc::legend("Selected Chip")         << std::endl
       << cgicc::input().set("class","vfatButtonInput")//.set("style","width:auto")
                        .set("type","submit").set("value","Read VFAT").set("name","VFAT2ControlOption")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::input().set("class","vfatButtonInput")//.set("style","width:auto")
                        .set("type","submit").set("value","Write VFAT").set("name","VFAT2ControlOption")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::label("Compare written values").set("for","Compare") << std::endl
       << cgicc::input().set("type","checkbox").set("id","Compare").set("value","Compare")
                       .set("name","Compare")
       << std::endl
       << cgicc::br() << std::endl
       << cgicc::br() << std::endl
       //changed from type submit to type button
       << cgicc::input().set("class","vfatButtonInput")////.set("style","width:auto")
                        .set("type","button").set("value","Select All Registers")
			.set("name","SelectAll")
       << std::endl
       << cgicc::br() << std::endl
       //changed from type submit to type button
       << cgicc::input().set("class","vfatButtonInput")////.set("style","width:auto")
			.set("type","button").set("value","Select None")
                        .set("name","SelectNone")
       << std::endl
       << cgicc::comment() << "ending the Selected chip fieldset" << cgicc::comment()
       << std::endl
       << cgicc::fieldset() << std::endl
       << cgicc::br() << std::endl
       << std::endl;

}
