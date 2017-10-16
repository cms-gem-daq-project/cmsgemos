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
        self.broadcastRead.argtypes = [c_uint, c_char_p, c_uint, POINTER(c_uint32)]
        self.broadcastRead.restype = c_uint

        # Define broadcast write
        self.broadcastWrite = self.lib.broadcastWrite
        self.broadcastWrite.argtypes = [c_uint, c_char_p, c_uint, c_uint]
        self.broadcastWrite.restype = c_uint

        # Define VFAT3 Configuration
        #configureVFAT3s = lib.configureVFAT3s
        #configureVFAT3s.argTypes = [ c_uint, c_uint ]
        #configureVFAT3s.restype = c_uint
        
        # Define TTC Configuration
        ttcGenConf = self.lib.ttcGenConf
        ttcGenConf.restype = c_uint
        ttcGenConf.argtypes = [c_uint, c_uint]

        # Define v3 scan module
        #genScan = lib.genScan
        #genScan.restype = c_uint
        #genScan.argtypes = [c_uint, c_uint, c_uint, c_uint, c_uint, c_uint, c_uint, c_uint, c_char_p, POINTER(c_uint32)]

        return

    def connect(self, amc="eagle26"):
        try:
            if 0 != self.rpc_connect(amc):
                print("Failed to open RPC connection for device %s"%(amc))
                sys.exit(os.EX_SOFTWARE)
        except Exception as e:
            print e
