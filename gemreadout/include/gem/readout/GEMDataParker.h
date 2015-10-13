#ifndef gem_readout_GEMDataParker_h
#define gem_readout_GEMDataParker_h

#include "gem/readout/GEMDataAMCformat.h"

#include "xdata/String.h"
#include <string>

#include "gem/utils/GEMLogging.h"

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
      GEMDataParker        (gem::hw::glib::HwGLIB& glibDevice, 
                            std::string const& outFileName, 
                            std::string const& errFileName, 
                            std::string const& outputType
                            );
      ~GEMDataParker() {};

      uint32_t* dumpData   ( uint8_t const& mask );
      uint32_t* selectData ( uint32_t Counter[5]
                           );
      uint32_t* getGLIBData( uint8_t const& link,
                             uint32_t Counter[5]
                           );
      uint32_t* GEMEventMaker( uint32_t Counter[5]
                             );
      void GEMevSelector   ( const  uint32_t& ES,
                             int MaxEvent = 0,
                             int MaxErr   = 0
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
      /*
      // SOAP interface, updates the header used for calibration runs
      xoap::MessageReference updateCalibHeaders(xoap::MessageReference message)
      throw (xoap::exception::Exception);
      */

    private:

      log4cplus::Logger m_gemLogger;
      gem::hw::glib::HwGLIB* glibDevice_;
      std::string outFileName_;
      std::string errFileName_;
      std::string outputType_;

      /*
       * Counter all in one
       *   [0] VFAT's Blocks Counter
       *   [1] Events Counter
       *   [2] VFATs counter per last event
       */
      uint32_t counter_[5];

      // VFAT's Blocks Counter     
      uint64_t vfat_;

      // Events Counter     
      uint64_t event_;
         
      // VFATs counter per event
      int sumVFAT_;

    };
  }
}
#endif
