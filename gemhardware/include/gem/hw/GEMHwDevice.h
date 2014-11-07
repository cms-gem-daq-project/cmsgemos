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
	typedef struct DeviceErrors {
	  int badHeader_;
	  int readError_;
	  int timeouts_;
	  int controlHubErr_;
	} DeviceErrors;
	
      public:
	GEMHwDevice(xdaq::Application* xdaqApp);

	virtual ~GEMHwDevice();
	
	virtual void connectDevice();
	virtual void releaseDevice();
	//virtual void initDevice();
	virtual void configureDevice()=0;
	virtual void enableDevice();
	//virtual void disableDevice();
	//virtual void startDevice();
	//virtual void stopDevice();
	//virtual void pauseDevice();
	//virtual void resumeDevice();
	//virtual void haltDevice();
	
	bool isGEMHwDeviceConnected() { return gemHWP_ != 0; };
	
	virtual std::string getBoardID()   ;
	virtual std::string getSystemID()  ;
	virtual std::string getIPAddress() ;
	virtual std::string getFirmwareVer( std::string const& fwRegPrefix="glib_regs.sysregs") ;
	virtual std::string getFirmwareDate(std::string const& fwRegPrefix="glib_regs.sysregs") ;

	/**
	 *Generic read/write functions or IPBus devices
	 *operation will be the same for the GLIB, MP7, VFAT2/3, 
	 * and AMC13 ( we should use the already defined AMC13, rather than write our own,
	 unless there are GEM specific functions we need to implement)
	 */
	//perform a single read transaction
	virtual uint32_t readReg(  std::string const& regName);
	//read list of registers in a single transaction (one dispatch call) into the supplied vector regList
	virtual void     readRegs( std::vector<std::pair<std::string, uint32_t> > &regList);

	//perform a single write transaction
	virtual void     writeReg( std::string const& regName, uint32_t const);
	//write list of registers in a single transaction (one dispatch call) using the supplied vector regList
	virtual void     writeRegs(std::vector<std::pair<std::string, uint32_t> > const& regList);
	//write single value to a list of registers in a single transaction (one dispatch call) using the supplied vector regList
	virtual void     writeValueToRegs(std::vector<std::string> const& regList, uint32_t const& regValue);

	//write zero to a single register
	virtual void     zeroReg(  std::string const& regName);
	//write zero to a list of registers in a single transaction (one dispatch call) using the supplied vector regNames
	virtual void     zeroRegs( std::vector<std::string> const& regNames);

	virtual std::vector<uint32_t> readBlock( std::string const& regName);
	virtual std::vector<uint32_t> readBlock( std::string const& regName,
					 size_t const nWords);
	virtual void writeBlock(std::string const& regName,
				std::vector<uint32_t> const values);
	virtual void zeroBlock( std::string const& regName) {
	  writeReg(regName, 0); }


	// These methods provide access to the member variables
	// specifying the uhal address table name and the IPbus protocol
	// version.
	std::string getAddressTableFileName() { return addressTable_;   };
	std::string getIPbusProtocolVersion() { return ipbusProtocol_;  };
	std::string getDeviceBaseNode()       { return deviceBaseNode_; };
	std::string getDeviceID()             { return deviceID_;       };

	void setAddressTableFileName(std::string const& name) {
	  addressTable_ = "file://${BUILD_HOME}/data/"+name; };
	void setIPbusProtocolVersion(std::string const& version) {
	  ipbusProtocol_ = version; };
	void setDeviceBaseNode(std::string const& deviceBase) {
	  deviceBaseNode_ = deviceBase; };
	void setDeviceID(std::string const& deviceID) {
	  deviceID_ = deviceID; };
	
	uhal::HwInterface& getGEMHwInterface();// ;
	
	void updateErrorCounters(std::string const& errCode);
	
	DeviceErrors ipBusErrs;
	
	std::string printErrorCounts();
	
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
	
	std::string uint32ToString(uint32_t const val) {
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
