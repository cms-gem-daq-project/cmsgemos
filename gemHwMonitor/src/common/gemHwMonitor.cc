#include "gem/hwMon/gemHWmonitor.h"

XDAQ_INSTANTIATOR_IMPL(gem::hwMon::gemHWmonitor)

gem::hwMon::gemHWmonitor::GEMController(xdaq::ApplicationStub * s)
    throw (xdaq::exception::Exception)
{
    device_ = "VFAT9";
}
