#ifndef gem_base_utils_GEMLogging_h
#define gem_base_utils_GEMLogging_h

#define DEBUG(MSG) LOG4CPLUS_DEBUG(gemLogger_ , MSG)
#define INFO(MSG)  LOG4CPLUS_INFO( gemLogger_ , MSG)
#define WARN(MSG)  LOG4CPLUS_WARN( gemLogger_ , MSG)
#define ERROR(MSG) LOG4CPLUS_ERROR(gemLogger_ , MSG)
#define FATAL(MSG) LOG4CPLUS_FATAL(gemLogger_ , MSG)

#endif
