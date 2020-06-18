.. _gemostag-usage:

============================================
Creating ``xdaq`` versions with ``gemostag``
============================================

The help menu from the ``gemostag`` tool should provide enough information to get started.

.. code-block:: sh

   USAGE: gemostag [-t TAG] [-p PACKAGE] [-m MSG] [-r RELTYPE] [NAME]
    NAME     -- a github user name (or defaults to 'cms-gem-daq-project') (OPTIONAL)
    PACKAGE  -- package to create the tag for, e.g., 'hardware' for gemhardware (REQUIRED)
    MSG      -- tag message (REQUIRED)
    RELTYPE  -- if changing the code base, will update the package version file depending on the type of release
                Valid options are (case insensitive): major (maj), minor (min), patch (pat) (OPTIONAL)
    PRERELTYPE  -- add extra to the tag name to indicate pre-release, alpha, beta status (will be added as '_type')
                Valid options are (case insensitive): alpha, beta, dev, pre (OPTIONAL)

   EXAMPLE: config/gemostag -h will print this menu
   EXAMPLE: config/gemostag -d will show you which subpackages have uncommitted changes
   EXAMPLE: config/gemostag --tagdiff <tag> will show you which subpackages have differences
                                            with respect to the specified tag
   EXAMPLE: config/gemostag -p hardware will extract the value in the gemhardware/Makefile
                                        and create the tag
   EXAMPLE: config/gemostag -p hardware -r min will extract the value in the gemhardware/Makefile
                                               update the minor version by one in the Makefile, and the version.h file,
                                               commit the update to git,
                                               and create the tag based on this new version
