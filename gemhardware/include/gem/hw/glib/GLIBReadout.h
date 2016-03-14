#ifndef gem_hw_glib_GLIBReadout_h
#define gem_hw_glib_GLIBReadout_h

#include <gem/base/GEMReadoutApplication.h>
#include <gem/readout/GEMDataAMCformat.h>

namespace gem {
  
  namespace readout {
    struct GEMDataAMCformat;
  }
  
  namespace hw {
    namespace glib {
      class HwGLIB;

      typedef std::shared_ptr<HwGLIB>  glib_shared_ptr;

      class GLIBReadout: public gem::base::GEMReadoutApplication
        {
        public:
          XDAQ_INSTANTIATOR();
	  
          GLIBReadout(xdaq::ApplicationStub* s, glib_shared_ptr glib);

          virtual ~GLIBReadout();
          
          /** Recieve a message to update the scan parameters
           * @param   message
           * message should contain 
           *  - the scan type
           *  - the value(s) to store in the RunParams bits in the data format
           * @returns xoap::MessageReference
           */
          xoap::MessageReference updateScanParameters(xoap::MessageReference message) throw (xoap::exception::Exception);

        protected:
          
        private:

          
        }; // end class GLIBReadout
    } //end namespace gem::hw::glib
  } //end namespace gem::hw
} //end namespace gem
#endif
