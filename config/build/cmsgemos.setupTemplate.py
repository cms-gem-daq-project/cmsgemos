
import os, sys
from distutils.core import setup
from os.path import join

_rpmVersion       = '__version__'
_name             = '__packagename__'
_author           = '__author__'
_author_email     = '__email__'
_description      = '__summary__'
_long_description = '__description__'
_url              = '__url__'
# _requires         = __requires__
# _build_requires   = __build_requires__
_packages         = __python_packages__
_license          = 'MIT'

def readme():
    with open('README.md') as f:
        return f.read()

def getreqs():
    with open('requirements.txt') as f:
        reqs = f.readlines()
        return [x.strip() for x in reqs]

setup(name=_name,
      version          = _rpmVersion,
      description      = __description__,
      long_description = __long_description__,
      # long_description = readme(),
      author = 'GEM Online Systems Group',
      author_email = 'cms-gem-online-sw@cern.ch',
      url = 'https://cmsgemdaq.web.cern.ch/cmsgemdaq/',
      # author           = __author__,
      # author_email     = __author_email__,
      # url              = __url__,
      license          = _license,
      requires         = getreqs(),
      # build_requires   = _build_requires,
      packages         = _packages,
      package_dir      = {'' : ''},
      package_data     = dict((pkg,['*.so']) for pkg in _packages)
)
