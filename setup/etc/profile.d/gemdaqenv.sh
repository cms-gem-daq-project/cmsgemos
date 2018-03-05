# xdaq initialization

if [ "$LD_LIBRARY_PATH" != "0" ]; then
    export LD_LIBRARY_PATH
fi

if [ "$XDAQ_ROOT" != "0" ]; then
    echo "executing gem daq setup"
    export XDAQ_ROOT=/opt/xdaq
    # export XDAQ_DOCUMENT_ROOT=$XDAQ_ROOT/htdocs
    # only do this for regular users
    export XDAQ_DOCUMENT_ROOT=/data/xdaq/${USER}
    export uHALROOT=/opt/cactus
fi

if [ "$LD_LIBRARY_PATH" != "0" ]; then
    if [ "$LD_LIBRARY_PATH" != "" ]; then
	export LD_LIBRARY_PATH=${XDAQ_ROOT}/lib:${LD_LIBRARY_PATH}
	export LD_LIBRARY_PATH=${uHALROOT}/lib:${LD_LIBRARY_PATH}
    else
	export LD_LIBRARY_PATH=${XDAQ_ROOT}/lib
	export LD_LIBRARY_PATH=${uHALROOT}/lib:${LD_LIBRARY_PATH}
    fi
else
    export LD_LIBRARY_PATH=${XDAQ_ROOT}/lib
    export LD_LIBRARY_PATH=$uHALROOT/lib:${LD_LIBRARY_PATH}
fi

if [ "$PYTHONPATH" != "0" ]; then
    if [ "$PYTHONPATH" != "" ]; then
	export PYTHONPATH=$PYTHONPATH
    else
	export PYTHONPATH=
    fi
else
    export PYTHONPATH=
fi

if [[ -n "$XDAQ_OS" ]]; then
    echo XDAQ_OS $XDAQ_OS
else
    if [[ $(uname -s) = "Linux" ]]; then
        XDAQ_OS=linux
    elif [[ $(uname -s) = "Darwin" ]]; then
        XDAQ_OS=macosx
    fi
    export XDAQ_OS
fi

## The plateform is not set. Let's guess it
if [[ -n "$XDAQ_PLATFORM" ]]; then
    echo XDAQ_PLATFORM $XDAQ_PLATFORM
else
    if [[ $(uname -m) = "i386" ]]; then
        XDAQ_PLATFORM=x86
    elif [[ $(uname -m) = "i486" ]]; then
        XDAQ_PLATFORM=x86
    elif [[ $(uname -m) = "i586" ]]; then
        XDAQ_PLATFORM=x86
    elif [[ $(uname -m) = "i686" ]]; then
        XDAQ_PLATFORM=x86
    elif [[ $(uname -m) = "x86_64" ]]; then
        XDAQ_PLATFORM=x86_64
    elif [[ $(uname -m) = "Power" ]]; then
        XDAQ_PLATFORM=ppc
    elif [[ $(uname -m) = "Macintosh" ]]; then
        XDAQ_PLATFORM=ppc
    fi
    XDAQ_PLATFORM=${XDAQ_PLATFORM}_$(source $XDAQ_ROOT/config/checkos.sh)
    echo XDAQ_PLATFORM $XDAQ_PLATFORM
    export XDAQ_PLATFORM
fi

export PATH=$PATH:$XDAQ_ROOT/bin
export PATH=$PATH:$uHALROOT/bin
export PATH=$PATH:$uHALROOT/bin/amc13
export AMC13_ADDRESS_TABLE_PATH=${uHALROOT}/etc/amc13/
export GEMHOST=`hostname --short`
export GEM_OS_PROJECT=cmsgemos

echo "PATH=${PATH}"
echo "AMC13_ADDRESS_TABLE_PATH=${AMC13_ADDRESS_TABLE_PATH}"
echo "GEMHOST=${GEMHOST}"
echo "GEM_OS_PROJECT=${GEM_OS_PROJECT=}"
echo "XDAQ_DOCUMENT_ROOT=${XDAQ_DOCUMENT_ROOT}"
echo "uHALROOT=${uHALROOT}"
echo "System setup done"
