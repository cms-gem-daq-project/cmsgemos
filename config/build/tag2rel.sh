#!/bin/bash -e

## Script to extract versioning information from a git tag
usage(){
    usage="Usage: $0 <tag>
  Options:
    <tag> is (optionally) the tag being used to create a release in the form vX.Y.Z(-(alpha|beta|pre|rc)[0-9]+)?(-git[a-fA-F0-9]{6,8})?
  Returns:
    * Major is the current major version number
    * Minor is the current minor version number
    * Patch is the current patch version number
    * Release(relver) is the release version added to an RPM i.e., X.Y.Z-relver (see https://fedoraproject.org/wiki/Package_Versioning_Examples for examples)
    * Version(version) is the base X.Y.Z version of the tag which is the parent of the commit being built
    * FullVersion
    * TagVersion is the full name of the tag which is a parent of this commit
    * BuildTag: 
    * Revision(gitrev) is the git revision hash of the commit
    * GitVersion(gitver) e.g., v0.99.0-pre10-2-g47878f-dirty
    * Revision:${gitrev}
    * GitVersion:${gitver}
    * NextMajorVer:${NextMajorVer}
    * NextMinorVer:${NextMinorVer}
    * NextPatchVer:${NextPatchVer}
Major:0 Minor:99 Patch:0 Release:0.3.pre10 Version:0.99.0 FullVersion:v0.99.0-pre10 TagVersion:v0.99.0 Revision:47878ff GitVersion:v0.99.0-pre10-2-g47878f-dirty

  Description:
    This script aims to provide a unique identifier for every built version of a package.
    Whether for a build done by the CI system, or a local build made by a developer, certain information will be encoded
    For an untagged release, gitver will be used to determine the base tag, and will compute the distance from this most recent, adding this as a devX release for python and -0.0.X.dev for the RPM 'Release' tag (alpha|beta|pre|rc)X releases (from the tag) will be built as adding this as a pre-release build (alpha|beta|pre|rc)X release for python and -0.Y.X.(alpha|beta|pre|rc) for the RPM 'Release' tag, where Y indicates 1,2,3,4 for alpha,beta,pre,rc (resp.)
    If the parent tag is a complete tag, the returned tag will be bumped and the Release indicates the number of commits since that tag and the .devX designation
"
    return 0
}

#   list of tags and associated rpm/python release versions:
##  ** Examples:
#        tag name       RPM Release               pre-release
#        v0.2.2         0.2.2 1                   "-git<hash>"
#         * commit 1    0.2.2 1.0.1.dev          "-final.dev1-git<hash>"
#       OR
#         * commit 1    0.2.3 0.0.1.dev          "-dev1-git<hash>"
#         * commit 2    0.2.3 0.0.2.dev          "-dev2-git<hash>"
#         * commit 3    0.2.3 0.0.3.dev          "-dev3-git<hash>"
#         * commit 4    0.2.3 0.0.4.dev          "-dev4-git<hash>"
#        v0.2.3-alpha1  0.2.3 0.1.1.alpha1       "-alpha1-git<hash>"
#         * commit 1    0.2.3 0.1.1.1.alpha1.dev "-alpha1.dev1-git<hash>"
#         * commit 2    0.2.3 0.1.1.2.alpha1.dev "-alpha1.dev2-git<hash>"
#         * commit 3    0.2.3 0.1.1.3.alpha1.dev "-alpha1.dev3-git<hash>"
#        v0.2.3-alpha2  0.2.3 0.1.2.alpha2       "-alpha2-git<hash>"
#        v0.2.3-alpha3  0.2.3 0.1.3.alpha3       "-alpha3-git<hash>"
#        v0.2.3-beta1   0.2.3 0.2.1.beta1        "-beta1-git<hash>"
#        v0.2.3-beta2   0.2.3 0.2.2.beta2        "-beta2-git<hash>"
#        v0.2.3-pre1    0.2.3 0.3.1.pre1         "-pre1-git<hash>"
#        v0.2.3-pre2    0.2.3 0.3.2.pre2         "-pre2-git<hash>"
#        v0.2.3-pre3    0.2.3 0.3.3.pre3         "-pre3-git<hash>"
#        v0.2.3-rc1     0.2.3 0.4.1.rc1          "-rc1-git<hash>"
#        v0.2.3-rc2     0.2.3 0.4.2.rc2          "-rc2-git<hash>"
#        v0.2.3         0.2.3 1                  "-git<hash>"

# if previuos tag is complete, e.g., v0.2.2, bump next version values:
#    Major: 1.0.0
#    Minor: 0.3.0
#    Patch: 0.2.3

# if there are no tags, version string should be
#    0.0.0-untagged-<shorthash>git
#    or maybe just untagged-<shorthash>git
# probably better to try to use setuptools_scm, which does this natively, but want something generically applicable

if [ -z ${1+x} ]
then
    version=$(git describe --abbrev=0 --tags --always 2>/dev/null)
else
    version=$1
fi

relver=1
gitrev=$(git rev-parse --short HEAD 2>/dev/null)
gitver=$(git describe --abbrev=6 --dirty --always --tags 2>/dev/null)

tagcommit=$(git rev-list --tags --max-count=1 2>/dev/null)
if [ ! -n ${tagcommit+x} ]
then
    lasttag=$(git describe --tags ${tagcommit} 2> /dev/null)
else
    ## In the case that there are no tags, set lasttag to the initial commit
    lasttag=$(git rev-list --max-parents=0 HEAD 2> /dev/null)
fi

revision=0
if [ ! -z ${lasttag+x} ]
then
    revision=$(git rev-list ${lasttag}.. --count  2>/dev/null)
fi

if [ -z ${revision+x} ]
then
    revision=0
fi

# basic version unit is vX.Y.Z
vre='^v?(\.)?([0-9]+)\.([0-9]+)\.([0-9]+)'
gre='(git[0-9a-fA-F]{6,8})'

fullver=${version}

if [[ $version =~ $vre$ ]]
then
    basever=$version
    if [ ${revision} = "0" ]
    then
        buildtag="-final"
    else
        ## needs a version bump somehow
        # relver=0.0.$revision.dev
        ## doesn't need a version bump
        relver=1.0.${revision}.dev
        buildtag="-final.dev${revision}"
    fi
elif [[ $version =~ $vre* ]]
then
    pretag=""

    if [[ ${version##*-} =~ ^git ]]
    then
        ## only kept for legacy reason, don't put this in your tag
        basever=${version%-*}
        prerel=${version##*-}
        version=${version%-*}
        pretag="-git"
        relnum=0
    fi

    if [[ ${version##*-} =~ ^dev ]]
    then
        ## only kept for legacy reason, don't put this in your tag
        basever=${version%-*}
        prerel=${version##*-}
        version=${version%-*}
        pretag="-dev"
        relnum=0
    fi

    if [[ ${version##*-} =~ ^(alpha|beta|pre|rc) ]]
    then
        basever=${version%-*}
        prerel=${version##*-}
        if [[ ${prerel} =~ ^alpha ]]
        then
            pretag="-alpha"
            relnum=1
        elif [[ ${prerel} =~ ^beta ]]
        then
            pretag="-beta"
            relnum=2
        elif [[ ${prerel} =~ ^pre ]]
        then
            pretag="-pre"
            relnum=3
        elif [[ ${prerel} =~ ^rc ]]
        then
            pretag="-rc"
            relnum=3
        fi
    fi

    tags=( $(git tag -l "*${basever}${pretag}*") )
    ntags=$((${#tags[@]}))

    if  [[ ${prerel} =~ ^rc ]]
    then
        # because setuptools rewrites 'pre' as 'rc', need to count the number of 'pre' tags prior to adding the number of 'rc' tags
        # or we enforce that 'pre' and 'rc' are the "same" class, 'rc'>'pre',
        # and must numerically increase e.g., pre1->pre2->pre3->rc4->rc5
        pretags=( $(git tag -l "*${basever}-pre*") )
        npretags=$((${#pretags[@]}))
        ntags=$((ntags+npretags))
    fi

    if [ ${revision} = "0" ]
    then
        relver=0.${relnum}.${ntags}.${prerel}
        buildtag="${pretag}${ntags}"
    else
        relver=0.${relnum}.${ntags}.${revision}.${prerel}
        buildtag="${pretag}${ntags}.dev${revision}"
    fi
else
    basever=untagged
    buildtag="-dev${revision}"
    relver=0.0.${revision}
    buildtag="untagged.dev${revision}"
fi

if ! [[ ${basever} =~ "untagged" ]]
then
    version=${basever##v}
    patch=${version##*.}
    version=${version%.*}
    minor=${version##*.}
    major=${version%.*}
    version=${basever##v}
else
    ## maybe try to extract tag info from Makefile (whole point was to remove this from the makefile...)?
    version=${basever##v}
    fullver=${version}-git${gitrev}
    major=0
    minor=0
    patch=0
fi

NextMajorVer=$((major+1)).0.0
NextMinorVer=${major}.$((minor+1)).0
NextPatchVer=${major}.${minor}.$((patch+1))

## Output a single parseable line? or output multiple lines?
#-relver=${relver}.cmsgemos_${basever##*v}
echo Major:${major} \
     Minor:${minor} \
     Patch:${patch} \
     Release:${relver} \
     Version:${version} \
     FullVersion:${fullver} \
     TagVersion:${basever} \
     BuildTag:${buildtag} \
     Revision:${gitrev} \
     GitVersion:${gitver} \
     NextMajorVer:${NextMajorVer} \
     NextMinorVer:${NextMinorVer} \
     NextPatchVer:${NextPatchVer}
