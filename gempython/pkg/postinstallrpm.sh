#!/bin/sh

patchfile=/opt/cmsgemos/etc/patches/cmsgemos.gempython.patch
if [ -e $patchfile ]
then
    cd %{python2_sitelib}
    patch -p1 < $patchfile
fi
