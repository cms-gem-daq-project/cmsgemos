#
# Makefile for gemhardware package
#

BUILD_HOME?= $(shell dirname `cd ./; pwd`)

ifndef BUILD_SUPPORT
BUILD_SUPPORT=build
endif

ifndef PROJECT_NAME
PROJECT_NAME=cmsgemos
endif

SUBPACKAGES := \
        utils \
        devices \
        managers

# SUBPACKAGES.DOC      := $(patsubst %,%.doc,    ${SUBPACKAGES})
# SUBPACKAGES.CHECK    := $(patsubst %,%.check,    ${SUBPACKAGES})
SUBPACKAGES.DEBUG    := $(patsubst %,%.debug,    ${SUBPACKAGES})
SUBPACKAGES.INSTALL  := $(patsubst %,%.install,  ${SUBPACKAGES})
SUBPACKAGES.RPM      := $(patsubst %,%.rpm,      ${SUBPACKAGES})
SUBPACKAGES.CLEANRPM := $(patsubst %,%.cleanrpm, ${SUBPACKAGES})
SUBPACKAGES.CLEAN    := $(patsubst %,%.clean,    ${SUBPACKAGES})
SUBPACKAGES.TESTS    := $(patsubst %,%.tests,    ${SUBPACKAGES})
SUBPACKAGES.RUNTESTS := $(patsubst %,%.run-tests, ${SUBPACKAGES})
SUBPACKAGES.RUNTESTSCI := $(patsubst %,%.run-tests-ci, ${SUBPACKAGES})

default: all

all: $(SUBPACKAGES)

release: all doc

install: $(LIBDIR) $(SUBPACKAGES) $(SUBPACKAGES.INSTALL)

rpm: $(SUBPACKAGES) $(SUBPACKAGES.RPM)

cleanrpm: $(SUBPACKAGES.CLEANRPM)

clean: $(SUBPACKAGES.CLEAN)

tests: $(SUBPACKAGES) $(SUBPACKAGES.TESTS)

run-tests: $(SUBPACKAGES.RUNTESTS)

run-tests-ci: $(SUBPACKAGES.RUNTESTSCI)

debug: $(SUBPACKAGES.DEBUG)

# check: $(SUBPACKAGES.CHECK)

$(LIBDIR):
	mkdir -p $(LIBDIR)

$(SUBPACKAGES):
	$(MAKE) -C $@

$(SUBPACKAGES.RPM):
	$(MAKE) -C $(patsubst %.rpm,%, $@) rpm

$(SUBPACKAGES.CLEANRPM):
	$(MAKE) -C $(patsubst %.cleanrpm,%, $@) cleanrpm

# $(SUBPACKAGES.CHECK):
# 	$(MAKE) -C $(patsubst %.check,%, $@) check

# $(SUBPACKAGES.DEBUG):
# 	$(MAKE) -C $(patsubst %.debug,%, $@) debug

$(SUBPACKAGES.INSTALL):
	-find  $(patsubst %.install,%, $@)/lib -name *.so -print -exec cp {} ${LIBDIR} \;

$(SUBPACKAGES.CLEAN):
	$(MAKE) -C $(patsubst %.clean,%, $@) clean

$(SUBPACKAGES.TESTS): all
	$(MAKE) -C $(patsubst %.tests,%, $@) tests

$(SUBPACKAGES.RUNTESTS): tests
	$(MAKE) -C $(patsubst %.run-tests,%, $@) run-tests

$(SUBPACKAGES.RUNTESTSCI): tests
	$(MAKE) -C $(patsubst %.run-tests-ci,%, $@) run-tests-ci

.PHONY: $(SUBPACKAGES) $(SUBPACKAGES.INSTALL) $(SUBPACKAGES.CLEAN) $(SUBPACKAGES.CHECK) $(SUBPACKAGES.DEBUG)

#$(SUBPACKAGES.CHECK) $(SUBPACKAGES.DEBUG)

utils: gemutils

devices: gemutils gemhardware/utils

managers: gemutils gembase gemreadout gemhardware/devices

include $(BUILD_HOME)/config/mfDefsGEM.mk
include $(BUILD_HOME)/config/mfPythonDefsGEM.mk

include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/Makefile.rules
include $(BUILD_HOME)/config/mfRPMDefsGEM.mk

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
	@echo GEMDEVLOPER   $(GEMDEVLOPER)
	@echo CC            $(CC)
	@echo CPP           $(CPP)
	@echo CXX           $(CXX)
	@echo LD            $(LD)
	@echo AR            $(AR)
	@echo NM            $(NM)
	@echo RANLIB        $(RANLIB)
	@echo GCCVERSION    $(GCCVERSION)
	@echo CLANGVERSION  $(CLANGVERSION)
	@echo DependentLibraryDirs  $(DependentLibraryDirs)
