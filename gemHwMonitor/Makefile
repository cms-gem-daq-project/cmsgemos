#CFLAGS = -g3 -O0 -fno-inline
#CXXFLAGS = -g3 -O0 -fno-inline

BUILD_HOME:=$(shell pwd)/../..

include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

#Project=gem
Project=gemdaq-testing
Package=gemHwMonitor
LongPackage=gemHwMonitor
ShortPackage=hwMonitor
PackageName=hwMonitor

Sources= gemHwMonitorWeb.cc gemHwMonitorBase.cc
#Sources= gemHwMonitorWeb.cc 

DynamicLibrary=gem_hwMonitor
#IncludeDirs = $(uHALROOT)/include
#DependentLibraryDirs = $(uHALROOT)/lib

IncludeDirs = \
        $(BUILD_HOME)/$(Project)/$(Package)/include \
        $(XDAQ_ROOT)/include \
        $(uHALROOT)/include \
        $(BUILD_HOME)/$(Project)/gembase/include \

DependentLibraryDirs = \
        $(XDAQ_ROOT)/lib \
        $(uHALROOT)/lib \
        $(BUILD_HOME)/$(Project)/gembase/lib/linux/x86_64_slc6 \

LibraryDirs = \
        $(XDAQ_ROOT)/lib \
        $(uHALROOT)/lib \
        $(BUILD_HOME)/$(Project)/gembase/lib/linux/x86_64_slc6 \

UserCFlags = -O0 -g3 -fno-inline
# -std=c++0x
UserCCFlags = -O0 -g3 -fno-inline -std=c++0x
# -std=c++0x
UserDynamicLinkFlags = 
#UserDynamicLinkFlags = -Wl --no-undefined
UserStaticLinkFlags =
#UserExecutableLinkFlags = -Wl,-rpath-link ${XDAQ_ROOT}/lib -Wl,-rpath-link $(uHALROOT)/lib -Wl,-rpath-link ${HCAL_XDAQ_ROOT}/lib \
#                          -L$(CACTUS_HOME)/lib \
#                          -lreadline -lncurses -lpthread -lboost_filesystem -lboost_regex -lboost_system -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal

#DependentLibraries = config xcept boost_system cactus_uhal_uhal libgem_base
DependentLibraries = config xcept boost_system cactus_uhal_uhal
Libraries = config xcept numa toolbox xdaq2rc asyncresolv uuid log4cplus cactus_uhal_uhal

include $(XDAQ_ROOT)/config/Makefile.rules
include $(XDAQ_ROOT)/config/mfRPM.rules
