/**
 * class: OptoHybridMonitor
 * description: Monitor application for OptoHybrid cards
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date: 
 */

#include "gem/hw/optohybrid/HwOptoHybrid.h"

#include "gem/hw/optohybrid/OptoHybridMonitor.h"
#include "gem/hw/optohybrid/OptoHybridManager.h"
#include "gem/base/GEMApplication.h"
#include "gem/base/GEMFSMApplication.h"

typedef gem::base::utils::GEMInfoSpaceToolBox::UpdateType GEMUpdateType;

gem::hw::optohybrid::OptoHybridMonitor::OptoHybridMonitor(std::shared_ptr<HwOptoHybrid> optohybrid,
                                                          OptoHybridManager* optohybridManager,
                                                          int const& index) :
  GEMMonitor(optohybridManager->getApplicationLogger(), static_cast<xdaq::Application*>(optohybridManager), index),
  p_optohybrid(optohybrid)
{
  // application info space is added in the base class constructor
  // addInfoSpace("Application", optohybridManager->getApplicationInfoSpace());
  // addInfoSpace("OptoHybrid", p_optohybrid->getHwInfoSpace());

  // if (!p_optohybrid->getHwInfoSpace()->hasItem(monname))
  //   p_optohybrid->getHwInfoSpace()->fireItemAvailable(monname, &monvar);

  updateMonitorables();
}


