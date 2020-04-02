/** @file GEMHwDevice.h */

#ifndef GEM_HW_GEMHWDEVICE_H
#define GEM_HW_GEMHWDEVICE_H

#include <iomanip>
#include <memory>

// can we decouple completely from these?
#include "toolbox/string.h"

#include "xhal/client/XHALInterface.h"
#include "xhal/common/rpc/call.h"

#include "ctp7_modules/common/hw_constants.h"
#include "ctp7_modules/common/utils.h"

#include "gem/hw/utils/GEMCrateUtils.h"
#include "gem/hw/devices/exception/Exception.h"

/// TODO: would be good to decouple from utils, maybe migrate RegisterUtils to gem::hw::utils
#include "gem/utils/GEMLogging.h"
#include "gem/utils/GEMRegisterUtils.h"
// TODO: This is a generically useful lock, both inside and outside of the HW, but is it used in the HW yet?
#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

namespace gem {
  namespace hw {

    class GEMHwDevice : public xhal::client::XHALInterface
    {

    public:
      /**
       * @brief GEMHwDevice constructor based on hostname and connection file
       *
       * @param deviceName card name (IP resolveable)
       */
      GEMHwDevice(std::string const& deviceName);

      virtual ~GEMHwDevice();

      /* FIXME
       * make pure virtual, non-virtual?
       * check RPC connection?
       */
      virtual bool isHwConnected() { return true; };

      ///////////////////////////////////////////////////////////////////////////////////////
      //****************Methods implemented for convenience on uhal devices****************//
      ///////////////////////////////////////////////////////////////////////////////////////
      //FIXME review docs w.r.t. uhal
      /**
       * @defgroup uhalwrappers Generic read/write wrapper functions on uhal/IPBus devices.
       * The operations will be the same for the GLIB, CTP7, OptoHybrid3 and AMC13
       * (we should use the already defined AMC13, rather than write our own,
       * unless there are GEM specific functions that need to be implemented)
       */

      /**
       * @ingroup uhalwrappers
       * @brief read from a register identified by a register name
       *
       * @usage FILLME
       *
       * @param regName name of the register to read
       *
       * @retval returns the 32 bit unsigned value in the register
       */
      uint32_t readReg(std::string const& regName);

      /**
       * @ingroup uhalwrappers
       * @brief write to a register identified by a register name
       *
       * @usage FILLME
       *
       * @param regName name of the register to read
       * @param val value to write to the register
       */
      void     writeReg(std::string const& regName, uint32_t const val);

      // These methods provide access to the member variables
      // specifying the `uhal` address table name and the IPbus protocol
      // version.
      /**
         FIXME getters
      */
      const std::string getDeviceID()             const { return m_deviceID;       };

      /**
         FIXME setters, should maybe be private/protected? defeats the purpose?
      */
      void setDeviceID(std::string const& deviceID)         { m_deviceID = deviceID; };

      ///////////////////////////////////////////////////////////////////////////////////////
      //******************* Generic properties of the GEMHwDevice object ******************//
      ///////////////////////////////////////////////////////////////////////////////////////

      std::string getLoggerName() const { return m_gemLogger.getName(); };

      /**
       * @brief performs a general reset of the GLIB
       *
       * @usage FILLME
       *
       */
      virtual void generalReset()=0;

      /**
       * @brief performs a reset of the counters
       *
       * @usage FILLME
       *
       */
      virtual void counterReset()=0;

      /**
       * @brief performs a reset of the GLIB link
       *
       * @usage FILLME
       *
       * @param link is the link to perform the reset on
       */
      virtual void linkReset(uint8_t const& link)=0;

      bool b_is_connected;

    protected:
      log4cplus::Logger m_gemLogger;

      mutable gem::utils::Lock m_hwLock;

      /**
       * @brief loads the necessary modules into the RPC module manager
       * @details Provides basic read/write functionality, reimplemented in daughter classes
       *
       * @usage FILLME
       *
       * @param should reconnect in the case of an already connected manager
       */
      virtual void connectRPC(bool reconnect=false);


      /**
       * @brief Performs basic setup for the device
       * sets connection details (OBSOLETE)
       * sets logging level to Error
       * Not inherited
       **/
      void setup(std::string const& deviceName);

      /**
       * @brief Extracts the device parameters from the device name
       *
       * @param deviceName should be of the format: gem-shelfXX-amcYY
       *        optionally including -optohybridZZ
       * @param index selects which of XX, YY, ZZ to return
       **/
      static uint8_t extractDeviceID(std::string const& deviceName, uint8_t const& index);
      
    private:
      // Do Not use default constructor. GEMHwDevice object should only be made using
      // either connection file method or with a list of URIs and Address Tables
      GEMHwDevice();

      // Prevent copying of GEMHwDevice objects
      GEMHwDevice( const GEMHwDevice& other) ; // prevents construction-copy
      GEMHwDevice& operator=( const GEMHwDevice&) ; // prevents copying

      std::string m_deviceID;       ///< FILLME

      // All GEMHwDevice objects should have these properties
      uint8_t m_crate;  ///< Crate number the AMC is housed in
      uint8_t m_slot;   ///< Slot number in the uTCA shelf the AMC is sitting in

    };  // class GEMHwDevice
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_GEMHWDEVICE_H
