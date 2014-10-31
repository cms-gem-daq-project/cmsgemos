#ifndef gem_hw_amc13_exception_Exception_h
#define gem_hw_amc13_exception_Exception_h

#include <string>

#include "gem/hw/exception/Exception.h"

// Mimick XCEPT_DEFINE_EXCEPTION from xcept/Exception.h.
#define GEM_HW_AMC13_DEFINE_EXCEPTION(EXCEPTION_NAME)	                    \
  namespace gem {					                    \
    namespace hw {					                    \
      namespace amc13 {					                    \
	namespace exception {                                               \
	  class EXCEPTION_NAME : public gem::hw::amc13::exception::Exception \
	    {								\
	    public :							\
	    EXCEPTION_NAME(std::string name,				\
			   std::string message,				\
			   std::string module,				\
			   int line,					\
			   std::string function) :			\
	      gem::hw::amc13::exception::Exception(name, message, module, line, function) \
		{};							\
	    EXCEPTION_NAME(std::string name,				\
			   std::string message,				\
			   std::string module,				\
			   int line,					\
			   std::string function,			\
			   gem::hw::exception::Exception& err) :	\
	      gem::hw::amc13::exception::Exception(name, message, module, line, function, err) \
		{};							\
	    };								\
	}                                                               \
      }                                                                 \
    }                                                                   \
  } 



// And a little helper to save us some typing.
#define GEM_HW_AMC13_DEFINE_ALARM(ALARM_NAME) GEM_HW_AMC13_DEFINE_EXCEPTION(ALARM_NAME)

namespace gem {
  namespace hw {
    namespace amc13 {
      namespace exception {
	
	class Exception : public gem::hw::exception::Exception
	  {
	  public:
	  Exception(std::string name,
		    std::string message,
		    std::string module,
		    int line,
		    std::string function) :
	    gem::hw::exception::Exception(name, message, module, line, function)
	      {};
	  Exception(std::string name,
		    std::string message,
		    std::string module,
		    int line,
		    std::string function,
		    gem::hw::exception::Exception& err) :
	    gem::hw::exception::Exception(name, message, module, line, function, err)
	      {};
	  };
	
      } // namespace gem::hw::amc13::exception
    } // namespace gem::hw::amc13
  } // namespace gem::hw
} // namespace gem

// The GEM HW exceptions.
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

// The GEM HW alarms.
GEM_HW_AMC13_DEFINE_ALARM(MonitoringFailureAlarm)

#endif // gem_hw_amc13_exception_Exception_h
