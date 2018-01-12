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
        read -u 3 -r -n 1 -p $'\e[1;35m'"${1:-Continue?}"$' [y/N/q]:\033[0m ' REPLY
        case $REPLY in
            [yY]) echo ; return 0 ;;
            [nN]) echo ; return 1 ;;
            [qQ]) echo ; exit 1 ;;
            *) echo ; return 1 ;;
        esac
    done 3<&0
}

new_service() {
    if [ -z "$2" ] || [[ ! "$2" =~ ^("on"|"off") ]]
    then
        echo -e "\033[1;33mPlease specify a service to configure, and whether it should be enabled ('on') or not ('off')\033[0m"
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

new_system_group() {
    if [ -z "$2" ]
    then
        echo -e "$0 $1 $2 $3"
        echo -e "\033[1;33mUsage: new_system_group <groupname> <gid>\033[0m"
        return 1
    fi

    echo -e "\033[1;32mCreating group $1 with gid $2\033[0m"
    groupadd $1
    groupmod -g $2 $1
}

new_system_user() {
    if [ -z "$3" ]
    then
        echo -e "$0 $1 $2 $3 $4"
        echo -e "\033[1;33mUsage: new_system_user <username> <uid> <primary gid>\033[0m"
        return 1
    fi

    echo -e "\033[1;32mCreating user $1 with uid $2 in primary group $3\033[0m"
    useradd $1
    usermod -u $2 $1
    groupmod -g $3 $1

    # machine local directory
    if [ ! -d /home/$1 ]
    then
        mkdir --context=system_u:object_r:user_home_dir_t:s0 /home/$1
    else
        echo "changing conditions for $1 home directory"
        mkdir --context=system_u:object_r:user_home_dir_t:s0 /tmp/testcons
        chcon --reference=/tmp/testcons /home/$1
        rm -rf /tmp/testcons
    fi

    # chmod a+rx /home/$1
    chown $1:$3 -R /home/$1

    prompt_confirm "Create directory for $1 on connected NAS?"
    if [ "$?" = "0" ]
    then
        mkdir -p --context=system_u:object_r:nfs_t:s0 /data/bigdisk/users/$1
        chown -R $1:zh /data/bigdisk/users/$1
    fi

    passwd $1
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
        read -r -p $'\e[1;32mPlease specify desired IP address:\033[0m ' ipaddr
        read -r -p $'\e[1;32mPlease specify desired network:\033[0m ' network
        read -r -p $'\e[1;32mPlease specify correct netmask:\033[0m ' netmask
    fi

    cfgbase="/etc/sysconfig/network-scripts"
    cfgfile="ifcfg-${netdev}"
    if [ -e ${cfgbase}/${cfgfile} ]
    then
        echo "Old config file is:"
        cat ${cfgbase}/${cfgfile}
        mv ${cfgbase}/${cfgfile} ${cfgbase}/.${cfgfile}.backup
        while IFS='' read -r line <&4 || [[ -n "$line" ]]
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
        done 4< ${cfgbase}/.${cfgfile}.backup
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
    yum -y install telnet htop arp-scan screen tmux

    yum -y install libuuid-devel e2fsprogs-devel readline-devel ncurses-devel curl-devel boost-devel \
        numactl-devel freeipmi-devel libusb libusbx libusb-devel libusbx-devel \
        protobuf-devel protobuf-lite-devel

    if [ "${osver}" = "6" ]
    then
        yum -y install mysql-devel mysql-server
        yum -y install sl-release-scl
    elif [ "${osver}" = "7" ]
    then
        yum -y install mariadb-devel mariadb-server
        yum -y install centos-release-scl
    else
        echo "Unknown release ${osver}"
    fi

    prompt_confirm "Install updated emacs?"
    if [ "$?" = "0" ]
    then
        if [ "${osver}" = "6" ]
        then
            wget http://pj.freefaculty.org/EL/pjku-EL6.repo -O /etc/yum.repos.d/pjku.repo
        elif [ "${osver}" = "7" ]
        then
            wget http://pj.freefaculty.org/EL/pjku.repo -O /etc/yum.repos.d/pjku.repo
        fi
        rpm --import http://pj.freefaculty.org/EL/PaulJohnson-BinaryPackageSigningKey
        yum -y install emacs --disablerepo=* --enablerepo=pjku
        yum -y install emacs emacs-auctex emacs-common emacs-filesystem emacs-git \
            emacs-git-el emacs-gnuplot emacs-gnuplot-el emacs-rpm-spec-mode emacs-yaml-mode
    fi
}

install_sysmgr() {
    # Option 'S'
    echo Installing UW sysmgr RPMS...
    wget https://www.hep.wisc.edu/uwcms-repos/el${osver}/release/uwcms.repo -O /etc/yum.repos.d/uwcms.repo
    yum -y install freeipmi libxml++ libxml++-devel libconfuse libconfuse-devel xinetd
    yum -y install sysmgr-complete
    # yum -y install sysmgr

    prompt_confirm "Setup machine to communicate directly to a CTP7?"
    if [ "$?" = "0" ]
    then
        new_service sysmgr on
        new_service xinetd on
    else
        new_service sysmgr off
        new_service xinetd off
    fi
}

install_root() {
    # Option 'r'
    echo Installing root...
    yum -y install root root-\*
}

install_python() {
    # Option 'p'
    sclpyvers=( python27 python33 python34 )
    for sclpy in "${sclpyvers[@]}"
    do
        prompt_confirm "Install ${sclpy}?"
        if [ "$?" = "0" ]
        then
            eval yum -y install ${sclpy}*
        fi
    done

    rhpyvers=( rh-python34 rh-python35 rh-python36 )
    for rhpy in "${rhpyvers[@]}"
    do
        prompt_confirm "Install ${rhpy}?"
        if [ "$?" = "0" ]
        then
            eval yum -y install ${rhpy}*
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
    yum -y install tcl-devel tk-devel

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
       yum -y install rh-git29*
    fi

    prompt_confirm "Install git-lfs?"
    if [ "$?" = "0" ]
    then
        curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.rpm.sh|bash
        yum -y install git-lfs
    fi

    rubyvers=( rh-ruby22 rh-ruby23 rh-ruby24 )
    for rver in "${rubyvers[@]}"
    do
        prompt_confirm "Install ${rver}?"
        if [ "$?" = "0" ]
        then
            yum -y install ${rver}*
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
                eval yum -y install ${dtool}-${tool}*
            fi
        done
    done
}

setup_nas() {
    # Option 'n'
    read -r -p $'\e[1;33mPlease specify the hostname of the NAS to set up:\033[0m ' nashost

    ping -c 5 -i 0.01 ${nashost}

    if [ ! "$?" = "0" ]
    then
        echo Unable to ping ${nashost}, are you sure the hostname is correct or the NAS is on?
        return 1
    fi

    echo Connecting to the NAS at ${nashost}
    cat <<EOF>/etc/auto.nas
GEMDAQ_Documentation    -context="system_u:object_r:nfs_t:s0",nosharecache,auto,rw,async,timeo=14,intr,rsize=32768,wsize=32768,tcp,nosuid,noexec,acl               ${nashost}:/share/gemdata/GEMDAQ_Documentation
GEM-Data-Taking         -context="system_u:object_r:httpd_sys_content_t:s0",nosharecache,auto,rw,async,timeo=14,intr,rsize=32768,wsize=32768,tcp,nosuid,noexec,acl ${nashost}:/share/gemdata/GEM-Data-Taking
sw                      -context="system_u:object_r:nfs_t:s0",nosharecache,auto,rw,async,timeo=14,intr,rsize=32768,wsize=32768,tcp,nosuid                          ${nashost}:/share/gemdata/sw
users                   -context="system_u:object_r:nfs_t:s0",nosharecache,auto,rw,async,timeo=14,intr,rsize=32768,wsize=32768,tcp,nosuid,acl                      ${nashost}:/share/gemdata/users
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
        echo -e "\033[1;31mNo Mellanox device detected, are you sure you have the interface installed?\033[0m"
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
        echo -e "\033[1;31mUnable to download Mellanox driver, trying NAS installed version...\033[0m"
        if [ -e /data/bigdisk/sw/${drvfile} ]
        then
            cp /data/bigdisk/sw/${drvfile} .
        else
            echo -e "\033[1;31m${drvfile} not found, exiting\033[0m"
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
    yumdownloader --source daq-xpcidrv --enablerepo=xdaq-sources

    if [ ! "$?" = "0" ]
    then
        echo -e "\033[1;31mFailed to download daq-xpcidrv sources\033[0m"
        return 1
    fi

    yum-builddep -y daq-xpcidrv-*.src.rpm

    # this part should be run as non-root
    echo "Entering user subshell"
    sudo -u gemuser echo "Entered user subshell"
    sudo -u gemuser rpm -ihv daq-xpcidrv-*.src.rpm
    sudo -u gemuser mkdir -p ~gemuser/rpmbuild/RPMS/x86_64/old
    sudo -u gemuser mv ~gemuser/rpmbuild/RPMS/x86_64/*.rpm ~gemuser/rpmbuild/RPMS/x86_64/old
    sudo -u gemuser rpmbuild -bb ~gemuser/rpmbuild/SPECS/daq-xpcidrv.x86_64*.spec
    sudo -u gemuser echo "Leaving user subshell"

    # should be run with elevated privileges
    yum -y remove daq-xpcidrv daq-xpcidrv-debuginfo daq-xpcidrv-devel
    yum -y install ~gemuser/rpmbuild/RPMS/x86_64/kernel-module-daq-xpcidrv-*.rpm
    yum -y install ~gemuser/rpmbuild/RPMS/x86_64/daq-xpcidrv-*.rpm

    lsmod|fgrep xpci
    if [ ! "$?" = "0" ]
    then
        modprobe xpci
    fi
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
            echo -e "\033[1;36mCurrent configuration for ${netdev} is:\033[0m"
            ifconfig ${netdev}
            while true
            do
                read -r -n 1 -p $'\e[1;34m Select interface type: local uTCA (1) or uFEDKIT (2)\033[0m ' REPLY
                case $REPLY in
                    [1]) echo -e "\033[1;36mConfiguring ${netdev} for local uTCA network...\033[0m"
                         configure_interface ${netdev} uTCA
                         break
                         ;;
                    [2]) echo -e "\033[1;36mConfiguring ${netdev} for uFEDKIT...\033[0m"
                         configure_interface ${netdev} uFEDKIT
                         break
                         ;;
                    [sS]) echo -e "\033[1;33mSkipping $REPLY...\033[0m" ; break ;;
                    [qQ]) echo -e "\033[1;34mQuitting...\033[0m" ; return 0 ;;
                    *) printf "\033[1;31m %s \n\033[0m" "Invalid choice, please specify an interface type, or press s(S) to skip, or q(Q) to quit";;
                esac
            done
        fi
    done
}

#### Compatibility with CTP7 (NEEDS TO BE WRITTEN)
connect_ctp7s() {
    # Option 'C'
    echo -e "\033[1;32mSetting up for ${hostname} for CTP7 usage\033[0m"
    # Updated /etc/sysmgr/sysmgr.conf to enable the GenericUW configuration module to support "WISC CTP-7" cards.
    if [ -e /etc/sysmgr/sysmgr.conf ]
    then
        mv /etc/sysmgr/sysmgr.conf.bak
    fi

    authkey="Aij8kpjf"

    cat <<EOF > /etc/sysmgr/sysmgr.conf
socket_port = 4681

# If ratelimit_delay is set, it defines the number of microseconds that the
# system manager will sleep after sending a command to a crate on behalf of a
# client application.  This can be used to avoid session timeouts due to
# excessive rates of requests.
#
# Note that this will suspend only the individual crate thread, and other
# crates will remain unaffected, as will any operation that does not access an
# individual crate.  The default, 0, is no delay.
ratelimit_delay = 100000

# If true, the system manager will run as a daemon, and send stdout to syslog.
daemonize = true

authentication {
	raw = { "${authkey}" }
	manage = { }
	read = { "" }
}

cardmodule {
	module = "GenericUW.so"
	config = {
		"ivtable=ipconfig.xml",
 		"poll_count=12",
 		"poll_delay=15"
	}
}

cardmodule {
        module = "GenericUW.so"
        config = {
                "ivtable=ipconfig.xml",
                # "poll_count=27448000",
                "poll_count=52596000",
                "poll_delay=30",
                "support=WISC CTP-6",
                "support=WISC CIOX",
                # "support=WISC CIOZ",
                "support=BU AMC13"
        }
}

cardmodule {
	module = "UWDirect.so"
	config = {
		"ivtable=ipconfig.xml",
                # "poll_count=27448000",
		"poll_count=105192000",
		"poll_delay=15",
		"support=WISC CTP-7#19",
		"support=WISC CIOZ#14"
	}
}
EOF

    # add crates

    cat <<EOF
crate {
	host = "${ipaddr}"
	mch = "${type}"
	username = ""
	password = ""
	authtype = none
	description = "${desc}"
}
EOF


    # Created /etc/sysmgr/ipconfig.xml to map geographic address assignments for crates 1 and 2 matching the /24
    # subnets associated with the MCHs listed for them in /etc/sysmgr/sysmgr.conf.  These addresses will occupy
    # 192.168.*.40 to 192.168.*.52 which nmap confirms are not in use.
    if [ -e /etc/sysmgr/ipconfig.xml ]
    then
        mv /etc/sysmgr/ipconfig.xml.bak
    fi

    authkey="Aij8kpjf"

    cat <<EOF > /etc/sysmgr/ipconfig.xml
<IVTable>
    <Crate number="1">
        <Slot number="1">
            <Card type="WISC CTP-7"><FPGA id="0">1 192 168 ${crate} ${slot} 255 255 0 0 192 168 0 180 192 168 0 180 0 0</FPGA></Card>
        </Slot>
    </Crate>

    <Crate number="2">
        <Slot number="1">
            <Card type="WISC CTP-7"><FPGA id="0">1 192 168 ${crate} ${slot} 255 255 0 0 192 168 0 180 192 168 0 180 0 0</FPGA></Card>
        </Slot>
    </Crate>
</IVTable>
EOF
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
    new_system_user gemuser 5075 5075
    chmod a+rx /home/gemuser

    ### gempro (production) account for running the system as an expert
    new_system_user gempro 5060 5060
    chmod g+rx /home/gempro

    ### gemdev (development) account for running tests
    new_system_user gemdev 5050 5050
    chmod g+rx /home/gemdev

    ### daqbuild account for building the releases
    prompt_confirm "\033[1;32mCreate user daqbuild?"
    if [ "$?" =  "0" ]
    then
        new_system_user daqbuild 2050 2050
        # chmod g+rx /home/daqbuild
    fi

    ### daqpro account for building the releases
    new_system_user daqpro 2055 2055
    chmod g+rx /home/daqpro

    # Groups for sudo rules only
    ### gemdaq group for DAQ pro tasks on the system
    new_system_group gemdaq 2075

    ### gemsudoers group for administering the system
    new_system_group gemsudoers 1075
}

add_cern_users() {
    # Option 'u'
    ### probably better to have a list of users that is imported from a text file/db
    # or better yet, an LDAP group!
    groups=( gempro gemdev daqpro gemdaq gemsudoers )
    while true
    do
        read -r -p $'\e[1;34mPlease specify text file with NICE users to add:\033[0m ' REPLY
        if [ -e "$REPLY" ]
        then
            while IFS='' read -r user <&4 || [[ -n "$user" ]]
            do
                prompt_confirm "Add ${user} to machine $HOST?"
                if [ "$?" = "0" ]
                then
                    if [ ! "$(fgrep ${user} /etc/passwd)" ]
                    then
                        echo "Adding NICE user $user"
                        ./newcernuser.sh ${user}
                    fi

                    for gr in "${groups[@]}"
                    do
                        if [ "$(fgrep ${gr} /etc/group)" ]
                        then
                            prompt_confirm "Add ${user} to ${gr} group?"
                            if [ "$?" = "0" ]
                            then
                                echo "Adding ${user} to ${gr} group"
                                usermod -aG ${gr} ${user}
                            fi
                        else
                            echo -e "\033[1;31mUnable to find '${gr}' group, have you created the standard users and groups on this machine yet?\033[0m"
                        fi
                    done
                fi
                echo -e "\033[1;35mDone setting up $user\033[0m"
            done 4< "$REPLY"
            return 0
        else
            case $REPLY in
                [qQ]) echo -e "\033[1;34mQuitting...\033[0m" ; return 0 ;;
                *) printf "\033[1;31m %s \n\033[0m" "File does not exist, please specify a file, or press q(Q) to quit";;
            esac
        fi
    done
}

usage() {
    echo -e \
         "Usage: $0 [options]\n" \
         " Options:\n" \
         "    -a Setup new system with defaults for DAQ with accounts (implies -iAN)\n" \
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
         "    -Z Install Xilinx USB drivers\n" \
         "    -V Install Xilinx Vivado\n" \
         "    -I Install Xilinx ISE\n" \
         "    -L Install Xilinx LabTools\n" \
         "    -u <file> Add accounts of NICE users (specified in file)\n" \
         "\n" \
         " Examples:\n" \
         "   Set up newly installed machine and add CERN NICE users: ${0} -au\n" \
         "   Set up newly installed machine and add uFEDKIT support: ${0} -aM\n" \
         "\n" \
         "Plese report bugs to\n" \
         "https://github.com/cms-gem-daq-project/cmsgemos\n"
}

while getopts "aixcmSrpdnNCMXAZVILuh" opt
do
    case $opt in
        a)
            echo -e "\033[1;34mDoing all steps necessary for new machine\033[0m"
            install_xdaq
            install_cactus
            install_root
            install_sysmgr
            install_misc_rpms
            create_accounts
            setup_network
            ;;
        i)
            echo -e "\033[1;33mInstalling necessary packages\033[0m"
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
        Z)
            : ;;
        V)
            : ;;
        I)
            : ;;
        L)
            : ;;
        u)
            add_cern_users ;;
        h)
            echo >&2 ; usage ; exit 1 ;;
        \?)
            echo -e "\033[1;31mInvalid option: -$OPTARG\033[0m" >&2 ; usage ; exit 1 ;;
        [?])
            echo >&2 ; usage ; exit 1 ;;
    esac
done
