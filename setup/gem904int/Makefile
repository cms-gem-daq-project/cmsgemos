# $Id: Makefile,v 1.21 2012-04-26 20:06:21 mansj Exp $

#########################################################################
# XDAQ Components for Distributed Data Acquisition                      #
# Copyright (C) 2000-2004, CERN.			                #
# All rights reserved.                                                  #
# Authors: J. Gutleber and L. Orsini					#
#                                                                       #
# For the licensing terms see LICENSE.		                        #
# For the list of contributors see CREDITS.   			        #
#########################################################################

##
#
#
#
#
##


#
# Packages to be built
#

include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

PackageName=gem904int
Project=cmsgemos
ShortProject=gem
Package=setup/${PackageName}

PACKAGE_VER_MAJOR=0
PACKAGE_VER_MINOR=0
PACKAGE_VER_PATCH=0

Summary=Setup data for the GEM 904 integration setup

Description=GEM integration setup 904

Authors=Jared Sturdy

Link=http://xdaqwiki.cern.ch

_all: basic

default: basic

all:
	echo ${PACKAGE_VER_MAJOR}_${PACKAGE_VER_MINOR}_${PACKAGE_VER_PATCH} > conf/$(PackageName).release
	# rm -rf $(BUILD_HOME)/${Project}/setup
	# mkdir -p $(BUILD_HOME)/${Project}/setup
	# cp -rf $(BUILD_HOME)/$(Project)/setup/${PackageName} $(BUILD_HOME)/${Project}/setup/.
	mkdir -p etc/cron.d
	touch etc/cron.d/$(PackageName).cron

basic:
	echo ${PACKAGE_VER_MAJOR}_${PACKAGE_VER_MINOR}_${PACKAGE_VER_PATCH} > conf/$(PackageName).release
	# rm -rf $(BUILD_HOME)/${Project}/setup
	# mkdir -p $(BUILD_HOME)/${Project}/setup
	# cp -rf $(BUILD_HOME)/gem/setup/${PackageName} $(BUILD_HOME)/${Project}/setup/.
	mkdir -p etc/cron.d
	touch etc/cron.d/$(PackageName).cron

_cleanall: clean

clean:
	-rm -f conf/$(PackageName).release
	# -rm -rf $(BUILD_HOME)/${Project}/setup

include $(XDAQ_ROOT)/config/mfSetupRPM.rules
