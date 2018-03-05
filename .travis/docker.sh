#!/bin/sh -xe

# Thanks to:
# https://github.com/opensciencegrid/htcondor-ce/tree/master/tests

OS_VERSION=$1
PYTHON_VERSION=""
GCC_VERSION=""
CLANG_VERSION=""
COMPILER=""
COMPILER_OPTIONS=""
LIBC_VERSION=""

# Clean the yum cache
yum -y clean all
yum -y clean expire-cache

uname -a

# $PYTHON_VERSON

export BUILD_HOME=/home/daqbuild

cd ${BUILD_HOME}/cmsgemos

. setup/etc/profile.d/gemdaqenv.sh
. setup/paths.sh

## drive the different options here, passed in from the parent?
# setBuildEnv.sh -p ${PYTHON_VERSION} -c ${COMPILER_VERSION}
make debug -j4
