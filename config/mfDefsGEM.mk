XDAQ_ROOT?=/opt/xdaq
XHAL_ROOT?=/opt/xhal
uHALROOT?=/opt/cactus

ifndef BUILD_SUPPORT
BUILD_SUPPORT=build
endif

include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/mfAutoconf.rules
include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/mfDefs.$(XDAQ_OS)

BUILD_HOME?= $(shell pwd)/..

GIT_VERSION  := $(shell git describe --dirty --always --tags)
GEMDEVELOPER := $(shell id --user --name)
BUILD_VERSION:= $(shell $(BUILD_HOME)/config/build/tag2rel.sh | awk '{split($$0,a," "); print a[4];}' | awk '{split($$0,b,":"); print b[2];}')

Project?=cmsgemos
ShortProject?=gem

## Version variables from Makefile and LongPackage
LongPackageLoc=$(shell echo "$(ShortProject)$(PackageName)" | tr '[:lower:]' '[:upper:]')

ifndef PACKAGE_VER_MAJOR
PACKAGE_VER_MAJOR := $($(LongPackageLoc)_VER_MAJOR)
endif

ifndef PACKAGE_VER_MINOR
PACKAGE_VER_MINOR := $($(LongPackageLoc)_VER_MINOR)
endif

ifndef PACKAGE_VER_PATCH
PACKAGE_VER_PATCH := $($(LongPackageLoc)_VER_PATCH)
endif

ifndef BUILD_VERSION
BUILD_VERSION=1
endif

ifndef PACKAGE_FULL_VERSION
PACKAGE_FULL_VERSION = $(PACKAGE_VER_MAJOR).$(PACKAGE_VER_MINOR).$(PACKAGE_VER_PATCH)
endif

VER_EXISTS=no
ifneq ($(PACKAGE_VER_MAJOR),)
ifneq ($(PACKAGE_VER_MINOR),)
ifneq ($(PACKAGE_VER_PATCH),)
VER_EXISTS=yes
endif
endif
endif

$(info VER_EXISTS $(VER_EXISTS))
$(info PACKAGE_VER_MAJOR $(PACKAGE_VER_MAJOR))
$(info PACKAGE_VER_MINOR $(PACKAGE_VER_MINOR))
$(info PACKAGE_VER_PATCH $(PACKAGE_VER_PATCH))
$(info LongPackageLoc $(LongPackageLoc))
$(info BUILD_HOME $(BUILD_HOME))
$(info BUILD_VERSION $(BUILD_VERSION))

# Tools
MakeDir=mkdir -p

# Testing targets (do nothing by default)
.PHONY: run-tests run-tests-ci
run-tests:
run-tests-ci:
# Clear the default goal
.DEFAULT_GOAL :=

# Debugging and profiling flags
DEBUG_CFlags  = -O0 -g3 -fno-inline
DEBUG_CCFlags = ${DEBUG_CFlags}

## Flags for building ABI check libraries
OPT_CFlags  = -g -Og
OPT_CCFlags = ${DEBUG_CFlags}

COVERAGE_Flags   = -gcov -coverage

PROFILING_Flags   = -pg
PROFILING_LDFlags = ${PROFILING_Flags}
PROFILING_LIBS    = profiler tcmalloc

# Modern compiler support, when available
GCC44Flags = -std=c++0x -std=gnu++0x
GCC47Flags = -std=c++11 -std=gnu++11
GCC48Flags = -std=c++1y -std=gnu++1y
GCC49Flags = -std=c++1z -std=gnu++1z
GCC50Flags = -std=c++17 -std=gnu++17
GCC51Flags = -std=c++17 -std=gnu++17
GCC60Flags = -std=c++17 -std=gnu++17
GCC70Flags = -std=c++17 -std=gnu++17
GCC80Flags = -std=c++2a -std=gnu++2a
GCC90Flags = -std=c++2a -std=gnu++2a

CLANG34Flags = -std=c++1y
CLANG35Flags = -std=c++1z
CLANG36Flags = -std=c++1z
CLANG38Flags = -std=c++1z
CLANG39Flags = -std=c++1z
CLANG40Flags = -std=c++1z
CLANG50Flags = -std=c++17
CLANG60Flags = -std=c++17
CLANG70Flags = -std=c++17

