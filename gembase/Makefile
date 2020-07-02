#
# Makefile for gembase package
#

Project=cmsgemos
ShortProject=gem
Package=gembase
LongPackage=gembase
ShortPackage=base
PackageName=base

GEMBASE_VER_MAJOR:=$(shell $(BUILD_HOME)/$(Project)/config/build/tag2rel.sh | awk '{split($$0,a," "); print a[1];}' | awk '{split($$0,b,":"); print b[2];}')
GEMBASE_VER_MINOR:=$(shell $(BUILD_HOME)/$(Project)/config/build/tag2rel.sh | awk '{split($$0,a," "); print a[2];}' | awk '{split($$0,b,":"); print b[2];}')
GEMBASE_VER_PATCH:=$(shell $(BUILD_HOME)/$(Project)/config/build/tag2rel.sh | awk '{split($$0,a," "); print a[3];}' | awk '{split($$0,b,":"); print b[2];}')

include $(BUILD_HOME)/$(Project)/config/mfDefsGEM.mk
include $(BUILD_HOME)/$(Project)/config/mfPythonDefsGEM.mk

Sources =version.cc
Sources+=GEMApplication.cc GEMFSMApplication.cc GEMFSM.cc
Sources+=GEMWebApplication.cc GEMMonitor.cc
Sources+=utils/GEMInfoSpaceToolBox.cc

DynamicLibrary=gembase

IncludeDirs+=$(BUILD_HOME)/$(Project)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemutils/include

DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)

DependentLibraries+=gemutils

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
	@echo CC            $(CC)
	@echo CPP           $(CPP)
	@echo CXX           $(CXX)
	@echo LD            $(LD)
	@echo AR            $(AR)
	@echo NM            $(NM)
	@echo RANLIB        $(RANLIB)
	@echo GCCVERSION    $(GCCVERSION)
	@echo CLANGVERSION  $(CLANGVERSION)
