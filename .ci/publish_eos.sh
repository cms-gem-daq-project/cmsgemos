#!/bin/sh -eu

source $(dirname $0)/utils.sh

trap cleanup SIGHUP SIGINT SIGQUIT SIGABRT SIGTERM

## Structure of ARTIFACTS_DIR
# └── ${ARTIFACTS_DIR}
#     ├── api
#     └── repos
#         ├── ${CI_PROJECT_NAME}_${REL_VERSION}_{ARCH}.repo
#         ├── SRPMS ## should be arch independent...
#         └── ${ARCH}/{'','base','testing'}
#             ├── tarballs
#             ├── RPMS
#             ├── DEBUGRPMS
#             └── SRPMS ## should be arch independent...

#### Structure of EOS website
## from https://gist.github.com/jsturdy/a9cbc64c947364a01057a1d40e228452
# ├── index.html # landing page
# ├── guides ## user/developer guides and other synthesied information, if versioning of this is foreseen, need to address
# │   ├── user
# │   │   └── index.html
# │   └── developers
# │       └── index.html
# ├── docs
# │   ├── index.html
# │   ├── ${CI_PROJECT_NAME} ## one for each repo, this would be he entry point to the versioned
# │   │   ├── index.html
# │   │   ├── unstable ## filled from `develop` or symlink to the above `api/latest`
# │   │   ├── latest ## filled from last tagged build, or as a symlink to releases/M.M/api/latest
# │   │   ├── api (${EOS_DOC_NAME})
# │   │   │   ├── unstable ## filled from `develop`, or any non tagged CI job
# │   │   │   ├── latest ## filled from last tagged build or symlink to the very latest api version build?
# │   │   │   ├── ${PKG_BASE_TAG}.Z+2
# │   │   │   ├── ${PKG_BASE_TAG}.Z+1
# │   │   │   └── ${PKG_BASE_TAG}.Z
# │   │   └── styles/scripts/css/js  ## styles that we will not change
# │   └── styles/scripts/css/js  ## styles that we will not change, maybe even package agnostic?
# ├── ci-builds
# │   └── ${CI_PROJECT_NAME} ## one for each repo, this would be he entry point to the versioned
# │       └── ${CI_MERGE_REQUEST_ID}
# │           └── compat_report
# └── sw
#     ├── index.html
#     │   ### one yum repo per git repo (easiest)
#     ├── RPM-GPG-KEY-cmsgemdaq
#     ├── ${CI_PROJECT_NAME}_${REL_VERSION}_{ARCH}.repo (or in the ${CI_PROJECT_NAME} namespace?
#     ├── ${CI_PROJECT_NAME}
#     │   ├── ${CI_PROJECT_NAME}_${REL_VERSION}_{ARCH}.repo
#     │   ├── unstable (${EOS_UNSTABLE_DIR}) ## all builds not on a release branch
#     │   │   └── repos (${EOS_REPO_NAME})
#     │   │       ├── SRPMS ## should be arch independent
#     │   │       │   └── repodata
#     │   │       └── ${ARCH} ## (slc6_x86_64/centos7_x86_64/centos8_x86_64/arm/peta/noarch/pythonX.Y/gccXYZ/clangXYZ?)
#     │   │           ├── tarballs
#     │   │           ├── RPMS  ## keep all versions, manual cleanup only?
#     │   │           │   └── repodata
#     │   │           └── DEBUGRPMS
#     │   │               └── repodata
#     │   │   ##### Option 1
#     │   ├── releases (${EOS_RELEASE_DIR})
#     │   │   └── repos (${EOS_REPO_NAME})
#     │   │       └── ${REL_VERSION} ## Maj.Min
#     │   │           ├── base
#     │   │           │   ├── SRPMS ## should be arch independent
#     │   │           │   │   └── repodata
#     │   │           │   └── ${ARCH} ## (slc6_x86_64/centos7_x86_64/centos8_x86_64/arm/peta/noarch/pythonX.Y/gccXYZ/clangXYZ?)
#     │   │           │       ├── tarballs
#     │   │           │       ├── RPMS
#     │   │           │       │   └── repodata
#     │   │           │       └── DEBUGRPMS
#     │   │           │           └── repodata
#     │   │           └── testing ## all untagged builds along a given release tree
#     │   │               ├── SRPMS ## should be arch independent
#     │   │               │   └── repodata
#     │   │               └── ${ARCH} ## (slc6_x86_64/centos7_x86_64/centos8_x86_64/arm/peta/noarch/pythonX.Y/gccXYZ/clangXYZ?)
#     │   │                   ├── tarballs
#     │   │                   ├── RPMS
#     │   │                   │   └── repodata
#     │   │                   └── DEBUGRPMS
#     │   │                       └── repodata
#     │   │   ##### Option 2
#     │   └── releases (${EOS_RELEASE_DIR})
#     │       └── ${REL_VERSION} ## Maj.Min
#     │           └── repos (${EOS_REPO_NAME})
#     │               ├── base
#     │               │   └── ${ARCH} ## (slc6_x86_64/centos7_x86_64/centos8_x86_64/arm/peta/noarch/pythonX.Y/gccXYZ/clangXYZ?)
#     │               │       ├── tarballs
#     │               │       ├── RPMS
#     │               │       │   └── repodata
#     │               │       └── DEBUGRPMS
#     │               │           └── repodata
#     │               └── testing ## all untagged builds along a given release tree
#     │                   └── ${ARCH} ## (slc6_x86_64/centos7_x86_64/centos8_x86_64/arm/peta/noarch/pythonX.Y/gccXYZ/clangXYZ?)
#     │                       ├── tarballs
#     │                       ├── RPMS
#     │                       │   └── repodata
#     │                       └── DEBUGRPMS
#     │                           └── repodata
#     └── extras ## holds all extra/external packages we build for compatibility
#         ├── SRPMS ## provide source RPMs for extras?
#         │   └── repodata
#         └── ${ARCH} ## (slc6_x86_64/centos7_x86_64/centos8_x86_64/arm/peta/noarch/pythonX.Y/gccXYZ/clangXYZ?)
#             ├── RPMS
#             │   └── repodata
#             └── DEBUGRPMS
#                 └── repodata
#     │ ### gemos releases

