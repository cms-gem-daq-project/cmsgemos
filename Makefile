#
# Global Makefile for GEM
#

SUBPACKAGES := \
        gemutils \
        gembase \
        gemhardware \
        gemreadout \
        gemsupervisor \
        # gemHwMonitor \

SUBPACKAGES.DEBUG    := $(patsubst %,%.debug,    ${SUBPACKAGES})
SUBPACKAGES.INSTALL  := $(patsubst %,%.install,  ${SUBPACKAGES})
SUBPACKAGES.RPM      := $(patsubst %,%.rpm,      ${SUBPACKAGES})
SUBPACKAGES.CLEANRPM := $(patsubst %,%.cleanrpm, ${SUBPACKAGES})
SUBPACKAGES.CLEAN    := $(patsubst %,%.clean,    ${SUBPACKAGES})

#OS:=linux
#ARCH:=x86_64
#LIBDIR:=lib/$(OS)/$(ARCH)

default: all

all: $(SUBPACKAGES)

release: all doc

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

doc:  $(SUBPACKAGES)
	@echo "Generating doxygen"
	@rm -fr ./doc/html 2> /dev/null
	@doxygen -s ./doc/cmsgemos.cfg  > /dev/null 2>&1
	#@git checkout gh-pages  > /dev/null 2>&1
	#@git add -f ./doc/html  > /dev/null 2>&1
	#@git commit -m "generating doxygen" ./doc/html  > /dev/null 2>&1
	#@git tag ./doc/html  > /dev/null 2>&1

install: $(LIBDIR) $(SUBPACKAGES) $(SUBPACKAGES.INSTALL)

rpm: $(SUBPACKAGES) $(SUBPACKAGES.RPM)

cleanrpm: $(SUBPACKAGES.CLEANRPM)

clean: $(SUBPACKAGES.CLEAN)

$(LIBDIR):
	mkdir -p $(LIBDIR)

$(SUBPACKAGES):
	$(MAKE) -C $@

$(SUBPACKAGES.RPM):
	$(MAKE) -C $(patsubst %.rpm,%, $@) rpm

$(SUBPACKAGES.CLEANRPM):
	$(MAKE) -C $(patsubst %.cleanrpm,%, $@) cleanrpm

$(SUBPACKAGES.INSTALL):
	-find  $(patsubst %.install,%, $@)/lib -name *.so -print -exec cp {} ${LIBDIR} \;

$(SUBPACKAGES.CLEAN):
	$(MAKE) -C $(patsubst %.clean,%, $@) clean

.PHONY: $(SUBPACKAGES) $(SUBPACKAGES.INSTALL) $(SUBPACKAGES.CLEAN)


gemHwMonitor: gemutils gembase gemhardware

gemhardware: gemutils gembase gemreadout

gembase: gemutils

gemsupervisor: gemutils gembase gemhardware gemreadout

gemutils:

gemreadout: gemutils gembase

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
