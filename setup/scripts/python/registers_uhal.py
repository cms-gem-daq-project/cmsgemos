import sys, os, time, signal, random
sys.path.append('${GEM_PYTHON_PATH}')

import uhal

from gemlogger import GEMLogger
gemlogger = GEMLogger("registers_uhal").gemlogger

gMAX_RETRIES = 5
gRetries = 0

class colors:
	WHITE   = '\033[97m'
	CYAN    = '\033[96m'
	MAGENTA = '\033[95m'
	BLUE    = '\033[94m'
	YELLOW  = '\033[93m'
	GREEN   = '\033[92m'
	RED     = '\033[91m'
	ENDC    = '\033[0m'

        
def readRegister(device, register, debug=False):
        """
	read register 'register' from uhal device 'device'
	returns value of the register
	"""
	global gRetries
	nRetries = 0
	while (nRetries < gMAX_RETRIES):
		try:
			controlChar = device.getNode(register).read()
			device.dispatch()
			return controlChar
	#want to be able to return nothing in the result of a failed transaction
		except uhal.exception, e:
			nRetries += 1
			gRetries += 1
			if ('amount of data' in e):
				msg = colors.BLUE, "bad header",register, "-> Error : ", e, colors.ENDC
                                gemlogger.debug(msg)
			elif ('INFO CODE = 0x4L' in e):
				msg = colors.CYAN, "read error",register, "-> Error : ", e, colors.ENDC
                                gemlogger.debug(msg)
			elif ('INFO CODE = 0x6L' in e or 'timed out' in e):
				msg = colors.YELLOW, "timed out",register, "-> Error : ", e, colors.ENDC
                                gemlogger.debug(msg)
			else:
				msg = colors.MAGENTA, "other error",register, "-> Error : ", e, colors.ENDC
                                gemlogger.debug(msg)
                                pass
                        if ((nRetries % 10)==0):
				msg = colors.MAGENTA,"read error encountered (%s), retrying operation (%d,%d)"%(register,nRetries,gRetries),e,colors.ENDC
                                gemlogger.warn(msg)
                                pass
                        continue
		pass
	msg = colors.RED, "error encountered, retried read operation (%d)"%(nRetries)
        gemlogger.error(msg)
	return 0x0

def readBlock(device, register, nwords, debug=False):
	"""
	read block 'register' from uhal device 'device'
	returns 'nwords' values in the register
	"""
	global gRetries
        nRetries = 0
	while (nRetries < gMAX_RETRIES):
		try:
                        msg = "reading %d words from register %s"%(nwords,register)
                        gemlogger.debug(msg)
                        words = device.getNode(register).readBlock(nwords)
			device.dispatch()
                        msg = words
                        gemlogger.debug(msg)
			return words
                # want to be able to return nothing in the result of a failed transaction
		except uhal.exception, e:
			nRetries += 1
			gRetries += 1
			if ('amount of data' in e):
				msg = colors.BLUE, "bad header",register, "-> Error : ", e, colors.ENDC
                                gemlogger.debug(msg)
			elif ('INFO CODE = 0x4L' in e):
				msg = colors.CYAN, "read error",register, "-> Error : ", e, colors.ENDC
                                gemlogger.debug(msg)
			elif ('INFO CODE = 0x6L' in e or 'timed out' in e):
				msg = colors.YELLOW, "timed out",register, "-> Error : ", e, colors.ENDC
                                gemlogger.debug(msg)
			else:
				msg = colors.MAGENTA, "other error",register, "-> Error : ", e, colors.ENDC
                                gemlogger.debug(msg)
                                pass
			if ((nRetries % 10)==0):
				msg = colors.MAGENTA,"read error encountered (%s), retrying operation (%d,%d)"%(register,nRetries,gRetries),e,colors.ENDC
                                gemlogger.warn(msg)
                                pass
			continue
		pass
	msg = colors.RED, "error encountered, retried read operation (%d)"%(nRetries)
        gemlogger.error(msg)
	return 0x0
	
def writeRegister(device, register, value, debug=False):
	"""
	write value 'value' into register 'register' from uhal device 'device'
	"""
	global gRetries
	nRetries = 0
	while (nRetries < gMAX_RETRIES):
		try:
			device.getNode(register).write(value)
			device.dispatch()
			return
		
		except uhal.exception, e:
			if ('amount of data' in e):
				msg = colors.BLUE, "bad header",register, "-> Error : ", e, colors.ENDC
                                gemlogger.debug(msg)
			elif ('INFO CODE = 0x4L' in e):
				msg = colors.CYAN, "read error",register, "-> Error : ", e, colors.ENDC
                                gemlogger.debug(msg)
			elif ('INFO CODE = 0x6L' in e or 'timed out' in e):
				msg = colors.YELLOW, "timed out",register, "-> Error : ", e, colors.ENDC
                                gemlogger.debug(msg)
			else:
				msg = colors.MAGENTA, "other error",register, "-> Error : ", e, colors.ENDC
                                gemlogger.debug(msg)
                                pass
			nRetries += 1
			gRetries += 1
			if ((nRetries % 10)==0):
				msg = colors.MAGENTA,"write error encountered (%s), retrying operation (%d,%d)"%(register,nRetries,gRetries),e,colors.ENDC
                                gemlogger.warn(msg)
                                pass
			continue
		pass
	msg = colors.RED, "error encountered, retried test write operation (%d)"%(nRetries)
        gemlogger.error(msg)
	pass
