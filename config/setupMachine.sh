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
            elif [[ "${line}" =~ ^("ZONE") ]]
            then
                echo "ZONE=trusted" >> ${cfgbase}/${cfgfile}
            else
                echo "${line}" >> ${cfgbase}/${cfgfile}
            fi
        done 4< ${cfgbase}/.${cfgfile}.backup
    else
        echo "No config file exists, creating..."
        echo "TYPE=Ethernet"    >> ${cfgbase}/${cfgfile}
        echo "NM_CONTROLLED=no" >> ${cfgbase}/${cfgfile}
        echo "BOOTPROTO=none"   >> ${cfgbase}/${cfgfile}
        echo "ONBOOT=yes"       >> ${cfgbase}/${cfgfile}
        echo "DEFROUTE=no"      >> ${cfgbase}/${cfgfile}
        echo "ZONE=trusted"     >> ${cfgbase}/${cfgfile}
    fi

    echo "IPADDR=${ipaddr}"   >> ${cfgbase}/${cfgfile}
    echo "NETWORK=${network}" >> ${cfgbase}/${cfgfile}
    echo "NETMASK=${netmask}" >> ${cfgbase}/${cfgfile}

    echo "New config file is:"
    cat ${cfgbase}/${cfgfile}

    # Configure firewall:
    ## cc7
    # firewall-cmd
    # zone trusted
    # sources 192.168.2.0/24

    ## slc6
    # iptables
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

install_cactus() {
    # Option 'c'
    echo Installing cactus packages...
    uhalVersion="2.6"
    useUHAL25=0
    prompt_confirm "Install uHAL?"
    if [ "$?" = "0" ]
    then
        while true
        do
            read -r -n 1 -p "Select uhal version: 2.5 (1), 2.6 (2) " REPLY
            case $REPLY in
                [1]) echo "Installing ipbus uhal version 2.5"
                     uhalVersion="2.5"
                     useUHAL25=1
                     break
                     ;;
                [2]) echo "Installing ipbus uhal version 2.6"
                     uhalVersion="2.6/repos"
                     useUHAL25=0
                     break
                     ;;
                [sS]) echo "Skipping $REPLY..." ; break ;;
                [qQ]) echo "Quitting..." ; return 0 ;;
                *) printf "\033[31m %s \n\033[0m" "Invalid choice, please specify a uhal version, press s(S) to skip, or q(Q) to quit";;
            esac
        done

        cat <<EOF > /etc/yum.repos.d/ipbus-sw.repo
[ipbus-sw-base]
name=CACTUS Project Software Repository
baseurl=http://www.cern.ch/ipbus/sw/release/${uhalVersion}/${ostype}${osver}_x86_64/base/RPMS
enabled=1
gpgcheck=0

[ipbus-sw-updates]
name=CACTUS Project Software Repository Updates
baseurl=http://www.cern.ch/ipbus/sw/release/${uhalVersion}/${ostype}${osver}_x86_64/updates/RPMS
enabled=1
gpgcheck=0
EOF

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
        if [ ${osver} = "6" ] && [ $useUHAL25 = "0" ]
        then
            echo "AMC13 libraries on ${ostype}${osver} are incompatible with uhal 2.6"
            continue
        elif [ ${osver} = "6" ]
        then
            amc13Version="1.2"
        elif [ ${useUHAL25} = "0" ]
        then
            amc13Version="1.2/uhal2.6"
        fi
        cat <<EOF > /etc/yum.repos.d/amc13-sw.repo
[cactus-amc13-base]
name=CACTUS Project Software Repository for amc13 packages
baseurl=http://www.cern.ch/cactus/release/amc13/${amc13Version}/${ostype}${osver}_x86_64/base/RPMS
enabled=1
gpgcheck=0

[cactus-updates]
name=CACTUS Project Software Repository Updates for amc13 packages
baseurl=http://www.cern.ch/cactus/release/amc13/${amc13Version}/${ostype}${osver}_x86_64/updates/RPMS
enabled=1
gpgcheck=0
EOF
        yum -y groupinstall amc13
    fi
}

