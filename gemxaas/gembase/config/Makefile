# $Id$

#########################################################################
# XDAQ Components for Distributed Data Acquisition                      #
# Copyright (C) 2000-2015, CERN.                                        #
# All rights reserved.                                                  #
# Authors: L. Orsini and D. Simelevicius                                #
#                                                                       #
# For the licensing terms see LICENSE.                                  #
# For the list of contributors see CREDITS.                             #
#########################################################################


##
#
#  Makefile for generic GEM setup machine configuration
#
##

#
# Packages to be built
#
BUILD_HOME:=$(shell pwd)/../../../..

include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/mfAutoconf.rules
include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/mfDefs.$(XDAQ_OS)

Project:=cmsgemos
PackageName=$(shell pwd | awk -F"/" '{split($$0,a,"/");  print a[NF-1]}')
SubPackageName=$(shell pwd | awk -F"/" '{split($$0,a,"/");  print a[NF-0]}')
PackageType=$(shell pwd | awk -F"/" '{split($$0,a,"/");  print a[NF-2]}')
Package=$(PackageType)/$(PackageName)/$(SubPackageName)

INSTALL_PATH=/opt/$(Project)
ZONE_NAME=$(PackageName)

Summary=GEM DAQ machine configuration for $(ZONE_NAME) service

Description=This package provides machine configurations for cmsgemos on zone $(ZONE_NAME)

Link=https://gitlab.cern.ch/cms-gem-daq-project/cmsgemos

PACKAGE_VER_MAJOR=1
PACKAGE_VER_MINOR=0
PACKAGE_VER_PATCH=0

include $(BUILD_HOME)/$(Project)/config/mfCommonDefs.mk

ifndef BUILD_VERSION
BUILD_VERSION=1
endif

_all: basic
	@echo Executing '_all' target

default: basic
	@echo Executing 'default' target

all:
	@echo Executing 'all' target
	@echo PackagePath $(BUILD_HOME)/$(Project)/$(Package)
	@echo ${PACKAGE_VER_MAJOR}_${PACKAGE_VER_MINOR}_${PACKAGE_VER_PATCH} > $(PackageName).release
	@echo Project $(Project)
	@echo PackageName $(PackageName)
	@echo PackageType $(PackageType)
	@echo Package $(Package)
	@mkdir -p ./scripts
	@cp -t ./scripts $(BUILD_HOME)/$(Project)/setup/etc/addresstables/linkuhaltables.sh
	@mkdir -p ./etc/profile.d
	@cp -t ./etc/profile.d $(BUILD_HOME)/$(Project)/setup/etc/profile.d/gemdaqenv.sh
	-cp -t ./etc/tcds $(TCDS_CONFIG_PATH)/$(PackageName)/*.txt
	perl -p -i -e 's#__zonename__#$(ZONE_NAME)#' spec.template

basic:
	@echo Executing 'basic' target
	echo ${PACKAGE_VER_MAJOR}_${PACKAGE_VER_MINOR}_${PACKAGE_VER_PATCH} > $(PackageName).release
	@mkdir -p ./scripts
	@cp -t ./scripts $(BUILD_HOME)/$(Project)/setup/etc/addresstables/linkuhaltables.sh
	@mkdir -p ./etc/profile.d
	@cp -t ./etc/profile.d $(BUILD_HOME)/$(Project)/setup/etc/profile.d/gemdaqenv.sh
	-cp -t ./etc/tcds $(TCDS_CONFIG_PATH)/$(PackageName)/*.txt
	perl -p -i -e 's#__zonename__#$(ZONE_NAME)#' spec.template

_cleanall: clean
	@echo Executing '_cleanall' target

clean:
	@echo Executing 'clean' target
	-rm -f $(PackageName).release
	-rm -f ./scripts/*.sh
	-rm -f ./etc/profile.d/*.sh
	-rm -f ./etc/tcds/*.txt

include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/mfSetupRPM.rules
