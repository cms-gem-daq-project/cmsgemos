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
#  Makefile for generic GEM setup addon
#
##

#
# Packages to be built
#
BUILD_HOME:=$(shell pwd)/../../../../..

PACKAGE_VER_MAJOR=1
PACKAGE_VER_MINOR=0
PACKAGE_VER_PATCH=0

include $(CMSGEMOS_ROOT)/config/mfXaaSDefs.mk

build: _buildall

_buildall: all

_all: all

default: all

all:
	make -f $(TEMPLATEDIR)/addon.makefile PROJECT_NAME=$(Project) ZONE_NAME=$(PackageName) PACKAGE_TYPE=$(PackageType)
_installall: install

install:

_cleanall: clean

clean:
	make -f $(TEMPLATEDIR)/addon.makefile PROJECT_NAME=$(Project) ZONE_NAME=$(PackageName) PACKAGE_TYPE=$(PackageType) clean

_rpmall: rpm

rpm:
	make -f $(TEMPLATEDIR)/addon.makefile PROJECT_NAME=$(Project) ZONE_NAME=$(PackageName) PACKAGE_TYPE=$(PackageType) BUILD_VERSION=$(BUILD_VERSION) PACKAGE_VER_MAJOR=$(PACKAGE_VER_MAJOR) PACKAGE_VER_MINOR=$(PACKAGE_VER_MINOR) PACKAGE_VER_PATCH=$(PACKAGE_VER_PATCH) rpm


_installrpmall: installrpm

installrpm:
	make -f $(TEMPLATEDIR)/addon.makefile PROJECT_NAME=$(Project) ZONE_NAME=$(PackageName) PACKAGE_TYPE=$(PackageType)  installrpm

_cleanrpmall: cleanrpm

cleanrpm:
	make -f $(TEMPLATEDIR)/addon.makefile PROJECT_NAME=$(Project) ZONE_NAME=$(PackageName) PACKAGE_TYPE=$(PackageType) cleanrpm