install_misc_rpms() {
    # Option 'm'
    echo Installing miscellaneous RPMS...
    yum -y install tree telnet htop arp-scan screen tmux cppcheck

    yum -y install libuuid-devel e2fsprogs-devel readline-devel ncurses-devel curl-devel boost-devel \
        numactl-devel libusb-devel libusbx-devel protobuf-devel protobuf-lite-devel pugixml-devel

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
    yum -y install freeipmi-devel libxml++-devel libconfuse-devel xinetd dnsmasq
    yum -y install sysmgr-complete
    # yum -y install sysmgr

    prompt_confirm "Setup machine to communicate directly to a CTP7?"
    if [ "$?" = "0" ]
    then
        connect_ctp7s
        new_service sysmgr on
        new_service xinetd on
        new_service dnsmasq on
    else
        new_service sysmgr off
        new_service xinetd off
        new_service dnsmasq off
    fi
}

install_root() {
    # Option 'r'
    echo Installing root...
    yum -y install root root-\* python\*-root
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
        echo -e "\n\033[1;36mCurrent configuration for ${netdev} is:\033[0m"
        ifconfig ${netdev}
        prompt_confirm "Configure network device: ${netdev}?"
        if [ "$?" = "0" ]
        then
            while true
            do
                read -r -n 1 -p $'\e[1;34m Select interface type: local uTCA (1) or uFEDKIT (2) or dnsmasq (3)\033[0m ' REPLY
                case $REPLY in
                    [1]) echo -e "\n\033[1;36mConfiguring ${netdev} for local uTCA network...\033[0m"
                         configure_interface ${netdev} uTCA
                         break
                         ;;
                    [2]) echo -e "\n\033[1;36mConfiguring ${netdev} for uFEDKIT...\033[0m"
                         configure_interface ${netdev} uFEDKIT
                         break
                         ;;
                    [3]) echo -e "\n\033[1;36mConfiguring ${netdev} for dnsmasq...\033[0m"
                         cat <<EOF > /etc/dnsmasq.d/ctp7
interface=${netdev}
EOF

                         break
                         ;;
                    [sS]) echo -e "\033[1;33mSkipping $REPLY...\033[0m" ; break ;;
                    [qQ]) echo -e "\033[1;34mQuitting...\033[0m" ; return 0 ;;
                    *) printf "\033[1;31m %s \n\033[0m" "Invalid choice, please specify an interface type, press s(S) to skip, or q(Q) to quit";;
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
        mv /etc/sysmgr/sysmgr.conf /etc/sysmgr/sysmgr.conf.bak
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

EOF

    # add crates
    # take input from prompt for ipaddress, type, password, and description
    nShelves=0
    while true
    do
        prompt_confirm "Add uTCA shelf to sysmgr config?"
        if [ "$?" = "0" ]
        then
            while true
            do
                read -r -n 1 -p $'\e[1;34m Add uTCA shelf with MCH of type: VadaTech (1) or NAT (2)\033[0m ' REPLY
                case $REPLY in
                    [1]) echo -e "\n\033[1;36mAdding uTCA shelf with VadaTech MCH...\033[0m"
                         type="VadaTech"
                         password="vadatech"
                         break
                         ;;
                    [2]) echo -e "\n\033[1;36mAdding uTCA shelf with NAT MCH...\033[0m"
                         type="NAT"
                         password=""
                         break
                         ;;
                    [sS]) echo -e "\033[1;33mSkipping $REPLY...\033[0m" ; break ;;
                    [qQ]) echo -e "\033[1;34mQuitting...\033[0m" ; return 0 ;;
                    *) printf "\033[1;31m %s \n\033[0m" "Invalid choice, please specify an MCH type, press s(S) to skip, or q(Q) to quit";;
                esac
            done

            while true
            do
                read -r -p $'\e[1;34mPlease specify the IP address of the MCH:\033[0m ' REPLY
                rx='([1-9]?[0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])'
                oct='(0[0-7]{0,2})|([1-9][0-9])|(1[0-9][0-9])|(2[0-4][0-9])|(25[0-5])'
                rex="^$oct.$ot.$oct.$oct$"
                re1="^([0-9]{1,3}\.){3}(\.?[0-9]{1,3})$"
                re2="^([0-9]{1,3}.){3}(.?[0-9]{1,3})$"
                re3='^([0-9]{1,3}.){3}(.?[0-9]{1,3})$'
                # if ! [[ "$REPLY" =~ $re1 ]]
                # if ! [[ "$REPLY" =~ $re2 ]]
                if ! [[ "$REPLY" =~ $re3 ]]
                then
                    printf "\033[1;31m %s \n\033[0m" "Invalid IP address ${REPLY} specified"
                    continue
                fi
                ipaddr=$REPLY
                break
            done

            read -r -p $'\e[1;34mPlease enter a description for this uTCA shelf:\033[0m ' REPLY
            desc=$REPLY
            cat <<EOF >> /etc/sysmgr/sysmgr.conf
