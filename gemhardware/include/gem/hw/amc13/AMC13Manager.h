#ifndef GEM_HW_AMC13_AMC13MANAGER_H
#define GEM_HW_AMC13_AMC13MANAGER_H

//copying general structure of the HCAL DTCManager (HCAL name for AMC13)
#include "uhal/uhal.hpp"

#include "gem/base/GEMFSMApplication.h"
#include "gem/hw/amc13/exception/Exception.h"
//#include "gem/hw/amc13/AMC13Monitoring.hh"

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

          AMC13Manager(xdaq::ApplicationStub * s)
            throw (xdaq::exception::Exception);

          virtual ~AMC13Manager();

        protected:
          virtual void init();

          virtual void actionPerformed(xdata::Event& event);

          ::amc13::Status *getHTMLStatus()  const;
          ::amc13::AMC13  *getAMC13Device() const { return p_amc13; };

          //state transitions
          virtual void initializeAction() throw (gem::hw::amc13::exception::Exception);
          virtual void configureAction()  throw (gem::hw::amc13::exception::Exception);
          virtual void startAction()      throw (gem::hw::amc13::exception::Exception);
          virtual void pauseAction()      throw (gem::hw::amc13::exception::Exception);
          virtual void resumeAction()     throw (gem::hw::amc13::exception::Exception);
          virtual void stopAction()       throw (gem::hw::amc13::exception::Exception);
          virtual void haltAction()       throw (gem::hw::amc13::exception::Exception);
          virtual void resetAction()      throw (gem::hw::amc13::exception::Exception);

          xoap::MessageReference sendTriggerBurst(xoap::MessageReference mns)
	    throw (xoap::exception::Exception);
          xoap::MessageReference enableTriggers(xoap::MessageReference mns)
	    throw (xoap::exception::Exception);
          xoap::MessageReference disableTriggers(xoap::MessageReference mns)
	    throw (xoap::exception::Exception);

          //virtual void noAction()         throw (gem::hw::amc13::exception::Exception); 
	
          virtual void failAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);

          virtual void resetAction(toolbox::Event::Reference e)
            throw (toolbox::fsm::exception::Exception);

	  class BGOInfo
	  {
	  public:
            BGOInfo();

	    void registerFields(xdata::Bag<BGOInfo> *bag);

	    // can configure up to 4 BGO channels
	    xdata::Integer           channel;
	    xdata::UnsignedInteger32 cmd;
	    xdata::UnsignedInteger32 bx;
	    xdata::UnsignedInteger32 prescale;
	    xdata::Boolean           repeat;
	    xdata::Boolean           isLong;
	  };

	  class L1AInfo
	  {
	  public:
            L1AInfo();

	    void registerFields(xdata::Bag<L1AInfo> *bag);

	    xdata::Boolean           enableLocalL1A;
	    xdata::UnsignedInteger32 internalPeriodicPeriod;
	    xdata::Integer           l1Amode;
	    xdata::Integer           l1Arules;
	    xdata::UnsignedInteger32 l1Aburst;
	    xdata::Boolean           sendl1ATriburst;
	    xdata::Boolean           startl1ATricont;

	  };

          class AMC13Info
          {
          public:

	    void registerFields(xdata::Bag<AMC13Info> *bag);

            xdata::String connectionFile;
            xdata::String cardName;
            xdata::String amcInputEnableList;
            xdata::String amcIgnoreTTSList;

            xdata::Boolean enableDAQLink;
            xdata::Boolean enableFakeData;
            xdata::Boolean monBackPressure;
            xdata::Boolean enableLocalTTC;

	    xdata::Bag<L1AInfo> localTriggerConfig;

            xdata::Vector<xdata::Bag<BGOInfo> > bgoConfig;

            xdata::Integer prescaleFactor;
            xdata::Integer bcOffset;

            xdata::UnsignedInteger32 fedID;
            xdata::UnsignedInteger32 sfpMask;
            xdata::UnsignedInteger32 slotMask;

            //xdata::UnsignedInteger64 localL1AMask;
          };

        private:
          mutable gem::utils::Lock m_amc13Lock;

          ::amc13::AMC13 *p_amc13;
	  //hcal::utca::DTCMonitoring m_monitoringHelper; to be developed!!!

          //paramters taken from hcal::DTCManager (the amc13 manager for hcal)
          xdata::Integer m_crateID, m_slot;

          xdata::Bag<AMC13Info>               m_amc13Params;
          xdata::Vector<xdata::Bag<BGOInfo> > m_bgoConfig;
	  xdata::Bag<L1AInfo>                 m_localTriggerConfig;
          //seems that we've duplicated the members of the m_amc13Params as class variables themselves
          //what is the reason for this?  is it necessary/better to have these variables?
          std::string m_connectionFile, m_cardName, m_amcInputEnableList, m_slotEnableList, m_amcIgnoreTTSList;
          bool m_enableDAQLink, m_enableFakeData;
          bool m_monBackPressEnable, m_megaMonitorScale;
          bool m_enableLocalTTC, m_ignoreAMCTTS, m_enableLocalL1A, m_sendL1ATriburst, m_startL1ATricont,
	    m_bgoRepeat, m_bgoIsLong; //m_enableCalpulse,
          int m_localTriggerMode, m_localTriggerPeriod, m_localTriggerRate, m_L1Amode, m_L1Arules;
          int m_prescaleFactor, m_bcOffset, m_bgoChannel;
	  uint8_t m_bgoCMD;
	  uint16_t m_bgoBX, m_bgoPrescale;
          uint32_t m_fedID, m_sfpMask, m_slotMask, m_internalPeriodicPeriod, m_L1Aburst;
          //uint64_t m_localL1AMask;

          ////counters

        protected:

        };  // class AMC13Manager

    }  // namespace gem::hw::amc13
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_AMC13_AMC13MANAGER_H
