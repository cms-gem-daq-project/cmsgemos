#!/bin/bash

version=$1
relver=1

if [[ "${version##*.}" =~ ^git ]]
then
    version=${version%.*}
fi

if [[ "${version##*.}" =~ ^(alpha|beta|dev|pre|rc) ]]
then
    echo "extracting release information"
    basever=${version%.*}
    tags=( $(git tag -l "*${basever}*") )
    ntags=$((${#tags[@]}+1))
    relver=0.$ntags.${version##*.}

    # Currently not safe about absolutely incrementing, i.e.,:
    #0.1.alpha0
    #0.2.alpha1
    #0.3.beta0
    #0.4.beta1
    #0.5.pre0
    #0.6.pre1
    #0.7.rc0
    #0.8.rc1
fi

echo $relver
