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
# Makefile for generic .cms904 zone
#
##

#
# Packages to be built
#
BUILD_HOME:=$(shell pwd)/../../../../..

include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

Project=daq
PackageName=$(shell pwd | awk -F"/" '{split($$0,a,"/");  print a[NF-1]}')
PackageType=$(shell pwd | awk -F"/" '{split($$0,a,"/");  print a[NF-2]}')
Package=xaas/$(PackageType)/$(PackageName)

ifndef BUILD_VERSION
BUILD_VERSION=1
endif

PACKAGE_VER_MAJOR=1
PACKAGE_VER_MINOR=0
PACKAGE_VER_PATCH=0

TEMPLATEDIR=$(BUILD_HOME)/$(Project)/xaas/template/zone

build: _buildall

_buildall: all

_all: all

default: all

all:
    make -f $(TEMPLATEDIR)/zone.makefile ZONE_NAME=$(PackageName) PACKAGE_TYPE=$(PackageType)
_installall: install

install:

_cleanall: clean

clean:
    make -f $(TEMPLATEDIR)/zone.makefile ZONE_NAME=$(PackageName) PACKAGE_TYPE=$(PackageType) clean

_rpmall: rpm

rpm:
    make -f $(TEMPLATEDIR)/zone.makefile ZONE_NAME=$(PackageName) PACKAGE_TYPE=$(PackageType) BUILD_VERSION=$(BUILD_VERSION) PACKAGE_VER_MAJOR=$(PACKAGE_VER_MAJOR) PACKAGE_VER_MINOR=$(PACKAGE_VER_MINOR) PACKAGE_VER_PATCH=$(PACKAGE_VER_PATCH) rpm


_installrpmall: installrpm

installrpm:
    make -f $(TEMPLATEDIR)/zone.makefile ZONE_NAME=$(PackageName) PACKAGE_TYPE=$(PackageType)  installrpm

_cleanrpmall: cleanrpm

cleanrpm:
    make -f $(TEMPLATEDIR)/zone.makefile ZONE_NAME=$(PackageName) PACKAGE_TYPE=$(PackageType) cleanrpm