crate {
	host = "${ipaddr}"
	mch = "${type}"
	username = ""
	password = "${password}"
	authtype = none
	description = "${desc}"
}
EOF
            nShelves=$((nShelves+1))
        elif [ "$?" = "1" ]
        then
            echo -e "\n\033[1;36mDone adding ${nShelves} shelves, now moving on to configs\033[0m"
            break
        fi
    done

    cat <<EOF >> /etc/sysmgr/sysmgr.conf
# *** Modules ***
#
# These modules will be loaded in the order specified here.  When a new card is
# detected, they will be checked in reverse order to determine which module
# will service that card.  If no module claims a card, it will be serviced by
# the system manager with no special functionality.

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
                "support=WISC CIOZ",
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

    # Created /etc/sysmgr/ipconfig.xml to map geographic address assignments for crates 1 and 2 matching the /24
    # subnets associated with the MCHs listed for them in /etc/sysmgr/sysmgr.conf.  These addresses will occupy
    # 192.168.*.40 to 192.168.*.52 which nmap confirms are not in use.
    if [ -e /etc/sysmgr/ipconfig.xml ]
    then
        mv /etc/sysmgr/ipconfig.xml /etc/sysmgr/ipconfig.xml.bak
    fi

    if [ -d /etc/cmsgemos ]
    then
        if [ -e /etc/cmsgemos/connections.xml ]
        then
            mv /etc/cmsgemos/connections.xml /etc/cmsgemos/connections.xml.bak
        fi
    else
        mkdir /etc/cmsgemos
    fi

    echo -e "\033[1;32mCreating configuratinos assuming a 'local' network topology and only CTP7s, if this is not appropriate for your use case, please modify the resulting files found at /etc/sysmgr/ipconfig.xml,  /etc/cmsgemos/connections.xml\033[0m"
    authkey="Aij8kpjf"

    cat <<EOF > /etc/cmsgemos/connections.xml
<?xml version="1.0" encoding="UTF-8"?>

<connections>
EOF
    cat <<EOF > /etc/sysmgr/ipconfig.xml
<IVTable>
EOF
    if [ $nShelves = "0" ]
    then
        nShelves=1
    fi

    for crate in $(eval echo "{1..$nShelves}")
    do
        cid=$(printf '%02d' ${crate})
        cat <<EOF >> /etc/sysmgr/ipconfig.xml
    <Crate number="${crate}">
EOF
    cat <<EOF >> /etc/hosts

192.168.${crate}.10 mch-c${cid} mch-c${cid}.utca
192.168.1.13 amc-c${cid}-s13-t1 amc-c${cid}-s13-t1.utca
192.168.1.14 amc-c${cid}-s13-t2 amc-c${cid}-s13-t2.utca
EOF
    cat <<EOF >> /etc/cmsgemos/connections.xml
  <!-- uTCA shelf ${crate} -->
  <connection id="gem.shelf${cid}.amc13.T1" uri="chtcp-2.0://localhost:10203?target=amc-c${cid}-13-t1:50001"
              address_table="file://${AMC13_ADDRESS_TABLE_PATH}/AMC13XG_T1.xml" />
  <connection id="gem.shelf${cid}.amc13.T2" uri="chtcp-2.0://localhost:10203?target=amc-c${cid}-13-t2:50001"
              address_table="file://${AMC13_ADDRESS_TABLE_PATH}/AMC13XG_T2.xml" />
