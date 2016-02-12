#ifndef gem_readout_GEMDataParker_h
#define gem_readout_GEMDataParker_h

#include "gem/readout/GEMDataAMCformat.h"
#include "gem/readout/gemOnlineDQM.h"

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

namespace gem {
  namespace hw {
    namespace glib {
      class HwGLIB;
    }   
  }
  namespace readout {
    struct GEMDataAMCformat;
  }
  namespace readout {

    class GEMDataParker
    {
    public:
      static const int I2O_READOUT_NOTIFY;
      static const int I2O_READOUT_CONFIRM;
      uint8_t latency_m, VT1_m, VT2_m;

      GEMDataParker        (gem::hw::glib::HwGLIB& glibDevice, 
                            std::string const& outFileName, 
                            std::string const& errFileName, 
                            std::string const& outputType,
                            std::string const& slotFileName                            
                            );
      ~GEMDataParker() {};//delete m_gemOnlineDQM;};

      uint32_t* dumpData   ( uint8_t const& mask );
      uint32_t* selectData ( uint32_t Counter[5]
                           );
      uint32_t* getGLIBData( uint8_t const& link,
                             uint32_t Counter[5]
                           );
      uint32_t* GEMEventMaker( uint32_t Counter[5]
                             );
      void GEMevSelector   ( const  uint32_t& ES
                           );
      void GEMfillHeaders  ( uint32_t const& BC,
                             uint32_t const& BX,
                             gem::readout::GEMDataAMCformat::GEMData& gem,
                             gem::readout::GEMDataAMCformat::GEBData& geb 
                           );
      bool VFATfillData    ( int const& islot,
                             gem::readout::GEMDataAMCformat::GEBData& geb
                           );
      void GEMfillTrailers ( gem::readout::GEMDataAMCformat::GEMData& gem,
                             gem::readout::GEMDataAMCformat::GEBData& geb
                           );
      void writeGEMevent   ( std::string outFile,
                             bool const& OKprint,
                             std::string const& TypeDataFlag,
                             gem::readout::GEMDataAMCformat::GEMData& gem,
                             gem::readout::GEMDataAMCformat::GEBData& geb,
                             gem::readout::GEMDataAMCformat::VFATData& vfat
                           );
      int queueDepth       () {return dataque.size();}
      

      void ScanRoutines(uint8_t latency_,uint8_t VT1_,uint8_t VT2_);
      //      void ScanRoutines(int latency_,int VT1_,int VT2_);
      uint64_t Runtype(){
	uint64_t RunType = BOOST_BINARY( 1 ); // :4
	uint64_t lat =  (0xff & latency_m); // :8
	uint64_t vt1 =  (0xff & VT1_m); // :8
	uint64_t vt2 =  (0xff & VT2_m); // :8

	//	return ((((((RunType<<4|lat)<<8))|vt1)<<8)|vt2);
	return (RunType << 24)|(lat << 16)|(vt1 << 8)|(vt2) ;//||(lat << 32)||(vt1<<16)||(vt2);
      }


      // SOAP interface, updates the header used for calibration runs
      xoap::MessageReference updateScanParameters(xoap::MessageReference message)
        throw (xoap::exception::Exception);


      
    private:

      void readVFATblock(std::queue<uint32_t>& m_dataque);
      uint32_t dat10,dat11, dat20,dat21, dat30,dat31, dat40,dat41;
      uint32_t BX;
      uint16_t bcn, evn, chipid, vfatcrc;
      uint16_t b1010, b1100, b1110;
      uint8_t  flags;


      static const int MaxVFATS = 24; // was 32 ???
      static const int MaxERRS  = 4095; // should this also be 24? Or we can accomodate full GLIB FIFO of bad blocks belonging to the same event?
      
      std::unique_ptr<GEMslotContents> slotInfo;
      
      log4cplus::Logger m_gemLogger;
      gem::hw::glib::HwGLIB* glibDevice_;
      std::string outFileName_;
      std::string slotFileName_;
      std::string errFileName_;
      std::string outputType_;

      // queue safety
      mutable gem::utils::Lock m_queueLock;
      // The main data flow
      std::queue<uint32_t> dataque;



      
      // Online histograms
      gemOnlineDQM* m_gemOnlineDQM;
      /*
       * Counter all in one
       *   [0] VFAT's Blocks Counter
       *   [1] Events Counter
       *   [2] VFATs counter per last event
       *   [3] Good Events counter
       *   [4] Bad Events counter
       */
      uint32_t counter_[5];

      // VFAT's Blocks Counter     
      uint64_t vfat_;

      // Events Counter     
      uint64_t event_;
         
      // VFATs counter per event
      int sumVFAT_;
      
      int16_t scanParam;


    };
  }
}
#endif
