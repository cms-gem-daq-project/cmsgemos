#
# Makefile for gemonlinedb package
#

#Project=gem
Project=cmsgemos
ShortProject=gem
Package=gemonlinedb
LongPackage=gemonlinedb
ShortPackage=onlinedb
PackageName=onlinedb

GEMONLINEDB_VER_MAJOR=0
GEMONLINEDB_VER_MINOR=1
GEMONLINEDB_VER_PATCH=0

include $(BUILD_HOME)/$(Project)/config/mfDefsGEM.mk
include $(BUILD_HOME)/$(Project)/config/mfPythonDefsGEM.mk

Sources = \
    AMC13Configuration.cc \
    AMCConfiguration.cc \
    AMCConfigurationGen.cc \
    ConfigurationLinker.cc \
    ConfigurationManager.cc \
    DBInterface.cc \
    FileUtils.cc \
    GBTXConfigurationGen.cc \
    OHv3Configuration.cc \
    OHv3ConfigurationGen.cc \
    Run.cc \
    SystemTopology.cc \
    VFAT3ChannelConfiguration.cc \
    VFAT3ChipConfigurationGen.cc \
    version.cc \
    XMLConfigurationProvider.cc \
    XMLSerializationData.cc \
    XMLUtils.cc \
    GEMOnlineDBManager.cc \
    GEMOnlineDBManagerWeb.cc

DynamicLibrary=gemonlinedb

SimpleTestExecutables = \
    test/testAMC13Configuration.cc \
    test/testConfigurationLinker.cc \
    test/testConfigurationManager.cc \
    test/testFileUtils.cc \
    test/testSystemTopology.cc \
    test/testVFAT3ChannelConfiguration.cc \
    test/testXMLConfigurationProvider.cc \
    test/testXMLSerialization.cc \

DBTestExecutables = \
    test/testTStoreSerialization.cc \

TestExecutables = $(SimpleTestExecutables) $(DBTestExecutables)

IncludeDirs+=$(BUILD_HOME)/$(Project)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gembase/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemutils/include

UserCFlags  +=$(PYTHONCFLAGS)
UserCCFlags +=$(PYTHONCFLAGS)

DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gembase/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)

DependentLibraries=gembase gemutils xdaq cgicc xgi peer xdata mimetic xoap xerces-c xcept toolbox logudpappender logxmlappender log4cplus config cgicc
DependentLibraries += boost_filesystem boost_regex boost_system boost_thread jansson

# DependentLibraries+=$(StandardLibraries)

TestLibraries= $(DependentLibraries) boost_unit_test_framework tstoreapi wsaddressing tstoreutils
TestLibraryDirs= $(DependentLibraryDirs)

# Targets with additional dependencies. Keep "default" first
.PHONY: default clean tests run-tests run-tests-ci
default: xsd
clean: clean-xsd clean-gen-headers
tests: default
run-tests: run-tests-xsd run-tests-simple run-tests-db
run-tests-ci: run-tests-xsd run-tests-simple

# Common definitions
include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/config/mfRPMDefsGEM.mk

# Auto-generated files
xml/schema/%.xsd include/gem/onlinedb/%Gen.h src/common/%Gen.cc: defs/%.json parseDef.py
	python parseDef.py $<

# XML schemas
.PHONY: xsd
xsd: xml/schema/AMCConfiguration.xsd xml/schema/VFAT3ChipConfiguration.xsd xml/schema/OHv3Configuration.xsd xml/schema/GBTXConfiguration.xsd

.PHONY: clean-xsd
clean-xsd:
	$(RM) xml/schema/AMCConfiguration.xsd
	$(RM) xml/schema/VFAT3ChipConfiguration.xsd
	$(RM) xml/schema/OHv3Configuration.xsd
	$(RM) xml/schema/GBTXConfiguration.xsd

.PHONY: run-tests-xsd
run-tests-xsd:
	xmllint --noout --schema xml/schema/VFAT3ChipConfiguration.xsd xml/examples/VFAT3_Chip_Configuration.xml
	xmllint --noout --schema xml/schema/VFAT3ChannelConfiguration.xsd xml/examples/VFAT3_Channel_Configuration.xml
	xmllint --noout --schema xml/schema/OHv3Configuration.xsd xml/examples/OHv3_Configuration.xml
	xmllint --noout --schema xml/schema/AMCConfiguration.xsd xml/examples/AMC_Configuration.xml
	xmllint --noout --schema xml/schema/GBTXConfiguration.xsd xml/examples/GBTX_Configuration.xml
	xmllint --noout --schema xml/schema/system-topology.xsd xml/examples/system-topology.xml

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

# Auto-generated headers
.PHONY: clean-gen-headers
clean-gen-headers:
	$(RM) include/gem/onlinedb/detail/*Gen.h

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
