#ifndef gem_hw_GEMHwDevice_h
#define gem_hw_GEMHwDevice_h

#include "xdaq/Application.h"
#include "xdata/ActionListener.h"

#include "gem/hw/exception/Exception.h"

#include "uhal/uhal.hpp"

#define DEBUG(MSG) LOG4CPLUS_DEBUG(logGEMHw_ , MSG)
#define INFO(MSG)  LOG4CPLUS_INFO(logGEMHw_  , MSG)
#define WARN(MSG)  LOG4CPLUS_WARN(logGEMHw_  , MSG)
#define ERROR(MSG) LOG4CPLUS_ERROR(logGEMHw_ , MSG)
#define FATAL(MSG) LOG4CPLUS_FATAL(logGEMHw_ , MSG)

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
	typedef struct OpticalLinkStatus {
	  uint32_t linkErrCnt     ;
	  uint32_t linkVFATI2CRec ;
	  uint32_t linkVFATI2CSnt ;
	  uint32_t linkRegisterRec;
	  uint32_t linkRegisterSnt;
	} OpticalLinkStatus;
	
	typedef struct DeviceErrors {
	  int badHeader_;
	  int readError_;
	  int timeouts_;
	  int controlHubErr_;
	} DeviceErrors;
	
	/** 
	 * GEMHwDevice constructor 
	 * @param xdaqApp pointer to xdaq::Application
	 **/
	GEMHwDevice(xdaq::Application* xdaqApp);
	/*
	GEMHwDevice(xdaq::Application* xdaqApp,
	            std::string& addressFileName,
	            std::string& ipAddress,
	            std::string& controlHubAddress,
		    );
	*/

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
	
	/**
	 *Generic read/write functions or IPBus devices
	 * operation will be the same for the GLIB, MP7, VFAT2/3, 
	 * and AMC13 ( we should use the already defined AMC13, rather than write our own,
	 * unless there are GEM specific functions we need to implement)
	 */

	/** readReg(std::string const& regName)
	 * @param regName name of the register to read 
	 * @retval returns the 32 bit unsigned value in the register
	 */
	virtual uint32_t readReg( std::string const& regName);

	/** readReg(std::string const& regPrefix, std::string const& regName)
	 * @param regPrefix prefix in the address table, possibly root nodes
	 * @param regName name of the register to read from the address table
	 * @retval returns the 32 bit unsigned value
	 */
	virtual uint32_t readReg( std::string const& regPrefix,
			  std::string const& regName) {
	  std::string name = regPrefix+"."+regName;
	  return readReg(name); };
	//read list of registers in a single transaction (one dispatch call) into the supplied vector regList
	void     readRegs( std::vector<std::pair<std::string, uint32_t> > &regList);

	//perform a single write transaction
	virtual void     writeReg( std::string const& regName, uint32_t const);
	virtual void     writeReg( std::string const& regPrefix,
				   std::string const& regName, uint32_t const val) {
	  std::string name = regPrefix+"."+regName;
	  return writeReg(name, val); };
	//write list of registers in a single transaction (one dispatch call) using the supplied vector regList
	virtual void     writeRegs(std::vector<std::pair<std::string, uint32_t> > const& regList);
	//write single value to a list of registers in a single transaction (one dispatch call) using the supplied vector regList
	virtual void     writeValueToRegs(std::vector<std::string> const& regList, uint32_t const& regValue);
	
	//write zero to a single register
	virtual void     zeroReg(  std::string const& regName) { writeReg(regName,0); };
	//write zero to a list of registers in a single transaction (one dispatch call) using the supplied vector regNames
	virtual void     zeroRegs( std::vector<std::string> const& regNames);

	virtual std::vector<uint32_t> readBlock( std::string const& regName);
	virtual std::vector<uint32_t> readBlock( std::string const& regName,
						 size_t const nWords);
	virtual void writeBlock(std::string const& regName,
				std::vector<uint32_t> const values);
	virtual void zeroBlock( std::string const& regName) { writeReg(regName, 0); }


	// These methods provide access to the member variables
	// specifying the uhal address table name and the IPbus protocol
	// version.
	const std::string getAddressTableFileName() const { return addressTable_;   };
	const std::string getIPbusProtocolVersion() const { return ipbusProtocol_;  };
	const std::string getDeviceBaseNode()       const { return deviceBaseNode_; };
	const std::string getDeviceIPAddress()      const { return deviceIPAddr_;   };
	const std::string getDeviceID()             const { return deviceID_;       };

	void setAddressTableFileName(std::string const& name) {
	  addressTable_ = "file://${BUILD_HOME}/data/"+name; };
	void setIPbusProtocolVersion(std::string const& version) {
	  ipbusProtocol_ = version; };
	void setDeviceBaseNode(std::string const& deviceBase) {
	  deviceBaseNode_ = deviceBase; };
	void setDeviceIPAddress(std::string const& deviceIPAddr) {
	  deviceIPAddr_ = deviceIPAddr; };
	void setDeviceID(std::string const& deviceID) {
	  deviceID_ = deviceID; };
	
	uhal::HwInterface& getGEMHwInterface() const;
	
	void updateErrorCounters(std::string const& errCode);
	
	DeviceErrors ipBusErrs;
	
	std::string printErrorCounts() const;
	
      protected:
	uhal::ConnectionManager *gemConnectionManager;
	log4cplus::Logger logGEMHw_;
	uhal::HwInterface *gemHWP_;
		
	std::string uint32ToString(uint32_t const val) const{
	  std::stringstream res;
	  res << char((val & uint32_t(0xff000000)) / 16777216);
	  res << char((val & uint32_t(0x00ff0000)) / 65536);
	  res << char((val & uint32_t(0x0000ff00)) / 256);
	  res << char((val & uint32_t(0x000000ff)));
	  return res.str(); }
	
      private:
	std::string addressTable_;
	std::string ipbusProtocol_;
	std::string deviceBaseNode_;
	std::string deviceIPAddr_;
	std::string deviceID_;
		
	//std::string registerToChar(uint32_t value) const;	

      }; //end class GEMHwDevice

  } //end namespace gem::hw

} //end namespace gem
#endif
