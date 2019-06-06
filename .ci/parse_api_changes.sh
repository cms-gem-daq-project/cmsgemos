#!/bin/sh -eu

TARGET_BRANCH=${CI_MERGE_REQUEST_TARGET_BRANCH_NAME}
SHARED_COMMIT=${CI_MERGE_REQUEST_TARGET_BRANCH_SHA}
SHARED_COMMIT=$(git merge-base ${TARGET_BRANCH} HEAD)

## check out SHARED_COMMIT
## compile SHARED_COMMIT with -g -Og
## copy libs to oldlibs
## generate ABI dumps
##  abi-dumper oldlibs/<libname>.so -o <libname>.SHARED_COMMIT.dump
## check out HEAD
## compile HEADwith -g -Og
## copy libs to newlibs
## generate ABI dumps
##  abi-dumper newlibs/lib<name>.so -o <name>.HEAD.dump
## run analysis
##  abi-compliance-checker -l <name> -old <name>.SHARED_COMMIT.dump -new <name>.HEAD.dump
## parse report
##  compat_reports/gemhardware_devices/X_to_Y/compat_report.html 
## check proposed version, ensure appropriate bump (major, minor, patch)

libs=( gemutils \
           gembase \
           gemhardware \
           gemreadout \
           gemsupervisor \
           gempython \
           gemdaqmonitor \
           gemonlinedb \
           gemcalibration \
           # gemHwMonitor \
     )

makeABIDump () {
    if ! [ -n "${1}" ]
    then
        echo ERROR: missing required output directory argument
        exit 1
    elif ! [ -n "${2}" ]
    then
        echo ERROR: missing required hash argument
        exit 1
    fi

    outdir  = ${1}
    githash = ${2}

    mkdir outdir

    for lib in ${libs[@]}
    do
        abi-dumper ${outdir}/lib${lib}.so -o ${lib}.${githash}.dump -lver ${githash}
    done
}