# RELEASE_DIR=${EOS_RELEASE_DIR}/${REL_VERSION}

# BASE_DIR=${PWD}

##### RPMs

## map source dir to output dir
echo "Figuring out appropriate tag"
## choose the correct of: base|prerel|testing|unstable
if [[ ${BRANCH_NAME} =~ ${relre} ]]
then
    DEPLOY_DIR=${EOS_RELEASE_DIR}/${REL_VERSION}
    if [[ ${BUILD_VER}${BUILD_TAG} =~ $vre-final$ ]]
    then
        ## base for tag vX.Y.Z
        TAG_REPO_TYPE=base/base
    elif [[ ${BUILD_TAG} =~ (alpha|beta|pre|rc) ]]
    then
        ## prerel for tag vX.Y.Z-(alpha|beta|pre|rc)\d+-git<hash> or untagged on release/*
        TAG_REPO_TYPE=prerel/prerel
    else
        ## testing for untagged merges onto release/*
        TAG_REPO_TYPE=testing/testing
    fi
elif [[ ${BUILD_TAG} =~ (-final$) ]] && [[ ${BRANCH_NAME} =~ (^master$|${relre}) ]]
then
    ## base for tag vX.Y.Z
    DEPLOY_DIR=${EOS_RELEASE_DIR}/${REL_VERSION}
    TAG_REPO_TYPE=base/base
elif [[ ${BUILD_TAG} =~ (dev) ]] || [[ ${BRANCH_NAME} =~ (^develop$) ]] 
then
    ## unstable for dev tag or 'develop' branch
    DEPLOY_DIR=${EOS_UNSTABLE_DIR}
    TAG_REPO_TYPE=/unstable
else
    ## unstable for unknown or untagged
    DEPLOY_DIR=${EOS_UNSTABLE_DIR}
    TAG_REPO_TYPE=/unstable
fi

# CI_DOCS_DIR=${DEPLOY_DIR}/${EOS_DOC_NAME}
CI_DOCS_DIR=${EOS_DOC_NAME}
CI_REPO_DIR=${EOS_REPO_NAME}/${DEPLOY_DIR}

EOS_REPO_PATH=${EOS_BASE_WEB_DIR}/${EOS_SW_DIR}/${CI_REPO_DIR}/${TAG_REPO_TYPE%%/*}

EOS_SW_PATH=${EOS_BASE_WEB_DIR}/${EOS_SW_DIR}
EOS_DOCS_PATH=${EOS_BASE_WEB_DIR}/${EOS_DOCS_DIR%%/${CI_PROJECT_NAME}}

echo TAG_REPO_TYPE is ${TAG_REPO_TYPE}
echo DEPLOY_DIR is ${DEPLOY_DIR}
echo CI_DOCS_DIR is ${CI_DOCS_DIR}
echo CI_REPO_DIR is ${CI_REPO_DIR}
echo EOS_REPO_PATH is ${EOS_REPO_PATH}
echo EOS_SW_PATH is ${EOS_SW_PATH}
echo EOS_DOCS_PATH is ${EOS_DOCS_PATH}

echo "Tag ${BUILD_VER}${BUILD_TAG} determined to be ${TAG_REPO_TYPE#*/}"

signRPMs
echo Signed RPMs

signTarballs
echo Signed tarballs

# KRB_CACHE=$(klist |egrep FILE| awk '{split($0,a, " "); print a[3];}')
authenticateKRB

publishRepository

##### Documentation, only done for final tags?
echo "Publishing documentation for ${BUILD_TAG}"
pushd ${ARTIFACTS_DIR}/api

if [ -n "${BUILD_TAG}" ]
then
    ## we are on a release X.Y version
    if [[ ${BUILD_TAG} =~ (dev) ]] || [[ ${BRANCH_NAME} =~ (^develop$) ]] 
    then
        publishDocs "unstable"
    elif [[ ${BUILD_TAG} =~ (-final$) ]] && [[ ${BRANCH_NAME} =~ (^master$|${relre}) ]]
    then
        publishDocs "base"
    elif [[ ${BUILD_TAG} =~ (alpha|beta|pre|rc) ]] && [[ ${BRANCH_NAME} =~ ${relre} ]]
    then
        publishDocs "testing"
    elif [[ ${BRANCH_NAME} =~ ${relre} ]]
    then
        publishDocs "testing"
    else
        publishDocs "unstable"
    fi
fi

popd

unauthenticateKRB

# if [ -n ${KRB_CACHE} ] && [ -f ${KRB_CACHE##'FILE:'} ]
# then
#     export KRB5CCNAME=${KRB_CACHE}
#     kinit -R
# fi
