/** @file AMC13Readout.h */

#ifndef GEM_HW_AMC13_AMC13READOUT_H
#define GEM_HW_AMC13_AMC13READOUT_H

#include <gem/readout/GEMReadoutApplication.h>
#include <gem/hw/amc13/exception/Exception.h>
#include <ctime>

namespace amc13 {
  class AMC13;
}

typedef ::amc13::Exception::exBase amc13Exception;

namespace gem {
  namespace hw {
    namespace amc13 {

      typedef std::shared_ptr< ::amc13::AMC13>  amc13_shared_ptr;

      class AMC13Readout: public gem::readout::GEMReadoutApplication
        {
        public:
          XDAQ_INSTANTIATOR();

          AMC13Readout(xdaq::ApplicationStub* s);

          virtual ~AMC13Readout();

        protected:
          virtual void actionPerformed(xdata::Event& event);

          //state transitions
          virtual void initializeAction();
          virtual void configureAction();
          virtual void startAction();
          virtual void pauseAction();
          virtual void resumeAction();
          virtual void stopAction();
          virtual void haltAction();
          virtual void resetAction();

          virtual int readout(unsigned int expected, unsigned int* eventNumbers, std::vector< ::toolbox::mem::Reference* >& data);

          int dumpData();

        private:
          amc13_shared_ptr p_amc13;
          xdata::String  m_cardName;
          xdata::Integer m_crateID, m_slot;
          int cnt;
          int nwrote_global;
          std::clock_t m_start;
          double m_duration;
      };
    }  // namespace gem::hw::amc13
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_AMC13_AMC13READOUT_H
