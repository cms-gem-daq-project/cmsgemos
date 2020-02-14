/** @file AMCManager.h */

#ifndef GEM_HW_AMC_AMCMANAGER_H
#define GEM_HW_AMC_AMCMANAGER_H

#include <array>

#include "gem/base/GEMFSMApplication.h"
// #include "gem/hw/amc/AMCSettings.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"

namespace gem {
  namespace hw {
    namespace glib {
      class HwGLIB;
    }
    namespace amc {

      class AMCManagerWeb;
      class AMCMonitor;

      using amc_shared_ptr = std::shared_ptr<gem::hw::glib::HwGLIB>;
      using is_toolbox_ptr  = std::shared_ptr<gem::base::utils::GEMInfoSpaceToolBox>;

      class AMCManager : public gem::base::GEMFSMApplication
        {

          friend class AMCManagerWeb;
          // friend class AMCMonitor;

        public:
          XDAQ_INSTANTIATOR();

          AMCManager(xdaq::ApplicationStub* s);

          virtual ~AMCManager();

        protected:
          /* virtual void init() override; */
          void init();

          virtual void actionPerformed(xdata::Event& event) override;

          // state transitions FIXME: remove exception specifiers
          virtual void initializeAction() override;
          virtual void configureAction()  override;
          virtual void startAction()      override;
          virtual void pauseAction()      override;
          virtual void resumeAction()     override;
          virtual void stopAction()       override;
          virtual void haltAction()       override;
          virtual void resetAction()      override;
          // virtual void noAction()         override;

          void failAction(toolbox::Event::Reference e);

          virtual void resetAction(toolbox::Event::Reference e) override;

        protected:
          /**
           * OBSOLETE not present in generic AMC FW
           */
          std::vector<uint32_t> dumpAMCFIFO(int const& amc);

          /**
           * OBSOLETE not present in generic AMC FW
           */
          void dumpAMCFIFO(xgi::Input* in, xgi::Output* out);

        private:
          void     createAMCInfoSpaceItems(is_toolbox_ptr is_amc, amc_shared_ptr amc);
          uint16_t m_amcEnableMask;

          toolbox::task::WorkLoop *p_amc_wl;                     ///< paralelize the calls to different AMCs
          toolbox::BSem m_amc_wl_semaphore[MAX_AMCS_PER_CRATE];  ///< do we need a semaphore for the workloop or each of them?

          class AMCInfo {

          public:
            AMCInfo();
            void registerFields(xdata::Bag<AMCManager::AMCInfo>* bag);

            // monitoring information
            xdata::Boolean present;  ///< FIXME BAD USAGE
            xdata::Integer crateID;  ///< Specifies the crate to which the OptoHybrid is connected
            xdata::Integer slotID;   ///< Specifies the AMC slot to which the OptoHybrid is connected

            // list of GTX links to enable in the DAQ
            xdata::String            gtxLinkEnableList;
            xdata::UnsignedInteger32 gtxLinkEnableMask;

            // registers to set
            xdata::Integer sbitSource;
            xdata::Boolean enableZS;

            inline std::string toString() {
              std::stringstream os;
              os << "present:"  << present.toString()  << std::endl
                 << "crateID:"  << crateID.toString()  << std::endl
                 << "slotID:"   << slotID.toString()   << std::endl

                 << "gtxLinkEnableList:" << gtxLinkEnableList.toString() << std::endl
                 << "gtxLinkEnableMask:" << std::hex << gtxLinkEnableMask.value_ << std::dec << std::endl

                 << "sbitSource:0x"      << std::hex << sbitSource.value_ << std::dec << std::endl
                 << "enableZS:0x"        << std::hex << enableZS.value_   << std::dec << std::endl
                 << std::endl;
              return os.str();
            };
          };

          mutable gem::utils::Lock m_deviceLock;  ///< [MAX_AMCS_PER_CRATE];

          std::array<amc_shared_ptr, MAX_AMCS_PER_CRATE> m_amcs;                      ///< HwGLIB pointers to be managed
          std::array<std::shared_ptr<AMCMonitor>, MAX_AMCS_PER_CRATE> m_amcMonitors;  ///< AMCMonito pointers to be managed
          std::array<is_toolbox_ptr, MAX_AMCS_PER_CRATE> is_amcs;                      ///< AMC InfoSpace pointers to be managed

          xdata::Vector<xdata::Bag<AMCInfo> > m_amcInfo;  ///< [MAX_AMCS_PER_CRATE];
          xdata::String  m_amcSlots;           ///< 
          xdata::String  m_connectionFile;     ///< 
          xdata::Boolean m_doPhaseShift;       ///< Whether or not to do a phase shifting procedure during configuration
          xdata::Boolean m_bc0LockPhaseShift;  ///< Use BC0 to find the best phase during the phase shifting procedure
          xdata::Boolean m_relockPhase;        ///< Relock the phase during phase shifting

	  uint32_t m_lastLatency;         ///< Special variable for latency scan mode
          uint32_t m_lastVT1, m_lastVT2;  ///< Special variable for threshold scan mode 
        };  // class AMCManager

    }  // namespace gem::hw::amc
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_AMC_AMCMANAGER_H
