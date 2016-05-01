#ifndef GEM_HW_GEMHWDEVICE_H
#define GEM_HW_GEMHWDEVICE_H

#include <iomanip>

//#include "xdata/InfoSpace.h"
#include "xdata/InfoSpaceFactory.h"

#include "xdata/String.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedInteger32.h"
#include "toolbox/string.h"
#include "toolbox/mem/Reference.h"
#include "toolbox/mem/Pool.h"

#include "uhal/uhal.hpp"
#include "uhal/Utilities.hpp"

#include "gem/utils/GEMLogging.h"
#include "gem/utils/GEMRegisterUtils.h"

#include "gem/utils/Lock.h"
#include "gem/utils/LockGuard.h"

#include "gem/hw/exception/Exception.h"

typedef uhal::exception::exception uhalException;

// for multiple reads with single dispatch with named registers
typedef std::pair<std::string, uint32_t> register_pair;
typedef std::vector<register_pair>       register_pair_list;

// for multiple reads with single dispatch with addressed registers
typedef std::pair<uint32_t, uint32_t>           addressed_register_pair;
typedef std::vector<addressed_register_pair>    addressed_register_pair_list;

// for multiple reads with single dispatch with addressed and masked registers
typedef std::pair<std::pair<uint32_t, uint32_t>, uint32_t> masked_register_pair;
typedef std::vector<masked_register_pair>                  masked_register_pair_list;

typedef std::pair<std::string, uhal::ValWord<uint32_t> > register_value;
typedef std::vector<register_value>                      register_val_list;


namespace uhal {
  class HwInterface;
}

namespace gem {
  namespace hw {

    class GEMHwDevice
    {

    public:
      /* IPBus transactions still have some problems in the firmware
         so it helps to retry a few times in the case of a failure
         that is recognized
      */
      static const unsigned MAX_IPBUS_RETRIES = 25;

      /**
       * @struct OpticalLinkStatus
       * @brief This structure stores retrieved counters related to the GTX link
       * @var OpticalLinkStatus::TRK_Errors
       * TRK_Errors is a counter for the number of errors on the tracking data link
       * @var OpticalLinkStatus::TRG_Errors
       * TRG_Errors is a counter for the number of errors on the trigger data link
       * @var OpticalLinkStatus::Data_Packets
       * Data_Packets is a counter for the number of data packets transferred on the tracking data link
       */
      typedef struct OpticalLinkStatus {
        uint32_t TRK_Errors  ;
        uint32_t TRG_Errors  ;
        uint32_t Data_Packets;

      OpticalLinkStatus() :
        TRK_Errors(0),TRG_Errors(0),Data_Packets(0) {};
        void reset() {
          TRK_Errors=0; TRG_Errors=0;Data_Packets=0;
          return; };
      } OpticalLinkStatus;

      /**
       * @struct DeviceErrors
       * @brief This structure stores retrieved counters related to the IPBus transaction errors
       * @var DeviceErrors::BadHeader
       * BadHeader is a counter for the number times the IPBus transaction returned a bad header
       * @var DeviceErrors::ReadError
       * ReadError is a counter for the number read transaction errors
       * @var DeviceErrors::Timeout
       * Timeout is a counter for the number for the number of timeouts
       * @var DeviceErrors::ControlHubErr
       * ControlHubErr is a counter for the number control hub errors encountered
       */
      typedef struct DeviceErrors {
        int BadHeader    ;
        int ReadError    ;
        int Timeout      ;
        int ControlHubErr;

      DeviceErrors() : BadHeader(0),ReadError(0),Timeout(0),ControlHubErr(0) {};
        void reset() { BadHeader=0; ReadError=0; Timeout=0; ControlHubErr=0; return; };
      } DeviceErrors;

      typedef std::pair<uint8_t, OpticalLinkStatus>  linkStatus;
      //typedef std::vector<linkStatus>                linkStatus;

      /**
       * GEMHwDevice constructor
       * @param deviceName string to put into the logger
       */
      GEMHwDevice(std::string const& deviceName);

      GEMHwDevice(std::string const& deviceName,
                  std::string const& connectionFile);

      GEMHwDevice(std::string const& deviceName,
                  std::string const& connectionURI,
                  std::string const& addressTable);

      GEMHwDevice(std::string const& deviceName,
                  uhal::HwInterface& uhalDevice);

      virtual ~GEMHwDevice();

      virtual bool isHwConnected() { return p_gemHW != 0; };

      /**
       * Generic read/write functions or IPBus devices
       * operation will be the same for the GLIB, MP7, VFAT2/3,
       * and AMC13 ( we should use the already defined AMC13, rather than write our own,
       * unless there are GEM specific functions we need to implement)
       */

      /**
       * readReg(std::string const& regName)
       * @param regName name of the register to read
       * @retval returns the 32 bit unsigned value in the register
       */
      uint32_t readReg( std::string const& regName);

