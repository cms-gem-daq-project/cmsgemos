#ifndef GEM_HW_AMC13_HWAMC13_H
#define GEM_HW_AMC13_HWAMC13_H

#include "xdaq/Application.h"

#include "xdata/String.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"
#include "xdata/ActionListener.h"

#include "gem/hw/GEMHwDevice.h"

//#include "gem/hw/vfat/AMC13Monitor.h"
#include "gem/hw/vfat/AMC13Settings.h"
#include "gem/hw/vfat/AMC13SettingsEnums.h"
#include "gem/hw/vfat/AMC13Enums2Strings.h"
#include "gem/hw/vfat/AMC13Strings2Enums.h"

#include "gem/hw/vfat/exception/Exception.h"

typedef uhal::exception::exception uhalException;

namespace uhal {
  class HwInterface;
}

namespace xdaq {
  class Application;;
}

namespace gem {
  namespace hw {
    namespace amc13 {

      class HwAMC13: public gem::hw::GEMHwDevice
        {
        public:
          //XDAQ_INSTANTIATOR();

          HwAMC13(xdaq::Application * amc13App,
                  std::string const& vfatDevice="CMS_hybrid_J8");
          // access by serial number?
          //HwAMC13(xdaq::Application * amc13App);
          //throw (xdaq::exception::Exception);

        protected:
          //uhal::ConnectionManager *manageAMC13Connection;
          //log4cplus::Logger logAMC13_;
          //uhal::HwInterface *hwAMC13_;

          gem::hw::amc13::AMC13ControlParams amc13Params_;

          //uhal::HwInterface& getVFA2Hw() const;

          //AMC13Monitor *monAMC13_;

        };  // class HwAMC13

    }  // namespace gem::hw::amc13
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_AMC13_HWAMC13_H
