#!/bin/sh

echo ln -fs uhal_gem_amc_ctp7_jared_10032017_AMC.xml uhal_gem_amc_ctp7_amc.xml
ln -fs uhal_gem_amc_ctp7_jared_10032017_AMC.xml uhal_gem_amc_ctp7_amc.xml

for link in {0..11}
do
    echo ln -fs uhal_gem_amc_ctp7_jared_10032017_OH${link}.xml uhal_gem_amc_ctp7_link$(printf "%02d" ${link}).xml
    ln -fs uhal_gem_amc_ctp7_jared_10032017_OH${link}.xml uhal_gem_amc_ctp7_link$(printf "%02d" ${link}).xml
done
