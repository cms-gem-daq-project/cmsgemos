mkdir -p ${XDAQ_DOCUMENT_ROOT}/gemdaq/gembase
mkdir -p ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemutils
mkdir -p ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemhardware
mkdir -p ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemreadout
mkdir -p ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemsupervisor
mkdir -p ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemcalibration
mkdir -p ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemHwMonitor

unlink ${XDAQ_DOCUMENT_ROOT}/gemdaq/gembase/html
unlink ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemutils/html
unlink ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemhardware/html
unlink ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemreadout/html
unlink ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemsupervisor/html
unlink ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemcalibration/html
unlink ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemHwMonitor/html

ln -s ${BUILD_HOME}/${GEM_OS_PROJECT}/gembase/html ${XDAQ_DOCUMENT_ROOT}/gemdaq/gembase/html
ln -s ${BUILD_HOME}/${GEM_OS_PROJECT}/gemutils/html ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemutils/html
ln -s ${BUILD_HOME}/${GEM_OS_PROJECT}/gemhardware/html ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemhardware/html
ln -s ${BUILD_HOME}/${GEM_OS_PROJECT}/gemreadout/html ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemreadout/html
ln -s ${BUILD_HOME}/${GEM_OS_PROJECT}/gemsupervisor/html ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemsupervisor/html
ln -s ${BUILD_HOME}/${GEM_OS_PROJECT}/gemcalibration/html ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemcalibration/html
ln -s ${BUILD_HOME}/${GEM_OS_PROJECT}/gemHwMonitor/html ${XDAQ_DOCUMENT_ROOT}/gemdaq/gemHwMonitor/html
