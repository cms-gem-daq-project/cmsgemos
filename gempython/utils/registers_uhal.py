import sys, re, signal
import time, datetime, os
sys.path.append('${GEM_PYTHON_PATH}')

import uhal

from gempython.utils.nesteddict import nesteddict

from gempython.utils.gemlogger import colormsg

import logging
reglogger = logging.getLogger(__name__)

def setRegisterLogLevel(level):
    reglogger.setLevel(level)
    pass

gMAX_RETRIES = 5
gRetries = 0

class GEMUHALRegisterException(Exception):
    pass

def readRegister(device, register, debug=False):
    """
    read register 'register' from uhal device 'device'
    returns value of the register
    """
    global gRetries
    nRetries = 0
    msg = """%s: Trying to read register %s (%s)
address 0x%08x  mask 0x%08x  permission %s  mode 0x%08x  size 0x%08x
"""%(device,
     register,
     device.getNode(register).getPath(),
     device.getNode(register).getAddress(),
     device.getNode(register).getMask(),
     device.getNode(register).getPermission(),
     device.getNode(register).getMode(),
     device.getNode(register).getSize()
     )
    reglogger.debug(msg)

    while (nRetries < gMAX_RETRIES):
        try:
            controlChar = device.getNode(register).read()
            device.dispatch()
            return controlChar
        except uhal.exception, e:
            nRetries += 1
            gRetries += 1
            if ((nRetries % 10)==0):
                msg = "%s: read error encountered (%s), retrying operation (%d,%d)"%(device,register,nRetries,gRetries)
                msg+= e
                reglogger.warning(msg)
                continue
        pass

    msg = colormsg("%s: Failed to readRegister %s"%(device,register),logging.ERROR)
    reglogger.error(msg)

    return 0x0

def readRegisterList(device, registers, debug=False):
    """
    read registers 'registers' from uhal device 'device'
    returns values of the registers in a dict
    """
    global gRetries
    nRetries = 0
    msg = "%s\n"%(device)
    if debug:
        for reg in registers:
            msg+= "%s\n"%(reg)
            pass
        pass
    reglogger.debug(msg)

    while (nRetries < gMAX_RETRIES):
        try:
            results = nesteddict()
            for reg in registers:
                results[reg] = device.getNode(reg).read()
                pass
            msg = "%s\n"%(device)
            if (debug):
                for reg in results.keys():
                    msg+= "%s: 0x%x"%(reg,results[reg])
                    pass
                pass
            reglogger.debug(msg)

            device.dispatch()
            time.sleep(0.1)

            msg = "%s\n"%(device)
            if (debug):
                for reg in results.keys():
                    msg+= "%s: 0x%x"%(reg,results[reg])
                    pass
                pass
            reglogger.debug(msg)

            return results

        except uhal.exception, e:
            msg ="%s: read error encountered, retrying operation (%d,%d)"%(device,nRetries,gRetries),
            msg+= e
            reglogger.warning(msg)
            nRetries += 1
            gRetries += 1
            if ((nRetries % 10)==0):
                continue
            pass
        pass

    msg ="%s: Failed to readRegisterList"%(device)
    reglogger.error(msg)

    return 0x0

