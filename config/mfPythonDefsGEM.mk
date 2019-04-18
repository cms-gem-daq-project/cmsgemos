
## Python
PYTHON_INCLUDE_PREFIX ?= $(shell python -c "import distutils.sysconfig;print(distutils.sysconfig.get_python_inc())")
PYTHON_LIB_PREFIX     ?= $(shell python -c "from distutils.sysconfig import get_python_lib; import os.path; print(os.path.split(get_python_lib(standard_lib=True))[0])")
# PYTHON_VERSION ?= $(shell python -c "import distutils.sysconfig;print(distutils.sysconfig.get_python_version())")
PYTHON_VERSION ?= $(shell python -c "import sys; sys.stdout.write(sys.version[:3])")
PYTHON_LIB     ?= python${PYTHON_VERSION}

# Python Config
PYTHONCFLAGS = $(shell pkg-config python --cflags)
PYTHONLIBS   = $(shell pkg-config python --libs)
PYTHONGLIBS  = $(shell pkg-config python --glibs)

IncludeDirs+=${PYTHON_INCLUDE_PREFIX}


## these should come from PYTHONCFLAGS and PYTHONLIBS variables
# IncludeDirs+=${PYTHON_INCLUDE_PREFIX}
# DependentLibraries+=python${PYTHON_VERSION}
# DynamicLinkFlags+=
