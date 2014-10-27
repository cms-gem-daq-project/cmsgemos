#ifndef gem_hw_GEMHwDevice_h
#define gem_hw_GEMHwDevice_h

#include "xdaq/Application.h"
#include "xdata/ActionListener.h"

#include "gem/hw/exception/Exception.h"

#include "uhal/uhal.hpp"

#define MAX_VFAT_RETRIES 25

typedef uhal::exception::exception uhalException;

namespace uhal {
  class HwInterface;
}

namespace xdaq {
  class Application;;
}

namespace gem {
  
  namespace hw {
    
    class GEMHwDevice
      {
      public:
	GEMHwDevice(xdaq::Application* xdaqApp);

	virtual ~GEMHwDevice();
	
	virtual void connectDevice();
	virtual void releaseDevice();
	//virtual void initDevice();
	//virtual void configureDevice();
	virtual void enableDevice();
	//virtual void disableDevice();
	//virtual void startDevice();
	//virtual void stopDevice();
	//virtual void pauseDevice();
	//virtual void resumeDevice();
	//virtual void haltDevice();
	
	bool isGEMHwDeviceConnected() const { return gemHWP_ != 0; };
	
	std::string getBoardID()   const;
	std::string getSystemID()  const;
	std::string getIPAddress() const;
	std::string getFirmwareVer( std::string const& fwRegPrefix="glib_regs.sysregs") const;
	std::string getFirmwareDate(std::string const& fwRegPrefix="glib_regs.sysregs") const;

	/**
	 *Generic read/write functions or IPBus devices
	 *operation will be the same for the GLIB, MP7, VFAT2/3, 
	 * and AMC13 ( we should use the already defined AMC13, rather than write our own,
	 unless there are GEM specific functions we need to implement)
	 */
	//perform a single read transaction
	virtual uint32_t readReg(  std::string const& regName) const;
	//read list of registers in a single transaction (one dispatch call) into the supplied vector regList
	virtual void     readRegs( std::vector<std::pair<std::string, uint32_t> > &regList) const;

	//perform a single write transaction
	virtual void     writeReg( std::string const& regName, uint32_t const) const;
	//write list of registers in a single transaction (one dispatch call) using the supplied vector regList
	virtual void     writeRegs(std::vector<std::pair<std::string, uint32_t> > const& regList) const;
	//write single value to a list of registers in a single transaction (one dispatch call) using the supplied vector regList
	virtual void     writeValueToRegs(std::vector<std::string> const& regList, uint32_t const& regValue) const;

	//write zero to a single register
	virtual void     zeroReg(  std::string const& regName) const;
	//write zero to a list of registers in a single transaction (one dispatch call) using the supplied vector regNames
	virtual void     zeroRegs( std::vector<std::string> const& regNames) const;

	virtual std::vector<uint32_t> readBlock( std::string const& regName) const;
	virtual std::vector<uint32_t> readBlock( std::string const& regName,
					 size_t const nWords) const;
	virtual void writeBlock(std::string const& regName,
				std::vector<uint32_t> const values) const;
	virtual void zeroBlock( std::string const& regName) const {
	  writeReg(regName, 0); }


	// These methods provide access to the member variables
	// specifying the uhal address table name and the IPbus protocol
	// version.
	std::string getAddressTableFileName() const { return addressTable_;   };
	std::string getIPbusProtocolVersion() const { return ipbusProtocol_;  };
	std::string getDeviceBaseNode()       const { return deviceBaseNode_; };
	std::string getDeviceID()             const { return deviceID_;       };

	void setAddressTableFileName(std::string const& name) {
	  addressTable_ = "file://${BUILD_HOME}/data/"+name; };
	void setIPbusProtocolVersion(std::string const& version) {
	  ipbusProtocol_ = version; };
	void setDeviceBaseNode(std::string const& deviceBase) {
	  deviceBaseNode_ = deviceBase; };
	void setDeviceID(std::string const& deviceID) {
	  deviceID_ = deviceID; };
	
	uhal::HwInterface& getGEMHwInterface() const;

      protected:
	uhal::ConnectionManager *gemConnectionManager;
	log4cplus::Logger logGEMHw_;
	uhal::HwInterface *gemHWP_;
		
      private:
	std::string addressTable_;
	std::string ipbusProtocol_;
	std::string deviceBaseNode_;
	std::string deviceID_;
	
	//std::string registerToChar(uint32_t value);
	
	std::string uint32ToString(uint32_t const val) const {
	  std::stringstream res;
	  res << char((val & uint32_t(0xff000000)) / 16777216);
	  res << char((val & uint32_t(0x00ff0000)) / 65536);
	  res << char((val & uint32_t(0x0000ff00)) / 256);
	  res << char((val & uint32_t(0x000000ff)));
	  return res.str(); }


      }; //end class GEMHwDevice

  } //end namespace gem::hw

} //end namespace gem
#endif
