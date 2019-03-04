/** @file Exception.h */

#ifndef GEM_HW_DEVICES_EXCEPTION_EXCEPTION_H
#define GEM_HW_DEVICES_EXCEPTION_EXCEPTION_H

#include <string>

#include "xcept/Exception.h"

/***
 // Macros defined in xdaq code that are useful to remember
 //! Macro to throw an excpetion with line number and function name
 #define XCEPT_RAISE( EXCEPTION, MSG ) \
 throw EXCEPTION( #EXCEPTION, MSG, __FILE__, __LINE__, __FUNCTION__)

 #define XCEPT_RETHROW( EXCEPTION, MSG, PREVIOUS ) \
 throw EXCEPTION( #EXCEPTION, MSG, __FILE__, __LINE__, __FUNCTION__, PREVIOUS)

 #define XCEPT_ASSERT(COND, EXCEPTION, MSG) \
 if (!(COND)) \
 {\
 XCEPT_RAISE(EXCEPTION, MSG);\
 }

 // Create a new exception and use
 // it as a variable called VAR
 #define XCEPT_DECLARE( EXCEPTION, VAR, MSG )				\
 EXCEPTION VAR( #EXCEPTION, MSG, __FILE__, __LINE__, __FUNCTION__)

 // Create a new exception from a previous one and use
 // it as a variable called VAR
 #define XCEPT_DECLARE_NESTED( EXCEPTION, VAR, MSG, PREVIOUS )		\
 EXCEPTION VAR( #EXCEPTION, MSG, __FILE__, __LINE__, __FUNCTION__, PREVIOUS)
***/

#define GEM_HW_DEFINE_EXCEPTION(EXCEPTION_NAME)                          \
  namespace gem {                                                        \
    namespace hw {                                                       \
      namespace devices {                                               \
        namespace exception {                                           \
          class EXCEPTION_NAME : virtual public xcept::Exception        \
          {                                                             \
          public :                                                      \
          EXCEPTION_NAME() :                                            \
            xcept::Exception()                                          \
              {};                                                       \
          EXCEPTION_NAME(std::string name,                              \
                         std::string message,                           \
                         std::string module,                            \
                         int line,                                      \
                         std::string function) :                        \
            xcept::Exception(name, message, module, line, function)     \
              {};                                                       \
          EXCEPTION_NAME(std::string name,                              \
                         std::string message,                           \
                         std::string module,                            \
                         int line,                                      \
                         std::string function,                          \
                         xcept::Exception& err) :                       \
            xcept::Exception(name, message, module, line, function, err) \
              {};                                                       \
          };                                                            \
        }  /* namespace gem::hw::devices::exception */                  \
      }  /* namespace gem::hw::devices */                               \
    }  /* namespace gem::hw            */                               \
  }  /* namespace gem                */

// The gem::hw exceptions.
GEM_HW_DEFINE_EXCEPTION(Exception)
GEM_HW_DEFINE_EXCEPTION(WriteValueMismatch)
GEM_HW_DEFINE_EXCEPTION(ConfigurationParseProblem)
GEM_HW_DEFINE_EXCEPTION(ConfigurationProblem)
GEM_HW_DEFINE_EXCEPTION(ConfigurationValidationProblem)

GEM_HW_DEFINE_EXCEPTION(HardwareProblem)
GEM_HW_DEFINE_EXCEPTION(UninitializedDevice)

GEM_HW_DEFINE_EXCEPTION(RCMSNotificationError)
GEM_HW_DEFINE_EXCEPTION(SOAPTransitionProblem)
GEM_HW_DEFINE_EXCEPTION(NULLReadoutPointer)

GEM_HW_DEFINE_EXCEPTION(SoftwareProblem)
GEM_HW_DEFINE_EXCEPTION(TransitionProblem)
GEM_HW_DEFINE_EXCEPTION(ValueError)

GEM_HW_DEFINE_EXCEPTION(RPCMethodError)

GEM_HW_DEFINE_EXCEPTION(DeviceNameParseError)

// The gem::hw alarms.
#define GEM_HW_DEFINE_ALARM(ALARM_NAME) GEM_HW_DEFINE_EXCEPTION(ALARM_NAME)

GEM_HW_DEFINE_ALARM(MonitoringFailureAlarm)

/**
 * Macros to catch standard cases
 * @param base of the error message
 * @param exception type that is emitted
 */

// FIXME: SHOULD I BE A MACRO OR A FUNCTION ELSEWHERE?
#define GEM_CATCH_RPC_ERROR(MSG_BASE, EX_TYPE)                 \
catch (xhal::utils::XHALRPCNotConnectedException const& e) {   \
  std::stringstream errmsg;                                    \
  errmsg << e.what();                                          \
  CMSGEMOS_ERROR(MSG_BASE << "error: " << errmsg.str());       \
  XCEPT_RAISE(EX_TYPE, errmsg.str());                          \
} catch (xhal::utils::XHALRPCException const& e) {             \
  std::stringstream errmsg;                                    \
  errmsg << e.what();                                          \
  CMSGEMOS_ERROR(MSG_BASE << "error: " << errmsg.str());       \
  XCEPT_RAISE(EX_TYPE, errmsg.str());                          \
} catch (gem::hw::exception::RPCMethodError const& e) {        \
  std::stringstream errmsg;                                    \
  errmsg << e.what();                                          \
  CMSGEMOS_ERROR(MSG_BASE << "error: " << errmsg.str());       \
  XCEPT_RAISE(EX_TYPE, errmsg.str());                          \
}

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

#endif  // GEM_HW_DEVICES_EXCEPTION_EXCEPTION_H
