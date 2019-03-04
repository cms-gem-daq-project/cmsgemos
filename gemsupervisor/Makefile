#
# Makefile for gemsupervisor package
#

Project=cmsgemos
ShortProject=gem
Package=gemsupervisor
LongPackage=gemsupervisor
ShortPackage=supervisor
PackageName=supervisor

GEMSUPERVISOR_VER_MAJOR=1
GEMSUPERVISOR_VER_MINOR=0
GEMSUPERVISOR_VER_PATCH=1

include $(BUILD_HOME)/$(Project)/config/mfDefsGEM.mk
include $(BUILD_HOME)/$(Project)/config/mfPythonDefsGEM.mk

Sources =version.cc
Sources+=tbutils/VFAT2XMLParser.cc
# Sources+=tbutils/GEMTBUtil.cc tbutils/ThresholdScan.cc tbutils/LatencyScan.cc
Sources+=GEMSupervisor.cc GEMSupervisorWeb.cc GEMSupervisorMonitor.cc GEMGlobalState.cc
#Sources+=tbutils/ADCScan.cc
#Sources+=GEMGLIBSupervisorWeb.cc

DynamicLibrary=gemsupervisor

IncludeDirs+=$(BUILD_HOME)/$(Project)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gembase/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemutils/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemhardware/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemreadout/include
IncludeDirs+=$(uHALROOT)/include
IncludeDirs+=$(XHAL_ROOT)/include

DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemhardware/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemreadout/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gembase/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(uHALROOT)/lib
DependentLibraryDirs+=$(XHAL_ROOT)/lib

UserCFlags  +=$(PYTHONCFLAGS)
UserCCFlags +=$(PYTHONCFLAGS)

DependentLibraries =gemhardware_managers xdaq2rc

## not building an exe
Libraries =

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/config/mfRPMDefsGEM.mk


print-env:
	@echo BUILD_HOME    $(BUILD_HOME)
	@echo XDAQ_ROOT     $(XDAQ_ROOT)
	@echo XDAQ_OS       $(XDAQ_OS)
	@echo XDAQ_PLATFORM $(XDAQ_PLATFORM)
	@echo PYTHON_VERSION $(PYTHON_VERSION)
	@echo LIBDIR        $(LIBDIR)
	@echo ROOTCFLAGS    $(ROOTCFLAGS)
	@echo ROOTLIBS      $(ROOTLIBS)
	@echo ROOTGLIBS     $(ROOTGLIBS)
	@echo GIT_VERSION   $(GIT_VERSION)
	@echo GEMDEVELOPER  $(GEMDEVELOPER)
	@echo CC            $(CC)
	@echo CPP           $(CPP)
	@echo CXX           $(CXX)
	@echo LD            $(LD)
	@echo AR            $(AR)
	@echo NM            $(NM)
	@echo RANLIB        $(RANLIB)
	@echo GCCVERSION    $(GCCVERSION)
	@echo CLANGVERSION  $(CLANGVERSION)
