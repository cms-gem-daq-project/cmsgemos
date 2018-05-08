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

zones=(
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

idx=0
for zone in "${zones[@]}"
do
    for dir in "${dirs[@]}"
    do
        cp gembase/$dir/Makefile $zone/$dir/
    done
    modifier="${modifiers[$idx]}"
    perl -pi -e "s|for generic GEM setup|${modifier}|g" ./*/Makefile

    cp ../setup/etc/addresstables/linkuhaltables.sh $zone/config/scripts/
    cp ../setup/etc/profile.d/gemdaqenv.sh $zone/config/scripts/
    cp $TCDS_CONFIG_PATH/${zone}/*.txt $zone/config/tcds/

    let "idx++"
done
