import logging
import logging.config

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

    # log levels
    colorlevels = {
        "FATAL"   :"%s%s%s"%(background.RED,style.BRIGHT,YELLOW),
        "CRITICAL":"%s%s%s"%(background.RED,style.BRIGHT,YELLOW),
        "ERROR"   :"%s%s"%(style.BRIGHT,RED),
        "WARNING" :"%s%s"%(style.BRIGHT,YELLOW),
        "WARN"    :"%s%s"%(style.BRIGHT,YELLOW),
        "INFO"    :"%s%s"%(style.BRIGHT,GREEN),
        "DEBUG"   :"%s%s"%(style.BRIGHT,MAGENTA),
        "NOTSET"  :ENDC
        }

    pass

# from https://stackoverflow.com/questions/384076/how-can-i-color-python-logging-output#384125
class ColoredFormatter(logging.Formatter):
    def __init__(self, fmt=None, datefmt=None, use_color=True):
        logging.Formatter.__init__(self, fmt=fmt, datefmt=datefmt)
        self.use_color = use_color

    def format(self, record):
        orig = logging.Formatter.format(self, record)
        levelname = record.levelname
        if self.use_color and levelname in colors.colorlevels.keys():
            orig = "%s%s%s"%(colors.colorlevels[levelname],orig,colors.colorlevels["NOTSET"])
        return orig

dictLogConfig = {
    ## doesn't work in python26
    "version":1,
    "handlers":{
        "file":{
            "class":"logging.FileHandler",
            "formatter":"nocolor",
            "filename":"gemlogger.log"
            },
        "console":{
            "class":"logging.StreamHandler",
            "formatter":"color",
            }
        },        
    "loggers":{
        "root":{
            "handlers":["console","file"],
            "level":"INFO",
            },
        "gempython.tools.amc_user_functions_uhal":{
            "handlers":["console","file"],
            "level":"INFO",
            },
        "gempython.tools.optohybrid_user_functions_uhal":{
            "handlers":["console","file"],
            "level":"INFO",
            },
        "gempython.tools.vfat_user_functions_uhal":{
            "handlers":["console","file"],
            "level":"INFO",
            },
        "gempython.tools.scan_utils_uhal":{
            "handlers":["console","file"],
            "level":"INFO",
            },
        "gempython.utils.registers_uhal":{
            "handlers":["console","file"],
            "level":"WARNING",
            },
        "gempython.tests.amc_info_uhal":{
            "handlers":["console","file"],
            "level":"INFO",
            },
        "gempython.tests.optohybrid_info_uhal":{
            "handlers":["console","file"],
            "level":"INFO",
            },
        "gempython.tests.vfat_info_uhal":{
            "handlers":["console","file"],
            "level":"INFO",
            },
        },
    
    "formatters":{
        "nocolor":{
            ():"gempython.utils.gemlogger.ColoredFormatter",
            "format": '%(asctime)s.%(msecs)03d [%(thread)x] %(levelname)s - %(module)s::%(funcName)s <> - %(message)s',
            "datefmt":'%d %b %Y %H:%M:%S',
            "use_color":"False"
            },
        "color":{
            ():"gempython.utils.gemlogger.ColoredFormatter",
            "format": '%(asctime)s.%(msecs)03d [%(thread)x] %(levelname)s - %(module)s::%(funcName)s <> - %(message)s',
            "datefmt":'%d %b %Y %H:%M:%S',
            "use_color":"True"
            },
        }
    }
 
def getGEMLogger(logclassname=logging.getLoggerClass(), loglevel=logging.WARN,
                 logfile=None,logfilelevel=logging.DEBUG):
    """
    Extend a logger object, add a file handler, if specified, with a specific log level
    """
    import sys,os
    # # logfmt  = '%(asctime)s.%(msecs)03d [%(thread)x] %(levelname)s:%(levelno)d  '
    # logfmt  = '%(asctime)s.%(msecs)03d [%(thread)x] %(levelname)s - '
    # logfmt += '%(module)s::%(funcName)s <> - '
    # logfmt += '%(message)s'
    # datefmt = '%d %b %Y %H:%M:%S'

    # nocolor_formatter = ColoredFormatter(fmt=logfmt,datefmt=datefmt,use_color=False)
    # file_handler = None
    # if not logfile:
    #     file_handler = logging.FileHandler("/tmp/%s/python_log_file.txt"%(os.getenv("USER")))
    # else:
    #     file_handler = logging.FileHandler("%s"%(logfile))

    # file_handler.setLevel(logfilelevel)
    # file_handler.setFormatter(nocolor_formatter)

    # # improve colorized logging
    # color_formatter = ColoredFormatter(fmt=logfmt,datefmt=datefmt,use_color=True)
    # stream_handler  = logging.StreamHandler()
    # stream_handler.setLevel(loglevel)
    # stream_handler.setFormatter(color_formatter)

    # logging.config.dictConfig(dictLogConfig)
    logging.config.fileConfig("%s/gempython/utils/gemlogging_config.cfg"%(os.getenv("GEM_PYTHON_PATH")))

    logger = logging.getLogger(logclassname)

    return logger

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

def printmsg(msg, loglevel=logging.NOTSET,printonly=False):
    print colormsg(msg,loglevel,printonly)
    pass

def printmsg(msg, foreground, background):
    openfmt = "%s"%(colors.ENDC)
    print "%s%s%s%s"%(background,foreground,msg,colors.ENDC)
    pass

# will modify the source of the logger message, so not super helpful
# better to get the colorized formatter working properly
def gemdebug(loclogger,msg):
    msg = colormsg(msg,loglevel=logging.DEBUG)
    loclogger.debug(msg)

def geminfo(loclogger,msg):
    msg = colormsg(msg,loglevel=logging.INFO)
    loclogger.info(msg)

def gemwarning(loclogger,msg):
    msg = colormsg(msg,loglevel=logging.WARNING)
    loclogger.warning(msg)

def gemerror(loclogger,msg):
    msg = colormsg(msg,loglevel=logging.ERROR)
    loclogger.error(msg)

def gemfatal(loclogger,msg):
    msg = colormsg(msg,loglevel=logging.FATAL)
    loclogger.fatal(msg)

def gemcritical(loclogger,msg):
    msg = colormsg(msg,loglevel=logging.CRITICAL)
    loclogger.critical(msg)
