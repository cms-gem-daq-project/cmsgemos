#ifndef gem_supervisor_GEMTestBeamSupervisor_h
#define gem_supervisor_GEMTestBeamSupervisor_h

#include <cstdlib>
#include "string.h"
#include "limits.h"
#include <string>
#include <string>
#include <deque>
#include <map>

#include "xdaq/Application.h"
#include "log4cplus/logger.h"

#include "toolbox/BSem.h"
#include "toolbox/task/WorkLoop.h"
#include "toolbox/fsm/FiniteStateMachine.h"
#include "toolbox/mem/MemoryPoolFactory.h"

#include "xdata/Bag.h"
#include "xdata/Vector.h"
#include "xdata/InfoSpace.h"
#include "xdata/ActionListener.h"

#include "xdata/Float.h"
#include "xdata/String.h"
#include "xdata/Boolean.h"
#include "xdata/Integer.h"
#include "xdata/Integer64.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger.h"

#include "xoap/MessageReference.h"
#include "i2o/utils/AddressMap.h"

#include "uhal/uhal.hpp"

#include "xgi/framework/Method.h"
#include "cgicc/HTMLClasses.h"

//namespace cgicc {
//  BOOLEAN_ELEMENT(section,"section");
//}

namespace toolbox {
  namespace fsm {
    class AsynchronousFiniteStateMachine;
  }
}

namespace gem {
  namespace hw{
    class GEMHwDevice;
    
    namespace vfat {
      class VFAT2Manager;
    }
  }
  
  namespace supervisor {
    
    class GEMTestBeamSupervisorWeb;

    namespace tbutils {
      class LatencyScan;
      class ThresholdScan;
      class DAQWeb;
    }
    
    class GEMTestBeamSupervisor : virtual public xdaq::Application, virtual public xdata::ActionListener {

    public:
      GEMTestBeamSupervisor(xdaq::ApplicationStub* stub);
      ~GEMTestBeamSupervisor();

      
      
      GEMTestBeamSupervisorWeb* webInterface_;

      tbutils::LatencyScan*   latencyScan_;
      tbutils::ThresholdScan* thresholdScan_;
      tbutils::DAQWeb*        daqWebInterface_;

      // Connected chips (chipID,GEB slot)
      std::vector<std::pair<uint16_t,uint8_t> > connectedChips_;
      // VFAT2Manager app for each connected chip
      std::vector<gem::hw::vfat::VFAT2Manager*> managerApps_;

      void actionPerformed(xdata::Event& event);
      
    protected:
      log4cplus::Logger m_gemLogger;
      
    private:
      /**
       * Here we should set up the connection to each of the VFATs connected to the system
       * or that we are interested in
       * Simple loading of address table and finding the connected devices
       * this list should be saved into the managerApps_ variable (we should create a VFAT2Manager
       * for each connected VFAT)
       * Additionally, the connection to the web interface should be established here
       **/
      void initialize();
      
      /**
       * Is the supervisor an FSM compatible application?
       **/
      toolbox::fsm::AsynchronousFiniteStateMachine* gemTBfsmP_;
      
      i2o::utils::AddressMap *i2oAddressMap_;
      toolbox::mem::MemoryPoolFactory *poolFactory_;
      
      /**** application properties ****/
      xdata::InfoSpace *appInfoSpace_;
      
      xdaq::ApplicationDescriptor *appDescriptor_;
      xdaq::ApplicationContext    *appContext_;
      xdaq::ApplicationGroup      *appGroup_;
      
      std::string xmlClass_;
      unsigned long instance_;
      std::string appURN_;

      xdata::Boolean isInitialized_;
      
      //for the asynchronous operation?
      toolbox::BSem semaphore_;
      toolbox::BSem wl_semaphore_;
      
      toolbox::task::WorkLoop *wl_;
      
      toolbox::task::ActionSignature *enable_sig_, *configure_sig_, *initialize_sig_;
      toolbox::task::ActionSignature *start_sig_,  *pause_sig_, *resume_sig_;
      toolbox::task::ActionSignature *stop_sig_,   *halt_sig_;
    };
    
  } //end namespace supervisor
} //end namespace gem
#endif
