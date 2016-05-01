#include "gem/hwMonitor/gemHwMonitorWeb.h"

#include <fstream>

#include "boost/algorithm/string.hpp"

XDAQ_INSTANTIATOR_IMPL(gem::hwMonitor::gemHwMonitorWeb);

gem::hwMonitor::gemHwMonitorWeb::gemHwMonitorWeb(xdaq::ApplicationStub* s)
  throw (xdaq::exception::Exception):
  xdaq::WebApplication(s)
{
  xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::Default,                "Default"               );
  xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::Dummy,                  "Dummy"                 );
  xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::controlPanel,           "Control Panel"         );
  xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::setConfFile,            "setConfFile"           );
  xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::uploadConfFile,         "uploadConfFile"        );
  xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::displayConfFile,        "displayConfFile"       );
  xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::getCratesConfiguration, "getCratesConfiguration");
  xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::pingCrate,              "pingCrate"             );
  xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::expandCrate,            "expandCrate"           );
  xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::cratePanel,             "cratePanel"            );
  xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::expandGLIB,             "expandGLIB"            );
  xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::glibPanel,              "glibPanel"             );
  xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::expandOH,               "expandOH"              );
  xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::ohPanel,                "ohPanel"               );
  xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::expandVFAT,             "expandVFAT"            );
  xgi::framework::deferredbind(this, this, &gemHwMonitorWeb::vfatPanel,              "vfatPanel"             );
  m_gemHwMonitorSystem = new gemHwMonitorSystem();
  // m_gemHwMonitorCrate = new gemHwMonitorCrate();
  // m_gemHwMonitorGLIB = new gemHwMonitorGLIB();
  // m_gemHwMonitorOH = new gemHwMonitorOH();
  // m_gemHwMonitorVFAT = new gemHwMonitorVFAT();
  p_gemSystemHelper = new gemHwMonitorHelper(m_gemHwMonitorSystem);
  m_crateCfgAvailable = false;
}

gem::hwMonitor::gemHwMonitorWeb::~gemHwMonitorWeb()
{
  delete m_gemHwMonitorSystem;
  // delete m_gemHwMonitorCrate;
  for_each(m_gemHwMonitorCrate.begin(), m_gemHwMonitorCrate.end(), free);
  for_each(m_gemHwMonitorGLIB.begin(),  m_gemHwMonitorGLIB.end(),  free);
  for_each(m_gemHwMonitorOH.begin(),    m_gemHwMonitorOH.end(),    free);
  for_each(m_gemHwMonitorVFAT.begin(),  m_gemHwMonitorVFAT.end(),  free);
  // delete m_gemHwMonitorGLIB;
  // delete m_gemHwMonitorOH;
  // delete m_gemHwMonitorVFAT;
  delete p_gemSystemHelper;
}

void gem::hwMonitor::gemHwMonitorWeb::Default(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  this->controlPanel(in, out);
}

void gem::hwMonitor::gemHwMonitorWeb::pingCrate(xgi::Input* in, xgi::Output* out )

  throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);
  for (unsigned i = 0; i != m_gemHwMonitorSystem->getDevice()->getSubDevicesRefs().size(); ++i) {
    if (cgi.queryCheckbox(m_gemHwMonitorSystem->getDevice()->getSubDevicesRefs().at(i)->getDeviceId())) {
      //            // gem::hw::GEMHwDevice* crateDevice_ = new gem::hw::GEMHwDevice();
      // this really needs to go
      std::stringstream tmpURI;
      tmpURI << "chtcp-2.0://localhost:10203?target=" << "192.168.0.170" << ":50001";
      vfat_shared_ptr crateDevice_(new gem::hw::vfat::HwVFAT2("VFAT0", tmpURI.str(),
                                                              "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));
      if (crateDevice_->isHwConnected()) {
        m_gemHwMonitorSystem->setSubDeviceStatus(0, i);
      } else {
        m_gemHwMonitorSystem->setSubDeviceStatus(1, i);
      }
      // delete crateDevice_;
    }
  }
  this->controlPanel(in, out);
}

void gem::hwMonitor::gemHwMonitorWeb::Dummy(xgi::Input* in, xgi::Output* out )

  throw (xgi::exception::Exception)
{
  *out << "Dummy" <<std::endl;
}

/* Generates the main page interface. Allows to choose the configuration file, then
* shows the availability of crates corresponding to this configuration.
* Allows to launch the test utility to check the crates state.
* */
void gem::hwMonitor::gemHwMonitorWeb::controlPanel(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  try {
    *out << "<link rel=\"stylesheet\" type=\"text/css\" "
         << "href=\"/gemdaq/gemHwMonitor/html/css/bootstrap.css\">"
         << std::endl
         << "<link rel=\"stylesheet\" type=\"text/css\" "
         << "href=\"/gemdaq/gemHwMonitor/html/css/bootstrap-theme.css\">"
         << std::endl;
    /*out << "<link rel=\"stylesheet\" type=\"text/css\" "
      << "href=\"/gemdaq/gemHwMonitor/html/css/bootstrap.min.css\">" << std::endl
      << "<link rel=\"stylesheet\" type=\"text/css\" "
      << "href=\"/gemdaq/gemHwMonitor/html/css/bootstrap-theme.min.css\">" << std::endl;
    */
    ///////////////////////////////////////////////
    //
    // GEM System Configuration
    //
    ///////////////////////////////////////////////


    *out << "<div class=\"panel panel-primary\">" << std::endl;
    *out << "<div class=\"panel-heading\">" << std::endl;
    *out << "<h1><div align=\"center\">GEM System Configuration</div></h1>" << std::endl;
    *out << "</div>" << std::endl;
    *out << "<div class=\"panel-body\">" << std::endl;

    *out << "<h3><div class=\"alert alert-info\" role=\"alert\" align=\"center\">"
         << "Current configuration file : "<< p_gemSystemHelper->getXMLconfigFile()
         <<  "</div></h3>" << std::endl;
    *out << cgicc::br()<< std::endl;

    std::string methodText = toolbox::toString("/%s/setConfFile",
                                               getApplicationDescriptor()->getURN().c_str());



    // *out << cgicc::form().set("method", "POST").set("action", methodText) << std::endl ;
    // *out << cgicc::input().set("type", "text").set("name", "xmlFilename")
    //   .set("size", "120").set("ENCTYPE", "multipart/form-data")
    //   .set("value",p_gemSystemHelper->getXMLconfigFile())
    //      << std::endl;
    // *out << "<button type=\"submit\" class=\"btn btn-primary\">Set configuration file</button>" << std::endl;
    // *out << cgicc::form() << std::endl ;

    // std::string methodUpload = toolbox::toString("/%s/uploadConfFile",
    //                                              getApplicationDescriptor()->getURN().c_str());
    // *out << cgicc::form().set("method", "POST").set("enctype", "multipart/form-data").set("action", methodUpload)
    //      << std::endl ;
    // // *out << cgicc::input().set("type", "file").set("name", "xmlFilenameUpload").set("size", "80") << std::endl;
    // *out << "<span class=\"btn btn-primary btn-file\">Browse <input type=\"file\" "
    //      << "name=\"xmlFilenameUpload\"></span>" << std::endl;
    // *out << "<button type=\"submit\" class=\"btn btn-primary\">Submit - BROKEN</button>" << std::endl;
    // *out << cgicc::form() << std::endl ;

    // *out << cgicc::br()<< std::endl;




    *out << "<h5><div class=\"alert alert-warning\" align=\"center\" role=\"alert\">Note: Configuration file must be located in ";
    *out << "BUILD_HOME/gemdaq-testing/gembase/xml/ </div></h5>" << std::endl;


    *out << cgicc::form().set("method", "POST").set("action", methodText) << std::endl ;
    *out << "<input id=\"Filename\" name=\"xmlFile\" type=\"file\" style=\"display:none\"> " << std::endl;
    *out << "<div class=\"input-append\">" << std::endl;
    *out << "<input id=\"xmlFilename\" name=\"xmlFileUpload\" class=\"input-large\" type=\"text\" size=\"120\" " << std::endl;
    *out << "value=\"" << p_gemSystemHelper->getXMLconfigFile() << "\" >" << std::endl;
    *out << "<a class=\"btn\" onclick=\"$('input[id=Filename]').click();\">Choose XML</a>" << std::endl;
    *out << "</div>" << std::endl;
    *out << "<button type=\"submit\" class=\"btn btn-primary\" >Set XML file</button>" << std::endl;
    *out << cgicc::form() << std::endl ;

    *out << "<script type=\"text/javascript\">" << std::endl;
    *out << "$('input[id=Filename]').change(function() {" << std::endl;
    *out << "$('#xmlFilename').val($(this).val());" << std::endl;
    *out << "});" << std::endl;
    *out << "</script>" << std::endl;


    *out << cgicc::br()<< std::endl;

    std::string methodDisplayXML = toolbox::toString("/%s/displayConfFile",
                                                     getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method", "POST").set("enctype", "multipart/form-data")
      .set("action", methodDisplayXML) << std::endl ;
    *out << "<button type=\"submit\" class=\"btn btn-primary\">View XML</button>" << std::endl;
    *out << cgicc::form() << std::endl ;

    *out << cgicc::hr()<< std::endl;

    *out << "<h2><div align=\"center\">Connected Crates</div></h2>" << std::endl;
    this->showCratesAvailability(in, out);
    *out << cgicc::hr()<< std::endl;
    *out << cgicc::br();
  } catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(), "Something went wrong displaying ControlPanel xgi: "
                   << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  } catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(), "Something went wrong displaying the ControlPanel: "
                   << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}
void gem::hwMonitor::gemHwMonitorWeb::showCratesAvailability(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  // If crates config is not available yet prompt to get it
  if (!m_crateCfgAvailable) {
    *out << "<h3><div class=\"alert alert-warning\" role=\"alert\" align=\"center\">"
         << "Crates configuration isn't available. Please, get the crates configuration</div></h3>"
         << std::endl;
    std::string methodGetCratesCfg = toolbox::toString("/%s/getCratesConfiguration",
                                                       getApplicationDescriptor()->getURN().c_str());
    *out << "<div align=\"center\">" << std::endl;
    *out << cgicc::form().set("method", "POST").set("action", methodGetCratesCfg) << std::endl ;
    *out << "<button type=\"submit\" class=\"btn btn-primary\">Get crates configuration</button>" << std::endl;
    *out << cgicc::form() << std::endl ;
    *out << "</div>" << std::endl;
  } else {
    // *out << "Crates configuration is taken from XML. In order to check their availability please select needed crates and press 'Check selected crates availability' button. " <<
    // "To have more information about their state press 'Test selected crates' button" <<
    //    cgicc::br() << std::endl;
    std::string methodExpandCrate = toolbox::toString("/%s/expandCrate",
                                                      getApplicationDescriptor()->getURN().c_str());
    std::string methodSelectCrate = toolbox::toString("/%s/selectCrate",
                                                      getApplicationDescriptor()->getURN().c_str());
    std::string methodPingCrate = toolbox::toString("/%s/pingCrate",
                                                    getApplicationDescriptor()->getURN().c_str());
    *out << "<div align=\"center\">" << std::endl;
    *out << cgicc::table().set("class", "table-condensed");
    *out << cgicc::tr();
    for (int i = 0; i < m_nCrates; ++i) {
      std::string currentCrateID;
      currentCrateID += m_gemHwMonitorSystem->getCurrentSubDeviceId(i);
      *out << cgicc::td();
      *out << cgicc::form().set("method", "POST").set("action", methodExpandCrate) << std::endl ;
      if (m_gemHwMonitorSystem->getSubDeviceStatus(i) == 0) {
        *out << "<button type=\"submit\" class=\"btn btn-success\" name=\"crateButton\" value=\""
             << currentCrateID << "\">" << currentCrateID<< "</button>" << std::endl;
      } else if (m_gemHwMonitorSystem->getSubDeviceStatus(i) == 1) {
        *out << "<button type=\"submit\" class=\"btn btn-warning\" name=\"crateButton\" value=\""
             << currentCrateID << "\">" << currentCrateID<< "</button>" << std::endl;
      } else if (m_gemHwMonitorSystem->getSubDeviceStatus(i) == 2) {
        *out << "<button type=\"submit\" class=\"btn btn-disabled\" name=\"crateButton\" value=\""
             << currentCrateID << "\" disabled>" << currentCrateID<< "</button>" << std::endl;
      }
      *out << cgicc::form() << std::endl ;
      *out << cgicc::td();
    }
    *out << cgicc::tr();
    *out << cgicc::tr();
    *out << cgicc::form().set("method", "GET").set("action", methodPingCrate) << std::endl ;
    for (int i = 0; i < m_nCrates; ++i) {
      std::string currentCrateID;
      currentCrateID += m_gemHwMonitorSystem->getCurrentSubDeviceId(i);
      *out << cgicc::td();
      *out << "<div align=\"center\">" << cgicc::input().set("type", "checkbox").set("name", currentCrateID)
           << "</div>" << std::endl;
      *out << cgicc::td();
    }
    *out << cgicc::tr();
    *out << cgicc::table();
    *out << "<button type=\"submit\" class=\"btn btn-primary\">"
         << "Check availability of selected crates</button>" << std::endl;
    *out << cgicc::form();
    *out << "</div>" << std::endl;
    *out << cgicc::br();
  }
}

void gem::hwMonitor::gemHwMonitorWeb::setConfFile(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);
  std::string rawFile = cgi("xmlFileUpload");
  std::string newFile = p_gemSystemHelper->fixXMLconfigFile(rawFile.c_str());
  // std::cout<<"newFile: "<<newFile<<std::endl;

  // conditional statement would not recognize good xml file
  struct stat buffer;
  if (stat(newFile.c_str(), &buffer) == 0) {
    p_gemSystemHelper->setXMLconfigFile(newFile.c_str());
    m_crateCfgAvailable = false;
  } else {
    XCEPT_RAISE(xgi::exception::Exception, "File not found");
  }
  this->controlPanel(in, out);
}

