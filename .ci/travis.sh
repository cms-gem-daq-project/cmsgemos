#!/bin/sh -xe

# Thanks to:
# https://github.com/opensciencegrid/htcondor-ce/tree/master/tests

OS_VERSION=$1
PYTHON_VERSION=$2
GCC_VERSION=""
CLANG_VERSION=""
COMPILER=""
COMPILER_OPTIONS=""
LIBC_VERSION=""

export BUILD_HOME=/home/daqbuild

cd ${BUILD_HOME}/cmsgemos

. ${BUILD_HOME}/cmsgemos/setup/etc/profile.d/gemdaqenv.sh
. ${BUILD_HOME}/cmsgemos/setup/paths.sh

## drive the different options here, passed in from the parent?
# setBuildEnv.sh -p ${PYTHON_VERSION} -c ${COMPILER_VERSION}
eval "$1"
