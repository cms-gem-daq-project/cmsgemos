echo "Setting up xdaqgem variables"
export PATH=${PATH}:/sbin
export PATH=${PATH}:/usr/sbin
export PATH=${PATH}:/usr/local/sbin

export uHALROOT=/opt/cactus
export PATH=${PATH}:${uHALROOT}/bin
export PATH=${PATH}:/home/sturdy/xdaqgem/glib_dev/amc_glib/trunk/glib_v3/sw/PyChips/scripts
export LD_LIBRARY_PATH=${uHALROOT}/lib
export PYTHONPATH /home/sturdy/xdaqgem/glib_dev/amc_glib/trunk/glib_v3/sw/PyChips/src

export XDAQ_ROOT=/opt/xdaq
export XDAQ_DOCUMENT_ROOT=${XDAQ_ROOT}/htdocs
export PATH=${PATH}:${XDAQ_ROOT}/bin
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${XDAQ_ROOT}/lib
