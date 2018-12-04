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

Sources = \
    VFAT3ChipConfigurationGen.cc \
    version.cc \
    XMLSerializationData.cc \

DynamicLibrary=gemonlinedb

TestLibraries = $(DependentLibraries) xoap boost_unit_test_framework

TestExecutables = \
    test/testXMLSerialization.cc \

IncludeDirs+=$(BUILD_HOME)/$(Project)/$(Package)/include

# Targets with additional dependencies. Keep "default" first
.PHONY: default clean tests
default: xsd
clean: clean-xsd clean-gen-headers
tests: default test-xsd

# Common definitions
include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/config/mfRPMDefsGEM.mk

# Auto-generated files
schema/%.xsd include/gem/onlinedb/%Gen.h src/common/%Gen.cc: defs/%.json parseDef.py
	python parseDef.py $<

# XML schemas
.PHONY: xsd
xsd: schema/VFAT3ChipConfiguration.xsd

.PHONY: clean-xsd
clean-xsd:
	$(RM) schema/VFAT3ChipConfiguration.xsd

.PHONY: test-xsd
test-xsd:
	xmllint --noout --schema schema/VFAT3ChipConfiguration.xsd xml/VFAT3_Chip_Configuration.xml

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
