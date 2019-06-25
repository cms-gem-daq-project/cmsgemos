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

      using optohybrid_shared_ptr = std::shared_ptr<HwOptoHybrid>;
      using is_toolbox_ptr        = std::shared_ptr<gem::base::utils::GEMInfoSpaceToolBox>;

      class OptoHybridManager : public gem::base::GEMFSMApplication
        {

          friend class OptoHybridManagerWeb;

        public:
          XDAQ_INSTANTIATOR();

          OptoHybridManager(xdaq::ApplicationStub * s);

          virtual ~OptoHybridManager();

        protected:
          /* virtual void init() override; */
          void init();

          virtual void actionPerformed(xdata::Event& event) override;

          // state transitions
          virtual void initializeAction() throw (gem::hw::optohybrid::exception::Exception) override;
          virtual void configureAction()  throw (gem::hw::optohybrid::exception::Exception) override;
          virtual void startAction()      throw (gem::hw::optohybrid::exception::Exception) override;
          virtual void pauseAction()      throw (gem::hw::optohybrid::exception::Exception) override;
          virtual void resumeAction()     throw (gem::hw::optohybrid::exception::Exception) override;
          virtual void stopAction()       throw (gem::hw::optohybrid::exception::Exception) override;
          virtual void haltAction()       throw (gem::hw::optohybrid::exception::Exception) override;
          virtual void resetAction()      throw (gem::hw::optohybrid::exception::Exception) override;
          // virtual void noAction()         throw (gem::hw::optohybrid::exception::Exception) override;

          void failAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);

          virtual void resetAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception) override;

        protected:
          class OptoHybridInfo
          {
          public:
            OptoHybridInfo();
            void registerFields(xdata::Bag<OptoHybridManager::OptoHybridInfo>* bag);

            xdata::Boolean present;  ///< FIXME BAD USAGE
            xdata::Integer crateID;  ///< Specifies the crate to which the OptoHybrid is connected
            xdata::Integer slotID;   ///< Specifies the AMC slot to which the OptoHybrid is connected
            xdata::Integer linkID;   ///< Specifies the AMC link to which the OptoHybrid is connected

            inline std::string toString() {
              std::stringstream os;
              os << "present:" << present.toString() << std::endl
                 << "crateID:" << crateID.toString() << std::endl
                 << "slotID:"  << slotID.toString()  << std::endl
                 << "linkID:"  << linkID.toString()  << std::endl
                 << std::endl;
              return os.str();
            };
          };

        private:

          void createOptoHybridInfoSpaceItems(is_toolbox_ptr is_optohybrid, optohybrid_shared_ptr optohybrid);

          mutable gem::utils::Lock m_deviceLock;  ///< [MAX_OPTOHYBRIDS_PER_AMC*MAX_AMCS_PER_CRATE];

          /**
           * @brief Template container that contains a matrix of elements of some type
           * @tparam T the type of object in the collection
           * @tparam N First dimension of the matrix
           * @tparam M Second dimension of the matrix
           */
          template<class T, size_t N, size_t M>
            using HWMapMatrix = std::array<std::array<T, N>, M>;

          HWMapMatrix<optohybrid_shared_ptr, MAX_OPTOHYBRIDS_PER_AMC, MAX_AMCS_PER_CRATE>
            m_optohybrids;  ///< HwOptoHybrid pointers to be managed

          HWMapMatrix<std::shared_ptr<OptoHybridMonitor>, MAX_OPTOHYBRIDS_PER_AMC, MAX_AMCS_PER_CRATE>
            m_optohybridMonitors;  ///< OptoHybridMonitor pointers to be managed

          HWMapMatrix<is_toolbox_ptr, MAX_OPTOHYBRIDS_PER_AMC, MAX_AMCS_PER_CRATE>
            is_optohybrids;  ///< OptoHybrid InfoSpace pointers to be managed

          xdata::Vector<xdata::Bag<OptoHybridInfo> > m_optohybridInfo;  ///< 
          xdata::String m_connectionFile;  ///< 

          HWMapMatrix<uint32_t, MAX_OPTOHYBRIDS_PER_AMC, MAX_AMCS_PER_CRATE>
            m_trackingMask;   ///< VFAT slots to ignore I2C and tracking data
          HWMapMatrix<uint32_t, MAX_OPTOHYBRIDS_PER_AMC, MAX_AMCS_PER_CRATE>
            m_broadcastList;  ///< VFAT slots to block slow control broadcasts
          HWMapMatrix<uint32_t, MAX_OPTOHYBRIDS_PER_AMC, MAX_AMCS_PER_CRATE>
            m_sbitMask;       ///< VFAT slots to block trigger data

          HWMapMatrix<std::vector<std::pair<uint8_t, uint32_t> >, MAX_OPTOHYBRIDS_PER_AMC, MAX_AMCS_PER_CRATE>
            m_vfatMapping;  ///< VFAT mapping 

	  uint32_t m_lastLatency;         ///< Special variable for latency scan mode
          uint32_t m_lastVT1, m_lastVT2;  ///< Special variable for threshold scan mode 

          std::map<int,std::set<int> > m_hwMapping;        ///< FIXME UNUSED
          std::map<std::string, uint8_t > m_vfatSettings;  ///< FIXME OBSOLETE V3
        };  // class OptoHybridManager

    }  // namespace gem::hw::optohybrid
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_OPTOHYBRID_OPTOHYBRIDMANAGER_H
