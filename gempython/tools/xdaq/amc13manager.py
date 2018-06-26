#!/bin/env python

# Link to amc13 API doc: http://cactus.web.cern.ch/cactus/release/amc13/1.1/api/html/index.html

import uhal
import amc13
import sys
import os
import struct
import time

class AMC13manager:
  def __init__(self):
    pass

  def connect(self,cardname,verbosity):
    self.connection = os.getenv("GEM_ADDRESS_TABLE_PATH")+"connections.xml"
    self.verbosity = verbosity
    self.device = amc13.AMC13(self.connection, cardname+".T1", cardname+".T2") # connect to amc13. Should work like this, but has to be tested
    #reset amc13
    self.device.AMCInputEnable(0x0)
    self.reset()

  def reset(self):
    self.device.reset(self.device.Board.T1)
    self.device.reset(self.device.Board.T2)
    self.device.resetCounters()
    self.device.resetDAQ()

  def configureInputs(self, inlist):
    mask = self.device.parseInputEnableList(inlist, True)
    self.device.AMCInputEnable(mask)

  def configureTrigger(self, ena, mode = 0, burst = 1, rate = 10, rules = 0):
  # see http://cactus.web.cern.ch/cactus/release/amc13/1.1/api/html/d5/df6/classamc13_1_1_a_m_c13.html#a800843bf6119f6aaeb470e406778a9b2
    self.localTrigger = ena
    if self.localTrigger:
      self.device.configureLocalL1A(ena, mode, burst, rate, rules)

  def configureCalPulse(self, delay=50, prescale=1, repeat=True)
  ''' Configure BGO calibration pulse command on channel 0. 
  Calibration pulse command code is 0x14 and can be verified by reading GEM_AMC.TTC.CONFIG.CMD_CALPULSE
  
  :param int delay: Delay relative to local L1A in mode 0, when L1A is sent on 500 BX of each orbit. Resembles latency in a given operation mode. Default value is 50
  :param int prescale: Orbit prescale. Commands sent every prescale orbits. Default value is 1
  :param bool repeat: True for periodic commands, False for single commands. Default value is True
  
  '''
    bx = 500+delay
    self.device.configureBGOShort(0,0x14,bx,prescale,repeat)

  #def startDataTaking(self, ofile, nevents):
  def startDataTaking(self, ofile):
    if self.localTrigger:
      self.device.startContinuousL1A()
    #submit work loop here
    self.isRunning = True
    datastring = ''
    packer = struct.Struct('Q')
    pEvt = []
    read_event = self.device.readEvent
    cnt = 0
    while self.isRunning:
      nevt = self.device.read(self.device.Board.T1, 'STATUS.MONITOR_BUFFER.UNREAD_BLOCKS')
      #nevt = self.device.read(self.device.Board.T1, 'STATUS.MONITOR_BUFFER.UNREAD_EVENTS')
      print "Trying to read %s events" % nevt
      for i in range(nevt):
        pEvt += read_event()
      for word in pEvt:
        datastring += packer.pack(word)
      #if nevt > 0:
      with open (ofile+"_chunk_"+str(cnt)+".dat", "wb") as compdata:
        compdata.write(datastring)
      cnt += 1;
      time.sleep(5)

  def stopDataTaking(self):
    if self.localTrigger:
      self.device.stopContinuousL1A()
    #submit work loop here
    self.isRunning = False
