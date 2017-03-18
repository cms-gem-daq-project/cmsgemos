#!/bin/bash

if [ -z ${1+x} ];
then
    echo "Error in: ${0} ${1} ${2}"
    echo "Please specify the directory where the address table files are located"
    echo "Usage: ./linkuhaltables.sh <dir to files> <identifier>"
    exit
fi

if [ -z ${2+x} ];
then
    echo "Error in: ${0} ${1} ${2}"
    echo "Please specify the string identifier"
    echo "E.g. to link to uhal_gem_amc_ctp7_jared_10032017_amc.xml, specify jared_10032017"
    echo "Usage: ./linkuhaltables.sh <dir to files> <identifier>"
    exit
fi

subdir=${1}
subname=${2}

echo ln -fs ${subdir}/uhal_gem_amc_ctp7_${subname}_AMC.xml uhal_gem_amc_ctp7_amc.xml
ln -fs ${subdir}/uhal_gem_amc_ctp7_${subname}_AMC.xml uhal_gem_amc_ctp7_amc.xml

for link in {0..11}
do
    linknum=$(printf "%02d" ${link})
    echo ln -fs ${subdir}/uhal_gem_amc_ctp7_${subname}_OH${linknum}.xml uhal_gem_amc_ctp7_link${linknum}.xml
    ln -fs ${subdir}/uhal_gem_amc_ctp7_${subname}_OH${linknum}.xml uhal_gem_amc_ctp7_link${linknum}.xml
done
