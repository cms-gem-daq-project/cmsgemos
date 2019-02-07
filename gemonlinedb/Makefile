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
GEMONLINEDB_VER_MINOR=0
GEMONLINEDB_VER_PATCH=0

include $(BUILD_HOME)/$(Project)/config/mfDefsGEM.mk
include $(BUILD_HOME)/$(Project)/config/mfPythonDefsGEM.mk

DependentLibraries += boost_filesystem

Sources = \
    AMC13Configuration.cc \
    AMCConfiguration.cc \
    AMCConfigurationGen.cc \
    ConfigurationLinker.cc \
    FileUtils.cc \
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

TestLibraries = $(DependentLibraries) xoap boost_unit_test_framework tstoreapi xdata mimetic wsaddressing tstoreutils

TestExecutables = \
    test/testAMC13Configuration.cc \
    test/testConfigurationLinker.cc \
    test/testFileUtils.cc \
    test/testSystemTopology.cc \
    test/testTStoreSerialization.cc \
    test/testVFAT3ChannelConfiguration.cc \
    test/testXMLConfigurationProvider.cc \
    test/testXMLSerialization.cc \
    test/testTStoreSerialization.cc \

IncludeDirs+=$(BUILD_HOME)/$(Project)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gembase/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemutils/include

DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gembase/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)

DependentLibraries=gembase gemutils xdaq xdata mimetic xoap xerces-c xcept toolbox log4cplus
# DependentLibraries+=$(StandardLibraries)

# Targets with additional dependencies. Keep "default" first
.PHONY: default clean tests
default: xsd
clean: clean-xsd clean-gen-headers
tests: default
run-tests: run-tests-xsd run-tests-native

# Common definitions
include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/config/mfRPMDefsGEM.mk

# Auto-generated files
xml/schema/%.xsd include/gem/onlinedb/%Gen.h src/common/%Gen.cc: defs/%.json parseDef.py
	python parseDef.py $<

# XML schemas
.PHONY: xsd
xsd: xml/schema/AMCConfiguration.xsd xml/schema/VFAT3ChipConfiguration.xsd xml/schema/OHv3Configuration.xsd

.PHONY: clean-xsd
clean-xsd:
	$(RM) xml/schema/AMCConfiguration.xsd
	$(RM) xml/schema/VFAT3ChipConfiguration.xsd
	$(RM) xml/schema/OHv3Configuration.xsd

.PHONY: run-tests-xsd
run-tests-xsd:
	xmllint --noout --schema xml/schema/VFAT3ChipConfiguration.xsd xml/examples/VFAT3_Chip_Configuration.xml
	xmllint --noout --schema xml/schema/VFAT3ChannelConfiguration.xsd xml/examples/VFAT3_Channel_Configuration.xml
	xmllint --noout --schema xml/schema/OHv3Configuration.xsd xml/examples/OHv3_Configuration.xml
	xmllint --noout --schema xml/schema/AMCConfiguration.xsd xml/examples/AMC_Configuration.xml
	xmllint --noout --schema xml/schema/system-topology.xsd xml/examples/system-topology.xml

TEST_ENV = LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/
TEST_LOC = test/$(XDAQ_OS)/$(XDAQ_PLATFORM)
TEST_EXE = $(TestExecutables:.cc=.exe)

.PHONY: run-tests-native
run-tests-native:
	@for test in $(TEST_EXE); do \
	    echo Testing: $$test; \
	    $(TEST_ENV) $(TEST_LOC)/$$test; \
	done

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
