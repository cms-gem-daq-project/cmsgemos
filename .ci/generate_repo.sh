#!/bin/sh -xeu

source $(dirname $0)/utils.sh

# generate_repo.sh <os> <arch> <repodir>
GEM_OS=${1}
GEM_ARCH=${2}
ARTIFACTS_DIR=${3}
SOURCERPM_DIR=${4}
PACKAGE_NAME=${5}

## or variables should come from the parent shell
## will fail for unset or unbound variables

RELEASE_PLATFORM=${GEM_OS}_${GEM_ARCH}
# PYTHON_VERSION=${PYEXE}$(${PYEXE} -c "import sys; sys.stdout.write(sys.version[:3])")
# BUILD_COMPILER=${COMPILER}$(${COMPILER} -dumpfullversion -dumpversion | sed -e 's|\.|_|g')
# RELEASE_PLATFORM=${RELEASE_PLATFORM}_${BUILD_COMPILER}

mkdir -p ${ARTIFACTS_DIR}/repos/{tarballs,SRPMS}/${PACKAGE_NAME}
mkdir -p ${ARTIFACTS_DIR}/repos/${RELEASE_PLATFORM}/{RPMS,DEBUGRPMS}/${PACKAGE_NAME}

find ${SOURCERPM_DIR} \( -type d -wholename ${ARTIFACTS_DIR}/repos \) -prune -o -iname '*.src.rpm' \
     -print0 -exec mv -t ${ARTIFACTS_DIR}/repos/SRPMS/${PACKAGE_NAME} {} \+ 2>&1 > /dev/null

find ${SOURCERPM_DIR} \( -type d -wholename ${ARTIFACTS_DIR}/repos \) -prune -o -iname '*-debuginfo*.rpm' \
     -print0 -exec mv -t ${ARTIFACTS_DIR}/repos/${RELEASE_PLATFORM}/DEBUGRPMS/${PACKAGE_NAME} {} \+ 2>&1 > /dev/null

find ${SOURCERPM_DIR} \( -type d -wholename ${ARTIFACTS_DIR}/repos \) -prune -o -iname '*.rpm' \
     -print0 -exec mv -t ${ARTIFACTS_DIR}/repos/${RELEASE_PLATFORM}/RPMS/${PACKAGE_NAME} {} \+ 2>&1 > /dev/null

find ${SOURCERPM_DIR} \( -type d -wholename ${ARTIFACTS_DIR}/repos \) -prune -o \
     \( -iname '*.tar.gz' -o -iname '*.tar.bz' -o -iname '*.tbz2' -o -iname '*.tgz' -o -iname '*.zip' \) \
     -print0 -exec mv -t ${ARTIFACTS_DIR}/repos/tarballs/${PACKAGE_NAME} {} \+ 2>&1 > /dev/null

rename tar. t ${ARTIFACTS_DIR}/repos/tarballs/${PACKAGE_NAME}*tar*

### dump the yum repo file
if [[ ${REL_VERSION} =~ PKG ]]
then
    REL_VERSION=$(echo ${REL_VERSION} | sed "s|PKG|${PACKAGE_NAME}|g")
    cat <<EOF > ${ARTIFACTS_DIR}/repos/gemos_${REL_VERSION/./_}_${RELEASE_PLATFORM}.repo
[gemos-unstable]
name     = gemos -- unstable RPMs
baseurl  = \${EOS_SITE_URL}/sw/gemos/repos/unstable/${RELEASE_PLATFORM}/RPMS
gpgkey   = \${EOS_SITE_URL}/sw/gemos/repos/RPM-GPG-KEY-gemos
enabled  = 0
gpgcheck = 1
repo_gpgcheck=1

[gemos-unstable-sources]
name     = gemos -- unstable source RPMs
baseurl  = \${EOS_SITE_URL}/sw/gemos/repos/unstable/SRPMS/${RELEASE_PLATFORM}
gpgkey   = \${EOS_SITE_URL}/sw/gemos/repos/RPM-GPG-KEY-gemos
enabled  = 0
gpgcheck = 1
repo_gpgcheck=1

[gemos-unstable-debug]
name     = gemos -- unstable debuginfo RPMS
baseurl  = \${EOS_SITE_URL}/sw/gemos/repos/unstable/${RELEASE_PLATFORM}/DEBUGRPMS
gpgkey   = \${EOS_SITE_URL}/sw/gemos/repos/RPM-GPG-KEY-gemos
enabled  = 0
gpgcheck = 1
repo_gpgcheck=1
EOF
else
    cat <<EOF > ${ARTIFACTS_DIR}/repos/gemos_${REL_VERSION/./_}_${RELEASE_PLATFORM}.repo
[gemos-base]
name     = gemos -- ${REL_VERSION} RPMs
baseurl  = \${EOS_SITE_URL}/sw/gemos/repos/releases/${REL_VERSION}/base/${RELEASE_PLATFORM}/RPMS
gpgkey   = \${EOS_SITE_URL}/sw/gemos/repos/RPM-GPG-KEY-gemos
enabled  = 1
gpgcheck = 1
repo_gpgcheck=1

