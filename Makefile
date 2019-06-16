#
# Global Makefile for GEM
#
BUILD_HOME:=$(shell pwd)

SUBPACKAGES := \
        gemutils \
        gembase \
        gemhardware/utils \
        gemhardware/devices \
        gemhardware/managers \
        gemreadout \
        gemsupervisor \
        gempython \
        gemdaqmonitor \
        gemonlinedb \
        # gemHwMonitor \

# SUBPACKAGES.DOC      := $(patsubst %,%.doc,    ${SUBPACKAGES})
# SUBPACKAGES.CHECK    := $(patsubst %,%.check,    ${SUBPACKAGES})
SUBPACKAGES.DEBUG    := $(patsubst %,%.debug,    ${SUBPACKAGES})
SUBPACKAGES.INSTALL  := $(patsubst %,%.install,  ${SUBPACKAGES})
SUBPACKAGES.RPM      := $(patsubst %,%.rpm,      ${SUBPACKAGES})
SUBPACKAGES.CLEANRPM := $(patsubst %,%.cleanrpm, ${SUBPACKAGES})
SUBPACKAGES.CLEAN    := $(patsubst %,%.clean,    ${SUBPACKAGES})
SUBPACKAGES.TESTS    := $(patsubst %,%.tests,    ${SUBPACKAGES})
SUBPACKAGES.ABICHECK := $(patsubst %,%.check-abi, ${SUBPACKAGES})
SUBPACKAGES.RUNTESTS := $(patsubst %,%.run-tests, ${SUBPACKAGES})
SUBPACKAGES.RUNTESTSCI := $(patsubst %,%.run-tests-ci, ${SUBPACKAGES})

#OS:=linux
#ARCH:=x86_64
#LIBDIR:=lib/$(OS)/$(ARCH)

default: all

all: $(SUBPACKAGES)

release: all doc rpm

gcc47: UserCFlags+=${GCC47Flags}
gcc47: UserCCFlags+=${GCC47Flags}
gcc47: $(SUBPACKAGES)

gcc48: UserCFlags+=${GCC47Flags}
gcc48: UserCFlags+=${GCC48Flags}
gcc48: UserCCFlags+=${GCC47Flags}
gcc48: UserCCFlags+=${GCC48Flags}
gcc48: $(SUBPACKAGES)

gcc49: UserCFlags+=${GCC47Flags}
gcc49: UserCFlags+=${GCC48Flags}
gcc49: UserCFlags+=${GCC49Flags}
gcc49: UserCCFlags+=${GCC47Flags}
gcc49: UserCCFlags+=${GCC48Flags}
gcc49: UserCCFlags+=${GCC49Flags}
gcc49: $(SUBPACKAGES)

debug: UserCFlags+=${DEBUG_CFlags}
debug: UserCCFlags+=${DEBUG_CCFlags}
debug: $(SUBPACKAGES)

profile: UserCFlags+=${PROFILING_Flags}
profile: UserCCFlags+=${PROFILING_Flags}
profile: UserDynamicLinkFlags+=${PROFILING_LDFlags}
profile: DependentLibraries+=${PROFILING_LIBS}
profile: $(SUBPACKAGES)

dbgprofile: UserCFlags+=${DEBUG_CFlags} ${PROFILING_Flags}
dbgprofile: UserCCFlags+=${DEBUG_CCFlags} ${PROFILING_Flags}
dbgprofile: UserDynamicLinkFlags+=${PROFILING_LDFlags}
dbgprofile: DependentLibraries+=${PROFILING_LIBS}
dbgprofile: $(SUBPACKAGES)

.PHONY: doc
doc:
	@echo "Generating doxygen"
	@mkdir ./doc/build
	@rm -fr ./doc/build/* 2> /dev/null
	@doxygen -s ./doc/cmsgemos.cfg
#	@git checkout gh-pages  > /dev/null 2>&1
#	@git add -f ./doc/html  > /dev/null 2>&1
#	@git commit -m "generating doxygen" ./doc/html  > /dev/null 2>&1
#	@git tag ./doc/html  > /dev/null 2>&1

install: $(LIBDIR) $(SUBPACKAGES) $(SUBPACKAGES.INSTALL)

rpm: $(SUBPACKAGES) $(SUBPACKAGES.RPM)

cleanrpm: $(SUBPACKAGES.CLEANRPM)

clean: $(SUBPACKAGES.CLEAN)

tests: $(SUBPACKAGES) $(SUBPACKAGES.TESTS)

check-abi: $(SUBPACKAGES)
	mkdir -p libs-git$(git describe --dirty --always --tags)
	find gem*/lib/* -inmae '*.so' -print0 -exec cp -t libs-git$(git describe --dirty --always --tags) -rfp {} \+
#	make clean -j8

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

# $(SUBPACKAGES.ABICHECK): tests
# find all generated libraries and copy to directory
# $(MakeDir) libs-git$(GIT_VERSION)
# find gem*/lib/* -inmae '*.so' -print0 -exec cp -t libs-git$(GIT_VERSION) -rfp {} \+
# make clean -j8
# check out target commit
# make -j8
# find all generated libraries and copy to directory
# $(MAKE) -C $(patsubst %.check-abi,%, $@) check-abi

$(SUBPACKAGES.RUNTESTS): tests
	$(MAKE) -C $(patsubst %.run-tests,%, $@) run-tests

$(SUBPACKAGES.RUNTESTSCI): tests
	$(MAKE) -C $(patsubst %.run-tests-ci,%, $@) run-tests-ci

.PHONY: $(SUBPACKAGES) $(SUBPACKAGES.INSTALL) $(SUBPACKAGES.CLEAN) $(SUBPACKAGES.CHECK) $(SUBPACKAGES.DEBUG)

#$(SUBPACKAGES.CHECK) $(SUBPACKAGES.DEBUG)
.PHONY: gemhardware

gemhardware: gemhardware/utils gemhardware/devices gemhardware/managers

gemhardware/utils: gemutils

gemhardware/devices: gemutils gemhardware/utils

gemhardware/managers: gemutils gembase gemreadout gemhardware/devices

gemHwMonitor: gemutils gembase gemhwdevices

## only gemhardware.devices... how to fix this?
gempython: gemhardware/devices

gembase: gemutils

gemsupervisor: gemutils gembase gemhardware gemreadout

gemutils:

gemonlinedb: gemutils gembase

gemreadout: gemutils gembase gemhardware/devices

gemdaqmonitor: gembase gemhardware/devices gemhardware/utils

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
