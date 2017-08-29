#!/bin/sh -xe

# Thanks to:
# https://github.com/opensciencegrid/htcondor-ce/tree/master/tests

OS_VERSION=$1

# $PYTHON_VERSON

ls -l $PWD

# Clean the yum cache
yum -y clean all
yum -y clean expire-cache

uname -a

export BUILD_HOME=$PWD

cd ${BUILD_HOME}/cmsgemos

. setup/etc/profile.d/xdaqenv.sh
. setup/paths.sh

make debug -j4
