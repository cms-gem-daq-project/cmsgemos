#!/bin/sh

# default action
python setup.py install --single-version-externally-managed -O1 --root=$RPM_BUILD_ROOT --record=INSTALLED_FILES

# install 'scripts' to /opt/cmsgemos/bin
mkdir -p %{buildroot}/opt/cmsgemos/bin
cp -rfp gempython/scripts/*.py %{buildroot}/opt/cmsgemos/bin/

# set permissions
cat <<EOF >>INSTALLED_FILES
%attr(-,root,root) /opt/cmsgemos/bin/*.py
EOF
echo "Modified INSTALLED_FILES"
cat INSTALLED_FILES
