#!/bin/bash

arch=`uname -i`
# setup_version=2.7.2-1.cmsos12
osver=`expr $(uname -r) : '.*el\([0-9]\)'`
ostype=centos
if [ "${osver}" = "6" ]
then
    ostype=slc
fi

###### Helper functions #####
prompt_confirm() {
    while true
    do
        read -r -n 1 -p "${1:-Continue?} [y/n]: " REPLY
        case $REPLY in
            [yY]) echo ; return 0 ;;
            [nN]) echo ; return 1 ;;
            *) echo ; return 1 ;;
        esac
    done
}

new_service() {
    if [ -z "$2" ] || [[ ! "$2" =~ ^("on"|"off") ]]
    then
        echo "Please specify a service to configure, and whether it should be enabled ('on') or not ('off')"
        return 1
    fi

    newserv=${1}
    status=${2}
    
    if [ "${osver}" = "6" ]
    then
        # for slc6 machines
        chkconfig --level 345 $1 ${status}
        if [ "${status}" = "on" ]
        then
           service ${newserv} restart
        else
           service ${newserv} stop
        fi
    elif [ "${osver}" = "7" ]
    then
        # for cc7 machines
        if [ "${status}" = "on" ]
        then
            svsta="enable"
            svcmd="restart"
        else
            svsta="disable"
            svcmd="stop"
        fi
        systemctl ${svsta} ${newserv}.service
        systemctl daemon-reload
        systemctl ${svcmd} ${newserv}.service
    fi
}

configure_interface() {
    if [ -z "$2" ] || [[ ! "$2" =~ ^("uTCA"|"uFEDKIT") ]]
    then
        echo -e \
             "Usage: configure_interface <device> <type>\n" \
             "   device must be listed in /sys/class/net\n" \
             "   type myst be one of:\n" \
             "     uTCA for uTCA on local network\n" \
             "     uFEDKIT for uFEDKIT on 10GbE\n"
        return 1
    fi

    netdev=$1
    type=$2

    ipaddr=10.0.0.5
    netmask=255.255.255.0
    network=10.0.0.0
    if [ $type = "uTCA" ]
    then
        read -r -p "Please specify desired IP address: " ipaddr
        read -r -p "Please specify desired network: " network
        read -r -p "Please specify correct netmask: " netmask
    fi

    cfgbase="/etc/sysconfig/network-scripts"
    cfgfile="ifcfg-${netdev}"
    if [ -e ${cfgbase}/${cfgfile} ]
    then
        echo "Old config file is:"
        cat ${cfgbase}/${cfgfile}
        mv ${cfgbase}/${cfgfile} ${cfgbase}/.${cfgfile}.backup
        while IFS='' read -r line || [[ -n "$line" ]]
        do
            if [[ "${line}" =~ ^("IPADDR"|"NETWORK"|"NETMASK") ]]
            then
                #skip
                :
            elif [[ "${line}" =~ ^("IPV6"|"NM_CON") ]]
            then
                echo "#${line}" >> ${cfgbase}/${cfgfile}
            elif [[ "${line}" =~ ^("BOOTPROTO") ]]
            then
                echo "BOOTPROTO=none" >> ${cfgbase}/${cfgfile}
            elif [[ "${line}" =~ ^("DEFROUTE") ]]
            then
                echo "DEFROUTE=no" >> ${cfgbase}/${cfgfile}
            elif [[ "${line}" =~ ^("USERCTL") ]]
            then
                echo "USERCTL=no" >> ${cfgbase}/${cfgfile}
            elif [[ "${line}" =~ ^("ONBOOT") ]]
            then
                echo "ONBOOT=yes" >> ${cfgbase}/${cfgfile}
            else
                echo "${line}" >> ${cfgbase}/${cfgfile}
            fi
        done < ${cfgbase}/.${cfgfile}.backup
    else
        echo "No config file exists, creating..."
        echo "TYPE=Ethernet" >> ${cfgbase}/${cfgfile}
        echo "NM_CONTROLLED=no" >> ${cfgbase}/${cfgfile}
        echo "BOOTPROTO=none" >> ${cfgbase}/${cfgfile}
        echo "ONBOOT=yes" >> ${cfgbase}/${cfgfile}
        echo "DEFROUTE=no" >> ${cfgbase}/${cfgfile}
    fi

    echo "IPADDR=${ipaddr}" >> ${cfgbase}/${cfgfile}
    echo "NETWORK=${network}" >> ${cfgbase}/${cfgfile}
    echo "NETMASK=${netmask}" >> ${cfgbase}/${cfgfile}

    echo "New config file is:"
    cat ${cfgbase}/${cfgfile}    
}

