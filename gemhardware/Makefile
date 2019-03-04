#
# Makefile for gemhardware package
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
Package=gemhardware
LongPackage=gemhardware
ShortPackage=hw
PackageName=hw

GEMHARDWARE_VER_MAJOR=1
GEMHARDWARE_VER_MINOR=0
GEMHARDWARE_VER_PATCH=1

include $(BUILD_HOME)/config/mfDefsGEM.mk
include $(BUILD_HOME)/config/mfPythonDefsGEM.mk

include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/Makefile.rules
include $(BUILD_HOME)/config/mfRPMDefsGEM.mk

.PHONY: devices managers _hackery

_hackery:
	if [[ -L Makefile ]]; \
	then \
	unlink Makefile; \
	cp Makefile.progress Makefile; \
	fi

devices:
	$(MAKE) _hackery
	cp Makefile Makefile.progress
	ln -sf Makefile.devices Makefile
	$(MAKE); ret=$$?; \
	$(MAKE) -f Makefile.progress _hackery; \
	exit $$ret

managers: devices
	$(MAKE) _hackery
	cp Makefile Makefile.progress
	ln -sf Makefile.managers Makefile; \
	$(MAKE); ret=$$?; \
	$(MAKE) -f Makefile.progress _hackery; \
	exit $$ret

_all: devices managers

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
	@echo UserCFlags    $(UserCFlags)
	@echo CCVERSION     $(CCVERSION)
	@echo CLANGNUM      $(CLANGNUM)
	@echo GCCNUM        $(GCCNUM)
