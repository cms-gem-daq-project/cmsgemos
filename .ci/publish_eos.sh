#!/bin/sh -eu

BUILD_VER=$(${BUILD_HOME}/${CI_PROJECT_NAME}/config/build/tag2rel.sh | \
                   awk '{split($$0,a," "); print a[5];}' | \
                   awk '{split($$0,b,":"); print b[2];}')
BUILD_TAG=$(${BUILD_HOME}/${CI_PROJECT_NAME}/config/build/tag2rel.sh | \
                   awk '{split($$0,a," "); print a[8];}' | \
                   awk '{split($$0,b,":"); print b[2];}')

REL_VERSION=${BUILD_VER%.*}

## Structure of ARTIFACTS_DIR
# └── artifacts
#     ├── api
#     └── repos
#         ├── SRPMS ## should be arch independent...
#         └── ${ARCH}/{'','base','testing'}
#             ├── tarballs
#             ├── RPMS
#             ├── DEBUGRPMS
#             └── SRPMS ## should be arch independent...

## from https://gist.github.com/jsturdy/a9cbc64c947364a01057a1d40e228452
# ├── index.html
# ├── sw
# │   ├── ${CI_PROJECT_NAME}
# │   │   ├── latest/unstable ## all builds not on a release branch?
# │   │   │   ├── api
# │   │   │   │   └── latest  ## overwrite with latest each build?
# │   │   │   └── repos
# │   │   │       ├── SRPMS ## should be arch independent
# │   │   │       │   └── repodata
# │   │   │       └── ${ARCH} ## (slc6_x86_64/centos7_x86_64/centos8_x86_64/arm/peta/noarch/pythonX.Y/gccXYZ/clangXYZ?)
# │   │   │           ├── tarballs
# │   │   │           ├── RPMS  ## keep all versions, manual cleanup only?
# │   │   │           │   └── repodata
# │   │   │           └── DEBUGRPMS
# │   │   │               └── repodata
# │   │   └── releases
# │   │       ├── api
# │   │       │   └── ${REL_VERSION} ## Maj.Min, might even not have this directory?
# │   │       │       ├── latest -> ${REL_VERSION}.Z+2
# │   │       │       ├── ${REL_VERSION}.Z+2
# │   │       │       ├── ${REL_VERSION}.Z+1
# │   │       │       └── ${REL_VERSION}.Z
# │   │       └── repos
# │   │           └── ${REL_VERSION} ## Maj.Min
# │   │               ├── base
# │   │               │   ├── SRPMS ## should be arch independent
# │   │               │   │   └── repodata
# │   │               │   └── ${ARCH} ## (slc6_x86_64/centos7_x86_64/centos8_x86_64/arm/peta/noarch/pythonX.Y/gccXYZ/clangXYZ?)
# │   │               │       ├── tarballs
# │   │               │       ├── RPMS
# │   │               │       │   └── repodata
# │   │               │       └── DEBUGRPMS
# │   │               │           └── repodata
# │   │               └── testing ## all untagged builds along a given release tree
# │   │                   ├── SRPMS ## should be arch independent
# │   │                   │   └── repodata
# │   │                   └── ${ARCH} ## (slc6_x86_64/centos7_x86_64/centos8_x86_64/arm/peta/noarch/pythonX.Y/gccXYZ/clangXYZ?)
# │   │                       ├── tarballs
# │   │                       ├── RPMS
# │   │                       │   └── repodata
# │   │                       └── DEBUGRPMS
# │   │                           └── repodata
############### BEGIN OR
# │   │   └── releases
# │   │       └── ${REL_VERSION} ## Maj.Min
# │   │           ├── api
# │   │           │   ├── latest -> ${REL_VERSION}.Z+2
# │   │           │   ├── ${REL_VERSION}.Z+2
# │   │           │   ├── ${REL_VERSION}.Z+1
# │   │           │   └── ${REL_VERSION}.Z
# │   │           └── repos
# │   │               ├── base
# │   │               │   └── ${ARCH} ## (slc6_x86_64/centos7_x86_64/centos8_x86_64/arm/peta/noarch/pythonX.Y/gccXYZ/clangXYZ?)
# │   │               │       ├── tarballs
# │   │               │       ├── RPMS
# │   │               │       │   └── repodata
# │   │               │       └── DEBUGRPMS
# │   │               │           └── repodata
# │   │               └── testing ## all untagged builds along a given release tree
# │   │                   └── ${ARCH} ## (slc6_x86_64/centos7_x86_64/centos8_x86_64/arm/peta/noarch/pythonX.Y/gccXYZ/clangXYZ?)
# │   │                       ├── tarballs
# │   │                       ├── RPMS
# │   │                       │   └── repodata
# │   │                       └── DEBUGRPMS
# │   │                           └── repodata
# │   └── extras ## holds all extra/external packages we build for compatibility
# │       ├── SRPMS ## provide source RPMs for extras?
# │       │   └── repodata
# |       └── ${ARCH} ## (slc6_x86_64/centos7_x86_64/centos8_x86_64/arm/peta/noarch/pythonX.Y/gccXYZ/clangXYZ?)
# │           ├── RPMS
# │           │   └── repodata
# │           └── DEBUGRPMS
# │               └── repodata
# ├── guides ## user/developer guides and other synthesied information, if versioning of this is foreseen, need to address
# │   ├── user
# |   │   └── index.html
# │   └── developers
# |       └── index.html
# └── docs
#     ├── index.html
#     └── ${CI_PROJECT_NAME} ## one for each repo, this would be he entry point to the versioned
#         ├── index.html
#         ├── unstable ## filled from `develop` or symlink to the above `api/latest`
#         ├── latest ## filled from last tagged build, or as a symlink to releases/M.M/api/latest
#         └── styles/scripts/css/js  ## styles that we will not change

