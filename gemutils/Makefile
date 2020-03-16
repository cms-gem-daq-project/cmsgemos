#
# Makefile for gemutils package
#

BUILD_HOME:=$(shell pwd)/..

ifndef BUILD_SUPPORT
BUILD_SUPPORT=build
endif

ifndef PROJECT_NAME
PROJECT_NAME=cmsgemos
endif

Project=$(PROJECT_NAME)
ShortProject=gem
Package=gemutils
LongPackage=gemutils
ShortPackage=utils
PackageName=utils

GEMUTILS_VER_MAJOR=1
GEMUTILS_VER_MINOR=0
GEMUTILS_VER_PATCH=1

include $(BUILD_HOME)/config/mfDefsGEM.mk

Sources =version.cc
Sources+=Lock.cc
Sources+=GEMRegisterUtils.cc
Sources+=soap/GEMSOAPToolBox.cc
# Sources+=gemXMLparser.cc

DynamicLibrary=gemutils

IncludeDirs =$(XDAQ_ROOT)/include
IncludeDirs+=$(BUILD_HOME)/$(Package)/include

DependentLibraries =xdaq peer xgi xdata xoap toolbox config xcept
DependentLibraries+=xerces-c logudpappender logxmlappender log4cplus
DependentLibraries+=asyncresolv uuid cgicc mimetic

# UserDynamicLinkFlags+=

include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/Makefile.rules
include $(BUILD_HOME)/config/mfRPMDefsGEM.mk

check:
	$(CPPCHECK) $(CPPCHECKFLAGS) --xml $(addprefix -I,$(IncludeDirs)) src/common 2> cppcheck.gembase.xml; \
	$(CPPCHECK) $(CPPCHECKFLAGS) $(addprefix -I,$(IncludeDirs)) src/common 2> cppcheck.gembase.log

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
	@echo IncludeDirs   $(IncludeDirs)
	@echo LibraryDirs   $(LibraryDirs)
	@echo DependentLibraryDirs $(DependentLibraryDirs)
	@echo DependentLibraries   $(DependentLibraries)
