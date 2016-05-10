#ifndef GEM_HW_VFAT_VFATMANAGER_H
#define GEM_HW_VFAT_VFATMANAGER_H

//#include "uhal/uhal.hpp"

#include "gem/base/GEMFSMApplication.h"
//#include "gem/hw/vfat/VFATSettings.h"

#include "gem/hw/vfat/exception/Exception.h"

namespace gem {
  namespace hw {
    namespace vfat {

      class HwVFAT2;
      class VFATManagerWeb;

      typedef std::shared_ptr<HwVFAT2>  vfat_shared_ptr;

      class VFATManager : public gem::base::GEMFSMApplication
        {

          friend class VFATManagerWeb;

        public:
          XDAQ_INSTANTIATOR();

          VFATManager(xdaq::ApplicationStub* s);

          virtual ~VFATManager();

        protected:
          virtual void init();

          virtual void actionPerformed(xdata::Event& event);

          //state transitions
          virtual void initializeAction() throw (gem::hw::vfat::exception::Exception);
          virtual void configureAction()  throw (gem::hw::vfat::exception::Exception);
          virtual void startAction()      throw (gem::hw::vfat::exception::Exception);
          virtual void pauseAction()      throw (gem::hw::vfat::exception::Exception);
          virtual void resumeAction()     throw (gem::hw::vfat::exception::Exception);
          virtual void stopAction()       throw (gem::hw::vfat::exception::Exception);
          virtual void haltAction()       throw (gem::hw::vfat::exception::Exception);
          virtual void resetAction()      throw (gem::hw::vfat::exception::Exception);
          //virtual void noAction()         throw (gem::hw::vfat::exception::Exception);

          virtual void failAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);

          virtual void resetAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);

        private:
	  uint16_t parseAMCEnableList(std::string const&);
	  bool     isValidSlotNumber( std::string const&);

          uint16_t m_amcEnableMask;

          class VFATInfo {

          public:
            VFATInfo();
            void registerFields(xdata::Bag<VFATManager::VFATInfo>* bag);
            //monitoring information
            xdata::Boolean present;
            xdata::Integer crateID;
            xdata::Integer slotID;

            //configuration parameters
            xdata::String controlHubAddress;
            xdata::String deviceIPAddress;
            xdata::String ipBusProtocol;
            xdata::String addressTable;

            xdata::UnsignedInteger32 controlHubPort;
            xdata::UnsignedInteger32 ipBusPort;

            //registers to set
            xdata::Integer triggerSource;
            xdata::Integer sbitSource;

            inline std::string toString() {
              // write obj to stream
              std::stringstream os;
              os << "present:" << present.toString() << std::endl
                 << "crateID:" << crateID.toString() << std::endl
                 << "slotID:"  << slotID.toString()  << std::endl

                 << "controlHubAddress:" << controlHubAddress.toString() << std::endl
                 << "deviceIPAddress:"   << deviceIPAddress.toString()     << std::endl
                 << "ipBusProtocol:"     << ipBusProtocol.toString()       << std::endl
                 << "addressTable:"      << addressTable.toString()        << std::endl
                 << "controlHubPort:"    << controlHubPort.toString()      << std::endl
                 << "ipBusPort:"         << ipBusPort.toString()           << std::endl
                 << "triggerSource:0x"   << triggerSource.toString()       << std::endl
                 << "sbitSource:0x"      << sbitSource.toString()          << std::endl
                 << std::endl;
              return os.str();
            };
          };

          mutable gem::utils::Lock m_deviceLock;//[MAX_AMCS_PER_CRATE];

          std::shared_ptr<HwVFAT2> m_vfats[MAX_VFATS_PER_GEB*MAX_OPTOHYBRIDS_PER_AMC];
          xdata::InfoSpace*        is_vfats[MAX_VFATS_PER_GEB*MAX_OPTOHYBRIDS_PER_AMC];
          xdata::Vector<xdata::Bag<VFATInfo> > m_vfatInfo;//[MAX_AMCS_PER_CRATE];
          xdata::String        m_amcSlots;
          xdata::String        m_connectionFile;
        };  // class VFATManager

    }  // namespace gem::hw::vfat
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_VFAT_VFATMANAGER_H
