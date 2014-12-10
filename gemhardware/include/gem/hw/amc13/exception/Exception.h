#ifndef gem_hw_amc13_exception_Exception_h
#define gem_hw_amc13_exception_Exception_h

#include <string>

#include "gem/hw/exception/Exception.h"

#define GEM_HW_AMC13_DEFINE_EXCEPTION(EXCEPTION_NAME)			\
  namespace gem {							\
    namespace hw {							\
      namespace amc13 {							\
	namespace exception {						\
	  class EXCEPTION_NAME : virtual public xcept::Exception	\
	    {								\
	    public :							\
	    EXCEPTION_NAME() :						\
	      xcept::Exception()					\
		{};							\
	    EXCEPTION_NAME(std::string name,				\
			   std::string message,				\
			   std::string module,				\
			   int line,					\
			   std::string function) :			\
	      xcept::Exception(name, message, module, line, function)	\
		{};							\
	    EXCEPTION_NAME(std::string name,				\
			   std::string message,				\
			   std::string module,				\
			   int line,					\
			   std::string function,			\
			   gem::hw::exception::Exception& err) :	\
	      xcept::Exception(name, message, module, line, function, err) \
		{};							\
	    };								\
	}                                                               \
      }                                                                 \
    }                                                                   \
  } 

// The gem::hw::amc13 exceptions.
GEM_HW_AMC13_DEFINE_EXCEPTION(Exception)
GEM_HW_AMC13_DEFINE_EXCEPTION(ConfigurationParseProblem)
GEM_HW_AMC13_DEFINE_EXCEPTION(ConfigurationProblem)
GEM_HW_AMC13_DEFINE_EXCEPTION(ConfigurationValidationProblem)

GEM_HW_AMC13_DEFINE_EXCEPTION(HardwareProblem)
GEM_HW_AMC13_DEFINE_EXCEPTION(NonexistentChannel)

GEM_HW_AMC13_DEFINE_EXCEPTION(RCMSNotificationError)
GEM_HW_AMC13_DEFINE_EXCEPTION(SOAPTransitionProblem)
GEM_HW_AMC13_DEFINE_EXCEPTION(TransitionProblem)

GEM_HW_AMC13_DEFINE_EXCEPTION(SoftwareProblem)
GEM_HW_AMC13_DEFINE_EXCEPTION(ValueError)

// The gem::hw::amc13 alarms.
#define GEM_HW_AMC13_DEFINE_ALARM(ALARM_NAME) GEM_HW_AMC13_DEFINE_EXCEPTION(ALARM_NAME)

GEM_HW_AMC13_DEFINE_ALARM(MonitoringFailureAlarm)

#endif // gem_hw_amc13_exception_Exception_h
