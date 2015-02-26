#include "gem/hwMonitor/gemHwMonitorBase.h"

template <class T> gem::hwMonitor::gemHwMonitorBase<T>::gemHwMonitorBase()
    throw (xdaq::exception::Exception)
{
    std::string defaulXMLcfgFile = std::getenv("BUILD_HOME");
    defaulXMLcfgFile +="/gemdaq-testing/gembase/xml/gem_conf.xml";
    this->setXMLconfigFile(defaulXMLcfgFile.c_str());
    this->setDeviceStatus(2);
}

template <class T> gem::hwMonitor::gemHwMonitorBase<T>::~gemHwMonitorBase()
{
    delete gemXMLparser_;
}

template <class T> bool gem::hwMonitor::gemHwMonitorBase<T>::isConfigured()
    throw (xgi::exception::Exception)
{
    return isConfigured_;
}
template <class T> void gem::hwMonitor::gemHwMonitorBase<T>::setXMLconfigFile (const char* inputXMLfilename)
    throw (xgi::exception::Exception)
{
    xmlConfigFileName_ = inputXMLfilename;
    isConfigured_ = false;
}

template <class T> const std::string& gem::hwMonitor::gemHwMonitorBase<T>::getXMLconfigFile ()
    throw (xgi::exception::Exception)
{
    return xmlConfigFileName_;
}

template <class T> const std::string& gem::hwMonitor::gemHwMonitorBase<T>::getDeviceId ()
    throw (xgi::exception::Exception)
{
    if (isConfigured_){
        return gemDevice_->getDeviceId();
    } else {
        return "Device is not configured";
    }
}

template <class T> void gem::hwMonitor::gemHwMonitorBase<T>::getDeviceConfiguration()
    throw (xgi::exception::Exception)
{
    gemXMLparser_ = new gem::base::utils::gemXMLparser(xmlConfigFileName_);
    gemXMLparser_->parseXMLFile();
    gemDevice_ = gemXMLparser_->getGEMDevice();
    isConfigured_ = true;
}

template <class T> int gem::hwMonitor::gemHwMonitorBase<T>::getNumberOfSubDevices()
    throw (xgi::exception::Exception)
{
    if (isConfigured_){
        return gemDevice_->getSubDevicesIds().size();
    } else {
        return -1;
    }
}

template <class T> std::string gem::hwMonitor::gemHwMonitorBase<T>::getCurrentSubDeviceId(unsigned int subDeviceNumber)
    throw (xgi::exception::Exception)
{
    return gemDevice_->getSubDevicesRefs().at(subDeviceNumber)->getDeviceId();
}
template class gem::hwMonitor::gemHwMonitorBase<gem::base::utils::gemSystemProperties>; 