void gem::hw::optohybrid::OptoHybridMonitor::setupHwMonitoring()
{
  // create the values to be monitored in the info space
  addMonitorableSet("SYSTEM", "HWMonitoring");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("BOARD_ID", "OptoHybrid.SYSTEM.BOARD_ID"),
                 GEMUpdateType::NOUPDATE, "id");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("SYSTEM_ID", "OptoHybrid.SYSTEM.SYSTEM_ID"),
                 GEMUpdateType::NOUPDATE, "id");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("FIRMWARE_ID", "OptoHybrid.SYSTEM.FIRMWARE.ID"),
                 GEMUpdateType::NOUPDATE, "fwver");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("FIRMWARE_DATE", "OptoHybrid.SYSTEM.FIRMWARE.DATE"),
                 GEMUpdateType::NOUPDATE, "date");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("IP_ADDRESS", "OptoHybrid.SYSTEM.IP_INFO"),
                 GEMUpdateType::NOUPDATE, "ip");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("V6_CPLD",  "OptoHybrid.SYSTEM.STATUS.V6_CPLD"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("SYSTEM", "HWMonitoring",
                 std::make_pair("CPLD_LOCK", "OptoHybrid.SYSTEM.STATUS.CDCE_LOCK"),
                 GEMUpdateType::HW32, "hex");
  
  addMonitorableSet("CONTROL", "HWMonitoring");
  addMonitorable("CONTROL", "HWMonitoring",
                 std::make_pair("L1A", "OptoHybrid.CONTROL.T1.L1A"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("CONTROL", "HWMonitoring",
                 std::make_pair("CalPulse", "OptoHybrid.CONTROL.T1.CalPulse"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("CONTROL", "HWMonitoring",
                 std::make_pair("Resync", "OptoHybrid.CONTROL.T1.Resync"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("CONTROL", "HWMonitoring",
                 std::make_pair("BC0", "OptoHybrid.CONTROL.T1.BC0"),
                 GEMUpdateType::HW32, "hex");

  addMonitorableSet("IPBus", "HWMonitoring");
  addMonitorable("IPBus", "HWMonitoring",
                 std::make_pair("OptoHybrid_0", "OptoHybrid.COUNTERS.IPBus"),
                 GEMUpdateType::I2CSTAT, "i2c/hex");
  addMonitorable("IPBus", "HWMonitoring",
                 std::make_pair("OptoHybrid_1", "OptoHybrid.COUNTERS.IPBus"),
                 GEMUpdateType::I2CSTAT, "i2c/hex");
  addMonitorable("IPBus", "HWMonitoring",
                 std::make_pair("TRK_0", "OptoHybrid.COUNTERS.IPBus"),
                 GEMUpdateType::I2CSTAT, "i2c/hex");
  addMonitorable("IPBus", "HWMonitoring",
                 std::make_pair("TRK_1", "OptoHybrid.COUNTERS.IPBus"),
                 GEMUpdateType::I2CSTAT, "i2c/hex");
  addMonitorable("IPBus", "HWMonitoring",
                 std::make_pair("Counters", "OptoHybrid.COUNTERS.IPBus"),
                 GEMUpdateType::I2CSTAT, "i2c/hex");

  addMonitorableSet("ThresholdLatencyScanController", "HWMonitoring");
  addMonitorable("ThresholdLatencyScanController", "HWMonitoring",
                 std::make_pair("START", "ScanController.THLAT.START"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("ThresholdLatencyScanController", "HWMonitoring",
                 std::make_pair("MODE", "ScanController.THLAT.MODE"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("ThresholdLatencyScanController", "HWMonitoring",
                 std::make_pair("CHIP", "ScanController.THLAT.CHIP"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("ThresholdLatencyScanController", "HWMonitoring",
                 std::make_pair("CHAN", "ScanController.THLAT.CHAN"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("ThresholdLatencyScanController", "HWMonitoring",
                 std::make_pair("MIN", "ScanController.THLAT.MIN"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("ThresholdLatencyScanController", "HWMonitoring",
                 std::make_pair("MAX", "ScanController.THLAT.MAX"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("ThresholdLatencyScanController", "HWMonitoring",
                 std::make_pair("STEP", "ScanController.THLAT.STEP"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("ThresholdLatencyScanController", "HWMonitoring",
                 std::make_pair("NTRIGS", "ScanController.THLAT.NTRIGS"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("ThresholdLatencyScanController", "HWMonitoring",
                 std::make_pair("MONITOR", "ScanController.THLAT.MONITOR"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("ThresholdLatencyScanController", "HWMonitoring",
                 std::make_pair("RESET", "ScanController.THLAT.RESET"),
                 GEMUpdateType::HW32, "hex");

  addMonitorableSet("DACScanController", "HWMonitoring");
  addMonitorable("DACScanController", "HWMonitoring",
                 std::make_pair("START", "ScanController.DAC.START"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DACScanController", "HWMonitoring",
                 std::make_pair("MODE", "ScanController.DAC.MODE"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DACScanController", "HWMonitoring",
                 std::make_pair("CHIP", "ScanController.DAC.CHIP"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DACScanController", "HWMonitoring",
                 std::make_pair("CHAN", "ScanController.DAC.CHAN"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DACScanController", "HWMonitoring",
                 std::make_pair("MIN", "ScanController.DAC.MIN"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DACScanController", "HWMonitoring",
                 std::make_pair("MAX", "ScanController.DAC.MAX"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DACScanController", "HWMonitoring",
                 std::make_pair("STEP", "ScanController.DAC.STEP"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DACScanController", "HWMonitoring",
                 std::make_pair("NTRIGS", "ScanController.DAC.NTRIGS"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DACScanController", "HWMonitoring",
                 std::make_pair("MONITOR", "ScanController.DAC.MONITOR"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("DACScanController", "HWMonitoring",
                 std::make_pair("RESET", "ScanController.DAC.RESET"),
                 GEMUpdateType::HW32, "hex");

  addMonitorableSet("WishboneMasterCounters", "HWMonitoring");
  addMonitorable("WishboneMasterCounters", "HWMonitoring",
                 std::make_pair("GTXStrobe",    "COUNTERS.WB.MASTER.Strobe.GTX"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneMasterCounters", "HWMonitoring",
                 std::make_pair("GTXAck",       "COUNTERS.WB.MASTER.Ack.GTX"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneMasterCounters", "HWMonitoring",
                 std::make_pair("ExtI2CStrobe", "COUNTERS.WB.MASTER.Strobe.ExtI2C"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneMasterCounters", "HWMonitoring",
                 std::make_pair("ExtI2CAck",    "COUNTERS.WB.MASTER.Ack.ExtI2C"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneMasterCounters", "HWMonitoring",
                 std::make_pair("ScanStrobe",   "COUNTERS.WB.MASTER.Strobe.Scan"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneMasterCounters", "HWMonitoring",
                 std::make_pair("ScanAck",      "COUNTERS.WB.MASTER.Ack.Scan"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneMasterCounters", "HWMonitoring",
                 std::make_pair("DACStrobe",    "COUNTERS.WB.MASTER.Strobe.DAC"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneMasterCounters", "HWMonitoring",
                 std::make_pair("DACAck",       "COUNTERS.WB.MASTER.Ack.DAC"),
                 GEMUpdateType::HW32, "hex");

  addMonitorableSet("WishboneSlaveCounters", "HWMonitoring");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("I2C0Strobe",    "COUNTERS.WB.SLAVE.Strobe.I2C0"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("I2C0Ack",       "COUNTERS.WB.SLAVE.Ack.I2C0"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("I2C1Strobe",    "COUNTERS.WB.SLAVE.Strobe.I2C1"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("I2C1Ack",       "COUNTERS.WB.SLAVE.Ack.I2C1"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("I2C2Strobe",    "COUNTERS.WB.SLAVE.Strobe.I2C2"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("I2C2Ack",       "COUNTERS.WB.SLAVE.Ack.I2C2"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("I2C3Strobe",    "COUNTERS.WB.SLAVE.Strobe.I2C3"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("I2C3Ack",       "COUNTERS.WB.SLAVE.Ack.I2C3"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("I2C4Strobe",    "COUNTERS.WB.SLAVE.Strobe.I2C4"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("I2C4Ack",       "COUNTERS.WB.SLAVE.Ack.I2C4"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("I2C5Strobe",    "COUNTERS.WB.SLAVE.Strobe.I2C5"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("I2C5Ack",       "COUNTERS.WB.SLAVE.Ack.I2C5"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("ExtI2CStrobe",    "COUNTERS.WB.SLAVE.Strobe.ExtI2C"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("ExtI2CAck",       "COUNTERS.WB.SLAVE.Ack.ExtI2C"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("ScanStrobe",    "COUNTERS.WB.SLAVE.Strobe.Scan"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("ScanAck",       "COUNTERS.WB.SLAVE.Ack.Scan"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("T1Strobe",    "COUNTERS.WB.SLAVE.Strobe.T1"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("T1Ack",       "COUNTERS.WB.SLAVE.Ack.T1"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("DACStrobe",    "COUNTERS.WB.SLAVE.Strobe.DAC"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("DACAck",       "COUNTERS.WB.SLAVE.Ack.DAC"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("ADCStrobe",    "COUNTERS.WB.SLAVE.Strobe.ADC"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("ADCAck",       "COUNTERS.WB.SLAVE.Ack.ADC"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("ClockingStrobe", "COUNTERS.WB.SLAVE.Strobe.Clocking"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("ClockingAck",    "COUNTERS.WB.SLAVE.Ack.Clocking"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("CountersStrobe", "COUNTERS.WB.SLAVE.Strobe.Counters"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("CountersAck",    "COUNTERS.WB.SLAVE.Ack.Counters"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("SystemStrobe",   "COUNTERS.WB.SLAVE.Strobe.System"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("WishboneSlaveCounters", "HWMonitoring",
                 std::make_pair("SystemAck",      "COUNTERS.WB.SLAVE.Ack.System"),
                 GEMUpdateType::HW32, "hex");

  addMonitorableSet("VFATCRC", "HWMonitoring");
  addMonitorable("VFATCRC", "HWMonitoring",
                 std::make_pair("VFAT0_Valid",   "COUNTERS.CRC.VALID.VFAT0"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("VFATCRC", "HWMonitoring",
                 std::make_pair("VFAT0_Invalid", "COUNTERS.CRC.INVALID.VFAT0"),
                 GEMUpdateType::HW32, "hex");

  addMonitorableSet("T1Counters", "HWMonitoring");
  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("TTCL1A",      "COUNTERS.T1.TTC.L1A"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("TTCCalPulse", "COUNTERS.T1.TTC.CalPulse"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("TTCResync",   "COUNTERS.T1.TTC.Resync"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("TTCBC0",      "COUNTERS.T1.TTC.BC0"),
                 GEMUpdateType::HW32, "hex");

  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("INTERNALL1A",      "COUNTERS.T1.INTERNAL.L1A"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("INTERNALCalPulse", "COUNTERS.T1.INTERNAL.CalPulse"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("INTERNALResync",   "COUNTERS.T1.INTERNAL.Resync"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("INTERNALBC0",      "COUNTERS.T1.INTERNAL.BC0"),
                 GEMUpdateType::HW32, "hex");

  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("EXTERNALL1A",      "COUNTERS.T1.EXTERNAL.L1A"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("EXTERNALCalPulse", "COUNTERS.T1.EXTERNAL.CalPulse"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("EXTERNALResync",   "COUNTERS.T1.EXTERNAL.Resync"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("EXTERNALBC0",      "COUNTERS.T1.EXTERNAL.BC0"),
                 GEMUpdateType::HW32, "hex");

  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("LOOPBACKL1A",      "COUNTERS.T1.LOOPBACK.L1A"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("LOOPBACKCalPulse", "COUNTERS.T1.LOOPBACK.CalPulse"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("LOOPBACKResync",   "COUNTERS.T1.LOOPBACK.Resync"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("LOOPBACKBC0",      "COUNTERS.T1.LOOPBACK.BC0"),
                 GEMUpdateType::HW32, "hex");

  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("SENTL1A",      "COUNTERS.T1.SENT.L1A"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("SENTCalPulse", "COUNTERS.T1.SENT.CalPulse"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("SENTResync",   "COUNTERS.T1.SENT.Resync"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("T1Counters", "HWMonitoring",
                 std::make_pair("SENTBC0",      "COUNTERS.T1.SENT.BC0"),
                 GEMUpdateType::HW32, "hex");

  addMonitorableSet("GTCCounters", "HWMonitoring");
  addMonitorable("GTCCounters", "HWMonitoring",
                 std::make_pair("TrackingLinkErrors", "COUNTERS.GTX.TRK_ERR"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("GTCCounters", "HWMonitoring",
                 std::make_pair("TriggerLinkErrors",  "COUNTERS.GTX.TRG_ERR"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("GTCCounters", "HWMonitoring",
                 std::make_pair("DataPackets",        "COUNTERS.GTX.DATA_Packets"),
                 GEMUpdateType::HW32, "hex");

  addMonitorableSet("ADC", "HWMonitoring");
  addMonitorable("ADC", "HWMonitoring",
                 std::make_pair("ADC_CONTROL", "OptoHybrid.ADC.CONTROL"),
                 GEMUpdateType::HW32, "hex");
  addMonitorable("ADC", "HWMonitoring",
                 std::make_pair("ADC_SPY", "OptoHybrid.ADC.SPY"),
                 GEMUpdateType::HW32, "hex");
  updateMonitorables();
}

gem::hw::optohybrid::OptoHybridMonitor::~OptoHybridMonitor()
{

}

void gem::hw::optohybrid::OptoHybridMonitor::updateMonitorables()
{
  // define how to update the desired values
  // get SYSTEM monitorables
  // can this be split into two loops, one just to do a list read, the second to fill the InfoSpace with the returned values
  DEBUG("OptoHybridMonitor: Updating monitorables");
  for (auto monlist = m_monitorableSetsMap.begin(); monlist != m_monitorableSetsMap.end(); ++monlist) {
    DEBUG("OptoHybridMonitor: Updating monitorables in set " << monlist->first);
    for (auto monitem = monlist->second.begin(); monitem != monlist->second.end(); ++monitem) {
      DEBUG("OptoHybridMonitor: Updating monitorable " << monitem->first);
      std::stringstream regName;
      regName << monitem->second.regname;
      uint32_t address = p_optohybrid->getGEMHwInterface().getNode(regName.str()).getAddress();
      uint32_t mask    = p_optohybrid->getGEMHwInterface().getNode(regName.str()).getMask();
      if (monitem->second.updatetype == GEMUpdateType::HW8) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_optohybrid->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::HW16) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_optohybrid->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::HW24) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_optohybrid->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::HW32) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_optohybrid->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::HW64) {
        address = p_optohybrid->getGEMHwInterface().getNode(regName.str()+".LOWER").getAddress();
        mask    = p_optohybrid->getGEMHwInterface().getNode(regName.str()+".LOWER").getMask();
        uint32_t lower = p_optohybrid->readReg(address,mask);
        address = p_optohybrid->getGEMHwInterface().getNode(regName.str()+".UPPER").getAddress();
        mask    = p_optohybrid->getGEMHwInterface().getNode(regName.str()+".UPPER").getMask();
        uint32_t upper = p_optohybrid->readReg(address,mask);
        (monitem->second.infoSpace)->setUInt64(monitem->first, (((uint64_t)upper) << 32) + lower);
      } else if (monitem->second.updatetype == GEMUpdateType::I2CSTAT) {
        std::stringstream strobeReg;
        strobeReg << regName.str() << ".Strobe." << monitem->first;
        address = p_optohybrid->getGEMHwInterface().getNode(strobeReg.str()).getAddress();
        mask    = p_optohybrid->getGEMHwInterface().getNode(strobeReg.str()).getMask();
        uint32_t strobe = p_optohybrid->readReg(address,mask);
        std::stringstream ackReg;
        ackReg << regName.str() << ".Ack." << monitem->first;
        address = p_optohybrid->getGEMHwInterface().getNode(ackReg.str()).getAddress();
        mask    = p_optohybrid->getGEMHwInterface().getNode(ackReg.str()).getMask();
        uint32_t ack = p_optohybrid->readReg(address,mask);
        (monitem->second.infoSpace)->setUInt64(monitem->first, (((uint64_t)ack) << 32) + strobe);
      } else if (monitem->second.updatetype == GEMUpdateType::PROCESS) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_optohybrid->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::TRACKER) {
        (monitem->second.infoSpace)->setUInt32(monitem->first,p_optohybrid->readReg(address,mask));
      } else if (monitem->second.updatetype == GEMUpdateType::NOUPDATE) {
        continue;
      } else {
        ERROR("OptoHybridMonitor: Unknown update type encountered");
        continue;
      }
    } // end loop over items in list
  } // end loop over monitorableSets
}

void gem::hw::optohybrid::OptoHybridMonitor::buildMonitorPage(xgi::Output* out)
{
  DEBUG("OptoHybridMonitor::buildMonitorPage");
  if (m_infoSpaceMonitorableSetMap.find("HWMonitoring") == m_infoSpaceMonitorableSetMap.end()) {
    WARN("Unable to find item set HWMonitoring in monitor");
    return;
  }
  
  auto monsets = m_infoSpaceMonitorableSetMap.find("HWMonitoring")->second;
  
  // loop over the list of monitor sets and grab the monitorables from each one
  // create a div tab for each set, and a table for each set of values
  *out << "<div class=\"xdaq-tab-wrapper\">" << std::endl;
  for (auto monset = monsets.begin(); monset != monsets.end(); ++monset) {
    *out << "<div class=\"xdaq-tab\" title=\""  << *monset << "\" >"  << std::endl;
    *out << "<table class=\"xdaq-table\" id=\"" << *monset << "_table\">" << std::endl
         << cgicc::thead() << std::endl
         << cgicc::tr()    << std::endl //open
         << cgicc::th()    << "Register name"    << cgicc::th() << std::endl
         << cgicc::th()    << "Value"            << cgicc::th() << std::endl
         << cgicc::th()    << "Register address" << cgicc::th() << std::endl
         << cgicc::th()    << "Description"      << cgicc::th() << std::endl
         << cgicc::tr()    << std::endl //close
         << cgicc::thead() << std::endl 
         << "<tbody>" << std::endl;
    
    for (auto monitem = m_monitorableSetsMap.find(*monset)->second.begin();
         monitem != m_monitorableSetsMap.find(*monset)->second.end(); ++monitem) {
      *out << "<tr>"    << std::endl;
      
      *out << "<td>"    << std::endl
           << monitem->first
           << "</td>"   << std::endl;
        
      DEBUG("OptoHybridMonitor::" << monitem->first << " formatted to "
            << (monitem->second.infoSpace)->getFormattedItem(monitem->first,monitem->second.format));
      //this will be repeated for every OptoHybridMonitor in the OptoHybridManager..., need a better unique ID
      *out << "<td id=\"" << monitem->second.infoSpace->name() << "-" << monitem->first << "\">" << std::endl
           << (monitem->second.infoSpace)->getFormattedItem(monitem->first,monitem->second.format)
           << "</td>"   << std::endl;

      *out << "<td>"    << std::endl
           << monitem->second.regname
           << "</td>"   << std::endl;

      *out << "<td>"    << std::endl
           << "description"
           << "</td>"   << std::endl;

      *out << "</tr>"   << std::endl;
    }
    *out << "</tbody>"  << std::endl
         << "</table>"  << std::endl
         << "</div>"    << std::endl;
  }
  *out << "</div>"  << std::endl;
  
}

void gem::hw::optohybrid::OptoHybridMonitor::reset()
{
  //have to get rid of the timer 
  DEBUG("GEMMonitor::reset");
  for (auto infoSpace = m_infoSpaceMap.begin(); infoSpace != m_infoSpaceMap.end(); ++infoSpace) {
    DEBUG("OptoHybridMonitor::reset removing " << infoSpace->first << " from p_timer");
    try {
      p_timer->remove(infoSpace->first);
    } catch (toolbox::task::exception::Exception& te) {
      ERROR("OptoHybridMonitor::Caught exception while removing timer task " << infoSpace->first << " " << te.what());
    }
  }
  stopMonitoring();
  DEBUG("GEMMonitor::reset removing timer " << m_timerName << " from timerFactory");
  try {
    toolbox::task::getTimerFactory()->removeTimer(m_timerName);
  } catch (toolbox::task::exception::Exception& te) {
    ERROR("OptoHybridMonitor::Caught exception while removing timer " << m_timerName << " " << te.what());
  }
  
  DEBUG("OptoHybridMonitor::reset - clearing all maps");
  m_infoSpaceMap.clear();
  m_infoSpaceMonitorableSetMap.clear();
  m_monitorableSetInfoSpaceMap.clear();
  m_monitorableSetsMap.clear();
}
