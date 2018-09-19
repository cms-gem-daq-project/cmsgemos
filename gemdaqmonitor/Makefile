#
# Makefile for gemdaqmonitor package
#

#Project=gem
Project=cmsgemos
ShortProject=gem
Package=gemdaqmonitor
LongPackage=gemdaqmonitor
ShortPackage=daqmon
PackageName=daqmonitor

GEMDAQMONITOR_VER_MAJOR=0
GEMDAQMONITOR_VER_MINOR=2
GEMDAQMONITOR_VER_PATCH=0

include $(BUILD_HOME)/$(Project)/config/mfDefsGEM.mk
include $(BUILD_HOME)/$(Project)/config/mfPythonDefsGEM.mk

Sources =DaqMonitor.cc version.cc

DynamicLibrary=gemdaqmonitor

IncludeDirs+=$(BUILD_HOME)/$(Project)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gembase/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemutils/include
IncludeDirs+=/opt/xhal/include

DependentLibraryDirs+=/opt/xhal/lib
DependentLibraries+=xhal

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/config/mfRPMDefsGEM.mk

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
