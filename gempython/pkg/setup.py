
from setuptools import setup,find_packages

from os import listdir
from os.path import isfile,join

scriptdir  = 'gempython/scripts'
scriptpath = '/opt/cmsgemos/bin'
scripts    = listdir(scriptdir)

def readme():
    with open('README.md') as f:
        return f.read()

def getscripts():
    # would prefer to use this for executables, if one can control the install location
    # to be user defined rather than /usr/bin
    # return dict(('{0:s}/{1:s}'.format(scriptpath,x),
    #       '{0:s}/{1:s}'.format(scriptdir,x)) for x in scripts if isfile(join(scriptdir,x)) )
    return ['{0:s}/{1:s}'.format(scriptdir,x) for x in scripts if isfile(join(scriptdir,x)) ]
    # return ['gempython/scripts/{0:s}'.format(x) for x in scripts if isfile(join(scriptdir,x)) ]

def getpkgdata():
    # actual package data
    data = dict((pkg,['*.txt','*.so','*.cfg']) for pkg in __pythonmodules__)
    # hack just to get the build to work
    data['gempython/scripts'] = ['gempython/scripts/*.py']
    return data

def getreqs():
    with open('requirements.txt') as f:
        reqs = f.readlines()
        return [x.strip() for x in reqs]
    pass

def getVersion():
    __version__='___version___'
    __release__='___release___'
    try:
        __prerel__='-{0:s}'.format(__release__.split('.')[2])
    except Exception as e:
        __prerel__=''
    __buildtag__='___buildtag___'
    __gitrev__='___gitrev___'
    __gitver__='___gitver___'
    __packager__='___packager___'
    __builddate__='___builddate___'
    with open("gempython/core/_version.py","w") as verfile:
        verfile.write("""
## This file is generated automatically from cmsgemos_gempython setup.py
__version__='{0:s}'
__release__='{1:s}'
__buildtag__='{2:s}'
__gitrev__='{3:s}'
__gitver__='{4:s}'
__packager__='{5:s}'
__builddate__='{6:s}'
""".format(__version__,__release__,__buildtag__,__gitrev__,__gitver__,__packager__,__builddate__))
    # return '{0:s}{1:s}'.format(__version__,__prerel__)
    return '{0:s}'.format(__version__)

setup(name             = '__packagename__',
      version          = getVersion(),
      # use_scm_version  = True,
      description      = '__description__',
      long_description = readme(),
      # author           = __author__,
      author           = 'GEM Online Systems Group',
      # author_email     = __author_email__,
      author_email     = 'cms-gem-online-sw@cern.ch',
      # url              = __url__,
      url              = 'https://cms-gem-daq-project.github.io/gem-plotting-tools',
      # namespace_package = "gempython",
      # packages         = __pythonmodules__, # for PEP420 native namespace util
      packages           = find_packages(), # for pkgutil namespace method
      # package_dir      = {'' : ''},
      include_package_data = True,
      package_data     = getpkgdata(),
      # dependency_links   = ['http://cmsgemos.web.cern.ch/cmsgemos/repo/tarball/master#egg=package-1.0']
      zip_safe         = False,
      setup_requires   = [
          'setuptools>=25.0'
          # ,'setuptools_scm'
      ],
      install_requires = getreqs(),
      license          = 'MIT',
)
