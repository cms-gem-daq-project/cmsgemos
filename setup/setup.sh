echo "Setting up xdaqgem variables"
export PATH=${PATH}:/sbin
export PATH=${PATH}:/usr/sbin
export PATH=${PATH}:/usr/local/sbin

export uHALROOT=/opt/cactus
export PATH=${PATH}:${uHALROOT}/bin
export PATH=${PATH}:/home/cscdev/PyChips_1_4_3/scripts
export LD_LIBRARY_PATH=${uHALROOT}/lib
export PYTHONPATH=/home/cscdev/PyChips_1_4_3/src

export BUILD_HOME=/home/kingr/gem-daq-code
export GLIBTEST=/home/kingr/gem-daq-code

export XDAQ_ROOT=/opt/xdaq
export XDAQ_DOCUMENT_ROOT=${XDAQ_ROOT}/htdocs
export PATH=${PATH}:${XDAQ_ROOT}/bin
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${XDAQ_ROOT}/lib