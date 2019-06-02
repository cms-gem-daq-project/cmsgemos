#!/bin/sh -ie

REPO_NAME=$1
ARTIFACTS_DIR=$2

### fail if KRB_USERNAME not set
# if ! [ -n "${KRB_USERNAME}" ]

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

# ## normally taken from the gitlab CI job
# EOS_BASE_WEB_DIR=/eos/project/c/cmsgemdaq/www
# EOS_COMMON_WEB_DIR=cmsgemdaq
# EOS_SITE_WEB_DIR=${EOS_BASE_WEB_DIR}/${REPO_NAME}
# EOS_SW_DIR=${EOS_BASE_WEB_DIR}/sw/${REPO_NAME}
# EOS_DOC_DIR=${EOS_BASE_WEB_DIR}/docs/${REPO_NAME}
# EOS_LATEST_DIR=${EOS_SW_DIR}/unstable
# EOS_RELEASE_DIR=${EOS_SW_DIR}/releases
# EOS_DOC_DIR_NAME=api
# EOS_REPO_DIR_NAME=repos

BUILD_VER=$(${BUILD_HOME}/${REPO_NAME}/config/build/tag2rel.sh | \
                   awk '{split($$0,a," "); print a[5];}' | \
                   awk '{split($$0,b,":"); print b[2];}')
BUILD_TAG=$(${BUILD_HOME}/${REPO_NAME}/config/build/tag2rel.sh | \
                   awk '{split($$0,a," "); print a[8];}' | \
                   awk '{split($$0,b,":"); print b[2];}')

REL_VERSION=${BUILD_VER%.*}

## from https://gist.github.com/jsturdy/a9cbc64c947364a01057a1d40e228452
# ├── index.html
# ├── sw
# │   ├── ${REPO_NAME}
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
#     └── ${REPO_NAME} ## one for each repo, this would be he entry point to the versioned
#         ├── index.html
#         ├── unstable ## filled from `develop` or symlink to the above `api/latest`
#         ├── latest ## filled from last tagged build, or as a symlink to releases/M.M/api/latest
#         └── styles/scripts/css/js  ## styles that we will not change

RELEASE_DIR=${EOS_RELEASE_DIR}/${REL_VERSION}

##### RPMs
# basic version unit is vX.Y.Z
vre='^v?(\.)?([0-9]+)\.([0-9]+)\.([0-9]+)'
gre='(git[0-9a-fA-F]{6,8})'

## map source dir to output dir
echo "Figuring out appropriate tag"
## choose the correct of: base|testing|latest/unstable
if [[ ${BUILD_TAG} =~ (dev) ]] || [[ ${CI_COMMIT_REF_NAME} =~ (develop) ]] 
then
    ## unstable for dev tag or 'develop' branch
    DEPLOY_DIR=${EOS_LATEST_DIR}
    DOCS_DIR=${DEPLOY_DIR}/${EOS_DOC_DIR_NAME}
    TAG_REPO_TYPE=unstable
    REPO_DIR=${DEPLOY_DIR}/${EOS_REPO_DIR_NAME}
elif [[ ${BUILD_TAG} =~ (alpha|beta|pre|rc) ]]
then
    ## testing for tag vx.y.z-(alpha|beta|pre|rc)\d+-git<hash>
    DEPLOY_DIR=${EOS_RELEASE_DIR}/${REL_VERSION}
    DOCS_DIR=${DEPLOY_DIR}/${EOS_DOC_DIR_NAME}
    TAG_REPO_TYPE=testing
    REPO_DIR=${DEPLOY_DIR}/${EOS_REPO_DIR_NAME}
elif [[ ${BUILD_VER}${BUILD_TAG} =~ $vre$ ]]
then
    ## base for tag vx.y.z
    DEPLOY_DIR=${EOS_RELEASE_DIR}/${REL_VERSION}
    DOCS_DIR=${DEPLOY_DIR}/${EOS_DOC_DIR_NAME}
    TAG_REPO_TYPE=base
    REPO_DIR=${DEPLOY_DIR}/${EOS_REPO_DIR_NAME}
else
    ## unstable for unknown or untagged
    DEPLOY_DIR=${EOS_LATEST_DIR}
    DOCS_DIR=${DEPLOY_DIR}/${EOS_DOC_DIR_NAME}
    TAG_REPO_TYPE=unstable
    REPO_DIR=${DEPLOY_DIR}/${EOS_REPO_DIR_NAME}
fi

echo mkdir -p ${DOCS_DIR}
echo mkdir -p ${REPO_DIR}/{tarballs,RPMS,DEBUGRPMS,SRPMS}
echo "Done creating repository structure"

echo "Tag ${BUILD_VER}${BUILD_TAG} determined to be ${TAG_REPO_TYPE}"

CI_REPO_DIR=${REPO_DIR}

cd ${ARTIFACTS_DIR}/repos
echo rsync --relative . ${KRB_USERNAME}@lxplus:${CI_REPO_DIR}
## unstable example
## rsync --relative . ${KRB_USERNAME}@lxplus:${EOS_BASE_WEB_DIR}/sw/cmsgemos/unstable/repos/
## stable example
## rsync --relative . ${KRB_USERNAME}@lxplus:${EOS_BASE_WEB_DIR}/sw/cmsgemos/releases/repos/1.2/

## update the repositories
echo "Updating the repository"
echo createrepo --update ${CI_REPO_DIR}/RPMS
echo createrepo --update ${CI_REPO_DIR}/DEBUGRPMS
echo createrepo --update ${CI_REPO_DIR}/SRPMS

## update the groups?

##### Documentation, only done for final tags?
echo "Publishing documentation"
CI_DOCS_DIR=${DOCS_DIR}
## if unstable, rsync docs with --delete
cd ${ARTIFACTS_DIR}/api
if [ -n "${BUILD_TAG}" ]
then
    ## stable example
    ## rsync --relative . ${KRB_USERNAME}@lxplus:${EOS_BASE_WEB_DIR}/sw/cmsgemos/releases/api/1.2/${BUILD_VER}
    TAG_DOC_DIR=${CI_DOCS_DIR}/${BUILD_VER}
    LATEST_DOC_DIR=${RELEASE_DIR}/api/latest
    LATEST_DOC_DIR=${EOS_DOC_DIR}/latest
    echo rsync ${ARTIFACTS_DIR}/doc/html/ --delete ${KRB_USERNAME}@lxplus:${TAG_DOC_DIR}
    echo "ln -sf ${TAG_DOC_DIR} ${LATEST_DOC_DIR}"
    ## update the index file?
    ## or have the landing page running some scripts querying the git tags, populating some JSON, and dynamically adapting the content
else
    ## unstable example
    ## rsync --relative --delete . ${KRB_USERNAME}@lxplus:${EOS_BASE_WEB_DIR}/sw/cmsgemos/unstable/api/
    TAG_DOC_DIR=${CI_DOCS_DIR}
    LATEST_DOC_DIR=${EOS_DOC_DIR}/unstable
    echo rsync ${ARTIFACTS_DIR}/doc/html/ --delete ${KRB_USERNAME}@lxplus:${TAG_DOC_DIR}
    echo "ln -sf ${TAG_DOC_DIR} ${LATEST_DOC_DIR}"
    ## update the index file?
    ## or have the landing page running some scripts querying the git tags, populating some JSON, and dynamically adapting the content
fi

