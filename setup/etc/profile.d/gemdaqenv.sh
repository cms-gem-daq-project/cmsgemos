# xdaq initialization

if [ "$LD_LIBRARY_PATH" != "0" ]; then
    export LD_LIBRARY_PATH
fi

if [ "$XDAQ_ROOT" != "0" ]; then
    if [ -z ${SSH_TTY+x} ]; then echo "executing gem daq setup"; fi
    export XDAQ_ROOT=/opt/xdaq
    export XDAQ_DOCUMENT_ROOT=$XDAQ_ROOT/htdocs
    # only do this for regular users
    # export XDAQ_DOCUMENT_ROOT=/data/xdaq/${USER}
    export uHALROOT=/opt/cactus
    export XHAL_ROOT=/opt/xhal
    export CMSGEMOS_ROOT=/opt/cmsgemos
fi

if [ "$LD_LIBRARY_PATH" != "0" ]; then
    if [ "$LD_LIBRARY_PATH" != "" ]; then
	export LD_LIBRARY_PATH=${XDAQ_ROOT}/lib:${LD_LIBRARY_PATH}
	export LD_LIBRARY_PATH=${uHALROOT}/lib:${LD_LIBRARY_PATH}
	export LD_LIBRARY_PATH=${CMSGEMOS_ROOT}/lib:${LD_LIBRARY_PATH}
	export LD_LIBRARY_PATH=${XHAL_ROOT}/lib:${LD_LIBRARY_PATH}
    else
	export LD_LIBRARY_PATH=${XDAQ_ROOT}/lib
	export LD_LIBRARY_PATH=${uHALROOT}/lib:${LD_LIBRARY_PATH}
	export LD_LIBRARY_PATH=${CMSGEMOS_ROOT}/lib:${LD_LIBRARY_PATH}
	export LD_LIBRARY_PATH=${XHAL_ROOT}/lib:${LD_LIBRARY_PATH}
    fi
else
    export LD_LIBRARY_PATH=${XDAQ_ROOT}/lib
    export LD_LIBRARY_PATH=${uHALROOT}/lib:${LD_LIBRARY_PATH}
    export LD_LIBRARY_PATH=${CMSGEMOS_ROOT}/lib:${LD_LIBRARY_PATH}
    export LD_LIBRARY_PATH=${XHAL_ROOT}/lib:${LD_LIBRARY_PATH}
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
    if [ -z ${SSH_TTY+x} ]; then echo XDAQ_OS $XDAQ_OS; fi
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
    if [ -z ${SSH_TTY+x} ]; then echo XDAQ_PLATFORM $XDAQ_PLATFORM; fi
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
    XDAQ_PLATFORM=${XDAQ_PLATFORM}_$(source $XDAQ_ROOT/build/checkos.sh)
    if [ -z ${SSH_TTY+x} ]; then echo XDAQ_PLATFORM $XDAQ_PLATFORM; fi
    export XDAQ_PLATFORM
fi

export PATH=$CMSGEMOS_ROOT/bin:$PATH
export PATH=$XHAL_ROOT/bin:$PATH
export PATH=$XDAQ_ROOT/bin:$PATH
export PATH=$uHALROOT/bin:$PATH
export PATH=$uHALROOT/bin/amc13:$PATH
export AMC13_ADDRESS_TABLE_PATH=${uHALROOT}/etc/amc13/
export GEM_ADDRESS_TABLE_PATH=${CMSGEMOS_ROOT}/etc/maps/
export GEMHOST=`hostname --short`
export GEM_OS_PROJECT=cmsgemos

if [ -z ${SSH_TTY+x} ]
then
    echo "PATH=${PATH}"
    echo "AMC13_ADDRESS_TABLE_PATH=${AMC13_ADDRESS_TABLE_PATH}"
    echo "GEM_ADDRESS_TABLE_PATH=${GEM_ADDRESS_TABLE_PATH}"
    echo "GEMHOST=${GEMHOST}"
    echo "GEM_OS_PROJECT=${GEM_OS_PROJECT=}"
    echo "XDAQ_DOCUMENT_ROOT=${XDAQ_DOCUMENT_ROOT}"
    echo "uHALROOT=${uHALROOT}"
    echo "CMSGEMOS_ROOT=${CMSGEMOS_ROOT}"
    echo "XHAL_ROOT=${XHAL_ROOT}"
    echo "System setup done"
fi
