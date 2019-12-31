#!/bin/sh -xeu

## Only runs on merge requests, easy
# TARGET_BRANCH=${CI_MERGE_REQUEST_TARGET_BRANCH_NAME}
# SHARED_COMMIT=${CI_MERGE_REQUEST_TARGET_BRANCH_SHA}

## Run on every commit, manually, or on tags, more difficult
# SHARED_COMMIT=$(git merge-base ${TARGET_BRANCH} HEAD)

#makeABIDump ()
getABI () {
    set -x
    if ! [ -n "$1" ]
    then
        echo "ERROR: missing required branch argument"
        exit 1
    fi

    branch=$1

    if ! [[ $(git rev-parse --abbrev-ref HEAD) =~ ${branch} ]]
    then
        if ! git checkout ${branch} >& git.checkout.status
        then
            echo "Unable to change to branch ${branch} ($?)"
            cat git.checkout.status
            exit 1
        fi
    fi

}
#    TargetArch=${TargetArch} OPTFLAGS="-g -Og" make -j8

dumpABI() {
    branch=$(git rev-parse --abbrev-ref HEAD 2>/dev/null)
    githash=$(git rev-parse --short HEAD 2>/dev/null)
    outdir=${CI_PROJECT_DIR}/abi-checking/${branch}
    mkdir -p ${outdir}
    find . \( -type d -iname xcompile -o -type d -iname 'compat_reports' \) -prune \
         -o -iname '*.so' -print0 -exec abi-dumper {} -o ${outdir}/{}.dump -lver ${githash} \;
    set +x
}

checkABI()
{
    set -x
    if ! [ -n "$1" ]
    then
        echo "ERROR: missing required library argument"
        exit 1
    elif ! [ -n "$2" ]
    then
        echo "ERROR: missing required output directory argument"
        exit 1
    fi

    libobj=$1
    outdir=$2
    # oldsha=$3
    # newsha=$4

    libname=${libobj##*/}
    echo "Report for ${libobj}:" >> report.txt
    abi-compliance-checker -l ${libname} -old ${outdir}/old-abi/${libobj}.dump -new ${outdir}/new-abi/${libobj}.dump | tee -a report.txt

    ## Sample output format
    # Preparing, please wait ...
    # Comparing ABIs ...
    # Comparing APIs ...
    # Creating compatibility report ...
    # Binary compatibility: 99.5%
    # Source compatibility: 99.7%
    # Total binary compatibility problems: 2, warnings: 0
    # Total source compatibility problems: 2, warnings: 0
    set +x
}

############################################################################################################################
# usage() {
#     # branchname=$(git rev-parse --abbrev-ref HEAD 2>/dev/null)
# }

if [[ "$1" =~ ^getnew$ ]]
then
    git fetch -p --all
    # if ! git checkout -b new-abi origin/${CI_MERGE_REQUEST_SOURCE_BRANCH_SHA} >& /dev/null ]
    if ! git checkout -b new-abi origin/${CI_MERGE_REQUEST_SOURCE_BRANCH_NAME} >& git.checkout.status
    then
        echo "Unable to create branch new-abi ($?)"
        cat git.checkout.status
    fi

    getABI new-abi
elif [[ "$1" =~ ^getold$ ]]
then
    git fetch -p --all
    # if ! git checkout -b old-abi origin/${CI_MERGE_REQUEST_TARGET_BRANCH_SHA} >& /dev/null ]
    if ! git checkout -b old-abi origin/${CI_MERGE_REQUEST_TARGET_BRANCH_NAME} >& git.checkout.status
    then
        echo "Unable to create branch old-abi ($?)"
        cat git.checkout.status
    fi

    getABI old-abi
elif [[ "$1" =~ ^dumpnew$ ]]
then
    dumpABI new-abi
elif [[ "$1" =~ ^dumpold$ ]]
then
    dumpABI old-abi
elif [[ "$1" =~ ^compare$ ]]
then
    find . \( -type d -iname xcompile -o -type d -iname 'compat_reports' \) -prune \
         -o -iname '*.so' -print0 > libnames
    libraries=()
    while IFS=  read -r -d $'\0'
    do
        libraries+=("$REPLY")
    done < libnames

    for lib in ${libraries[@]}
    do
        checkABI ${lib} ${CI_PROJECT_DIR}/abi-checking
    done

    # mv report.txt compat_reports
    # tar cjf ${CI_MERGE_REQUEST_ID}.tbz2 compat_reports
else
    echo "Invalid option: $1"
    exit 1
fi
