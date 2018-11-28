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

Sources =version.cc

DynamicLibrary=gemonlinedb

IncludeDirs+=$(BUILD_HOME)/$(Project)/$(Package)/include

# XML schemas
default: xsd test
clean: clean-xsd
test: test-xsd

schema/%.xsd include/gem/onlinedb/%Gen.h: defs/%.json parseDef.py
	python parseDef.py $<

.PHONY: xsd
xsd: schema/VFAT3ChipConfiguration.xsd

.PHONY: clean-xsd
clean-xsd:
	$(RM) schema/VFAT3ChipConfiguration.xsd

.PHONY: test-xsd
test-xsd:
	xmllint --noout --schema schema/VFAT3ChipConfiguration.xsd xml/VFAT3_Chip_Configuration.xml

# Common definitions
include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/config/mfRPMDefsGEM.mk

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
