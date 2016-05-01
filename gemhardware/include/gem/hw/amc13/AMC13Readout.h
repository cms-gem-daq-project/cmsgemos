#ifndef GEM_HW_AMC13_AMC13READOUT_H
#define GEM_HW_AMC13_AMC13READOUT_H

#include <gem/readout/GEMReadoutApplication.h>
#include <gem/hw/amc13/exception/Exception.h>

namespace amc13 {
  class AMC13;
}

namespace gem {
  namespace hw {
    namespace amc13 {

      typedef std::shared_ptr< ::amc13::AMC13>  amc13_shared_ptr;

      class AMC13Readout: public gem::readout::GEMReadoutApplication
        {
        public:
          XDAQ_INSTANTIATOR();

          AMC13Readout(xdaq::ApplicationStub* s)
            throw (xdaq::exception::Exception);

          virtual ~AMC13Readout();

        protected:
          virtual void actionPerformed(xdata::Event& event);

          //state transitions
          virtual void initializeAction() throw (gem::hw::amc13::exception::Exception);
          virtual void configureAction()  throw (gem::hw::amc13::exception::Exception);
          virtual void startAction()      throw (gem::hw::amc13::exception::Exception);
          virtual void pauseAction()      throw (gem::hw::amc13::exception::Exception);
          virtual void resumeAction()     throw (gem::hw::amc13::exception::Exception);
          virtual void stopAction()       throw (gem::hw::amc13::exception::Exception);
          virtual void haltAction()       throw (gem::hw::amc13::exception::Exception);
          virtual void resetAction()      throw (gem::hw::amc13::exception::Exception);

          virtual int readout(unsigned int expected, unsigned int* eventNumbers, std::vector< ::toolbox::mem::Reference* >& data);

          int dumpData();

        private:
          amc13_shared_ptr p_amc13;
          xdata::String  m_cardName;
          xdata::Integer m_crateID, m_slot;
      };
    }  // namespace gem::hw::amc13
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_AMC13_AMC13READOUT_H
