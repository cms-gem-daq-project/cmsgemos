import logging

class colors:
    class foreground:
        # Foreground
        WHITE     = '\033[37m'
        CYAN      = '\033[36m'
        MAGENTA   = '\033[35m'
        BLUE      = '\033[34m'
        YELLOW    = '\033[33m'
        GREEN     = '\033[32m'
        RED       = '\033[31m'
        BLACK     = '\033[30m'
        END       = '\033[39m'
        pass

    class background:
        # Background
        WHITE     = '\033[47m'
        CYAN      = '\033[46m'
        MAGENTA   = '\033[45m'
        BLUE      = '\033[44m'
        YELLOW    = '\033[43m'
        GREEN     = '\033[42m'
        RED       = '\033[41m'
        BLACK     = '\033[40m'
        END       = '\033[49m'
        pass

    WHITE     = '\033[97m'
    CYAN      = '\033[96m'
    MAGENTA   = '\033[95m'
    BLUE      = '\033[94m'
    YELLOW    = '\033[93m'
    GREEN     = '\033[92m'
    RED       = '\033[91m'
    BLACK     = '\033[90m'
    END       = '\033[99m'

    # styles
    class style:
        DIM       = '\033[2m'
        BRIGHT    = '\033[1m'
        NORMAL    = '\033[22m'
        UNDERLINE = '\033[4m'
        pass

    # resets
    ENDC      = '\033[0m'

    pass

def getGEMLogger(logclassname=logging.getLoggerClass(), loglevel=logging.WARN,
                 logfile=None,logfilelevel=logging.DEBUG):
    """
    Extend a logger object, add a file handler, if specified, with a specific log level
    """
    import sys,os
    logfmt  = '%(asctime)s.%(msecs)03d [%(thread)d] %(levelname)s:%(levelno)d  '
    logfmt += '%(module)s::%(funcName)s <> - '
    logfmt += '%(message)s'
    datefmt = '%d %b %Y %H:%M:%S'

    if not logfile:
        file_handler = logging.FileHandler("/tmp/%s/python_log_file.txt"%(os.getenv("USER")))
    else
        file_handler = logging.FileHandler("%s"%(logfile))

    file_handler.setLevel(logfilelevel)
    
    if not logfile:
        logging.basicConfig(level=loglevel,
                            format=logfmt,
                            datefmt=datefmt)
    else:
        logging.basicConfig(filename=logfile,
                            level=loglevel,
                            format=logfmt,
                            datefmt=datefmt)
    logger = logging.getLogger(logclassname)
    return logger

# class GEMLogger(logging.getLoggerClass()):

#     # def __init__(self,logclassname=logging.getLoggerClass(), loglevel=logging.WARN):
#     def __init__(self):
#         self.loglevel     = loglevel
#         self.logclassname = logclassname

#         logfmt  = '%(asctime)s.%(msecs)03d [%(thread)d] %(levelname)s:%(levelno)d  '
#         logfmt += '%(module)s::%(funcName)s <> - '
#         logfmt += '%(message)s'
#         datefmt = '%d %b %Y %H:%M:%S'

#         logging.basicConfig(level=self.loglevel,
#                             format=logfmt,
#                             datefmt=datefmt)

#         # self.gemlogger = logging.getLogger(self.logclassname)
#         # logging.setLoggerClass(GEMLogger)
#         pass

#     # CRITICAL = logging.CRITICAL # 50
#     # FATAL    = logging.CRITICAL # 50
#     # ERROR    = logging.ERROR    # 40
#     # WARNING  = logging.WARNING  # 30
#     # WARN     = logging.WARNING  # 30
#     # INFO     = logging.INFO     # 20
#     # DEBUG    = logging.DEBUG    # 10
#     # NOTSET   = logging.NOTSET   #  0

# @staticmethod
def colormsg(msg, loglevel=logging.NOTSET,printonly=False):
    openfmt = colors.ENDC
    if loglevel == logging.CRITICAL or loglevel == logging.FATAL:
        openfmt = "%s%s%s"%(colors.background.RED,colors.style.BRIGHT,colors.YELLOW)
        pass
    elif loglevel == logging.ERROR:
        openfmt = "%s%s"%(colors.style.BRIGHT,colors.RED)
        pass
    elif loglevel == logging.WARNING or loglevel == logging.WARN:
        openfmt = "%s%s"%(colors.style.BRIGHT,colors.YELLOW)
        pass
    elif loglevel == logging.INFO and not printonly:
        openfmt = "%s%s"%(colors.style.BRIGHT,colors.CYAN)
        pass
    elif loglevel == logging.INFO:
        openfmt = "%s%s"%(colors.style.BRIGHT,colors.GREEN)
        pass
    elif loglevel == logging.DEBUG and not printonly:
        openfmt = "%s%s"%(colors.style.BRIGHT,colors.WHITE)
        pass
    elif loglevel == logging.DEBUG:
        openfmt = "%s%s"%(colors.style.BRIGHT,colors.MAGENTA)
        pass
    elif loglevel == logging.NOTSET:
        openfmt = "%s"%(colors.ENDC)
        pass
    return "%s%s%s"%(openfmt,msg,colors.ENDC)

# @staticmethod
def printmsg(msg, loglevel=logging.NOTSET,printonly=False):
    print colormsg(msg,loglevel,printonly)
    pass

def printmsg(msg, foreground, background):
    openfmt = "%s"%(colors.ENDC)
    print "%s%s%s%s"%(background,foreground,msg,colors.ENDC)
    pass

# pass
# printmsg("CRITICAL",logging.CRITICAL)
# printmsg("FATAL",   logging.FATAL)
# printmsg("ERROR",   logging.ERROR)
# printmsg("WARNING", logging.WARNING)
# printmsg("WARN",    logging.WARN)
# printmsg("INFO",    logging.INFO)
# printmsg("DEBUG",   logging.DEBUG)
# printmsg("NOTSET",  logging.NOTSET)
# printmsg("OK",      logging.INFO, printonly=True)
# printmsg("ALT",     logging.DEBUG,printonly=True)
