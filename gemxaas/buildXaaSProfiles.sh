#!/bin/sh

xdaqver=""
if [ "$XDAQ_PLATFORM" == "x86_64_slc6" ]
then
    xdaqver="13"
elif [ "$XDAQ_PLATFORM" == "x86_64_centos7" ]
then
    xdaqver="14"
fi

BUILD_DIR=/tmp/${USER}/daq/xaas
echo svn co -N svn+ssh://sturdy@svn.cern.ch/reps/cmsos/releases/baseline${xdaqver}/trunk/daq/xaas ${BUILD_DIR}
# echo cd ${BUILD_DIR}
echo svn up ${BUILD_DIR}/template
echo svn up -N ${BUILD_DIR}/slim
# echo cd slim
echo svn up ${BUILD_DIR}/slim/template
echo cp -rfp ${BUILD_HOME}/${GEM_OS_PROJECT}/gemxaas/gem* ${BUILD_DIR}/slim/
echo cp -rfp ${BUILD_HOME}/${GEM_OS_PROJECT}/gemxaas/Makefile ${BUILD_DIR}/slim

echo cd ${BUILD_DIR}/slim
echo make
echo make rpm
