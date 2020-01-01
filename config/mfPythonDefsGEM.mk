PYVER?=python
## Python
PYTHON_VERSION        = $(shell $(PYVER) -c "import distutils.sysconfig;print(distutils.sysconfig.get_python_version())")
# PYTHON_VERSION        = $(shell $(PYVER) -c "import sys; sys.stdout.write(sys.version[:3])")
PYTHON_LIB            = python$(PYTHON_VERSION)
PYTHON_LIB_PREFIX     = $(shell $(PYVER) -c "from distutils.sysconfig import get_python_lib;import os.path;print(os.path.split(get_python_lib(standard_lib=True))[0])")
PYTHON_SITE_PREFIX    = $(shell $(PYVER) -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())")
PYTHON_INCLUDE_PREFIX = $(shell $(PYVER) -c "import distutils.sysconfig;print(distutils.sysconfig.get_python_inc())")

# Python Config
PYTHONCFLAGS = $(shell pkg-config $(PYVER) --cflags)
PYTHONLIBS   = $(shell pkg-config $(PYVER) --libs)
PYTHONGLIBS  = $(shell pkg-config $(PYVER) --glibs)

IncludeDirs+=$(PYTHON_INCLUDE_PREFIX)

# DependentLibraries+=python$(PYTHON_VERSION)

## these should come from PYTHONCFLAGS and PYTHONLIBS variables
# IncludeDirs+=${PYTHON_INCLUDE_PREFIX}
# DependentLibraries+=python${PYTHON_VERSION}
# DynamicLinkFlags+=
