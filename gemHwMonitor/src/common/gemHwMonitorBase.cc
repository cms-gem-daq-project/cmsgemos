#include "gem/hwMonitor/gemHwMonitorBase.h"

template <class T> const std::string& gem::hwMonitor::gemHwMonitorBase<T>::getDeviceId ()
    throw (xgi::exception::Exception)
{
    if (isConfigured_){
        return gemDevice_->getDeviceId();
    } else {
        return "Device is not configured";
    }
}

template <class T> void gem::hwMonitor::gemHwMonitorBase<T>::getDeviceConfiguration(T* device)
    throw (xgi::exception::Exception)
{
    gemDevice_ = device;
    isConfigured_ = true;
}

template <class T> int gem::hwMonitor::gemHwMonitorBase<T>::getNumberOfSubDevices()
    throw (xgi::exception::Exception)
{
    if (isConfigured_ && !(this->getDeviceId().find('VFAT') == std::string::npos)){
        return gemDevice_->getSubDevicesIds().size();
    } else {
        return -1;
    }
}

template <class T> std::string gem::hwMonitor::gemHwMonitorBase<T>::getCurrentSubDeviceId(unsigned int subDeviceNumber)
    throw (xgi::exception::Exception)
{
    if !(this->getDeviceId().find('VFAT') == std::string::npos) 
    {
        return gemDevice_->getSubDevicesRefs().at(subDeviceNumber)->getDeviceId();
    }
    else {
        return "VFATs don't have subdevices";
    }
}
template class gem::hwMonitor::gemHwMonitorBase<gem::base::utils::gemSystemProperties>; 
template class gem::hwMonitor::gemHwMonitorBase<gem::base::utils::gemCrateProperties>; 