RELEASE_DIR=${EOS_RELEASE_DIR}/${REL_VERSION}

BASE_DIR=${PWD}

##### RPMs
# basic version unit is vX.Y.Z
vre='^v?(\.)?([0-9]+)\.([0-9]+)\.([0-9]+)'
gre='(git[0-9a-fA-F]{6,8})'

## map source dir to output dir
echo "Figuring out appropriate tag"
## choose the correct of: base|testing|unstable
if [[ ${BUILD_TAG} =~ (dev) ]] || [[ ${CI_COMMIT_REF_NAME} =~ (develop) ]] 
then
    ## unstable for dev tag or 'develop' branch
    DEPLOY_DIR=${EOS_UNSTABLE_DIR}
    CI_DOCS_DIR=${DEPLOY_DIR}/${EOS_DOC_NAME}
    TAG_REPO_TYPE=unstable
    CI_REPO_DIR=${DEPLOY_DIR}/${EOS_REPO_NAME}
elif [[ ${BUILD_TAG} =~ (alpha|beta|pre|rc) ]]
then
    ## testing for tag vX.Y.Z-(alpha|beta|pre|rc)\d+-git<hash>
    DEPLOY_DIR=${EOS_RELEASE_DIR}/${REL_VERSION}
    CI_DOCS_DIR=${DEPLOY_DIR}/${EOS_DOC_NAME}
    TAG_REPO_TYPE=testing
    CI_REPO_DIR=${DEPLOY_DIR}/${EOS_REPO_NAME}
elif [[ ${BUILD_VER}${BUILD_TAG} =~ $vre$ ]]
then
    ## base for tag vX.Y.Z
    DEPLOY_DIR=${EOS_RELEASE_DIR}/${REL_VERSION}
    CI_DOCS_DIR=${DEPLOY_DIR}/${EOS_DOC_NAME}
    TAG_REPO_TYPE=base
    CI_REPO_DIR=${DEPLOY_DIR}/${EOS_REPO_NAME}
else
    ## unstable for unknown or untagged
    DEPLOY_DIR=${EOS_UNSTABLE_DIR}
    CI_DOCS_DIR=${DEPLOY_DIR}/${EOS_DOC_NAME}
    TAG_REPO_TYPE=unstable
    CI_REPO_DIR=${DEPLOY_DIR}/${EOS_REPO_NAME}
fi

echo "Tag ${BUILD_VER}${BUILD_TAG} determined to be ${TAG_REPO_TYPE}"

echo "FIXME: echo ${KRB_PASSWORD} | kinit -A -f ${KRB_USERNAME}@CERN.CH"

cd ${ARTIFACTS_DIR}/repos

echo FIXME: rsync --relative .  --rsync-path="mkdir -p ${CI_REPO_DIR} && rsync" ${KRB_USERNAME}@lxplus.cern.ch:${CI_REPO_DIR}

## unstable example
## rsync --relative . ${KRB_USERNAME}@lxplus.cern.ch:${EOS_BASE_WEB_DIR}/sw/cmsgemos/unstable/repos/
## stable example
## rsync --relative . ${KRB_USERNAME}@lxplus.cern.ch:${EOS_BASE_WEB_DIR}/sw/cmsgemos/releases/repos/1.2/

echo "Updating the repositories"
## Push the latest repo file
echo FIXME: rsync ${ARTIFACTS_DIR}/*.repo ${KRB_USERNAME}@lxplus.cern.ch:${CI_REPO_DIR}

## update the groups files?

## update the repositories
echo FIXME: ssh ${KRB_USERNAME}@lxplus.cern.ch find ${DEPLOY_DIR} -type d -name '*RPMS' -print0 -exec createrepo --update {} \;

##### Documentation, only done for final tags?
echo "Publishing documentation"
cd ${BASE_DIR}/${ARTIFACTS_DIR}/api

if [ -n "${BUILD_TAG}" ]
then
    TAG_DOC_DIR=${CI_DOCS_DIR}
    LATEST_DOC_DIR=${EOS_DOCS_DIR}/unstable
    echo FIXME: rsync ${ARTIFACTS_DIR}/doc/html/ --delete --rsync-path="mkdir -p ${TAG_DOC_DIR} && rsync" ${KRB_USERNAME}@lxplus.cern.ch:${TAG_DOC_DIR}
    echo "FIXME: ln -sf ${TAG_DOC_DIR} ${LATEST_DOC_DIR}"
    ## update the index file?
    ## or have the landing page running some scripts querying the git tags, populating some JSON, and dynamically adapting the content
else
    TAG_DOC_DIR=${CI_DOCS_DIR}/${BUILD_VER}
    LATEST_TAG_DOC_DIR=${RELEASE_DIR}/api/latest
    LATEST_DOC_DIR=${EOS_DOCS_DIR}/latest
    echo FIXME: rsync ${ARTIFACTS_DIR}/doc/html/ --delete --rsync-path="mkdir -p ${TAG_DOC_DIR} && rsync" ${KRB_USERNAME}@lxplus.cern.ch:${TAG_DOC_DIR}
    echo "FIXME: ln -sf ${TAG_DOC_DIR} ${LATEST_DOC_DIR}"
    ## update the index file?
    ## or have the landing page running some scripts querying the git tags, populating some JSON, and dynamically adapting the content
fi

echo "FIXME: echo kdestroy"
