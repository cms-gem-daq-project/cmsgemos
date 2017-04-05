
def runCommand(cmd,log=None):
    """
    Provides a wrapper around the subprocess.call command
    cmd should be an array of the form: ["command","arg1",...]
    log should specify a logfile to write to
    """

    import datetime,os,sys
    import subprocess
    from subprocess import CalledProcessError
    from gempython.utils.gemlogger import colors,colormsg
    import logging
    logger = logging.getLogger(__name__)

    try:
        msg = "Executing command:"
        for c in cmd:
            msg+=" %s"%(c)
        logger.info(colormsg(msg,logging.INFO))
        sys.stdout.flush()
        returncode = subprocess.call(cmd,stdout=log,stderr=log)
    except CalledProcessError as e:
        msg =  "Caught exception"
        msg+=str(e)
        msg+=" running:"
        for c in cmd:
            msg+=" %s"%(c)
        logger.error(colormsg(msg,logging.ERROR))
        sys.stdout.flush()
        pass
    return returncode

def envCheck(envVar):
    import os
    if os.getenv(envVar) == None or os.getenv(envVar) == '':
        print 'You must source the environment properly, %s is not set'%envVar
        raise EnvironmentError('%s is not set'%envVar)
        pass
    return True
