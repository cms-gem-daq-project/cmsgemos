
// Boost includes
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/python/slice.hpp>
#include <boost/python.hpp>

// gempython includes
#include "gempython/cmsgemos_enums.h"

// gemlogging includes
#include "gem/utils/GEMLogging.h"

// gemhardware device includes
#include "gem/hw/devices/GEMHwDevice.h"
#include "gem/hw/devices/amc/HwGenericAMC.h"
#include "gem/hw/devices/glib/HwGLIB.h"
// #include "gem/hw/devices/ctp7/HwCTP7.h"
#include "gem/hw/devices/optohybrid/HwOptoHybrid.h"

#include "gem/hw/utils/GEMCrateUtils.h"

// // gempython includes
// #include "gem/python/converters.hpp"
// #include "gem/python/enums_logging.hpp"
// #include "gem/python/exceptions.hpp"

// // define the consts
// const unsigned gem::hw::HwGenericAMC::N_GTX;
// const uint8_t  gem::hw::optohybrid::MAX_VFATS;
// const uint32_t gem::hw::optohybrid::ALL_VFATS_BCAST_MASK;
// const uint32_t gem::hw::optohybrid::ALL_VFATS_DATA_MASK;

namespace bpy = boost::python;

namespace gempython {}

// BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(GEMHwDevice_overloads, readReg, 0, 1)
// BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(GEMHwDevice_overloads, writeReg, 0, 1)

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getBoardID_overloads, getBoardID, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getBoardType_overloads, getBoardType, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getFirmwareVer_overloads, getFirmwareVer, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getFirmwareDate_overloads, getFirmwareDate, 0, 1)

