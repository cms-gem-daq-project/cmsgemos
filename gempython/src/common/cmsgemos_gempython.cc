
// Boost includes
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/python/slice.hpp>
#include <boost/python.hpp>

// gemlogging includes
#include "gem/utils/GEMLogging.h"

// gemhardware device includes
#include "gem/hw/GEMHwDevice.h"
#include "gem/hw/HwGenericAMC.h"
#include "gem/hw/glib/HwGLIB.h"
// #include "gem/hw/ctp7/HwCTP7.h"
#include "gem/hw/optohybrid/HwOptoHybrid.h"

// gemhardware enum includes
#include "gem/hw/GenericAMCSettingsEnums.h"
#include "gem/hw/optohybrid/OptoHybridSettingsEnums.h"
#include "gem/hw/glib/GLIBSettingsEnums.h"
// #include "gem/hw/ctp7/CTP7SettingsEnums.h"
// #include "gem/hw/vfat/VFAT2SettingsEnums.h"
// #include "gem/hw/vfat/VFAT3SettingsEnums.h"

// // gempython includes
// #include "gem/python/converters.hpp"
// #include "gem/python/enums_logging.hpp"
// #include "gem/python/exceptions.hpp"


namespace bpy = boost::python;

namespace gempython {}

// BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS

BOOST_PYTHON_MODULE(_cmsgemos_gempython) {

  // GEMHwDevice class
  bpy::class_<gem::hw::GEMHwDevice, boost::noncopyable>("GEMHwDevice", bpy::no_init)
    .def("getLoggerName", &gem::hw::GEMHwDevice::getLoggerName);
  // bpy::class_<gem::hw::GEMHwDevice, boost::noncopyable>("GEMHwDevice", bpy::init<const std::string&, const std::string&>());
                                    // bpy::no_init)
    // .def(/*__str__*/ bpy::self_ns::str(bpy::self));

  bpy::class_<gem::hw::HwGenericAMC, bpy::bases<gem::hw::GEMHwDevice>, boost::noncopyable>("HwGenericAMC",
                                                                        bpy::init<const std::string&,const std::string&>())
    .def("getBoardID",          &gem::hw::HwGenericAMC::getBoardID)
    .def("getBoardIDRaw",       &gem::hw::HwGenericAMC::getBoardIDRaw)
    .def("getFirmwareVer",      &gem::hw::HwGenericAMC::getFirmwareVer)
    .def("getFirmwareVerRaw",   &gem::hw::HwGenericAMC::getFirmwareVerRaw)
    .def("getFirmwareDate",     &gem::hw::HwGenericAMC::getFirmwareDate)
    .def("getFirmwareDateRaw",  &gem::hw::HwGenericAMC::getFirmwareDateRaw)
    .def("getUserFirmware",     &gem::hw::HwGenericAMC::getUserFirmware)
    .def("getUserFirmwareDate", &gem::hw::HwGenericAMC::getUserFirmwareDate)
    // .def(/*__str__*/ bpy::self_ns::str(bpy::self))
    ;

  bpy::class_<gem::hw::optohybrid::HwOptoHybrid, bpy::bases<gem::hw::GEMHwDevice>, boost::noncopyable>("HwOptoHybrid",
                                                                                    bpy::init<const std::string&,const std::string&>())
    .def("getFirmwareVersion",       &gem::hw::optohybrid::HwOptoHybrid::getFirmwareVersion)
    .def("getFirmwareVersionString", &gem::hw::optohybrid::HwOptoHybrid::getFirmwareVersionString)
    .def("getFirmwareDate",          &gem::hw::optohybrid::HwOptoHybrid::getFirmwareDate)
    .def("getFirmwareDateString",    &gem::hw::optohybrid::HwOptoHybrid::getFirmwareDateString)
    // .def(/*__str__*/ bpy::self_ns::str(bpy::self))
    ;
}
