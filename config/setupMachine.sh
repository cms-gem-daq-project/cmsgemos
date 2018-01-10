#!/bin/bash

arch=`uname -i`
setup_version=2.7.2-1.cmsos12
osver=`expr $(uname -r) : '.*el\([0-9]\)'`
ostype=centos
if [ "${osver}" = "6" ]
then
    ostype=slc
fi

prompt_confirm() {
    while true; do
        read -r -n 1 -p "${1:-Continue?} [y/n]: " REPLY
        case $REPLY in
            [yY]) echo ; return 0 ;;
            [nN]) echo ; return 1 ;;
            # *) printf " \033[31m %s \n\033[0m" "invalid input"
            *) echo ; return 1 ;;
        esac
    done
}

### copied from HCAL setup scripts
install_xdaq() {
    # option 'x'
    wget https://svnweb.cern.ch/trac/cactus/export/HEAD/trunk/scripts/release/xdaq.${ostype}${osver}.x86_64.repo -O /etc/yum.repos.d/xdaq.repo
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
        mysql-devel mysql-server numactl-devel freeipmi-devel arp-scan \
        libusb libusbx libusb-devel libusbx-devel
    yum install -y centos-release-scl
}

install_developer_tools() {
    # option 'd'
    echo Installing developer tools RPMS...

    prompt_confirm "Install rh-git29?"
    if [ "$?" = "0" ]
    then
       yum install -y rh-git29*
    fi

    rubyvers=( rh-ruby22 rh-ruby23 rh-ruby24 )
    for rver in "${rubyvers[@]}"
    do
        prompt_confirm "Install ${rver}?"
        if [ "$?" = "0" ]
        then
            yum install -y ${rver}*
        fi
    done

    devtools=( devtoolset-3 devtoolset-4 devtoolset-6 devtoolset-7 )
    for dtool in "${devtools[@]}"
    do
        tools=( make gcc oprofile valgrind )
        for tool in "${tools[@]}"
        do
            prompt_confirm "Install ${dtool}-${tool}?"
            if [ "$?" = "0" ]
            then
                eval yum install -y ${dtool}-${tool}*
            fi
        done
    done
}

setup_nas() {
    # option 'n'
    echo Connecting to the NAS
    cat <<EOF>/etc/auto.nas
GEMDAQ_Documentation    -context="system_u:object_r:nfs_t:s0",nosharecache,auto,rw,async,timeo=14,intr,rsize=32768,wsize=32768,tcp,nosuid,noexec,acl               gem904nas01:/share/gemdata/GEMDAQ_Documentation
GEM-Data-Taking         -context="system_u:object_r:httpd_sys_content_t:s0",nosharecache,auto,rw,async,timeo=14,intr,rsize=32768,wsize=32768,tcp,nosuid,noexec,acl gem904nas01:/share/gemdata/GEM-Data-Taking
sw                      -context="system_u:object_r:nfs_t:s0",nosharecache,auto,rw,async,timeo=14,intr,rsize=32768,wsize=32768,tcp,nosuid                          gem904nas01:/share/gemdata/sw
users                   -context="system_u:object_r:nfs_t:s0",nosharecache,auto,rw,async,timeo=14,intr,rsize=32768,wsize=32768,tcp,nosuid,acl                      gem904nas01:/share/gemdata/users
+auto.nas
EOF
    if [ -f /etc/auto.master ]
    then
        echo "/data/bigdisk   /etc/auto.nas   --timeout=360" >> /etc/auto.master
    else
        cat <<EOF>/etc/auto.master
+auto.master
/data/bigdisk   /etc/auto.nas   --timeout=360
EOF
    fi

    if [ "${osver}" = "6" ]
    then
        # for slc6 machines
        chkconfig --level 345 autofs on
        /etc/init.d/autofs restart
    elif [ "${osver}" = "7" ]
    then
        # for cc7 machines
        systemctl enable autofs.service
        systemctl restart autofs.service
    fi
}

install_cactus() {
    # option 'c'
    echo Installing cactus packages...

    wget https://ipbus.web.cern.ch/ipbus/doc/user/html/_downloads/ipbus-sw.${ostype}${osver}.x86_64.repo -O /etc/yum.repos.d/ipbus-sw.repo
    yum -y groupinstall uhal

    wget https://svnweb.cern.ch/trac/cactus/export/HEAD/trunk/scripts/release/cactus-amc13.${ostype}${osver}.x86_64.repo -O /etc/yum.repos.d/cactus-amc13.repo
    yum -y groupinstall amc13

    if [ "${osver}" = "6" ]
    then
        # for slc6 machines
        chkconfig --level 0123456 controlhub off
        echo "If this is a controlhub machine, please execute with privileges:"
        echo "chkconfig --level 345 controlhub on"
        echo "service controlhub start"
    elif [ "${osver}" = "7" ]
    then
        # for cc7 machines
        systemctl disable controlhub.service
        echo "If this is a controlhub machine, please execute with privileges:"
        echo "systemctl enable controlhub.service"
        echo "systemctl daemon-reload"
        echo "systemctl start controlhub.service"
    fi
}

