#!/bin/sh

# default action
python setup.py install --single-version-externally-managed -O1 --root=$RPM_BUILD_ROOT --record=INSTALLED_FILES

# install 'scripts' to /opt/cmsgemos/bin
mkdir -p %{buildroot}/opt/cmsgemos/bin
cp -rfp gempython/scripts/*.py %{buildroot}/opt/cmsgemos/bin/
find %{buildroot} -type f -exec chmod a+r {} \;
find %{buildroot} -type f -iname '*.cfg' -exec chmod a-x {} \;
logcfg=$(find %{buildroot} -type f -name 'gemlogging_config.cfg')
perl -pi -e 's|/tmp/gempython.log|\/var\/log\/gemdaq\/gempython.log|g' $logcfg

# set permissions
cat <<EOF >>INSTALLED_FILES
%attr(-,root,root) /opt/cmsgemos/bin/*.py
%attr(0644,root,root) /usr/lib/python*/site-packages/gempython/**/*.py
%attr(0644,root,root) /usr/lib/python*/site-packages/gempython/**/*.pyo
%attr(0644,root,root) /usr/lib/python*/site-packages/gempython/**/*.pyc
%attr(0755,root,root) /usr/lib/python*/site-packages/gempython/scripts/*.py
%config(noreplace) ${logcfg##*x86_64}

EOF
echo "Modified INSTALLED_FILES"
cat INSTALLED_FILES
