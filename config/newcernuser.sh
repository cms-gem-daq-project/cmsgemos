#!/bin/bash

/usr/sbin/addusercern $1
/usr/sbin/usermod -a -G gemuser $1
mkdir -Z system_u:object_r:user_home_dir_t:s0 /home/$1
chown $1:zh -R /home/$1
usermod -d /home/$1 $1
mkdir -p -Z system_u:object_r:usr_t:s0 /data/xdaq/$1/gemdaq

mkdir -p -Z system_u:object_r:nfs_t:s0 /data/bigdisk/users/$1
chown -R $1:zh /data/bigdisk/users/$1

ln -sn /opt/xdaq/htdocs/* /data/xdaq/$1/

chown $1:zh -R /data/xdaq/$1

chown root:root -R /opt/xdaq/htdocs
