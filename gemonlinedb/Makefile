#
# Makefile for gemonlinedb package
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
Package=gemonlinedb
LongPackage=gemonlinedb
ShortPackage=onlinedb
PackageName=onlinedb

ProjectBase = $(BUILD_HOME)
CfgDir      = $(ProjectBase)/config

GEMONLINEDB_VER_MAJOR=0
GEMONLINEDB_VER_MINOR=1
GEMONLINEDB_VER_PATCH=0

include $(CfgDir)/mfDefsGEM.mk
include $(CfgDir)/mfPythonDefsGEM.mk

Sources = \
    AMC13Configuration.cc \
    AMC13ConfigurationGen.cc \
    AMCConfiguration.cc \
    AMCConfigurationGen.cc \
    ConfigurationLinker.cc \
    ConfigurationManager.cc \
    DBInterface.cc \
    FileConfigurationProvider.cc \
    FileUtils.cc \
    GBTXConfigurationGen.cc \
    OHv3Configuration.cc \
    OHv3ConfigurationGen.cc \
    Run.cc \
    SystemTopology.cc \
    VFAT3ChannelConfigurationGen.cc \
    VFAT3ChipConfigurationGen.cc \
    version.cc \
    XMLUtils.cc \
    GEMOnlineDBManager.cc \
    GEMOnlineDBManagerWeb.cc

DynamicLibrary=gemonlinedb

SimpleTestExecutables = \
    test/testAMC13Configuration.cc \
    test/testConfigurationLinker.cc \
    test/testConfigurationManager.cc \
    test/testFileConfigurationProvider.cc \
    test/testFileUtils.cc \
    test/testSerializationData.cc \
    test/testSystemTopology.cc \
    test/testVFAT3ChannelConfiguration.cc \
    test/testVFAT3ChipConfiguration.cc \

DBTestExecutables = \
    test/testTStoreSerialization.cc \

TestExecutables = $(SimpleTestExecutables) $(DBTestExecutables)

IncludeDirs+=$(BUILD_HOME)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/gembase/include
IncludeDirs+=$(BUILD_HOME)/gemutils/include

UserCFlags+=$(PYTHONCFLAGS)

DependentLibraryDirs+=$(BUILD_HOME)/gembase/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)

DependentLibraries=gembase gemutils xdaq cgicc xgi peer xdata mimetic xoap xerces-c xcept toolbox logudpappender logxmlappender log4cplus config cgicc
DependentLibraries+=boost_filesystem boost_regex boost_system boost_thread

# DependentLibraries+=$(StandardLibraries)

TestLibraries= $(DependentLibraries) boost_unit_test_framework tstoreapi wsaddressing tstoreutils occi clntsh nnz12 ons clntshcore
TestLibraryDirs= $(DependentLibraryDirs)

# Targets with additional dependencies. Keep "default" first
.PHONY: default clean tests run-tests run-tests-ci
default: gen-files
clean: clean-gen-files
tests: default
run-tests: run-tests-simple run-tests-db
run-tests-ci: run-tests-simple

# Common definitions
include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/Makefile.rules
include $(CfgDir)/mfRPMDefsGEM.mk

# Auto-generated files
include/gem/onlinedb/detail/%Gen.h src/common/%Gen.cc: defs/%.json parseDef.py
	python parseDef.py $<

# Make sure to generate the files
GenFiles = include/gem/onlinedb/detail/AMC13ConfigurationGen.h \
           include/gem/onlinedb/detail/AMCConfigurationGen.h \
           include/gem/onlinedb/detail/GBTXConfigurationGen.h \
           include/gem/onlinedb/detail/OHv3ConfigurationGen.h \
           include/gem/onlinedb/detail/VFAT3ChannelConfigurationGen.h \
           include/gem/onlinedb/detail/VFAT3ChipConfigurationGen.h \
           src/common/AMC13ConfigurationGen.cc \
           src/common/AMCConfigurationGen.cc \
           src/common/GBTXConfigurationGen.cc \
           src/common/OHv3ConfigurationGen.cc \
           src/common/VFAT3ChannelConfigurationGen.cc \
           src/common/VFAT3ChipConfigurationGen.cc \

.PHONY: gen-files
gen-files: $(GenFiles)

# Auto-generated headers
.PHONY: clean-gen-files
clean-gen-files:
	$(RM) $(GenFiles)

TEST_ENV = LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/
TEST_LOC = test/$(XDAQ_OS)/$(XDAQ_PLATFORM)
SIMPLE_TEST_EXE = $(SimpleTestExecutables:.cc=.exe)
DB_TEST_EXE = $(DBTestExecutables:.cc=.exe)

.PHONY: run-tests-simple
run-tests-simple:
	@status=0; \
	for test in $(SIMPLE_TEST_EXE); do \
	    echo Testing: $$test; \
	    $(TEST_ENV) $(TEST_LOC)/$$test || set status=1; \
	done; \
	exit $$status

.PHONY: run-tests-db
run-tests-db:
	@status=0; \
	for test in $(DB_TEST_EXE); do \
	    echo Testing: $$test; \
	    $(TEST_ENV) $(TEST_LOC)/$$test || set status=1; \
	done; \
	exit $$status

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
	@echo IncludeDirs   $(IncludeDirs)
	@echo DependentLibraries   $(DependentLibraries)
	@echo DependentLibraryDirs   $(DependentLibraryDirs)
