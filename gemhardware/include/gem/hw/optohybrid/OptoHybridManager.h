/** @file OptoHybridManager.h */

#ifndef GEM_HW_OPTOHYBRID_OPTOHYBRIDMANAGER_H
#define GEM_HW_OPTOHYBRID_OPTOHYBRIDMANAGER_H

#include <array>
#include <set>

#include "gem/base/GEMFSMApplication.h"
// #include "gem/hw/optohybrid/OptoHybridSettings.h"

#include "gem/hw/optohybrid/exception/Exception.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"
#include "gem/utils/exception/Exception.h"


namespace gem {
  namespace hw {
    namespace optohybrid {

      class HwOptoHybrid;
      class OptoHybridManagerWeb;
      class OptoHybridMonitor;

      typedef std::shared_ptr<HwOptoHybrid> optohybrid_shared_ptr;
      typedef std::shared_ptr<gem::base::utils::GEMInfoSpaceToolBox> is_toolbox_ptr;

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

          // state transitions
          virtual void initializeAction() throw (gem::hw::optohybrid::exception::Exception);
          virtual void configureAction()  throw (gem::hw::optohybrid::exception::Exception);
          virtual void startAction()      throw (gem::hw::optohybrid::exception::Exception);
          virtual void pauseAction()      throw (gem::hw::optohybrid::exception::Exception);
          virtual void resumeAction()     throw (gem::hw::optohybrid::exception::Exception);
          virtual void stopAction()       throw (gem::hw::optohybrid::exception::Exception);
          virtual void haltAction()       throw (gem::hw::optohybrid::exception::Exception);
          virtual void resetAction()      throw (gem::hw::optohybrid::exception::Exception);
          // virtual void noAction()         throw (gem::hw::optohybrid::exception::Exception);

          virtual void failAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);

          virtual void resetAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);

        protected:
          class OptoHybridInfo
          {
          public:
            OptoHybridInfo();
            void registerFields(xdata::Bag<OptoHybridManager::OptoHybridInfo>* bag);
            // monitoring information
            xdata::Boolean present;
            xdata::Integer crateID;
            xdata::Integer slotID;
            xdata::Integer linkID;
            xdata::String  cardName; // FIXME OBSOLETE/UPDATE?

            /* // configuration parameters */
            /* xdata::String controlHubAddress; */
            /* xdata::String deviceIPAddress; */
            /* xdata::String ipBusProtocol; */
            /* xdata::String addressTable; */
            /* xdata::UnsignedInteger32 controlHubPort; */
            /* xdata::UnsignedInteger32 ipBusPort; */

            xdata::String            vfatBroadcastList; // FIXME OBSOLETE/UPDATE?
            xdata::UnsignedInteger32 vfatBroadcastMask; // FIXME OBSOLETE/UPDATE?

            xdata::String            vfatSBitList; // FIXME OBSOLETE/UPDATE?
            xdata::UnsignedInteger32 vfatSBitMask; // FIXME OBSOLETE/UPDATE?

            // registers to set
            xdata::Integer triggerSource;  // FIXME OBSOLETE/UPDATE?
            xdata::Integer refClkSrc;      // FIXME OBSOLETE/UPDATE?

            /* xdata::Bag<SBitConfig> sbitConfig; // USE DB CONFIG OBJECT */
            /* xdata::Bag<CommonVFATSettings> commonVFATSettings; // USE DB CONFIG OBJECT */

            inline std::string toString() {
              // write obj to stream
              std::stringstream os;
              os << "present:" << present.toString() << std::endl
                 << "crateID:" << crateID.toString() << std::endl
                 << "slotID:"  << slotID.toString()  << std::endl
                 << "linkID:"  << linkID.toString()  << std::endl
                 << "cardName:" << cardName.toString() << std::endl

                 /* << "controlHubAddress:" << controlHubAddress.toString() << std::endl */
                 /* << "deviceIPAddress:"   << deviceIPAddress.toString()   << std::endl */
                 /* << "ipBusProtocol:"     << ipBusProtocol.toString()     << std::endl */
                 /* << "addressTable:"      << addressTable.toString()      << std::endl */
                 /* << "controlHubPort:"    << controlHubPort.value_    << std::endl */
                 /* << "ipBusPort:"         << ipBusPort.value_         << std::endl */

                 << "vfatBroadcastList:"   << vfatBroadcastList.toString() << std::endl
                 << "vfatBroadcastMask:0x" << std::hex << vfatBroadcastMask.value_ << std::dec << std::endl

                 << "vfatSBitList:"   << vfatSBitList.toString() << std::endl
                 << "vfatSBitMask:0x" << std::hex << vfatSBitMask.value_ << std::dec << std::endl

                 << "triggerSource:0x" << std::hex << triggerSource.value_ << std::dec << std::endl
                 << "refClkSrc:0x"     << std::hex << refClkSrc.value_     << std::dec << std::endl
                 << std::endl;
              return os.str();
            };
          };

        private:

          void     createOptoHybridInfoSpaceItems(is_toolbox_ptr is_optohybrid, optohybrid_shared_ptr optohybrid);

          mutable gem::utils::Lock m_deviceLock;  // [MAX_OPTOHYBRIDS_PER_AMC*MAX_AMCS_PER_CRATE];

          // Matrix<optohybrid_shared_ptr, MAX_OPTOHYBRIDS_PER_AMC, MAX_AMCS_PER_CRATE>
          std::array<std::array<optohybrid_shared_ptr, MAX_OPTOHYBRIDS_PER_AMC>, MAX_AMCS_PER_CRATE>
            m_optohybrids;

          std::array<std::array<std::shared_ptr<OptoHybridMonitor>, MAX_OPTOHYBRIDS_PER_AMC>, MAX_AMCS_PER_CRATE>
            m_optohybridMonitors;

          std::array<std::array<is_toolbox_ptr, MAX_OPTOHYBRIDS_PER_AMC>, MAX_AMCS_PER_CRATE>
            is_optohybrids;

          xdata::Vector<xdata::Bag<OptoHybridInfo> > m_optohybridInfo;
          xdata::String        m_connectionFile;

          std::array<std::array<uint32_t, MAX_OPTOHYBRIDS_PER_AMC>, MAX_AMCS_PER_CRATE>
            m_trackingMask;   ///< VFAT slots to ignore tracking data
          std::array<std::array<uint32_t, MAX_OPTOHYBRIDS_PER_AMC>, MAX_AMCS_PER_CRATE>
            m_broadcastList;  ///< VFATs to receive I2C broadcasts
          std::array<std::array<uint32_t, MAX_OPTOHYBRIDS_PER_AMC>, MAX_AMCS_PER_CRATE>
            m_sbitMask;       ///< mask specific VFATs

          std::array<std::array<std::vector<std::pair<uint8_t, uint32_t> >, MAX_OPTOHYBRIDS_PER_AMC>, MAX_AMCS_PER_CRATE>
            m_vfatMapping;

	  uint32_t m_lastLatency, m_lastVT1, m_lastVT2;

          std::map<int,std::set<int> > m_hwMapping;
          std::map<std::string, uint8_t > m_vfatSettings;
        };  // class OptoHybridManager

    }  // namespace gem::hw::optohybrid
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_OPTOHYBRID_OPTOHYBRIDMANAGER_H
