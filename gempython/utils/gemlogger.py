import logging

class GEMLogger(logging.Logger):
    def __init__(self,logclassname, loglevel=logging.WARN):
        self.loglevel     = loglevel
        self.logclassname = logclassname

        logfmt  = '%(asctime)s.%(msecs)03d [%(thread)d] %(levelname)s:%(levelno)d  '
        logfmt += '%(module)s::%(funcName)s <> - '
        logfmt += '%(message)s'
        datefmt = '%d %b %Y %H:%M:%S'

        logging.basicConfig(level=self.loglevel,
                            format=logfmt,
                            datefmt=datefmt)

        self.gemlogger = logging.getLogger(self.logclassname)
        pass

    CRITICAL = logging.CRITICAL
    FATAL    = logging.FATAL
    ERROR    = logging.ERROR
    WARNING  = logging.WARNING
    WARN     = logging.WARN
    INFO     = logging.INFO
    DEBUG    = logging.DEBUG
    NOTSET   = logging.NOTSET