def readBlock(device, register, nwords, debug=False):
    """
    read block 'register' from uhal device 'device'
    returns 'nwords' values in the register
    """
    global gRetries
    nRetries = 0
    msg = """%s: Trying to read register %s (%s)
address 0x%08x  mask 0x%08x  permission %s  mode 0x%08x  size 0x%08x
"""%(device,
     register,
     device.getNode(register).getPath(),
     device.getNode(register).getAddress(),
     device.getNode(register).getMask(),
     device.getNode(register).getPermission(),
     device.getNode(register).getMode(),
     device.getNode(register).getSize()
     )
    reglogger.debug(msg)

    while (nRetries < gMAX_RETRIES):
        try:
            msg = "%s: reading %d words from register %s"%(device,nwords,register)
            reglogger.debug(msg)
            words = device.getNode(register).readBlock(nwords)
            device.dispatch()

            msg = "%s: readBlock"%(device)
            if (debug):
                for word in words:
                    msg+= "0x%x"%(word)
                    pass
                pass
            reglogger.debug(msg)

            return words
        # want to be able to return nothing in the result of a failed transaction
        except uhal.exception, e:
            nRetries += 1
            gRetries += 1
            # if ('amount of data' in e):
            #     print colors.BLUE, "bad header",register, "-> Error : ", e, colors.ENDC
            # elif ('INFO CODE = 0x4L' in e):
            #     print colors.CYAN, "read error",register, "-> Error : ", e, colors.ENDC
            # elif ('INFO CODE = 0x6L' in e or 'timed out' in e):
            #     print colors.YELLOW, "timed out",register, "-> Error : ", e, colors.ENDC
            # else:
            #     print colors.MAGENTA, "other error",register, "-> Error : ", e, colors.ENDC
            if ((nRetries % 10)==0):
                msg = "%s: read error encountered (%s), retrying operation (%d,%d)"%(device,register,nRetries,gRetries)
                msg+= e
                reglogger.warning(msg)
            continue
        pass
    msg = "%s: error encountered, retried read operation (%d)"%(device,nRetries)
    reglogger.error(msg)
    
    return []

def writeRegister(device, register, value, debug=False):
    """
    write value 'value' into register 'register' from uhal device 'device'
    """
    global gRetries
    nRetries = 0
    msg = """%s: Trying to write value 0x%x to register %s (%s)
address 0x%08x  mask 0x%08x  permission %s  mode 0x%08x  size 0x%08x
"""%(device,
     value,
     register,
     device.getNode(register).getPath(),
     device.getNode(register).getAddress(),
     device.getNode(register).getMask(),
     device.getNode(register).getPermission(),
     device.getNode(register).getMode(),
     device.getNode(register).getSize(),
     )
    reglogger.debug(msg)

    while (nRetries < gMAX_RETRIES):
        try:
            device.getNode(register).write(0xffffffff&value)
            device.dispatch()
            return

        except uhal.exception, e:
            # if ('amount of data' in e):
            #     print colors.BLUE, "bad header",register, "-> Error : ", e, colors.ENDC
            # elif ('INFO CODE = 0x4L' in e):
            #     print colors.CYAN, "read error",register, "-> Error : ", e, colors.ENDC
            # elif ('INFO CODE = 0x6L' in e or 'timed out' in e):
            #     print colors.YELLOW, "timed out",register, "-> Error : ", e, colors.ENDC
            # else:
            #     print colors.MAGENTA, "other error",register, "-> Error : ", e, colors.ENDC
            nRetries += 1
            gRetries += 1
            if ((nRetries % 10)==0) and debug:
                msg = "%s: write error encountered (%s), retrying operation (%d,%d)"%(device,register,nRetries,gRetries)
                msg+= e
                reglogger.warning(msg)
                pass
            continue
        pass

    msg = "%s: error encountered, retried test write operation (%d)"%(device,nRetries)
    reglogger.error(msg)

    pass

def writeRegisterList(device, regs_with_vals, debug=False):
    """
    write value 'value' into register 'register' from uhal device 'device'
    from an input dict
    """
    global gRetries
    nRetries = 0
    while (nRetries < gMAX_RETRIES):
        try:
            for reg in regs_with_vals.keys():
                device.getNode(reg).write(0xffffffff&regs_with_vals[reg])
                pass
            device.dispatch()
            return

        except uhal.exception, e:
            msg = "%s: write error encountered ["%(device)
            for reg in regs_with_vals:
                msg+= "%s:0x%x,"%(reg,regs_with_vals[reg])
            msg+= "], retrying operation (%d,%d)"%(nRetries,gRetries)
            msg+= e
            reglogger.warning(msg)
            nRetries += 1
            gRetries += 1
            if ((nRetries % 10)==0) and debug:
                pass
            continue
        pass
    msg = "%s: write error encountered, retried operation (%d)"%(nRetries)
    reglogger.error(msg)
    pass
