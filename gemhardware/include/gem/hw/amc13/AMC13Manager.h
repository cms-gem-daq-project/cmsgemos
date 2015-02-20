#ifndef gem_hw_amc13_AMC13Manager_h
#define gem_hw_amc13_AMC13Manager_h

#include <string>

#include "uhal/uhal.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "amc13/AMC13.hh"
//#include "amc13/Status.hh"

namespace amc13 {
  class AMC13;
}

namespace gem {
  namespace hw {
    namespace amc13 {
      
      class AMC13Manager : public gem::base::GEMFSMApplication
	//class AMC13Manager : public xdaq::WebApplication, public xdata::ActionListener
	{
	  
	public:
	  XDAQ_INSTANTIATOR();
	  
	  AMC13Manager(xdaq::ApplicationStub * s)
	    throw (xdaq::exception::Exception);

	  virtual ~AMC13Manager();
	  
	protected:
	  virtual void init()    throw (gem::base::exception::Exception);
	  virtual void enable()  throw (gem::base::exception::Exception);
	  virtual void disable() throw (gem::base::exception::Exception);
	  
	  
	  void Default(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void RegisterView(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void ControlPanel(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void ExpertView(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void Peek(xgi::Input *in, xgi::Output *out )
	    throw (xgi::exception::Exception);
	  void controlAMC13(xgi::Input * in, xgi::Output * out)
	    throw (xgi::exception::Exception);
	  
	  void getCheckedRegisters(cgicc::Cgicc cgi, std::vector<std::pair<std::string,uint8_t> > &regValsToSet)
	    throw (xgi::exception::Exception);
	  void performAction(cgicc::Cgicc cgi, std::vector<std::pair<std::string,uint8_t> > regValsToSet)
	    throw (xgi::exception::Exception);
	  
	  //void readCounters(  xgi::Input *in)
	  //  throw (xgi::exception::Exception);
	  //void readRegisters (xgi::Input *in)
	  //  throw (xgi::exception::Exception);
	  //void writeRegisters(xgi::Input *in)
	  //  throw (xgi::exception::Exception);
	  
	  void actionPerformed(xdata::Event& event);

	private:
	  amc13::AMC13* amc13Device_;
	  
	  //paramters taken from hcal::DTCManager (the amc13 manager for hcal)
	  std::string m_AMCInputEnableList, m_SlotEnableList;
	  bool m_daqLinkEnable, m_fakeDataEnable, m_localTtcSignalEnable;
	  bool m_ttcRxEnable, m_monBufBackPressEnable, m_megaMonitorScale;
	  bool m_internalPeriodicEnable;
	  int m_internalPeriodicPeriod, m_preScaleFactNumOfZeros;
	  uint32_t m_fedId;
	  
	  void readAMC13Registers(gem::hw::amc13::AMC13ControlParams& params);
	  //void readAMC13Registers();
	  
	  std::map<std::string,uint32_t>    amc13FullRegs_;
	  std::map<std::string,uint8_t>     amc13Regs_;
	  gem::hw::amc13::AMC13ControlParams amc13Params_;

	private:
	  std::vector<std::string>          nodes_;
	  ////counters
	  //uint16_t amc13_chipid_;
	  //uint8_t  amc13_upsetcounter_;
	  //uint32_t amc13_hitcounter_;

	protected:
	  /**
	   * Create a mapping between the AMC13 chipID and the connection name specified in the
	   * address table.  Can be done at initialization of the system as this will not change
	   * while running.  Possibly able to do the system scan and compare to a hardware databse
	   * This mapping can be used to send commands to specific chips through the manager interface
	   * the string is the name in the connection file, while the uint16_t is the chipID, though
	   * it need only be a uint12_t
	  **/
	  std::map<std::string, uint16_t> systemMap;
	  
	  //xdata::UnsignedLong myParameter_;
	  xdata::String device_;
	  xdata::String settingsFile_;
	  
	}; //end class AMC13Manager

    }//end namespace gem::hw::amc13
    
  }//end namespace gem::hw
  
}//end namespace gem

#endif