# User C and CC flags
UserCFlags =-DGIT_VERSION=\"$(GIT_VERSION)\" -DGEMDEVELOPER=\"$(GEMDEVELOPER)\" -DXDAQ15

## want to pick the best options based on gcc/clang version
CLANGNUM:= $(shell clang -dumpfullversion -dumpversion|sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g')
GCCNUM  := $(shell gcc   -dumpfullversion -dumpversion|sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g')

$(info Trying to to determine flags for gcc($(GCCNUM))/clang($(CLANGNUM)))
GCCGT440 := $(shell expr ${GCCNUM} \>= 40400)
GCCGT470 := $(shell expr ${GCCNUM} \>= 40700)
GCCGT480 := $(shell expr ${GCCNUM} \>= 40800)
GCCGT490 := $(shell expr ${GCCNUM} \>= 40900)
GCCGT500 := $(shell expr ${GCCNUM} \>= 50000)
GCCGT510 := $(shell expr ${GCCNUM} \>= 50100)
GCCGT600 := $(shell expr ${GCCNUM} \>= 60000)
GCCGT700 := $(shell expr ${GCCNUM} \>= 70000)
GCCGT800 := $(shell expr ${GCCNUM} \>= 80000)
GCCGT900 := $(shell expr ${GCCNUM} \>= 90000)

CLANGGT330 := $(shell expr ${CLANGNUM} \>= 30300)
CLANGGT340 := $(shell expr ${CLANGNUM} \>= 30400)
CLANGGT400 := $(shell expr ${CLANGNUM} \>= 40000)
CLANGGT500 := $(shell expr ${CLANGNUM} \>= 50000)
CLANGGT600 := $(shell expr ${CLANGNUM} \>= 60000)
CLANGGT700 := $(shell expr ${CLANGNUM} \>= 70000)

ifeq ($(GCCGT900), 1)
$(info Using GCC90Flags for GCC $(GCCNUM))
UserCFlags+=${GCC90Flags}
else ifeq ($(GCCGT800), 1)
$(info Using GCC80Flags for GCC $(GCCNUM))
UserCFlags+=${GCC80Flags}
else ifeq ($(GCCGT700), 1)
$(info Using GCC70Flags for GCC $(GCCNUM))
UserCFlags+=${GCC70Flags}
else ifeq ($(GCCGT600), 1)
$(info Using GCC60Flags for GCC $(GCCNUM))
UserCFlags+=${GCC60Flags}
else ifeq ($(GCCGT510), 1)
$(info Using GCC51Flags for GCC $(GCCNUM))
UserCFlags+=${GCC51Flags}
else ifeq ($(GCCGT500), 1)
$(info Using GCC50Flags for GCC $(GCCNUM))
UserCFlags+=${GCC50Flags}
else ifeq ($(GCCGT490), 1)
$(info Using GCC49Flags for GCC $(GCCNUM))
UserCFlags+=${GCC49Flags}
else ifeq ($(GCCGT480), 1)
$(info Using GCC48Flags for GCC $(GCCNUM))
UserCFlags+=${GCC48Flags}
else ifeq ($(GCCGT470), 1)
$(info Using GCC47Flags for GCC $(GCCNUM))
UserCFlags+=${GCC47Flags}
else ifeq ($(GCCGT440), 1)
$(info Using GCC44Flags for GCC $(GCCNUM))
UserCFlags+=${GCC44Flags}
else
$(info Unable to determine flags for gcc($(GCCNUM))/clang($(CLANGNUM)))
endif

## we should really keep these separate, no? C vs C++...
UserCCFlags=${UserCFlags}

##  -Wl,--no-undefined ## not necessarily a good things always
UserStaticLinkFlags  = -Wl,--as-needed -Wl,--warn-unresolved-symbols
UserDynamicLinkFlags = -Wl,--as-needed -Wl,--warn-unresolved-symbols

## Set up include dirs, might be better to do all this setup at a per package level for minimum linking
IncludeDirs+=$(XDAQ_ROOT)/include

# ROOT Config
ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS   = $(shell root-config --libs)
ROOTGLIBS  = $(shell root-config --glibs)

# MySQL Config
MySQLCFLAGS = $(shell mysql_config --cflags)
MySQLLIBS   = $(shell mysql_config --libs)

LibraryDirs+=$(XDAQ_ROOT)/lib
# LibraryDirs+=/usr/local/lib

DependentLibraryDirs+=$(XDAQ_ROOT)/lib
# DependentLibraryDirs+=/usr/local/lib

# Let's clean this up and only link against the requried libs, may require moving non-xdaq dependencies out of certain build chains
## libraries are linked against in a one-pass mode, so least depended to the left, more depended to the right
StandardLibraries =
StandardLibraries+=xdaq2rc xdaq
StandardLibraries+=xoap xdata xerces-c xcept
StandardLibraries+=peer xgi
StandardLibraries+=toolbox config
StandardLibraries+=logudpappender logxmlappender log4cplus
StandardLibraries+=cgicc mimetic numa uuid asyncresolv
# StandardLibraries+=boost_filesystem boost_regex boost_system
# StandardLibraries+=readline ncurses pthread

## DependentLibraries is used for building any shared object
DependentLibraries =

## Libraries is used for building any exe
Libraries =

# UserExecutableLinkFlags+=-Wl,-soname
#UserExecutableLinkFlags+=-Wl,-rpath-link ${XDAQ_ROOT}/lib -Wl,-rpath-link $(uHALROOT)/lib
#UserExecutableLinkFlags+=-Wl,-rpath-link ${HCAL_XDAQ_ROOT}/lib
#UserExecutableLinkFlags+=-L$(CACTUS_HOME)/lib
#UserExecutableLinkFlags+=-lreadline -lncurses -lpthread -lboost_filesystem
#UserExecutableLinkFlags+=-lboost_regex -lboost_system -lcactus_extern_pugixml
#UserExecutableLinkFlags+=-lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal

#include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/Makefile.rules
#include $(BUILD_HOME)/config/mfRPM_gem.rules

## Choose the version of GCC that we want to use
#GCC_VERSION = 6.1.0
#CC     = gcc-${GCC_VERSION}
#CPP    = g++-${GCC_VERSION}
#CXX    = g++-${GCC_VERSION}
#LD     = g++-${GCC_VERSION}
#AR     = ar-${GCC_VERSION}
#NM     = nm-${GCC_VERSION}
#RANLIB = ranlib-${GCC_VERSION}
#LIBDIR = /usr/local/gcc/${GCC_VERSION}/lib:/usr/local/gcc/${GCC_VERSION}/lib64
##If you ever happen to want to link against installed libraries
##in a given directory, LIBDIR, you must either use libtool, and
##specify the full pathname of the library, or use the `-LLIBDIR'
##flag during linking and do at least one of the following:
##   - add LIBDIR to the `LD_LIBRARY_PATH' environment variable
##     during execution
#LibraryDirs=${LIBDIR}:${LibraryDirs}
#DependentLibraryDirs=${LIBDIR}:${DependentLibraryDirs}
##   - add LIBDIR to the `LD_RUN_PATH' environment variable
##     during linking
##LD_RUN_PATH=${LIBDIR}:${LD_RUN_PATH}
##   - use the `-Wl,-rpath -Wl,LIBDIR' linker flag
#UserStaticLinkFlags+=-Wl,-rpath -Wl,${LIBDIR}'
#UserDynamicLinkFlags+=-Wl,-rpath -Wl,${LIBDIR}'
#UserExecutableLinkFlags+=-Wl,-rpath -Wl,${LIBDIR}'

# GCCVERSION = $(shell $(CC) --version | awk '/gcc /{print $0;exit 0;}')
GCCVERSION = $(shell $(CC) -dumpversion)

## Choose the version of Clang that we want to use
##CLANG_VERSION = 3.4.2
#CC     = clang
#CPP    = clang++
#CXX    = clang++
#LD     = clang++
#AR     = llvm-ar
#NM     = llvm-nm
#RANLIB = llvm-ranlib
# CLANGVERSION = $(shell $(CC) --version | awk '/clang /{print $0;exit 0;}')
CLANGVERSION = $(shell $(CC) -dumpfullversion -dumpversion)

CCVERSION = $(shell $(CC) -dumpfullversion -dumpversion)
CPPCHECK  = cppcheck
CPPCHECKFLAGS = --quiet --verbose --inconclusive --force --enable=information,unusedFunction,warning --suppress=purgedConfiguration

CLANGTIDY = clang-tidy
CLANGTIDYFLAGS =

CLANGFORMAT = clang-format