install_sysmgr() {
    # option 'S'
    echo Installing UW sysmgr RPMS...
    wget https://www.hep.wisc.edu/uwcms-repos/el${osver}/release/uwcms.repo -O /etc/yum.repos.d/uwcms.repo
    yum install -y freeipmi libxml++ libxml++-devel libconfuse libconfuse-devel
    yum install -y sysmgr

    if [ "${osver}" = "6" ]
    then
        # for slc6 machines
        chkconfig --level 0123456 sysmgr off
        echo "If this is a mchine that needs to communicate directly to a CTP7, please execute with privileges:"
        echo "chkconfig --level 345 sysmgr on"
        echo "service sysmgr start"
    elif [ "${osver}" = "7" ]
    then
        # for cc7 machines
        systemctl disable sysmgr.service
        echo "If this is a mchine that needs to communicate directly to a CTP7, please execute with privileges:"
        echo "systemctl enable sysmgr.service"
        echo "systemctl daemon-reload"
        echo "systemctl start sysmgr.service"
    fi
}

install_python() {
    # option 'p'
    sclpyvers=( python27 python33 python34 )
    for sclpy in "${sclpyvers[@]}"
    do
        prompt_confirm "Install ${sclpy}?"
        if [ "$?" = "0" ]
        then
            eval yum install -y ${sclpy}*
        fi
    done

    rhpyvers=( rh-python34 rh-python35 rh-python36 )
    for rhpy in "${rhpyvers[@]}"
    do
        prompt_confirm "Install ${rhpy}?"
        if [ "$?" = "0" ]
        then
            eval yum install -y ${rhpy}*
        fi
    done

    echo Installing python2.7 from source
    return
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
    yum install -y root root\* numpy root-numpy
}

create_accounts() {
    # option 'A'
    # may want LDAP users/groups to manage these as well
    # or even 'service' accounts, but better to have them tied to egroups
    # so one can log in with NICE credentials, a la cchcal
    ### generic gemuser group for running tests
    useradd gemuser
    usermod -u 5030 gemuser
    groupmod -g 5030 gemuser
    passwd gemuser
    chmod og+rx /home/gemuser

    ### gempro (production) account for running the system as an expert
    useradd gempro
    usermod -u 5050 gempro
    groupmod -g 5050 gempro
    passwd gempro
    chmod g+rx /home/gempro

    ### gemdev (development) account for running tests
    useradd gemdev
    usermod -u 5055 gemdev
    groupmod -g 5055 gemdev
    passwd gemdev
    chmod g+rx /home/gemdev

    ### daqbuild account for building the releases
    useradd daqbuild
    usermod -u 2050 daqbuild
    groupmod -g 2050 daqbuild
    passwd daqbuild

    ### daqpro account for building the releases
    useradd daqpro
    usermod -u 2055 daqpro
    groupmod -g 2055 daqpro
    passwd daqpro

    ### gemdaq group for DAQ pro tasks on the system
    groupadd gemdaq
    groupmod -g 2075 gemdaq

    ### gemsudoers group for administering the system
    groupadd gemsudoers
    groupmod -g 1075 gemsudoers
}

add_cern_users() {
    # option 'u'
    ### probably better to have a list of users that is imported from a text file/db
    # or better yet, an LDAP group!
    users=( amagnani bravo dldecker dorney evka huangh \
                     hyunyong ivai jlt mdalchen mgruchal \
                     mmaggi rband sturdy szaleski
          )

    for user in "${users[@]}"
    do
        sudo newcernuser.sh ${user}
    done
}

usage() {
echo -e "Usage: $0 [options]\n" \
" Options:\n" \
"    -a Setup new system with defaults for DAQ with accounts (implies -iCu)\n" \
"    -i Install only software (implies -xcmrS\n" \
"    -d Install developer tools\n" \
"    -c Install cactus tools (uhal and amc13)\n" \
"    -x install xdaq software\n" \
"    -y CTP7 Uptime\n" \
"    -n Setup mounting of NAS\n" \
"    -r Install root\n" \
"    -p Install additional python versions\n" \
"    -u <file> Add accounts of NICE users (specified in file)\n" \
"    -C Create common users and groups\n" \
"    -S Install UW system manager\n" \
"\n" \
"Plese report bugs to\n" \
"https://github.com/cms-gem-daq-project/cmsgemos\n"
}

while getopts "aidxynruscwmpSAh" opt; do
    case $opt in
        a)
            echo "Doing all steps"
            install_xdaq
            install_cactus
            install_root
            install_sysmgr
            install_misc_rpms
            add_cern_users
            create_accounts
            ;;
        i)
            echo "Installing necessary packages"
            install_xdaq
            install_cactus
            install_root
            install_sysmgr
            install_misc_rpms
            ;;
        d)
            install_developer_tools
            ;;
        x)
            install_xdaq
            ;;
        y)
            ;;
        n)
            setup_nas
            ;;
        r)
            install_root
            ;;
        s)
            ;;
        c)
            install_cactus
            ;;
        w)
            ;;
        m)
            install_misc_rpms
            ;;
        p)
            install_python
            ;;
        u)
            add_cern_users
            ;;
        S)
            install_sysmgr
            ;;
        A)
            create_accounts
            ;;
        h)
            echo >&2
            usage
            exit 1;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            usage
            exit 1;;
        [?])
            echo >&2
            usage
            exit 1;;
        # \?)
        #     echo "Invalid option: -$OPTARG" >&2
        #     ;;
    esac
done
