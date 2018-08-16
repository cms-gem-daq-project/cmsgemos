/** @file AMC13Manager.h */

#ifndef GEM_HW_AMC13_AMC13MANAGER_H
#define GEM_HW_AMC13_AMC13MANAGER_H

#include <iomanip>

//copying general structure of the HCAL DTCManager (HCAL name for AMC13)
#include "uhal/uhal.hpp"

#include "gem/base/GEMFSMApplication.h"
#include "gem/hw/amc13/exception/Exception.h"
//#include "gem/hw/amc13/AMC13Monitoring.hh"

#include "toolbox/task/TimerFactory.h"
#include "toolbox/task/TimerListener.h"
#include "toolbox/task/TimerEvent.h"
#include "toolbox/lang/Class.h"
#include "toolbox/TimeVal.h"
#include "toolbox/TimeInterval.h"

namespace amc13 {
  class AMC13;
  class Status;
}

namespace gem {
  namespace hw {
    namespace amc13 {

      typedef std::shared_ptr< ::amc13::Status> amc13_status_ptr;
      typedef std::shared_ptr< ::amc13::AMC13>  amc13_ptr;
      typedef std::shared_ptr<gem::base::utils::GEMInfoSpaceToolBox> is_toolbox_ptr;

      class AMC13ManagerWeb;
      /*
      class AMC13ManagerListener :
	{
	public:
	  toolbox::task::Timer* p_timer;    // timer for general info space updates
	  void timer_triggerupdate() throw (xgi::exception::Exception);
	  virtual void timeExpired(toolbox::task::TimerEvent& event);
	}
      */
      class AMC13Manager : public gem::base::GEMFSMApplication, public toolbox::task::TimerListener
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

          amc13_status_ptr getHTMLStatus()  const;
          amc13_ptr        getAMC13Device() const { return p_amc13; };

          void setDisplayLevel(xgi::Input *in, xgi::Output *out)
            throw (xgi::exception::Exception);

          void updateStatus(xgi::Input *in, xgi::Output * out)
            throw (xgi::exception::Exception);

          //state transitions
          virtual void initializeAction() ; // throw (gem::hw::amc13::exception::Exception);
          virtual void configureAction()  ; // throw (gem::hw::amc13::exception::Exception);
          virtual void startAction()      ; // throw (gem::hw::amc13::exception::Exception);
          virtual void pauseAction()      ; // throw (gem::hw::amc13::exception::Exception);
          virtual void resumeAction()     ; // throw (gem::hw::amc13::exception::Exception);
          virtual void stopAction()       ; // throw (gem::hw::amc13::exception::Exception);
          virtual void haltAction()       ; // throw (gem::hw::amc13::exception::Exception);
          virtual void resetAction()      ; // throw (gem::hw::amc13::exception::Exception);

          xoap::MessageReference sendTriggerBurst(xoap::MessageReference mns);
          xoap::MessageReference enableTriggers(xoap::MessageReference mns);
          xoap::MessageReference disableTriggers(xoap::MessageReference mns);

	  void endScanPoint();

	  virtual void timeExpired(toolbox::task::TimerEvent& event);

          // virtual void noAction();

          virtual void failAction(toolbox::Event::Reference e);

          virtual void resetAction(toolbox::Event::Reference e);

	  class BGOInfo
	  {
	  public:
            BGOInfo();
            void registerFields(xdata::Bag<BGOInfo> *bag);

	    xdata::Integer           channel;
	    xdata::UnsignedInteger32 cmd;
	    xdata::UnsignedInteger32 bx;
	    xdata::UnsignedInteger32 prescale;
	    xdata::Boolean           repeat;
	    xdata::Boolean           isLong;

            inline std::string toString() {
              std::stringstream os;
              os << "channel : " <<  channel.toString()  << std::endl
                 << "cmd     : " <<  cmd.toString()      << std::endl
                 << "bx      : " <<  bx.toString()       << std::endl
                 << "prescale: " <<  prescale.toString() << std::endl
                 << "repeat  : " <<  repeat.toString()   << std::endl
                 << "isLong  : " <<  isLong.toString()   << std::endl
                 << std::endl;
              return os.str();
            };
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
	    xdata::Boolean           sendl1ATriburst; // need to remove
	    xdata::Boolean           startl1ATricont; // need to remove
	    xdata::Boolean           enableLEMO;

            inline std::string toString() {
              std::stringstream os;
              os << "enableLocalL1A        : " <<  enableLocalL1A.toString()         << std::endl
                 << "internalPeriodicPeriod: " <<  internalPeriodicPeriod.toString() << std::endl
                 << "l1Amode               : " <<  l1Amode.toString()                << std::endl
                 << "l1Arules              : " <<  l1Arules.toString()               << std::endl
                 << "l1Aburst              : " <<  l1Aburst.toString()               << std::endl
                 << "sendl1ATriburst       : " <<  sendl1ATriburst.toString()        << std::endl
                 << "startl1ATricont       : " <<  startl1ATricont.toString()        << std::endl
                 << "enableLEMO            : " <<  enableLEMO.toString()             << std::endl
                 << std::endl;
              return os.str();
            };
	  };

	  class TTCInfo
	  {
	  public:
            TTCInfo();
	    void registerFields(xdata::Bag<TTCInfo> *bag);

	    xdata::UnsignedInteger32 resyncCommand;
	    xdata::UnsignedInteger32 resyncMask;
	    xdata::UnsignedInteger32 oc0Command;
	    xdata::UnsignedInteger32 oc0Mask;

