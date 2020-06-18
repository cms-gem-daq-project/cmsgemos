.. _setup-machine-script:

====================================
Using the ``setupMachine.sh`` script
====================================

The purpose of the ``setupMachine.sh`` script is to automate much of the GEM DAQ machine setup.

The help menu of the script describes the available options.

.. code-block:: sh

   Usage: ./setupMachine.sh [options]
     Options:
        -a Setup new system with defaults for DAQ with accounts (implies -iAN)
        -i Install only software (implies -xcmrS)
        -x install xdaq software
        -c Install cactus tools (uhal and amc13)
        -m Install miscellaneous packages
        -S Install UW system manager
        -r Install root
        -p Install additional python versions
        -d Install developer tools
        -n Setup mounting of NAS
        -C Set up CTP7 connections
        -N Set up network interfaces
        -M Install Mellanox 10GbE drivers for uFEDKIT
        -X Install/update xpci drivers
        -A Create common users and groups
        -Z Install Xilinx USB drivers
        -V Install Xilinx Vivado
        -I Install Xilinx ISE
        -L Install Xilinx LabTools
        -u <file> Add accounts of NICE users (specified in file)
    
     Examples:
       Set up newly installed machine and add CERN NICE users: ./setupMachine.sh -au
       Set up newly installed machine and add uFEDKIT support: ./setupMachine.sh -aM
    
    Plese report bugs to
    https://github.com/cms-gem-daq-project/cmsgemos

.. important::
   Currently not all options are fully implemented, notably those relating to setting up the Xilinx tools.
   However, the basic machine setup is expected to be fully supported, and any bugs encountered should be reported.
