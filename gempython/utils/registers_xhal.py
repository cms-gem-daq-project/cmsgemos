from ctypes import *
from gempython.utils.wrappers import envCheck

import os, sys
sys.path.append('${GEM_PYTHON_PATH}')

class rpcService:
    def __init__(self):
        envCheck("XHAL_ROOT")

        # Define the connection
        self.lib = CDLL(os.getenv("XHAL_ROOT")+"/lib/x86_64/librpcman.so")
        self.rpc_connect = self.lib.init
        self.rpc_connect.argtypes = [c_char_p]
        self.rpc_connect.restype = c_uint

        # Define broadcast read
        self.broadcastRead = self.lib.broadcastRead
        self.broadcastRead.argtypes = [c_uint, c_char_p, c_uint]
        self.broadcastRead.restype = c_uint

        # Define broadcast write
        self.broadcastWrite = self.lib.broadcastWrite
        self.broadcastWrite.argtypes = [c_uint, c_char_p, c_uint, c_uint]
        self.broadcastWrite.restype = c_uint

        return

    def connect(self, amc="eagle26"):
        self.rpc_connect(amc)
