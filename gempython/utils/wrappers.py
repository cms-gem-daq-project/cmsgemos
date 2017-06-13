
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

def linkParser(linkstring, linkmax=None):
    """
    Provides functionality to parse a string as a list of integers
    1,2-4,6,9-12 will return [1,2,3,4,6,9,10,11,12]
    Allowed delimeters are ',', ' ', and ';'
    Will not fail on invalid options:
      - 3-3 will be ignored
      - 6-2 will be ignored
      - 2-  will be ignored
      -  -2 will be ignored
    If 'linkmax' is set, then values parsed as integers greater than 'linkmax' will be ignored
    """
    import sys,re

    from gempython.utils.gemlogger import colors,colormsg
    import logging
    logger = logging.getLogger(__name__)

    delimiters = "|".join(map(re.escape,(","," ",";")))
    first = re.split(delimiters,linkstring)
    links = []
    for link in first:
        msg = "parsing link '%s'"%(link)
        logger.info(colormsg(msg,logging.INFO))
        sys.stdout.flush()
        if link.find("-") > 0:
            sublinks = link.split("-")
            if len(sublinks) != 2:
                msg = "error parsing '%s' as list of links"%(sublinks)
                logger.warning(colormsg(msg,logging.WARN))
                sys.stdout.flush()
                continue
            try:
                low = int(sublinks[0])
                hi  = int(sublinks[1])
                if not (int(low) < int(hi)):
                    msg = "error parsing '%s' as list of links"%(sublinks)
                    logger.warning(colormsg(msg,logging.WARN))
                    sys.stdout.flush()
                    continue
                else:
                    if linkmax and hi > linkmax:
                        hi = linkmax
                        msg = "Ignoring values larger than '%d'"%(linkmax)
                        logger.warning(colormsg(msg,logging.WARN))
                        sys.stdout.flush()
                        pass
                    for li in range(low,hi+1):
                        links.append(li)
                        msg = "adding link '%d' to list of links"%(li)
                        logger.info(colormsg(msg,logging.INFO))
                        sys.stdout.flush()
                        pass
                    pass
            except Exception as e:
                msg = "Caught exception parsing '%s' as list of links"%(sublinks)
                logger.warning(colormsg(msg,logging.WARN))
                sys.stdout.flush()
                pass
            pass
        else:
            try:
                li = int(link)
                if not (linkmax and li > linkmax):
                    links.append(li)
                    msg = "adding link '%d' to list of links"%(li)
                    logger.info(colormsg(msg,logging.INFO))
                    sys.stdout.flush()
                    pass
                else:
                    msg = "Ignoring values larger than '%d'"%(linkmax)
                    logger.warning(colormsg(msg,logging.WARN))
                    sys.stdout.flush()
                    pass
            except Exception as e:
                msg = "Caught exception parsing '%s' as a link"%(link)
                logger.warning(colormsg(msg,logging.WARN))
                sys.stdout.flush()
                pass
            pass
        pass
    return [li for li in set(links)]
