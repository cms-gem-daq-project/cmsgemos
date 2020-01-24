/** @file gem/hw/managers/exception/Exception.h */

#ifndef GEM_HW_MANAGERS_EXCEPTION_EXCEPTION_H
#define GEM_HW_MANAGERS_EXCEPTION_EXCEPTION_H

#include "gem/utils/exception/Exception.h"

// The gem::hw exceptions.
#define GEM_HW_MANAGERS_DEFINE_EXCEPTION(EXCEPTION_NAME) GEM_DEFINE_EXCEPTION(EXCEPTION_NAME, hw::managers)
GEM_HW_MANAGERS_DEFINE_EXCEPTION(Exception)
GEM_HW_MANAGERS_DEFINE_EXCEPTION(WriteValueMismatch)
GEM_HW_MANAGERS_DEFINE_EXCEPTION(ConfigurationParseProblem)
GEM_HW_MANAGERS_DEFINE_EXCEPTION(ConfigurationProblem)
GEM_HW_MANAGERS_DEFINE_EXCEPTION(ConfigurationValidationProblem)

GEM_HW_MANAGERS_DEFINE_EXCEPTION(HardwareProblem)
GEM_HW_MANAGERS_DEFINE_EXCEPTION(UninitializedDevice)

GEM_HW_MANAGERS_DEFINE_EXCEPTION(RCMSNotificationError)
GEM_HW_MANAGERS_DEFINE_EXCEPTION(SOAPTransitionProblem)
GEM_HW_MANAGERS_DEFINE_EXCEPTION(NULLReadoutPointer)

GEM_HW_MANAGERS_DEFINE_EXCEPTION(SoftwareProblem)
GEM_HW_MANAGERS_DEFINE_EXCEPTION(TransitionProblem)
GEM_HW_MANAGERS_DEFINE_EXCEPTION(ValueError)

GEM_HW_MANAGERS_DEFINE_EXCEPTION(RPCMethodError)

GEM_HW_MANAGERS_DEFINE_EXCEPTION(DeviceNameParseError)

// The gem::hw::manager alarms.
#define GEM_HW_MANAGERS_DEFINE_ALARM(ALARM_NAME) GEM_HW_MANAGERS_DEFINE_EXCEPTION(ALARM_NAME)

GEM_HW_MANAGERS_DEFINE_ALARM(MonitoringFailureAlarm)

// FIXME: SHOULD I BE A MACRO OR A FUNCTION ELSEWHERE
// FIXME: SHOULD TRANSITION ERRORS FORCE FSM TO ERROR?
#define GEM_HW_TRANSITION_CATCH(MSG_BASE, EX_TYPE)             \
catch (uhalException const& e) {                               \
  std::stringstream errmsg;                                    \
  errmsg << MSG_BASE << " caught uHAL exception " << e.what(); \
  CMSGEMOS_ERROR(errmsg.str());                                \
  XCEPT_RAISE(EX_TYPE, errmsg.str());                          \
} catch (EX_TYPE const& e) {                                   \
  std::stringstream errmsg;                                    \
  errmsg << MSG_BASE << " caught exception " << e.what();      \
  CMSGEMOS_ERROR(errmsg.str());                                \
  XCEPT_RAISE(EX_TYPE, errmsg.str());                          \
} catch (toolbox::net::exception::MalformedURN const& e) {     \
  std::stringstream errmsg;                                    \
  errmsg << MSG_BASE << " caught exception " << e.what();      \
  CMSGEMOS_ERROR(errmsg.str());                                \
  XCEPT_RAISE(EX_TYPE, errmsg.str());                          \
} catch (std::exception const& e) {                            \
  std::stringstream errmsg;                                    \
  errmsg << MSG_BASE << " caught exception " << e.what();      \
  CMSGEMOS_ERROR(errmsg.str());                                \
  XCEPT_RAISE(EX_TYPE, errmsg.str());                          \
}

#endif  // GEM_HW_MANAGERS_EXCEPTION_EXCEPTION_H
