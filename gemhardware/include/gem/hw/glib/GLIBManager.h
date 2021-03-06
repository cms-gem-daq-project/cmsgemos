/** @file GLIBManager.h */

#ifndef GEM_HW_GLIB_GLIBMANAGER_H
#define GEM_HW_GLIB_GLIBMANAGER_H

#include <array>

#include "gem/base/GEMFSMApplication.h"
//#include "gem/hw/glib/GLIBSettings.h"

#include "gem/hw/glib/exception/Exception.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"
#include "gem/utils/exception/Exception.h"

namespace gem {
  namespace hw {
    namespace glib {

      class HwGLIB;
      class GLIBManagerWeb;
      class GLIBMonitor;

      typedef std::shared_ptr<HwGLIB>  glib_shared_ptr;
      typedef std::shared_ptr<gem::base::utils::GEMInfoSpaceToolBox> is_toolbox_ptr;

      class GLIBManager : public gem::base::GEMFSMApplication
        {

          friend class GLIBManagerWeb;
          //friend class GLIBMonitor;

        public:
          XDAQ_INSTANTIATOR();

          GLIBManager(xdaq::ApplicationStub* s);

          virtual ~GLIBManager();

        protected:
          virtual void init();

          virtual void actionPerformed(xdata::Event& event);

          //state transitions
          virtual void initializeAction() throw (gem::hw::glib::exception::Exception);
          virtual void configureAction()  throw (gem::hw::glib::exception::Exception);
          virtual void startAction()      throw (gem::hw::glib::exception::Exception);
          virtual void pauseAction()      throw (gem::hw::glib::exception::Exception);
          virtual void resumeAction()     throw (gem::hw::glib::exception::Exception);
          virtual void stopAction()       throw (gem::hw::glib::exception::Exception);
          virtual void haltAction()       throw (gem::hw::glib::exception::Exception);
          virtual void resetAction()      throw (gem::hw::glib::exception::Exception);
          //virtual void noAction()         throw (gem::hw::glib::exception::Exception);

          virtual void failAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);

          virtual void resetAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);

	  bool is_initialized_, is_configured_, is_running_, is_paused_, is_resumed_;

        protected:
          /**
           */
          std::vector<uint32_t> dumpGLIBFIFO(int const& glib);

          /**
           */
          void dumpGLIBFIFO(xgi::Input* in, xgi::Output* out);

        private:
	  //uint16_t parseAMCEnableList(std::string const&);
	  //bool     isValidSlotNumber( std::string const&);
          void     createGLIBInfoSpaceItems(is_toolbox_ptr is_glib, glib_shared_ptr glib);
          uint16_t m_amcEnableMask;

          class GLIBInfo {

          public:
            GLIBInfo();
            void registerFields(xdata::Bag<GLIBManager::GLIBInfo>* bag);

            //monitoring information
            xdata::Boolean present;
            xdata::Integer crateID;
            xdata::Integer slotID;
            xdata::String  cardName;
            xdata::String  birdName;

            //configuration parameters
            xdata::String controlHubAddress;
            xdata::String deviceIPAddress;
            xdata::String ipBusProtocol;
            xdata::String addressTable;

            // list of GTX links to enable in the DAQ
            xdata::String            gtxLinkEnableList;
            xdata::UnsignedInteger32 gtxLinkEnableMask;

            xdata::UnsignedInteger32 controlHubPort;
            xdata::UnsignedInteger32 ipBusPort;

            //registers to set
            xdata::Integer sbitSource;

            inline std::string toString() {
              std::stringstream os;
              os << "present:"  << present.toString()  << std::endl
                 << "crateID:"  << crateID.toString()  << std::endl
                 << "slotID:"   << slotID.toString()   << std::endl
                 << "cardName:" << cardName.toString() << std::endl
                 << "birdName:" << birdName.toString() << std::endl

                 << "controlHubAddress:" << controlHubAddress.toString() << std::endl
                 << "deviceIPAddress:"   << deviceIPAddress.toString()   << std::endl
                 << "ipBusProtocol:"     << ipBusProtocol.toString()     << std::endl
                 << "addressTable:"      << addressTable.toString()      << std::endl
                 << "controlHubPort:"    << controlHubPort.value_        << std::endl
                 << "ipBusPort:"         << ipBusPort.value_             << std::endl

                 << "gtxLinkEnableList:" << gtxLinkEnableList.toString() << std::endl
                 << "gtxLinkEnableMask:" << std::hex << gtxLinkEnableMask.value_ << std::dec << std::endl

                 << "sbitSource:0x"      << std::hex << sbitSource.value_ << std::dec << std::endl
                 << std::endl;
              return os.str();
            };
          };

          mutable gem::utils::Lock m_deviceLock;  // [MAX_AMCS_PER_CRATE];

          std::array<glib_shared_ptr, MAX_AMCS_PER_CRATE>              m_glibs;
          std::array<std::shared_ptr<GLIBMonitor>, MAX_AMCS_PER_CRATE> m_glibMonitors;
          std::array<is_toolbox_ptr, MAX_AMCS_PER_CRATE>               is_glibs;

          xdata::Vector<xdata::Bag<GLIBInfo> > m_glibInfo;  // [MAX_AMCS_PER_CRATE];
          xdata::String                        m_amcSlots;
          xdata::String                        m_connectionFile;
          xdata::Boolean                       m_uhalPhaseShift;
          xdata::Boolean                       m_bc0LockPhaseShift;
          xdata::Boolean                       m_relockPhase;
          xdata::Boolean                       m_enableZS;

	  uint32_t m_lastLatency, m_lastVT1, m_lastVT2;
        };  // class GLIBManager

    }  // namespace gem::hw::glib
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_GLIB_GLIBMANAGER_H
