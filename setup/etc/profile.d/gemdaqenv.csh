#! /bin/csh -f
# xdaq initialization
if ( $?USER_XDAQ_ROOT ) then
    if ( "$USER_XDAQ_ROOT" != "" ) then
	#when USER_XDAQ_ROOT defined do not override
	#user specified XDAQ_ROOT and use them
	#goto finish
	exit
    endif
endif

setenv XDAQ_ROOT /opt/xdaq
#setenv XDAQ_DOCUMENT_ROOT ${XDAQ_ROOT}/htdocs
setenv XDAQ_DOCUMENT_ROOT /data/xdaq/${USER}
setenv uHALROOT /opt/cactus

if ( $?LD_LIBRARY_PATH ) then
    if ( "$LD_LIBRARY_PATH" != "" ) then
	setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${XDAQ_ROOT}/lib
	setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${uHALROOT}/lib
    else
	setenv LD_LIBRARY_PATH ${XDAQ_ROOT}/lib
	setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${uHALROOT}/lib
    endif
else
    setenv LD_LIBRARY_PATH ${XDAQ_ROOT}/lib
    setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${uHALROOT}/lib
endif

if ( $?PYTHONPATH ) then
    if ( "$PYTHONPATH" != "" ) then
	setenv PYTHONPATH ${PYTHONPATH}:/data/sources/glib_v3/sw/PyChips/src
    else
	setenv PYTHONPATH /data/sources/glib_v3/sw/PyChips/src
    endif
else
    setenv PYTHONPATH /data/sources/glib_v3/sw/PyChips/src
endif

setenv PATH ${PATH}:${XDAQ_ROOT}/bin
setenv PATH ${PATH}:${uHALROOT}/bin
setenv PATH ${PATH}:${uHALROOT}/bin/amc13
setenv AMC13_ADDRESS_TABLE_PATH ${uHALROOT}/etc/amc13/
setenv GEMHOST `hostname --short`
