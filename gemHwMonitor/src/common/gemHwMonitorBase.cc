#include "gem/hwMonitor/gemHwMonitorBase.h"

//XDAQ_INSTANTIATOR_IMPL(gem::hwMonitor::gemHwMonitorBase)

//gem::hwMonitor::gemHwMonitorBase::gemHwMonitorBase(xdaq::ApplicationStub * s)
gem::hwMonitor::gemHwMonitorBase::gemHwMonitorBase()
    throw (xdaq::exception::Exception)
{
    std::string defaulXMLcfgFile = std::getenv("BUILD_HOME")+"gemdaq-testing/gembase/xml/gem_conf.xml";
    this->setXMLconfigFile(defaulXMLcfgFile);
    this->setDeviceId("GEM");
    this->setDeviceStatus(2);
}

void gem::hwMonitor::gemHwMonitorBase::initParser()
    throw (xgi::exception::Exception)
{
    gemXMLparser_ = new gem::base::utils::gemXMLparser(xmlConfigFileName_);
}

void gem::hwMonitor::gemHwMonitorBase::getDeviceConfiguration()
    throw (xgi::exception::Exception)
{
    gemXMLparser_->parseXMLFile();
    subDevicesRefs_ = gemXMLparser_->getCrateRefs();
}
std::string gem::hwMonitor::gemHwMonitorBase::getCurrentSubdeviceId(unsigned int subDeviceNumber)
    throw (xgi::exception::Exception)
{
    return subDevicesRefs_.at(subDeviceNumber)->getDeviceId();
}
