BUILD_HOME ?= $(shell dirname `pwd`)
$(info Using BUILD_HOME=$(BUILD_HOME))

# cmsgemos config. This section should be sourced from /opt/cmsgemos/config
ifndef INSTALL_PATH
CMSGEMOS_ROOT := /opt/cmsgemos
endif

CMSGEMOS_PLATFORM := $(shell python -c "import platform; print(platform.platform())")
CMSGEMOS_OS       := "unknown.os"

GIT_VERSION  := $(shell git describe --dirty --always --tags)
GEMDEVELOPER := $(shell id --user --name)
GITREV       := $(shell git rev-parse --short HEAD)
BUILD_DATE   := $(shell date -u +"%d%m%Y")

UNAME=$(strip $(shell uname -s))
ifeq ($(UNAME),Linux)
    ifneq ($(findstring redhat-5,$(CMSGEMOS_PLATFORM)),)
        CMSGEMOS_OS=slc5
    else ifneq ($(findstring redhat-6,$(CMSGEMOS_PLATFORM)),)
        CMSGEMOS_OS=slc6
    else ifneq ($(findstring centos-6,$(CMSGEMOS_PLATFORM)),)
        CMSGEMOS_OS=centos6
    else ifneq ($(findstring centos-7,$(CMSGEMOS_PLATFORM)),)
        CMSGEMOS_OS=centos7
    else ifneq ($(findstring centos-8,$(CMSGEMOS_PLATFORM)),)
        CMSGEMOS_OS=centos8
    else ifneq ($(findstring fedora-26,$(CMSGEMOS_PLATFORM)),)
        CMSGEMOS_OS=fedora26
    else ifneq ($(findstring fedora-27,$(CMSGEMOS_PLATFORM)),)
        CMSGEMOS_OS=fedora27
    else ifneq ($(findstring fedora-28,$(CMSGEMOS_PLATFORM)),)
        CMSGEMOS_OS=fedora28
    endif
endif
ifeq ($(UNAME),Darwin)
    CMSGEMOS_OS=osx
endif

$(info OS Detected: $(CMSGEMOS_OS))
# end of cmsgemos config

# Tools
MakeDir=mkdir -p

## Version variables from Makefile and ShortPackage
ShortPackageLoc := $(shell echo "$(ShortPackage)" | tr '[:lower:]' '[:upper:]')
PACKAGE_VER_MAJOR ?= $($(ShortPackageLoc)_VER_MAJOR)
PACKAGE_VER_MINOR ?= $($(ShortPackageLoc)_VER_MINOR)
PACKAGE_VER_PATCH ?= $($(ShortPackageLoc)_VER_PATCH)

BUILD_VERSION ?= 1

PACKAGE_FULL_VERSION ?= $(PACKAGE_VER_MAJOR).$(PACKAGE_VER_MINOR).$(PACKAGE_VER_PATCH)
PACKAGE_FULL_RELEASE ?= $(BUILD_VERSION).$(GITREV)git.$(CMSGEMOS_OS)

.PHONY: default all _all

default:

