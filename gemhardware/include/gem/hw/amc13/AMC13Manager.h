#ifndef gem_hw_amc13_AMC13Manager_h
#define gem_hw_amc13_AMC13Manager_h

//copying general structure of the HCAL DTCManager (HCAL name for AMC13)
#include "uhal/uhal.hpp"

#include "gem/base/GEMFSMApplication.h"
#include "gem/hw/amc13/exception/Exception.h"

namespace amc13 {
  class AMC13;
  class Status;
}

namespace gem {
  namespace hw {
    namespace amc13 {
      
      class AMC13ManagerWeb;
      
      class AMC13Manager : public gem::base::GEMFSMApplication
        {
	  
          friend class AMC13ManagerWeb;

        public:
          XDAQ_INSTANTIATOR();
	  
          AMC13Manager(xdaq::ApplicationStub * s);

          virtual ~AMC13Manager();
	  
        protected:

          virtual void preInit() throw (gem::base::exception::Exception);
          virtual void init()    throw (gem::base::exception::Exception);
          virtual void enable()  throw (gem::base::exception::Exception);
          virtual void disable() throw (gem::base::exception::Exception);
	  
          virtual void actionPerformed(xdata::Event& event);
          
          ::amc13::Status *getHTMLStatus()  const;
          ::amc13::AMC13  *getAMC13Device() const {
            return p_amc13;
          };

          //state transitions
          virtual void initializeAction() throw (gem::hw::amc13::exception::Exception);
          virtual void configureAction()  throw (gem::hw::amc13::exception::Exception);
          virtual void startAction()      throw (gem::hw::amc13::exception::Exception);
          virtual void pauseAction()      throw (gem::hw::amc13::exception::Exception);
          virtual void resumeAction()     throw (gem::hw::amc13::exception::Exception);
          virtual void stopAction()       throw (gem::hw::amc13::exception::Exception);
          virtual void haltAction()       throw (gem::hw::amc13::exception::Exception);
          virtual void resetAction()      throw (gem::hw::amc13::exception::Exception);
          //virtual void noAction()         throw (gem::hw::amc13::exception::Exception); 
	
          virtual void failAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception); 
	
          virtual void resetAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);
          
          class ConfigParams 
          {   
          public:
            void registerFields(xdata::Bag<ConfigParams> *bag);
            
            xdata::String connectionFile;
            xdata::String amcInputEnableList;
            xdata::String amcIgnoreTTSList;
            
            xdata::Boolean enableDAQLink;
            xdata::Boolean enableFakeData;
            xdata::Boolean monBackPressure;
            xdata::Boolean enableLocalTTC;

            xdata::Integer prescaleFactor;
            xdata::Integer bcOffset;

            xdata::UnsignedInteger32 fedID;
            xdata::UnsignedInteger32 sfpMask;
            xdata::UnsignedInteger32 slotMask;

            xdata::UnsignedInteger64 enableLocalL1AMask;
          };
          
        private:
          mutable gem::utils::Lock m_amc13Lock;
	
          ::amc13::AMC13 *p_amc13;
	  
          //paramters taken from hcal::DTCManager (the amc13 manager for hcal)
          xdata::Integer m_crateID, m_slot;

          xdata::Bag<ConfigParams> m_amc13Params;
          std::string m_AMCInputEnableList, m_SlotEnableList, m_AMCIgnoreTTSList;
          bool m_fakeDataEnable, m_localTtcSignalEnable;
          bool m_monBufBackPressEnable, m_megaMonitorScale;
          bool m_internalPeriodicEnable;
          bool m_ignoreAMCTTS;
          int m_internalPeriodicPeriod, m_preScaleFactNumOfZeros;
          int m_bcOffset;
          uint32_t m_fedId, m_sfpMask, m_slotMask;
	  
          ////counters

        protected:
	  
        }; //end class AMC13Manager

    }//end namespace gem::hw::amc13
    
  }//end namespace gem::hw
  
}//end namespace gem

#endif
