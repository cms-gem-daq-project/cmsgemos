
import os, sys
from distutils.core import setup
from os.path import join

_rpmVersion       = '__version__'
_name             = '__packagename__'
_author           = 'GEM Online Systems Group'
_author_email     = 'cms-gem-daq@cern.ch'
_description      = '__summary__'
_long_description = '__description__'
_url              = 'https://cms-gem-daq-project.github.io/cmsgemos/'
_packages         = __python_packages__


setup(name=_name,
      version          = _rpmVersion,
      description      = _description,
      long_description = _long_description,
      author           = _author,
      author_email     = _author_email,
      url              = _url,
      packages         = _packages,
      package_dir      = {'' : ''},
      package_data     = dict((pkg,['*.so']) for pkg in _packages)
      )

