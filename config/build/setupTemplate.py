
import os, sys
from distutils.core import setup
from os.path import join

_rpmVersion   = '__version__'
_name         = '__packagename__'
_author       = '__author__'
_author_email = ''
_description  = '__description__'
_url          = '__url__'
_packages     = __python_packages__

def readme():
    with open('gempython.md') as f:
        return f.read()

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
