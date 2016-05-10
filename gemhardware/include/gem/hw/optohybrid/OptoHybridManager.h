#ifndef GEM_HW_OPTOHYBRID_OPTOHYBRIDMANAGER_H
#define GEM_HW_OPTOHYBRID_OPTOHYBRIDMANAGER_H
/** @file OptoHybridManager.h */


//#include "uhal/uhal.hpp"

#include "gem/base/GEMFSMApplication.h"
//#include "gem/hw/optohybrid/OptoHybridSettings.h"

#include "gem/hw/optohybrid/exception/Exception.h"

namespace gem {
  namespace hw {
    namespace optohybrid {

      class HwOptoHybrid;
      class OptoHybridManagerWeb;

      typedef std::shared_ptr<HwOptoHybrid> optohybrid_shared_ptr;

      class OptoHybridManager : public gem::base::GEMFSMApplication
        {

          friend class OptoHybridManagerWeb;

        public:
          XDAQ_INSTANTIATOR();

          OptoHybridManager(xdaq::ApplicationStub * s);

          virtual ~OptoHybridManager();

        protected:
          virtual void init();

          virtual void actionPerformed(xdata::Event& event);

          //state transitions
          virtual void initializeAction() throw (gem::hw::optohybrid::exception::Exception);
          virtual void configureAction()  throw (gem::hw::optohybrid::exception::Exception);
          virtual void startAction()      throw (gem::hw::optohybrid::exception::Exception);
          virtual void pauseAction()      throw (gem::hw::optohybrid::exception::Exception);
          virtual void resumeAction()     throw (gem::hw::optohybrid::exception::Exception);
          virtual void stopAction()       throw (gem::hw::optohybrid::exception::Exception);
          virtual void haltAction()       throw (gem::hw::optohybrid::exception::Exception);
          virtual void resetAction()      throw (gem::hw::optohybrid::exception::Exception);
          //virtual void noAction()         throw (gem::hw::optohybrid::exception::Exception);

          virtual void failAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);

          virtual void resetAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);

        private:
	  uint32_t parseVFATMaskList(std::string const&);
	  bool     isValidSlotNumber(std::string const&);

          //std::vector<uint32_t> v_vfatBroadcastMask;// one for each optohybrid

          class OptoHybridInfo {

          public:
            OptoHybridInfo();
            void registerFields(xdata::Bag<OptoHybridManager::OptoHybridInfo>* bag);
            //monitoring information
            xdata::Boolean present;
            xdata::Integer crateID;
            xdata::Integer slotID;
            xdata::Integer linkID;

            //configuration parameters
            xdata::String controlHubAddress;
            xdata::String deviceIPAddress;
            xdata::String ipBusProtocol;
            xdata::String addressTable;

            xdata::UnsignedInteger32 controlHubPort;
            xdata::UnsignedInteger32 ipBusPort;

            xdata::String            vfatBroadcastList;
            xdata::UnsignedInteger32 vfatBroadcastMask;

            //registers to set
            xdata::Integer triggerSource;
            xdata::Integer sbitSource;
            xdata::Integer refClkSrc;
            xdata::Integer vfatClkSrc;
            xdata::Integer cdceClkSrc;

            inline std::string toString() {
              // write obj to stream
              std::stringstream os;
              os << "present:" << present.toString() << std::endl
                 << "crateID:" << crateID.toString() << std::endl
                 << "slotID:"  << slotID.toString()  << std::endl
                 << "linkID:"  << linkID.toString()  << std::endl

                 << "controlHubAddress:" << controlHubAddress.toString() << std::endl
                 << "deviceIPAddress:"   << deviceIPAddress.toString()   << std::endl
                 << "ipBusProtocol:"     << ipBusProtocol.toString()     << std::endl
                 << "addressTable:"      << addressTable.toString()      << std::endl
                 << "controlHubPort:"    << controlHubPort.value_    << std::endl
                 << "ipBusPort:"         << ipBusPort.value_         << std::endl

                 << "vfatBroadcastList:"   << vfatBroadcastList.toString() << std::endl
                 << "vfatBroadcastMask:0x" << std::hex << vfatBroadcastMask.value_ << std::dec << std::endl

                 << "triggerSource:0x" << std::hex << triggerSource.value_ << std::dec << std::endl
                 << "sbitSource:0x"    << std::hex << sbitSource.value_    << std::dec << std::endl
                 << "refClkSrc:0x"     << std::hex << refClkSrc.value_     << std::dec << std::endl
                 << "vfatClkSrc:0x"    << std::hex << vfatClkSrc.value_    << std::dec << std::endl
                 << "cdceClkSrc:0x"    << std::hex << cdceClkSrc.value_    << std::dec << std::endl
                 << std::endl;
              return os.str();
            };
          };

          mutable gem::utils::Lock m_deviceLock;//[MAX_OPTOHYBRIDS_PER_AMC*MAX_AMCS_PER_CRATE];

          optohybrid_shared_ptr m_optohybrids[MAX_OPTOHYBRIDS_PER_AMC*MAX_AMCS_PER_CRATE];
          xdata::InfoSpace*     is_optohybrids[MAX_OPTOHYBRIDS_PER_AMC*MAX_AMCS_PER_CRATE];
          xdata::Vector<xdata::Bag<OptoHybridInfo> > m_optohybridInfo;
          xdata::String        m_connectionFile;
        };  // class OptoHybridManager

    }  // namespace gem::hw::optohybrid
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_OPTOHYBRID_OPTOHYBRIDMANAGER_H
