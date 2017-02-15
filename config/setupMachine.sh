#!/bin/bash

arch=`uname -i`
setup_version=2.7.2-1.cmsos12
slc_version=`expr $(uname -r) : '.*el\([0-9]\)'`

### copied from HCAL setup scripts
install_xdaq() {
    # option 'x'
    echo Installing XDAQ...
    # generic XDAQ
    yum -y groupinstall extern_coretools coretools extern_powerpack powerpack
    # add-on packages
    yum --skip-broken -y groupinstall database_worksuite general_worksuite dcs_worksuite hardware_worksuite
    # for fedKit
    yum -y groupinstall daq_kernel_modules
    # debug modules
    yum -y groupinstall exetern_coretools_debuginfo coretools_debuginfo extern_powerpack_debuginfo powerpack_debuginfo \
        database_worksuite_debuginfo general_worksuite_debuginfo dcs_worksuite_debuginfo hardware_worksuite_debuginfo
}

install_misc_rpms() {
    # option 'm'
    echo Installing miscellaneous RPMS...
    yum install -y libuuid-devel e2fsprogs-devel readline-devel ncurses-devel curl-devel boost-devel \
        mysql-devel mysql-server numactl-devel freeipmi-devel arp-scan
}

install_cactus() {
    # option 'c'
    echo Installing cactus...
    yum -y groupinstall cactus
    yum -y groupinstall amc13

    chkconfig --level 35 controlhub on
    #ln -s /etc/rc.d/init.d/controlhub /etc/rc.d/rc3.d/S89controlhub
    #ln -s /etc/rc.d/init.d/controlhub /etc/rc.d/rc5.d/S89controlhub
}

install_sysmgr() {
    # option 'S'
    echo Installing UW sysmgr RPMS...
    yum install -y freeipmi libxml++ libxml++-devel libconfuse libconfuse-devel
    wget https://github.com/uwcms/sysmgr/releases/download/v1.1.1/sysmgr-1.1.1-1.el6.x86_64.rpm
    yum --nogpgcheck -y install sysmgr-1.1.1-1.el6.x86_64.rpm
}

install_python27() {
    # option 'p'
    echo Installing python2.7
    # install dependencies
    yum install -y tcl-devel tk-devel
    # common source directory, may already exist
    mkdir -p /data/bigdisk/sw/python2.7/
    cd /data/bigdisk/sw/python2.7/
    # setup scripts
    wget https://bootstrap.pypa.io/ez_setup.py
    wget https://www.python.org/ftp/python/2.7.12/Python-2.7.12.tar.xz
    tar xf Python-2.7.12.tar.xz
    cd Python-2.7.12
    ./configure --prefix=/usr/local --enable-unicode=ucs4 --enable-shared LDFLAGS="-Wl,-rpath /usr/local/lib"
    # clean up in case previously compiled and then compile
    make clean && make -j5
    # do the installation
    make altinstall
    
    cd /data/bigdisk/sw/python2.7/
    /usr/local/bin/python2.7 ez_setup.py
    /usr/local/bin/easy_install-2.7 pip
    
    # add links to the python26 packages we'll need
    ln -s /usr/lib/python2.6/site-packages/uhal /usr/local/lib/python2.7/site-packages/uhal
    
    ln -s /usr/lib/python2.6/site-packages/cactusboards_amc13_python-1.1.10-py2.6.egg-info /usr/local/lib/python2.7/site-packages/cactusboards_amc13_python-1.1.10-py2.6.egg-info
    ln -s /usr/lib/python2.6/site-packages/cactuscore_tsxdaqclient-2.6.3-py2.6.egg-info /usr/local/lib/python2.7/site-packages/cactuscore_tsxdaqclient-2.6.3-py2.6.egg-info
    ln -s /usr/lib/python2.6/site-packages/cactuscore_uhal_gui-2.3.0-py2.6.egg-info /usr/local/lib/python2.7/site-packages/cactuscore_uhal_gui-2.3.0-py2.6.egg-info
    ln -s /usr/lib/python2.6/site-packages/cactuscore_uhal_pycohal-2.4.0-py2.6.egg-info /usr/local/lib/python2.7/site-packages/cactuscore_uhal_pycohal-2.4.0-py2.6.egg-info
    
    ln -s /usr/lib/python2.6/site-packages/amc13 /usr/local/lib/python2.7/site-packages/amc13
    ln -s /usr/lib64/python2.6/site-packages/ROOT.py /usr/local/lib/python2.7/site-packages/ROOT.py
    ln -s /usr/lib64/python2.6/site-packages/ROOTwriter.py /usr/local/lib/python2.7/site-packages/ROOTwriter.py
    ln -s /usr/lib64/python2.6/site-packages/libPyROOT.so /usr/local/lib/python2.7/site-packages/libPyROOT.so
}

