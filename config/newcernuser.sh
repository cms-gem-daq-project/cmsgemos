#!/bin/sh

if [ ! "$(fgrep ${1} /etc/passwd)" ]
then
    /usr/sbin/addusercern $1
fi

if [ "$(fgrep gemuser /etc/group)" ] && [ ! "$(fgrep ${1} /etc/group|fgrep gemuser)" ]
then
    usermod -aG gemuser ${1}
else
    "Unable to find 'gemuser' group, have you created the standard users and groups on this machine yet?"
fi

if [ ! -d /home/$1 ]
then
    mkdir -Z system_u:object_r:user_home_dir_t:s0 /home/$1
    chown $1:zh -R /home/$1
    echo "To set the home directory for $1 to /home/$1, execute"
    echo "usermod -d /home/$1 $1"
fi

if [ ! -d /data/xdaq/$1 ]
then
    mkdir -p -Z system_u:object_r:usr_t:s0 /data/xdaq/$1/gemdaq
fi
unlink /data/xdaq/$1/*
ln -sn /opt/xdaq/htdocs/* /data/xdaq/$1/
chown $1:zh -R /data/xdaq/$1

chown root:root -R /opt/xdaq/htdocs

if [ ! -d /data/bigdisk/users/$1 ]
then
    mkdir -p -Z system_u:object_r:nfs_t:s0 /data/bigdisk/users/$1
    chown -R $1:zh /data/bigdisk/users/$1
fi
