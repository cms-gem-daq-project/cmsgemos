CFLAGS = -g3 -O0 -fno-inline
CXXFLAGS = -g3 -O0 -fno-inline

BUILD_HOME:=$(shell pwd)/../..

include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

Project=gemdaq-testing
Package=gemsupervisor

Sources= GemSupervisorWeb.cc version.cc

DynamicLibrary=gemsupervisor

IncludeDirs = $(uHALROOT)/include
DependentLibraryDirs = $(uHALROOT)/lib

IncludeDirs = \
        $(BUILD_HOME)/$(Project)/$(Package)/include \
        $(XDAQ_ROOT)/include \
        $(uHALROOT)/include

DependentLibraryDirs = \
        $(XDAQ_ROOT)/lib \
        $(uHALROOT)/lib

LibraryDirs = \
        $(XDAQ_ROOT)/lib \
        $(uHALROOT)/lib

UserCFlags = 
UserCCFlags = 
UserDynamicLinkFlags =
UserStaticLinkFlags =
#UserExecutableLinkFlags = -Wl,-rpath-link ${XDAQ_ROOT}/lib -Wl,-rpath-link $(uHALROOT)/lib -Wl,-rpath-link ${HCAL_XDAQ_ROOT}/lib \
#                          -L$(CACTUS_HOME)/lib \
#                          -lreadline -lncurses -lpthread -lboost_filesystem -lboost_regex -lboost_system -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal

DependentLibraries = config xcept boost_system cactus_uhal_uhal
Libraries = config xcept numa toolbox asyncresolv uuid log4cplus

include $(XDAQ_ROOT)/config/Makefile.rules
include $(XDAQ_ROOT)/config/mfRPM.rules
