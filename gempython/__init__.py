__path__ = __import__('pkgutil').extend_path(__path__, __name__)

try:
    from _cmsgemos_gempython import *
except ImportError as e:
    import sys
    from os import environ
    if ('LD_LIBRARY_PATH' not in environ) or '/opt/cmsgemos/lib' not in environ['LD_LIBRARY_PATH'].split(':'):
        new_msg = (e.message +
                   "\nN.B. ImportError raised when cmsgemos's __init__.py tries to load python bindings library" +
                   '\n     Maybe you need to add "/opt/cmsgemos/lib", or some other path, to the "LD_LIBRARY_PATH" environment variable?')
        raise type(e), type(e)(new_msg), sys.exc_info()[2]
    else:
        raise
                                       
from _version import __version__   as __version__
from _version import __builddate__ as __builddate__
from _version import __gitrev__    as __gitrev__
from _version import __release__   as __release__
from _version import __buildtag__  as __buildtag__
from _version import __gitver__    as __gitver__
from _version import __version__   as __version__
from _version import __packager__  as __packager__

# ##################################################
# # Pythonic additions to gem::python::exception API

# def _exception_to_string(self):
#    return self.what

# exception.__str__ = _exception_to_string