            inline std::string toString() {
              std::stringstream os;
              os << "resyncCommand: " << std::hex << resyncCommand.value_ << std::endl
                 << "resyncMask:    " << std::hex << resyncMask.value_    << std::endl
                 << "oc0Command:    " << std::hex << oc0Command.value_    << std::endl
                 << "oc0Mask:       " << std::hex << oc0Mask.value_       << std::endl
                 << std::endl;
              return os.str();
            };
	  };

          class AMC13Info
          {
          public:
            AMC13Info();
	    void registerFields(xdata::Bag<AMC13Info> *bag);

            xdata::String connectionFile;
            xdata::String cardName;
            xdata::String amcInputEnableList;
            xdata::String amcIgnoreTTSList;

            xdata::Boolean enableDAQLink;
            xdata::Boolean enableFakeData;
            xdata::Boolean monBackPressure;
            xdata::Boolean enableLocalTTC;
            xdata::Boolean skipPLLReset;

	    xdata::Bag<L1AInfo> localTriggerConfig;
	    xdata::Bag<TTCInfo> amc13TTCConfig;

	    // can configure up to 4 BGO channels
            xdata::Vector<xdata::Bag<BGOInfo> > bgoConfig;

            xdata::Integer prescaleFactor;
            xdata::Integer bcOffset;

            xdata::UnsignedInteger32 fedID;
            xdata::UnsignedInteger32 sfpMask;
            xdata::UnsignedInteger32 slotMask;

            //xdata::UnsignedInteger64 localL1AMask;

            inline std::string toString() {
              std::stringstream os;
              os << "connectionFile:     " << connectionFile.toString()         << std::endl
                 << "cardName:           " << cardName.toString()               << std::endl
                 << "amcInputEnableList: " << amcInputEnableList.toString()     << std::endl
                 << "amcIgnoreTTSList:   " << amcIgnoreTTSList.toString()       << std::endl
                 << "enableDAQLink:      " << enableDAQLink.toString()          << std::endl
                 << "enableFakeData:     " << enableFakeData.toString()         << std::endl
                 << "monBackPressure:    " << monBackPressure.toString()        << std::endl
                 << "enableLocalTTC:     " << enableLocalTTC.toString()         << std::endl
                 << "skipPLLReset:       " << skipPLLReset.toString()           << std::endl
                 << "localTriggerConfig: " << localTriggerConfig.bag.toString() << std::endl
                 << "amc13TTCConfig:     " << amc13TTCConfig.bag.toString()     << std::endl;
              // can configure up to 4 BGO channels
               for (auto bgo = bgoConfig.begin(); bgo != bgoConfig.end(); ++bgo)
                 os << "bgoConfig: " << bgo->bag.toString() << std::endl;
               os << "prescaleFactor: " << prescaleFactor.toString() << std::endl
                  << "bcOffset:       " << bcOffset.toString()       << std::endl
                  << "fedID         0x" << std::hex << std::setw(8) << std::setfill('0') << fedID.value_    << std::endl
                  << "sfpMask       0x" << std::hex << std::setw(8) << std::setfill('0') << sfpMask.value_  << std::endl
                  << "slotMask      0x" << std::hex << std::setw(8) << std::setfill('0') << slotMask.value_ << std::endl;
              // os << "localL1AMask: 0x" << std::hex << std::setw(8) << std::setfill('0') << localL1AMask << std::dec << std::endl;
               return os.str();
            };
          };

        private:
          mutable gem::utils::Lock m_amc13Lock;

          amc13_ptr p_amc13;

	  toolbox::task::Timer* p_timer;    // timer for general info space updates

          //paramters taken from hcal::DTCManager (the amc13 manager for hcal)
          xdata::Integer m_crateID, m_slot;

          xdata::Bag<AMC13Info>               m_amc13Params;
          xdata::Vector<xdata::Bag<BGOInfo> > m_bgoConfig;
	  xdata::Bag<L1AInfo>                 m_localTriggerConfig;
	  xdata::Bag<TTCInfo>                 m_amc13TTCConfig;


          //seems that we've duplicated the members of the m_amc13Params as class variables themselves
          //what is the reason for this?  is it necessary/better to have these variables?
          std::string m_connectionFile, m_cardName, m_amcInputEnableList, m_slotEnableList, m_amcIgnoreTTSList;
          bool m_enableDAQLink, m_enableFakeData;
          bool m_monBackPressEnable, m_megaMonitorScale;
          bool m_enableLocalTTC, m_skipPLLReset, m_enableLocalL1A,
            m_sendL1ATriburst, m_startL1ATricont, // need to remove
	    m_bgoRepeat, m_bgoIsLong, m_enableLEMO;
          int m_localTriggerMode, m_localTriggerPeriod, m_localTriggerRate, m_L1Amode, m_L1Arules;
          int m_prescaleFactor, m_bcOffset, m_bgoChannel;
	  uint8_t m_bgoCMD;
	  uint16_t m_bgoBX, m_bgoPrescale;
          uint32_t m_ignoreAMCTTS, m_fedID, m_sfpMask, m_slotMask,
            m_internalPeriodicPeriod, m_L1Aburst;
          //uint64_t m_localL1AMask;
	  uint64_t m_updatedL1ACount;
          ////counters

        protected:

        };  // class AMC13Manager

    }  // namespace gem::hw::amc13
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_AMC13_AMC13MANAGER_H
