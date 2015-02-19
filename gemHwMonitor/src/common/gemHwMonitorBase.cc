#include "gem/hwMonitor/gemHwMonitorBase.h"

//XDAQ_INSTANTIATOR_IMPL(gem::hwMonitor::gemHwMonitorBase)

//gem::hwMonitor::gemHwMonitorBase::gemHwMonitorBase(xdaq::ApplicationStub * s)
gem::hwMonitor::gemHwMonitorBase::gemHwMonitorBase()
    throw (xdaq::exception::Exception)
{
    this->setXMLconfigFile("/home/mdalchen/private/gem-daq-code/gemdaq-testing/gembase/xml/gem_conf.xml");
    this->setCurrentCrate("N/A");
}

void gem::hwMonitor::gemHwMonitorBase::initParser()
    throw (xgi::exception::Exception)
{
    gemXMLparser_ = new gem::base::utils::gemXMLparser(xmlConfigFileName_);
}

void gem::hwMonitor::gemHwMonitorBase::getSystemConfiguration()
    throw (xgi::exception::Exception)
{
    gemXMLparser_->parseXMLFile();
    crateRefs_ = gemXMLparser_->getCrateRefs();
}
int gem::hwMonitor::gemHwMonitorBase::getNumberOfCrates()
    throw (xgi::exception::Exception)
{ 
    return crateRefs_.size();
    //return 0;
}
std::string gem::hwMonitor::gemHwMonitorBase::getCurrentCrateId(unsigned int crateNumber)
    throw (xgi::exception::Exception)
{
    return crateRefs_.at(crateNumber)->getDeviceId();
}
int gem::hwMonitor::gemHwMonitorBase::getCurrentCrateNumberOfGLIBs(std::string& crateID)
    throw (xgi::exception::Exception)
{
    int crateNumber=0;
    for (unsigned int i = 0; i<crateRefs_.size(); i++)
        {
            if (crateID==crateRefs_.at(i)->getDeviceId()) {
                crateNumber = i;
                break;
            }
        }
    glibRefs_ = crateRefs_.at(crateNumber)->getGLIBRefs(); 
    return glibRefs_.size();
}