      /**
       * readReg(uint32_t const& regAddr)
       * @param regAddr address of the register to read
       * @retval returns the 32 bit unsigned value in the register
       */
      uint32_t readReg( uint32_t const& regAddr);

      /**
       * readReg(uint32_t const& regAddr)
       * @param regAddr address of the register to read
       * @param regMask mask of the register to read
       * @retval returns the 32 bit unsigned value in the register
       */
      uint32_t readReg( uint32_t const& regAddr, uint32_t const& regMask);

      /**
       * readReg(std::string const& regPrefix, std::string const& regName)
       * @param regPrefix prefix in the address table, possibly root nodes
       * @param regName name of the register to read from the address table
       * @retval returns the 32 bit unsigned value
       */
      uint32_t readReg( const std::string &regPrefix,
                        const std::string &regName) {
        return readReg(regPrefix+"."+regName); };

      /**
       * readMaskedAddress(std::string const& regName)
       * @param regName name of the register to read
       * @retval returns the 32 bit unsigned value in the register
       */
      uint32_t readMaskedAddress( std::string const& regName);

      /**
       * readRegs( register_pair_list &regList)
       * read list of registers in a single transaction (one dispatch call)
       * into the supplied vector regList
       * @param regList list of register name and uint32_t value to store the result
       */
      void     readRegs( register_pair_list &regList);

      /**
       * readRegs( addressed_register_pair_list &regList)
       * read list of registers in a single transaction (one dispatch call)
       * into the supplied vector regList
       * @param regList list of register address and uint32_t value to store the result
       */
      void     readRegs( addressed_register_pair_list &regList);

      /**
       * readRegs( masked_register_pair_list &regList)
       * read list of registers in a single transaction (one dispatch call)
       * into the supplied vector regList
       * @param regList list of register address/mask pair and uint32_t value to store the result
       */
      void     readRegs( masked_register_pair_list &regList);

      /**
       * writeReg(std::string const& regName, uint32_t const val)
       * @param regName name of the register to read
       * @param val value to write to the register
       */
      void     writeReg( std::string const& regName, uint32_t const val);

      /**
       * writeReg(uint32_t const& regAddr, uint32_t const val)
       * @param regAddr address of the register to read
       * @param val value to write to the register
       */
      void     writeReg( uint32_t const& regAddr, uint32_t const val);

      /**
       * writeReg(std::string const& regPrefux, std::string const& regName, uint32_t const val)
       * @param regPrefix prefix in the address table to the register
       * @param regName name of the register to write to
       * @param val value to write to the register
       */
      void     writeReg( const std::string &regPrefix,
                         const std::string &regName,
                         uint32_t const val) {
        return writeReg(regPrefix+"."+regName, val); };

      /**
       * writeRegs(register_pair_list const& regList)
       * write list of registers in a single transaction (one dispatch call)
       * using the supplied vector regList
       * @param regList std::vector of a pairs of register names and values to write
       */
      void     writeRegs(register_pair_list const& regList);

      /**
       * writeRegs(register_pair_list const& regList)
       * write single value to a list of registers in a single transaction
       * (one dispatch call) using the supplied vector regList
       * @param regList list of registers to write a value to
       * @param regValue uint32_t value to write to the list of registers
       */
      void     writeValueToRegs(std::vector<std::string> const& regList, uint32_t const& regValue);

      /**
       * zeroReg(std::string const& regName)
       * write zero to a single register
       * @param regName register to zero
       */
      void     zeroReg(  std::string const& regName) { writeReg(regName,0); };

      /**
       * zeroRegs(std::vector<std::string> const& regNames)
       * write zero to a list of registers in a single transaction (one dispatch call)
       * using the supplied vector regNames
       * @param regNames registers to zero
       */
      void     zeroRegs( std::vector<std::string> const& regNames);

      /**
       * readBlock(std::string const& regName)
       * read from a memory block
       * @param regName fixed size memory block to read from
       */
      std::vector<uint32_t> readBlock( std::string const& regName);

      /**
       * readBlock(std::string const& regName, size_t const nWords)
       * read from a memory block
       * @param regName memory block to read from
       * @param nWords size of the memory block to read
       * @retval returns a vector of 32 bit unsigned value
       */
      std::vector<uint32_t> readBlock( std::string const& regName,
                                       size_t      const& nWords);

      uint32_t readBlock(std::string const& regName, uint32_t* buffer, size_t const& nWords);
      uint32_t readBlock(std::string const& regName, std::vector<toolbox::mem::Reference*>& buffer,
                         size_t const& nWords);

      /**
       * writeBlock(std::string const& regName, std::vector<uint32_t> const values)
       * write to a memory block
       * @param regName memory block to write to
       * @param values list of 32-bit words to write into the memory block
       */
      void writeBlock(std::string           const& regName,
                      std::vector<uint32_t> const values);

      /**
       * zeroBlock( std::string const& regName)
       * write zeros to a block of memory
       * @param regName block or memory to zero
       */
      void zeroBlock( std::string const& regName);

