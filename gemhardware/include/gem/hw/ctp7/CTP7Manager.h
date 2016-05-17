/** @file CTP7Manager.h */

#ifndef GEM_HW_CTP7_CTP7MANAGER_H
#define GEM_HW_CTP7_CTP7MANAGER_H

//#include "uhal/uhal.hpp"

#include "gem/base/GEMFSMApplication.h"
//#include "gem/hw/ctp7/CTP7Settings.h"

#include "gem/hw/ctp7/exception/Exception.h"

namespace gem {
  namespace hw {
    namespace ctp7 {

      class HwCTP7;
      class CTP7ManagerWeb;
      class CTP7Monitor;

      typedef std::shared_ptr<HwCTP7>  ctp7_shared_ptr;
      typedef std::shared_ptr<gem::base::utils::GEMInfoSpaceToolBox> is_toolbox_ptr;

      class CTP7Manager : public gem::base::GEMFSMApplication
        {

          friend class CTP7ManagerWeb;
          //friend class CTP7Monitor;

        public:
          XDAQ_INSTANTIATOR();

          CTP7Manager(xdaq::ApplicationStub* s);

          virtual ~CTP7Manager();

        protected:
          virtual void init();

          virtual void actionPerformed(xdata::Event& event);

          //state transitions
          virtual void initializeAction() throw (gem::hw::ctp7::exception::Exception);
          virtual void configureAction()  throw (gem::hw::ctp7::exception::Exception);
          virtual void startAction()      throw (gem::hw::ctp7::exception::Exception);
          virtual void pauseAction()      throw (gem::hw::ctp7::exception::Exception);
          virtual void resumeAction()     throw (gem::hw::ctp7::exception::Exception);
          virtual void stopAction()       throw (gem::hw::ctp7::exception::Exception);
          virtual void haltAction()       throw (gem::hw::ctp7::exception::Exception);
          virtual void resetAction()      throw (gem::hw::ctp7::exception::Exception);
          //virtual void noAction()         throw (gem::hw::ctp7::exception::Exception);

          virtual void failAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);

          virtual void resetAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);

	  bool is_initialized_, is_configured_, is_running_, is_paused_, is_resumed_;

        protected:
          /**
           */
          std::vector<uint32_t> dumpCTP7FIFO(int const& ctp7);

          /**
           */
          void dumpCTP7FIFO(xgi::Input* in, xgi::Output* out);

        private:
	  uint16_t parseAMCEnableList(std::string const&);
	  bool     isValidSlotNumber( std::string const&);
          void     createCTP7InfoSpaceItems(is_toolbox_ptr is_ctp7, ctp7_shared_ptr ctp7);
          uint16_t m_amcEnableMask;

          class CTP7Info {

          public:
            CTP7Info();
            void registerFields(xdata::Bag<CTP7Manager::CTP7Info>* bag);
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
            xdata::Integer sbitSource;

            inline std::string toString() {
              std::stringstream os;
              os << "present:" << present.toString() << std::endl
                 << "crateID:" << crateID.toString() << std::endl
                 << "slotID:"  << slotID.toString()  << std::endl

                 << "controlHubAddress:" << controlHubAddress.toString() << std::endl
                 << "deviceIPAddress:"   << deviceIPAddress.toString()   << std::endl
                 << "ipBusProtocol:"     << ipBusProtocol.toString()     << std::endl
                 << "addressTable:"      << addressTable.toString()      << std::endl
                 << "controlHubPort:"    << controlHubPort.value_        << std::endl
                 << "ipBusPort:"         << ipBusPort.value_             << std::endl
                 << "sbitSource:0x"      << std::hex << sbitSource.value_    << std::dec << std::endl
                 << std::endl;
              return os.str();
            };
          };

          mutable gem::utils::Lock m_deviceLock;  // [MAX_AMCS_PER_CRATE];

          std::array<ctp7_shared_ptr, MAX_AMCS_PER_CRATE>              m_ctp7s;
          std::array<std::shared_ptr<CTP7Monitor>, MAX_AMCS_PER_CRATE> m_ctp7Monitors;
          std::array<is_toolbox_ptr, MAX_AMCS_PER_CRATE>               is_ctp7s;

          xdata::Vector<xdata::Bag<CTP7Info> > m_ctp7Info;  // [MAX_AMCS_PER_CRATE];
          xdata::String                        m_amcSlots;
          xdata::String                        m_connectionFile;
        };  // class CTP7Manager

    }  // namespace gem::hw::ctp7
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_CTP7_CTP7MANAGER_H