setup_ctp7() {
    # option 'C'
    echo Setting up for CTP7 usage...
    # Updated /etc/sysmgr/sysmgr.conf to enable the GenericUW configuration module to support "WISC CTP-7" cards.

    # Created /etc/sysmgr/ipconfig.xml to map geographic address assignments for crates 1 and 2 matching the /24
    # subnets associated with the MCHs listed for them in /etc/sysmgr/sysmgr.conf.  These addresses will occupy
    # 192.168.*.40 to 192.168.*.52 which nmap confirms are not in use.

    # time-stream xinetd service was enabled by editing /etc/xinetd.d/time-stream and restarting xinetd via init script.

    # create /etc/rsyslog.d/ctp7.conf
    # Created /etc/logrotate.d/ctp7 with configuration to rotate these logs.
    
    # Created configuration file /etc/dnsmasq.d/ctp7 which I hope will work correctly.

    # /etc/hosts has been updated with CTP7-related dns names
    # local= entry has been added to /etc/dnsmasq.d/ctp7 to allow serving requests for *.utca names.

}

install_root() {
    echo Installing root...
    yum install -y root root\*
}

create_accounts() {
    # option 'A'
    # may want LDAP users/groups to manage these as well
    # or even 'service' accounts, but better to have them tied to egroups
    # so one can log in with NICE credentials, a la cchcal
    ### generic gemuser group for running tests
    useradd gemuser
    usermod -u 1030 gemuser
    groupmod -g 1030 gemuser
    passwd gemuser
    chmod og+rx /home/gemuser

    ### gempro (production) account for running the system as an expert
    useradd gempro
    usermod -u 1050 gempro
    groupmod -g 1050 gempro
    passwd gempro

    ### gemdev (development) account for running tests
    useradd gemdev
    usermod -u 1055 gemdev
    groupmod -g 1055 gemdev
    passwd gemdev

    ### daqbuild account for building the releases
    useradd daqbuild
    usermod -u 2050 daqbuild
    groupmod -g 2050 daqbuild
    passwd daqbuild

    ### gemdaqadmins group for administering the system
    groupadd gemdaqadmins
    groupmod -g 1075 gemdaqadmins
}

add_cern_users() {
    # option 'u'
    ### probably better to have a list of users that is imported
    # or better yet, an LDAP group!
    sudo $HOME/newcernuser.sh archie
    sudo $HOME/newcernuser.sh bravo
    sudo $HOME/newcernuser.sh castaned
    sudo $HOME/newcernuser.sh cfgonzal
    sudo $HOME/newcernuser.sh chmclean
    sudo $HOME/newcernuser.sh dldecker
    sudo $HOME/newcernuser.sh dorney
    sudo $HOME/newcernuser.sh evka
    sudo $HOME/newcernuser.sh gechen
    sudo $HOME/newcernuser.sh hamd
    sudo $HOME/newcernuser.sh ivai
    sudo $HOME/newcernuser.sh jpilot
    sudo $HOME/newcernuser.sh jruizalv
    sudo $HOME/newcernuser.sh kingr
    sudo $HOME/newcernuser.sh mdalchen
    sudo $HOME/newcernuser.sh qhassan
    sudo $HOME/newcernuser.sh szaleski
    sudo $HOME/newcernuser.sh tlenzi
    sudo $HOME/newcernuser.sh waqar
}

while getopts "xayruscwmSAp" opt; do
  case $opt in
    a)
      echo "Doing all steps"
    u)
      create_users
      ;;
    S)
      install_sysmgr
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      ;;
  esac
done