      /**
       * readFIFO(std::string const& regName)
       * read from a FIFO
       * @param regName fixed size memory block to read from
       */
      std::vector<uint32_t> readFIFO( std::string const& regName);
      //size_t readFIFO( std::string const& regName, size_t nWords, uint32_t* buffer); /*hcal style */

      /**
       * readFIFO(std::string const& regName, size_t const nWords)
       * read from a FIFO
       * @param regName FIFO to read from
       * @param nWords number of words to read from the FIFO
       * @retval returns a vector of 32 bit unsigned value
       */
      std::vector<uint32_t> readFIFO( std::string const& regName,
                                      size_t      const& nWords);

      /**
       * writeFIFO(std::string const& regName, std::vector<uint32_t> const values)
       * write to a FIFO
       * @param regName FIFO to write to
       * @param values list of 32-bit words to write into the FIFO
       */
      void writeFIFO(std::string           const& regName,
                     std::vector<uint32_t> const values);

      /**
       * zeroFIFO( std::string const& regName)
       * reset a FIFO
       * @param regName FIFO to zero
       */
      void zeroFIFO( std::string const& regName);


      // These methods provide access to the member variables
      // specifying the uhal address table name and the IPbus protocol
      // version.
      //getters
      const std::string getControlHubIPAddress()  const { return m_controlHubIPAddress;};
      const std::string getDeviceIPAddress()      const { return m_deviceIPAddress;    };
      const std::string getIPBusProtocolVersion() const { return m_ipBusProtocol;      };
      const std::string getAddressTableFileName() const { return m_addressTable;       };

      const std::string getDeviceBaseNode()       const { return m_deviceBaseNode; };
      const std::string getDeviceID()             const { return m_deviceID;       };

      const uint32_t getControlHubPort() const { return m_controlHubPort;};
      const uint32_t getIPBusPort()      const { return m_ipBusPort;     };

      //setters, should maybe be private/protected? defeats the purpose?
      void setControlHubIPAddress(std::string const& ipAddress) {
        m_controlHubIPAddress = ipAddress; };
      void setIPBusProtocolVersion(std::string const& version) {
        m_ipBusProtocol = version; };
      void setDeviceIPAddress(std::string const& deviceIPAddr) {
        m_deviceIPAddress = deviceIPAddr; };
      void setAddressTableFileName(std::string const& name) {
        m_addressTable = "file://${GEM_ADDRESS_TABLE_PATH}/"+name; };

      void setDeviceBaseNode(std::string const& deviceBase) {
        m_deviceBaseNode = deviceBase; };
      void setDeviceID(std::string const& deviceID) {
        m_deviceID = deviceID; };

      void setControlHubPort(uint32_t const& port) {
        m_controlHubPort = port; };
      void setIPBusPort(uint32_t const& port) {
        m_ipBusPort = port; };

      uhal::HwInterface& getGEMHwInterface() const;

      void updateErrorCounters(std::string const& errCode);

      virtual std::string printErrorCounts() const;

      /**
       * @brief performs a general reset of the GLIB
       */
      virtual void generalReset();

      /**
       * @brief performs a reset of the GLIB counters
       */
      virtual void counterReset();

      /**
       * @brief performs a reset of the GLIB link
       * @param link is the link to perform the reset on
       */
      virtual void linkReset(uint8_t const& link);

      DeviceErrors m_ipBusErrs;

      bool b_is_connected;

      xdata::InfoSpace* getHwInfoSpace() { return p_hwCfgInfoSpace; };

    protected:
      std::shared_ptr<uhal::ConnectionManager> p_gemConnectionManager;
      std::shared_ptr<uhal::HwInterface> p_gemHW;

      xdata::InfoSpace *p_hwCfgInfoSpace;       /* Infospace for configuration values */

      log4cplus::Logger m_gemLogger;

      mutable gem::utils::Lock m_hwLock;

      void setParametersFromInfoSpace();
      void setup(std::string const& deviceName);

    private:
      std::string m_controlHubIPAddress;
      std::string m_addressTable;
      std::string m_ipBusProtocol;
      std::string m_deviceIPAddress;

      std::string m_deviceBaseNode;
      std::string m_deviceID;

      uint32_t m_controlHubPort;
      uint32_t m_ipBusPort;

      //infospace im(ex)portables
      xdata::String xs_controlHubIPAddress;
      xdata::String xs_deviceIPAddress;
      xdata::String xs_ipBusProtocol;
      xdata::String xs_addressTable;

      xdata::UnsignedInteger32 xs_controlHubPort;
      xdata::UnsignedInteger32 xs_ipBusPort;

      bool knownErrorCode(std::string const& errCode) const;

      //std::string registerToChar(uint32_t value) const;
    };  // class GEMHwDevice
  }  // namespace gem::hw
}  // namespace gem

#endif  // GEM_HW_GEMHWDEVICE_H