###### Main functions #####
### copied from HCAL setup scripts
install_xdaq() {
    # Option 'x'
    # doesn't include SRPMS
    wget https://svnweb.cern.ch/trac/cactus/export/HEAD/trunk/scripts/release/xdaq.${ostype}${osver}.x86_64.repo \
         -O /etc/yum.repos.d/xdaq.repo
    perl -pi -e 's|# ||g' /etc/yum.repos.d/xdaq.repo

    echo Installing XDAQ...

    # generic XDAQ
    yum -y remove openslp
    yum -y groupinstall extern_coretools coretools extern_powerpack powerpack

    # add-on packages
    yum --skip-broken -y groupinstall database_worksuite general_worksuite dcs_worksuite hardware_worksuite

    # for fedKit
    prompt_confirm "Install kernel modules for uFEDKIT?"
    if [ "$?" = "0" ]
    then
        yum -y groupinstall daq_kernel_modules
    fi

    # debug modules
    prompt_confirm "Install xdaq debuginfo modules?"
    if [ "$?" = "0" ]
    then
        yum -y groupinstall exetern_coretools_debuginfo coretools_debuginfo extern_powerpack_debuginfo powerpack_debuginfo \
            database_worksuite_debuginfo general_worksuite_debuginfo dcs_worksuite_debuginfo hardware_worksuite_debuginfo
    fi
}

install_cactus() {
    # Option 'c'
    echo Installing cactus packages...

    prompt_confirm "Install uHAL?"
    if [ "$?" = "0" ]
    then
        wget https://ipbus.web.cern.ch/ipbus/doc/user/html/_downloads/ipbus-sw.${ostype}${osver}.x86_64.repo -O /etc/yum.repos.d/ipbus-sw.repo
        yum -y groupinstall uhal

        prompt_confirm "Setup machine as controlhub?"
        if [ "$?" = "0" ]
        then
            new_service controlhub on
        else
            new_service controlhub off
        fi
    fi

    prompt_confirm "Install amc13 libraries?"
    if [ "$?" = "0" ]
    then
        wget https://svnweb.cern.ch/trac/cactus/export/HEAD/trunk/scripts/release/cactus-amc13.${ostype}${osver}.x86_64.repo -O /etc/yum.repos.d/cactus-amc13.repo
        yum -y groupinstall amc13
    fi
}

install_misc_rpms() {
    # Option 'm'
    echo Installing miscellaneous RPMS...
    yum install -y libuuid-devel e2fsprogs-devel readline-devel ncurses-devel curl-devel boost-devel \
        mysql-devel mysql-server numactl-devel freeipmi-devel arp-scan \
        libusb libusbx libusb-devel libusbx-devel
    yum install -y centos-release-scl
}

install_sysmgr() {
    # Option 'S'
    echo Installing UW sysmgr RPMS...
    wget https://www.hep.wisc.edu/uwcms-repos/el${osver}/release/uwcms.repo -O /etc/yum.repos.d/uwcms.repo
    yum install -y freeipmi libxml++ libxml++-devel libconfuse libconfuse-devel
    yum install -y sysmgr

    prompt_confirm "Setup machine to communicate directly to a CTP7?"
    if [ "$?" = "0" ]
    then
        new_service sysmgr on
    else
        new_service sysmgr off
    fi
}

install_root() {
    # Option 'r'
    echo Installing root...
    yum install -y root root-\*
}

