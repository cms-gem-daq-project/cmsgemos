/** @file Exception.h */

#ifndef GEM_HW_VFAT_EXCEPTION_EXCEPTION_H
#define GEM_HW_VFAT_EXCEPTION_EXCEPTION_H

#include <string>

#include "gem/hw/exception/Exception.h"

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

#define GEM_HW_VFAT_DEFINE_EXCEPTION(EXCEPTION_NAME)                    \
  namespace gem {                                                       \
    namespace hw {                                                      \
      namespace vfat {                                                  \
        namespace exception {                                           \
          class EXCEPTION_NAME : virtual public xcept::Exception        \
            {                                                           \
            public :                                                    \
            EXCEPTION_NAME() :                                          \
              xcept::Exception()                                        \
                {};                                                     \
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
                           gem::hw::exception::Exception& err) :        \
              xcept::Exception(name, message, module, line, function, err) \
                {};                                                     \
            };                                                          \
        }/* namespace gem::hw::vfat::exception */                       \
      }/* namespace gem::hw::vfat            */                         \
    }/* namespace gem::hw                  */                           \
  }/* namespace gem                      */

// The gem::hw::vfat exceptions.
GEM_HW_VFAT_DEFINE_EXCEPTION(Exception)
GEM_HW_VFAT_DEFINE_EXCEPTION(VFATCfgProblem)
GEM_HW_VFAT_DEFINE_EXCEPTION(VFATCfgParseProblem)
GEM_HW_VFAT_DEFINE_EXCEPTION(VFATCfgValidationProblem)

GEM_HW_VFAT_DEFINE_EXCEPTION(VFATHwProblem)
GEM_HW_VFAT_DEFINE_EXCEPTION(TransactionError)
GEM_HW_VFAT_DEFINE_EXCEPTION(InvalidTransaction)
GEM_HW_VFAT_DEFINE_EXCEPTION(WrongTransaction)
GEM_HW_VFAT_DEFINE_EXCEPTION(NonexistentChannel)

GEM_HW_VFAT_DEFINE_EXCEPTION(RCMSNotificationError)
GEM_HW_VFAT_DEFINE_EXCEPTION(SOAPTransitionProblem)
GEM_HW_VFAT_DEFINE_EXCEPTION(TransitionProblem)

GEM_HW_VFAT_DEFINE_EXCEPTION(SoftwareProblem)
GEM_HW_VFAT_DEFINE_EXCEPTION(ValueError)

// The gem::hw alarms.
#define GEM_HW_VFAT_DEFINE_ALARM(ALARM_NAME) GEM_HW_VFAT_DEFINE_EXCEPTION(ALARM_NAME)

GEM_HW_VFAT_DEFINE_ALARM(MonitoringFailureAlarm)

#endif  // GEM_HW_VFAT_EXCEPTION_EXCEPTION_H