EOF
        for slot in {1..12}
        do
            sid=$(printf '%02d' $slot)
            cat <<EOF >> /etc/sysmgr/ipconfig.xml
        <Slot number="${slot}">
            <Card type="WISC CTP-7">
                <FPGA id="0">${slot} 192 168 ${crate} $((slot+40)) 255 255 0 0 192 168 0 180 192 168 0 180 0 0</FPGA>
            </Card>
        </Slot>
EOF
            cat <<EOF >> /etc/hosts
192.168.1.$((slot+40)) amc-c${cid}-s${sid} amc-c${cid}-s${sid}.utca
EOF
            cat <<EOF >> /etc/cmsgemos/connections.xml
  <!-- AMC slot ${slot} shelf ${crate} -->
  <connection id="gem.shelf${cid}.amc03" uri="ipbustcp-2.0://amc-c${cid}-s${sid}:60002"
	      address_table="file://${GEM_ADDRESS_TABLE_PATH}/uhal_gem_amc_ctp7_amc.xml" />
EOF
            for lin in {0..11}
            do
                lid=$(printf '%02d' $lin)
            cat <<EOF >> /etc/cmsgemos/connections.xml
  <connection id="gem.shelf${cid}.amc03.optohybrid${lid}" uri="ipbustcp-2.0://amc-c${cid}-s${sid}:60002"
	      address_table="file://${GEM_ADDRESS_TABLE_PATH}/uhal_gem_amc_ctp7_link${lid}.xml" />
EOF
                done
            cat <<EOF >> /etc/cmsgemos/connections.xml

EOF
        done
        cat <<EOF >> /etc/sysmgr/ipconfig.xml
        <Slot number="13">
            <Card type="BU AMC13">
                <FPGA id="0">13 255 255 0 0 192 168 ${crate} 14</FPGA> <!-- T2 -->
                <FPGA id="1">13 255 255 0 0 192 168 ${crate} 13</FPGA> <!-- T1 -->
            </Card>
        </Slot>
EOF
        cat <<EOF >> /etc/hosts
EOF
        cat <<EOF >> /etc/sysmgr/ipconfig.xml
    </Crate>
EOF
        cat <<EOF >> /etc/cmsgemos/connections.xml

EOF
    done
    cat <<EOF >> /etc/sysmgr/ipconfig.xml
</IVTable>
EOF

    cat <<EOF >> /etc/cmsgemos/connections.xml
</connections>
EOF

    ## Set up host machine to act as time server
    if [ -e /etc/xinetd.d/time-stream ]
    then
        line=$(sed -n '/disable/=' /etc/xinetd.d/time-stream)
        cp /etc/xinetd.d/{time-stream,time-stream.bak}
        sed -i "$line s|yes|no|g" /etc/xinetd.d/time-stream
        ## restart xinetd
        # new_service xinetd on
    fi

    ## Set up rsyslog
    cat <<EOF > /etc/logrotate.d/ctp7
$ModLoad imudp

$UDPServerAddress 192.168.0.180
$UDPServerRun 514

$template RemoteLog,"/var/log/remote/%HOSTNAME%/messages.log"
:fromhost-ip, startswith, "192.168." ?RemoteLog
EOF

    ## Configure logrotate to rotate ctp7 logs
    cat <<\EOF > /etc/logrotate.d/ctp7
