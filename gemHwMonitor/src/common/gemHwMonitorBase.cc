#include "gem/hwMonitor/gemHwMonitorBase.h"

//XDAQ_INSTANTIATOR_IMPL(gem::hwMonitor::gemHwMonitorBase)

//gem::hwMonitor::gemHwMonitorBase::gemHwMonitorBase(xdaq::ApplicationStub * s)
gem::hwMonitor::gemHwMonitorBase::gemHwMonitorBase()
    throw (xdaq::exception::Exception)
{
    this->setXMLconfigFile("/home/mdalchen/test/gem-daq-code/gemdaq-testing/gemHwMonitor/xml/gemHwMonitor.xml");
    this->setCurrentCrate("N/A");
}
