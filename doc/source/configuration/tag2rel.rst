.. _tag2rel-script:

============================
Understanding ``tag2rel.sh``
============================

The ``tag2rel.sh`` script is used to extract release and tag meta information from the status of the ``git`` repository.

A look at the usage docstring will hopefully provide useful information.

.. code-block:: sh

   usage() {
       usage="Usage: $0 <tag>
     Options:
       <tag> is (optionally) the tag being used to create a release in the form vX.Y.Z(-(alpha|beta|pre|rc)[0-9]+)?(-git[a-fA-F0-9]{6,8})?
     Returns:
       * Major is the current major version number
       * Minor is the current minor version number
       * Patch is the current patch version number
       * Version(version) is the base X.Y.Z version of the tag which is the parent of the commit being built
       * Release(relver) is the release version added to an RPM i.e., X.Y.Z-relver (see https://fedoraproject.org/wiki/Package_Versioning_Examples for examples)
       * FullVersion
       * TagVersion is the full name of the tag which is a parent of this commit
       * Revision(gitrev) is the git revision hash of the commit
       * GitVersion(gitver) e.g., v0.99.0-pre10-2-g47878f-dirty
   Major:0 Minor:99 Patch:0 Release:0.3.pre10 Version:0.99.0 FullVersion:v0.99.0-pre10 TagVersion:v0.99.0 Revision:47878ff GitVersion:v0.99.0-pre10-2-g47878f-dirty
   
     Description:
       This script aims to provide a unique identifier for every built version of a package.
       Whether for a build done by the CI system, or a local build made by a developer, certain information will be encoded
       For an untagged release, gitver will be used to determine the base tag, and will compute the distance from this most recent, adding this as a devX release for python and -0.0.X.d
   ev for the RPM 'Release' tag (alpha|beta|pre|rc)X releases (from the tag) will be built as adding this as a pre-release build (alpha|beta|pre|rc)X release for python and -0.Y.X.(alph
   a|beta|pre|rc) for the RPM 'Release' tag, where Y indicates 1,2,3,4 for alpha,beta,pre,rc (resp.)
       If the parent tag is a complete tag, the returned tag will be bumped and the Release indicates the number of commits since that tag and the .devX designation
   "
       return 0
   }


Sample output
-------------

.. code-block:: sh

   ./tag2rel.sh
   Major:1 Minor:0 Patch:1 Release:1.0.6.dev Version:1.0.1 FullVersion:v1.0.1 TagVersion:v1.0.1 BuildTag:-final.dev6 Revision:2a4c3eee GitVersion:v1.0.1-6-g2a4c3e-dirty NextMajorVer:2.0.0 NextMinorVer:1.1.0 NextPatchVer:1.0.2

What this output is saying is that the nearest parent ``git`` tag is ``v1.0.1``, and that current commit (``2a4c3eee``) is 6 commits beyond this tag.
This will result in the ``Version`` of the package being set to ``1.0.1``, and the ``Release`` entry in the ``rpm`` metadata will be set to ``1.0.6.dev``.
For a ``python`` package, the ``--build-tag`` passed to ``setup.py`` will be set to ``final.dev6``.
It also notes that there are unstaged or uncommitted changes in current working directory, which is dirty.
