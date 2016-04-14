#ifndef gem_hw_glib_AMC13Readout_h
#define gem_hw_glib_AMC13Readout_h

#include <gem/base/GEMReadoutApplication.h>
#include <gem/hw/amc13/exception/Exception.h>

namespace gem {
  namespace hw {
    namespace amc13 {
      class HwAMC13;

      typedef std::shared_ptr<HwAMC13>  amc13_shared_ptr;

      class AMC13Readout: public gem::base::GEMReadoutApplication
      {
        public:
          XDAQ_INSTANTIATOR();

          static const uint32_t kUPDATE;
          static const uint32_t kUPDATE7;

          AMC13Readout(xdaq::ApplicationStub* s);
          virtual ~AMC13Readout();
        protected:
          virtual void actionPerformed(xdata::Event& event);
	  
          //state transitions
          virtual void initializeAction() throw (gem::hw::glib::exception::Exception);
          virtual void configureAction()  throw (gem::hw::glib::exception::Exception);
          virtual void startAction()      throw (gem::hw::glib::exception::Exception);
          virtual void pauseAction()      throw (gem::hw::glib::exception::Exception);
          virtual void resumeAction()     throw (gem::hw::glib::exception::Exception);
          virtual void stopAction()       throw (gem::hw::glib::exception::Exception);
          virtual void haltAction()       throw (gem::hw::glib::exception::Exception);
          virtual void resetAction()      throw (gem::hw::glib::exception::Exception);
          void dumpData();

        private:
          uint32_t m_runType;
          uint32_t m_runParams;
          amc13_shared_ptr p_amc13;
          std::string m_outFileName;
      };
    }
  }
}
