#!/bin/sh

xdaqver=""
if [ "$XDAQ_PLATFORM" == "x86_64_slc6" ]
then
    xdaqver="13"
elif [ "$XDAQ_PLATFORM" == "x86_64_centos7" ]
then
    xdaqver="14"
else
    xdaqver="15"
fi

XAAS_DIR=/tmp/${USER}/daq/xaas
svn co -N svn+ssh://sturdy@svn.cern.ch/reps/cmsos/releases/baseline${xdaqver}/trunk/daq/xaas ${XAAS_DIR}

svn up ${XAAS_DIR}/template
svn up -N ${XAAS_DIR}/slim

if [[ ${xdaqver} =~ 13 ]]
then
    svn up ${XAAS_DIR}/slim/template
fi

cp -rfp ${BUILD_HOME}/${GEM_OS_PROJECT}/gemxaas/gem* ${XAAS_DIR}/slim/
cp -rfp ${BUILD_HOME}/${GEM_OS_PROJECT}/gemxaas/Makefile ${XAAS_DIR}/slim

cd ${XAAS_DIR}/slim

make
make rpm
