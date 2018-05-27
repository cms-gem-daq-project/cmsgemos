#!/bin/sh

#############################################
# Modify common makefiles in gembase package
# Copy scritps from setup/etc
# Need specific connection file maybe
#############################################

# zone=$1
# modifier=$2
# if [ -z ${2+x} ]
# then
#     echo modifier unset
# else
#     echo Set modifier to  $modifier
#fi
#scriptdir=$()

cd $(dirname "$0")

zones=(
    gembase
    gem904
    gem904int
    gem904qc8
    gemp5
    gemp5dev
    gemp5pro
)
modifiers=(
    "for generic GEM setup"
    "for 904 .cms904 setup"
    "for 904 integration setup"
    "for 904 QC8 setup"
    "for GEM P5 .cms setup"
    "for GEM P5 development setup"
    "for GEM P5 production setup"
)
dirs=(
    addon
    client
    config
    extension
    service
    settings
    zone
)

basedir=$PWD

idx=0
for zone in "${zones[@]}"
do
    if ! [ ${zone} = "gembase" ]
    then
        for dir in "${dirs[@]}"
        do
            cp gembase/$dir/Makefile $zone/$dir/
            cp gembase/$dir/spec.template $zone/$dir/
            if [ $dir = "config" ]
            then
                mkdir -p $zone/$dir/etc/profile.d
            fi
        done
        modifier="${modifiers[$idx]}"
        perl -pi -e "s|for generic GEM setup|${modifier}|g" ./*/Makefile
    fi

    cd $zone/config
    make clean && make cleanrpm && make
    make rpm
    cd $basedir
    
    let "idx++"
done

mkdir -p $basedir/rpm
find . -iname '*.rpm' -print0 -exec mv {} $basedir/rpm \;
