#ifndef gem_readout_GEMDataParker_h
#define gem_readout_GEMDataParker_h

#include "toolbox/SyncQueue.h"
#include "i2o/i2o.h"
#include "toolbox/Task.h"
#include "toolbox/mem/Pool.h"

#include "xoap/MessageReference.h"
#include "xoap/Method.h"

#include "xdata/String.h"
#include <string>
#include <queue>

#include "gem/utils/GEMLogging.h"
#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

#include "gem/hw/amc13/Module.hh"
#include "amc13/AMC13.hh"

namespace gem {
  namespace readout {

    class GEMDataParker
    {
    public:
      static const int I2O_READOUT_NOTIFY;
      static const int I2O_READOUT_CONFIRM;

      static const uint32_t kUPDATE;
      static const uint32_t kUPDATE7;

      GEMDataParker        (amc13::AMC13& amc13, 
                            std::string const& outFileName);
      ~GEMDataParker() {};

      void dumpData();

      int queueDepth       () {return m_dataque.size();}

      //void ScanRoutines(uint8_t latency, uint8_t VT1, uint8_t VT2);

      //uint64_t Runtype() {
	    //  uint64_t RunType = BOOST_BINARY( 1 ); // :4
	    //  return (RunType << 24)|(m_latency << 16)|(m_VT1 << 8)|(m_VT2);
      //}

      // SOAP interface, updates the header used for calibration runs
      //xoap::MessageReference updateScanParameters(xoap::MessageReference message)
      //  throw (xoap::exception::Exception);

      
    private:

      uint8_t m_latency, m_VT1, m_VT2;

      static const int MaxVFATS = 24; // was 32 ???
      
      log4cplus::Logger m_gemLogger;
      amc13::AMC13* p_amc13;
      std::string m_outFileName;

      // queue safety
      mutable gem::utils::Lock m_queueLock;
      // The main data flow
      std::queue<uint32_t> m_dataque;
      
      /*
       * Counter all in one
       *   [0] VFAT's Blocks counter
       *   [1] Events counter
       *   [2] VFATs counter per last event
       *   [3] Good Events counter
       *   [4] Bad Events counter
      uint32_t m_counter[5];
       */

      // Events counter     
      uint64_t m_event;
         
      int16_t m_scanParam;

    };
  }
}
#endif
