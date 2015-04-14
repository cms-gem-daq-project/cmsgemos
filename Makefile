#
# Global Makefile for GEM
#

SUBPACKAGES := \
        gemutils \
        gembase \
        gemhardware \
        gemsupervisor \
        gemHwMonitor \

OS:=linux
ARCH:=x86_64
LIBDIR:=lib/$(OS)/$(ARCH)

SUBPACKAGES.INSTALL := $(patsubst %,%.install, ${SUBPACKAGES})
SUBPACKAGES.RPM := $(patsubst %,%.rpm, ${SUBPACKAGES})
SUBPACKAGES.CLEAN := $(patsubst %,%.clean, ${SUBPACKAGES})

all: $(SUBPACKAGES)

install: $(LIBDIR) $(SUBPACKAGES) $(SUBPACKAGES.INSTALL)

rpm: $(SUBPACKAGES) $(SUBPACKAGES.RPM)

clean: $(SUBPACKAGES.CLEAN)

$(LIBDIR):
	mkdir -p $(LIBDIR)

$(SUBPACKAGES):
	$(MAKE) -C $@

$(SUBPACKAGES.RPM):
	$(MAKE) -C $(patsubst %.rpm,%, $@) rpm

$(SUBPACKAGES.INSTALL):
	-find  $(patsubst %.install,%, $@)/lib -name *.so -print -exec cp {} ${LIBDIR} \;

$(SUBPACKAGES.CLEAN):
	$(MAKE) -C $(patsubst %.clean,%, $@) clean

.PHONY: $(SUBPACKAGES) $(SUBPACKAGES.INSTALL) $(SUBPACKAGES.CLEAN)


gemHwMonitor: gembase gemhardware

gemhardware: gemutils

gembase: gemutils

gemsupervisor: gemhardware

gemutils: 
