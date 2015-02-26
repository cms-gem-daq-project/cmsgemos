#include "gem/hwMonitor/gemHwMonitorBase.h"

gem::hwMonitor::gemHwMonitorBase::gemHwMonitorBase()
    throw (xdaq::exception::Exception)
{
    std::string defaulXMLcfgFile = std::getenv("BUILD_HOME");
    defaulXMLcfgFile +="/gemdaq-testing/gembase/xml/gem_conf.xml";
    this->setXMLconfigFile(defaulXMLcfgFile);
    this->setDeviceStatus(2);
}

gem::hwMonitor::gemHwMonitorBase::~gemHwMonitorBase()
{
    delete gemXMLparser_;
}

bool gem::hwMonitor::gemHwMonitorBase::isConfigured()
    throw (xgi::exception::Exception);
{
    return isConfigured_;
}
void gem::hwMonitor::gemHwMonitorBase::setXMLconfigFile (const char* inputXMLfilename)
    throw (xgi::exception::Exception)
{
    xmlConfigFileName_ = inputXMLfilename;
    isConfigured_ = false;
}

const std::string& gem::hwMonitor::gemHwMonitorBase::getXMLconfigFile ()
    throw (xgi::exception::Exception)
{
    return xmlConfigFileName_;
}

const std::string& gem::hwMonitor::gemHwMonitorBase::getDeviceId ()
    throw (xgi::exception::Exception)
{
    if (isConfigured_){
        return gemSystem_.getDeviceId();
    } else {
        return "Device is not configured";
    }
}

void gem::hwMonitor::gemHwMonitorBase::getDeviceConfiguration()
    throw (xgi::exception::Exception)
{
    gemXMLparser_ = new gem::base::utils::gemXMLparser(xmlConfigFileName_);
    gemXMLparser_->parseXMLFile();
    gemSystem_ = gemXMLparser_->getGEMDevice();
    isConfigured = true;
}

int gem::hwMonitor::gemHwMonitorBase::getNumberOfSubDevices()
    throw (xgi::exception::Exception)
{
    if (isConfigured_){
        return gemSystem_.getSubDevicesIds().size();
    } else {
        return -1;
    }
}

std::string gem::hwMonitor::gemHwMonitorBase::getCurrentSubDeviceId(unsigned int subDeviceNumber)
    throw (xgi::exception::Exception)
{
    return gemSystem_.getSubDeviceRefs().at(subDeviceNumber)->getDeviceId();
}
