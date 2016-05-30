#ifndef GEM_UTILS_GEMLOGGING_H
#define GEM_UTILS_GEMLOGGING_H

#include "log4cplus/logger.h"
#include "log4cplus/loglevel.h"
#include "log4cplus/loggingmacros.h"

namespace gem {
#define TRACE(MSG) LOG4CPLUS_TRACE(m_gemLogger, MSG)
#define DEBUG(MSG) LOG4CPLUS_DEBUG(m_gemLogger, MSG)
#define INFO( MSG) LOG4CPLUS_INFO( m_gemLogger, MSG)
#define WARN( MSG) LOG4CPLUS_WARN( m_gemLogger, MSG)
#define ERROR(MSG) LOG4CPLUS_ERROR(m_gemLogger, MSG)
#define FATAL(MSG) LOG4CPLUS_FATAL(m_gemLogger, MSG)
  
#define TRACE_LOGGER(LOGGER, MSG) LOG4CPLUS_TRACE(LOGGER, MSG)
#define DEBUG_LOGGER(LOGGER, MSG) LOG4CPLUS_DEBUG(LOGGER, MSG)
#define INFO_LOGGER( LOGGER, MSG) LOG4CPLUS_INFO( LOGGER, MSG)
#define WARN_LOGGER( LOGGER, MSG) LOG4CPLUS_WARN( LOGGER, MSG)
#define ERROR_LOGGER(LOGGER, MSG) LOG4CPLUS_ERROR(LOGGER, MSG)
#define FATAL_LOGGER(LOGGER, MSG) LOG4CPLUS_FATAL(LOGGER, MSG)

  //generic function to trace hierarchy in the Logger objects from non-xdaq applications
  //copied from HCAL hcalHTR.cc
  //  void setLogger(const log4cplus::Logger& parentLogger) {
  //    m_logger = log4cplus::Logger::getInstance(buildLogName(parentLogger,m_slot));
  //  }

}

#endif  // GEM_UTILS_GEMLOGGING_H
