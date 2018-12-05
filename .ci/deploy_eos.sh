#!/bin/sh -ie

REPO_NAME=$1
ARTIFACT_DIR=$2
EOS_BASE_DIR=/eos/project/c/cmsgemdaq/www
EOS_PKG_DIR=${EOS_BASE_DIR}/${REPO_NAME}

BUILD_VER=$(${BUILD_HOME}/${REPO_NAME}/config/build/tag2rel.sh | \
                   awk '{split($$0,a," "); print a[5];}' | \
                   awk '{split($$0,b,":"); print b[2];}')
BUILD_TAG=$(${BUILD_HOME}/${REPO_NAME}/config/build/tag2rel.sh | \
                   awk '{split($$0,a," "); print a[8];}' | \
                   awk '{split($$0,b,":"); print b[2];}')

REL_VERSION=${BUILD_VER%.*}

# /eos/project/c/cmsgemdaq/www/
# └── ${REPO_NAME}
#     ├── sw
#     │   └── release
#     │       └── ${REL_VERSION}
#     │           ├── api
#     │           │    ├── latest
#     │           │    └── ${REL_VERSION}.Z+2
#     │           │    └── ${REL_VERSION}.Z+1
#     │           │    └── ${REL_VERSION}.Z
#     │           └── repos
#     │               ├── ${ARCH} (slc6_x86_64/centos7_x86_64/centos8_x86_64/arm/peta/noarch/pythonX.Y/gccXYZ/clangXYZ?)
#     │               │   ├── base
#     │               │   │   ├── RPMS
#     │               │   │   │   └── repodata
#     │               │   │   └── SRPMS
#     │               │   │       └── repodata
#     │               │   ├── updates
#     │               │   │   ├── RPMS
#     │               │   │   │   └── repodata
#     │               │   │   └── SRPMS
#     │               │   │       └── repodata
#     │               │   ├── testing
#     │               │   │   ├── RPMS
#     │               │   │   │   └── repodata
#     │               │   │   └── SRPMS
#     │               │   │       └── repodata
#     │               │   ├── devel
#     │               │   │   ├── RPMS
#     │               │   │   │   └── repodata
#     │               │   │   └── SRPMS
#     │               │   │       └── repodata
#     │               │   └── extras
#     │               │       ├── RPMS
#     │               │       │   └── repodata
#     │               │       └── SRPMS
#     │               │           └── repodata
RELEASE_DIR=${EOS_PKG_DIR}/sw/release/${REL_VERSION}

##### RPMs
ARCH='arch'
REPO_DIR=$RELEASE_DIR/repos/${ARCH}
if ! [ -d ${RELEASE_DIR} ]
then
    echo mkdir -p ${RELEASE_DIR}/{api,repos}

    REPO_DIRS=( base updates testing devel extras )
    for rep in ${REPO_DIRS[@]}
    do
        echo mkdir -p ${REPO_DIR}/${rep}/{RPMS,SRPMS}
        echo createrepo ${REPO_DIR}/${rep}/RPMS
        echo createrepo ${REPO_DIR}/${rep}/SRPMS
    done
    echo "Done creating repository structure"
fi

echo "Figuring out appropriate tag"
TAG_REPO_TYPE='testing'
## choose the correct of: base|updates|testing|devel
if [[ ${BUILD_TAG} =~ '(alpha|beta|pre|rc)' ]]
then
    ## testing for tag vx.y.z-(alpha|beta|pre|rc)\d+-git<hash>
    TAG_REPO_TYPE='testing'
elif [[ ${BUILD_TAG} =~ 'dev' ]]
then
    ## devel for tag vx.y.z-dev\d+-git<hash>
    TAG_REPO_TYPE='devel'
elif [[ ${BUILD_VER} =~ '0$' ]]
then
    ## base for tag vx.y.0
    TAG_REPO_TYPE='base'
else
    ## updates for tag vx.y.z, z> 0
    TAG_REPO_TYPE='updates'
fi

echo "Tag ${BUILD_VER}-${BUILD_TAG} determined to be ${TAG_REPO_TYPE}"

CI_REPO_DIR=${REPO_DIR}/${TAG_REPO_TYPE}
echo mkdir ${ARTIFACT_DIR}/done

## source RPM
echo rsync ${ARTIFACT_DIR}/*.src.rpm  ${KRB_USERNAME}@lxplus:${CI_REPO_DIR}/SRPMS
echo mv    ${ARTIFACT_DIR}/*.src.rpm  ${ARTIFACT_DIR}/done

## primary RPM, devel RPM, and debuginfo RPM
echo rsync ${ARTIFACT_DIR}/*-debuginfo*.rpm  ${KRB_USERNAME}@lxplus:${CI_REPO_DIR}/RPMS
echo mv    ${ARTIFACT_DIR}/*-debuginfo*.rpm  ${ARTIFACT_DIR}/done
echo rsync ${ARTIFACT_DIR}/*-devel*.rpm      ${KRB_USERNAME}@lxplus:${CI_REPO_DIR}/RPMS
echo mv    ${ARTIFACT_DIR}/*-deveo*.rpm      ${ARTIFACT_DIR}/done
echo rsync ${ARTIFACT_DIR}/*.rpm             ${KRB_USERNAME}@lxplus:${CI_REPO_DIR}/RPMS

## update the repositories
echo "Updating the repository"
echo createrepo --update ${CI_REPO_DIR}/RPMS
echo createrepo --update ${CI_REPO_DIR}/SRPMS

## update the groups?

##### Documentation, only done for final tags?
echo "Publishing documentation"
TAG_DOC_DIR=${RELEASE_DIR}/api/${BUILD_VER}
LATEST_TAG_DOC_DIR=${RELEASE_DIR}/api/latest
echo rsync ${ARTIFACT_DIR}/doc/html/ ${KRB_USERNAME}@lxplus:${RELEASE_DIR}/api/${BUILD_VER}
echo "ln -sf ${TAG_DOC_DIR} ${LATEST_TAG_DOC_DIR}"

## update the index file?
