#ifndef GEM_BASE_UTILS_EXCEPTION_EXCEPTION_H
#define GEM_BASE_UTILS_EXCEPTION_EXCEPTION_H

#include <string>

#include "gem/utils/exception/Exception.h"

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

#define GEM_BASE_UTILS_DEFINE_EXCEPTION(EXCEPTION_NAME)                 \
  namespace gem {                                                       \
    namespace base {                                                    \
      namespace utils {                                                 \
        namespace exception {                                           \
          class EXCEPTION_NAME : virtual public xcept::Exception        \
            {                                                           \
            public :                                                    \
            EXCEPTION_NAME(std::string name,                            \
                           std::string message,                         \
                           std::string module,                          \
                           int line,                                    \
                           std::string function) :                      \
              xcept::Exception(name, message, module, line, function)   \
                {};                                                     \
            EXCEPTION_NAME(std::string name,                            \
                           std::string message,                         \
                           std::string module,                          \
                           int line,                                    \
                           std::string function,                        \
                           xcept::Exception& err) :                     \
              xcept::Exception(name, message, module, line, function, err) \
                {};                                                     \
            };                                                          \
        }  /* namespace gem::base::utils::exception */                  \
      }  /* namespace gem::base::utils            */                    \
    }  /* namespace gem::base                   */                      \
  }  /* namespace gem                         */

// The gem::base::utils exceptions.
GEM_BASE_UTILS_DEFINE_EXCEPTION(Exception)
GEM_BASE_UTILS_DEFINE_EXCEPTION(CfgParseProblem)
GEM_BASE_UTILS_DEFINE_EXCEPTION(CfgProblem)
GEM_BASE_UTILS_DEFINE_EXCEPTION(CfgValidationProblem)

GEM_BASE_UTILS_DEFINE_EXCEPTION(RCMSNotificationError)
GEM_BASE_UTILS_DEFINE_EXCEPTION(SOAPTransitionProblem)

GEM_BASE_UTILS_DEFINE_EXCEPTION(InfoSpaceProblem)

GEM_BASE_UTILS_DEFINE_EXCEPTION(SoftwareProblem)
GEM_BASE_UTILS_DEFINE_EXCEPTION(TransitionProblem)
GEM_BASE_UTILS_DEFINE_EXCEPTION(ValueError)

// The gem::base::utils alarms.
#define GEM_BASE_UTILS_DEFINE_ALARM(ALARM_NAME) GEM_BASE_UTILS_DEFINE_EXCEPTION(ALARM_NAME)

GEM_BASE_UTILS_DEFINE_ALARM(MonitoringFailureAlarm)

#endif  // GEM_BASE_UTILS_EXCEPTION_EXCEPTION_H