void gem::hwMonitor::gemHwMonitorWeb::uploadConfFile(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);
  std::string newFile = cgi.getElement("xmlFilenameUpload")->getValue();
  struct stat buffer;
  if (stat(newFile.c_str(), &buffer) == 0) {
    p_gemSystemHelper->setXMLconfigFile(newFile.c_str());
    m_crateCfgAvailable = false;
  } else {
    std::cout << "newFile: " << newFile << std::endl;

    XCEPT_RAISE(xgi::exception::Exception, "File not found");
  }
  this->controlPanel(in, out);
}

void gem::hwMonitor::gemHwMonitorWeb::displayConfFile(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);
  std::ifstream infile(p_gemSystemHelper->getXMLconfigFile());
  std::string line;
  while (std::getline(infile, line)) {
    std::replace(line.begin(), line.end(), '<', '[');
    std::replace(line.begin(), line.end(), '>', ']');
    std::replace(line.begin(), line.end(), '"', '^');
    *out << "<pre>" << line << "</pre>" << std::endl;
  }
}

void gem::hwMonitor::gemHwMonitorWeb::getCratesConfiguration(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  p_gemSystemHelper->configure();
  std::cout << "Configured." << std::endl;
  m_crateCfgAvailable = true;
  m_nCrates = m_gemHwMonitorSystem->getNumberOfSubDevices();
  // continuously redefining the variable 'i' is bad form, though maybe this is the point of the compiler comment below
  for (int i = 0; i < m_nCrates; ++i) {
    m_gemHwMonitorSystem->addSubDeviceStatus(2);
    for (unsigned i = 0; i != m_gemHwMonitorSystem->getDevice()->getSubDevicesRefs().size(); ++i) {
      m_gemHwMonitorCrate.push_back(new  gemHwMonitorCrate());
      m_gemHwMonitorCrate.back()->setDeviceConfiguration(*m_gemHwMonitorSystem->getDevice()->getSubDevicesRefs().at(i));
      for (unsigned i = 0; i != m_gemHwMonitorCrate.back()->getDevice()->getSubDevicesRefs().size(); ++i) {
        m_gemHwMonitorGLIB.push_back(new gemHwMonitorGLIB());
        m_gemHwMonitorGLIB.back()->setDeviceConfiguration(*m_gemHwMonitorCrate.back()->getDevice()->getSubDevicesRefs().at(i));
        m_gemHwMonitorCrate.back()->addSubDeviceStatus(0);
        std::map <std::string, std::string> glibProperties_;
        glibProperties_ = m_gemHwMonitorGLIB.back()->getDevice()->getDeviceProperties();
        for (auto it = glibProperties_.begin(); it != glibProperties_.end(); ++it)
          if (it->first == "IP") m_glibIP = it->second;

        for (unsigned i = 0; i != m_gemHwMonitorGLIB.back()->getDevice()->getSubDevicesRefs().size(); ++i) {
          m_gemHwMonitorOH.push_back(new gemHwMonitorOH());
          m_gemHwMonitorOH.back()->setDeviceConfiguration(*m_gemHwMonitorGLIB.back()->getDevice()->getSubDevicesRefs().at(i));
          m_gemHwMonitorGLIB.back()->addSubDeviceStatus(0);
          for (long long int i = 0; i < 24; ++i) {  // because compiler doesn't reconginze -std=c++11...
            m_gemHwMonitorVFAT.push_back(new gemHwMonitorVFAT());
            m_gemHwMonitorVFAT.back()->setDeviceStatus(3);
            m_gemHwMonitorOH.back()->addSubDeviceStatus(3);
            std::string vfatName = "VFAT";
            vfatName += std::to_string(i);
            m_gemHwMonitorVFAT.back()->getDevice()->setDeviceId(vfatName.c_str());
            for (unsigned j = 0; j != m_gemHwMonitorOH.back()->getDevice()->getSubDevicesRefs().size(); ++j) {
              if (m_gemHwMonitorOH.back()->getDevice()->getSubDevicesRefs().at(j)->getDeviceId() == m_gemHwMonitorVFAT.back()->getDevice()->getDeviceId()) {
                m_gemHwMonitorVFAT.back()->setDeviceConfiguration(*m_gemHwMonitorOH.back()->getDevice()->getSubDevicesRefs().at(j));
                std::stringstream tmpURI;
                tmpURI << "chtcp-2.0://localhost:10203?target=" << m_glibIP << ":50001";
                p_vfatDevice = vfat_shared_ptr(new gem::hw::vfat::HwVFAT2(m_gemHwMonitorVFAT.back()->getDevice()->getDeviceId(),
                                                                          tmpURI.str(),
                                                                          "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));
                std::cout << "vfat ID from XML: " << m_gemHwMonitorVFAT.back()->getDevice()->getDeviceId() << std::endl;
                if (p_vfatDevice->isHwConnected()) {
                  m_gemHwMonitorVFAT.back()->setDeviceStatus(0);
                  m_gemHwMonitorOH.back()->setSubDeviceStatus(0, i);
                  // m_gemHwMonitorOH.back()->addSubDeviceStatus(0);
                } else {
                  m_gemHwMonitorVFAT.back()->setDeviceStatus(2);
                  m_gemHwMonitorOH.back()->setSubDeviceStatus(2, i);
                  // m_gemHwMonitorOH.back()->addSubDeviceStatus(2);
                }
                // delete p_vfatDevice;
              }
            }
          }
        }
      }
    }
  }
  this->controlPanel(in, out);
}

void gem::hwMonitor::gemHwMonitorWeb::selectCrate(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
}

void gem::hwMonitor::gemHwMonitorWeb::expandCrate(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);
  m_crateToShow = cgi.getElement("crateButton")->getValue();
  // for (auto i = m_gemHwMonitorSystem->getDevice()->getSubDevicesRefs().begin(); i != m_gemHwMonitorSystem->getDevice()->getSubDevicesRefs().end(); ++i)
  // {
  // if (i->getDeviceId() == m_crateToShow) {m_gemHwMonitorCrate->setDeviceConfiguration(*i);}
  //  Auto-pointer doesn't work for some reason. Improve this later.
  for (unsigned i = 0; i != m_gemHwMonitorSystem->getDevice()->getSubDevicesRefs().size(); ++i) {
    if (m_gemHwMonitorSystem->getDevice()->getSubDevicesRefs().at(i)->getDeviceId() == m_crateToShow) {
      m_gemHwMonitorCrate.at(i)->setDeviceConfiguration(*m_gemHwMonitorSystem->getDevice()->getSubDevicesRefs().at(i));
      m_indexCrate = i;
      for (int i = 0; i < m_gemHwMonitorCrate.at(m_indexCrate)->getNumberOfSubDevices(); ++i) {
        m_gemHwMonitorGLIB.at(i)->setDeviceConfiguration(*m_gemHwMonitorCrate.at(m_indexCrate)->getDevice()->getSubDevicesRefs().at(i));
        std::map <std::string, std::string> glibProperties_;
        glibProperties_ = m_gemHwMonitorGLIB.at(i)->getDevice()->getDeviceProperties();
        for (auto it = glibProperties_.begin(); it != glibProperties_.end(); ++it) {
          if (it->first == "IP") m_glibIP = it->second;
          std::cout << "property: " << it->first << " - GLIB IP is " << m_glibIP << std::endl;
        }
        std::stringstream tmpURI;
        tmpURI << "chtcp-2.0://localhost:10203?target=" << m_glibIP << ":50001";
        p_glibDevice = glib_shared_ptr(new gem::hw::glib::HwGLIB("HwGLIB", tmpURI.str(),
                                                                "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));
        if (p_glibDevice->isHwConnected())
          {
            m_gemHwMonitorCrate.at(m_indexCrate)->addSubDeviceStatus(0);
          } else {
          m_gemHwMonitorCrate.at(m_indexCrate)->addSubDeviceStatus(2);
        }
      }
    }
  }
  this->cratePanel(in, out);
}

void gem::hwMonitor::gemHwMonitorWeb::cratePanel(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  *out << "<link rel=\"stylesheet\" type=\"text/css\" "
       << "href=\"/gemdaq/gemHwMonitor/html/css/bootstrap.css\">"
       << std::endl
       << "<link rel=\"stylesheet\" type=\"text/css\" "
       << "href=\"/gemdaq/gemHwMonitor/html/css/bootstrap-theme.css\">"
       << std::endl;
  *out << "<div class=\"panel panel-primary\">" << std::endl;
  *out << "<div class=\"panel-heading\">" << std::endl;
  *out << "<h1><div align=\"center\">Chip Id : "<< m_crateToShow << "</div></h1>" << std::endl;
  *out << "</div>" << std::endl;
  *out << "<div class=\"panel-body\">" << std::endl;
  *out << "<h3><div class=\"alert alert-info\" role=\"alert\" align=\"center\">Device base node : "
       << "ENDCAP 1"
       << "<br> PLACEHOLDER <br>"<< "</div></h3>" << std::endl;
  *out << cgicc::hr()<< std::endl;
  *out << "<h2><div align=\"center\">Connected AMC13 boards</div></h2>" << std::endl;
  *out << cgicc::br()<< std::endl;
  *out << "<h3><div align=\"center\">There're no AMC13 boards</div></h3>" << std::endl;
  *out << cgicc::hr()<< std::endl;
  std::string methodExpandGLIB = toolbox::toString("/%s/expandGLIB",
                                                   getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::table().set("class", "table");
  *out << "<tr><h2><div align=\"center\">Connected GLIBs</div></h2></tr>" << std::endl;
  *out << "<tr>" << std::endl;
  for (int i = 0; i < m_gemHwMonitorCrate.at(m_indexCrate)->getNumberOfSubDevices(); ++i) {
    std::string currentGLIBId;
    currentGLIBId += m_gemHwMonitorCrate.at(m_indexCrate)->getCurrentSubDeviceId(i);
    *out << cgicc::td();
    *out << cgicc::form().set("method", "POST").set("action", methodExpandGLIB) << std::endl ;
    if (m_gemHwMonitorCrate.at(m_indexCrate)->getSubDeviceStatus(i) == 0) {
      *out << "<button type=\"submit\" class=\"btn btn-success\" name=\"glibButton\" value=\""
           << currentGLIBId << "\">" << currentGLIBId<< "</button>" << std::endl;
    } else if (m_gemHwMonitorCrate.at(m_indexCrate)->getSubDeviceStatus(i) == 1) {
      *out << "<button type=\"submit\" class=\"btn btn-warning\" name=\"glibButton\" value=\""
           << currentGLIBId << "\">" << currentGLIBId<< "</button>" << std::endl;
    } else if (m_gemHwMonitorCrate.at(m_indexCrate)->getSubDeviceStatus(i) == 2) {
      *out << "<button type=\"submit\" class=\"btn btn-disabled\" name=\"glibButton\" value=\""
           << currentGLIBId << "\" disabled>" << currentGLIBId<< "</button>" << std::endl;
    }
    *out << cgicc::form() << std::endl ;
    *out << cgicc::td();
  }
  *out << "</tr>" << std::endl;
  *out << cgicc::table() <<std::endl;;
  *out << "</div>" << std::endl;
  *out << cgicc::br()<< std::endl;
  *out << cgicc::hr()<< std::endl;
}

void gem::hwMonitor::gemHwMonitorWeb::expandGLIB(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);
  m_glibToShow = cgi.getElement("glibButton")->getValue();
  // Auto-pointer doesn't work for some reason. Improve this later.
  for (unsigned i = 0; i != m_gemHwMonitorCrate.at(m_indexCrate)->getDevice()->getSubDevicesRefs().size(); ++i) {
    if (m_gemHwMonitorCrate.at(m_indexCrate)->getDevice()->getSubDevicesRefs().at(i)->getDeviceId() == m_glibToShow) {
      m_indexGLIB = i;
      for (int i = 0; i < m_gemHwMonitorGLIB.at(m_indexGLIB)->getNumberOfSubDevices(); ++i) {
        std::map <std::string, std::string> glibProperties_;
        glibProperties_ = m_gemHwMonitorGLIB.at(m_indexGLIB)->getDevice()->getDeviceProperties();
        std::string ohIP = "";
        for (auto it = glibProperties_.begin(); it != glibProperties_.end(); ++it) {
          if (it->first == "IP") ohIP = it->second;
          std::cout << "property: " << it->first << " - OH IP is " << ohIP << std::endl;
        }
        // this needs to come from the xml config somehow
        int ohGTX = 0;
        std::string currentOHId = toolbox::toString("HwOptoHybrid_%d", ohGTX);
        std::stringstream tmpURI;
        tmpURI << "chtcp-2.0://localhost:10203?target=" << ohIP << ":50001";
        p_ohDevice = optohybrid_shared_ptr(new gem::hw::optohybrid::HwOptoHybrid(currentOHId, tmpURI.str(),
                                                                                "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));
        if (p_ohDevice->isHwConnected()) {
          m_gemHwMonitorGLIB.at(m_indexGLIB)->addSubDeviceStatus(0);
        } else {
          m_gemHwMonitorGLIB.at(m_indexGLIB)->addSubDeviceStatus(2);
        }
      }
    }
  }
  this->glibPanel(in, out);
}

void gem::hwMonitor::gemHwMonitorWeb::glibPanel(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  *out << "<link rel=\"stylesheet\" type=\"text/css\" "
       << "href=\"/gemdaq/gemHwMonitor/html/css/bootstrap.css\">"
       << std::endl
       << "<link rel=\"stylesheet\" type=\"text/css\" "
       << "href=\"/gemdaq/gemHwMonitor/html/css/bootstrap-theme.css\">"
       << std::endl;
  std::string methodExpandCrate = toolbox::toString("/%s/expandCrate",
                                                    getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::table().set("class", "table");
  *out << "</tr>" << std::endl;
  *out << cgicc::td();
  *out << cgicc::form().set("method", "POST").set("action", methodExpandCrate) << std::endl ;
  *out << "<button type=\"submit\" class=\"btn btn-info\" name=\"crateButton\" value=\""
       << m_crateToShow << "\">" << m_crateToShow<< "</button>" << std::endl;
  *out << cgicc::form() << std::endl ;
  *out << cgicc::td();
  *out << "</tr>" << std::endl;
  *out << cgicc::table() <<std::endl;

  std::map <std::string, std::string> glibProperties_;
  glibProperties_ = m_gemHwMonitorGLIB.at(m_indexGLIB)->getDevice()->getDeviceProperties();
  for (auto it = glibProperties_.begin(); it != glibProperties_.end(); ++it)
    if (it->first == "IP") m_glibIP = it->second;

  std::stringstream tmpURI;
  tmpURI << "chtcp-2.0://localhost:10203?target=" << m_glibIP << ":50001";
  p_glibDevice = glib_shared_ptr(new gem::hw::glib::HwGLIB("HwGLIB", tmpURI.str(),
                                                          "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));

  *out << "<div class=\"panel panel-primary\">" << std::endl;
  *out << "<div class=\"panel-heading\">"       << std::endl;
  *out << "<h1><div align=\"center\">Chip Id : "<< m_glibToShow
       << "<br> Firmware version : " << p_glibDevice->getUserFirmwareDate()
       << "</div></h1>" << std::endl;
  *out << "</div>"      << std::endl;
  *out << "<div class=\"panel-body\">" << std::endl;
  *out << "<h3><div class=\"alert alert-info\" role=\"alert\" align=\"center\">Device base node : "
       << m_crateToShow << "</div></h3>" << std::endl;
  std::string methodExpandOH = toolbox::toString("/%s/expandOH",
                                                 getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::table().set("class", "table");
  *out << "<tr><h2><div align=\"center\">Connected Optohybrids</div></h2></tr>" << std::endl;
  *out << "<tr>" << std::endl;
  for (int i = 0; i < m_gemHwMonitorGLIB.at(m_indexGLIB)->getNumberOfSubDevices(); ++i) {
    std::string currentOHId;
    currentOHId += m_gemHwMonitorGLIB.at(m_indexGLIB)->getCurrentSubDeviceId(i);
    *out << cgicc::td();
    *out << cgicc::form().set("method", "POST").set("action", methodExpandOH) << std::endl ;
    if (m_gemHwMonitorGLIB.at(m_indexGLIB)->getSubDeviceStatus(i) == 0) {
      *out << "<button type=\"submit\" class=\"btn btn-success\" name=\"ohButton\" value=\""
           << currentOHId << "\">" << currentOHId<< "</button>" << std::endl;
    } else if (m_gemHwMonitorGLIB.at(m_indexGLIB)->getSubDeviceStatus(i) == 1) {
      *out << "<button type=\"submit\" class=\"btn btn-warning\" name=\"ohButton\" value=\""
           << currentOHId << "\">" << currentOHId<< "</button>" << std::endl;
    } else if (m_gemHwMonitorGLIB.at(m_indexGLIB)->getSubDeviceStatus(i) == 2) {
      *out << "<button type=\"submit\" class=\"btn btn-disabled\" name=\"ohButton\" value=\""
           << currentOHId << "\" disabled>" << currentOHId<< "</button>" << std::endl;
    }
    *out << cgicc::form() << std::endl;
    *out << cgicc::td();
  }
  *out << "</tr>" << std::endl;
  *out << cgicc::table() <<std::endl;

  gem::hw::GEMHwDevice::OpticalLinkStatus linkStatus_;

  // moved table header outside the loop
  *out << cgicc::table().set("class", "table");
  *out << "<tr>" << std::endl;
  *out << "<td>" << std::endl;
  *out << "GTX N" << std::endl;
  *out << "</td>" << std::endl;
  *out << "<td>" << std::endl;
  *out << "Tracking Link Err" << std::endl;
  *out << "</td>" << std::endl;
  *out << "<td>" << std::endl;
  *out << "Trigger Link Err" << std::endl;
  *out << "</td>" << std::endl;
  *out << "<td>" << std::endl;
  *out << "Received Data Packets" << std::endl;
  *out << "</td>" << std::endl;
  *out << "</tr>" << std::endl;

  for (uint8_t i = 0; i < gem::hw::glib::HwGLIB::N_GTX; ++i) {
    linkStatus_ = p_glibDevice->LinkStatus(i);
    *out << "<tr>" << std::endl;
    *out << "<td>" << std::endl;
    *out << static_cast<int>(i) << std::endl;
    *out << "</td>" << std::endl;
    *out << "<td>" << std::endl;
    *out << linkStatus_.TRK_Errors << std::endl;
    *out << "</td>" << std::endl;
    *out << "<td>" << std::endl;
    *out << linkStatus_.TRG_Errors << std::endl;
    *out << "</td>" << std::endl;
    *out << "<td>" << std::endl;
    *out << linkStatus_.Data_Packets << std::endl;
    *out << "</td>" << std::endl;
    *out << "</tr>" << std::endl;
    }

  // moved table header outside the loop
  *out << cgicc::table() <<std::endl;

  *out << cgicc::table().set("class", "table");
  *out << "<tr>" << std::endl;
  *out << "<td>" << std::endl;
  *out << "Device IP" << std::endl;
  *out << "</td>" << std::endl;
  *out << "<td>" << std::endl;
  *out << p_glibDevice->getIPAddress() << std::endl;
  *out << "</td>" << std::endl;
  *out << "</td>" << std::endl;
  *out << "</tr>" << std::endl;
  *out << "<tr>" << std::endl;
  *out << "<td>" << std::endl;
  *out << "Device MAC address" << std::endl;
  *out << "</td>" << std::endl;
  *out << "<td>" << std::endl;
  *out << p_glibDevice->getMACAddress() << std::endl;
  *out << "</td>" << std::endl;
  *out << "</td>" << std::endl;
  *out << "</tr>" << std::endl;
  *out << "<tr>" << std::endl;
  *out << "<td>" << std::endl;
  *out << "PCIe clock multiplier" << std::endl;
  *out << "</td>" << std::endl;
  *out << "<td>" << std::endl;
  *out << static_cast<int>(p_glibDevice->PCIeClkFSel()) << std::endl;
  *out << "</td>" << std::endl;
  *out << "</td>" << std::endl;
  *out << "</tr>" << std::endl;
  *out << "<tr>" << std::endl;
  *out << "<td>" << std::endl;
  *out << "PCIe clock reset state" << std::endl;
  *out << "</td>" << std::endl;
  *out << "<td>" << std::endl;
  *out << static_cast<int>(p_glibDevice->PCIeClkMaster()) << std::endl;
  *out << "</td>" << std::endl;
  *out << "</td>" << std::endl;
  *out << "</tr>" << std::endl;
  *out << "<tr>" << std::endl;
  *out << "<td>" << std::endl;
  *out << "PCIe clock output status" << std::endl;
  *out << "</td>" << std::endl;
  *out << "<td>" << std::endl;
  *out << static_cast<int>(p_glibDevice->PCIeClkOutput()) << std::endl;
  *out << "</td>" << std::endl;
  *out << "</td>" << std::endl;
  *out << "</tr>" << std::endl;
  *out << "<tr>" << std::endl;
  *out << "<td>" << std::endl;
  *out << "CDCE clock output status" << std::endl;
  *out << "</td>" << std::endl;
  *out << "<td>" << std::endl;
  *out << static_cast<int>(p_glibDevice->CDCEPower()) << std::endl;
  *out << "</td>" << std::endl;
  *out << "</td>" << std::endl;
  *out << "</tr>" << std::endl;
  *out << "<tr>" << std::endl;
  *out << "<td>" << std::endl;
  *out << "CDCE reference clock" << std::endl;
  *out << "</td>" << std::endl;
  *out << "<td>" << std::endl;
  *out << static_cast<int>(p_glibDevice->CDCEReference()) << std::endl;
  *out << "</td>" << std::endl;
  *out << "</td>" << std::endl;
  *out << "</tr>" << std::endl;
  *out << "<tr>" << std::endl;
  *out << "<td>" << std::endl;
  *out << "CDCE syncronization status" << std::endl;
  *out << "</td>" << std::endl;
  *out << "<td>" << std::endl;
  *out << static_cast<int>(p_glibDevice->CDCESync()) << std::endl;
  *out << "</td>" << std::endl;
  *out << "</td>" << std::endl;
  *out << "</tr>" << std::endl;
  *out << "<tr>" << std::endl;
  *out << "<td>" << std::endl;
  *out << "CDCE control output status" << std::endl;
  *out << "</td>" << std::endl;
  *out << "<td>" << std::endl;
  *out << static_cast<int>(p_glibDevice->CDCEControl()) << std::endl;
  *out << "</td>" << std::endl;
  *out << "</td>" << std::endl;
  *out << "</tr>" << std::endl;
  *out << "<td>" << std::endl;
  *out << "TClkB output to the backplane status" << std::endl;
  *out << "</td>" << std::endl;
  *out << "<td>" << std::endl;
  *out << static_cast<int>(p_glibDevice->TClkBOutput()) << std::endl;
  *out << "</td>" << std::endl;
  *out << "</td>" << std::endl;
  *out << "</tr>" << std::endl;

  *out << cgicc::table() <<std::endl;

  *out << "</div>" << std::endl;
  *out << cgicc::br()<< std::endl;
  *out << cgicc::hr()<< std::endl;
}

void gem::hwMonitor::gemHwMonitorWeb::expandOH(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);
  m_ohToShow = cgi.getElement("ohButton")->getValue();
  // Auto-pointer doesn't work for some reason. Improve this later.
  for (unsigned i = 0; i != m_gemHwMonitorGLIB.at(m_indexGLIB)->getDevice()->getSubDevicesRefs().size(); ++i) {
    if ((m_gemHwMonitorGLIB.at(m_indexGLIB)->getDevice()->getSubDevicesRefs().at(i)->getDeviceId() == m_ohToShow)
        && (!(m_gemHwMonitorOH.at(i)->isConfigured()))) {
      m_indexOH = i;
      m_gemHwMonitorOH.at(m_indexOH)->setIsConfigured(true);
      std::stringstream tmpURI;
      tmpURI << "chtcp-2.0://localhost:10203?target=" << m_glibIP << ":50001";
      p_vfatDevice = vfat_shared_ptr(new gem::hw::vfat::HwVFAT2(m_vfatToShow, tmpURI.str(),
                                                               "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));
      for (int i = 0; i < m_gemHwMonitorOH.at(m_indexOH)->getNumberOfSubDevices(); ++i) {
        std::string vfatID_ = m_gemHwMonitorOH.at(m_indexOH)->getDevice()->getSubDevicesRefs().at(i)->getDeviceId();
        std::cout << "vfat ID from XML" << vfatID_ << std::endl;
        if (p_vfatDevice->isHwConnected()) {
          m_gemHwMonitorOH.at(m_indexOH)->addSubDeviceStatus(0);
        } else {
          m_gemHwMonitorOH.at(m_indexOH)->addSubDeviceStatus(2);
        }
      }
    }
  }
  this->ohPanel(in, out);
}

void gem::hwMonitor::gemHwMonitorWeb::ohPanel(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  *out << "<link rel=\"stylesheet\" type=\"text/css\" "
       << "href=\"/gemdaq/gemHwMonitor/html/css/bootstrap.css\">"
       << std::endl
       << "<link rel=\"stylesheet\" type=\"text/css\" "
       << "href=\"/gemdaq/gemHwMonitor/html/css/bootstrap-theme.css\">"
       << std::endl;
  std::string methodExpandCrate = toolbox::toString("/%s/expandCrate",
                                                    getApplicationDescriptor()->getURN().c_str());
  std::string methodExpandGLIB = toolbox::toString("/%s/expandGLIB",
                                                   getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::table().set("class", "table");
  *out << "</tr>" << std::endl;
  *out << cgicc::td();
  *out << cgicc::form().set("method", "POST").set("action", methodExpandCrate) << std::endl ;
  *out << "<button type=\"submit\" class=\"btn btn-info\" name=\"crateButton\" value=\""
       << m_crateToShow << "\">" << m_crateToShow<< "</button>" << std::endl;
  *out << cgicc::form() << std::endl ;
  *out << cgicc::td();
  *out << cgicc::td();
  *out << cgicc::form().set("method", "POST").set("action", methodExpandGLIB) << std::endl ;
  *out << "<button type=\"submit\" class=\"btn btn-info\" name=\"glibButton\" value=\""
       << m_glibToShow << "\">" << m_glibToShow<< "</button>" << std::endl;
  *out << cgicc::form() << std::endl ;
  *out << cgicc::td();
  *out << "</tr>" << std::endl;
  *out << cgicc::table() <<std::endl;;

  // this needs to come from the xml config somehow
  int ohGTX = 0;
  std::string currentOHId = toolbox::toString("HwOptoHybrid_%d", ohGTX);
  std::stringstream tmpURI;
  tmpURI << "chtcp-2.0://localhost:10203?target=" << m_glibIP << ":50001";
  p_ohDevice = optohybrid_shared_ptr(new gem::hw::optohybrid::HwOptoHybrid(currentOHId, tmpURI.str(),
                                                                                "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));
  if (!p_ohDevice->isHwConnected()) {
    *out << "<h1><div align=\"center\">Device connection failed!</div></h1>" << std::endl;
  } else {
    std::vector<gem::hw::GEMHwDevice::linkStatus> activeLinks_;
    activeLinks_ = p_ohDevice->getActiveLinks();
    *out << "<div class=\"panel panel-primary\">" << std::endl;
    *out << "<div class=\"panel-heading\">" << std::endl;
    *out << "<h1><div align=\"center\">Chip Id : "<< m_ohToShow << "<br> Firmware version : "
         << p_ohDevice->getFirmwareDate() << "</div></h1>" << std::endl;
    *out << "</div>" << std::endl;
    *out << "<div class=\"panel-body\">" << std::endl;
    *out << "<h3><div class=\"alert alert-info\" role=\"alert\" align=\"center\">Device base node : "
         << m_crateToShow << "::" << m_glibToShow << "</div></h3>" << std::endl;
    std::string methodExpandVFAT = toolbox::toString("/%s/expandVFAT",
                                                     getApplicationDescriptor()->getURN().c_str());

    gem::hw::GEMHwDevice::OpticalLinkStatus linkStatus_;
    /*
    for (int i = 0; i < 3; ++i) {
      if (!p_ohDevice->isLinkActive(i)) {
        *out << "<div class=\"panel panel-danger\">" << std::endl;
        *out << "<div class=\"panel-heading\">" << std::endl;
        *out << "<tr><h2><div align=\"center\">LINK " << i << " is not available </div></h2></tr>" << std::endl;
        *out << "</div>" << std::endl;
        *out << "</div>" << std::endl;
      }
    }
    */
    // for (auto l = activeLinks_.begin(); l != activeLinks_.end(); ++l) {  // no longer multiple links on OH
    *out << "<div class=\"panel panel-info\">" << std::endl;
    *out << "<div class=\"panel-heading\">" << std::endl;

    *out << cgicc::table().set("class", "table");
    *out << "<tr><h2><div align=\"center\">Optical Link Status </div></h2></tr>" << std::endl;
    *out << "<tr>" << std::endl;
    *out << "<td>" << std::endl;
    *out << "Tracking Link Err" << std::endl;
    *out << "</td>" << std::endl;
    *out << "<td>" << std::endl;
    *out << "Trigger Link Err" << std::endl;
    *out << "</td>" << std::endl;
    *out << "<td>" << std::endl;
    *out << "Received Data Packets" << std::endl;
    *out << "</td>" << std::endl;
    *out << "</tr>" << std::endl;

    linkStatus_ = p_ohDevice->LinkStatus();
    *out << "<tr>" << std::endl;
    *out << "<td>" << std::endl;
    *out << linkStatus_.TRK_Errors << std::endl;
    *out << "</td>" << std::endl;
    *out << "<td>" << std::endl;
    *out << linkStatus_.TRG_Errors << std::endl;
    *out << "</td>" << std::endl;
    *out << "<td>" << std::endl;
    *out << linkStatus_.Data_Packets << std::endl;
    *out << "</td>" << std::endl;
    *out << "</tr>" << std::endl;
    *out << cgicc::table() <<std::endl;

    *out << cgicc::table().set("class", "table");
    *out << "<tr><h3><div align=\"center\">Connected VFAT's</div></h3></tr>" << std::endl;
    *out << "<tr>" << std::endl;
    // for (int i = 0; i < m_gemHwMonitorOH.at(m_indexOH)->getNumberOfSubDevices(); ++i) {
    // int linkIncreement = 8*i;
    for (long long int i = 0; i < 24; ++i) {  // because compiler doesn't reconginze -std=c++11…
      std::string currentVFATId = "VFAT";
      // currentVFATId += m_gemHwMonitorOH.at(m_indexOH)->getCurrentSubDeviceId(i+linkIncreement);
      currentVFATId += std::to_string(i);
      std::stringstream tmpURI;
      tmpURI << "chtcp-2.0://localhost:10203?target=" << m_glibIP << ":50001";
      p_vfatDevice = vfat_shared_ptr(new gem::hw::vfat::HwVFAT2(currentVFATId, tmpURI.str(),
                                                               "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));
      std::string runmode;
      int n_chan = 0;
      if (p_vfatDevice->isHwConnected()) {
        p_vfatDevice->getAllSettings();  // takes time. See with Jared how to make it better
        runmode = gem::hw::vfat::RunModeToString.at(p_vfatDevice->getVFAT2Params().runMode);
        for (uint8_t chan = 1; chan < 129; ++chan) {
          if (p_vfatDevice->getVFAT2Params().channels[chan-1].mask < 1) ++n_chan;
        }
      } else {
        runmode = "N/A";
      }

      *out << cgicc::td();
      *out << cgicc::form().set("method", "POST").set("action", methodExpandVFAT) << std::endl ;
      // if (m_gemHwMonitorOH.at(m_indexOH)->getSubDeviceStatus(i+linkIncreement) == 0) {
      if (m_gemHwMonitorOH.at(m_indexOH)->getSubDeviceStatus(i) == 0) {
        *out << "<div align=\"center\">"
             << "<button type=\"submit\" class=\"btn btn-success\" name=\"vfatButton\" value=\""
             << currentVFATId << "\">" << n_chan << "</button></div>" << std::endl;
      } else if (m_gemHwMonitorOH.at(m_indexOH)->getSubDeviceStatus(i) == 1) {
        *out << "<div align=\"center\">"
             << "<button type=\"submit\" class=\"btn btn-warning\" name=\"vfatButton\" value=\""
             << currentVFATId << "\">" << n_chan << "</button></div>" << std::endl;
      } else if (m_gemHwMonitorOH.at(m_indexOH)->getSubDeviceStatus(i) == 2) {
        *out << "<div align=\"center\">"
             << "<button type=\"submit\" class=\"btn btn-danger\" name=\"vfatButton\" value=\""
             << currentVFATId << "\">" <<  "000" << "</button></div>" << std::endl;
      } else if (m_gemHwMonitorOH.at(m_indexOH)->getSubDeviceStatus(i) == 3) {
        *out << "<div align=\"center\">"
             << "<button type=\"submit\" class=\"btn btn-disabled\" name=\"vfatButton\" value=\""
             << currentVFATId << "\" disabled>" << "000" << "</button></div>" << std::endl;
      }

      *out << cgicc::form() << std::endl;
      *out << cgicc::br();
      *out << "<div align=\"center\">" << runmode << "</div>" << std::endl;
      *out << cgicc::td();
    }
    *out << "</tr>" << std::endl;
    *out << cgicc::table() <<std::endl;
    *out << "</div>" << std::endl;
    *out << cgicc::br();
    // }  // no more link dependence

    /*
    std::pair<bool,bool> statusVFATClock_;
    statusVFATClock_ = p_ohDevice->StatusVFATClock();
    *out << cgicc::table().set("class", "table");
    *out << "<tr>" << std::endl;
    *out << "<td>" << std::endl;
    *out << "VFAT Clock Source" << std::endl;
    *out << "</td>" << std::endl;
    *out << "<td>" << std::endl;
    *out << statusVFATClock_.first << std::endl;
    *out << "</td>" << std::endl;
    *out << "</tr>" << std::endl;
    *out << "<tr>" << std::endl;
    *out << "<td>" << std::endl;
    *out << "VFAT Clock Fallback" << std::endl;
    *out << "</td>" << std::endl;
    *out << "<td>" << std::endl;
    *out << statusVFATClock_.second << std::endl;
    *out << "</td>" << std::endl;
    *out << "</tr>" << std::endl;
    // *out << cgicc::table() <<std::endl;

    std::pair<bool,bool> statusCDCEClock_;
    statusCDCEClock_ = p_ohDevice->StatusCDCEClock();
    // *out << cgicc::table().set("class", "table");
    *out << "<tr>" << std::endl;
    *out << "<td>" << std::endl;
    *out << "CDCE Clock Source" << std::endl;
    *out << "</td>" << std::endl;
    *out << "<td>" << std::endl;
    *out << statusCDCEClock_.first << std::endl;
    *out << "</td>" << std::endl;
    *out << "</tr>" << std::endl;
    *out << "<tr>" << std::endl;
    *out << "<td>" << std::endl;
    *out << "CDCE Clock Fallback" << std::endl;
    *out << "</td>" << std::endl;
    *out << "<td>" << std::endl;
    *out << statusCDCEClock_.second << std::endl;
    *out << "</td>" << std::endl;
    *out << "</tr>" << std::endl;
    // *out << cgicc::table() <<std::endl;
    */
    // *out << cgicc::table().set("class", "table");
    *out << "<tr>" << std::endl;
    *out << "<td>" << std::endl;
    *out << "Reference Clock Source" << std::endl;
    *out << "</td>" << std::endl;
    *out << "<td>" << std::endl;
    *out << static_cast<int>(p_ohDevice->getReferenceClock()) << std::endl;
    *out << "</td>" << std::endl;
    *out << "</tr>" << std::endl;
    *out << "<tr>" << std::endl;
    *out << "<td>" << std::endl;
    *out << "Trigger Source" << std::endl;
    *out << "</td>" << std::endl;
    *out << "<td>" << std::endl;
    *out << static_cast<int>(p_ohDevice->getTrigSource()) << std::endl;
    *out << "</td>" << std::endl;
    *out << "</tr>" << std::endl;
    *out << "<tr>" << std::endl;
    *out << "<td>" << std::endl;
    *out << "S-bit Source" << std::endl;
    *out << "</td>" << std::endl;
    *out << "<td>" << std::endl;
    *out << static_cast<int>(p_ohDevice->getSBitSource()) << std::endl;
    *out << "</td>" << std::endl;
    *out << "</tr>" << std::endl;
    // *out << cgicc::table() <<std::endl;

    const char *l1CountNames[] = {"TTC T1 Counter",
                                  "Internal T1 Counter",
                                  "External T1 Counter",
                                  "Loopback T1 Counter",
                                  "Sent T1 Counter"};
    *out << cgicc::table().set("class", "table");
    for (uint8_t i = 0; i < 5; ++i) {
      *out << "<tr>"  << std::endl;
      *out << "<td>"  << std::endl;
      *out << "</td>" << std::endl;
      *out << "<td>"  << std::endl;
      *out << l1CountNames[i] << std::endl;
      *out << "</td>" << std::endl;
      *out << "</tr>" << std::endl;
    }

    for (uint8_t i = 0; i < 5; ++i) {
      *out << "<tr>" << std::endl;
      *out << "<td>" << std::endl;
      *out << "L1A" << std::endl;
      *out << "</td>" << std::endl;
      *out << "<td>" << std::endl;
      *out << p_ohDevice->getL1ACount(i) << std::endl;
      *out << "</td>" << std::endl;
      *out << "</tr>" << std::endl;

      *out << "<tr>" << std::endl;
      *out << "<td>" << std::endl;
      *out << "CalPulse" << std::endl;
      *out << "</td>" << std::endl;
      *out << "<td>" << std::endl;
      *out << p_ohDevice->getCalPulseCount(i) << std::endl;
      *out << "</td>" << std::endl;
      *out << "</tr>" << std::endl;

      *out << "<tr>" << std::endl;
      *out << "<td>" << std::endl;
      *out << "Resync" << std::endl;
      *out << "</td>" << std::endl;
      *out << "<td>" << std::endl;
      *out << p_ohDevice->getResyncCount(i) << std::endl;
      *out << "</td>" << std::endl;
      *out << "</tr>" << std::endl;

      *out << "<tr>" << std::endl;
      *out << "<td>" << std::endl;
      *out << "BC0" << std::endl;
      *out << "</td>" << std::endl;
      *out << "<td>" << std::endl;
      *out << p_ohDevice->getBC0Count(i) << std::endl;
      *out << "</td>" << std::endl;
      *out << "</tr>" << std::endl;
    }
    *out << cgicc::table() <<std::endl;

    *out << "</div>" << std::endl;
  }
  *out << cgicc::br()<< std::endl;
  *out << cgicc::hr()<< std::endl;
}

void gem::hwMonitor::gemHwMonitorWeb::expandVFAT(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);
  m_vfatToShow = cgi.getElement("vfatButton")->getValue();
  // Auto-pointer doesn't work for some reason. Improve this later.
  // for (unsigned i = 0; i != m_gemHwMonitorOH.at(m_indexOH)->getDevice()->getSubDevicesRefs().size(); ++i)
  for (int i = 24*m_indexOH; i < 24*(m_indexOH+1); ++i) {
    // if (m_gemHwMonitorOH.at(m_indexOH)->getDevice()->getSubDevicesRefs().at(i)->getDeviceId() == m_vfatToShow)
    if (m_gemHwMonitorVFAT.at(i)->getDevice()->getDeviceId() == m_vfatToShow) {
      m_indexVFAT = i;
    }
  }
  this->vfatPanel(in, out);
}

void gem::hwMonitor::gemHwMonitorWeb::vfatPanel(xgi::Input* in, xgi::Output* out )
  throw (xgi::exception::Exception)
{
  *out << "<link rel=\"stylesheet\" type=\"text/css\" "
       << "href=\"/gemdaq/gemHwMonitor/html/css/bootstrap.css\">"
       << std::endl
       << "<link rel=\"stylesheet\" type=\"text/css\" "
       << "href=\"/gemdaq/gemHwMonitor/html/css/bootstrap-theme.css\">"
       << std::endl;
  *out << "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js\"></script>"
       << std::endl;
  *out << "<script src=\"/gemdaq/gemHwMonitor/html/js/bootstrap.min.js\"></script>" << std::endl;

  if (m_gemHwMonitorVFAT.at(m_indexVFAT)->getDeviceStatus() == 2) {
    *out << "<div class=\"panel panel-danger\">" << std::endl;
    *out << "<div class=\"panel-heading\">" << std::endl;
    *out << "<h1><div align=\"center\">Chip Id : "<< m_vfatToShow << " is not responding</div></h1>"
         << std::endl;
    *out << "<br>" << std::endl;
    *out << "<h3><div class=\"alert alert-info\" role=\"alert\" align=\"center\">Device base node : "
         << m_crateToShow << "::" << m_glibToShow << "::" << m_ohToShow <<  "</div></h3>" << std::endl;
    *out << "</div>" << std::endl;
    *out << "</div>" << std::endl;
  } else {
    std::stringstream tmpURI;
    tmpURI << "chtcp-2.0://localhost:10203?target=" << m_glibIP << ":50001";
    p_vfatDevice = vfat_shared_ptr(new gem::hw::vfat::HwVFAT2(m_vfatToShow, tmpURI.str(),
                                                             "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));
    p_vfatDevice->getAllSettings();
    std::string methodExpandCrate = toolbox::toString("/%s/expandCrate",
                                                      getApplicationDescriptor()->getURN().c_str());
    std::string methodExpandGLIB = toolbox::toString("/%s/expandGLIB",
                                                     getApplicationDescriptor()->getURN().c_str());
    std::string methodExpandOH = toolbox::toString("/%s/expandOH",
                                                   getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::table().set("class", "table");
    *out << "<tr>" << std::endl;
    *out << cgicc::td();
    *out << cgicc::form().set("method", "POST").set("action", methodExpandCrate) << std::endl ;
    *out << "<button type=\"submit\" class=\"btn btn-info\" name=\"crateButton\" value=\""
         << m_crateToShow << "\">" << m_crateToShow<< "</button>" << std::endl;
    *out << cgicc::form() << std::endl ;
    *out << cgicc::td();
    *out << cgicc::td();
    *out << cgicc::form().set("method", "POST").set("action", methodExpandGLIB) << std::endl ;
    *out << "<button type=\"submit\" class=\"btn btn-info\" name=\"glibButton\" value=\""
         << m_glibToShow << "\">" << m_glibToShow<< "</button>" << std::endl;
    *out << cgicc::form() << std::endl ;
    *out << cgicc::td();
    *out << cgicc::td();
    *out << cgicc::form().set("method", "POST").set("action", methodExpandOH) << std::endl ;
    *out << "<button type=\"submit\" class=\"btn btn-info\" name=\"ohButton\" value=\""
         << m_ohToShow << "\">" << m_ohToShow<< "</button>" << std::endl;
    *out << cgicc::form() << std::endl ;
    *out << cgicc::td();
    *out << "</tr>" << std::endl;
    *out << cgicc::table() <<std::endl;;
    *out << "<div class=\"panel panel-primary\">" << std::endl;
    *out << "<div class=\"panel-heading\">" << std::endl;
    *out << "<h1><div align=\"center\">Chip Id : "<< m_vfatToShow << "</div></h1>" << std::endl;
    *out << "</div>" << std::endl;
    *out << "<div class=\"panel-body\">" << std::endl;
    *out << "<h3><div class=\"alert alert-info\" role=\"alert\" align=\"center\">Device base node : "
         << m_crateToShow << "::" << m_glibToShow << "::" << m_ohToShow <<  "</div></h3>" << std::endl;
    std::map <std::string, std::string> vfatProperties_;
    vfatProperties_ = m_gemHwMonitorVFAT.at(m_indexVFAT)->getDevice()->getDeviceProperties();
    *out << cgicc::table().set("class", "table");
    *out << cgicc::tr()<< std::endl;
    *out << cgicc::td();
    *out << cgicc::h3("Parameter");
    *out << cgicc::td()<< std::endl;
    *out << cgicc::td();
    *out << cgicc::h3("XML value");
    *out << cgicc::td()<< std::endl;
    *out << cgicc::td();
    *out << cgicc::h3("Hardware value");
    *out << cgicc::td()<< std::endl;
    *out << cgicc::tr() << std::endl;

    printVFAThwParameters("CalMode",
                          (vfatProperties_.find("CalMode")->second).c_str(),
                          (gem::hw::vfat::CalibrationModeToString.at(p_vfatDevice->getVFAT2Params().calibMode)).c_str(), out);
    printVFAThwParameters("CalPolarity",
                          (vfatProperties_.find("CalPolarity")->second).c_str(),
                          (gem::hw::vfat::CalPolarityToString.at(p_vfatDevice->getVFAT2Params().calPol)).c_str(), out);
    printVFAThwParameters("MSPolarity",
                          (vfatProperties_.find("MSPolarity")->second).c_str(),
                          (gem::hw::vfat::MSPolarityToString.at(p_vfatDevice->getVFAT2Params().msPol)).c_str(), out);
    printVFAThwParameters("TriggerMode",
                          (vfatProperties_.find("TriggerMode")->second).c_str(),
                          (gem::hw::vfat::TriggerModeToString.at(p_vfatDevice->getVFAT2Params().trigMode)).c_str(), out);
    printVFAThwParameters("RunMode",
                          (vfatProperties_.find("RunMode")->second).c_str(),
                          (gem::hw::vfat::RunModeToString.at(p_vfatDevice->getVFAT2Params().runMode)).c_str(), out);
    printVFAThwParameters("ReHitCT",
                          (vfatProperties_.find("ReHitCT")->second).c_str(),
                          (gem::hw::vfat::ReHitCTToString.at(p_vfatDevice->getVFAT2Params().reHitCT)).c_str(), out);
    printVFAThwParameters("LVDSPowerSave",
                          (vfatProperties_.find("LVDSPowerSave")->second).c_str(),
                          (gem::hw::vfat::LVDSPowerSaveToString.at(p_vfatDevice->getVFAT2Params().lvdsMode)).c_str(), out);
    printVFAThwParameters("DACMode",
                          (vfatProperties_.find("DACMode")->second).c_str(),
                          (gem::hw::vfat::DACModeToString.at(p_vfatDevice->getVFAT2Params().dacMode)).c_str(), out);
    printVFAThwParameters("DigInSel",
                          (vfatProperties_.find("DigInSel")->second).c_str(),
                          (gem::hw::vfat::DigInSelToString.at(p_vfatDevice->getVFAT2Params().digInSel)).c_str(), out);
    printVFAThwParameters("MSPulseLength",
                          (vfatProperties_.find("MSPulseLength")->second).c_str(),
                          (gem::hw::vfat::MSPulseLengthToString.at(p_vfatDevice->getVFAT2Params().msPulseLen)).c_str(), out);
    printVFAThwParameters("HitCountMode",
                          (vfatProperties_.find("HitCountMode")->second).c_str(),
                          (gem::hw::vfat::HitCountModeToString.at(p_vfatDevice->getVFAT2Params().hitCountMode)).c_str(), out);
    printVFAThwParameters("PbBG",
                          (vfatProperties_.find("PbBG")->second).c_str(),
                          (gem::hw::vfat::PbBGToString.at(p_vfatDevice->getVFAT2Params().padBandGap)).c_str(), out);
    printVFAThwParameters("TrimDACRange",
                          (vfatProperties_.find("TrimDACRange")->second).c_str(),
                          (gem::hw::vfat::TrimDACRangeToString.at(p_vfatDevice->getVFAT2Params().trimDACRange)).c_str(), out);
    printVFAThwParameters("IPreampIn",
                          (vfatProperties_.find("IPreampIn")->second).c_str(),
                          p_vfatDevice->getVFAT2Params().iPreampIn, out);
    printVFAThwParameters("IPreampFeed",
                          (vfatProperties_.find("IPreampFeed")->second).c_str(),
                          p_vfatDevice->getVFAT2Params().iPreampFeed, out);
    printVFAThwParameters("IPreampOut",
                          (vfatProperties_.find("IPreampOut")->second).c_str(),
                          p_vfatDevice->getVFAT2Params().iPreampOut, out);
    printVFAThwParameters("IShaper",
                          (vfatProperties_.find("IShaper")->second).c_str(),
                          p_vfatDevice->getVFAT2Params().iShaper, out);
    printVFAThwParameters("IShaperFeed",
                          (vfatProperties_.find("IShaperFeed")->second).c_str(),
                          p_vfatDevice->getVFAT2Params().iShaperFeed, out);
    printVFAThwParameters("IComp",
                          (vfatProperties_.find("IComp")->second).c_str(),
                          p_vfatDevice->getVFAT2Params().iComp, out);
    printVFAThwParameters("Latency",
                          (vfatProperties_.find("Latency")->second).c_str(),
                          p_vfatDevice->getVFAT2Params().latency, out);
    printVFAThwParameters("VCal",
                          (vfatProperties_.find("VCal")->second).c_str(),
                          p_vfatDevice->getVFAT2Params().vCal, out);
    printVFAThwParameters("VThreshold1",
                          (vfatProperties_.find("VThreshold1")->second).c_str(),
                          p_vfatDevice->getVFAT2Params().vThresh1, out);
    printVFAThwParameters("VThreshold2",
                          (vfatProperties_.find("VThreshold2")->second).c_str(),
                          p_vfatDevice->getVFAT2Params().vThresh2, out);
    printVFAThwParameters("CalPhase",
                          (vfatProperties_.find("CalPhase")->second).c_str(),
                          (p_vfatDevice->getVFAT2Params().calPhase), out);
    /*
    printVFAThwParameters("DFTest", (vfatProperties_.find("DFTest")->second).c_str(),
                          (gem::hw::vfat::DFTestPatternToString.at(p_vfatDevice->getVFAT2Params().sendTestPattern)).c_str(), out);
    printVFAThwParameters("ProbeMode", (vfatProperties_.find("ProbeMode")->second).c_str(),
                          (gem::hw::vfat::ProbeModeToString.at(p_vfatDevice->getVFAT2Params().probeMode)).c_str(), out);
    */
    // *out << cgicc::tr();
    *out << cgicc::table();
    *out << cgicc::br() << std::endl;

    // *out << "<table class=\"table\" >" << std::endl;
    // *out << "<tr>" << std::endl;
    // *out << "<th><h2><div align=\"center\">VFAT Channel Status </div></h2></th>" << std::endl;
    // *out << "</tr>" << std::endl;

    *out << "<div class=\"panel panel-info\">" << std::endl;
    *out << "<div class=\"panel-heading\">" << std::endl;
    *out << "<h2><div align=\"center\">VFAT Channel Status</div></h2>" << std::endl;
    *out << "<h4><div align=\"center\">" << "Trigger Mode: "
         << (gem::hw::vfat::TriggerModeToString.at(p_vfatDevice->getVFAT2Params().trigMode)).c_str()
         << "</div></h4>" << std::endl;
    *out << "<h4><div align=\"center\">" << "Hit Count: " <<  static_cast<int>(p_vfatDevice->getVFAT2Params().hitCounter)
         << " (" << (gem::hw::vfat::HitCountModeToString.at(p_vfatDevice->getVFAT2Params().hitCountMode)).c_str() << ")";
    *out << "</div></h4>" << std::endl;


    *out << std::endl;
    *out << "</div>" << std::endl;

    *out << "<div class=\"panel-body\">" << std::endl;
    // *out << "<div align=\"center\">" << "Trigger Mode: " <<  p_vfatDevice->getVFAT2Params().trigMode << "</div>" << std::endl;
    // *out << std::endl;
    if (p_vfatDevice->getVFAT2Params().trigMode == 0) {
      *out << "<div align=\"center\"><h4><font color=\"red\">VFAT is not in trigger mode, channels inactive</font></h4></div>" << std::endl;
      *out << std::endl;
    }

    // if (p_vfatDevice->getVFAT2Params().trigMode == 3) {

    *out << "<table class=\"table\" >" << std::endl;
    *out << "<tr>" << std::endl;
    for (int h = 1; h < 9; ++h) {
      *out << "<th>" << std::endl;
      *out << "<h4><div align=\"center\">Sector " << static_cast<int>(h) << "</div></h4>" << std::endl;
      *out << "</th>" << std::endl;
    }
    *out << "</tr>" << std::endl;
    *out << "<tr>" << std::endl;
    for (int i = 0; i < 8; ++i) {
      *out << "<td>" << std::endl;
      *out << "<table class=\"table\" >" << std::endl;

      for (int j = 4; j < 103; j += 24) {
        for (int k = 0; k < 3; ++k) {
          unsigned chann = 3*i + j + k;
          std::string btn_color;
          if (p_vfatDevice->getVFAT2Params().channels[chann-1].mask == 0 &&
              p_vfatDevice->getVFAT2Params().trigMode != 0) btn_color = "success";
          else if (p_vfatDevice->getVFAT2Params().trigMode == 0) btn_color = "warning";
          if (p_vfatDevice->getVFAT2Params().channels[chann-1].mask == 1) btn_color = "default";

          *out << "<tr>" << std::endl;
          *out << "<td>" << std::endl;
          *out << "<div align=\"center\">";
          *out << "<div class=\"btn-group\">" << std::endl;
          *out << "<button type=\"button\" class=\"btn btn-" << btn_color
               <<  " dropdown-toggle\" data-toggle=\"dropdown\" aria-haspopup=\"true\" aria-expanded=\"false\">";
          *out << std::setfill ('0') << std::setw (3) << chann
               << "<span class=\"caret\"></button>" << std::endl;
          *out << "<ul class=\"dropdown-menu\">" << std::endl;
          *out << "<li><a href=\"#\">" << "CalPulse: "
               << static_cast<int>(p_vfatDevice->getVFAT2Params().channels[chann-1].calPulse) << "</a></li>" << std::endl;
          *out << "<li><a href=\"#\">" << "Mask: "
               << static_cast<int>(p_vfatDevice->getVFAT2Params().channels[chann-1].mask) << "</a></li>" << std::endl;
          *out << "<li><a href=\"#\">" << "Trim DAC: "
               << static_cast<int>(p_vfatDevice->getVFAT2Params().channels[chann-1].trimDAC) << "</a></li>" << std::endl;
          *out << "</ul>" <<std::endl;
          *out << "</div>" << std::endl;
          *out << "</div>" << std::endl;
          *out << "</td>" << std::endl;
          *out << "</tr>" << std::endl;
        }
      }
      *out << "</table>" << std::endl;
      *out << "</td>" << std::endl;
    }
    *out << "</tr>" << std::endl;
    *out << "</table>" << std::endl;
    *out << "</div>" << std::endl;
    *out << "</div>" << std::endl;

    *out << cgicc::br()<< std::endl;
    *out << cgicc::hr()<< std::endl;
  }
}

void gem::hwMonitor::gemHwMonitorWeb::printVFAThwParameters(const char* key, const char* value1, const char* value2, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  std::string alertColor;
  (boost::iequals(value1, value2)) ? alertColor = "success" : alertColor = "danger";
  std::cout << m_vfatToShow << " status : " << m_gemHwMonitorVFAT.at(m_indexVFAT)->getDeviceStatus() << std::endl;
  if (!(m_gemHwMonitorVFAT.at(m_indexVFAT)->getDeviceStatus())) {
    std::cout << m_vfatToShow << " status : " << m_gemHwMonitorVFAT.at(m_indexVFAT)->getDeviceStatus() << std::endl;
    (boost::iequals(value1, value2)) ? m_gemHwMonitorVFAT.at(m_indexVFAT)->setDeviceStatus(0):m_gemHwMonitorVFAT.at(m_indexVFAT)->setDeviceStatus(1);
    m_gemHwMonitorOH.at(m_indexOH)->setSubDeviceStatus(1, m_indexVFAT%24);
    m_gemHwMonitorGLIB.at(m_indexGLIB)->setSubDeviceStatus(1, m_indexOH);
    m_gemHwMonitorCrate.at(m_indexCrate)->setSubDeviceStatus(1, m_indexGLIB);
  }
  *out << "<tr class=\"" << alertColor << "\">" << std::endl;
  *out << "<td>";
  *out << "<strong>" << key << ":" << "</strong>";
  *out << "</td>" << std::endl;
  *out << "<td>";
  *out << value1;
  *out << "</td>" << std::endl;
  *out << "<td>";
  *out << value2;
  *out << "</td>" << std::endl;
  *out << "</tr>" << std::endl;
}

void gem::hwMonitor::gemHwMonitorWeb::printVFAThwParameters(const char* key, const char* value, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  *out << "<tr>";
  *out << "<td>";
  *out << key << ":" << std::endl;
  *out << "</td>";
  *out << "<td>";
  *out << value << std::endl;
  *out << "</td>";
  *out << "</tr>";
}

void gem::hwMonitor::gemHwMonitorWeb::printVFAThwParameters(const char* key, const char* value1, uint8_t value2, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  std::stringstream ss;
  ss << std::dec << (unsigned) value2;
  std::string value_string = "";
  value_string.append(ss.str());
  printVFAThwParameters(key, value1, value_string.c_str(), out);
}

void gem::hwMonitor::gemHwMonitorWeb::printVFAThwParameters(const char* key, uint8_t value, xgi::Output* out)
  throw (xgi::exception::Exception)
{
  std::stringstream ss;
  ss << std::dec << (unsigned) value;
  std::string value_string = "";
  value_string.append(ss.str());
  printVFAThwParameters(key, value_string.c_str(), out);
}
