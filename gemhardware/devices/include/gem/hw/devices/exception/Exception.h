/** @file gem/hw/devices/exception/Exception.h */

#ifndef GEM_HW_DEVICES_EXCEPTION_EXCEPTION_H
#define GEM_HW_DEVICES_EXCEPTION_EXCEPTION_H

#include "gem/utils/exception/Exception.h"

// The gem::hw exceptions.
#define GEM_HW_DEVICES_DEFINE_EXCEPTION(EXCEPTION_NAME) GEM_DEFINE_EXCEPTION(EXCEPTION_NAME, hw::devices)
GEM_HW_DEVICES_DEFINE_EXCEPTION(Exception)
GEM_HW_DEVICES_DEFINE_EXCEPTION(WriteValueMismatch)
GEM_HW_DEVICES_DEFINE_EXCEPTION(ConfigurationParseProblem)
GEM_HW_DEVICES_DEFINE_EXCEPTION(ConfigurationProblem)
GEM_HW_DEVICES_DEFINE_EXCEPTION(ConfigurationValidationProblem)

GEM_HW_DEVICES_DEFINE_EXCEPTION(HardwareProblem)
GEM_HW_DEVICES_DEFINE_EXCEPTION(UninitializedDevice)

GEM_HW_DEVICES_DEFINE_EXCEPTION(RCMSNotificationError)
GEM_HW_DEVICES_DEFINE_EXCEPTION(SOAPTransitionProblem)
GEM_HW_DEVICES_DEFINE_EXCEPTION(NULLReadoutPointer)

GEM_HW_DEVICES_DEFINE_EXCEPTION(SoftwareProblem)
GEM_HW_DEVICES_DEFINE_EXCEPTION(TransitionProblem)
GEM_HW_DEVICES_DEFINE_EXCEPTION(ValueError)

GEM_HW_DEVICES_DEFINE_EXCEPTION(RPCMethodError)

GEM_HW_DEVICES_DEFINE_EXCEPTION(DeviceNameParseError)

GEM_HW_DEVICES_DEFINE_EXCEPTION(ReadoutProblem)
// The gem::hw alarms.
#define GEM_HW_DEVICES_DEFINE_ALARM(ALARM_NAME) GEM_HW_DEVICES_DEFINE_EXCEPTION(ALARM_NAME)

GEM_HW_DEVICES_DEFINE_ALARM(MonitoringFailureAlarm)

/**
 * Macros to catch standard cases
 * @param base of the error message
 * @param exception type that is emitted
 */

// FIXME: SHOULD I BE A MACRO OR A FUNCTION ELSEWHERE?
#define GEM_CATCH_RPC_ERROR(MSG_BASE, EX_TYPE)                   \
catch (xhal::common::utils::XHALRPCNotConnectedException const& e) {     \
  std::stringstream errmsg;                                      \
  errmsg << e.what();                                            \
  CMSGEMOS_ERROR(MSG_BASE << "error: " << errmsg.str());         \
  XCEPT_RAISE(EX_TYPE, errmsg.str());                            \
} catch (xhal::common::utils::XHALRPCException const& e) {               \
  std::stringstream errmsg;                                      \
  errmsg << e.what();                                            \
  CMSGEMOS_ERROR(MSG_BASE << "error: " << errmsg.str());         \
  XCEPT_RAISE(EX_TYPE, errmsg.str());                            \
} catch (gem::hw::devices::exception::RPCMethodError const& e) { \
  std::stringstream errmsg;                                      \
  errmsg << e.what();                                            \
  CMSGEMOS_ERROR(MSG_BASE << "error: " << errmsg.str());         \
  XCEPT_RAISE(EX_TYPE, errmsg.str());                            \
}

#endif  // GEM_HW_DEVICES_EXCEPTION_EXCEPTION_H
