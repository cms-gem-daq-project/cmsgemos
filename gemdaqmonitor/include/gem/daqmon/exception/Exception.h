/** @file gem/daqmon/exception/Exception.h */

#ifndef GEM_DAQMON_EXCEPTION_EXCEPTION_H
#define GEM_DAQMON_EXCEPTION_EXCEPTION_H

#include <string>

#include "gem/utils/exception/Exception.h"

// The gem::daqmon exceptions.
#define GEM_DAQMON_DEFINE_EXCEPTION(EXCEPTION_NAME) GEM_DEFINE_EXCEPTION(EXCEPTION_NAME, daqmon)
GEM_DAQMON_DEFINE_EXCEPTION(Exception)
GEM_DAQMON_DEFINE_EXCEPTION(SoftwareProblem)
GEM_DAQMON_DEFINE_EXCEPTION(ValueError)

// The gem::daqmon alarms.
#define GEM_DAQMON_DEFINE_ALARM(ALARM_NAME) GEM_DAQMON_DEFINE_EXCEPTION(ALARM_NAME)
GEM_DAQMON_DEFINE_ALARM(MonitoringFailureAlarm)

#endif  // GEM_DAQMON_EXCEPTION_EXCEPTION_H