/var/log/remote/*/messages.log {
        sharedscripts
        missingok
        create 0644 root wheel
        compress
        dateext
        weekly
        rotate 4
        lastaction
                /bin/kill -HUP `cat /var/run/syslogd.pid 2> /dev/null` 2> /dev/null || true
                /bin/kill -HUP `cat /var/run/rsyslogd.pid 2> /dev/null` 2> /dev/null || true
        endscript
}

EOF
    ## Restart rsyslog
    # new_service rsyslog on

    ## Set up dnsmasq
    if [ -e /etc/xinetd.d/time-stream ]
    then
        cp /etc/{dnsmasq.conf,dnsmasq.conf.bak}
        line=$(sed -n '/log-queries/=' /etc/dnsmasq.conf)
        sed -i "$line s|#log-queries|log-queries|g" /etc/dnsmasq.conf
        line=$(sed -n '/log-dhcp/=' /etc/dnsmasq.conf)
        sed -i "$line s|#log-dhcp|log-dhcp|g" /etc/dnsmasq.conf
    fi

    # Create configuration file /etc/dnsmasq.d/ctp7, needs interface name
    setup_network
    cat <<EOF >> /etc/dnsmasq.d/ctp7
bind-interfaces
dhcp-range=192.168.249.1,192.168.249.254,1h
dhcp-option=option:ntp-server,0.0.0.0 # autotranslated to server ip
domain=utca
local=/utca/

dhcp-host=00:04:a3:a4:a1:8e,192.168.250.1  # falcon1
dhcp-host=00:04:a3:62:9f:d9,192.168.250.2  # falcon2
dhcp-host=00:04:a3:a4:dc:c1,192.168.250.3  # raven1
dhcp-host=00:04:a3:a4:bc:54,192.168.250.4  # raven2
dhcp-host=00:04:a3:a4:60:b2,192.168.250.5  # raven3
dhcp-host=00:04:a3:a4:d7:40,192.168.250.6  # raven4
dhcp-host=00:04:a3:a4:67:56,192.168.250.7  # raven5
dhcp-host=00:04:a3:a4:bf:e7,192.168.250.8  # raven6
dhcp-host=00:1e:c0:85:ef:ad,192.168.250.9  # eagle1
dhcp-host=00:1e:c0:85:ef:96,192.168.250.10 # eagle2
dhcp-host=00:1e:c0:86:2a:b8,192.168.250.11 # eagle3
dhcp-host=00:1e:c0:85:a2:36,192.168.250.12 # eagle4
dhcp-host=00:1e:c0:85:73:6c,192.168.250.13 # eagle5
dhcp-host=00:1e:c0:85:b1:99,192.168.250.14 # eagle6
dhcp-host=00:1e:c0:85:c3:16,192.168.250.15 # eagle7
dhcp-host=00:1e:c0:85:ad:48,192.168.250.16 # eagle8
dhcp-host=00:1e:c0:86:34:7f,192.168.250.17 # eagle9
dhcp-host=00:1e:c0:85:96:42,192.168.250.18 # eagle10
dhcp-host=00:1e:c0:85:de:4b,192.168.250.19 # eagle11
dhcp-host=00:1e:c0:86:35:fb,192.168.250.20 # eagle12
dhcp-host=00:1e:c0:86:22:7a,192.168.250.21 # eagle13
dhcp-host=00:1e:c0:85:94:66,192.168.250.22 # eagle14
dhcp-host=00:1e:c0:85:af:b3,192.168.250.23 # eagle15
dhcp-host=00:1e:c0:85:88:79,192.168.250.24 # eagle16
dhcp-host=00:1e:c0:85:af:a2,192.168.250.25 # eagle17
dhcp-host=00:1e:c0:86:0c:91,192.168.250.26 # eagle18
dhcp-host=00:1e:c0:86:16:d1,192.168.250.27 # eagle19
dhcp-host=00:1e:c0:86:36:97,192.168.250.28 # eagle20
dhcp-host=00:1e:c0:86:0c:30,192.168.250.29 # eagle21
dhcp-host=00:1e:c0:86:14:9a,192.168.250.30 # eagle22
dhcp-host=00:1e:c0:85:f9:ea,192.168.250.31 # eagle23
dhcp-host=00:1e:c0:85:73:9d,192.168.250.32 # eagle24
dhcp-host=00:1e:c0:85:bf:5a,192.168.250.33 # eagle25
dhcp-host=00:1e:c0:85:ec:45,192.168.250.34 # eagle26
dhcp-host=00:1e:c0:85:bd:62,192.168.250.35 # eagle27
dhcp-host=00:1e:c0:86:16:f9,192.168.250.36 # eagle28
dhcp-host=00:1e:c0:86:26:17,192.168.250.37 # eagle29
dhcp-host=00:1e:c0:85:96:06,192.168.250.38 # eagle30
dhcp-host=00:1e:c0:85:96:14,192.168.250.39 # eagle31
dhcp-host=00:1e:c0:86:2b:3f,192.168.250.40 # eagle32
dhcp-host=00:1e:c0:85:7d:24,192.168.250.41 # eagle33
dhcp-host=00:1e:c0:86:2d:9d,192.168.250.42 # eagle34
dhcp-host=00:1e:c0:85:c2:a7,192.168.250.43 # eagle35
dhcp-host=00:1e:c0:85:a1:70,192.168.250.44 # eagle36
dhcp-host=00:1e:c0:85:73:cb,192.168.250.45 # eagle37
dhcp-host=00:1e:c0:85:7f:bd,192.168.250.46 # eagle38
dhcp-host=00:1e:c0:86:17:92,192.168.250.47 # eagle39
dhcp-host=00:1e:c0:85:ec:3a,192.168.250.48 # eagle40
dhcp-host=00:1e:c0:85:b0:78,192.168.250.49 # eagle41
dhcp-host=00:1e:c0:85:74:01,192.168.250.50 # eagle42
dhcp-host=00:1e:c0:85:b2:60,192.168.250.51 # eagle43
dhcp-host=00:1e:c0:85:a4:54,192.168.250.52 # eagle44
dhcp-host=00:1e:c0:85:d0:da,192.168.250.53 # eagle45
dhcp-host=00:1e:c0:86:2b:6b,192.168.250.54 # eagle46
dhcp-host=00:1e:c0:85:73:7b,192.168.250.55 # eagle47
dhcp-host=00:1e:c0:85:9f:30,192.168.250.56 # eagle48
dhcp-host=00:1e:c0:85:96:fc,192.168.250.57 # eagle49
dhcp-host=00:1e:c0:85:ee:b1,192.168.250.58 # eagle50
dhcp-host=00:1e:c0:86:0c:7d,192.168.250.59 # eagle51
dhcp-host=00:1e:c0:85:86:5c,192.168.250.60 # eagle52
dhcp-host=00:1e:c0:85:bc:ca,192.168.250.61 # eagle53
dhcp-host=00:1e:c0:85:73:b3,192.168.250.62 # eagle54
dhcp-host=00:1e:c0:85:97:3a,192.168.250.63 # eagle55
dhcp-host=00:1e:c0:85:bc:8a,192.168.250.64 # eagle56
dhcp-host=00:1e:c0:86:34:00,192.168.250.65 # eagle57
dhcp-host=00:1e:c0:86:0c:05,192.168.250.66 # eagle58
dhcp-host=00:1e:c0:85:bc:f4,192.168.250.67 # eagle59
dhcp-host=00:1e:c0:86:2b:5f,192.168.250.68 # eagle60
dhcp-host=00:1e:c0:85:f9:ed,192.168.250.69 # eagle61
dhcp-host=00:1e:c0:85:e1:b4,192.168.250.70 # eagle62
dhcp-host=00:1e:c0:85:72:c9,192.168.250.71 # eagle63
dhcp-host=00:1e:c0:86:2a:7e,192.168.250.72 # eagle64
dhcp-host=00:1e:c0:85:ca:01,192.168.250.73 # eagle65
EOF
    ## Restart dnsmasq
    # new_service dnsmasq on

    ## Update /etc/hosts with CTP7-related dns (bird) names
    cat <<EOF >> /etc/hosts

# falcons
EOF
    for bird in {1..2}
    do
        cat <<EOF >> /etc/hosts
192.168.250.$bird falcon$bird falcon$bird.utca
EOF
    done

    cat <<EOF >> /etc/hosts

# ravens
EOF
    for bird in {1..6}
    do
        cat <<EOF >> /etc/hosts
192.168.250.$((2+bird)) raven$bird raven$bird.utca
EOF
    done

    cat <<EOF >> /etc/hosts

# eagles
EOF
    for bird in {1..65}
    do
        cat <<EOF >> /etc/hosts
192.168.250.$((9+bird)) eagle$bird eagle$bird.utca
EOF
    done
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
    prompt_confirm "Create user daqbuild?"
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
