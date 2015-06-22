import sys, os, time, signal, random
sys.path.append('/opt/gemdaq/firmware/testing/src')

import uhal

gMAX_RETRIES = 11
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


def readRegister(device,register):
	"""
	read register 'register' from uhal device 'device'
	returns value of the register
	"""
	global gRetries
	nRetries = 0
	controlChar = device.getNode(register).read()
	device.dispatch()

	return controlChar
	#want to be able to return nothing in the result of a failed transaction
	# return 0x0
	
def writeRegister(device,register, value):
	"""
	write value 'value' into register 'register' from uhal device 'device'
	"""
	global gRetries
	nRetries = 0
	device.getNode(register).write(value)
	device.dispatch()

	return
