#######################
#######################

BUILD_HOME:=$(shell pwd)/..

ifndef BUILD_SUPPORT
BUILD_SUPPORT=config
endif

ifndef PROJECT_NAME
PROJECT_NAME=daq
endif

ifndef XAAS_ROOT
XAAS_ROOT=$(BUILD_HOME)/$(PROJECT_NAME)/xaas
endif
export XAAS_ROOT

include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/mfAutoconf.rules
include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/mfDefs.$(XDAQ_OS)

Project=daq
PackageName=$(shell pwd | awk -F"/" '{split($$0,a,"/");  print a[NF-1]}')
SubPackageName=$(shell pwd | awk -F"/" '{split($$0,a,"/");  print a[NF-0]}')
PackageType=$(shell pwd | awk -F"/" '{split($$0,a,"/");  print a[NF-2]}')
Package=xaas/$(PackageType)/$(PackageName)

ifndef BUILD_VERSION
BUILD_VERSION=1
endif

OS_VER=$(shell uname -r | cut -d '.' -f6)
ifeq ($(OS_VER),el7)
TEMPLATEDIR=$(XAAS_ROOT)/template/slim/$(SubPackageName)
else
TEMPLATEDIR=$(XAAS_ROOT)/slim/template/$(SubPackageName)
endif
