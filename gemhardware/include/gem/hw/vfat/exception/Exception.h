#ifndef gem_hw_vfat_exception_Exception_h_
#define gem_hw_vfat_exception_Exception_h_

#include <string>

#include "xcept/Exception.h"

// Mimick XCEPT_DEFINE_EXCEPTION from xcept/Exception.h.
#define GEM_HW_VFAT_DEFINE_EXCEPTION(EXCEPTION_NAME)	                    \
  namespace gem {					                    \
    namespace hw {					                    \
      namespace vfat {					                    \
	namespace exception {                                               \
	  class EXCEPTION_NAME : public gem::hw::vfat::exception::Exception \
	    {								\
	    public :							\
	    EXCEPTION_NAME(std::string name,				\
			   std::string message,				\
			   std::string module,				\
			   int line,					\
			   std::string function) :			\
	      gem::hw::vfat::exception::Exception(name, message, module, line, function) \
		{};							\
	    EXCEPTION_NAME(std::string name,				\
			   std::string message,				\
			   std::string module,				\
			   int line,					\
			   std::string function,			\
			   xcept::Exception& err) :			\
	      gem::hw::vfat::exception::Exception(name, message, module, line, function, err) \
		{};							\
	    };								\
	}                                                               \
      }                                                                 \
    }                                                                   \
  } 



// And a little helper to save us some typing.
#define GEM_HW_VFAT_DEFINE_ALARM(ALARM_NAME) GEM_HW_VFAT_DEFINE_EXCEPTION(ALARM_NAME)

namespace gem {
  namespace hw {
    namespace vfat {
      namespace exception {
	
	class Exception : public xcept::Exception
	  {
	  public:
	  Exception(std::string name,
		    std::string message,
		    std::string module,
		    int line,
		    std::string function) :
	    xcept::Exception(name, message, module, line, function)
	      {};
	  Exception(std::string name,
		    std::string message,
		    std::string module,
		    int line,
		    std::string function,
		    xcept::Exception& err) :
	    xcept::Exception(name, message, module, line, function, err)
	      {};
	  };
	
      } // namespace gem::hw::vfat::exception
    } // namespace gem::hw::vfat
  } // namespace gem::hw
} // namespace gem

// The GEM HW exceptions.
GEM_HW_VFAT_DEFINE_EXCEPTION(ConfigurationParseProblem)
GEM_HW_VFAT_DEFINE_EXCEPTION(ConfigurationProblem)
GEM_HW_VFAT_DEFINE_EXCEPTION(ConfigurationValidationProblem)

GEM_HW_VFAT_DEFINE_EXCEPTION(HardwareProblem)
GEM_HW_VFAT_DEFINE_EXCEPTION(NonexistentChannel)

GEM_HW_VFAT_DEFINE_EXCEPTION(RCMSNotificationError)
GEM_HW_VFAT_DEFINE_EXCEPTION(SOAPTransitionProblem)
GEM_HW_VFAT_DEFINE_EXCEPTION(TransitionProblem)

GEM_HW_VFAT_DEFINE_EXCEPTION(SoftwareProblem)
GEM_HW_VFAT_DEFINE_EXCEPTION(ValueError)

// The GEM HW alarms.
GEM_HW_VFAT_DEFINE_ALARM(MonitoringFailureAlarm)

#endif // gem_hw_vfat_exception_Exception_h_
