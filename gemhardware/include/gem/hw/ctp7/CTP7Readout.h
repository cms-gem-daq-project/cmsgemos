/** @file CTP7Readout.h */

#ifndef GEM_HW_CTP7_CTP7READOUT_H
#define GEM_HW_CTP7_CTP7READOUT_H

#include "gem/readout/GEMReadoutApplication.h"
#include "gem/readout/GEMDataAMCformat.h"
#include "gem/hw/ctp7/exception/Exception.h"

namespace gem {
  namespace readout {
    struct GEMDataAMCformat;
  }

  namespace hw {
    namespace ctp7 {
      class HwCTP7;

      typedef std::shared_ptr<HwCTP7>  ctp7_shared_ptr;

      class CTP7Readout: public gem::readout::GEMReadoutApplication
        {
        public:
          XDAQ_INSTANTIATOR();

          static const uint32_t kUPDATE;
          static const uint32_t kUPDATE7;

          CTP7Readout(xdaq::ApplicationStub* s);
          //CTP7Readout(xdaq::ApplicationStub* s, ctp7_shared_ptr ctp7);

          virtual ~CTP7Readout();

          void ScanRoutines(uint8_t latency, uint8_t VT1, uint8_t VT2);

          /** Recieve a message to update the scan parameters
           * @param   message
           * message should contain
           *  - the scan type
           *  - the value(s) to store in the RunParams bits in the data format
           * @returns xoap::MessageReference
           */
          xoap::MessageReference updateScanParameters(xoap::MessageReference message) throw (xoap::exception::Exception);

          // reply to a query about the queue depth, better to just export the queue depth into the infospace?
          //xoap::MessageReference queueDepth(xoap::MessageReference message) throw (xoap::exception::Exception);

        protected:

          virtual void actionPerformed(xdata::Event& event);

          //state transitions
          virtual void initializeAction() throw (gem::hw::ctp7::exception::Exception);
          virtual void configureAction()  throw (gem::hw::ctp7::exception::Exception);
          virtual void startAction()      throw (gem::hw::ctp7::exception::Exception);
          virtual void pauseAction()      throw (gem::hw::ctp7::exception::Exception);
          virtual void resumeAction()     throw (gem::hw::ctp7::exception::Exception);
          virtual void stopAction()       throw (gem::hw::ctp7::exception::Exception);
          virtual void haltAction()       throw (gem::hw::ctp7::exception::Exception);
          virtual void resetAction()      throw (gem::hw::ctp7::exception::Exception);

          virtual int readout(unsigned int expected, unsigned int* eventNumbers, std::vector< ::toolbox::mem::Reference* >& data);

          uint32_t* dumpData( uint8_t const& mask );

          uint32_t* selectData(uint32_t counter[5]);

          uint32_t* getCTP7Data(uint8_t const& link, uint32_t counter[5]);

          uint32_t* GEMEventMaker(uint32_t counter[5]);

          void GEMevSelector(const uint32_t& ES);

          void GEMfillHeaders(uint32_t const& BC, uint32_t const& BX,
                              gem::readout::GEMDataAMCformat::GEMData& gem,
                              gem::readout::GEMDataAMCformat::GEBData& geb);

          bool VFATfillData(/*int const& islot, */gem::readout::GEMDataAMCformat::GEBData& geb);

          void GEMfillTrailers(gem::readout::GEMDataAMCformat::GEMData& gem,
                               gem::readout::GEMDataAMCformat::GEBData& geb);

          void writeGEMevent(std::string outFile,
                             bool const& OKprint,
                             std::string const& TypeDataFlag,
                             gem::readout::GEMDataAMCformat::GEMData& gem,
                             gem::readout::GEMDataAMCformat::GEBData& geb,
                             gem::readout::GEMDataAMCformat::VFATData& vfat);

          int queueDepth() {return m_dataque.size();}

        private:
          uint32_t m_runType;
          uint32_t m_runParams;

          ctp7_shared_ptr p_ctp7;

          // copied in from GEMDataParker
          uint32_t m_ESexp;
          bool     m_isFirst;
          //uint64_t m_ZSFlag;
          uint32_t m_contvfats;

          void readVFATblock(std::queue<uint32_t>& dataque);

          // this can't be the best way to do this...
          uint32_t dat10,dat11, dat20,dat21, dat30,dat31, dat40,dat41;
          uint32_t BX;
          uint16_t bcn, evn, chipid, vfatcrc;
          uint16_t b1010, b1100, b1110;
          uint8_t  flags;

          uint8_t m_latency, m_VT1, m_VT2;

          //why are these global and not part of the header???
          std::vector<AMCVFATData> m_vfats;
          std::vector<AMCVFATData> m_erros;
          int m_rvent;

          static const int MaxVFATS = 24; // was 32 ???
          static const int MaxERRS  = 4095; // should this also be 24? Or we can accomodate full CTP7 FIFO of bad blocks belonging to the same event?

          //std::unique_ptr<GEMslotContents> slotInfo;// time to die!!!

          //log4cplus::Logger m_gemLogger;
          //gem::hw::ctp7::HwCTP7* p_ctp7Device;
          std::string m_outFileName;
          //std::string m_slotFileName; // time to die!!!
          std::string m_errFileName;
          std::string m_outputType;

          // queue safety
          mutable gem::utils::Lock m_queueLock;
          // The main data flow
          std::queue<uint32_t> m_dataque;

          xdata::UnsignedInteger64 m_queueDepth;
          /*
           * Counter all in one
           *   [0] VFAT's Blocks counter
           *   [1] Events counter
           *   [2] VFATs counter per last event
           *   [3] Good Events counter
           *   [4] Bad Events counter
           */
          uint32_t m_counter[5];

          // VFAT's blocks counter
          uint64_t m_vfat;

          // Events counter
          uint64_t m_event;

          // VFATs counter per event
          int m_sumVFAT;

          int16_t m_scanParam;
        };  // class CTP7Readout
    }  // namespace gem::hw::ctp7
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_CTP7_CTP7READOUT_H