BOOST_PYTHON_MODULE(_cmsgemos_gempython) {
  /**
   * TODO how best to structure the module?
   * Single file for each part (enums, amc, optohybrid, etc.)?
   */
  gempython::wrap_enums();

  // GEMHwDevice class
  bpy::class_<gem::hw::GEMHwDevice, boost::noncopyable>("GEMHwDevice", bpy::no_init)
    .def("getLoggerName", &gem::hw::GEMHwDevice::getLoggerName)
    .def("readReg",  static_cast<uint32_t (gem::hw::GEMHwDevice::*)(std::string const&)>(&gem::hw::GEMHwDevice::readReg))
    .def("readReg",  static_cast<uint32_t (gem::hw::GEMHwDevice::*)(uint32_t    const&)>(&gem::hw::GEMHwDevice::readReg))
    .def("readReg",  static_cast<uint32_t (gem::hw::GEMHwDevice::*)(std::string const&,std::string const&)>(&gem::hw::GEMHwDevice::readReg))
    .def("writeReg", static_cast<void (gem::hw::GEMHwDevice::*)(std::string const&, uint32_t const)>(&gem::hw::GEMHwDevice::writeReg))
    .def("writeReg", static_cast<void (gem::hw::GEMHwDevice::*)(uint32_t    const&, uint32_t const)>(&gem::hw::GEMHwDevice::writeReg))
    .def("writeReg", static_cast<void (gem::hw::GEMHwDevice::*)(std::string const&,std::string const&, uint32_t const)>(&gem::hw::GEMHwDevice::writeReg))
    // .def("scaConversion", &gem::hw::utils::scaConversion)
    ;
  // bpy::class_<gem::hw::GEMHwDevice, boost::noncopyable>("GEMHwDevice", bpy::init<const std::string&, const std::string&>());
                                    // bpy::no_init)
    // .def(/*__str__*/ bpy::self_ns::str(bpy::self));

  bpy::class_<gem::hw::HwGenericAMC, bpy::bases<gem::hw::GEMHwDevice>,
              boost::noncopyable>("HwGenericAMC", bpy::init<const std::string&, const std::string&>())
    .def(bpy::init<const std::string&,const std::string&,const std::string&>())

    // static const defs
    .def_readonly("MAX_GTX", &gem::hw::utils::N_GTX)

    // properties?
    // .add_static_property("name", &fget [,&fset])

    // functions
    .def("getBoardID",          &gem::hw::HwGenericAMC::getBoardID,         getBoardID_overloads())
    .def("getBoardType",        &gem::hw::HwGenericAMC::getBoardType,       getBoardType_overloads())
    .def("getFirmwareVer",      &gem::hw::HwGenericAMC::getFirmwareVer,     getFirmwareVer_overloads())
    .def("getFirmwareDate",     &gem::hw::HwGenericAMC::getFirmwareDate,    getFirmwareDate_overloads())
    .def("getUserFirmware",     &gem::hw::HwGenericAMC::getUserFirmware    )
    .def("getUserFirmwareDate", &gem::hw::HwGenericAMC::getUserFirmwareDate)

    .def("getSupportedOptoHybrids", &gem::hw::HwGenericAMC::getSupportedOptoHybrids)
    .def("supportsTriggerLink",     &gem::hw::HwGenericAMC::supportsTriggerLink    )

    // .def("getSupportedOptoHybrids", &gem::hw::HwGenericAMC::getSupportedOptoHybrids)
    // .def("getSupportedOptoHybrids", &gem::hw::HwGenericAMC::getSupportedOptoHybrids)
    // .def("getSupportedOptoHybrids", &gem::hw::HwGenericAMC::getSupportedOptoHybrids)
    // .def("getSupportedOptoHybrids", &gem::hw::HwGenericAMC::getSupportedOptoHybrids)

    // copy all by default
    // FIXME: prune the list of exposed functions
    // .def("linkCheck",                        &gem::hw::HwGenericAMC::linkCheck) // private
    .def("setTrigSource",                    &gem::hw::HwGenericAMC::setTrigSource                  )// FIXME add overload
    .def("getTrigSource",                    &gem::hw::HwGenericAMC::getTrigSource                  )// FIXME add overload
    .def("setSBitSource",                    &gem::hw::HwGenericAMC::setSBitSource                  )// FIXME add overload
    .def("getSBitSource",                    &gem::hw::HwGenericAMC::getSBitSource                  )// FIXME add overload

    .def("getL1ACount",                      &gem::hw::HwGenericAMC::getL1ACount                    )
    .def("getCalPulseCount",                 &gem::hw::HwGenericAMC::getCalPulseCount               )
    .def("getResyncCount",                   &gem::hw::HwGenericAMC::getResyncCount                 )
    .def("getBC0Count",                      &gem::hw::HwGenericAMC::getBC0Count                    )
    .def("resetL1ACount",                    &gem::hw::HwGenericAMC::resetL1ACount                  )
    .def("resetCalPulseCount",               &gem::hw::HwGenericAMC::resetCalPulseCount             )
    .def("resetResyncCount",                 &gem::hw::HwGenericAMC::resetResyncCount               )
    .def("resetBC0Count",                    &gem::hw::HwGenericAMC::resetBC0Count                  )
    .def("readTriggerFIFO",                  &gem::hw::HwGenericAMC::readTriggerFIFO                )
    .def("flushTriggerFIFO",                 &gem::hw::HwGenericAMC::flushTriggerFIFO               )

    .def("configureDAQModule",               &gem::hw::HwGenericAMC::configureDAQModule             )// FIXME add overload
    .def("enableDAQLink",                    &gem::hw::HwGenericAMC::enableDAQLink                  )// FIXME add overload
    .def("disableDAQLink",                   &gem::hw::HwGenericAMC::disableDAQLink                 )
    .def("setZS",                            &gem::hw::HwGenericAMC::setZS                          )// FIXME add overload
    .def("disableZS",                        &gem::hw::HwGenericAMC::disableZS                      )
    .def("resetDAQLink",                     &gem::hw::HwGenericAMC::resetDAQLink                   )// FIXME add overload
    .def("getDAQLinkControl",                &gem::hw::HwGenericAMC::getDAQLinkControl              )
    .def("getDAQLinkStatus",                 &gem::hw::HwGenericAMC::getDAQLinkStatus               )
    .def("daqLinkReady",                     &gem::hw::HwGenericAMC::daqLinkReady                   )
    .def("daqClockLocked",                   &gem::hw::HwGenericAMC::daqClockLocked                 )
    .def("daqTTCReady",                      &gem::hw::HwGenericAMC::daqTTCReady                    )
    .def("daqTTSState",                      &gem::hw::HwGenericAMC::daqTTSState                    )
    .def("daqAlmostFull",                    &gem::hw::HwGenericAMC::daqAlmostFull                  )
    .def("l1aFIFOIsEmpty",                   &gem::hw::HwGenericAMC::l1aFIFOIsEmpty                 )
    .def("l1aFIFOIsAlmostFull",              &gem::hw::HwGenericAMC::l1aFIFOIsAlmostFull            )
    .def("l1aFIFOIsFull",                    &gem::hw::HwGenericAMC::l1aFIFOIsFull                  )
    .def("l1aFIFOIsUnderflow",               &gem::hw::HwGenericAMC::l1aFIFOIsUnderflow             )
    .def("getDAQLinkEventsSent",             &gem::hw::HwGenericAMC::getDAQLinkEventsSent           )
    .def("getDAQLinkL1AID",                  &gem::hw::HwGenericAMC::getDAQLinkL1AID                )
    /*.def("getDAQLinkL1ARate",                &gem::hw::HwGenericAMC::getDAQLinkL1ARate              ) */
    .def("getDAQLinkDisperErrors",           &gem::hw::HwGenericAMC::getDAQLinkDisperErrors         )
    .def("getDAQLinkNonidentifiableErrors",  &gem::hw::HwGenericAMC::getDAQLinkNonidentifiableErrors)
    .def("getDAQLinkInputMask",              &gem::hw::HwGenericAMC::getDAQLinkInputMask            )
    .def("getDAQLinkDAVTimeout",             &gem::hw::HwGenericAMC::getDAQLinkDAVTimeout           )
    .def("getDAQLinkDAVTimer",               &gem::hw::HwGenericAMC::getDAQLinkDAVTimer             )
    .def("getLinkDAQStatus",                 &gem::hw::HwGenericAMC::getLinkDAQStatus               )
    .def("getLinkDAQCounters",               &gem::hw::HwGenericAMC::getLinkDAQCounters             )
    .def("getLinkLastDAQBlock",              &gem::hw::HwGenericAMC::getLinkLastDAQBlock            )
    .def("getDAQLinkInputTimeout",           &gem::hw::HwGenericAMC::getDAQLinkInputTimeout         )// FIXME add overload
    .def("getDAQLinkRunType",                &gem::hw::HwGenericAMC::getDAQLinkRunType              )
    .def("getDAQLinkRunParameters",          &gem::hw::HwGenericAMC::getDAQLinkRunParameters        )
    .def("getDAQLinkRunParameter",           &gem::hw::HwGenericAMC::getDAQLinkRunParameter         )
    .def("setDAQLinkInputTimeout",           &gem::hw::HwGenericAMC::setDAQLinkInputTimeout         )
    .def("setDAQLinkRunType",                &gem::hw::HwGenericAMC::setDAQLinkRunType              )
    .def("setDAQLinkRunParameters",          &gem::hw::HwGenericAMC::setDAQLinkRunParameters        )
    .def("setDAQLinkRunParameter",           &gem::hw::HwGenericAMC::setDAQLinkRunParameter         )

    .def("ttcModuleReset",                   &gem::hw::HwGenericAMC::ttcModuleReset                 )
    .def("ttcMMCMReset",                     &gem::hw::HwGenericAMC::ttcMMCMReset                   )
    .def("ttcMMCMPhaseShift",                &gem::hw::HwGenericAMC::ttcMMCMPhaseShift              )// FIXME add overload
    .def("checkPLLLock",                     &gem::hw::HwGenericAMC::checkPLLLock                   )
    .def("getMMCMPhaseMean",                 &gem::hw::HwGenericAMC::getMMCMPhaseMean               )
    .def("getGTHPhaseMean",                  &gem::hw::HwGenericAMC::getGTHPhaseMean                )
    .def("ttcCounterReset",                  &gem::hw::HwGenericAMC::ttcCounterReset                )
    .def("getL1AEnable",                     &gem::hw::HwGenericAMC::getL1AEnable                   )
    .def("setL1AEnable",                     &gem::hw::HwGenericAMC::setL1AEnable                   )// FIXME add overload
    .def("getTTCConfig",                     &gem::hw::HwGenericAMC::getTTCConfig                   )
    .def("setTTCConfig",                     &gem::hw::HwGenericAMC::setTTCConfig                   )
    .def("getTTCStatus",                     &gem::hw::HwGenericAMC::getTTCStatus                   )
    .def("getTTCErrorCount",                 &gem::hw::HwGenericAMC::getTTCErrorCount               )// FIXME add overload
    .def("getTTCCounter",                    &gem::hw::HwGenericAMC::getTTCCounter                  )
    .def("getL1AID",                         &gem::hw::HwGenericAMC::getL1AID                       )
    .def("getL1ARate",                       &gem::hw::HwGenericAMC::getL1ARate                     )
    .def("getTTCSpyBuffer",                  &gem::hw::HwGenericAMC::getTTCSpyBuffer                )

    .def("scaHardResetEnable",               &gem::hw::HwGenericAMC::scaHardResetEnable             )// FIXME add overload
    .def("triggerReset",                     &gem::hw::HwGenericAMC::triggerReset                   )
    .def("triggerCounterReset",              &gem::hw::HwGenericAMC::triggerCounterReset            )
    .def("getOptoHybridKillMask",            &gem::hw::HwGenericAMC::getOptoHybridKillMask          )
    .def("setOptoHybridKillMask",            &gem::hw::HwGenericAMC::setOptoHybridKillMask          )
    .def("getORTriggerRate",                 &gem::hw::HwGenericAMC::getORTriggerRate               )
    .def("getORTriggerCount",                &gem::hw::HwGenericAMC::getORTriggerCount              )
    .def("getOptoHybridTriggerRate",         &gem::hw::HwGenericAMC::getOptoHybridTriggerRate       )
    .def("getOptoHybridTriggerCount",        &gem::hw::HwGenericAMC::getOptoHybridTriggerCount      )
    .def("getOptoHybridClusterRate",         &gem::hw::HwGenericAMC::getOptoHybridClusterRate       )
    .def("getOptoHybridClusterCount",        &gem::hw::HwGenericAMC::getOptoHybridClusterCount      )
    .def("getOptoHybridDebugLastCluster",    &gem::hw::HwGenericAMC::getOptoHybridDebugLastCluster  )
    .def("getOptoHybridTriggerLinkCount",    &gem::hw::HwGenericAMC::getOptoHybridTriggerLinkCount  )
    .def("generalReset",                     &gem::hw::HwGenericAMC::generalReset                   )
    .def("counterReset",                     &gem::hw::HwGenericAMC::counterReset                   )
    .def("resetT1Counters",                  &gem::hw::HwGenericAMC::resetT1Counters                )
    .def("linkCounterReset",                 &gem::hw::HwGenericAMC::linkCounterReset               )
    .def("linkReset",                        &gem::hw::HwGenericAMC::linkReset                      )

    // .def("scaConversion",       &gem::hw::utils::scaConversion)
    // .def(/*__str__*/ bpy::self_ns::str(bpy::self))
    // .add_static_property("name", &fget [,&fset])
    // .def("f", &foo::f)
    // .staticmethod("f")
    ;

  bpy::class_<gem::hw::optohybrid::HwOptoHybrid, bpy::bases<gem::hw::GEMHwDevice>,
              boost::noncopyable>("HwOptoHybrid", bpy::init<const std::string&, const std::string&>())
    .def(bpy::init<const std::string&,const std::string&,const std::string&>())
    // static const defs
    .def_readonly("MAX_VFATS",            &gem::hw::utils::MAX_VFATS)
    .def_readonly("ALL_VFATS_BCAST_MASK", &gem::hw::utils::ALL_VFATS_BCAST_MASK)
    .def_readonly("ALL_VFATS_DATA_MASK",  &gem::hw::utils::ALL_VFATS_DATA_MASK)

    // properties?
    // .add_static_property("name", &fget [,&fset])

    // functions
    .def("getFirmwareVersion",       &gem::hw::optohybrid::HwOptoHybrid::getFirmwareVersion      )
    .def("getFirmwareVersionString", &gem::hw::optohybrid::HwOptoHybrid::getFirmwareVersionString)
    .def("getFirmwareDate",          &gem::hw::optohybrid::HwOptoHybrid::getFirmwareDate         )
    .def("getFirmwareDateString",    &gem::hw::optohybrid::HwOptoHybrid::getFirmwareDateString   )

    // copy all by default
    // FIXME: prune the list of exposed functions
    .def("generalReset",          &gem::hw::optohybrid::HwOptoHybrid::generalReset)
    .def("counterReset",          &gem::hw::optohybrid::HwOptoHybrid::counterReset)
    .def("linkReset",             &gem::hw::optohybrid::HwOptoHybrid::linkReset   )
    .def("getVFATMask",           &gem::hw::optohybrid::HwOptoHybrid::getVFATMask )
    .def("setVFATMask",           &gem::hw::optohybrid::HwOptoHybrid::setVFATMask )
    .def("resetVFATs",            &gem::hw::optohybrid::HwOptoHybrid::resetVFATs  )
    .def("setSBitMask",           &gem::hw::optohybrid::HwOptoHybrid::setSBitMask )
    .def("getSBitMask",           &gem::hw::optohybrid::HwOptoHybrid::getSBitMask )
    .def("setHDMISBitSource", static_cast<void (gem::hw::optohybrid::HwOptoHybrid::*)(uint8_t const&, uint8_t const&)>(&gem::hw::optohybrid::HwOptoHybrid::setHDMISBitSource))
    .def("setHDMISBitSource", static_cast<void (gem::hw::optohybrid::HwOptoHybrid::*)(std::array<uint8_t,8> const&)>(&gem::hw::optohybrid::HwOptoHybrid::setHDMISBitSource))
    .def("getHDMISBitSource",     &gem::hw::optohybrid::HwOptoHybrid::getHDMISBitSource)
    .def("setHDMISBitMode",       &gem::hw::optohybrid::HwOptoHybrid::setHDMISBitMode  )
    .def("getHDMISBitMode",       &gem::hw::optohybrid::HwOptoHybrid::getHDMISBitMode  )
    .def("hasSEUError",           &gem::hw::optohybrid::HwOptoHybrid::hasSEUError      )
    .def("getFPGATemp",           &gem::hw::optohybrid::HwOptoHybrid::getFPGATemp      )
    .def("getFPGAMaxTemp",        &gem::hw::optohybrid::HwOptoHybrid::getFPGAMaxTemp   )
    .def("getFPGAMinTemp",        &gem::hw::optohybrid::HwOptoHybrid::getFPGAMinTemp   )
    .def("getFPGAVccInt",         &gem::hw::optohybrid::HwOptoHybrid::getFPGAVccInt    )
    .def("getFPGAMaxVccInt",      &gem::hw::optohybrid::HwOptoHybrid::getFPGAMaxVccInt )
    .def("getFPGAMinVccInt",      &gem::hw::optohybrid::HwOptoHybrid::getFPGAMinVccInt )
    .def("getFPGAVccAux",         &gem::hw::optohybrid::HwOptoHybrid::getFPGAVccAux    )
    .def("getFPGAMaxVccAux",      &gem::hw::optohybrid::HwOptoHybrid::getFPGAMaxVccAux )
    .def("getFPGAMinVccAux",      &gem::hw::optohybrid::HwOptoHybrid::getFPGAMinVccAux )
    .def("getADCVPVN",            &gem::hw::optohybrid::HwOptoHybrid::getADCVPVN       )
    .def("getADCVAUX",            &gem::hw::optohybrid::HwOptoHybrid::getADCVAUX       )
    .def("getVFATDACOutV",        &gem::hw::optohybrid::HwOptoHybrid::getVFATDACOutV   )
    .def("getVFATDACOutI",        &gem::hw::optohybrid::HwOptoHybrid::getVFATDACOutI   )

    // requires exposing special types to python
    // std::vector<uint32_t> broadcastRead()
    // void broadcastWrite()
    // std::vector<std::pair<uint8_t,uint32_t> > getConnectedVFATs(bool update=false)
    // uint32_t getConnectedVFATMask(bool update=false)
    // overloaded
    // void setVFATsToDefaults(vt1,vt2,latency,broadcastMask)
    // void setVFATsToDefaults(std::map<std::string, uint8_t> regvals,broadcastMask)
    // .def("scaConversion",            &gem::hw::utils::scaConversion)
    // .def(/*__str__*/ bpy::self_ns::str(bpy::self))
    ;
}
