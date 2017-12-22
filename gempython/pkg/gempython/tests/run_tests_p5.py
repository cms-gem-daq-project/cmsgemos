#!/bin/env python

def launchTests(args):
  return launchTestsArgs(*args)

def launchTestsArgs(slot, link, chamber):
  import subprocess,datetime
  import sys,os

  startTime = datetime.datetime.now().strftime("%d.%m.%Y-%H.%M.%S.%f")
  log = file("test_%s_%s.log"%(chamber,startTime),"w")
  cmd = ["test.py","-s%d"%(slot),"-g%d"%(link), "--scantest", "--getdata"]
  print "executing", cmd
  sys.stdout.flush()
  try:
    returncode = subprocess.call(cmd,stdout=log)
    print "%s had return code %d"%(cmd,returncode)
  except CalledProcessError as e:
    print "Caught exception",e
    pass

  # print os.system("echo test.py -s%d -g%d --scantest > test_%s.log"%(slot,link,chamber))
  # #os.system("test.py -s%d -g%d --scantest > test_%s.log"%(slot,link,chamber))
  return

if __name__ == '__main__':
  import sys,os
  import subprocess
  import itertools
  from multiprocessing import Pool, freeze_support
  from optparse import OptionParser

  parser = OptionParser()
  parser.add_option("-s", "--slot", type="int", dest="slot",
                    help="slot in uTCA crate", metavar="slot", default=10)
  parser.add_option("--shelf", type="int", dest="shelf",default=1,
  		  help="uTCA shelf to access", metavar="shelf")
  parser.add_option("--parallel", action="store_true", dest="parallel",
                    help="Run tests in parllel (default is false)", metavar="parallel")
  parser.add_option("-d", "--debug", action="store_true", dest="debug",
                    help="print extra debugging information", metavar="debug")
  
  (options, args) = parser.parse_args()
  
  threads = []
  chamber_config = {
    0:"SC1L1",
    1:"SC1L2",
    2:"SC27L1",
    3:"SC27L2",
    4:"SC28L1",
    5:"SC28L2",
    6:"SC29L1",
    7:"SC29L2",
    8:"SC30L1",
    9:"SC30L2"
    }
  
  print itertools.izip([options.slot for x in range(len(chamber_config))],
                       chamber_config.keys(),
                       chamber_config.values())
  
  if options.parallel:
    print "Testing threaded mode"
    freeze_support()
    pool = Pool(8)
    res = pool.map(launchTests,
                   itertools.izip([options.slot for x in range(len(chamber_config))],
                                  chamber_config.keys(),
                                  chamber_config.values()))
    print res
    # for link in chamber_config.keys():
    #   chamber = chamber_config[link]
    #   threads.append(threading.Thread(target=launchTests, args=[options.slot,link,chamber]))
    #   pass
    # for t in threads:
    #   t.start()
    #   pass
    pass
  else:
    print "Testing serial mode"
    for link in chamber_config.keys():
      chamber = chamber_config[link]
      launchTests(options.slot,link,chamber)
      pass
    pass
