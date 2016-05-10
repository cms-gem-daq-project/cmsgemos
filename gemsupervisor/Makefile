#
# Makefile for gemsupervisor package
#

#Project=gem
Project=gemdaq-testing
ShortProject=gem
Package=gemsupervisor
LongPackage=gemsupervisor
ShortPackage=supervisor
PackageName=supervisor

GEMSUPERVISOR_VER_MAJOR=0
GEMSUPERVISOR_VER_MINOR=1
GEMSUPERVISOR_VER_PATCH=0

include $(BUILD_HOME)/$(Project)/config/mfDefs.gem

Sources =version.cc
Sources+=tbutils/VFAT2XMLParser.cc
Sources+=tbutils/GEMTBUtil.cc tbutils/ThresholdScan.cc tbutils/LatencyScan.cc 
Sources+=GEMSupervisor.cc GEMSupervisorWeb.cc GEMSupervisorMonitor.cc GEMGlobalState.cc
#Sources+=tbutils/ADCScan.cc
#Sources+=GEMGLIBSupervisorWeb.cc

DynamicLibrary=gemsupervisor

IncludeDirs+=$(BUILD_HOME)/$(Project)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gembase/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemutils/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemhardware/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemreadout/include
IncludeDirs+=$(BUILD_HOME)/gem-light-dqm/dqm-root/include
IncludeDirs+=$(uHALROOT)/include
IncludeDirs+=$(ROOTDIR)/include 

DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemhardware/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemreadout/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(uHALROOT)/lib

LibraryDirs+=$(uHALROOT)/lib

DependentLibraries+=cactus_uhal_uhal cactus_amc13_amc13
DependentLibraries+=gemhardware gemutils gemreadout

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/config/mfRPM_gem.rules
include $(XDAQ_ROOT)/config/mfRPM.rules

print-env:
	@echo BUILD_HOME    $(BUILD_HOME)
	@echo XDAQ_ROOT     $(XDAQ_ROOT)
	@echo XDAQ_OS       $(XDAQ_OS)
	@echo XDAQ_PLATFORM $(XDAQ_PLATFORM)
	@echo LIBDIR        $(LIBDIR)
	@echo ROOTCFLAGS    $(ROOTCFLAGS)
	@echo ROOTLIBS      $(ROOTLIBS)
	@echo ROOTGLIBS     $(ROOTGLIBS)
	@echo GIT_VERSION   $(GIT_VERSION)
	@echo GEMDEVELOPER  $(GEMDEVELOPER)
