#include "gem/hwMonitor/gemHwMonitorBase.h"

template <class T> const std::string gem::hwMonitor::gemHwMonitorBase<T>::getDeviceId ()
{
  if (m_isConfigured) {
    return p_gemDevice->getDeviceId();
  } else {
    return "Device is not configured";
  }
}

template <class T> void gem::hwMonitor::gemHwMonitorBase<T>::setDeviceConfiguration(T& device)
{
  p_gemDevice = &device;
  m_isConfigured = true;
}

template <class T> int gem::hwMonitor::gemHwMonitorBase<T>::getNumberOfSubDevices()
{
  return p_gemDevice->getSubDevicesIds().size();
}

// [GCC bug] Bugreport https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56480
namespace gem {
  namespace hwMonitor {
    template <>
    int gem::hwMonitor::gemHwMonitorBase<gem::utils::gemVFATProperties>::getNumberOfSubDevices()
    {
      return -1;
    }
  }
}

template <class T> const std::string gem::hwMonitor::gemHwMonitorBase<T>::getCurrentSubDeviceId(unsigned int subDeviceNumber)
{
  return p_gemDevice->getSubDevicesRefs().at(subDeviceNumber)->getDeviceId();
}

// [GCC bug] Bugreport https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56480
namespace gem {
  namespace hwMonitor {
    template <>
    const std::string gemHwMonitorBase<gem::utils::gemVFATProperties>::getCurrentSubDeviceId(unsigned int subDeviceNumber)
    {
      return "VFATs don't have subdevices";
    }
  }
}
template class gem::hwMonitor::gemHwMonitorBase<gem::utils::gemSystemProperties>;
template class gem::hwMonitor::gemHwMonitorBase<gem::utils::gemCrateProperties>;
template class gem::hwMonitor::gemHwMonitorBase<gem::utils::gemGLIBProperties>;
template class gem::hwMonitor::gemHwMonitorBase<gem::utils::gemOHProperties>;
template class gem::hwMonitor::gemHwMonitorBase<gem::utils::gemVFATProperties>;