install_python() {
    # Option 'p'
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

    return 0

    if [ ! -z "${1}" ]
    then
        echo No python version specified
        return 1
    fi

    pyver=${1}
    echo "Installing python2.7 (${pyver}) from source, no longer best option probably!"

    # install dependencies
    yum install -y tcl-devel tk-devel

    # common source directory, may already exist
    mkdir -p /data/bigdisk/sw/python2.7/
    cd /data/bigdisk/sw/python2.7/

    # setup scripts
    wget https://bootstrap.pypa.io/ez_setup.py
    wget https://www.python.org/ftp/python/${pyver}/Python-${pyver}.tar.xz
    tar xf Python-${pyver}.tar.xz
    cd Python-${pyver}
    ./configure --prefix=/usr/local --enable-unicode=ucs4 --enable-shared LDFLAGS="-Wl,-rpath /usr/local/lib"

    # clean up in case previously compiled and then compile
    make clean && make -j5

    # do the installation
    make altinstall
    
    cd /data/bigdisk/sw/python2.7/
    /usr/local/bin/python2.7 ez_setup.py
    /usr/local/bin/easy_install-2.7 pip
    
    # add links to the python26 packages we'll need, how to do this generally
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

install_developer_tools() {
    # Option 'd'
    echo Installing developer tools RPMS...

    prompt_confirm "Install rh-git29?"
    if [ "$?" = "0" ]
    then
       yum install -y rh-git29*
    fi

    prompt_confirm "Install git-lfs?"
    if [ "$?" = "0" ]
    then
        curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.rpm.sh|bash
        yum install git-lfs
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
    # Option 'n'
    read -r -p "Please specify the hostname of the NAS you'd like to setup: " nashost

    ping -c 5 -i 0.01 ${nashost}

    if [ ! "$?" = "0" ]
    then
        echo Unable to ping ${nashost}, are you sure the hostname is correct or the NAS is on?
        return 1
    fi
    
    echo Connecting to the NAS at ${nashost}
    cat <<EOF>/etc/auto.nas
GEMDAQ_Documentation    -context="system_u:object_r:nfs_t:s0",nosharecache,auto,rw,async,timeo=14,intr,rsize=32768,wsize=32768,tcp,nosuid,noexec,acl               gem904nas01:/share/gemdata/GEMDAQ_Documentation
GEM-Data-Taking         -context="system_u:object_r:httpd_sys_content_t:s0",nosharecache,auto,rw,async,timeo=14,intr,rsize=32768,wsize=32768,tcp,nosuid,noexec,acl gem904nas01:/share/gemdata/GEM-Data-Taking
sw                      -context="system_u:object_r:nfs_t:s0",nosharecache,auto,rw,async,timeo=14,intr,rsize=32768,wsize=32768,tcp,nosuid                          gem904nas01:/share/gemdata/sw
users                   -context="system_u:object_r:nfs_t:s0",nosharecache,auto,rw,async,timeo=14,intr,rsize=32768,wsize=32768,tcp,nosuid,acl                      gem904nas01:/share/gemdata/users
+auto.nas
EOF
    if [ -f /etc/auto.master ]
    then
        nascfg=$(fgrep auto.nas /etc/auto.master)
        if [ ! "$?" = "0" ]
        then
            echo "/data/bigdisk   /etc/auto.nas  --timeout=600 --ghost --verbose" >> /etc/auto.master
        fi
    else
        cat <<EOF>/etc/auto.master
+auto.master
/data/bigdisk   /etc/auto.nas   --timeout=360
EOF
    fi

    new_service autofs on
}

#### Drivers for special hardware ###
install_mellanox_driver() {
    # option '-M'
    lspci -v | grep Mellanox
    if [ ! "$?" = "0" ]
    then
        echo "No Mellanox device detected, are you sure you have the interface installed?"
        return 1
    fi
    crel=$(cat /etc/system-release)
    ncrel=${crel//[!0-9.]/}
    sncrel=${ncrel%${ncrel:3}}
    mlnxver=4.2-1.0.1.0
    drvfile=mlnx-en-${mlnxver}-rhel${sncrel}-x86_64.tgz
    curdir=$PWD
    cd /tmp/

    wget http://www.mellanox.com/downloads/ofed/MLNX_EN-${mlnxver}/${drvfile} -O ${drvfile}
    if [ ! "$?" = "0" ]
    then
        echo "Unable to download Mellanox driver, trying NAS installed version..."
        if [ -e /data/bigdisk/sw/${drvfile} ]
        then
            cp /data/bigdisk/sw/${drvfile} .
        else
            echo "${drvfile} not found, exiting"
            cd $curdir
            return 1
        fi
    fi

    # Unpack the downloaded tarball
    tar xzvf ${drvfile}
    # Change the working directory.
    cd mlnx-en-${mlnxver}-rhel${sncrel}-x86_64

    # # Run the installation script, failing to get the init script...
    # ./install
    
    # RPM with YUM
    rpm --import RPM-GPG-KEY-Mellanox
    cat <<EOF > /etc/yum.repos.d/mellanox.repo
[mlnx_en]
name=MLNX_EN Repository
baseurl=file://${PWD}/RPMS_ETH
enabled=0
gpgkey=file://${PWD}/RPM-GPG-KEY-Mellanox
gpgcheck=1
EOF
    yum -y install mlnx-en-eth-only --disablerepo=* --enablerepo=mlnx_en
    
    # Load the driver.
    new_service mlnx-en.d on
    cd $curdir
    return 0
}

update_xpci_driver() {
    # option 'X'
    curdir=$PWD
    mkdir -p /tmp/xpci_update
    cd /tmp/xpci_update
    yumdownloader --source daq-xpcidrv

    if [ ! "$?" = "0" ]
    then
        echo "Failed to download daq-xpcidrv sources"
        return 1
    fi

    yum-builddep -y daq-xpcidrv-*.src.rpm
    rpm -ihv daq-xpcidrv-*.src.rpm
    cd ~/rpmbuild
    mkdir -p RPMS/x86_64/old
    mv RPMS/x86_64/*.rpm RPMS/x86_64/old
    rpmbuild -bb SPECS/daq-xpcidrv.x86_64_slc6.spec
    cd RPMS/x86_64
    yum remove daq-xpcidrv daq-xpcidrv-debuginfo daq-xpcidrv-devel
    yum install kernel-module-daq-xpcidrv-*.rpm
    yum install daq-xpcidrv-*.rpm
    lsmod|fgrep xpc
    cd ${curdir}
}

#### Networking for certain devices ####
setup_network() {
    # Option 'N'
    netdevs=( $(ls /sys/class/net |egrep -v "virb|lo") )
    for netdev in "${netdevs[@]}"
    do
        prompt_confirm "Configure network device: ${netdev}?"
        if [ "$?" = "0" ]
        then
            echo "Current configuration for ${netdev} is:"
            ifconfig ${netdev}
            while true
            do
                read -r -n 1 -p "Select interface type: local uTCA (1) or uFEDKIT (2) " REPLY
                case $REPLY in
                    [1]) echo "Configuring ${netdev} for local uTCA network..."
                         configure_interface ${netdev} uTCA
                         break
                         ;;
                    [2]) echo "Configuring ${netdev} for uFEDKIT..."
                         configure_interface ${netdev} uFEDKIT
                         break
                         ;;
                    [sS]) echo "Skipping $REPLY..." ; break ;;
                    [qQ]) echo "Quitting..." ; return 0 ;;
                    *) printf "\033[31m %s \n\033[0m" "Invalid choice, please specify an interface type, or press s(S) to skip, or q(Q) to quit";;
                esac
            done
        fi
    done
}

#### Compatibility with CTP7 (NEEDS TO BE WRITTEN)
connect_ctp7s() {
    # Option 'C'
    echo "Setting up for ${hostname} for CTP7 usage"
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

#### Accounts and NICE users
create_accounts() {
    # Option 'A'
    # may want LDAP users/groups to manage these as well
    # or even 'service' accounts, but better to have them tied to egroups
    # so one can log in with NICE credentials, a la cchcal
    ### generic gemuser group for running tests
    echo "Creating user gemuser"
    useradd gemuser
    usermod -u 5030 gemuser
    groupmod -g 5030 gemuser
    passwd gemuser
    chmod og+rx /home/gemuser

    ### gempro (production) account for running the system as an expert
    echo "Creating user gempro"
    useradd gempro
    usermod -u 5050 gempro
    groupmod -g 5050 gempro
    passwd gempro
    chmod g+rx /home/gempro

    ### gemdev (development) account for running tests
    echo "Creating user gemdev"
    useradd gemdev
    usermod -u 5055 gemdev
    groupmod -g 5055 gemdev
    passwd gemdev
    chmod g+rx /home/gemdev

    ### daqbuild account for building the releases
    prompt_confirm "Create user daqbuild?"
    if [ "$?" =  "0" ]
    then
        useradd daqbuild
        usermod -u 2050 daqbuild
        groupmod -g 2050 daqbuild
        passwd daqbuild
    fi

    ### daqpro account for building the releases
    echo "Creating user daqpro"
    useradd daqpro
    usermod -u 2055 daqpro
    groupmod -g 2055 daqpro
    passwd daqpro

    ### gemdaq group for DAQ pro tasks on the system
    echo "Creating group gemdaq"
    groupadd gemdaq
    groupmod -g 2075 gemdaq

    ### gemsudoers group for administering the system
    echo "Creating group gemsudoers"
    groupadd gemsudoers
    groupmod -g 1075 gemsudoers
}

add_cern_users() {
    # Cption 'u'
    ### probably better to have a list of users that is imported from a text file/db
    # or better yet, an LDAP group!
    while true
    do
        read -r -p "Please specify text file with NICE users to add: " REPLY
        if [ -e "$REPLY" ]
        then
            while IFS='' read -r user || [[ -n "$user" ]]
            do
                echo "Adding NICE user $user"
                ./newcernuser.sh ${user}
            done < "$REPLY"
            return 0
        else
            case $REPLY in
                [qQ]) echo "Quitting..." ; return 0 ;;
                *) printf "\033[31m %s \n\033[0m" "File does not exist, please specify a file, or press q(Q) to quit";;
            esac
        fi
    done
}

usage() {
    echo -e \
         "Usage: $0 [options]\n" \
         " Options:\n" \
         "    -a Setup new system with defaults for DAQ with accounts (implies -iCu)\n" \
         "    -i Install only software (implies -xcmrS\n" \
         "    -x install xdaq software\n" \
         "    -c Install cactus tools (uhal and amc13)\n" \
         "    -m Install miscellaneous packages\n" \
         "    -S Install UW system manager\n" \
         "    -r Install root\n" \
         "    -p Install additional python versions\n" \
         "    -d Install developer tools\n" \
         "    -n Setup mounting of NAS\n" \
         "    -C Set up CTP7 connections\n" \
         "    -N Set up network interfaces\n" \
         "    -M Install Mellanox 10GbE drivers for uFEDKIT\n" \
         "    -X Install/update xpci drivers\n" \
         "    -A Create common users and groups\n" \
         "    -u <file> Add accounts of NICE users (specified in file)\n" \
         "\n" \
         "Plese report bugs to\n" \
         "https://github.com/cms-gem-daq-project/cmsgemos\n"
}

while getopts "aixcmSrpdnNCMXAuh" opt
do
    case $opt in
        a)
            echo "Doing all steps necessary for new machine"
            install_xdaq
            install_cactus
            install_root
            install_sysmgr
            install_misc_rpms
            add_cern_users
            create_accounts
            setup_nas
            setup_network
            ;;
        i)
            echo "Installing necessary packages"
            install_xdaq
            install_cactus
            install_root
            install_sysmgr
            install_misc_rpms
            ;;
        x)
            install_xdaq ;;
        c)
            install_cactus ;;
        m)
            install_misc_rpms ;;
        S)
            install_sysmgr ;;
        r)
            install_root ;;
        p)
            install_python ;;
        d)
            install_developer_tools ;;
        n)
            setup_nas ;;
        N)
            setup_network ;;
        C)
            connect_ctp7s ;;
        M)
            install_mellanox_driver ;;
        X)
            update_xpci_driver ;;
        A)
            create_accounts ;;
        u)
            add_cern_users ;;
        h)
            echo >&2 ; usage ; exit 1 ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2 ; usage ; exit 1 ;;
        [?])
            echo >&2 ; usage ; exit 1 ;;
    esac
done
