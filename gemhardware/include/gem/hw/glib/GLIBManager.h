#ifndef gem_hw_glib_GLIBManager_h
#define gem_hw_glib_GLIBManager_h

//#include "uhal/uhal.hpp"

#include "gem/base/GEMFSMApplication.h"
//#include "gem/hw/glib/GLIBSettings.h"

#include "gem/hw/glib/exception/Exception.h"

namespace gem {
  namespace hw {
    namespace glib {

      class HwGLIB;
      class GLIBManagerWeb;

      class GLIBManager : public gem::base::GEMFSMApplication
        {
	  
          friend class GLIBManagerWeb;

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
	
        private:
	  uint16_t parseAMCEnableList(std::string const&);
	  bool     isValidSlotNumber( std::string const&);

          uint16_t m_amcEnableMask;

          class GLIBInfo {
	    
          public:
            GLIBInfo();
            void registerFields(xdata::Bag<GLIBManager::GLIBInfo>* bag);
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
	  
          std::shared_ptr<HwGLIB> m_glibs[MAX_AMCS_PER_CRATE];
          xdata::InfoSpace* is_glibs[MAX_AMCS_PER_CRATE];
          xdata::Vector<xdata::Bag<GLIBInfo> > m_glibInfo;//[MAX_AMCS_PER_CRATE];
          xdata::String        m_amcSlots;
          xdata::String        m_connectionFile;
        }; //end class GLIBManager
            
    }//end namespace gem::hw::glib
  }//end namespace gem::hw
}//end namespace gem

#endif