[gemos-base-sources]
name     = gemos -- ${REL_VERSION} source RPMs
baseurl  = \${EOS_SITE_URL}/sw/gemos/repos/releases/${REL_VERSION}/base/SRPMS
gpgkey   = \${EOS_SITE_URL}/sw/gemos/repos/RPM-GPG-KEY-gemos
enabled  = 0
gpgcheck = 1
repo_gpgcheck=1

[gemos-base-debug]
name     = gemos -- ${REL_VERSION} debuginfo RPMs
baseurl  = \${EOS_SITE_URL}/sw/gemos/repos/releases/${REL_VERSION}/base/${RELEASE_PLATFORM}/DEBUGRPMS
gpgkey   = \${EOS_SITE_URL}/sw/gemos/repos/RPM-GPG-KEY-gemos
enabled  = 0
gpgcheck = 1
repo_gpgcheck=1

[gemos-prerel]
name     = gemos -- ${REL_VERSION} prerel RPMs
baseurl  = \${EOS_SITE_URL}/sw/gemos/repos/releases/${REL_VERSION}/prerel/${RELEASE_PLATFORM}/RPMS
gpgkey   = \${EOS_SITE_URL}/sw/gemos/repos/RPM-GPG-KEY-gemos
enabled  = 0
gpgcheck = 1
repo_gpgcheck=1

[gemos-prerel-sources]
name     = gemos -- ${REL_VERSION} prerel source RPMs
baseurl  = \${EOS_SITE_URL}/sw/gemos/repos/releases/${REL_VERSION}/prerel/${RELEASE_PLATFORM}/SRPMS
gpgkey   = \${EOS_SITE_URL}/sw/gemos/repos/RPM-GPG-KEY-gemos
enabled  = 0
gpgcheck = 1
repo_gpgcheck=1

[gemos-prerel-debug]
name     = gemos -- ${REL_VERSION} prerel debuginfo RPMs
baseurl  = \${EOS_SITE_URL}/sw/gemos/repos/releases/${REL_VERSION}/prerel/${RELEASE_PLATFORM}/DEBUGRPMS
gpgkey   = \${EOS_SITE_URL}/sw/gemos/repos/RPM-GPG-KEY-gemos
enabled  = 0
gpgcheck = 1
repo_gpgcheck=1

[gemos-testing]
name     = gemos -- ${REL_VERSION} testing RPMs
baseurl  = \${EOS_SITE_URL}/sw/gemos/repos/releases/${REL_VERSION}/testing/${RELEASE_PLATFORM}/RPMS
gpgkey   = \${EOS_SITE_URL}/sw/gemos/repos/RPM-GPG-KEY-gemos
enabled  = 0
gpgcheck = 1
repo_gpgcheck=1

[gemos-testing-sources]
name     = gemos -- ${REL_VERSION} testing source RPMs
baseurl  = \${EOS_SITE_URL}/sw/gemos/repos/releases/${REL_VERSION}/testing/${RELEASE_PLATFORM}/SRPMS
gpgkey   = \${EOS_SITE_URL}/sw/gemos/repos/RPM-GPG-KEY-gemos
enabled  = 0
gpgcheck = 1
repo_gpgcheck=1

[gemos-testing-debug]
name     = gemos -- ${REL_VERSION} testing debuginfo RPMs
baseurl  = \${EOS_SITE_URL}/sw/gemos/repos/releases/${REL_VERSION}/testing/${RELEASE_PLATFORM}/DEBUGRPMS
gpgkey   = \${EOS_SITE_URL}/sw/gemos/repos/RPM-GPG-KEY-gemos
enabled  = 0
gpgcheck = 1
repo_gpgcheck=1

[gemos-unstable]
name     = gemos -- unstable RPMs
baseurl  = \${EOS_SITE_URL}/sw/gemos/repos/unstable/${RELEASE_PLATFORM}/RPMS
gpgkey   = \${EOS_SITE_URL}/sw/gemos/repos/RPM-GPG-KEY-gemos
enabled  = 0
gpgcheck = 1
repo_gpgcheck=1

[gemos-unstable-sources]
name     = gemos -- unstable source RPMs
baseurl  = \${EOS_SITE_URL}/sw/gemos/repos/unstable/SRPMS/${RELEASE_PLATFORM}
gpgkey   = \${EOS_SITE_URL}/sw/gemos/repos/RPM-GPG-KEY-gemos
enabled  = 0
gpgcheck = 1
repo_gpgcheck=1

[gemos-unstable-debug]
name     = gemos -- unstable debuginfo RPMS
baseurl  = \${EOS_SITE_URL}/sw/gemos/repos/unstable/${RELEASE_PLATFORM}/DEBUGRPMS
gpgkey   = \${EOS_SITE_URL}/sw/gemos/repos/RPM-GPG-KEY-gemos
enabled  = 0
gpgcheck = 1
repo_gpgcheck=1
EOF
fi

