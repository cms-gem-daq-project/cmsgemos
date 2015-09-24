#ifndef gem_hw_optohybrid_OptoHybridManager_h
#define gem_hw_optohybrid_OptoHybridManager_h

//#include "uhal/uhal.hpp"

#include "gem/base/GEMFSMApplication.h"
//#include "gem/hw/optohybrid/OptoHybridSettings.h"

#include "gem/hw/optohybrid/exception/Exception.h"

namespace gem {
  namespace hw {
    namespace optohybrid {

      class HwOptoHybrid;
      class OptoHybridManagerWeb;
      
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
          class OptoHybridInfo {
            
          public:
            OptoHybridInfo();
            void registerFields(xdata::Bag<OptoHybridManager::OptoHybridInfo>* bag);
            //monitoring information
            xdata::Boolean present;
            xdata::Integer crateID;
            xdata::Integer slotID;

            //configuration parameters
            xdata::String controlHubIPAddress;
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
                
                 << "controlHubIPAddress:" << controlHubIPAddress.toString() << std::endl
                 << "deviceIPAddress:"     << deviceIPAddress.toString()     << std::endl
                 << "ipBusProtocol:"       << ipBusProtocol.toString()       << std::endl
                 << "addressTable:"        << addressTable.toString()        << std::endl
                 << "controlHubPort:"      << controlHubPort.toString()      << std::endl
                 << "ipBusPort:"           << ipBusPort.toString()           << std::endl
                 << "triggerSource:0x"     << triggerSource.toString()       << std::endl
                 << "sbitSource:0x"        << sbitSource.toString()          << std::endl
                 << std::endl;
              return os.str();
            };
          };
          
          mutable gem::utils::Lock m_deviceLock;//[MAX_AMCS_PER_CRATE];
	  
          HwOptoHybrid* m_optohybrids[MAX_AMCS_PER_CRATE];
          xdata::InfoSpace* is_optohybrids[MAX_AMCS_PER_CRATE];
          xdata::Vector<xdata::Bag<OptoHybridInfo> > m_optohybridInfo;
          xdata::String        m_connectionFile;
        }; //end class OptoHybridManager
      
    }//end namespace gem::hw::optohybrid
  }//end namespace gem::hw
}//end namespace gem

#endif
