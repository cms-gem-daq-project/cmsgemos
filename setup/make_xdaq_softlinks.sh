
packages=(
    gembase
    gemhardware
    gemsupervisor
    gemcalibration
    gemHwMonitor
    gemdaqmonitor
    gemonlinedb
    #gemutils
    #gemreadout
)

for pkg in ${packages[@]}
do
    base=${XDAQ_DOCUMENT_ROOT}/gemdaq/$pkg
    mkdir -p ${base}
    if [ -L ${base}/html ]
    then
        unlink ${base}/html
    fi
    ln -s ${BUILD_HOME}/$pkg/html ${base}/html
done
